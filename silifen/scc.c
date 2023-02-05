/* cc1.c9 R1.1.1 4/7/85 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "scc.h"

/* Silicon Fen Software Small-C compiler R1.1 for macro assembler */

/* version string for use in ask() and header() */
#define VERSION  "Silicon Fen Software: Small-C Compiler R1.1 4/7/85"

/* define symbol table parameters */
#define SYMSIZ               14
#define SYMTBSZ              5040
#define NUMGLBS              300
#define STARTGLB             symtab
#define ENDGLB               STARTGLB + NUMGLBS * SYMSIZ
#define STARTLOC             ENDGLB + SYMSIZ
#define ENDLOC               symtab + SYMTBSZ - SYMSIZ

/*   define symbol table entry format        */

#define NAME                 0
#define IDENT                9

#define TYPE                 10
#define STORAGE              11
#define OFFSET               12

/*   system wide name size for symbols       */

#define NAMESIZE             9
#define NAMEMAX              8

/*   define possible entries for "ident"     */

#define VARIABLE             1
#define ARRAY                2
#define POINTER              3
#define FUNCTION             4

/*   define possible entries for type        */

#define CCHAR                1
#define CINT                 2

/*   define possible entries for "storage"     */

#define STATIK               1
#define STKLOC               2

/*   define the "while" statement queue        */

#define WQTABSZ              100
#define WQSIZ                4
#define WQMAX                wq + WQTABSZ - WQSIZ

/*   define entry offsets in while queue      */

#define WQSYM                0
#define WQSP                 1
#define WQLOOP               2
#define WQLAB                3

/*   define the literal pool */

#define LITABSZ              3000
#define LITMAX               LITABSZ - 1

/*   define the input line   */

#define LINESIZE             132
#define LINEMAX              LINESIZE - 1
#define MPMAX                LINEMAX

/*     define the macro (define) pool  */

#define MACQSIZE             1000
#define MACMAX               MACQSIZE - 1

/*   define statement types (tokens) */

#define STIF                 1
#define STWHILE              2
#define STRETURN             3
#define STBREAK              4
#define STCONT               5
#define STASM                6
#define STEXP                7
#define STDO                 8
#define STFOR                9

/* define filename size */

#define FNAMSIZ              20

/*   now reserve some storage words  */

char  symtab[SYMTBSZ];          /*symbol table*/
char  *glbptr;
char  *locptr;                  /*pointers to next entry*/
int   wq[WQTABSZ];              /*while queue*/
int   *wqptr;                   /*pointer to next entry*/
char  litq[LITABSZ];            /*literal pool*/
int   litptr;                   /*pointer to next entry*/
char  macq[MACQSIZE];           /*macro string buffer*/
int   macptr;                   /*and its index*/
char  line[LINESIZE];           /*parsing buffer*/
char  mline[LINESIZE];          /*temp macro buffer*/
int   iflevel;                  /* #if.. nest level */
int   skiplevel;                /* level at which #if.. skipping started */
int   lptr;
int   mptr;                     /*pointers into each*/

/*   misc storage    */

char  *version;                 /*pointer to version string */
int   intwidth;                 /*integer width in bytes 2 or 4*/
int   nxtlab;                   /*next available label*/
int   litlab;                   /*label # assigned by literal pool*/
int   sp;                       /*compiler relative stack pointer*/
int   argstk;                   /*function arg sp*/
int   ncmp;                     /*# open compound statements*/
int   errcnt;                   /*# errors in compilation*/
int   eof;                      /*set non zero on final input eof*/
int   glbflag;                  /*non-zero if internal globals*/
int   ctext;                    /*non-zero to intermix c-source*/
int   cmode;
/*non-zero while parsing  c-code zero when parsing assembly code*/
int   lastst;                   /*last executed statement type*/
char  quote[2];                 /*literal string for '"' */
char  *cptr;                    /*work pointer to any char buffer*/
int   *iptr;                    /*work pointer to any int buffer*/
int   ch,nch;                   /*current and next characters as ints*/
char  infnam[FNAMSIZ];          /*buffer for current filename*/
char  incfnam[FNAMSIZ];         /*buffer for include filename*/
int   infline;                  /*line number for input file*/
int   incfline;                 /*line number for include file*/
int   curr_arg;                 /*current place in argument list*/
int   dump;                     /*dump statistics flag*/
int   gargc;                    /*global copy of arg count*/
int   *gargv;                   /*pointer to argument array*/
FILE  *output;                  /*output fcb pointer*/
FILE  *input;                   /*input file fcb pointer*/
FILE  *input2;                  /*include file fcb pointer */
/*                   >>> start cc1 <<<                               */
/*                                                                   */
main(argc,argv)
     int argc;
     char *argv[];
     {
     version = VERSION;              /*point at version string*/
     glbptr = STARTGLB;              /*clear global symbols*/
     locptr = STARTLOC;              /*clear local symbols*/
     wqptr = wq;                     /*clear while queue*/
     mptr = -1;                      /* flag not in preprocess */
     macptr =                        /*clear macro pool*/
     litptr =                        /*clear literal pool*/
     sp =                            /*stack ptr (relative)*/
     errcnt =                        /*no errors*/
     eof =                           /*not eof yet*/
     input =                         /*no input file*/
     input2 =                        /*no include file*/
     ncmp =                          /*no open compound states*/
     ch = nch =                      /*no input chars*/
     dump =                          /* dont dump stats */
     lastst = 0;                     /*no last statement yet*/
     output = stdout;                /*default to stdout for ask()*/
     quote[1] = 0;                   /* ... all set to zero ...*/
     *quote = '"';                   /*fake a quote literal*/
     cmode = 1;                      /*enable pre-processing*/
     gargc = argc;                   /*set up global copies*/
     gargv = argv;
     curr_arg = 1;                   /*start after program name*/
/*                   compiler body                                   */
     if (argc>1)
         cl_ask();                   /* get command line options */
     else
         ask();                      /*get interactive options*/
     openout();                      /*get an output file*/
     openin();                       /*and initial input file*/
     header();                       /*intro code*/
     codeseg();                      /*declare code segment*/
     parse();                        /*process ALL input*/
     dumpextf();                     /*dump defs and refs for fns*/
     litseg();                       /*declare literal segment*/
     dumplits();                     /*then dump literal pool*/
     dataseg();                      /*data segment*/
     dumpglbs();                     /*and all static memory*/
     errorsummary();                 /*summarise errors*/
     trailer();                      /*follow-up code*/
     closeout();                     /*close the output (if any)*/
     dumpstats();                    /*dump compiler statistics*/
     return;                         /*then exit to the system*/
     }
#if 1
ask() { return; )
#else
/*                                                                   */
/*                   get options from user                           */
/*                                                                   */
ask()
     {
     int     k;
     int     num;
     kill();                                 /*clear input line*/
     nl();
     pl(version);                       /* version string defined above */
     nl();
     pl("Interleave source as comments (y/*n): ");
     getl(line);                             /*get answer*/
     if (toupper(*line) == 'Y')
            ctext = 1;                      /*user said yes*/
     else
            ctext = 0;
     /* check for module compilation */
     pl("Declare global variables (*y/n)     : ");
     getl(line);                             /*get answer*/
     if ( toupper(*line) == 'N' )
            glbflag = 0;                    /*user said no*/
     else
            glbflag = 1;
     /* check for 32 bit integers */
     pl("Generate 32 bit macro code (y/*n)   : ");
     getl(line);
     if ( toupper(*line) == 'Y' )
            intwidth = 4;       /* 32 bit for 68000 */
     else
            intwidth = 2;       /* 16 bit for 6809, 6801 */
     /*get first allowable number of compiler generated labels
     in case user will append modules*/
     while ( 1 )
            {
            pl("Starting number for labels (*0)     : ");
            getl(line);
            if ( *line == 0 )
                    {
                    num = 0;
                    break;
                    }
            if ( btoi(line,4,&num,10) )
                    break;
            }
     nxtlab = num;                           /*first label = literal pool*/
     litlab = getlabel();                    /*first label = literal pool*/
     kill();                                 /*erase line*/
     }
#endif

cl_ask()
/* fix options from command line */
    {
    int i;
    char *p;
    ctext = 0;
    i = glbflag = 1;
    intwidth = 2;       /* assume 16 bit */
    while(i<gargc)    /*look through all args*/
        {
        p = gargv[i];
        if (*p == '-')   /*this is an option arg*/
            {
            if (*++p == 0)
                {
                puts("\nNo options given after '-' on command line");
                exit(1);
                }
            while(*p)
                {
                if (*p == 'i')           /* interleave source */
                    ctext = 1;
                else if (*p == 'm')      /* compile module only */
                    glbflag = 0;
                else if (*p == '4')      /* wide integers */
                    intwidth = 4;
                else if (*p == '2')      /* narrow integers */
                    intwidth = 2;
                else if (*p == 'd')      /* dump stats */
                    dump = 1;
                else if (*p == 'n')      /* label number */
                    {
                    ++p;
                    p = p-1+btoi(p,6,&nxtlab,10); /*convert into nxtlab*/
                    }
                else if (*p == 'o')       /* skip output file */
                    break;
                else
                    {
                    pl("Unknown option: -");
                    putchar(*p);
                    putchar(EOL);
                    exit(1);
                    }
                ++p;
                }
            }
        ++i;
        }
    litlab = getlabel();
    }

dumpstats()
/* print out compiler statistics */
        {
        if (dump)
                {
                pl("compilation errors ");
                outdec(errcnt);
                pl("global symbols     ");
                outdec((glbptr-STARTGLB)/SYMSIZ);
                pl("literal characters ");
                outdec(litptr);
                pl("define characters  ");
                outdec(macptr);
                nl();
                }
        }

parse()
     /* parse all the input text */
     {
     while ( eof == 0 )              /*do until no more input*/
            {
            if ( amatch("char",4))
                    /*look for character variable definition*/
                    {
                    declglb(CCHAR);
                    ns();
                    }
            else if ( amatch("int",3))
                    /*look for integer variable definition*/
                    {
                    declglb(CINT);
                    ns();
                    }
            else if ( match("#asm"))      /*is it assembly code?*/
                    doasm();
            else if ( match("#include"))  /*is it an include statement*/
                    doinclude();          /*if so include file*/
            else if ( match("#define"))
                    /*is it a substitution definition?*/
                    addmac();             /*if so add to macro pool*/
            else newfunc();     /*if no match assume its a function*/
            blanks();                       /*force eof if pending*/
            }
     }
/*                                                                   */
/*           dump the literal pool                                   */
/*                                                                   */
dumplits()
     {
     int     j;
     int     k;
     if ( litptr == 0 )                      /*if nothing there exit*/
            return;
     printlabel(litlab);                     /*print literal label*/
     k=0;                                    /*init an index ...*/
     while ( k < litptr )                    /* to loop with*/
            {
            defbyte();                      /*pseudo op to define byte*/
            j = 10;                         /*max bytes per line*/
            while ( j--)
                    {
                    outdec((litq[ k++ ]));
                    if (( j == 0 ) | (k >= litptr) )
                            {
                            nl();
                            break;
                            }
                    outbyte(',');           /*separate bytes*/
                    }
            }
     }
/*                                                                   */
/*           dump all static variables                                */
/*                                                                   */
dumpglbs()
     {
     int j, *temp;
     cptr = STARTGLB;
     while ( cptr < glbptr )
            {
            if ( cptr[IDENT] != FUNCTION )
                    /*do if anything but function*/
                    {
                    if ( glbflag )
                       {
                       outstr(cptr);           /*output name as label*/
                       temp = cptr + OFFSET; /* get int from char array */
                       j = *temp;     /*calc # bytes*/
                       if ((cptr[TYPE] == CINT ) | (cptr[IDENT] == POINTER))
                               j=j*intwidth;
                       defstorage( j );   /* define storage */
                       pubref(cptr);
                       }
                    else
                       {
                       extvref(cptr);
                       }
                    }
            cptr = cptr + SYMSIZ;
            }
     }

dumpextf()
/* dump references to external functions */
       {
       int *temp;
       cptr = STARTGLB;
       while( cptr < glbptr )
               {
               if ( cptr[IDENT] == FUNCTION )
                       {
                       temp = cptr + OFFSET;
                       if ( *temp == 0 )
                               extfref(cptr);
                       else
                               pubref(cptr);
                       }
               cptr = cptr + SYMSIZ;
               }
       }


/*                                                                   */
/*           report any errors                                       */
/*                                                                   */
errorsummary()
     {
     /*see if there is anything hanging*/
     if ( ncmp )
            error("missing closing brackets");/*open compound statement*/
     nl();
     comment();
     outdec(errcnt);                         /*total # errors*/
     outstr(" errors in compilation\n");
     }
/*                                                                   */
/*                   declare a static variable                       */
/*                   (i.e. define for use)                           */
/*                                                                   */
/*     makes an entry into the symbol table so that subsequent       */
/*   references can call symbol by name                              */
/*                                                                   */
declglb(typ)
int  typ;
     {
     int     k;
     int     j;
     char    sname[NAMESIZE];
     while(1)
            {
            while (1)
                    {
                    if ( endst() )                  /*do line*/
                            return;
                    k = 1;                          /*assume 1 element*/
                    if ( match ("*") )              /*pointer?*/
                            j = POINTER;            /*yes*/
                    else j = VARIABLE;              /*no*/
                    if (symname(sname) == 0 )       /*name ok?*/
                            illname();              /*no*/
                    if ( findglb(sname) )           /*already there*/
                            multidef(sname);
                    if ( match ("[") )              /*array*/
                            {
                            k = needsub();          /*get size*/
                            if ( k )
                                    j = ARRAY;      /*10 = array*/
                            else j = POINTER;       /*0 = pointer*/
                            }
                    addglb(sname , j , typ , k );   /*add symbol*/
                    break;
                    }
            if ( match(",") == 0 )                  /*more?*/
                    return;
            }
     }
/*                                                                   */
/*                   declare local variables                         */
/*                   (ie define for use)                             */
/*works just like "decglb" but modifies machine stack and adds symbol*/
/*table entry with appropriate stack offset to find it again         */
/*                                                                   */
declloc(typ)
int  typ;
     {
     int     k;
     int     j;
     char    sname[NAMESIZE];
     while(1)
            {
            while(1)
                    {
                    if ( endst() )          /*do line*/
                    return;
                    if ( match ("*") )              /*pointer?*/
                            j = POINTER;            /*yes*/
                    else j = VARIABLE;              /*no*/
                    if (symname(sname) == 0 )       /*name ok?*/
                            illname();              /*no*/
                    if ( findloc(sname) )           /*already there*/
                            multidef(sname);
                    if ( match ("[") )              /*array*/
                            {
                            k = needsub();             /*get size*/
                            if ( k )
                                    {
                                    j = ARRAY;      /*10 = array*/
                                    if ( typ == CINT )
                                            k = k * intwidth;
                                    }
                            else
                                    {
                                    j = POINTER;    /*0 = pointer*/
                                    k = intwidth;
                                    }
                            }
                    else if (( typ == CCHAR) & ( j != POINTER ))
                            k = 1;
                    else k = intwidth;
                    /*change machine stack*/
                    sp = modstk(sp - k );
                    addloc (sname , j , typ , sp );
                    break;
                    }
            if (match (",") == 0 )
                    return;
            }
     }
/* cc2.c9 R1.1 3/7/85 */
/*                                                                   */
/*invoked when declared variable is followed by a "[". This routine  */
/*makes subscript the absolute size of the array                     */
/*                                                                   */
needsub()
     {
     int     num;
     if ( match("]") )                           /*null size*/
            return 0;
     if ( number(&num) == 0 )                 /*go after number*/
            {
            error("it must be a constant");     /*it isn't*/
            num = 1;                         /*so force one*/
            }
     if ( num < 0 )
            {
            error("negative size illegal");     /*can't have -ve size*/
            num =  - num;              /*correct*/
            }
     needbrack("]");                         /*force single dimension*/
     return num;                          /*and return size*/
     }
/*                                                                   */
/*           begin a function                                        */
/*                                                                   */
/*called from "parse" routine tries to make a function out of what   */
/*follows                                                            */
newfunc()
     {
     char    n[NAMESIZE];
     char    *ptr;
     int     *temp;
     if ( symname(n) == 0 )
            {
            error("illegal function or declaration");
            kill();                         /*invalidate line*/
            return;
            }
     if ( ptr = findglb(n) )                /*already in symbol table?*/
            {
            temp = ptr + OFFSET;
            if ( ptr[IDENT] != FUNCTION )
                    multidef(n);      /*already variable by that name*/
            else if ( *temp == FUNCTION )
                    multidef(n);      /*already function by that name*/
            else *temp = FUNCTION; /*otherwise we have what was*/
                         /* earlier assumed to be a function*/
            }
     /*if not in table,define as a function now*/
     else addglb( n , FUNCTION , CINT , FUNCTION );
     /*we had better see open paren for args*/
     if (match( "(" ) == 0 )
            error("missing open parenthesis");
     outstr(n);                              /*print function name*/
     outstr(" FUNC ");                       /*function macro*/
        /* number of function params output below */
     argstk = 0;                             /*init arg count*/
     while ( match (")") == 0 )              /*then count args*/
            {
            /*any legal name bumps count*/
            if ( symname(n) )
                    argstk = argstk + intwidth;
            else
                    {
                    error("illegal argument name");
                    junk();
                    }
            blanks();
           /*if not a closing paren,should be a comma*/
            if ( streq(line + lptr , ")" ) == 0 )
                    {
                    if ( match(",") == 0 )
                            error("expected comma");
                    }
            if ( endst() )
                    break;
            }
     outdec(argstk/intwidth); nl();     /* output number of params */
     locptr = STARTLOC;               /*"clear" local symbol table*/
     sp = 0;                                /*preset stack pointer*/
     while ( argstk )
            {
            /*now let user declare what types of things the args were*/
            if (amatch("char" , 4 ))
                    {
                    getarg(CCHAR);
                    ns();
                    }
            else if ( amatch("int",3))
                    {
                    getarg(CINT);
                    ns();
                    }
            else
                    {
                    error("wrong number of args");
                    break;
                    }
            }
     if ( statement() != STRETURN )  /*do statement,but if */
                   /* its a return skip cleaning up stack*/
            {
            modstk(0);
            ret();
            }
     sp = 0;                  /*reset stack ptr again*/
     locptr = STARTLOC;       /*deallocate all local variables*/
     }

/*called from "newfunc". This routine adds an entry in the local*/
/*symbol table for each named argument                          */
getarg(t)      /*t = CCHAR or CINT*/
       int  t;
       {
       char    n[NAMESIZE];
       int     j,type;
       while (1)
               {
                type = t;       /* start with basic type for each arg */
                                /* else char c, *s; makes int *s; */
               if ( argstk == 0 )
                       return;                 /*no more args*/
               if ( match("*") )
                       j = POINTER;
               else j = VARIABLE;
               if ( symname(n) == 0)
                       illname();
               if ( findloc(n) )
                       multidef(n);
               if ( match("[") )               /*pointer?*/
               /* skip all stuff between "[]"*/
                       {
                       while ( inbyte() != ']' )
                               if ( endst() )
                                       break;
                       if ( j == POINTER )     /* already pointer so */
                               type = CINT;       /* pointer to pointer */
                       j = POINTER;      /*add entry as pointer*/
                       }
               if ( j == VARIABLE )    /* fix for 6809 byte reversal */
                       type = CINT;       /* use sign extended char as int */
               addloc( n , j , type , argstk );
               argstk = argstk - intwidth;
               if ( endst() )
                       return;
               if ( match( "," ) == 0 )
                       error("expected comma");
               }
       }

/*called whenever syntax requires a statement.This routine performs */
/*that statement and returns a number telling which one             */
/*some attempt has been made to look for common statements first    */
statement()
     {
     if ( (ch == 0 ) & ( eof ) )
            return( 0 );
     else if ( amatch("char",4) )
            {
            declloc( CCHAR );
            ns();
            }
     else if ( amatch("int",3) )
            {
            declloc(CINT);
            ns();
            }
     else if ( match("{") )
            compound();
     else if ( amatch("if",2) )
            {
            doif();
            lastst = STIF;
            }
     else if ( amatch("while",5) )
            {
            dowhile();
            lastst = STWHILE;
            }
     else if ( amatch("for",3) )
            {
            dofor();
            lastst = STFOR;
            }
     else if ( amatch("return",6) )
            {
            doreturn();
            ns();
            lastst = STRETURN;
            }
     else if ( amatch("break",5) )
            {
            dobreak();
            lastst = STBREAK;
            ns();
            }
     else if ( amatch("continue",8) )
            {
            docont();
            ns();
            lastst = STCONT;
            }
     else if ( match(";") )
            ;
     else if ( amatch("do",2) )
            {
            dodo();
            lastst = STDO;
            }
     else if ( match("#asm") )
            {
            doasm();
            lastst = STASM;
            }
     /*if nothing else , assume it's an expression*/
     else
            {
            expression();
            ns();
            lastst = STEXP;
            }
     return lastst;
     }

/*called whenever syntax requires a semicolon*/
ns()
     {
     if ( match(";") == 0 )
            error("missing semicolon");
     }

/* { statement; statement; ...... } */
/*allow any number of statements to fall between {}  */
compound()
     {
     ++ncmp;                                 /*new level open*/
     while ( match("}") == 0 )
            if (eof)
                    return;
            else statement();
     --ncmp;                                 /*close current level*/
     }

/*   if (expr) statement; else statement; */
doif()
     {
     int    flev;
     int     fsp;
     int     flab1;
     int     flab2;
     flev = locptr;             /*record current local level*/
     fsp = sp;                  /*record current stack pointer*/
     flab1 = getlabel();        /*get label for false branch*/
     test(flab1);               /*get expression and branch false*/
     statement();               /*if true do a statement*/
     sp= modstk(fsp);           /*then clean up stack*/
     locptr = flev;             /*and deallocate any locals*/
     if ( amatch("else",4) == 0 )        /*if...else?*/
            {
            /*simple "if"..print false label*/
            printlabel(flab1);
            nl();
            return;                         /*and exit*/
            }
     /*an "if".."else" statement*/
     jump(flab2 = getlabel() );       /*jump around false code*/
     printlabel(flab1);
     nl();
     statement();                /*and do "else" clause*/
     sp = modstk(fsp);           /*and clean up stack pointer*/
     locptr = flev;              /*and deallocate variables*/
     printlabel(flab2);          /*print true label*/
     nl();
     }

/*  while(expr) statement; */
dowhile()
     {
     int     wq[4];                      /*allocate local queue*/
     wq[WQSYM] = locptr;                 /*record local level*/
     wq[WQSP] = sp;                      /*and stack pointer*/
     wq[WQLOOP] = getlabel();            /*and looping label*/
     wq[WQLAB] = getlabel();             /*and exit label*/
     addwhile(wq);                       /*add entry to queue*/
                                        /*for "break" statement*/
     printlabel(wq[WQLOOP]);             /*loop label*/
     nl();
     test(wq[WQLAB] );                   /*see if true*/
     statement();                        /*if so , do statement*/
     locptr = wq[WQSYM];                 /*deallocate locals*/
     sp = modstk(wq[WQSP]);              /*clean up stack pointer*/
     jump(wq[WQLOOP]);                   /*loop to label*/
     printlabel(wq[WQLAB]);              /*exit label*/
     nl();
     delwhile();                         /*delete queue entry*/
     }

/*  for(expr1;expr2;expr3) statement; */
dofor()
     {
     int     wq[4];                      /*allocate local queue*/
     int     forlab1,forlab2;            /* expr3 labels */
     wq[WQSYM] = locptr;                 /*record local level*/
     wq[WQSP] = sp;                      /*and stack pointer*/
     wq[WQLOOP] = getlabel();            /*and looping label*/
     wq[WQLAB] = getlabel();             /*and exit label*/
     addwhile(wq);                       /*add entry to queue*/
                                        /*for "break" statement*/
     needbrack("(");
     if (match(";")==0)
        {
        expression();                      /* do expr1 once and first */
        ns();
        }
     printlabel(forlab1 = getlabel());             /*loop label*/
     nl();
     if (match(";")==0)
        {
        expression();                      /* test on expr2 */
        ns();
        testjump(wq[WQLAB] );                   /*see if true*/
        }
     jump(forlab2 = getlabel());
     printlabel(wq[WQLOOP]);
     nl();
     if (match(")")==0)
        {
        expression();                   /* do expr3 at the end */
        needbrack(")");
        }
     jump(forlab1);  
     printlabel(forlab2);
     nl();
     statement();                        /* do the statement*/
     locptr = wq[WQSYM];                 /*deallocate locals*/
     sp = modstk(wq[WQSP]);              /*clean up stack pointer*/
     jump(wq[WQLOOP]);                      /* goto expr3 label*/
     printlabel(wq[WQLAB]);              /*exit label*/
     nl();
     delwhile();                         /*delete queue entry*/
     }

/* do statement while(expr); */
dodo()
        {
        int wq[4],top;
        wq[WQSYM] = locptr;
        wq[WQSP] = sp;
        wq[WQLOOP] = getlabel();
        wq[WQLAB] = getlabel();
        addwhile(wq);
        printlabel(top=getlabel());nl();
        statement();
        if (match("while")==0)
                error("missing while after do");
        printlabel(wq[WQLOOP]);
        test(wq[WQLAB]);
        jump(top);
        printlabel(wq[WQLAB]);nl();
        delwhile();
        ns();
        }


/*  return expr; */
doreturn()
     {
     int rsp;
     /* if not end of statement , get an expression*/
     if ( endst() == 0 )
            expression();
     rsp = sp;  /* note stack pointer */
     modstk(0);                              /*clean up stk*/
     ret();                                  /*and exit function*/
     sp = rsp;  /* restore for rest of function */
     }

/*  break; */
dobreak()
     {
     int     *ptr,rsp;
     /*see if any whiles open*/
     if ( ( ptr = readwhile() ) == 0 )
            return;                         /*no*/
     rsp = sp;  /* save sp */
     modstk( (ptr[WQSP] ));        /*else clean up stack pointer*/
     jump( ptr[WQLAB] );                     /*jump to exit label*/
     sp = rsp;  /* restore sp for rest of while loop */
     }

/* continue; */
docont()
     {
     int *ptr,rsp;
     /*see if any whiles open*/
     if ( ( ptr = readwhile() ) == 0 )
            return;                         /*no*/
     rsp = sp;
     modstk((ptr[WQSP]));           /*else clean up stack pointer*/
     jump(ptr[WQLOOP]);                      /*jump to loop label*/
     sp = rsp;
     }

/*  #asm ..... #endasm */
/*enters mode where assembly language statements are passed intact*/
/*through the parser*/
doasm()
     {
     cmode = 0;                              /*mark mode as "asm"*/
     while(1)
            {
            myInline();                       /*get and print lines*/
            if ( match("#endasm") )
                    break;                  /*until #endasm*/
            if ( eof )
                    break;
            outstr(line);
            nl();
            }
     kill();                                 /*invalidate line*/
     cmode = 1;                          /*then back to parse level*/
     }
/*  cc3.c9 R1.1 26/4/85 */

/*called from "heir11",this routine will either call the named         */
/*function ,or if the supplied pointer is zero,will call the contents  */
/*of hl                                                                */
callfunction(ptr)
char *ptr;                           /*symbol table entry (or 0)*/
     {
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

junk()
     {
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

endst()
     {
     blanks();
     return ((streq(line + lptr , ";" ) | ( ch == 0 )));
     }

illname()
     {
     error("illegal symbol name");
     junk();
     }

multidef(sname)
     char *sname;
     {
     error("already defined");
     comment();
     outsnl(sname);
     }

needbrack(str)
     char *str;
     {
     if ( match(str) == 0 )
            {
            error("missing bracket");
            }
     }

needlval()
     {
     error("must be lvalue");
     }

findglb(sname)
     char *sname;
     {
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

findloc(sname)
     char *sname;
     {
     char    *ptr;
     ptr = STARTLOC;
     while ( ptr != locptr )
            {
            if ( astreq(sname,ptr,NAMEMAX))
                    return ptr;
            ptr = ptr + SYMSIZ;
            }
     return 0;
     }

addglb( sname , id , typ , value )
     char *sname, id, typ;
     int  value;
     {
     char    *ptr;
     int     *temp;
     if ( cptr = findglb(sname) )
            return cptr;
     if ( glbptr >= ENDGLB )
            {
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

addloc( sname , id , typ , value )
     char *sname, id, typ;
     int  value;
     {
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
symname(sname)
     char *sname;
     {
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
getlabel()
     {
     return( ++nxtlab );
     }

/*print specified number as a label*/
printlabel(label)
     int  label;
     {
     outstr("CC");
     outdec(label);
     }

/*test if a given character is alpha*/
alpha(c)
     char c;
     {
/*   return(((c>='a')&(c<='z'))|((c>='A')&(c<='Z'))|(c=='_')); */
     return (isalpha(c)|(c=='_'));
     }

/*test if a given character is numeric*/
numeric(c)
     char c;
     {
/*   return (( c >= '0' ) & ( c <= '9' )); */
     return isdigit(c);
     }

/*test if a character is alphanumeric*/
an(c)
     char c;
     {
     if (isalpha(c))
          return 1;
     if (isdigit(c))
          return 1;
     return (c=='_');
     }

/*print a cariage return and a string only to console*/
pl(str)
     char *str;
     {
     putchar(EOL);
     fputs(str,stdout);         /* dont add another EOL */
     }

addwhile(ptr)
     int  ptr[];
     {
     int     k;
     if ( wqptr == WQMAX )
            {
            error("too many active whiles");
            return;
            }
     k = 0;
     while ( k < WQSIZ )
            {
            *wqptr++ = ptr[k++];
            }
     }

delwhile()
     {
     if ( readwhile() )
            wqptr = wqptr - WQSIZ;
     }

readwhile()
     {
     if ( wqptr == wq )
            {
            error("no active whiles");
            return 0;
            }
     else return ( wqptr - WQSIZ);
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

gch()
/**if the current character is eol,this function returns 0.Otherwise this
function returns the current character , and leaves the pointer at the next
character in the line
*/
        {
        int c;
        if ( c = ch )
                {
                ++lptr;
                setch();
                }
        return c;
        }

setch()
/* set up ch and nch using the current lptr */
        {
        if (ch = nch = line[lptr]&127)
                nch = line[lptr+1]&127;
        }

kill()
/**this function deletes the current line by setting the pointer to the
start of the line,and setting the first character to eol.This in effect leaves
a blank line
*/
        {
        *line = lptr = ch = nch = 0;
        }

inbyte()
     {
     while ( ch == 0 )
            {
            if ( eof )
                    return 0;
            myInline();
            preprocess();
            }
     return gch();
     }

inchar()
     {
     if ( ch == 0 )
            myInline();
     if ( eof )
            return 0;
     return ( gch() );
     }

myInline()
     {
     char    *p;
     FILE    *unit;
     while(1)
            {
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
/* cc4.c9 R1.1 24/4/85 */

keepch(c)
char c;
/**store the character in the temporary store used for the processed input
line.This is used by preprocess.This function returns the argument to the
calling function,and bumps up the temporary line pointer,if the line is not
full
*/
     {
     mline[mptr] = c;
     if( mptr < MPMAX) ++mptr;
     return (c);
     }

preprocess()
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
     {
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

addmac()
/**this function adds a macro definition to the table
the form is the string followed by a null,followed by the replacement string
again terminated by a null.This function gives up if the macro table is full.
nothing is returned
*/
     {
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

putmac(c)
/**append the character in the macro list
this function returns the argument
*/
char c;
     {
     macq[macptr]=c;         /*store character in the array*/
     if( macptr<= MACMAX )
             macptr++;               /*and bump up pointer if room is left*/
     return (c);
     }

findmac(sname)
/**this function tries to find the macro in the #define list
if the character is found,the function returns the index of the string to
be used as the replacement.If no match is found,then the function returns
zero
*/
char *sname;
     {
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

outbyte(c)
/**this function outputs a character to the output device. 
if output = stdout the character is output to the screen */
        char c;
        {
        if( c == 0) return (0);
        if( fputc(c,output) != c)
                {
                closeout();
                error("Output file error");
                }
        return(c);
        }

outstr(ptr)
/**output the string to the disk if a file is open,otherwise the terminal*/
        char    *ptr;
        {
        fputs(ptr,output);
        if (ferror(output))
                {
                closeout();
                error("Output file error");
                }
        }

nl()
/**output a carriage return to the output device */
        {
        outbyte( EOL );
        }

error(ptr)
/**output an error message on the output device
the incorrect line is printed,followed by another line with an arrow pointing
to the error,and a further line,indicating the type of error
*/
char ptr[];
     {
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

ps(ptr)
/**this outputs the string to the terminal only,without a carriage return
*/
     char ptr[];
     {
     fputs(ptr,stdout);
     }

streq(str1,str2)
/**this function matches string 1 against string 2.string 1 may be longer
than 2.If the strings match,then the length of string 2 is returned,otherwise
0 is returned
*/
char str1[],str2[];
     {
     int n;
     n = strlen(str2);
     if (strncmp(str1,str2,n))
             return 0;
     else
             return n;
     }

astreq(str1,str2,len)
     char str1[],str2[];
     int len;
     {
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

match(lit)
     char *lit;
     {
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

amatch(lit,len)
     char *lit;
     int  len;
     {
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

blanks()
     {
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

outdec(number)
int  number;
     {
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
/* cc5.c9 R1.0 10/3/85  recursive descent expression evaluator */

/* lval[0] - symbol table address - 0 for constant */
/* lval[1] - type of indirect obj to fetch - 0 for static */
/* this file contains assignment, logical operators and comparisons */

expression()
     {
     int     lval[2];
     if ( heir1(lval) )
            rvalue(lval);
     }

heir1(lval)
     int  lval[];
     {
     int     k;
     int     lval2[2];
     k = heir2(lval);
     if ( match("=") )          /* need an lvalue to put result in */
            {
            if ( k == 0 )
                    {
                    needlval();
                    return 0;
                    }
            if ( lval[1] )
                    push();
            if ( heir1(lval2) )
                    rvalue(lval2);
            store(lval);
            return 0;
            }
     else return k;
     }

heir2(lval)
int  lval[];
     {
     int     k;
     int     lval2[2];
     k = heir3(lval);
     blanks();
     if ( ch != '|' )
            return k;
     if ( k )
            rvalue(lval);
     while ( 1 )
            {
            if ( match("|") )
                    {
                    push();
                    if ( heir3(lval2) )
                            rvalue(lval2);
                    or();              /* auto pop in or */
                    }
            else return 0;
            }
     }

heir3(lval)
int  lval[];
     {
     int     k;
     int     lval2[2];
     k = heir4(lval);
     blanks();
     if ( ch != '^' )
            return k;
     if ( k )
            rvalue(lval);
     while ( 1 )
            {
            if ( match("^") )
                    {
                    push();
                    if ( heir4(lval2) )
                            rvalue(lval2);
                    xor();             /* auto pop in xor */
                    }
            else return 0;
            }
     }

heir4(lval)
int  lval[];
     {
     int     k;
     int     lval2[2];
     k = heir5(lval);
     blanks();
     if ( ch != '&' )
            return k;
     if ( k )
            rvalue(lval);
     while ( 1 )
            {
            if ( match("&") )
                    {
                    push();
                    if ( heir5(lval2) )
                            rvalue(lval2);
                    and();             /* auto pop in and */
                    }
            else return 0;
            }
     }

heir5(lval)
int  lval[];
     {
     int     k;
     int     lval2[2];
     k = heir6(lval);
     blanks();
     if((streq(line+lptr,"==")==0) & (streq(line+lptr,"!=") == 0 ))
            return k;
     if ( k )
            rvalue(lval);
     while ( 1 )
            {
            if ( match("==") )
                    {
                    push();
                    if ( heir6(lval2))
                            rvalue(lval2);
                    eq();  /* takes one param in D and other on stack */
                    }
            else if ( match("!=") )
                    {
                    push();
                    if ( heir6(lval2) )
                            rvalue(lval2);
                    ne();
                    }
            else return 0;
            }
     }

heir6(lval)
int  lval[];
     {
     int     k;
     int     lval2[2];
     k = heir7(lval);
     blanks();
     if ( (streq(line + lptr,"<") == 0 ) &
            (streq(line + lptr,">" ) == 0 ) &
            (streq(line + lptr , "<=" ) == 0 ) &
            (streq(line + lptr , ">=" ) == 0 ) )
                    return k;
     if ( streq( line + lptr , ">>" ) )
            return k;
     if ( streq( line + lptr , "<<" ) )
            return k;
     if ( k )
            rvalue(lval);
     while(1)
            {
            if ( match("<=") )
                    {
                    push();
                    if ( heir7(lval2) )
                            rvalue(lval2);
                    if ( cptr = *lval )
                            if (cptr[IDENT] == POINTER )
                                    {
                                    ule();
                                    continue;
                                    }
                    if ( cptr = *lval2 )
                            if ( cptr[IDENT] == POINTER )
                                    {
                                    ule();
                                    continue;
                                    }
                    le();
                    }
            else if ( match(">=") )
                    {
                    push();
                    if ( heir7(lval2) )
                            rvalue(lval2);
                    if ( cptr = *lval )
                            if ( cptr[IDENT] == POINTER )
                                    {
                                    uge();
                                    continue;
                                    }
                    if ( cptr = *lval2 )
                            if ( cptr[IDENT] == POINTER )
                                    {
                                    uge();
                                    continue;
                                    }
                    ge();
                    }
            else if((streq(line+lptr,"<"))&(streq(line+lptr,"<<")==0))
                    {
                    inbyte();
                    push();
                    if ( heir7(lval2) )
                            rvalue(lval2);
                    if ( cptr = *lval )
                            if ( cptr[IDENT] == POINTER )
                                    {
                                    ult();
                                    continue;
                                    }
                    if ( cptr = *lval2 )
                            if ( cptr[IDENT] == POINTER )
                                    {
                                    ult();
                                    continue;
                                    }
                    lt();
                    }
            else if((streq(line+lptr,">"))&(streq(line+lptr,">>")==0))
                    {
                    inbyte();
                    push();
                    if ( heir7(lval2) )
                            rvalue(lval2);
                    if ( cptr = *lval )
                            if ( cptr[IDENT] == POINTER )
                                    {
                                    ugt();
                                    continue;
                                    }
                    if ( cptr = *lval2 )
                            if ( cptr[IDENT] == POINTER )
                                    {
                                    ugt();
                                    continue;
                                    }
                    gt();
                    }
            else return 0;
            }
     }
/*  cc6.c9 R1.1 24/4/85  */

heir7(lval)
int  lval[];
     {
     int     k,lval2[2];
     k = heir8(lval);
     blanks();
     if ((streq(line+lptr,">>")== 0)&(streq(line+lptr,"<")==0 ))
            return k;
     if ( k )
            rvalue(lval);
     while(1)
            {
            if ( match(">>") )
                    {
                    push();
                    if ( heir8(lval2) )
                            rvalue(lval2);
                    casr();   /* auto stack pop */
                    }
            else if ( match("<<") )
                    {
                    push();
                    if ( heir8(lval2) )
                            rvalue(lval2);
                    casl();
                    }
            else return 0;
            }
     }

heir8(lval)
int  lval[];
     {
     int     k,lval2[2];
     k = heir9(lval);
     blanks();
     if ( (ch != '-') & (ch != '+') )
            return k;
     if ( k )
            rvalue(lval);
     while(1)
            {
            if ( match("+") )
                    {
                    push();
                    if ( heir9(lval2) )
                            rvalue(lval2);
                    if ( cptr = *lval )
                         if((cptr[IDENT]==POINTER)&(cptr[TYPE]==CINT))
                                scale(intwidth);
                    cadd();             /* auto pull in add */
                    }
            else if ( match("-") )
                    {
                    push();
                    if ( heir9(lval2) )
                         rvalue(lval2);
                    if ( cptr = *lval )
                         if((cptr[IDENT]==POINTER)&(cptr[TYPE]==CINT))
                                scale(intwidth);
                    csub();             /* auto pull in sub */
                    }
            else return 0;
            }
     }

heir9(lval)
int  lval[];
     {
     int     k;
     int     lval2[2];
     k = heir10(lval);
     blanks();
     if ( (ch != '*') & (ch != '/') & (ch != '%') )
            return k;
     if ( k )
            rvalue(lval);
     while(1)
            {
            if ( match("*") )
                    {
                    push();
                    if ( heir9(lval2) )     /**should it be heir10?**/
                            rvalue(lval2);
                    mult();            /* cleans up its own stack */
                    }
            else if ( match("/") )
                    {
                    push();
                    if ( heir10(lval2) )
                            rvalue(lval2);
                    myDiv();
                    }
            else if ( match("%") )
                    {
                    push();
                    if ( heir10(lval2) )
                            rvalue(lval2);
                    mod();
                    }
            else return 0;
            }
     }

heir10(lval)
int  lval[];
     {
     int     k;
     char    *ptr;
     if (match("++") )
            {
            if ((k=heir10(lval)) == 0 )
                    {
                    needlval();
                    return 0;
                    }
            if ( lval[1] )
                    push();
            rvalue(lval);
            ptr = *lval;
            if ( (ptr[IDENT] == POINTER ) & (ptr[TYPE] == CINT ))
                    cinc( intwidth );
            else
                    cinc( 1 );
            store(lval);
            return 0;
            }
     if (match("--") )
            {
            if ((k=heir10(lval)) == 0 )
                    {
                    needlval();
                    return 0;
                    }
            if ( lval[1] )
                    push();
            rvalue(lval);
            ptr = *lval;
            if ( (ptr[IDENT] == POINTER ) & (ptr[TYPE] == CINT ))
                    cdec( intwidth );
            else
                    cdec( 1 );
            store(lval);
            return 0;
            }
     if (match("-") )
            {
            k = heir10(lval);
            if ( k )
                    rvalue(lval);
            cneg();
            return 0;
            }
     else if ( match("*") )   /* take contents of pointer */
            {
            k = heir10(lval);
            if ( k )
                    rvalue(lval);
            lval[1] = CINT;
            if ( ptr = *lval )
                    lval[1] = ptr[TYPE];
            *lval = 0;
            return 1;
            }
     else if ( match("&") )     /* take address of variable */
            {
            k = heir10(lval);
            if ( k == 0 )
                    {
                    error("illegal address");
                    return 0;
                    }
            else if ( lval[1] )
                    return 0;
            else
                    {
                    getglb();
                    outstr(ptr = *lval );
                    nl();
                    lval[1] = ptr[TYPE];
                    return 0;
                    }
            }
     else
            {
            k = heir11(lval);
            if ( match("++") )       /* post increment */
                    {
                    if ( k == 0 )
                            {
                            needlval();
                            return 0;
                            }
                    if ( lval[1] )
                            push();
                    rvalue(lval);
                    ptr = *lval;
                    if((ptr[IDENT]==POINTER)&(ptr[TYPE]==CINT))
                            {
                            cinc( intwidth );
                            store(lval);
                            cdec( intwidth );
                            }
                    else
                            {
                            cinc( 1 );
                            store(lval);
                            cdec( 1 );
                            }
 /* the value is incremented and stored then decremented before use */
                    return 0;
                    }
            else if (match("--") )   /* post decrement */
                    {
                    if ( k == 0 )
                            {
                            needlval();
                            return 0;
                            }
                    if ( lval[1] )
                            push();
                    rvalue(lval);
                    ptr = *lval;
                    if((ptr[IDENT]==POINTER)&(ptr[TYPE]==CINT))
                            {
                            cdec( intwidth );
                            store(lval);
                            cinc( intwidth );
                            }
                    else
                            {
                            cdec( 1 );
                            store(lval);
                            cinc( 1 );
                            }
                    return 0;
                    }
            else return k;
            }
     }
/*  cc7.c9 R1.1 24/4/85  */

heir11(lval)
int  *lval;
     {
     int     k;
     char    *ptr;
     k = primary(lval);
     ptr = *lval;
     blanks();
     if ( (ch == '[' ) | ( ch == '(' ) )
            while(1)
            {
            if ( match("[") )
                    {
                    if (ptr)
                            {
                            if ( ptr[IDENT] == POINTER )
                                  rvalue(lval);
                            }
                    push();
                    expression();
                    needbrack("]");
                    if (ptr)
                        if ( ptr[TYPE] == CINT )
                            scale(intwidth);
                    cadd();             /* auto pull */
                    *lval = 0;
                    if (ptr)
                        lval[1] = ptr[TYPE];
                    else
                        lval[1] = 0;
                    k = 1;
                    }
            else if ( match("(") )
                    {
                    if ( ptr == 0 )
                            {
                            callfunction(0);
                            }
                    else if ( ptr[IDENT] != FUNCTION )
                            {
                            rvalue(lval);
                            callfunction(0);
                            }
                    else callfunction(ptr);
                    k = *lval =0;
                    }
            else return k;
            }
     if ( ptr == 0 )
            return k;
     if ( ptr[IDENT] == FUNCTION )
            {
            getglb();
            outstr(ptr);
            nl();
            return 0;
            }
     return k;
     }

primary(lval)
int  *lval;
     {
     char    *ptr;
     char    sname[NAMESIZE];
     int     num;
     int     k;
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
     else
            {
            error("invalid expression");
            immed();
            outdec(0);
            nl();
            junk();
            return 0;
            }
     }

store(lval)
int  *lval;
     {
     if ( lval[1] == 0 )
            putmem(*lval);
     else putstk(lval[1]);
     }

rvalue(lval)
int  *lval;
     {
     if ( ( *lval != 0 ) & ( lval[1] == 0 ) )
            getmem(*lval);
     else indirect(lval[1]);
     }

test(label)
int  label;
     {
     needbrack("(");
     expression();
     needbrack(")");
     testjump(label);
     }

constant(val)
     int  *val;
     {
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

number(val)
     int  *val;
     {
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

pstr(val)
int  val[];
     {
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

qstr(val)
int  val[];
     {
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

litchar()
/* sort out backslash sequences */
        {
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

/* cc8.c9 R1.1 3/7/85  */

outsnl(s)
       char    *s;
       {
       outstr(s);
       nl();
       }

/*begin a comment line for the assembler*/
comment()
        {
        outbyte('*');
        outbyte(' ');
        }

outup(s)
    char *s;
    {
    while(*s)
        outbyte(toupper(*s++));
    }

/*print all assembler info before any code is generated*/
header()
        {
        if (glbflag)
                {
                comment();
                outsnl(version);
                outstr(" lib libdef\n");
                }
        else
                {
                outstr("/* small-C precompiled module */\n");
                outstr("#asm\n");
                comment();
                outsnl(version);
                }
        }

/*print any assembler stuff needed after all code*/
trailer()
        {
        if (glbflag)
                {
                outstr("end\n");     /* end of uninitialised data segment */
                outstr(" end START\n"); /* xfer address is START */
                }
        else
                outstr("#endasm\n");
        }

/*fetch a static memory cell into the primary register*/
getmem(sym)
       char *sym;
       {
       if ( ( sym[IDENT] != POINTER ) & ( sym[TYPE] == CCHAR ) )
               {
               outstr(" GETB ");
               outsnl(sym + NAME);
               }
       else
               {
               outstr(" GETW ");
               outsnl(sym + NAME);
               }
       }

/*fetch the address of the specified local into the primary register*/
getloc(sym)
       char *sym;
       {
       int *temp;
       outstr(" GETLOC ");
       temp = sym + OFFSET;
       outdec( *temp - sp );
       nl();
       }

/* fetch address of a global into the primary register */
getglb()
        {
        outstr(" GETGLB ");
        }

/*store the primary register into the specified static memory cell*/
putmem(sym)
       char *sym;
       {
       if ( ( sym[IDENT] != POINTER ) & ( sym[TYPE] == CCHAR ) )
               outstr(" PUTB ");
       else
               outstr(" PUTW ");
       outsnl(sym + NAME);
       }

/*store the specified object type in the primary register at the address*/
/*on top of the stack*/
putstk(typeobj)
       char typeobj;
       {
       if ( typeobj == CCHAR )
               outstr(" PUTSB\n");
       else
               outstr(" PUTSW\n");
/* store value at the address pointed to by s then pull address off stack */
       sp = sp + intwidth;
       }

/*fetch the specified object type indirect through the primary register*/
/*into the primary register*/
indirect(typeobj)
       char typeobj;
       {
       if ( typeobj == CCHAR )
               outstr(" GETBI\n");
       else
               outstr(" GETWI\n");
       }

/*swap primary and secondary registers*/
swap()
       {
       outstr(" SWAP\n");
       }

/*print partial instruction to get partial value*/
/*into primary register*/
immed()
       {
       outstr(" IMMED ");
       }

/*push the primary register onto the stack*/
push()
       {
       outstr(" PUSH\n");
       sp = sp - intwidth;
       }

/*swap the primary register and the top of the stack*/
swapstk()
       {
       outstr(" SWAPS\n");
       }

/*call the specified subroutine name*/
call(sname,argcnt)
       char *sname;
       int  argcnt;
       {
       outstr(" CALL ");
       outstr(sname);
       outbyte(',');
       outdec(argcnt);
       nl();
       }

/*return from a subroutine*/
ret()
       {
       outstr(" RET\n");
       }

/*call subroutine at address in stack*/
callstk(argcnt)
       int argcnt;
       {
       outstr(" CALLS ");
       outdec(argcnt);
       nl();
       }

/*jump to a specified internal label number*/
jump(label)
       int  label;
       {
       outstr(" GOTO ");
       printlabel(label);
       nl();
       }

/*test the primary register and jump if false to label*/
testjump(label)
       int  label;
       {
       outstr(" TEST ");
       printlabel(label);
       nl();
       }

codeseg()
/* declare code segment */
       {
       outstr(" CSEG\n");
       }

litseg()
/* declare literal segment */
        {
        if (glbflag)
                {
                outstr(" lib libload\n");  /* load runtime library code */
                outstr("etext LSEG\n"); /* label end of text segment for C */
                }
        else
                outstr(" LSEG\n");
        }

dataseg()
/* declare data segment */
        { /* label end of initialised data segment */
        if (glbflag)
                outstr("edata DSEG\n");
        else
                outstr(" DSEG\n");
        }

extfref(ptr)
       char    *ptr;
       {
       outstr(" EXTF ");
       outstr(ptr);
       outbyte(',');
       outup(ptr);
       nl();
       }

extvref(ptr)
        char *ptr;
        {
        outstr(" EXTV ");
        outsnl(ptr);
        }

pubref(ptr)
       char    *ptr;
       {
       outstr(" PUB ");
       outsnl(ptr);
       }

/*print pseudo op to define a byte*/
defbyte()
       {
       outstr(" fcb ");
       }

/*print pseudo-op to define storage*/
defstorage(n)
       int n;
       {
       outstr(" rmb ");
       outdec( n );
       nl();
       }

/*print pseudo-op to define a word*/
defword()
       {
       outstr(" fdb ");
       }

/* increment stack pointer by n */
incstack( n )
       int n;
       {
       outstr(" INCS ");
       outdec( n );
       nl();
       sp = sp + n;
       }

/*modify stack pointer to new value indicated*/
modstk(newsp)
       int  newsp;
       {
       int     k;
       k = newsp - sp;
       if ( k == 0 )
               return newsp;
       incstack( k );
       return newsp;
       }

scale(width)
        int width;
        {
        outstr(" SCALE ");
        outdec(width);
        nl();
        }

/* add the primary register to the stack and pull the stack ( results in*/
/* primary)*/
cadd()
       {
       outstr(" ADDS\n");
       sp = sp + intwidth;
       }

/*subtract the primary register from the stack and pull the stack (results*/
/*in primary)*/
csub()
       {
       outstr(" SUBS\n");
       sp = sp + intwidth;
       }

/*multiply the value on the stack by d and pull off( results in primary )*/
mult()
       {
       outstr(" MULS\n");
       sp = sp + intwidth;
       }

/*divide the value on the stack by the value in d 
 ( quotient in primary,remainder in secondary ) */
myDiv()
       {
       outstr(" DIVS\n");
       sp = sp + intwidth;
       }

/*compute the remainder ( mod ) of the value on the stack by the value in d
 (remainder in primary,quotient in secondary*/
mod()
       {
       outstr(" MODS\n");
       sp = sp + intwidth;
       }

/*inclusive or the primary reg with the stack and pull the stack */
/*( results in primary )*/
or()
       {
       outstr(" ORS\n");
       sp = sp + intwidth;
       }

/*exclusive or the primary reg with the stack and pull */
/*(results in primary )*/
xor()
       {
       outstr(" EORS\n");
       sp = sp + intwidth;
       }

/*'and' the primary reg with the stack and pull( results in primary )*/
and()
       {
       outstr(" ANDS\n");
       sp = sp + intwidth;
       }

/*arithmetic shift right the value on the stack no of times in d
(results in primary)*/
casr()
       {
       outstr(" ASRS\n");
       sp = sp + intwidth;
       }

/*arithmetic left shift the value on the stack number of times in d
(results in primary)*/
casl()
       {
       outstr(" ASLS\n");
       sp = sp + intwidth;
       }

/*form two's complement of primary register*/
cneg()
       {
       outstr(" NEGD\n");
       }

/*form one's complement of primary register*/
ccom()
       {
       outstr(" COMD\n");
       }

/*increment the primary register by n */
cinc( n )
       int n;
       {
       outstr(" INCD ");
       outdec( n );
       nl();
       }

/*decrement the primary register by n */
cdec( n )
       int n;
       {
       outstr(" DECD ");
       outdec( n );
       nl();
       }

/*following are the conditional operators*/
/*they compare the value on the stack against the primary and put a literal*/
/*1 in the primary if the condition is true,otherwise they clear the primary*/
/*register. They are pure macro's and pull the stack themselves */

/*test for equal*/
eq()
       {
       outstr(" CCEQ\n");
       sp = sp + intwidth;
       }

/*test for not equal*/
ne()
       {
       outstr(" CCNE\n");
       sp = sp + intwidth;
       }

/*test for less than ( signed )*/
lt()
       {
       outstr(" CCLT\n");
       sp = sp + intwidth;
       }

/*test for less than or equal to (signed)*/
le()
       {
       outstr(" CCLE\n");
       sp = sp + intwidth;
       }

/*test for greater than ( signed )*/
gt()
       {
       outstr(" CCGT\n");
       sp = sp + intwidth;
       }

/*test for greater than or equal to ( signed )*/
ge()
       {
       outstr(" CCGE\n");
       sp = sp + intwidth;
       }

/*test for less than ( unsigned )*/
ult()
       {
       outstr(" CCULT\n");
       sp = sp + intwidth;
       }

/*test for less than or equal to ( unsigned )*/
ule()
       {
       outstr(" CCULE\n");
       sp = sp + intwidth;
       }

/*test for greater than ( unsigned)*/
ugt()
       {
       outstr(" CCUGT\n");
       sp = sp + intwidth;
       }

/*test for greater than or equal to ( unsigned )*/
uge()
       {
       outstr(" CCUGE\n");
       sp = sp + intwidth;
       }
/*                   <<<<    end of compiler >>>>                    */
/* cc9.c9 R1.1 24/4/85 */

/*  get output file name  */
openout()
        {
        char *p;
        int i;
        kill();
        if (gargc>1)            /* command line parameters in use */
            {
            i = 1;
            while(i<gargc)
                {
                p = gargv[i];
                if (*p == '-')
                    if (*++p == 'o')
                        {       /* output file indicator found */
                        if (*++p == 0)
                            if (++i<gargc)
                                p = gargv[i];
                            else
                                {
                                perror("output file not specified after -o");
                                exit(1);
                                }
                        if ((output=fopen(p,"w"))==NULL)
                            {
                            fputc(EOL,stderr);
                            fputs("Can't open output file: ",stderr);
                            fputs(p,stderr);
                            exit(1);    /* back to flex to try again */
                            }
                        return; /* file now open */
                        }
                ++i;
                }
            }
       pl("output filename: ");         /*ask*/
       getl(line);                      /*get filename*/
       if ( *line == 0 )                 /*none given...*/
               {
               pl("screen output\n");
               output = stdout;
               return;
               }
       if ( ( output = fopen(line, "w" ) ) == NULL )
               {
               error("can't open file");
               pl(line);
               output = stdout;
               }
       kill();                          /*erase line*/
       }

/* get (next) input file  */
openin()
        {
        char *p;
        input = 0;                      /*none to start with*/
        while ( input == 0 )            /*any above 1 allowed*/
               {
               kill();
               if (eof)
                       break;
                if (gargc>1)    /* take from command line */
                    {
                    while(curr_arg<gargc)
                        {
                        p = gargv[curr_arg];
                        if (*p == '-')
                            {   /* option not wanted */
                            ++curr_arg;
                            if (*++p == 'o')
                                {/* output file indicator found */
                                if (*++p == 0) /* -o file */
                                    ++curr_arg;
                                }
                            continue;
                            }
                        if ((input=fopen(p,"r"))==NULL)
                            {
                            fputc(EOL,stderr);
                            fputs("Can't open input file: ",stderr);
                            fputs(p,stderr);
                            ++curr_arg;
                            continue;
                            }
                        strncpy(infnam,p,FNAMSIZ); /* remember name */
                        pl(infnam);   /* tell user current file */
                        if (output==stdout) nl();
                        ++curr_arg;     /* move on for next file */
                        infline = 0;    /* zero line number */
                        return; /* file now open */
                        } /* above temp fix for codegen error */
                    eof = 1;
                    break;      /* no more input files */
                    }
                else
                    {   /* get files interactively */
                    pl("input filename: ");
                    getl(line);             /*get name*/
                    if ( *line == 0 )
                        {
                        eof = 1;        /*none given*/
                        break;
                        }
                    if (( input = fopen( line,"r")) == NULL )
                        pl("can't open file");
                    else
                        {
                        strncpy(infnam,line,FNAMSIZ);
                        infline = 0;
                        }
                    }
               }
       kill();                         /*erase line*/
       }

/* open an include file */
doinclude()
       {
       char buffer[LINESIZE];          /*buffer for include file name*/
       char *pointer;                   /*pointer to buffer*/
       blanks();                       /*skip over to name*/
       if ( input2 != 0 )
               {
               error("can't nest include files");
               }
       else
               {
               pointer = buffer;            /*reset buffer pointer*/
               while ( gch() != '"' )  /*look for first ocurrence of '"'*/
                       {
                       if ( ch == 0 )                /*eol no file*/
                               {
                               error("bad include file specification");
                               return;                 /*give up*/
                               }
                       }
               while (( *pointer++ = gch() ) != '"' )
                       /*strip off file name*/
                       {
                       if ( ch == 0 )                /*eol*/
                               {
                               error("bad include file specification");
                               return;                 /*give up*/
                               }
                       }
               *--pointer = 0;                 /*append a null*/
               if ( ( input2 = fopen(buffer , "r" )) == NULL )
                       {
                       error("can't open include file");
                       }
               else
                       {
                       strncpy(incfnam,buffer,FNAMSIZ);
                       incfline = 0;
                       }
               }
       kill();                        /*clear rest of line so that */
                                       /*next read will come from new */
                                       /*file (if open)*/
       }

/* close the output file  */
closeout()
       {
       if ( output)                     /*if there is an output file*/
               fclose( output );        /*close it*/
       output = stdout;
       }

closein(unit)
/**this function closes the input file identified by "unit"
unit is a copy of input or input2 */
       FILE *unit;               /*indicates which file is to be closed*/
       {
       if ( unit )       /*if input is open*/
               {
               fclose( unit );
               if ( unit == input )
                       input = 0;      /*mark as closed*/
               else
                       input2 = 0;
               }
       }

#if 0
getl(line)
/*copy a line from stdin to line*/
/* note that line is 132 chars long and that FLEX line buf is 128 so ok */
       char    *line;
       {
       gets(line);
       }
#else
void
getl(char *buf) {
    // Really should put the null at the end of the buf
    if (fgets(buf, sizeof(buf), stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    }
}
#endif
