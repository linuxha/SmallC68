/* cc1.c9 R1.1.1 4/7/85 */
/* scc.c  v2.1.0 2023/02/06 <ncherry@linuxha.com> */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"
#include "scc.h"

/*                   >>> start cc1 <<<                               */
/*                                                                   */
int
main(int argc,char **argv) {
    version = VERSION;              /*point at version string*/
    glbptr = STARTGLB;              /*clear global symbols*/
    locptr = STARTLOC;              /*clear local symbols*/
    wqptr  = wq;                    /*clear while queue*/
    mptr   = -1;                    /* flag not in preprocess */

    macptr =                        /*clear macro pool*/
        litptr =                        /*clear literal pool*/
        sp =                            /*stack ptr (relative)*/
        errcnt =                        /*no errors*/
        eof =                           /*not eof yet*/
        ncmp =                          /*no open compound states*/
        ch = nch =                      /*no input chars*/
        dump =                          /* dont dump stats */
        lastst = 0;                     /*no last statement yet*/

    output = stdout;                /*default to stdout for ask()*/
        input =                         /*no input file*/
        input2 =                        /*no include file*/

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
    exit(0);                         /*then exit to the system*/
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
void
declloc( int  typ ) {
    int     k;
    int     j;
    char    sname[NAMESIZE];

    while(1) {
        while(1) {
            if ( endst() )                       /* do line */
                return;

            if ( match ("*") )                   /* pointer? */
                j = POINTER;                     /* yes */
            else
                j = VARIABLE;                    /* no */

            if (symname(sname) == 0 )            /* name ok? */
                illname();                       /* no */

            if ( findloc(sname) )                /* already there */
                multidef(sname);

            if ( match ("[") ) {                 /* array */
                k = needsub();                   /* get size */

                if ( k ) {
                    j = ARRAY;                   /* 10 = array */
                    if ( typ == CINT )
                        k = k * intwidth;
                } else {
                    j = POINTER;                 /* 0 = pointer */
                    k = intwidth;
                }
            } else if (( typ == CCHAR) & ( j != POINTER )) {
                k = 1;
            } else {
                k = intwidth;
            }

            /*change machine stack*/
            sp = modstk(sp - k );
            addloc (sname , j , typ , sp );
            break;
        }

        if (match (",") == 0 )
            return;
    }
}
