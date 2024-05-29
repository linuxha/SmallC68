#ifndef SMALLC_H
#define SMALLC_H
/************************************************/
/*                                              */
/*              small-c compiler                */
/*                                              */
/*                by Ron Cain                   */
/*                                              */
/************************************************/

#define BANNER  "** Ron Cain's Small-C V1.1.0 **"

#define VERSION " FLEX Version 2.1, 13 Aug 1982\n* Linux Version 0.0.1, Jan31 2023\n* Motorola 68xx"

#define AUTHOR "       By S. Stepanoff & Neil Cherry"

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
#define SQUOTE	0x27            // 'single'
#define DQUOTE	0x22            // "double"

/*      UNIX definitions (if not stand-alone)   */

/* #include <stdio.h>  */
/* #define EOL 10       */

/*      Define the symbol table parameters      */

#define SYMSIZ   14             /* Tied to symbol table entry and name size (below)  */
#define SYMTBSZ  5040
#define NUMGLBS  300
#define STARTGLB symtab
#define ENDGLB   STARTGLB+NUMGLBS*SYMSIZ
#define STARTLOC ENDGLB+SYMSIZ
#define ENDLOC   symtab+SYMTBSZ-SYMSIZ

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
#define ARRAY    2
#define POINTER  3
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

#define LINESIZE 160
#define LINEMAX LINESIZE-1
#define MPMAX   LINEMAX

/*      Define the macro (define) pool          */

#define MACQSIZE 1000
#define MACMAX   MACQSIZE-1

/*      Define statement types (tokens)         */

#define STIF     1
#define STWHILE  2
#define STRETURN 3
#define STBREAK  4
#define STCONT   5
#define STASM    6
#define STEXP    7

#ifdef SKIP
extern void parse();
extern void dumplits();
extern void dumpglbs();
extern void errorsummary();
extern void ask();
extern void openout();
extern void openin();
extern void doinclude();
extern void endinclude();
extern void closeout();
extern int  needsub();
extern void newfunc();
extern void newfile();
extern int  statement();
extern void ns();
extern void compound();
extern void doif();
extern void dowhile();
extern void doreturn();
extern void dobreak();
extern void docont();
extern void doasm();
extern void callfunction(char *ptr);
extern void junk();
extern int  endst();
extern void illname();
extern void multidef(char *sname);
extern void needbrack(char *str);
extern void needlval();
extern char *findglb(char *sname);
extern int  findloc(char *sname);

extern char *addglb(char *sname, char id, char typ, int value);
extern char *addloc(char *sname, char id, char typ, int value);
extern int  isSymname(char *sname);

extern int  getlabel();
extern void printlabel(int label);
extern void pl(char *str);
extern int  addwhile(int ptr[]);
extern void delwhile();
extern int  readwhile();
extern void kill();
extern int  inbyte();
extern int  inchar();
extern int  myInline();
extern int  keepch(char c);
extern int  preprocess();
extern int  addmac();
extern int  putmac(char c);
extern int  findmac(char *sname);
extern int  outbyte(char c);
extern int  outstr(char *ptr);
extern int  nl();
extern int  tab();
extern int  col();
extern int  bell();
extern int  error(char ptr[]);
extern int  ol(char ptr[]);
extern int  ot(char ptr[]);
extern int  match(char *lit);
extern int  amatch(char *lit, int len);
extern void blanks();
extern void outdec(int numb);
extern void expression();

extern int heir1(int lval[]);
extern int heir2(int lval[]);
extern int heir3(int lval[]);
extern int heir4(int lval[]);
extern int heir5(int lval[]);
extern int heir6(int lval[]);
extern int heir7(int lval[]);
extern int heir8(int lval[]);
extern int heir9(int lval[]);
extern int heir10(int lval[]);
extern int heir11(int *lval);

extern int  primary(int *lval);
extern void store(int *lval);
extern void rvalue(int *lval);
extern void test(int label);
extern int  constant(int val[]);
extern int  number(int val[]);
extern int  pstr(int val[]);
extern int  qstr(int val[]);
extern void comment();
extern void header();
extern void trailer();
extern void getmem(char *sym);
extern void getloc(char *sym);
extern void putmem(char *sym);
extern void putstk(char typeobj);
extern void indirect(char typeobj);
extern void immed();
extern void zpush();
extern void zpop();
extern void swapstk();
extern void zcall(char *sname);
extern void zret();
extern void callstk();
extern void jump(int label);
extern void testjump(int label);
extern void pseudoins(int k);
extern void defbyte();
extern void defstorage();
extern void defword();
extern int  modstk(int newsp);
extern void doublereg();
extern void zadd();
extern void zsub();
extern void mult();
extern void myDiv();
extern void zmod();
extern void zor();
extern void zxor();
extern void zand();
extern void asr();
extern void asl();
extern void neg();
extern void com();
extern void inc();
extern void dec();

extern int  alpha(char c);
extern int  numeric(char c);
extern int  an(char c);
extern int  ch();
extern int  nch();
extern int  gch();
extern void toconsole();
extern void tofile();
extern int  streq(char str1[], char str2[]);
extern int  astreq(char str1[], char str2[], int len;);

extern void myAbort();
extern int  myFclose(FILE *fp);
extern char *myGets(char *);

extern void declglb(int typ);
extern void declloc(int typ);
extern void getarg(int t);

extern void blanks();
extern void zeq();
extern void zne();
extern void zlt();
extern void zle();
extern void zgt();
extern void zge();
extern void ult();
extern void ule();
extern void ugt();
extern void uge();

extern void doError(int i);
extern void options(int argc, char **argv);

extern FILE *output;    /* no open units */
extern FILE *input;     /* no open units */
extern FILE *input2;    /* no open units */
extern FILE *saveout;   /* no diverted output */

extern char    symtab[SYMTBSZ];        /* symbol table */
extern char    *glbptr;                /* ptrs to next entries */
extern int     locptr;

extern char    wq[WQTABSZ];            /* while queue */
extern char    *wqidx;                 /* ptr to next entry */

extern char    litq[LITABSZ];          /* literal pool */
extern char    litidx;                 /* ptr to next entry */

extern char    macq[MACQSIZE];         /* macro string buffer */
extern char    macidx;                 /* and its index */

extern char    line[LINESIZE];         /* parsing buffer */
extern char    mline[LINESIZE];        /* temp macro buffer */
extern char    lidx,midx;              /* ptrs into each */

/*      Misc storage    */

extern int     nxtlab,         /* next avail label # */
        litlab,         /* label # assigned to literal pool */
        Zsp,            /* compiler relative stk ptr */
        argstk,         /* function arg sp */
        ncmp,           /* # open compound statements */
        errcnt,         /* # errors in compilation */
        errstop,        /* stop on error */
        eof,            /* set non-zero on final input eof */
        glbflag,        /* non-zero if internal globals */
        ctext,          /* non-zero to intermix c-source */
        cmode,          /* non-zero while parsing c-code */
                        /* zero when passing assembly code */
        lastst,         /* last executed statement type */
        mainflg,        /* output is to be first asm file */
        fnstart,        /* line# of start of current fn. */
        lineno,         /* line# in current file */
        infunc,         /* "inside function" flag */
        savestart,      /* copy of fnstart "    " */
        saveline,       /* copy of lineno  "    " */
        saveinfn;       /* copy of infunc  "    " */

extern char   *currfn,         /* ptr to symtab entry for current fn. */
       *savecurr;       /* copy of currfn for #include */
extern char    quote[2];       /* literal string for '"' */
extern char    *cptr;          /* work ptr to any char buffer */
extern int     *iptr;          /* work ptr to any int buffer */

#else
extern void parse();
extern void dumplits();
extern void dumpglbs();
extern void errorsummary();
extern void ask();
extern void openout();
extern void openin();
extern void doinclude();
extern void endinclude();
extern void closeout();
extern long needsub();
extern void newfunc();
extern void newfile();
extern long statement();
extern void ns();
extern void compound();
extern void doif();
extern void dowhile();
extern void doreturn();
extern void dobreak();
extern void docont();
extern void doasm();
extern void callfunction(char *ptr);
extern void junk();
extern long endst();
extern void illname();
extern void multidef(char *sname);
extern void needbrack(char *str);
extern void needlval();
extern char *findglb(char *sname);
extern long findloc(char *sname);

extern char *addglb(char *sname, char id, char typ, long value);
extern char *addloc(char *sname, char id, char typ, long value);
extern long isSymname(char *sname);

extern long getlabel();
extern void printlabel(long label);
extern void pl(char *str);
extern void addwhile(long ptr[]);
extern void delwhile();
extern long *readwhile();
extern void kill();
extern long inbyte();
extern long inchar();
extern void myInline();
extern long keepch(char c);
extern void preprocess();
extern void addmac();
extern char putmac(char c);
extern long findmac(char *sname);
extern long outbyte(char c);
extern void outstr(char *ptr);
extern void nl();
extern void tab();
extern void col();
extern void bell();
extern void error(char ptr[]);
extern void ol(char ptr[]);
extern void ot(char ptr[]);
extern long match(char *lit);
extern long amatch(char *lit, long len);
extern void blanks();
extern void outdec(long numb);
extern void expression();

extern long heir1(long lval[]);
extern long heir2(long lval[]);
extern long heir3(long lval[]);
extern long heir4(long lval[]);
extern long heir5(long lval[]);
extern long heir6(long lval[]);
extern long heir7(long lval[]);
extern long heir8(long lval[]);
extern long heir9(long lval[]);
extern long heir10(long lval[]);
extern long heir11(long *lval);

extern long primary(long *lval);
extern void store(long *lval);
extern void rvalue(long *lval);
extern void test(long label);
extern long constant(long val[]);
extern long number(long val[]);
extern long pstr(long val[]);
extern long qstr(long val[]);
extern void comment();
extern void header();
extern void trailer();
extern void getmem(char *sym);
extern void getloc(char *sym);
extern void putmem(char *sym);
extern void putstk(char typeobj);
extern void indirect(char typeobj);
extern void immed();
extern void zpush();
extern void zpop();
extern void swapstk();
extern void zcall(char *sname);
extern void zret();
extern void callstk();
extern void jump(long label);
extern void testjump(long label);
extern void pseudoins(long k);
extern void defbyte();
extern void defstorage();
extern void defword();
extern long modstk(long newsp);
extern void doublereg();
extern void zadd();
extern void zsub();
extern void mult();
extern void myDiv();
extern void zmod();
extern void zor();
extern void zxor();
extern void zand();
extern void asr();
extern void asl();
extern void neg();
extern void com();
extern void inc();
extern void dec();

extern long alpha(char c);
extern long numeric(char c);
extern long an(char c);
extern long ch();
extern long nch();
extern long gch();
extern void toconsole();
extern void tofile();
extern long streq(char str1[], char str2[]);
extern long astreq(char str1[], char str2[], long len;);

extern void myAbort();
extern long myFclose(FILE *fp);
extern char *myGets(char *);

extern void declglb(long typ);
extern void declloc(long typ);
extern void getarg(long t);

extern void blanks();
extern void zeq();
extern void zne();
extern void zlt();
extern void zle();
extern void zgt();
extern void zge();
extern void ult();
extern void ule();
extern void ugt();
extern void uge();

extern void doError(long i);
extern void options(int argc, char **argv);

extern FILE *output;    /* no open units */
extern FILE *input;     /* no open units */
extern FILE *input2;    /* no open units */
extern FILE *saveout;   /* no diverted output */

extern char    symtab[SYMTBSZ];        /* symbol table */
extern char    *glbptr;                /* ptrs to next entries */
extern long    locptr;

extern char    wq[WQTABSZ];            /* while queue */
extern char    *wqidx;                 /* ptr to next entry */

extern char    litq[LITABSZ];          /* literal pool */
extern char    litidx;                 /* ptr to next entry */

extern char    macq[MACQSIZE];         /* macro string buffer */
extern char    macidx;                 /* and its index */

extern char    line[LINESIZE];         /* parsing buffer */
extern char    mline[LINESIZE];        /* temp macro buffer */
extern char    lidx,midx;              /* ptrs into each */

/*      Misc storage    */

extern long    nxtlab,         /* next avail label # */
        litlab,         /* label # assigned to literal pool */
        Zsp,            /* compiler relative stk ptr */
        argstk,         /* function arg sp */
        ncmp,           /* # open compound statements */
        errcnt,         /* # errors in compilation */
        errstop,        /* stop on error */
        eof,            /* set non-zero on final input eof */
        glbflag,        /* non-zero if internal globals */
        ctext,          /* non-zero to intermix c-source */
        cmode,          /* non-zero while parsing c-code */
                        /* zero when passing assembly code */
        lastst,         /* last executed statement type */
        mainflg,        /* output is to be first asm file */
        fnstart,        /* line# of start of current fn. */
        lineno,         /* line# in current file */
        infunc,         /* "inside function" flag */
        savestart,      /* copy of fnstart "    " */
        saveline,       /* copy of lineno  "    " */
        saveinfn;       /* copy of infunc  "    " */

extern char   *currfn,         /* ptr to symtab entry for current fn. */
       *savecurr;       /* copy of currfn for #include */
extern char    quote[2];       /* literal string for '"' */
extern char    *cptr;          /* work ptr to any char buffer */
extern long    *iptr;          /* work ptr to any long buffer */
#endif

extern int errCnt;

#ifndef FDEBUG
#warning FDEBUG not defined
#define fdebugf(b,c) { }
#else
#warning FDEBUG IS defined
#define fdebugf(b,c) fprintf(stderr, b, c)
#endif

#endif
