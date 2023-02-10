/* cc4.c9 R1.1 24/4/85 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"


/**store the character in the temporary store used for the processed input
   line.This is used by preprocess.This function returns the argument to the
   calling function,and bumps up the temporary line pointer,if the line is not
   full
*/
char
keepch(char c) {
    mline[mptr] = c;
    if( mptr < MPMAX) ++mptr;
    return (c);
}

/**this function preprocess C input lines to allow the compiler to operate
   it performs the following:-
   1)   it strips out comments
   2)   it strips out strings of whitespaces,and replaces them with a single
   space
   3)   it checks for balanced '"' and '''
   4)   performs #define substitutions
   It preprocesses the line,putting the results in mline[],transfering them back
   to line when processing is complete
*/
void
preprocess() {
    int k;
    char c, sname[NAMESIZE];
    if( cmode == 0 ) return;                /*do not process #asm lines*/
    mptr=lptr=0;
    setch();
    while( ch )           /*until end of line*/
        {
            if(( ch==' ')|(ch==9))      /*look for whitespaces*/
                {
                    keepch(' ');            /*replace with single space*/
                    while(( ch==' ')|(ch==9))
                        gch();          /*dump rest of whitespaces*/
                }
            else if ( ch =='"')           /*look for strings*/
                {
                    keepch( ch );         /*and keep opening quotes*/
                    gch();                  /*and pass on rest of string*/
                    while(ch!= '"')
                        {
                            if( ch==0)            /*end of line*/
                                {
                                    error("missing quote");
                                    break;
                                }
                            keepch( gch() );
                        }
                    gch();
                    keepch('"'); /*output matching quotes*/
                }
            else if ( ch ==39 )
                {
                    keepch(39);
                    gch();
                    while( ch != 39 )
                        {
                            if( ch == 0 )
                                {
                                    error("missing apostrophe");
                                    break;
                                }
                            keepch( gch() );
                        }
                    gch();
                    keepch(39);
                }
            else if( (ch=='/')&(nch=='*')) /*strip out comments*/
                {
                    inchar(); inchar();
                    while((( ch=='*')&(nch=='/')) == 0 )
                        {
                            if( ch==0)
                                myInline();
                            else    inchar();
                            if( eof ) break;
                        }
                    inchar(); inchar();
                }
            else if( an(ch ) ) /*strip off string name and look
                                 for substitution in macro table*/
                {
                    k = 0;
                    while( an( ch ))
                        {
                            if( k < NAMEMAX)        /*store string name*/
                                {
                                    sname[k++] = ch;
                                }
                            gch();
                        }
                    sname[k] = 0;           /*append a null to terminate*/
                    if( k = findmac(sname))
                        {
                            while( c= macq[k++])    /*substitute*/
                                {
                                    keepch(c);
                                }
                        }
                    else
                        {
                            k=0;
                            while(c=sname[k++]) /*else copy first string*/
                                {
                                    keepch(c);
                                }
                        }
                }
            else keepch(gch() );
        }
    keepch(0);
    if( mptr >= MPMAX )
        error("preprocessed line too long");
    strcpy(line,mline);
    lptr = 0;
    setch();
}

/**this function adds a macro definition to the table
   the form is the string followed by a null,followed by the replacement string
   again terminated by a null.This function gives up if the macro table is full.
   nothing is returned
*/
void
addmac() {
    char sname[NAMESIZE];
    int k;
    if ( symname(sname)==0)
        {
            illname();
            kill();
            return;
        }
    k = 0;
    while( putmac( sname[ k++] ));
    while( ch==' ' | ch == 9 ) gch();
    while( putmac( gch() ) );
    if ( macptr >= MACMAX)  error("macro table full");
}

/**append the character in the macro list
   this function returns the argument
*/
char
putmac(char c) {
    macq[macptr]=c;         /*store character in the array*/
    if( macptr<= MACMAX )
        macptr++;               /*and bump up pointer if room is left*/
    return (c);
}

/**this function tries to find the macro in the #define list
   if the character is found,the function returns the index of the string to
   be used as the replacement.If no match is found,then the function returns
   zero
*/
int
findmac(char *sname) {
    char *p;
    p = macq;
    while( p < macq + macptr )         /*search up to end of current list*/
        {
            if( astreq( sname, p ,NAMEMAX ) ) /*if a match is found*/
                {
                    /*increment pointer past string and null to point to */
                    /* replacement string*/
                    return (p + 1 + strlen(p) - macq);
                }
            p = p + strlen(p) + 1;   /*no match find next string*/
            p = p + strlen(p) + 1;   /*next string is replacement string
                                       find next definition*/
        }
    return (0);                     /*no match return 0*/
}

/**this function outputs a character to the output device. 
   if output = stdout the character is output to the screen */
char
outbyte(char c) {
    if( c == 0) return (0);
    if( fputc(c,output) != c)
        {
            closeout();
            error("Output file error");
        }
    return(c);
}

/**output the string to the disk if a file is open,otherwise the terminal*/
void
outstr(char    *ptr) {
    fputs(ptr,output);
    if (ferror(output))
        {
            closeout();
            error("Output file error");
        }
}

/**output a carriage return to the output device */
void
nl() {
    outbyte( EOL );
}

/**output an error message on the output device
   the incorrect line is printed,followed by another line with an arrow pointing
   to the error,and a further line,indicating the type of error
*/
void
error(char ptr[]) {
    int k;
    FILE *store;                              /*output device store*/
    comment();
    outstr(line);                   /*output the faulty line*/
    nl();
    comment();
    k = 0;
    while( k < lptr )
        {
            if( line[k]==9)  outbyte(9);
            else outbyte(' ');
            ++k;
        }
    outbyte('^');           /*output error pointer*/
    nl();
    comment();
    if (input2)        /* include file is open */
        {
            outstr(incfnam);   /* print current file name */
            outstr(": ");
            outdec(incfline);  /* and line number */
        }
    else
        {
            outstr(infnam);
            outstr(": ");
            outdec(infline);
        }
    outstr("  ");
    outsnl(ptr);                      /*print error message*/
    ++errcnt;               /*bump up error count*/
    if ( output != stdout )     /*output was not to terminal */
        {
            store = output; /* redirect output for outdec */
            output = stdout;
            putchar(EOL);
            puts(line);               /*output faulty line to terminal*/
            k = 0;
            while( k < lptr )
                {
                    if( line[k]==9)
                        putchar(9);   /*output a tab*/
                    else putchar(' ');
                    ++k;
                }
            puts("^");
            if (input2)
                {
                    ps(incfnam);
                    ps(": ");
                    outdec(incfline);
                }
            else
                {
                    ps(infnam);
                    ps(": ");
                    outdec(infline);
                }
            putchar(' ');
            puts(ptr);
            output = store; /* restore output file */
        }
}

/**this outputs the string to the terminal only,without a carriage return
 */
char 
ps(char ptr[]) {
    fputs(ptr,stdout);
}

/**this function matches string 1 against string 2.string 1 may be longer
   than 2.If the strings match,then the length of string 2 is returned,otherwise
   0 is returned
*/
int
streq(char str1[], char str2[]) {
    int n;
    n = strlen(str2);
    if (strncmp(str1,str2,n))
        return 0;
    else
        return n;
}

int  
astreq(char str1[], char str2[], int len) {
    int k;
    k = 0;
    while( k <len )
        {
            if(( str1[k]) != (str2[k]) ) break;
            if( str1[k]==0 ) break;
            if( str2[k]==0 ) break;
            k++;
        }
    if( an(str1[k]))return (0);
    if( an(str2[k]))return (0);
    return(k);
}

int  
match(char *lit) {
    int     k;
    blanks();
    if ( k = streq(line + lptr,lit) )
        {
            lptr = lptr + k;
            setch();
            return 1;
        }
    return 0;
}

int
amatch(char *lit, int  len) {
    int     k;
    blanks();
    if ( k = astreq(line + lptr , lit , len ) )
        {
            lptr = lptr + k;
            setch();
            while ( an(ch) )
                inbyte();
            return 1;
        }
    return 0;
}

void 
blanks() {
    while (1)
        {
            while ( ch == 0 )
                {
                    myInline();
                    preprocess();
                    if (eof)
                        break;
                }
            if (( ch == ' ' ) | (ch == 9 ))
                gch();
            else return;
        }
}

void
outdec(int  number) {
    int     k;
    int     zs;
    char    c;
    zs = 0;
    k = 10000;
    if ( number < 0 )
        {
            number = ( - number );
            outbyte('-');
            if ( number < 0 )
                {
                    outstr("32768");     /* -(-32768) == -32768 !!!! */
                    return;
                }
        }
    while ( k >= 1 )
        {
            c = number / k + '0';
            if ( (c != '0' ) | ( k == 1 ) | ( zs ) )
                {
                    zs = 1;
                    outbyte(c);
                }
            number = number % k;
            k = k / 10;
        }
}

long
primary(long *lval) {
    char    *ptr;
    char    sname[NAMESIZE];
    int     num;
    long    k;

    if ( match("(") )
        {
            k = heir1(lval);
            needbrack(")");
            return k;
        }
    if ( symname(sname) )
        {
            if ( ptr = findloc(sname) )
                {
                    getloc(ptr);
                    *lval = ptr;
                    lval[1] = ptr[TYPE];
                    if ( ptr[IDENT] == POINTER )
                        lval[1] = CINT;
                    if ( ptr[IDENT] ==ARRAY )
                        return 0;
                    else return 1;
                }
            if ( ptr = findglb(sname) )
                if ( ptr[IDENT] != FUNCTION )
                    {
                        *lval = ptr;
                        lval[1] = 0;
                        if ( ptr[IDENT] != ARRAY )
                            return 1;
                        getglb();
                        outstr(ptr);
                        nl();
                        lval[1]= ptr[TYPE];
                        return 0;
                    }
            ptr = addglb( sname , FUNCTION , CINT , 0 );
            *lval =ptr;
            lval[1] = 0;
            return 0;
        }
    if ( constant(&num) )
        return ( *lval = lval[1] = 0 );
    else {
            error("invalid expression");
            immed();
            outdec(0);
            nl();
            junk();
            return 0;
        }
}

void
store(int  *lval) {
    if ( lval[1] == 0 )
        putmem(*lval);
    else putstk(lval[1]);
}

void
rvalue(long *lval) {
    if ( ( *lval != 0 ) & ( lval[1] == 0 ) )
        getmem(*lval);
    else indirect(lval[1]);
}

void
test(int  label) {
    needbrack("(");
    expression();
    needbrack(")");
    testjump(label);
}

int  
constant(int  *val) {
    if ( number(val) )
        {
            immed();
            outdec( *val );
            nl();
        }
    else if ( pstr(val) )
        {
            immed();
            outdec(*val);
            nl();
        }
    else if ( qstr(val) )
        {
            getglb();
            printlabel(litlab);
            outbyte('+');
            outdec(*val);
            nl();
        }
    else return 0;
    return 1;
}

int
number(int  *val) {
    int     k;
    int     minus,i,base;
    char    buf[20],c;
    k = 1;
    i = minus = 0;
    while ( k )
        {
            k = 0;
            if ( match("+") )
                k = 1;
            if ( match("-") )
                {
                    minus = 1;
                    k = 1;
                }
        }
    if ( numeric(ch) == 0 )
        return 0;
    while ( an(ch)&(i<19) )
        {
            buf[i++] = inbyte();
        }
    buf[i] = 0;
    if (i==19)
        {
            error("number truncated");
            while(an(ch)) inbyte();
        }
    if (*buf=='0')     /* work out base */
        {
            base = 8;
            if (toupper(buf[1])=='X')
                base = 16;
        }
    else
        base = 10;
    if (i != btoi(buf,i,&k,base))
        error("invalid number");
    if ( minus )
        k = ( - k );
    *val = k;
    return 1;
}

int
pstr(int val[]) {
    int     k;
    char    c;
    k = 0;
    if ( match("'") == 0 )
        return 0;
    while (( ch) != 39 )     /* ' */
        k = ( ( k & 255 ) << 8 ) + ( litchar() & 255 );
    gch();     /* clear trailing ' */
    *val = k;
    return 1;
}

int
qstr(int  val[]) {
    if ( match(quote) == 0 )
        return 0;
    *val = litptr;
    while ( ch != '"' )
        {
            if ( ch == 0 )
                break;
            if ( litptr >= LITMAX )
                {
                    error("string space exhausted");
                    while ( match(quote) == 0 )
                        if ( gch() == 0 )
                            break;
                    return 1;
                }
            litq[litptr++] = litchar();
        }
    gch();
    litq[litptr++] = 0;
    return 1;
}

/* sort out backslash sequences */
int  
litchar() {
    int i,oct;
    if ((ch!=92)|(nch==0))
        return gch();
    gch();
    if (ch=='n') {gch(); return EOL;}
    if (ch=='f') {gch(); return 12;}
    if (ch=='t') {gch(); return 9;}
    if (ch=='b') {gch(); return 8;}
    if (ch=='l') {gch(); return 10;}
    if (ch=='r') {gch(); return 13;}
    if (ch==92)  {gch(); return 92;}  /* '\\' for \ */
    if (ch=='"') {gch(); return '"';} /* "\"" for " */
    if (ch==39)  {gch(); return 39;}  /* '\'' for ' */
    i = 3;
    oct = 0;
    while(((i--)>0)&(ch>='0')&(ch<='7'))
        oct = (oct<<3) + gch() - '0';
    if (i==2)
        return gch();
    else
        return oct;
}
