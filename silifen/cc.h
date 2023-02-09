#ifndef CC_H
#define CC_H

/************************************************/
/*                                              */
/*              small-c compiler                */
/*                                              */
/*                by Ron Cain                   */
/*                                              */
/************************************************/

/*      Define system dependent parameters      */

extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stderr;

/*      Stand-alone definitions                 */

#ifndef NULL
#define NULL	0
#endif
#ifdef __linux
#define EOL 	'\n'            // \n = ^J 0x0A
#else
#define EOL 	13              // \r = ^M 0x0D
#endif

#define TAB	0x09
#define CLS	0x0C            // ^L
#define SQUOTE	0x27
#define DQUOTE	0x22

/*      UNIX definitions (if not stand-alone)   */

/* Silicon Fen Software Small-C compiler R1.1 for macro assembler */

/* version string for use in ask() and header() */
//#define VERSION  "Silicon Fen Software: Small-C Compiler R1.1.0 4/7/85"
#define VERSION  "Silicon Fen Software: Small-C Cross Compiler for Linux and the 68XX v2.1.0 2023/02/06"

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

/*
struct symbol{
    char name[MAXSIZE];
    int  ident;
    int  type;
    int  storage;
    long offset;
};
 */
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

extern char  symtab[SYMTBSZ];          /*symbol table*/
extern char  *glbptr;
extern char  *locptr;                  /*pointers to next entry*/
extern long  wq[WQTABSZ];              /*while queue*/
extern long  *wqptr;                   /*pointer to next entry*/
extern char  litq[LITABSZ];            /*literal pool*/
extern long  litptr;                   /*pointer to next entry*/
extern char  macq[MACQSIZE];           /*macro string buffer*/
extern long  macptr;                   /*and its index*/
extern char  line[LINESIZE];           /*parsing buffer*/
extern char  mline[LINESIZE];          /*temp macro buffer*/
extern long  iflevel;                  /* #if.. nest level */
extern long  skiplevel;                /* level at which #if.. skipping started */
extern long  lptr;
extern long  mptr;                     /*pointers into each*/

/*   misc storage    */

extern char  *version;                 /*pointer to version string */
extern int   intwidth;                 /*integer width in bytes 2 or 4*/
extern int   nxtlab;                   /*next available label*/
extern int   litlab;                   /*label # assigned by literal pool*/
extern int   sp;                       /*compiler relative stack pointer*/
extern int   argstk;                   /*function arg sp*/
extern int   ncmp;                     /*# open compound statements*/
extern int   errcnt;                   /*# errors in compilation*/
extern int   eof;                      /*set non zero on final input eof*/
extern int   glbflag;                  /*non-zero if internal globals*/
extern int   ctext;                    /*non-zero to intermix c-source*/
extern int   cmode;
/*non-zero while parsing  c-code zero when parsing assembly code*/
extern int   lastst;                   /*last executed statement type*/
extern char  quote[2];                 /*literal string for '"' */
extern char  *cptr;                    /*work pointer to any char buffer*/
extern long  *iptr;                    /*work pointer to any int buffer*/
extern int   ch,nch;                   /*current and next characters as ints*/
extern char  infnam[FNAMSIZ];          /*buffer for current filename*/
extern char  incfnam[FNAMSIZ];         /*buffer for include filename*/
extern int   infline;                  /*line number for input file*/
extern int   incfline;                 /*line number for include file*/
extern int   curr_arg;                 /*current place in argument list*/
extern int   dump;                     /*dump statistics flag*/

extern int   gargc;                    /*global copy of arg count*/
extern char  **gargv;                  /*pointer to argument array*/

extern FILE  *output;                  /*output fcb pointer*/
extern FILE  *input;                   /*input file fcb pointer*/
extern FILE  *input2;                  /*include file fcb pointer */

extern void ask();
extern void cl_asl();

#ifndef FDEBUG
#warning FDEBUG not defined
#define fdebugf(b,c) { }
#else
#warning FDEBUG IS defined
#define fdebugf(b,c) fprintf(stderr, b, c)
#endif

extern void declloc( int  typ );
extern char *itob(char *s, int i, int base);
extern char *btoi(char *s, int n, int *pinum, int base);
extern void ask();
extern void cl_ask();
extern void expression();

extern long heir1(long *lval);
extern long heir2(long *lval);
extern long heir3(long *lval);
extern long heir4(long *lval);
extern long heir5(long *lval);
extern long heir6(long *lval);
extern long heir7(long *lval);
extern long heir8(long *lval);
extern long heir9(long *lval);
extern long heir10(long *lval);
extern long heir11(long *lval);

extern int  needsub();
extern void newfunc();
extern void getarg(int t);
extern int  statement();
extern void ns();
extern void compound();
extern void doif();
extern void dowhile();
extern void dofor();
extern void dodo();
extern void doreturn();
extern void docont();
extern void doasm();
extern void dobreak();

extern void callfunction(char *ptr);
extern void junk();
extern int  endst();
extern void illname();
extern void multidef(char *sname);
extern void needbrack(char *str);
extern void needlval();
extern char *findglb(char *sname);
extern char *addglb(char *sname, char id, char typ, int value);
extern char *addloc(char *sname, char id, char typ, int value);
extern int  symname(char *sname);
extern int  getlabel();
extern void printlabel(int label);
extern int  alpha(char c);
extern int  numeric(char c);
extern int  an(char c);
extern void pl(char *str);
extern void addwhile(int ptr[]);
extern void delwhile();
extern int  readwhile();
extern int  gch();
extern void setch();
extern void kill();
extern int  inbyte();
extern int  inchar();
extern void myInline();

#endif  /* CC_H */

