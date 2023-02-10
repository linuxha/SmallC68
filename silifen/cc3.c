/*  cc3.c9 R1.1 26/4/85 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"

/*called from "heir11",this routine will either call the named         */
/*function ,or if the supplied pointer is zero,will call the contents  */
/*of hl                                                                */
void
callfunction(char *ptr) {             /*symbol table entry (or 0)*/
    int     nargs;

    nargs = 0;
    blanks();                       /*already saw open parens*/
    if ( ptr == 0 )
        {
            push();                 /*calling contents of D reg*/
            nargs = intwidth;
        }
    while ( streq(line + lptr , ")" ) == 0 )
        {
            if ( endst() )
                break;
            expression();           /*get an agument*/
            push();                 /*push argument*/
            nargs = nargs + intwidth;      /*count args*/
            if ( match(",") == 0 )
                break;
        }
    needbrack(")");
    if ( ptr )
        call(ptr,nargs);
    else
        callstk(nargs-intwidth); /* need to call ,s not 2,s */
    sp = modstk(sp + nargs);        /*clean up arguments*/
}

void
junk() {
    if ( an(inbyte() ) )
        while ( an(ch) )
            gch();
    else
        while( an(ch) == 0 )
            {
                if ( ch == 0 )
                    break;
                gch();
            }
    blanks();
}

int
endst() {
    blanks();
    return ((streq(line + lptr , ";" ) | ( ch == 0 )));
}

void
illname() {
    error("illegal symbol name");
    junk();
}

void
multidef(char *sname) {
    error("already defined");
    comment();
    outsnl(sname);
}

void
needbrack(char *str) {
    if ( match(str) == 0 ) {
        error("missing bracket");
    }
}

void
needlval() {
    error("must be lvalue");
}

char *
findglb(char *sname) {
    char    *ptr;
    ptr = STARTGLB;
    while ( ptr != glbptr)
        {
            if ( astreq(sname , ptr , NAMEMAX ))
                return ptr;
            ptr = ptr + SYMSIZ;
        }
    return 0;
}

char *
findloc(char *sname) {
    char    *ptr;

    ptr = STARTLOC;

    while ( ptr != locptr ) {
        if ( astreq(sname, ptr, NAMEMAX))
            return ptr;
        ptr = ptr + SYMSIZ;
    }

    return 0;
}

char *
addglb( char *sname, char id, char typ, int  value) {
    char    *ptr;
    int     *temp;
    if ( cptr = findglb(sname) )
        return cptr;
    if ( glbptr >= ENDGLB ) {
            error("global symbol table overflow");
            return 0;
        }
    cptr = ptr = glbptr;
    while ( an(*ptr++ = *sname++) ) ;       /*copy name*/
    cptr[IDENT] = id;
    cptr[TYPE] = typ;
    cptr[STORAGE] = STATIK;
    temp = cptr + OFFSET;
    *temp = value; /* write in as integer in character array */
    glbptr = glbptr + SYMSIZ;
    return cptr;
}

char *
addloc(char *sname, char id, char typ, int  value) {
    char    *ptr;
    int     *temp;
    if ( cptr = findloc(sname) )
        return cptr;
    if ( locptr >= ENDLOC )
        {
            error("local symbol table overflow");
            return 0;
        }
    cptr = ptr = locptr;
    while ( an(*ptr++ = *sname++) ) ;       /*copy name*/
    cptr[IDENT] = id;
    cptr[TYPE] = typ;
    cptr[STORAGE] = STKLOC;
    temp = cptr + OFFSET;
    *temp = value;    /* write integer into character array */
    locptr = locptr + SYMSIZ;
    return cptr;
}

/*test if next input is legal symbol name*/
int
symname(char *sname) {
    int     k;
    blanks();
    if ( alpha(ch) == 0 )
        return 0;
    k = 0;
    while ( an(ch))
        sname[k++] = gch();
    sname[k] = 0;
    return 1;
}

/*return next available internal label number*/
int  
getlabel() {
    return( ++nxtlab );
}

/*print specified number as a label*/
void 
printlabel(int  label) {
    outstr("CC");
    outdec(label);
}

/*test if a given character is alpha*/
int
alpha(char c) {
    /*   return(((c>='a')&(c<='z'))|((c>='A')&(c<='Z'))|(c=='_')); */
    return (isalpha(c)|(c=='_'));
}

/*test if a given character is numeric*/
int  
numeric(char c) {
    /*   return (( c >= '0' ) & ( c <= '9' )); */
    return isdigit(c);
}

/*test if a character is alphanumeric*/
int
an(char c) {
    if (isalpha(c))
        return 1;
    if (isdigit(c))
        return 1;
    return (c=='_');
}

/*print a cariage return and a string only to console*/
void
pl(char *str) {
    putchar(EOL);
    fputs(str,stdout);         /* dont add another EOL */
}

void
addwhile(int  ptr[]) {
    int     k;
    if ( wqptr == WQMAX ) {
            error("too many active whiles");
            return;
        }
    k = 0;
    while ( k < WQSIZ ) {
            *wqptr++ = ptr[k++];
        }
}

void
delwhile() {
    if ( readwhile() )
        wqptr = wqptr - WQSIZ;
}

int
readwhile() {
    if ( wqptr == wq ) {
            error("no active whiles");
            return 0;
    } else {
        return ( wqptr - WQSIZ);
    }
}

/* replaced by a char variable updated in gch()
   ch()
   this function returns the character pointed to by the pointer without
   incrementing the pointer
   {
   return ( line[lptr] );
   }
*/

/* replaced by a char variable updated in gch()
   nch()
   this function returns the character after the one pointed to by the line
   pointer ,unless the current character is eol , when this function returns 0
   {
   if ( ch() == 0 )
   return 0;
   else return(line[lptr + 1]);
   }
*/

/**if the current character is eol,this function returns 0.Otherwise this
   function returns the current character , and leaves the pointer at the next
   character in the line
*/
int
gch() {
    int c;
    if ( c = ch )
        {
            ++lptr;
            setch();
        }
    return c;
}

/* set up ch and nch using the current lptr */
void 
setch() {
    if (ch = nch = line[lptr]&127)
        nch = line[lptr+1]&127;
}

/**this function deletes the current line by setting the pointer to the
   start of the line,and setting the first character to eol.This in effect leaves
   a blank line
*/
void 
kill() {
    *line = lptr = ch = nch = 0;
}


int
inbyte() {
    while ( ch == 0 ) {
            if ( eof )
                return 0;
            myInline();
            preprocess();
        }
    return gch();
}

int
inchar() {
    if ( ch == 0 )
        myInline();
    if ( eof )
        return 0;
    return ( gch() );
}

void
myInline() {
    char    *p;
    FILE    *unit;

    while(1) {
            if ( input == 0 )
                openin();
            if ( eof )
                return;
            if ( input2 )       /* increment correct line counter */
                {               /* and read from include if open */
                    unit = input2;
                    ++incfline;
                }
            else
                {
                    unit = input;
                    ++infline;
                }
            kill();
            if ( fgets(line, LINEMAX, unit) == 0 )
                closein(unit);
            if (p=index(line,EOL))  /* find an EOL if present */
                *p = 0;         /* turn EOL into null */
            if (*line)         /* there is something in the line */
                {
                    if (ctext & cmode)
                        /*if not assembly code and source requested*/
                        {
                            comment();
                            outstr(line);/*output line as comment line*/
                            nl();
                        }
                    setch();
                    return;
                }
        }
}
