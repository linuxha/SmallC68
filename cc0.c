/************************************************/
/*                                              */
/*              small-c compiler                */
/*                                              */
/*                by Ron Cain                   */
/*                                              */
/************************************************/

#define BANNER  "** Ron Cain's Small-C V1.1 **"

#define VERSION "FLEX Version 2.1, 13 Aug 1982"

#define AUTHOR "       By S. Stepanoff"

/*      Define system dependent parameters      */

/*      Stand-alone definitions                 */

#define NULL 0
#define EOL 13

/*      UNIX definitions (if not stand-alone)   */

/* #include <stdio.h>  */
/* #define EOL 10       */

/*      Define the symbol table parameters      */

#define SYMSIZ  14
#define SYMTBSZ 5040
#define NUMGLBS 300
#define STARTGLB symtab
#define ENDGLB  STARTGLB+NUMGLBS*SYMSIZ
#define STARTLOC ENDGLB+SYMSIZ
#define ENDLOC  symtab+SYMTBSZ-SYMSIZ

/*      Define symbol table entry format        */

#define NAME    0
#define IDENT   9
#define TYPE    10
#define STORAGE 11
#define OFFSET  12

/*      System wide name size (for symbols)     */

#define NAMESIZE 9
#define NAMEMAX  8
/*      Define possible entries for "ident"     */

#define VARIABLE 1
#define ARRAY   2
#define POINTER 3
#define FUNCTION 4

/*      Define possible entries for "type"      */

#define CCHAR   1
#define CINT    2

/*      Define possible entries for "storage"   */

#define STATIK  1
#define STKLOC  2

/*      Define the "while" statement queue      */

#define WQTABSZ 100
#define WQSIZ   4
#define WQMAX   wq+WQTABSZ-WQSIZ

/*      Define entry offsets in while queue     */

#define WQSYM   0
#define WQSP    1
#define WQLOOP  2
#define WQLAB   3

/*      Define the literal pool                 */

#define LITABSZ 2000
#define LITMAX  LITABSZ-1

/*      Define the input line                   */

#define LINESIZE 80
#define LINEMAX LINESIZE-1
#define MPMAX   LINEMAX

/*      Define the macro (define) pool          */

#define MACQSIZE 1000
#define MACMAX  MACQSIZE-1

/*      Define statement types (tokens)         */

#define STIF    1
#define STWHILE 2
#define STRETURN 3
#define STBREAK 4
#define STCONT  5
#define STASM   6
#define STEXP   7


/*      Now reserve some storage words          */

char    symtab[SYMTBSZ];        /* symbol table */
char    *glbptr,*locptr;                /* ptrs to next entries */

int     wq[WQTABSZ];            /* while queue */
int     *wqptr;                 /* ptr to next entry */

char    litq[LITABSZ];          /* literal pool */
int     litptr;                 /* ptr to next entry */

char    macq[MACQSIZE];         /* macro string buffer */
int     macptr;                 /* and its index */

char    line[LINESIZE];         /* parsing buffer */
char    mline[LINESIZE];        /* temp macro buffer */
int     lptr,mptr;              /* ptrs into each */

/*      Misc storage    */

int     nxtlab,         /* next avail label # */
        litlab,         /* label # assigned to literal pool */
        Zsp,            /* compiler relative stk ptr */
        argstk,         /* function arg sp */
        ncmp,           /* # open compound statements */
        errcnt,         /* # errors in compilation */
        errstop,        /* stop on error */
        eof,            /* set non-zero on final input eof */
        input,          /* iob # for input file */
        output,         /* iob # for output file (if any) */
        input2,         /* iob # for "include" file */
        glbflag,        /* non-zero if internal globals */
        ctext,          /* non-zero to intermix c-source */
        cmode,          /* non-zero while parsing c-code */
                        /* zero when passing assembly code */
        lastst,         /* last executed statement type */
        mainflg,        /* output is to be first asm file */
        saveout,        /* holds output ptr when diverted to console */
        fnstart,        /* line# of start of current fn. */
        lineno,         /* line# in current file */
        infunc,         /* "inside function" flag */
        savestart,      /* copy of fnstart "    " */
        saveline,       /* copy of lineno  "    " */
        saveinfn;       /* copy of infunc  "    " */

char   *currfn,         /* ptr to symtab entry for current fn. */
       *savecurr;       /* copy of currfn for #include */
char    quote[2];       /* literal string for '"' */
char    *cptr;          /* work ptr to any char buffer */
int     *iptr;          /* work ptr to any int buffer */
