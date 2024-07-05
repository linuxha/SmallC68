/************************************************/
/*                                              */
/*              small-c compiler                */
/*                                              */
/*                by Ron Cain                   */
/*                                              */
/************************************************/

// This is the Linux Cross-compiler version of the Flex version of Ron Cain's
// Small C compiler V1.1
// This is being converted for use with a modern Linux C compiler so it won't
// compile itself anymore

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "smallc.h"

struct symbol {
    char name[NAMEMAX];         // 27 bytes 0-26
    char ident;                 // 01 bytes 27 (Var/Arr/Ptr/Func)
    char type;                  // 01 bytes 28 (Char/Int)
    char storage;               // 01 bytes 29 (STATIK/STKLOC)
    int  offset;                // 02 Offset 30-31 (value)
    /*
    cptr[OFFSET]   = value>>8;
    cptr[OFFSET+1] = value&255;
    */
};

/*      Now reserve some storage words          */

#ifdef SKIP
char    symtab[SYMTBSZ];        /* symbol table */
char    *glbptr;                /* ptrs to next entries */
int     locidx;

char    wq[WQTABSZ];            /* while queue */
char    *wqidx;                 /* ptr to next entry */

char    litq[LITABSZ];          /* literal pool */
char    litidx;                 /* ptr to next entry */

char    macq[MACQSIZE];         /* macro string buffer */
char    macidx;                 /* and its index */

char    line[LINESIZE];         /* parsing buffer */
char    mline[LINESIZE];        /* temp macro buffer */
char    lidx, midx;             /* ptrs into each */

FILE *output;            /* no open units */
FILE *input;             /* no open units */
FILE *input2;            /* no open units */
FILE *saveout;           /* no diverted output */

/*      Misc storage    */

int     nxtlab,         /* next avail label # */
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

char   *currfn,         /* ptr to symtab entry for current fn. */
       *savecurr;       /* copy of currfn for #include */
char    quote[2];       /* literal string for '"' */
char    *cptr;          /* work ptr to any char buffer */
int     *iptr;          /* work ptr to any int buffer */

#else // -----------------------------------------------------------------------

char    symtab[SYMTBSZ];        /* symbol table */
char    *glbptr;                /* ptrs to next entries */
long    locidx;

char    wq[WQTABSZ];            /* while queue */
char    *wqidx;                 /* ptr to next entry */

char    litq[LITABSZ];          /* literal pool */
char    litidx;                 /* ptr to next entry */

char    macq[MACQSIZE];         /* macro string buffer */
char    macidx;                 /* and its index */

char    line[LINESIZE];         /* parsing buffer */
char    mline[LINESIZE];        /* temp macro buffer */
char    lidx, midx;             /* ptrs into each */

FILE *output;            /* no open units */
FILE *input;             /* no open units */
FILE *input2;            /* no open units */
FILE *saveout;           /* no diverted output */

/*      Misc storage    */

long    nxtlab,         /* next avail label # */
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

char   *currfn,         /* ptr to symtab entry for current fn. */
       *savecurr;       /* copy of currfn for #include */
char    quote[2];       /* literal string for '"' */
char    *cptr;          /* work ptr to any char buffer */
long    *iptr;          /* work ptr to any long buffer */
#endif

int errCnt = 0;

/*      >>>>> start cc1 <<<<<<          */

/*                                      */
/*      Compiler begins execution here  */
/*                                      */
int
main(int argc, char *argv[]) {
    glbptr = STARTGLB;      /* clear global symbols, needs to be a symbol ptr */
    locidx = STARTLOC;      /* clear local symbols */
    wqidx  = wq;            /* clear while queue */

    output  = stdout;       /* no open units */
    input   = 0;            /* no open units */
    input2  = 0;            /* no open units */
    saveout = 0;            /* no diverted output */
    
    macidx=                 /* clear the macro pool */
        nxtlab   =
        litidx   =          /* clear literal pool */
        Zsp      =          /* stack ptr (relative) */
        errcnt   =          /* no errors */
        errstop  =          /* keep going after an error */
        eof      =          /* not eof yet */
        ncmp     =          /* no open compound states */
        lastst   =          /* no last statement yet */
        mainflg  =          /* not first file to asm */
        fnstart  =          /* current "function" started at line 0 */
        lineno   =          /* no lines read from file */
        infunc   =          /* not in function now */
        quote[1] =
        0;                  /*  ...all set to zero.... */
    quote[0]='"';           /* fake a quote literal */
    currfn=NULL;            /* no function yet */
    cmode=1;                /* enable preprocessing */
    /*                              */
    /*      compiler body           */
    /*                              */
    errstop = 0;

#ifdef __linux
    options(argc, argv);
#else
    ask();                  /* get user options */
    openout();              /* get an output file */
    openin();               /* and initial input file */
    header();               /* intro code */
#endif
    header();               /* intro code */

    /*
      @FIXME: We're not returning from parse but returning 0 ???
    */
    parse();                /* process ALL input */

    printf(";* ###\n;* ### Dump & trailer\n;* ###\n");

    dumplits();             /* then dump literal pool */
    dumpglbs();             /* and all static memory */
    trailer();              /* follow-up code */
    closeout();             /* close the output (if any) */
    errorsummary();         /* summarize errors (on console!) */

    printf(";* ###\n;* ### Done\n;* ###\n");

    return(errCnt);                 /* then exit to system */
}

/*                                      */
/*      Abort compilation               */
/*                                      */
void
myAbort() {
    pl("Compilation aborted.");  nl();

    if(input2) {
        endinclude();
    }

    if(input) {
        myFclose(input);
    }

    closeout();
    toconsole();

    pl("Compilation aborted.");  nl();

    exit(1); /* end abort */
}

/*                                          */
/*      Process all input text              */
/*                                          */
/* char                                     */
/* int                                      */
/* #asm/#endasm                             */
/* #include                                 */
/* #define                                  */
/*                                          */
/* At this level, only static declarations, */
/*      defines, includes, and function     */
/*      definitions are legal...            */
void
parse() {
    while (eof == 0) {          /* do until no more input */
        if(amatch("char",4)) {
            declglb(CCHAR);
            ns();
        } else if(amatch("int",3)) {
            declglb(CINT);
            ns();
        } else if(match("#asm")) {
            doasm();
        } else if(match("#include")) {
            doinclude();
        } else if(match("#define")) {
            addmac();
        } else if(match("#error")) {
            doError(1);
        } else if(match("#warning")) {
            doError(0);
        } else {
            newfunc();
        }

        blanks();       /* force eof if pending */
        if(input == 0) {
            break;
        }
    }

    outstr(";* ###\n;* ### Exit parse()\n;* ###\n");

}

/*                                      */
/*      Dump the literal pool           */
/*                                      */
void
dumplits() {
    int j,k;

    if (litidx==0) return;      /* if nothing there, exit...*/
    printlabel(litlab);col();   /* print literal label */
    k=0;                        /* init an index... */
    while (k<litidx) {          /*      to loop with */
        defbyte();              /* pseudo-op to define byte */
        j=10;                   /* max bytes per line */
        while(j--) {
            outdec((litq[k++]&127));
            if ((j==0) || (k>=litidx)) {
                nl();           /* need <cr> */
                break;
            }
            outbyte(',');       /* separate bytes */
        }
    }
}

/*                                      */
/*      Dump all static variables       */
/*                                      */
void
dumpglbs() {
    int j;

    if(glbflag==0)return;   /* don't if user said no */
    cptr=STARTGLB;
    while(cptr<glbptr) {
        if(cptr[IDENT]!=FUNCTION) {
            /* do if anything but function */
            outstr(cptr);col();
            /* output name as label... */
            defstorage();   /* define storage */
            j=(((cptr[OFFSET]&255)<<8) + (cptr[OFFSET+1]&255)); /* calc # bytes */
            if((cptr[TYPE]==CINT) || (cptr[IDENT]==POINTER)) {
                j=j+j;
            }
            outdec(j);      /* need that many */
            nl();
        }
        cptr=cptr+SYMSIZ;
    }
}

/*                                      */
/*      Report errors for user          */
/*                                      */
void
errorsummary() {
    /* see if anything left hanging... */
    if (ncmp) error("missing closing bracket");
    /* open compound statement ... */
    nl();
    outstr("There were ");
    outdec(errcnt); /* total # errors */
    outstr(" errors in compilation.");
    nl(); nl();
    pl("Table Usage:"); nl();
    outstr("symtab: "); outdec(glbptr-STARTGLB); nl();
    outstr("litq: "); outdec(litidx); nl();
    outstr("macq: "); outdec(macidx); nl();
}

/*                                      */
/*      Reset line count, etc.          */
/*                                      */
void
newfile() {
    lineno  = 0;    /* no lines read */
    fnstart = 0;    /* no fn. start yet. */
    currfn  = NULL; /* because no fn. yet */
    infunc  = 0;    /* therefore not in fn. */
}

/*                                      */
/*      Open an include file            */
/*                                      */
void
doinclude() {
    blanks();                   /* skip over to name */
    toconsole();
    outstr(";* #include ");
    outstr((char *) line+lidx);
    nl();

    tofile();

    /*
    ** Cleanup on aisle 2!
    ** @FIXME: Cleanup line+lidx (get rid of "" <> ;)
    */
    if(input2) {
        error("Cannot nest include files");
    } else if((input2 = fopen((char *) line+lidx,"r")) == NULL) {
        input2 = 0;
        error("Open failure on include file");
    } else {
        saveline  = lineno;
        savecurr  = currfn;
        saveinfn  = infunc;
        savestart = fnstart;
        newfile();
    }
    kill();                     /* clear rest of line */
                                /* so next read will come from */
                                /* new file (if open */
}

/*                                      */
/*      Close an include file           */
/*                                      */
void
endinclude() {
        toconsole();
        outstr(";* #end include"); nl();
        tofile();

        input2  = 0;
        lineno  = saveline;
        currfn  = savecurr;
        infunc  = saveinfn;
        fnstart = savestart;
}

/*                                      */
/*      Declare a static variable       */
/*        (i.e. define for use)         */
/*                                      */
/* makes an entry in the symbol table so subsequent */
/*  references can call symbol by name  */
void
declglb(long typ) {              /* typ is CCHAR or CINT */
    int k,j;char sname[NAMESIZE];
    while(1) {
        while(1) {
            if(endst())return;     /* do line */
            k=1;            /* assume 1 element */
            if(match("*"))  /* pointer ? */
                j=POINTER;      /* yes */
            else
                j=VARIABLE; /* no */
            if (isSymname(sname)==0) /* name ok? */
                illname(); /* no... */
            if(findglb(sname)) /* already there? */
                multidef(sname);
            if (match("[")) {        /* array? */
                k=needsub();   /* get size */
                if(k)j=ARRAY;   /* !0=array */
                else j=POINTER; /* 0=ptr */
            }
            addglb(sname,j,typ,k); /* add symbol */
            break;
        }
        if (match(",")==0) return; /* more? */
    }
}

/*                                      */
/*      Declare local variables         */
/*      (i.e. define for use)           */
/*                                      */
/* works just like "declglb" but modifies machine stack */
/*      and adds symbol table entry with appropriate */
/*      stack offset to find it again                   */
void
declloc(long typ) {              /* typ is CCHAR or CINT */
    int k,j,newsp; char sname[NAMESIZE];
    newsp=Zsp;
    while(1) {
        if(endst())break;
        if(match("*"))
            j=POINTER;
        else
            j=VARIABLE;
        if (isSymname(sname)==0)
            illname();
        if(findloc(sname))
            multidef(sname);
        if (match("[")) {
            k=needsub();
            if(k) {
                j=ARRAY;
                if(typ==CINT) k=k+k;
            } else {
                j=POINTER;
                k=2;
            }
        } else {
            if((typ==CCHAR) && (j!=POINTER))
                k=1;
            else
                k=2;
        }
        newsp=newsp-k;
        addloc(sname,j,typ,newsp);
        if (match(",")==0) break;
    }
    /* change machine stack */
    Zsp=modstk(newsp);
}
/*      >>>>>> start of cc2 <<<<<<<<    */

/*                                      */
/*      Get required array size         */
/*                                      */
/* invoked when declared variable is followed by "[" */
/*      this routine makes subscript the absolute */
/*      size of the array. */
//int
long
needsub() {
    long num[1];

    if(match("]")) return (0);       /* null size */
    if (number(num)==0) {    /* go after a number */
        error("must be constant");     /* it isn't */
        num[0]=1;               /* so force one */
    }
    if (num[0]<0) {
        error("negative size illegal");
        num[0]=(-num[0]);
    }
    needbrack("]");         /* force single dimension */
    return (num[0]);                /* and return size */
}

/*                                      */
/*      Begin a function                */
/*                                      */
/* Called from "parse" this routine tries to make a function */
/*      out of what follows.    */
void
newfunc() {
    char n[NAMESIZE];       /* ptr => currfn */
    if (isSymname(n)==0) {
        error("illegal function or declaration");
        kill(); /* invalidate line */
        return;
    }
    fnstart=lineno;         /* remember where fn began */
    infunc=1;               /* note, in function now. */
    if(currfn=findglb(n)) {  /* already in symbol table ? */
        if(currfn[IDENT]!=FUNCTION) {
            multidef(n); /* already variable by that name */
        } else if(currfn[OFFSET]==FUNCTION) {
            multidef(n); /* already function by that name */
        } else {
            currfn[OFFSET]=FUNCTION; /* otherwise we have what was earlier*/
            /*  assumed to be a function */
        }
    } else {
        /* if not in table, define as a function now */
        currfn=addglb(n,FUNCTION,CINT,FUNCTION);
    }

    toconsole();
    nl(); outstr(";* ====== "); outstr(currfn+NAME); outstr("()\n");
    tofile();

    /* we had better see open paren for args... */
    if(match("(")==0) error("missing open paren");
    outstr(n);col();nl();   /* print function name */
    argstk=0;               /* init arg count */
    while(match(")")==0) {   /* then count args */
        /* any legal name bumps arg count */
        if(isSymname(n)) {
            argstk=argstk+2;
        } else {
            error("illegal argument name");junk();
        }

        blanks();
        /* if not closing paren, should be comma */
        if(streq((char *) line+lidx,")")==0) {
            if(match(",")==0) {
                error("expected comma");
            }
            if(endst())break;
        }
    }
    locidx=STARTLOC;        /* "clear" local symbol table*/
    Zsp=0;                  /* preset stack ptr */
    while(argstk) {
        /* now let user declare what types of things */
        /*      those arguments were */
        if(amatch("char",4)){
            getarg(CCHAR);ns();
        } else if(amatch("int",3)) {
            getarg(CINT);ns();
        } else{
            error("wrong number args");break;
        }
    }
    if(statement()!=STRETURN) { /* do a statement, but if */
                                /* it's a return, skip */
                                /* cleaning up the stack */
        modstk(0);
        zret();
    }
    Zsp=0;                  /* reset stack ptr again */
    locidx=STARTLOC;        /* deallocate all locals */
    infunc=0;               /* not in fn. any more   */
}

/*                                      */
/*      Declare argument types          */
/*                                      */
/* called from "newfunc" this routine adds an entry in the */
/*      local symbol table for each named argument */
void
getarg(long t) {              /* t = CCHAR or CINT */
    char n[NAMESIZE],c;int j;
    while(1) {
        if(argstk==0)
            return;   /* no more args */

        if(match("*"))
            j=POINTER;
        else
            j=VARIABLE;

        if(isSymname(n)==0)
            illname();

        if(findloc(n))
            multidef(n);

        if(match("[")) { /* pointer ? */
            /* it is a pointer, so skip all */
            /* stuff between "[]" */
            while(inbyte()!=']')
                if(endst())break;
            j=POINTER;
            /* add entry as pointer */
        }
        if((t==CCHAR) && (j!=POINTER))
            addloc(n,j,t,argstk+1);
        else
            addloc(n,j,t,argstk);
        argstk=argstk-2;        /* cnt down */

        if(endst())
            return;
        if(match(",")==0)
            error("expected comma");
    }
}

/*                                         */
/*      Statement parser                   */
/*                                         */
/* called whenever syntax requires         */
/*      a statement.                       */
/*  this routine performs that statement   */
/*  and returns a number telling which one */
//int
long
statement() {
    if ((ch()==0) && (eof))
        return 0;
    else if(amatch("char",4))
        {declloc(CCHAR);ns();}
    else if(amatch("int",3))
        {declloc(CINT);ns();}
    else if(match("{"))
        compound();
    else if(amatch("if",2))
        {doif();lastst=STIF;}
    else if(amatch("while",5))
        {dowhile();lastst=STWHILE;}
    else if(amatch("return",6))
        {doreturn();ns();lastst=STRETURN;}
    else if(amatch("break",5))
        {dobreak();ns();lastst=STBREAK;}
    else if(amatch("continue",8))
        {docont();ns();lastst=STCONT;}
    else if(match(";"))
        /* end of statement */ ;
    else if(match("#asm"))
        {doasm();lastst=STASM;}
    /* if nothing else, assume it's an expression */
    else {
        expression();ns();lastst=STEXP;
    }
    return (lastst);
}

/*                                      */
/*      Semicolon enforcer              */
/*                                      */
/* called whenever syntax requires a semicolon */
void
ns() {
    if(match(";")==0)
        error("missing semicolon");
}

/*                                      */
/*      Compound statement              */
/*                                      */
/* allow any number of statements to fall between "{}" */
void
compound() {
    ++ncmp;         /* new level open */
    while (match("}")==0) statement(); /* do one */
    --ncmp;         /* close current level */
}

/*                                      */
/*              "if" statement          */
/*                                      */
void
doif() {
    int flev,fsp,flab1,flab2;

    flev  = locidx;              /* record current local level */
    fsp   = Zsp;                 /* record current stk ptr */
    flab1 = getlabel();          /* get label for false branch */
    test(flab1);                 /* get expression, and branch false */
    statement();                 /* if true, do a statement */
    Zsp=modstk(fsp);             /* then clean up the stack */
    locidx=flev;                 /* and deallocate any locals */

    if (amatch("else",4) == 0) { /* if...else ? */
        /* simple "if"...print false label */
        printlabel(flab1);col();nl();
        return;                 /* and exit */
    }
    /* an "if...else" statement. */
    jump(flab2=getlabel());       /* jump around false code */
    printlabel(flab1);col();nl(); /* print false label */
    statement();                  /* and do "else" clause */
    Zsp = modstk(fsp);            /* then clean up stk ptr */
    locidx = flev;                /* and deallocate locals */
    printlabel(flab2);col();nl(); /* print true label */
}

/*                                      */
/*      "while" statement               */
/*                                      */
void
dowhile() {
    long wql[4];            /* allocate local queue */

    wql[WQSYM]=locidx;      /* record local level */
    wql[WQSP]=Zsp;          /* and stk ptr */
    wql[WQLOOP]=getlabel(); /* and looping label */
    wql[WQLAB]=getlabel();  /* and exit label */
    addwhile(wql);          /* add entry to queue */

    /* (for "break" statement) */
    printlabel(wql[WQLOOP]);col();nl(); /* loop label */
    test(wql[WQLAB]);       /* see if true */
    statement();            /* if so, do a statement */
    jump(wql[WQLOOP]);      /* loop to label */
    printlabel(wql[WQLAB]);col();nl(); /* exit label */
    locidx=wql[WQSYM];      /* deallocate locals */
    Zsp=modstk(wql[WQSP]);  /* clean up stk ptr */
    delwhile();             /* delete queue entry */
}

/*                                      */
/*      "return" statement              */
/*                                      */
void
doreturn() {
    /* if not end of statement, get an expression */
    if(endst()==0)
        expression();
    modstk(0);      /* clean up stk */
    zret();         /* and exit function */
}

/*                                      */
/*      "break" statement               */
/*                                      */
void
dobreak() {
    long *ptr;
    /* see if any "whiles" are open */
    ptr = readwhile();
    if (ptr == 0)
        return;       /* no */
    modstk((ptr[WQSP]));    /* else clean up stk ptr */
    jump(ptr[WQLAB]);       /* jump to exit label */
}

/*                                      */
/*      "continue" statement            */
/*                                      */
void
docont() {
    long *ptr;
    /* see if any "whiles" are open */
    ptr = readwhile();
    if (ptr == 0) return;       /* no */
    modstk((ptr[WQSP]));    /* else clean up stk ptr */
    jump(ptr[WQLOOP]);      /* jump to loop label */
}

/*                                      */
/*      "asm" pseudo-statement          */
/*                                      */
/* enters mode where assembly language statement are */
/*      passed intact through parser    */
void
doasm() {
    cmode=0;                         /* mark mode as "asm" */
    while (1) {
        myInline();                    /* get and print lines */
        if (match("#endasm")) break; /* until... */
        if(eof)break;
        outstr(line);
        nl();
    }
    kill();                          /* invalidate line */
    cmode=1;                         /* then back to parse level */
}
/*      >>>>> start of cc3 <<<<<<<<<    */

/*                                      */
/*      Perform a function call         */
/*                                      */
/* called from heir11, this routine will either call */
/*      the named function, or if the supplied ptr is */
/*      zero, will call the contents of primary register */
void
callfunction(char *ptr) {
    int nargs;
    nargs=0;
    blanks();       /* already saw open paren */
    if(ptr==0)
        zpush();      /* calling primary register */

    while(streq((char *) line+lidx,")")==0) {
        if(endst())break;
        expression();   /* get an argument */
        if(ptr==0)
            swapstk(); /* don't push addr */
        zpush();        /* push argument */
        nargs=nargs+2;  /* count args*2 */
        if (match(",")==0)
            break;
    }
    needbrack(")");
    if(ptr)
        zcall(ptr);
    else
        callstk();
    Zsp=modstk(Zsp+nargs);  /* clean up arguments */
}

void
junk() {
    if(an(inbyte())) {
        while(an(ch()))gch();
    } else while(an(ch()) == 0) {
        if(ch()==0)break;
        gch();
    }
    blanks();
}

//int
long
endst() {
    blanks();
    return ((streq((char *) line+lidx,";") || (ch()==0)));
}

void
illname() {
    error("illegal symbol name");junk();
}

void
multidef(char *sname) {
    error("already defined");
    comment();
    outstr(sname);nl();
}

void
needbrack(char *str) {
    if (match(str)==0) {
        error("missing bracket");
        comment();outstr(str);nl();
    }
}

void
needlval(){
    error("must be lvalue");
}

char *
findglb(char *sname) {
    char *ptr;

    ptr = STARTGLB;
    while(ptr != glbptr) {
        if(astreq(sname, ptr, NAMEMAX))
            return (ptr);
        ptr=ptr+SYMSIZ;
    }
    return (NULL);
}

//int
long
findloc(char *sname) {
    char *ptr;

    ptr = STARTLOC;

    while(ptr != locidx) {
        if(astreq(sname, ptr, NAMEMAX)) {
            return (ptr);
        }

        ptr = ptr+SYMSIZ;
    }
    return (0);
}

char *
addglb(char *sname, char id, char typ, long value) {
    char *ptr;

    if(cptr = findglb(sname))
        return (cptr);
    if(glbptr >= ENDGLB) {
        error("global symbol table overflow");
        return (0);
    }

    cptr = ptr = glbptr;

    // cptr[NAME]  = sname (sorta)
    while(an(*ptr++ = *sname++));   /* copy name */

    cptr[IDENT]    = id;
    cptr[TYPE]     = typ;
    cptr[STORAGE]  = STATIK;
    cptr[OFFSET]   = value>>8;
    cptr[OFFSET+1] = value&255;

    glbptr         = glbptr+SYMSIZ;

    return (cptr);
}

char *
addloc(char *sname, char id, char typ, long value) {
    char *ptr;

    if(cptr = findloc(sname)){
        return (cptr);
    }

    if(locidx>=ENDLOC) {
        error("local symbol table overflow");
        return (0);
    }

    cptr = ptr = (char *) locidx;

    // cptr[NAME]  = sname (sorta)
    while(an(*ptr++ = *sname++));   /* copy name */

    cptr[IDENT]    = id;
    cptr[TYPE]     = typ;
    cptr[STORAGE]  = STKLOC;
    cptr[OFFSET]   = value>>8;
    cptr[OFFSET+1] = value&255;

    locidx         = locidx+SYMSIZ;

    return (cptr);
}
/* Test if next input string is legal symbol name */
//int
long
isSymname(char *sname) {
    int k;char c;
    blanks();
    if(alpha(ch())==0)
        return (0);
    k=0;
    while(an(ch()))
        sname[k++]=gch();
    sname[k]=0;
    return (1);
}

/* Return next avail internal label number */
//int
long
getlabel() {
    return(++nxtlab);
}

/* Print specified number as label */
void
printlabel(long label) {
    outstr("cc");
    outdec(label);
}

void
addwhile(long ptr[]) {
    int k;
    if (wqidx==WQMAX) {
        error("too many active whiles");return;}
    k=0;
    while (k<WQSIZ) {
        *wqidx++ = ptr[k++];
    }
}

void
delwhile() {
    if(readwhile())
        wqidx=wqidx-WQSIZ;
}

//int was
long *
readwhile() {
    if (wqidx==wq) {
        error("no active whiles");
        return (0);
    } else {
        return (wqidx-WQSIZ);
    }
}

void
kill() {
    lidx = 0;
    line[lidx] = 0;
}

//int
long
inbyte() {
    while(ch()==0) {
        if (eof)
            return (0);
        myInline();
        preprocess();
    }
    return (gch());
}

//int
long
inchar() {
    if(ch()==0)
        myInline();
    if(eof)
        return (0);
    return(gch());
}

void
myInline() {
    FILE *unit;
    int k;
    
    while(1) {
        /*
        ** We're seeing input 0 when we're done with the main input file
        */
        if(eof) {
            return;
        }

        // Well this is now stupid
        if (input == 0) { // njc
            // This has an exit if "" so this is wrong
            //openin();
            eof = 1;
            return;
        }

        if((unit = input2) == 0) {
            unit = input;
        }

        kill();

        while((k = getc(unit)) > 0) {
            if((k==EOL) || (lidx>=LINEMAX))
                break;
            line[lidx]=k;
            lidx++;
        }

        line[lidx]=0;   /* append null */
        lineno++;       /* read one more line */

        if(k <= 0) {
            myFclose(unit);

            if(input2) {
                endinclude();
            } else {
                input = 0;
            }
        }

        if(lidx) {
            if((ctext) && (cmode)) {
                comment();
                outstr(line);
                nl();
            }
            lidx=0;
            return;
        }
    }
}
/*      >>>>>> start of cc4 <<<<<<<     */

//int
long
keepch(char c) {
    mline[midx] = c;
    if(midx < MPMAX)
        midx++;

    return (c);
}

/*
* / * C Demo Program * /
*#include "run9.c"
^C
Program received signal SIGINT, Interrupt.
preprocess () at smallc.c:2221
2221        return(line[lidx]&127);
(gdb) bt
#0  preprocess () at smallc.c:2221
#1  0x00005555555590b6 in preprocess () at smallc.c:1005
#2  blanks () at smallc.c:1239
#3  amatch (lit=<synthetic pointer>, len=4) at smallc.c:1225
#4  parse () at smallc.c:151
#5  0x0000555555555212 in main (argc=<optimized out>, argv=<optimized out>) at smallc.c:115
(gdb) print line
$1 = "#include \"run9.c\"\000*", '\000' <repeats 59 times>
(gdb) print lidx
$2 = 0 '\000'

It seems to hang here
main()
  parse()
    amatch()
      blanks()
        preprocess()
line 2221 return(line[lidx]&127);
but that's actual part of ch() 
So we're off the rails
 */
void
preprocess() {
    int k;
    char c,sname[NAMESIZE];

    if(cmode == 0)
        return;

    midx = lidx = 0;

    while(ch()) {
        if((ch() == ' ') || (ch() == TAB)) { // Whitespace
            keepch(' ');
            while((ch() == ' ') || (ch() == TAB)) { //
                gch();
            }
        } else if(ch() == '"') { // Double Quote
            keepch(ch());
            gch();
            while(ch() != '"') {
                if(ch() == 0) {
                    error("missing quote");
                    break;
                }
                keepch(gch());
            }
            gch();
            keepch('"');
        } else if(ch() == SQUOTE) { // ' Singel Quote
            keepch(SQUOTE);
            gch();
            while(ch() != SQUOTE) {
                if(ch() == 0) {
                    error("missing apostrophe");
                    break;
                }
                keepch(gch());
            }
            gch();
            keepch( SQUOTE);
        } else if((ch() == '/') && (nch() == '*')) { // Comment
            inchar();inchar();
            while(((ch() == '*') && (nch() == '/')) == 0) {
                if(ch() == 0) myInline();
                else inchar();
                if(eof) break;
            }
            inchar();inchar();
        } else if(an(ch())) {   // A number
           k=0;
            while(an(ch())) {
                if(k<NAMEMAX) sname[k++]=ch();
                gch();
            }
            sname[k]=0;
            if(k = findmac(sname)) {
                while(c=macq[k++])
                    keepch(c);
            } else {
                k=0;
                while(c = sname[k++])
                    keepch(c);
            }
        } else if(ch() == NULL){
            break;
        } else {
            keepch(gch());
        }
    }

    keepch(0);

    if(midx >= MPMAX)
        error("line too long");
    
    lidx = midx = 0;

    while(line[lidx++] = mline[midx++]);
    lidx = 0;
}

void
addmac() {
    char sname[NAMESIZE];
    int k;

    if(isSymname(sname)==0) {
        illname();
        kill();
        return;
    }

    k=0;
    while(putmac(sname[k++]));
    while(ch()==' ' || ch()==9) gch();
    while(putmac(gch()));
    if(macidx>=MACMAX)
        error("macro table full");
}

//int
char
putmac(char c) {
    macq[macidx]=c;
    if(macidx<MACMAX)macidx++;
    return (c);
}

long
findmac(char *sname) {
    long k;

    k=0;

    while(k<macidx) {
        if(astreq(sname,macq+k,NAMEMAX)) {
            while(macq[k++]);
            return (k);
        }
        while(macq[k++]); // @FIXEME: Why twice? (see next line)
        while(macq[k++]);
    }
    return (0);
}

void
outstr(char *ptr) {
    while(outbyte(*ptr++));
}

#ifdef __linux
void
nl() {
    outbyte('\n');
}
#else
void
nl() {
    outbyte(EOL);
}
#endif

void
tab() {
    outbyte('\t');	
}

void
col() {
    outbyte(' ');
}

void
bell() {
    outbyte(7);
}

void
error(char ptr[]) {
    int k;
    char yunk[81];

    toconsole();
    //bell();
    outstr(";* ### <");
    outstr(line);
    outstr(">\n");
    outstr(";* ### error line #"); outdec(lineno); outstr(", ");
    if(infunc==0)
        outbyte('(');
    if(currfn==NULL)
        outstr("start of file");
    else
        outstr(currfn+NAME);

    if(infunc==0)
        outbyte(')');

    outstr(" + ");
    outdec(lineno-fnstart);
    outstr(": ");  outstr(ptr);  nl();

    outstr(line); nl();

    k=0;    /* skip to error position */

    while(k<lidx) {
        if(line[k++]==9)
            tab();
        else
            outbyte(' ');
    }
    outbyte('^');  nl();
    ++errcnt;

    if(errstop) {
        pl("Continue (Y,n,g) ? ");
        myGets(yunk);             
        k=yunk[0];
        if((k=='N') || (k=='n'))
            myAbort();
        if((k=='G') || (k=='g'))
            errstop=0;
    }
    tofile();
    /* end error */
}

void
ol(char ptr[]) {
    ot(ptr);
    nl();
}

void
ot(char ptr[]) {
    tab();
    outstr(ptr);
}

//int
long
match(char *lit) {
    int k;

    blanks();
    if (k=streq((char *) line+lidx,lit)) {
        lidx=lidx+k;
        return (1);
    }
    return (0);
}

//int
long
amatch(char *lit, long len) {
    int k;
    blanks();
    if (k=astreq((char *) line+lidx,lit,len)) {
        lidx=lidx+k;
        while(an(ch())) inbyte();
        return (1);
    }
    return (0);
}

void
blanks() {
    while(1) {
        while(ch() == 0) {
            myInline();
            preprocess();
            if(eof) break;
        }

        if(ch() == ' ')
            gch();
        else if(ch() == TAB)
            gch();
        else return;
    }
}

/*
** Well I think we need to convert this to 16 bits
*/
void
outdec(long numb) {
    int k, zs;
    char c;

    zs = 0;
    k  = 10000;

    if (numb < 0) {
        numb=(-numb);
        outbyte('-');
    }

    numb &= 0xFFFF;

    while (k>=1) {
        c = numb/k + '0';
        if ((c!='0') || (k==1)|(zs)) {
            zs = 1;
            outbyte(c);
        }
        numb = numb % k;
        k = k/10;
    }
}

/*      >>>>>>> start of cc5 <<<<<<<    */
void
expression() {
    long lval[2];

    if(heir1(lval)) {
        rvalue(lval);
    }
}

//int
long
primary(long *lval) {
    char *ptr;
    char sname[NAMESIZE];
    long num[1];

    int k;
    if(match("(")) {
        k=heir1(lval);
        needbrack(")");
        return (k);
    }

    //
    // GRRR sname gets populate in isSymname
    //
    if(isSymname(sname)) {
        if(ptr = (char *) findloc(sname)) {
            getloc(ptr);
            lval[0] = (long) ptr;
            lval[1] = (long) ptr[TYPE];
            if(ptr[IDENT]==POINTER)lval[1]=CINT;
            if(ptr[IDENT]==ARRAY)return (0);
            else return (1);
        }

        if(ptr = findglb(sname)) {
            if(ptr[IDENT]!=FUNCTION) {
                lval[0] = (long) ptr;
                lval[1] = 0;
                if(ptr[IDENT]!=ARRAY)return (1);
                immed();
                outstr((char *) ptr);nl();
                lval[1]=ptr[TYPE];
                return (0);
            }
        }

        ptr=addglb(sname,FUNCTION,CINT,0);
        lval[0] = (long) ptr;
        lval[1] = 0;
        return (0);
    }

    if(constant(num))
        return(lval[0]=lval[1]=0);
    else {
        error("invalid expression");
        immed();outdec(0);nl();
        junk();
        return (0);
    }
}

void
store(long *lval) {
    if (lval[1]==0) putmem((char *) lval[0]);
    else            putstk(lval[1]);
}

void
rvalue(long *lval) {
    if((lval[0] != 0) && (lval[1] == 0)) {
        getmem((char *) lval[0]);
    } else {
        indirect(lval[1]);
    }
}

void
test(long label) {
    needbrack("(");
    expression();
    needbrack(")");
    testjump(label);
}

//int
long
constant(long val[]) {
    if (number(val))
        immed();
    else if (pstr(val))
        immed();
    else if (qstr(val)) {
        immed();printlabel(litlab);outbyte('+');
    }
    else return(0);
    outdec(val[0]);
    nl();
    return(1);
}

//int
long
number(long val[]) {
    int k, minus; char c;
    k = minus = 1;
    while(k) {
        k = 0;
        if (match("+")) k = 1;
        if (match("-")) {minus = (-minus); k = 1;}
    }
    if (numeric(ch()) == 0) return(0);
    while (numeric(ch())) {
        c = inbyte();
        k = k * 10 + (c - '0');
    }
    if (minus < 0) k = (-k);
    val[0] = k;
    return(1);
}

//int
long
pstr(long val[]) {
    int k; char c;
    k = 0;
    if (match("'") == 0) return(0);
    while ((c = gch()) != 39)
        k = (k & 255) * 256 + (c & 127);
    val[0] = k;
    return(1);
}

//int
long
qstr(long val[]) {
    char c;
    if (match(quote) == 0) return(0);
    val[0] = litidx;

    while (ch() != '"') {
        if (ch() == 0) break;
        if (litidx >= LITMAX) {
            error("string space exhausted");
            while (match(quote) == 0)
                if (gch() == 0) break;
            return(1);
        }
        litq[litidx++]=gch();
    }
    gch();
    litq[litidx++]=0;
    return (1);
}

/*      >>>>>> start of cc8 <<<<<<<     */

/* Begin a comment line for the assembler */
void
comment() {
    #ifdef ASL
        // ASL assembler
        outstr(";* ");
    #else
        // TSC, Motorola assembler
        outbyte('*');
    #endif
}

/* Put out assembler info before any code is generated */
void
header() {
    comment();
    outstr(BANNER);
    nl();
    comment();
    outstr(VERSION);
    nl();
    comment();
    outstr(AUTHOR);
    nl();
    comment();
    nl();

    if(mainflg) {               /* do stuff needed for first */
        ol("LIB CCC.H");        /* assembler file */

        zcall("main");          /* call the code generated by small-c */
        pseudoins(43); ol("JMP WARMS");
    }
}

/* Print any assembler stuff needed after all code */
void
trailer() {
    ol("END");

    nl();
    comment();
    outstr(" --- End of Compilation ---");
    nl();
}

/* Fetch a static memory cell into the primary register */
void
getmem(char *sym) {
    if((sym[IDENT]!=POINTER)&(sym[TYPE]==CCHAR)) {
        outstr("\n;* getmem(3)\n");
        pseudoins(3);
    } else {
        outstr("\n;* getmem(2)\n");
        pseudoins(2);
    }
    defword(); outstr(sym+NAME); nl();
}

/* Fetch the address of the specified symbol */
/*      into the primary register */
void
getloc(char *sym) {
    outstr("\n;* getloc(1)\n");
    pseudoins(1); defword();
    outdec((sym[OFFSET]&255)*256+(sym[OFFSET+1]&255)-Zsp);
    nl();
}

/* Store the primary register into the specified */
/*      static memory cell */
void
putmem(char *sym) {
    if((sym[IDENT]!=POINTER)&(sym[TYPE]==CCHAR)) {
        outstr("\n;* putmem(7)\n");
        pseudoins(7);
    } else {
        outstr("\n;* putmem(6)\n");
        pseudoins(6);
    }
    defword();
    outstr(sym+NAME);
    nl();
}

/* Store the specified object type in the primary register */
/*      at the address on the top of the stack */
void
putstk(char typeobj) {
    zpop();
    if(typeobj==CCHAR) {
        outstr("\n;* putstk(9)\n");
        pseudoins(9);
    } else {
        outstr("\n;* putstk(8)\n");
        pseudoins(8);
    }
}

/* Fetch the specified object type indirect through the */
/*      primary register into the primary register  */
void
indirect(char typeobj) {
    if(typeobj==CCHAR) {
        outstr("\n;* indirect(5)\n");
        pseudoins(5);
    } else {
        outstr("\n;* indirect(4)\n");
        pseudoins(4);
    }
}

/* Print partial instruction to get an immediate value */
/*      into the primary register */
void
immed() {
    outstr("\n;* imediate(0)\n");
    pseudoins(0); defword();
}

/* Push the primary register onto the stack */
void
zpush() {
    outstr("\n;* zpush(10)\n");
    pseudoins(10);
    Zsp=Zsp-2;
}

/* Pop the top of the stack  */
void
zpop() {
    outstr("\n;* zpop()\n");
    Zsp=Zsp+2;
}

/* Swap the primary register and the top of the stack */
void
swapstk() {
    outstr("\n;* swapstk(11)\n");
    pseudoins(11);
}

/* Call the specified subroutine name */
void
zcall(char *sname) {
    outstr("\n;* zcall(14)\n");
    pseudoins(14); defword();
    outstr(sname);
    nl();
}

/* Return from subroutine */
void
zret() {
    outstr("\n;* zret(16)\n");
    pseudoins(16);
}

/* Perform subroutine call to value on top of stack */
void
callstk() {
    outstr("\n;* callstk(15)\n");
    pseudoins(15);
    Zsp=Zsp-2;
}

/* Jump to specified internal label number */
void
jump(long label) {
    outstr("\n;* jump(15)\n");
    pseudoins(12); defword();
    printlabel(label);
    nl();
}

/* Test the primary register and jump if false to label */
void
testjump(long label) {
    outstr("\n;* testjump(13)\n");
    pseudoins(13); defword();
    printlabel(label);
    nl();
}

char *JTable[] = {
    "LD1IM",
    "LD1SOFF",
    "LD1",
    "LDB1",
    "LD1R",
    "LDB1R",
    "ST1",
    "STB1",
    "ST1SP",
    "STB1SP",
    "PUSHR1",
    "EXG1",
    "JMPL",
    "BRZL",
    "JSRL",
    "JSRSP",
    "RTSC",
    "MODSP",
    "DBL1",
    "ADDS",
    "SUBFST",
    "MUL1",
    "DIV1",
    "MOD",
    "ORS",
    "XORS",
    "ANDS",
    "ASRS",
    "ASLS",
    "NEGR",
    "NOTR",
    "INCR",
    "DECR",
    "ZEQ",
    "ZNE",
    "ZLT",
    "ZLE",
    "ZGT",
    "ZGE",
    "ULT",
    "ULE",
    "UGT",
    "UGE",
    "ASMC"
};

/* Print a pseudo-instruction for interpreter  */
void
pseudoins(long k) {
    /*
    outstr(";* pseudoins("); // njc
    outdec(k);
    outstr(")\n");
    */
    defbyte();
    outdec(k+k);
    ot(";* "); outstr(JTable[k]);
    nl();
}

/* Print pseudo-op to define a byte */
void
defbyte() {
    ot("FCB ");
}

/*Print pseudo-op to define storage */
void
defstorage() {
    ot("RMB ");
}

/* Print pseudo-op to define a word */
void
defword() {
    ot("FDB ");
}

/* Modify the stack pointer to the new value indicated */
//int
long
modstk(long newsp) {
    int k;

    k=newsp-Zsp;
    if(k==0) return(newsp);
    outstr("\n;* modstk(17)\n");
    pseudoins(17); defword();
    outdec(k); nl();
    return (newsp);
}

/* Double the primary register */
void
doublereg() {
    outstr("\n;* doublereg(18)\n");
    pseudoins(18);
}

/* Add the primary and top of stack (pop) */
/*      (results in primary) */
void
zadd() {
    outstr("\n;* zadd(19)\n");
    pseudoins(19);
}

/* Subtract the primary register from the top of stack */
/*      (results in primary) */
void
zsub() {
    outstr("\n;* zsub(20)\n");
    pseudoins(20);
}

/* Multiply the primary register and top of stack */
/*      (results in primary */
void
mult() {
    outstr("\n;* mult(21)\n");
    pseudoins(21);
}

/* Divide the top of stack by the primary register */
/*      (quotient in primary) */
void
myDiv() {
    outstr("\n;* myDiv(22)\n");
    pseudoins(22);
}

/* Compute remainder (mod) of top of stack divided */
/*      by the primary */
/*      (remainder in primary)  */
void
zmod() {
    outstr("\n;* zmod(23)\n");
    pseudoins(23);
}

/* Inclusive 'or' the primary and the top of stack */
/*      (results in primary) */
void
zor() {
    outstr("\n;* zor(24)\n");
    pseudoins(24);
}

/* Exclusive 'or' the primary and the top of stack */
/*      (results in primary) */
void
zxor() {
    outstr("\n;* zxor(25)\n");
    pseudoins(25);
}

/* 'And' the primary and the top of stack */
/*      (results in primary) */
void
zand() {
    outstr("\n;* zand(26)\n");
    pseudoins(26);
}

/* Arithmetic shift right the top of stack number of */
/*      times in primary (results in primary) */
void
asr() {
    outstr("\n;* asr(27)\n");
    pseudoins(27);
}

/* Arithmetic left shift the top of stack number of */
/*      times in primary (results in primary) */
void
asl() {
    outstr("\n;* asl(28)\n");
    pseudoins(28);
}

/* Form two's complement of primary register */
void
neg() {
    outstr("\n;* neg(29)\n");
    pseudoins(29);
}

/* Form one's complement of primary register */
void
com() {
    outstr("\n;* com(30)\n");
    pseudoins(30);
}

/* Increment the primary register by one */
void
inc() {
    outstr("\n;* inc(31)\n");
    pseudoins(31);
}

/* Decrement the primary register by one */
void
dec() {
    outstr("\n;* dec(32)\n");
    pseudoins(32);
}

/* Following are the conditional operators */
/* They compare the top of the stack against the primary */
/* and put a literal 1 in the primary if the condition is */
/* true, otherwise they clear the primary register */

/* Test for equal */
void
zeq() {
    outstr("\n;* zeq()33\n");
    pseudoins(33);
}

/* Test for not equal */
void
zne() {
    outstr("\n;* zne(34)\n");
    pseudoins(34);
}
/* Test for less than (signed) */
void
zlt() {
    outstr("\n;* zlt(35)\n");
    pseudoins(35);
}

/* Test for less than or equal to (signed) */
void
zle() {
    outstr("\n;* zle(36)\n");
    pseudoins(36);
}

/* Test for greater than (signed) */
void
zgt() {
    outstr("\n;* zgt(37)\n");
    pseudoins(37);
}

/* Test for greater than or equal to (signed) */
void
zge() {
    outstr("\n;* zge(38)\n");
    pseudoins(38);
}

/* Test for less than (unsigned) */
void
ult() {
    outstr("\n;* ult(39)\n");
    pseudoins(39);
}

/* Test for less than or equal to (unsigned) */
void
ule() {
    outstr("\n;* ule(40)\n");
    pseudoins(40);
}

/* Test for greater than (unsigned) */
void
ugt() {
    outstr("\n;* ugt(41)\n");
    pseudoins(41);
}

/* Test for greater than or equal to (unsigned) */
void
uge() {
    outstr("\n;* uge(43)\n");
    pseudoins(42);
}

char *
myGets(char *buf) {
    // Really should put the null at the end of the buf
    if (fgets(buf, sizeof(buf), stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    }
}
//
// -----------------------------------------------------------------------------
//

/*       Test if given char is alpha     */
//int
long
alpha(char c) {
    c=c&127;
    return(((c>='a') && (c<='z')) ||
           ((c>='A') && (c<='Z')) ||
           (c=='_'));
}


/*       Test if given char is numeric   */
//int
long
numeric(char c) {
    c=c&127;

    return((c>='0') && (c<='9'));
}

/*       Test if given char is alphanumeric      */
//int
long
an(char c) {
    return( alpha(c) || numeric(c) );
}

// Current ch but not null
//int
long
ch() {
    return(line[lidx]&127);
}

// Next ch
//int
long
nch() {
    if(ch()==0)return(0);
    else return(line[lidx+1]&127);
}

// Not sure how this differs from ch()
//int
long
gch() {
    if(ch() == 0)return(0);
    else return(line[lidx++]&127);
}

/*  Compare two strings:  if equal return length,  else NULL */
//int
long
streq(char str1[], char str2[]) {
    int k;

    k=0;

    while (str2[k]) {
        if ((str1[k]) != (str2[k]))
            return 0;
        k++;
    }

    return k;
}

/*   compare strings of a given length */
//int
long
astreq(char str1[], char str2[], int len) {
    int k;

    k=0;

    while (k < len) {
        if ((str1[k]) != (str2[k])) {
            printf(";* ### (%s) != (%s) [%d]\n", str1, str2, k);
            printf(";* ### (%x) != (%x) [%d]\n", str1[k], str2[k], k);
            break;
        }
        if (str1[k] == 0)           break;
        if (str2[k] == 0)           break;
        k++;
    }
    return k;
}

void
doError(long i) {
    printf(";* ### %s\n", line);
    kill();
}

/*     <<<<< End of Small-C compiler >>>>>      */
