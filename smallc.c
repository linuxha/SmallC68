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
/*      >>>>> start cc1 <<<<<<          */

/*                                      */
/*      Compiler begins execution here  */
/*                                      */
main()
        {
        glbptr=STARTGLB;        /* clear global symbols */
        locptr=STARTLOC;        /* clear local symbols */
        wqptr=wq;               /* clear while queue */
        macptr=         /* clear the macro pool */
        litptr=         /* clear literal pool */
        Zsp =           /* stack ptr (relative) */
        errcnt=         /* no errors */
        errstop=        /* keep going after an error */
        eof=            /* not eof yet */
        input=          /* no input file */
        input2=         /* or include file */
        output=         /* no open units */
        saveout=        /* no diverted output */
        ncmp=           /* no open compound states */
        lastst=         /* no last statement yet */
        mainflg=        /* not first file to asm */
        fnstart=        /* current "function" started at line 0 */
        lineno=         /* no lines read from file */
        infunc=         /* not in function now */
        quote[1]=
        0;              /*  ...all set to zero.... */
        quote[0]='"';           /* fake a quote literal */
        currfn=NULL;    /* no function yet */
        cmode=1;        /* enable preprocessing */
        /*                              */
        /*      compiler body           */
        /*                              */
        ask();                  /* get user options */
        openout();              /* get an output file */
        openin();               /* and initial input file */
        header();               /* intro code */
        parse();                /* process ALL input */
        dumplits();             /* then dump literal pool */
        dumpglbs();             /* and all static memory */
        trailer();              /* follow-up code */
        closeout();             /* close the output (if any) */
        errorsummary();         /* summarize errors (on console!) */
        return;                 /* then exit to system */
        }

/*                                      */
/*      Abort compilation               */
/*                                      */
abort()
{
        if(input2)
                endinclude();
        if(input)
                fclose(input);
        closeout();
        toconsole();
        pl("Compilation aborted.");  nl();
        exit();
/* end abort */}

/*                                      */
/*      Process all input text          */
/*                                      */
/* At this level, only static declarations, */
/*      defines, includes, and function */
/*      definitions are legal...        */
parse()
        {
        while (eof==0)          /* do until no more input */
                {
                if(amatch("char",4)){declglb(CCHAR);ns();}
                else if(amatch("int",3)){declglb(CINT);ns();}
                else if(match("#asm"))doasm();
                else if(match("#include"))doinclude();
                else if(match("#define"))addmac();
                else newfunc();
                blanks();       /* force eof if pending */
                }
        }
/*                                      */
/*      Dump the literal pool           */
/*                                      */
dumplits()
        {int j,k;
        if (litptr==0) return;  /* if nothing there, exit...*/
        printlabel(litlab);col(); /* print literal label */
        k=0;                    /* init an index... */
        while (k<litptr)        /*      to loop with */
                {defbyte();     /* pseudo-op to define byte */
                j=10;           /* max bytes per line */
                while(j--)
                        {outdec((litq[k++]&127));
                        if ((j==0) | (k>=litptr))
                                {nl();          /* need <cr> */
                                break;
                                }
                        outbyte(',');   /* separate bytes */
                        }
                }
        }
/*                                      */
/*      Dump all static variables       */
/*                                      */
dumpglbs()
        {
        int j;
        if(glbflag==0)return;   /* don't if user said no */
        cptr=STARTGLB;
        while(cptr<glbptr)
                {if(cptr[IDENT]!=FUNCTION)
                        /* do if anything but function */
                        {outstr(cptr);col();
                                /* output name as label... */
                        defstorage();   /* define storage */
                        j=(((cptr[OFFSET]&255)<<8)+
                                (cptr[OFFSET+1]&255));
                                        /* calc # bytes */
                        if((cptr[TYPE]==CINT)|
                                (cptr[IDENT]==POINTER))
                                j=j+j;
                        outdec(j);      /* need that many */
                        nl();
                        }
                cptr=cptr+SYMSIZ;
                }
        }
/*                                      */
/*      Report errors for user          */
/*                                      */
errorsummary()
        {
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
        outstr("litq: "); outdec(litptr); nl();
        outstr("macq: "); outdec(macptr); nl();
        }
/*                                      */
/*      Get options from user           */
/*                                      */
ask()
        {
        int k,num[1];
        kill();                 /* clear input line */
        outbyte(12);            /* clear the screen */
        nl();nl();              /* print banner */
        pl(BANNER);
        nl();nl();
        pl(VERSION);
        nl();
        pl(AUTHOR);
        nl();
        nl();
        /* see if user wants to interleave the c-text */
        /*      in form of comments (for clarity) */
        pl("Do you want the c-text to appear (y,N) ? ");
        gets(line);             /* get answer */
        ctext=0;                /* assume no */
        if((ch()=='Y')|(ch()=='y'))
                ctext=1;        /* user said yes */
        /* see if the user is compiling everything at once */
        /*      (as is usually the case)                   */
        pl("Are you compiling the whole program at once (Y,n) ? ");
        gets(line);
        if((ch()!='N')&(ch()!='n')){    /* single file - assume... */
                glbflag=1;      /* define globals */
                mainflg=1;      /* first file to assembler */
                nxtlab =0;      /* start numbers at lowest possible */
                }
        else {          /* one of many - ask everything */
                /* see if user wants us to allocate static */
                /*  variables by name in this module    */
                /*      (pseudo external capability)    */
                pl("Do you want the globals to be defined (y,N) ? ");
                gets(line);
                glbflag=0;
                if((ch()=='Y')|(ch()=='y'))
                        glbflag=1;      /* user said yes */
                /* see if we should put out the stuff   */
                /*      needed for the first assembler  */
                /*      file.                           */
                pl("Is the output file the first one the assembler ");
                pl("will see (y,N) ? ");
                gets(line);
                mainflg=0;
                if((ch()=='Y')|(ch()=='y'))
                        mainflg=1;      /* indeed it is */
                /* get first allowable number for compiler-generated */
                /*      labels (in case user will append modules) */
                while(1){
                        pl("Starting number for labels (0) ? ");
                        gets(line);
                        if(ch()==0){num[0]=0;break;}
                        if(k=number(num))break;
                        }
                nxtlab=num[0];
                }
        /* see if user wants to be sure to see all errors */
        pl("Should I pause after an error (y,N) ? ");
        gets(line);
        errstop=0;
        if((ch()=='Y')|(ch()=='y'))
                errstop=1;

        litlab=getlabel();      /* first label=literal pool */ 
        kill();                 /* erase line */
        }
/*                                      */
/*      Get output filename             */
/*                                      */
openout()
        {
        kill();                 /* erase line */
        output=0;               /* start with none */
        pl("Output filename? "); /* ask...*/
        gets(line);     /* get a filename */
        if(ch()==0)return;      /* none given... */
        if((output=fopen(line,"w"))==NULL) /* if given, open */
                {output=0;      /* can't open */
                error("Open failure!");
                }
        kill();                 /* erase line */
}
/*                                      */
/*      Get (next) input file           */
/*                                      */
openin()
{
        input=0;                /* none to start with */
        while(input==0){        /* any above 1 allowed */
                kill();         /* clear line */
                if(eof)break;   /* if user said none */
                pl("Input filename? ");
                gets(line);     /* get a name */
                if(ch()==0)
                        {eof=1;break;} /* none given... */
                if((input=fopen(line,"r"))!=NULL)
                        newfile();
                else {  input=0;        /* can't open it */
                        pl("Open failure");
                        }
                }
        kill();         /* erase line */
        }

/*                                      */
/*      Reset line count, etc.          */
/*                                      */
newfile()
{
        lineno  = 0;    /* no lines read */
        fnstart = 0;    /* no fn. start yet. */
        currfn  = NULL; /* because no fn. yet */
        infunc  = 0;    /* therefore not in fn. */
/* end newfile */}

/*                                      */
/*      Open an include file            */
/*                                      */
doinclude()
{
        blanks();       /* skip over to name */

        toconsole();
        outstr("#include "); outstr(line+lptr); nl();
        tofile();

        if(input2)
                error("Cannot nest include files");
        else if((input2=fopen(line+lptr,"r"))==NULL)
                {input2=0;
                error("Open failure on include file");
                }
        else {  saveline = lineno;
                savecurr = currfn;
                saveinfn = infunc;
                savestart= fnstart;
                newfile();
                }
        kill();         /* clear rest of line */
                        /* so next read will come from */
                        /* new file (if open */
}

/*                                      */
/*      Close an include file           */
/*                                      */
endinclude()
{
        toconsole();
        outstr("#end include"); nl();
        tofile();

        input2  = 0;
        lineno  = saveline;
        currfn  = savecurr;
        infunc  = saveinfn;
        fnstart = savestart;
/* end endinclude */}

/*                                      */
/*      Close the output file           */
/*                                      */
closeout()
{
        tofile();       /* if diverted, return to file */
        if(output)fclose(output); /* if open, close it */
        output=0;               /* mark as closed */
}
/*                                      */
/*      Declare a static variable       */
/*        (i.e. define for use)         */
/*                                      */
/* makes an entry in the symbol table so subsequent */
/*  references can call symbol by name  */
declglb(typ)            /* typ is CCHAR or CINT */
        int typ;
{       int k,j;char sname[NAMESIZE];
        while(1)
                {while(1)
                        {if(endst())return;     /* do line */
                        k=1;            /* assume 1 element */
                        if(match("*"))  /* pointer ? */
                                j=POINTER;      /* yes */
                                else j=VARIABLE; /* no */
                         if (symname(sname)==0) /* name ok? */
                                illname(); /* no... */
                        if(findglb(sname)) /* already there? */
                                multidef(sname);
                        if (match("["))         /* array? */
                                {k=needsub();   /* get size */
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
declloc(typ)            /* typ is CCHAR or CINT */
        int typ;
        {
        int k,j,newsp; char sname[NAMESIZE];
        newsp=Zsp;
        while(1)  {
                if(endst())break;
                if(match("*"))
                        j=POINTER;
                        else j=VARIABLE;
                if (symname(sname)==0)
                        illname();
                if(findloc(sname))
                        multidef(sname);
                if (match("["))
                        {k=needsub();
                        if(k)
                                {j=ARRAY;
                                if(typ==CINT)k=k+k;
                                }
                        else
                                {j=POINTER;
                                k=2;
                                }
                        }
                else
                        if((typ==CCHAR)
                                &(j!=POINTER))
                                k=1;else k=2;
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
needsub()
        {
        int num[1];
        if(match("]"))return (0);       /* null size */
        if (number(num)==0)     /* go after a number */
                {error("must be constant");     /* it isn't */
                num[0]=1;               /* so force one */
                }
        if (num[0]<0)
                {error("negative size illegal");
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
newfunc()
        {
        char n[NAMESIZE];       /* ptr => currfn */
        if (symname(n)==0)
                {error("illegal function or declaration");
                kill(); /* invalidate line */
                return;
                }
        fnstart=lineno;         /* remember where fn began */
        infunc=1;               /* note, in function now. */
        if(currfn=findglb(n))   /* already in symbol table ? */
                {if(currfn[IDENT]!=FUNCTION)multidef(n);
                        /* already variable by that name */
                else if(currfn[OFFSET]==FUNCTION)multidef(n);
                        /* already function by that name */
                else currfn[OFFSET]=FUNCTION;
                        /* otherwise we have what was earlier*/
                        /*  assumed to be a function */
                }
        /* if not in table, define as a function now */
        else currfn=addglb(n,FUNCTION,CINT,FUNCTION);

        toconsole();
        nl(); outstr("====== "); outstr(currfn+NAME); outstr("()");
        tofile();

        /* we had better see open paren for args... */
        if(match("(")==0)error("missing open paren");
        outstr(n);col();nl();   /* print function name */
        argstk=0;               /* init arg count */
        while(match(")")==0)    /* then count args */
                /* any legal name bumps arg count */
                {if(symname(n))argstk=argstk+2;
                else{error("illegal argument name");junk();}
                blanks();
                /* if not closing paren, should be comma */
                if(streq(line+lptr,")")==0)
                        {if(match(",")==0)
                        error("expected comma");
                        }
                if(endst())break;
                }
        locptr=STARTLOC;        /* "clear" local symbol table*/
        Zsp=0;                  /* preset stack ptr */
        while(argstk)
                /* now let user declare what types of things */
                /*      those arguments were */
                {if(amatch("char",4)){getarg(CCHAR);ns();}
                else if(amatch("int",3)){getarg(CINT);ns();}
                else{error("wrong number args");break;}
                }
        if(statement()!=STRETURN) /* do a statement, but if */
                                /* it's a return, skip */
                                /* cleaning up the stack */
                {modstk(0);
                zret();
                }
        Zsp=0;                  /* reset stack ptr again */
        locptr=STARTLOC;        /* deallocate all locals */
        infunc=0;               /* not in fn. any more   */
        }
/*                                      */
/*      Declare argument types          */
/*                                      */
/* called from "newfunc" this routine adds an entry in the */
/*      local symbol table for each named argument */
getarg(t)               /* t = CCHAR or CINT */
        int t;
        {
        char n[NAMESIZE],c;int j;
        while(1)
                {if(argstk==0)return;   /* no more args */
                if(match("*"))j=POINTER;
                        else j=VARIABLE;
                if(symname(n)==0) illname();
                if(findloc(n))multidef(n);
                if(match("["))  /* pointer ? */
                /* it is a pointer, so skip all */
                /* stuff between "[]" */
                        {while(inbyte()!=']')
                                if(endst())break;
                        j=POINTER;
                        /* add entry as pointer */
                        }
                if((t==CCHAR)&(j!=POINTER)) addloc(n,j,t,argstk+1);
                        else    addloc(n,j,t,argstk);
                argstk=argstk-2;        /* cnt down */
                if(endst())return;
                if(match(",")==0)error("expected comma");
                }
        }
/*                                      */
/*      Statement parser                */
/*                                      */
/* called whenever syntax requires      */
/*      a statement.                     */
/*  this routine performs that statement */
/*  and returns a number telling which one */
statement()
{
        if ((ch()==0) & (eof)) return;
        else if(amatch("char",4))
                {declloc(CCHAR);ns();}
        else if(amatch("int",3))
                {declloc(CINT);ns();}
        else if(match("{"))compound();
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
        else if(match(";"));
        else if(match("#asm"))
                {doasm();lastst=STASM;}
        /* if nothing else, assume it's an expression */
        else{expression();ns();lastst=STEXP;}
        return (lastst);
}
/*                                      */
/*      Semicolon enforcer              */
/*                                      */
/* called whenever syntax requires a semicolon */
ns()    {if(match(";")==0)error("missing semicolon");}
/*                                      */
/*      Compound statement              */
/*                                      */
/* allow any number of statements to fall between "{}" */
compound()
        {
        ++ncmp;         /* new level open */
        while (match("}")==0) statement(); /* do one */
        --ncmp;         /* close current level */
        }
/*                                      */
/*              "if" statement          */
/*                                      */
doif()
        {
        int flev,fsp,flab1,flab2;
        flev=locptr;    /* record current local level */
        fsp=Zsp;                /* record current stk ptr */
        flab1=getlabel(); /* get label for false branch */
        test(flab1);    /* get expression, and branch false */
        statement();    /* if true, do a statement */
        Zsp=modstk(fsp);        /* then clean up the stack */
        locptr=flev;    /* and deallocate any locals */
        if (amatch("else",4)==0)        /* if...else ? */
                /* simple "if"...print false label */
                {printlabel(flab1);col();nl();
                return;         /* and exit */
                }
        /* an "if...else" statement. */
        jump(flab2=getlabel()); /* jump around false code */
        printlabel(flab1);col();nl();   /* print false label */
        statement();            /* and do "else" clause */
        Zsp=modstk(fsp);                /* then clean up stk ptr */
        locptr=flev;            /* and deallocate locals */
        printlabel(flab2);col();nl();   /* print true label */
        }
/*                                      */
/*      "while" statement               */
/*                                      */
dowhile()
        {
        int wql[4];             /* allocate local queue */
        wql[WQSYM]=locptr;      /* record local level */
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
        locptr=wql[WQSYM];      /* deallocate locals */
        Zsp=modstk(wql[WQSP]);  /* clean up stk ptr */
        delwhile();             /* delete queue entry */
        }
/*                                      */
/*      "return" statement              */
/*                                      */
doreturn()
        {
        /* if not end of statement, get an expression */
        if(endst()==0)expression();
        modstk(0);      /* clean up stk */
        zret();         /* and exit function */
        }
/*                                      */
/*      "break" statement               */
/*                                      */
dobreak()
        {
        int *ptr;
        /* see if any "whiles" are open */
        if ((ptr=readwhile())==0) return;       /* no */
        modstk((ptr[WQSP]));    /* else clean up stk ptr */
        jump(ptr[WQLAB]);       /* jump to exit label */
        }
/*                                      */
/*      "continue" statement            */
/*                                      */
docont()
        {
        int *ptr;
        /* see if any "whiles" are open */
        if ((ptr=readwhile())==0) return;       /* no */
        modstk((ptr[WQSP]));    /* else clean up stk ptr */
        jump(ptr[WQLOOP]);      /* jump to loop label */
        }
/*                                      */
/*      "asm" pseudo-statement          */
/*                                      */
/* enters mode where assembly language statement are */
/*      passed intact through parser    */
doasm()
        {
        cmode=0;                /* mark mode as "asm" */
        while (1)
                {inline();      /* get and print lines */
                if (match("#endasm")) break;    /* until... */
                if(eof)break;
                outstr(line);
                nl();
                }
        kill();         /* invalidate line */
        cmode=1;                /* then back to parse level */
        }
/*      >>>>> start of cc3 <<<<<<<<<    */

/*                                      */
/*      Perform a function call         */
/*                                      */
/* called from heir11, this routine will either call */
/*      the named function, or if the supplied ptr is */
/*      zero, will call the contents of primary register */
callfunction(ptr)
        char *ptr;      /* symbol table entry (or 0) */
{       int nargs;
        nargs=0;
        blanks();       /* already saw open paren */
        if(ptr==0)zpush();      /* calling primary register */
        while(streq(line+lptr,")")==0)
                {if(endst())break;
                expression();   /* get an argument */
                if(ptr==0)swapstk(); /* don't push addr */
                zpush();        /* push argument */
                nargs=nargs+2;  /* count args*2 */
                if (match(",")==0) break;
                }
        needbrack(")");
        if(ptr)zcall(ptr);
        else callstk();
        Zsp=modstk(Zsp+nargs);  /* clean up arguments */
}
junk()
{       if(an(inbyte()))
                while(an(ch()))gch();
        else while(an(ch())==0)
                {if(ch()==0)break;
                gch();
                }
        blanks();
}
endst()
{       blanks();
        return ((streq(line+lptr,";")|(ch()==0)));
}
illname()
{       error("illegal symbol name");junk();}
multidef(sname)
        char *sname;
{       error("already defined");
        comment();
        outstr(sname);nl();
}
needbrack(str)
        char *str;
{       if (match(str)==0)
                {error("missing bracket");
                comment();outstr(str);nl();
                }
}
needlval()
{       error("must be lvalue");
}
findglb(sname)
        char *sname;
{       char *ptr;
        ptr=STARTGLB;
        while(ptr!=glbptr)
                {if(astreq(sname,ptr,NAMEMAX))return (ptr);
                ptr=ptr+SYMSIZ;
                }
        return (0);
}
findloc(sname)
        char *sname;
{       char *ptr;
        ptr=STARTLOC;
        while(ptr!=locptr)
                {if(astreq(sname,ptr,NAMEMAX))return (ptr);
                ptr=ptr+SYMSIZ;
                }
        return (0);
}
addglb(sname,id,typ,value)
        char *sname,id,typ;
        int value;
{       char *ptr;
        if(cptr=findglb(sname))return (cptr);
        if(glbptr>=ENDGLB)
                {error("global symbol table overflow");
                return (0);
                }
        cptr=ptr=glbptr;
        while(an(*ptr++ = *sname++));   /* copy name */
        cptr[IDENT]=id;
        cptr[TYPE]=typ;
        cptr[STORAGE]=STATIK;
        cptr[OFFSET]=value>>8;
        cptr[OFFSET+1]=value&255;
        glbptr=glbptr+SYMSIZ;
        return (cptr);
}
addloc(sname,id,typ,value)
        char *sname,id,typ;
        int value;
{       char *ptr;
        if(cptr=findloc(sname))return (cptr);
        if(locptr>=ENDLOC)
                {error("local symbol table overflow");
                return (0);
                }
        cptr=ptr=locptr;
        while(an(*ptr++ = *sname++));   /* copy name */
        cptr[IDENT]=id;
        cptr[TYPE]=typ;
        cptr[STORAGE]=STKLOC;
        cptr[OFFSET]=value>>8;
        cptr[OFFSET+1]=value&255;
        locptr=locptr+SYMSIZ;
        return (cptr);
}
/* Test if next input string is legal symbol name */
symname(sname)
        char *sname;
{       int k;char c;
        blanks();
        if(alpha(ch())==0)return (0);
        k=0;
        while(an(ch()))sname[k++]=gch();
        sname[k]=0;
        return (1);
        }
/* Return next avail internal label number */
getlabel()
{       return(++nxtlab);
}
/* Print specified number as label */
printlabel(label)
        int label;
{       outstr("cc");
        outdec(label);
}
/* Print a carriage return and a string only to console */
pl(str)
        char *str;
{       int k;
        k=0;
        putchar(EOL);
        while(str[k])putchar(str[k++]);
}
addwhile(ptr)
        int ptr[];
 {
        int k;
        if (wqptr==WQMAX)
                {error("too many active whiles");return;}
        k=0;
        while (k<WQSIZ)
                {*wqptr++ = ptr[k++];}
}
delwhile()
        {if(readwhile()) wqptr=wqptr-WQSIZ;
        }
readwhile()
 {
        if (wqptr==wq){error("no active whiles");return (0);}
        else return (wqptr-WQSIZ);
 }
kill()
{       lptr=0;
        line[lptr]=0;
}
inbyte()
{
        while(ch()==0)
                {if (eof) return (0);
                inline();
                preprocess();
                }
        return (gch());
}
inchar()
{
        if(ch()==0)inline();
        if(eof)return (0);
        return(gch());
}
inline()
{
        int k,unit;
        while(1)
                {if (input==0)openin();
                if(eof)return;
                if((unit=input2)==0)unit=input;
                kill();
                while((k=getc(unit))>0)
                        {if((k==EOL)|(lptr>=LINEMAX))break;
                        line[lptr++]=k;
                        }
                line[lptr]=0;   /* append null */
                lineno++;       /* read one more line */
                if(k<=0)
                        {fclose(unit);
                        if(input2)endinclude();
                                else input=0;
                        }
                if(lptr)
                        {if((ctext)&(cmode))
                                {comment();
                                outstr(line);
                                nl();
                                }
                        lptr=0;
                        return;
                        }
                }
}
/*      >>>>>> start of cc4 <<<<<<<     */

keepch(c)
        char c;
{       mline[mptr]=c;
        if(mptr<MPMAX)mptr++;
        return (c);
}
preprocess()
{       int k;
        char c,sname[NAMESIZE];
        if(cmode==0)return;
        mptr=lptr=0;
        while(ch())
                {if((ch()==' ')|(ch()==9))
                        {keepch(' ');
                        while((ch()==' ')|
                                (ch()==9))
                                gch();
                        }
                else if(ch()=='"')
                        {keepch(ch());
                        gch();
                        while(ch()!='"')
                                {if(ch()==0)
                                  {error("missing quote");
                                  break;
                                  }
                                keepch(gch());
                                }
                        gch();
                        keepch('"');
                        }
                else if(ch()==39)
                        {keepch(39);
                        gch();
                        while(ch()!=39)
                                {if(ch()==0)
                                  {error("missing apostrophe");
                                  break;
                                  }
                                keepch(gch());
                                }
                        gch();
                        keepch(39);
                        }
                else if((ch()=='/')&(nch()=='*'))
                        {inchar();inchar();
                        while(((ch()=='*')&
                                (nch()=='/'))==0)
                                {if(ch()==0)inline();
                                        else inchar();
                                if(eof)break;
                                }
                        inchar();inchar();
                        }
                else if(an(ch()))
                        {k=0;
                        while(an(ch()))
                                {if(k<NAMEMAX)sname[k++]=ch();
                                gch();
                                }
                        sname[k]=0;
                        if(k=findmac(sname))
                                while(c=macq[k++])
                                        keepch(c);
                        else
                                {k=0;
                                while(c=sname[k++])
                                        keepch(c);
                                }
                        }
                else keepch(gch());
                }
        keepch(0);
        if(mptr>=MPMAX)error("line too long");
        lptr=mptr=0;
        while(line[lptr++]=mline[mptr++]);
        lptr=0;
        }
addmac()
{       char sname[NAMESIZE];
        int k;
        if(symname(sname)==0)
                {illname();
                kill();
                return;
                }
        k=0;
        while(putmac(sname[k++]));
        while(ch()==' ' | ch()==9) gch();
        while(putmac(gch()));
        if(macptr>=MACMAX)error("macro table full");
        }
putmac(c)
        char c;
{       macq[macptr]=c;
        if(macptr<MACMAX)macptr++;
        return (c);
}
findmac(sname)
        char *sname;
{       int k;
        k=0;
        while(k<macptr)
                {if(astreq(sname,macq+k,NAMEMAX))
                        {while(macq[k++]);
                        return (k);
                        }
                while(macq[k++]);
                while(macq[k++]);
                }
        return (0);
}

outbyte(c)
        char c;
{
        if(c==0)return (0);
        if(output)
                {if((putc(c,output))<=0)
                        {closeout();
                        error("Output file error");
                        abort();
                        }
                }
        else putchar(c);
        return (c);
}
outstr(ptr)
        char *ptr;
 {
        while(outbyte(*ptr++));
 }

nl()
        {outbyte(EOL);}
tab()
        {outbyte(' ');}
col()
        {outbyte(' ');}
bell()
        {outbyte(7);}

error(ptr)
char ptr[];
{       int k;
        char yunk[81];

        toconsole();
        bell();
        outstr("Line "); outdec(lineno); outstr(", ");
        if(infunc==0)
                outbyte('(');
        if(currfn==NULL)
                outstr("start of file");
        else    outstr(currfn+NAME);
        if(infunc==0)
                outbyte(')');
        outstr(" + ");
        outdec(lineno-fnstart);
        outstr(": ");  outstr(ptr);  nl();

        outstr(line); nl();

        k=0;    /* skip to error position */
        while(k<lptr){
                if(line[k++]==9)
                        tab();
                else    outbyte(' ');
                }
        outbyte('^');  nl();
        ++errcnt;

        if(errstop){
                pl("Continue (Y,n,g) ? ");
                gets(yunk);             
                k=yunk[0];
                if((k=='N') | (k=='n'))
                        abort();
                if((k=='G') | (k=='g'))
                        errstop=0;
                }
        tofile();
/* end error */}

ol(ptr)
        char ptr[];
{
        ot(ptr);
        nl();
}
ot(ptr)
        char ptr[];
{
        tab();
        outstr(ptr);
}
match(lit)
        char *lit;
{
        int k;
        blanks();
        if (k=streq(line+lptr,lit))
                {lptr=lptr+k;
                return (1);
                }
        return (0);
}
amatch(lit,len)
        char *lit;int len;
 {
        int k;
        blanks();
        if (k=astreq(line+lptr,lit,len))
                {lptr=lptr+k;
                while(an(ch())) inbyte();
                return (1);
                }
        return (0);
 }
blanks()
        {while(1)
                {while(ch()==0)
                        {inline();
                        preprocess();
                        if(eof)break;
                        }
                if(ch()==' ')gch();
                else if(ch()==9)gch();
                else return;
                }
        }
outdec(numb)
        int numb;
 {
        int k,zs;
        char c;
        zs = 0;
        k=10000;
        if (numb<0)
                {numb=(-numb);
                outbyte('-');
                }
        while (k>=1)
                {
                c=numb/k + '0';
                if ((c!='0')|(k==1)|(zs))
                        {zs=1;outbyte(c);}
                numb=numb%k;
                k=k/10;
                }
 }

/*      >>>>>>> start of cc5 <<<<<<<    */

expression()
{
        int lval[2];
        if(heir1(lval))rvalue(lval);
}
heir1(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir2(lval);
        if (match("="))
                {if(k==0){needlval();return (0);}
                if (lval[1])zpush();
                if(heir1(lval2))rvalue(lval2);
                store(lval);
                return (0);
                }
        else return (k);
}
heir2(lval)
        int lval[];
{       int k,lval2[2];
        k=heir3(lval);
        blanks();
        if(ch()!='|')return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("|"))
                        {zpush();
                        if(heir3(lval2)) rvalue(lval2);
                        zpop();
                        zor();
                        }
                else return (0);
                }
}
heir3(lval)
        int lval[];
{       int k,lval2[2];
        k=heir4(lval);
        blanks();
        if(ch()!='^')return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("^"))
                        {zpush();
                        if(heir4(lval2))rvalue(lval2);
                        zpop();
                        zxor();
                        }
                else return (0);
                }
}
heir4(lval)
        int lval[];
{       int k,lval2[2];
        k=heir5(lval);
        blanks();
        if(ch()!='&')return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("&"))
                        {zpush();
                        if(heir5(lval2))rvalue(lval2);
                        zpop();
                        zand();
                        }
                else return (0);
                }
}
heir5(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir6(lval);
        blanks();
        if((streq(line+lptr,"==")==0)&
                (streq(line+lptr,"!=")==0))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("=="))
                        {zpush();
                        if(heir6(lval2))rvalue(lval2);
                        zpop();
                        zeq();
                        }
                else if (match("!="))
                        {zpush();
                        if(heir6(lval2))rvalue(lval2);
                        zpop();
                        zne();
                        }
                else return (0);
                }
}
heir6(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir7(lval);
        blanks();
        if((streq(line+lptr,"<")==0)&
                (streq(line+lptr,">")==0)&
                (streq(line+lptr,"<=")==0)&
                (streq(line+lptr,">=")==0))return (k);
                if(streq(line+lptr,">>"))return (k);
                if(streq(line+lptr,"<<"))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("<="))
                        {zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {ule();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {ule();
                                continue;
                                }
                        zle();
                        }
                else if (match(">="))
                        {zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {uge();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {uge();
                                continue;
                                }
                        zge();
                        }
                else if((streq(line+lptr,"<"))&
                        (streq(line+lptr,"<<")==0))
                        {inbyte();
                        zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {ult();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {ult();
                                continue;
                                }
                        zlt();
                        }
                else if((streq(line+lptr,">"))&
                        (streq(line+lptr,">>")==0))
                        {inbyte();
                        zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {ugt();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {ugt();
                                continue;
                                }
                        zgt();
                        }
                else return (0);
                }
}
/*      >>>>>> start of cc6 <<<<<<      */

heir7(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir8(lval);
        blanks();
        if((streq(line+lptr,">>")==0)&
                (streq(line+lptr,"<<")==0))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match(">>"))
                        {zpush();
                        if(heir8(lval2))rvalue(lval2);
                        zpop();
                        asr();
                        }
                else if (match("<<"))
                        {zpush();
                        if(heir8(lval2))rvalue(lval2);
                        zpop();
                        asl();
                        }
                else return (0);
                }
}
heir8(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir9(lval);
        blanks();
        if((ch()!='+')&(ch()!='-'))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("+"))
                        {zpush();
                        if(heir9(lval2))rvalue(lval2);
                        if(cptr=lval[0])
                                if((cptr[IDENT]==POINTER)&
                                (cptr[TYPE]==CINT))
                                doublereg();
                        zpop();
                        zadd();
                        }
                else if (match("-"))
                        {zpush();
                        if(heir9(lval2))rvalue(lval2);
                        if(cptr=lval[0])
                                if((cptr[IDENT]==POINTER)&
                                (cptr[TYPE]==CINT))
                                doublereg();
                        zpop();
                        zsub();
                        }
                else return (0);
                }
}
heir9(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir10(lval);
        blanks();
        if((ch()!='*')&(ch()!='/')&
                (ch()!='%'))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("*"))
                        {zpush();
                        if(heir9(lval2))rvalue(lval2);
                        zpop();
                        mult();
                        }
                else if (match("/"))
                        {zpush();
                        if(heir10(lval2))rvalue(lval2);
                        zpop();
                        div();
                        }
                else if (match("%"))
                        {zpush();
                        if(heir10(lval2))rvalue(lval2);
                        zpop();
                        zmod();
                        }
                else return (0);
                }
}
heir10(lval)
        int lval[];
{
        int k;
        char *ptr;
        if(match("++"))
                {if((k=heir10(lval))==0)
                        {needlval();
                        return (0);
                        }
                if(lval[1])zpush();
                rvalue(lval);
                inc();
                ptr=lval[0];
                if((ptr[IDENT]==POINTER)&
                        (ptr[TYPE]==CINT))
                                inc();
                store(lval);
                return (0);
                }
        else if(match("--"))
                {if((k=heir10(lval))==0)
                        {needlval();
                        return (0);
                        }
                if(lval[1])zpush();
                rvalue(lval);
                dec();
                ptr=lval[0];
                if((ptr[IDENT]==POINTER)&
                        (ptr[TYPE]==CINT))
                                dec();
                store(lval);
                return (0);
                }
        else if (match("-"))
                {k=heir10(lval);
                if (k) rvalue(lval);
                neg();
                return (0);
                }
        else if(match("*"))
                {k=heir10(lval);
                if(k)rvalue(lval);
                lval[1]=CINT;
                if(ptr=lval[0])lval[1]=ptr[TYPE];
                lval[0]=0;
                return (1);
                }
        else if(match("&"))
                {k=heir10(lval);
                if(k==0)
                        {error("illegal address");
                        return (0);
                        }
                else if(lval[1])return (0);
                else
                        {immed();
                        outstr(ptr=lval[0]);
                        nl();
                        lval[1]=ptr[TYPE];
                        return (0);
                        }
                }
        else 
                {k=heir11(lval);
                if(match("++"))
                        {if(k==0)
                                {needlval();
                                return (0);
                                }
                        if(lval[1])zpush();
                        rvalue(lval);
                        inc();
                        ptr=lval[0];
                        if((ptr[IDENT]==POINTER)&
                                (ptr[TYPE]==CINT))
                                        inc();
                        store(lval);
                        dec();
                        if((ptr[IDENT]==POINTER)&
                                (ptr[TYPE]==CINT))
                                dec();
                        return (0);
                        }
                else if(match("--"))
                        {if(k==0)
                                {needlval();
                                return (0);
                                }
                        if(lval[1])zpush();
                        rvalue(lval);
                        dec();
                        ptr=lval[0];
                        if((ptr[IDENT]==POINTER)&
                                (ptr[TYPE]==CINT))
                                        dec();
                        store(lval);
                        inc();
                        if((ptr[IDENT]==POINTER)&
                                (ptr[TYPE]==CINT))
                                inc();
                        return (0);
                        }
                else return (k);
                }
        }
/*      >>>>>> start of cc7 <<<<<<      */

heir11(lval)
        int *lval;
{       int k;char *ptr;
        k=primary(lval);
        ptr=lval[0];
        blanks();
        if((ch()=='[')|(ch()=='('))
        while(1)
                {if(match("["))
                        {if(ptr==0)
                                {error("can't subscript");
                                junk();
                                needbrack("]");
                                return (0);
                                }
                        else if(ptr[IDENT]==POINTER)rvalue(lval);
                        else if(ptr[IDENT]!=ARRAY)
                                {error("can't subscript");
                                k=0;
                                }
                        zpush();
                        expression();
                        needbrack("]");
                        if(ptr[TYPE]==CINT)doublereg();
                        zpop();
                        zadd();
                        lval[1]=ptr[TYPE];
                        k=1;
                        }
                else if(match("("))
                        {if(ptr==0)
                                {callfunction(0);
                                }
                        else if(ptr[IDENT]!=FUNCTION)
                                {rvalue(lval);
                                callfunction(0);
                                }
                        else callfunction(ptr);
                        k=lval[0]=0;
                        }
                else return (k);
                }
        if(ptr==0)return (k);
        if(ptr[IDENT]==FUNCTION)
                {immed();
                outstr(ptr);
                nl();
                return (0);
                }
        return (k);
}
primary(lval)
        int *lval;
{       char *ptr,sname[NAMESIZE];int num[1];
        int k;
        if(match("("))
                {k=heir1(lval);
                needbrack(")");
                return (k);
                }
        if(symname(sname))
                {if(ptr=findloc(sname))
                        {getloc(ptr);
                        lval[0]=ptr;
                        lval[1]=ptr[TYPE];
                        if(ptr[IDENT]==POINTER)lval[1]=CINT;
                        if(ptr[IDENT]==ARRAY)return (0);
                                else return (1);
                        }
                if(ptr=findglb(sname))
                        if(ptr[IDENT]!=FUNCTION)
                        {lval[0]=ptr;
                        lval[1]=0;
                        if(ptr[IDENT]!=ARRAY)return (1);
                        immed();
                        outstr(ptr);nl();
                        lval[1]=ptr[TYPE];
                        return (0);
                        }
                ptr=addglb(sname,FUNCTION,CINT,0);
                lval[0]=ptr;
                lval[1]=0;
                return (0);
                }
        if(constant(num))
                return(lval[0]=lval[1]=0);
        else
                {error("invalid expression");
                immed();outdec(0);nl();
                junk();
                return (0);
                }
        }
store(lval)
        int *lval;
{       if (lval[1]==0)putmem(lval[0]);
        else putstk(lval[1]);
}
rvalue(lval)
        int *lval;
{       if((lval[0] != 0) & (lval[1] == 0))
                getmem(lval[0]);
                else indirect(lval[1]);
}
test(label)
        int label;
{
        needbrack("(");
        expression();
        needbrack(")");
        testjump(label);
}
constant(val)
        int val[];
{
        if (number(val))
                immed();
        else if (pstr(val))
                immed();
        else if (qstr(val))
                {immed();printlabel(litlab);outbyte("+ ");}
        else return(0);
        outdec(val[0]);
        nl();
          return(1);
}
number(val)
        int val[];
{
        int k, minus; char c;
        k = minus = 1;
        while(k)
                {k = 0;
                if (match("+")) k = 1;
                if (match("-")) {minus = (-minus); k = 1;}
                }
        if (numeric(ch()) == 0) return(0);
        while (numeric(ch()))
                {c = inbyte();
                k = k * 10 + (c - '0');
                }
        if (minus < 0) k = (-k);
        val[0] = k;
        return(1);
}
pstr(val)
        int val[];
{
        int k; char c;
        k = 0;
        if (match("'") == 0) return(0);
        while ((c = gch()) != 39)
                k = (k & 255) * 256 + (c & 127);
        val[0] = k;
        return(1);
}
qstr(val)
        int val[];
{
        char c;
        if (match(quote) == 0) return(0);
        val[0] = litptr;
        while (ch() != '"')
                {if (ch() == 0) break;
                if (litptr >= LITMAX)
                        {error("string space exhausted");
                        while (match(quote) == 0)
                                if (gch() == 0) break;
                        return(1);
                        }
                litq[litptr++]=gch();
                }
        gch();
        litq[litptr++]=0;
        return (1);
}
/*      >>>>>> start of cc8 <<<<<<<     */

/* Begin a comment line for the assembler */
comment()
{       outbyte('*');
}

/* Put out assembler info before any code is generated */
header()
{       comment();
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
        if(mainflg){            /* do stuff needed for first */
                ol("LIB CCC.H"); /* assembler file */

                zcall("main");  /* call the code generated by small-c */
                pseudoins(43); ol("JMP WARMS");
                }
}
/* Print any assembler stuff needed after all code */
trailer()
{       ol("END");

        nl();
        comment();
        outstr(" --- End of Compilation ---");
        nl();
}

/* Fetch a static memory cell into the primary register */
getmem(sym)
        char *sym;
{       if((sym[IDENT]!=POINTER)&(sym[TYPE]==CCHAR))
                pseudoins(3);
        else
                pseudoins(2);
        defword(); outstr(sym+NAME); nl();
        }
/* Fetch the address of the specified symbol */
/*      into the primary register */
getloc(sym)
        char *sym;
{       pseudoins(1); defword();
        outdec((sym[OFFSET]&255)*256+(sym[OFFSET+1]&255)-Zsp);
        nl();
        }
/* Store the primary register into the specified */
/*      static memory cell */
putmem(sym)
        char *sym;
{       if((sym[IDENT]!=POINTER)&(sym[TYPE]==CCHAR))
                pseudoins(7);
        else 
                pseudoins(6);
        defword();
        outstr(sym+NAME);
        nl();
        }
/* Store the specified object type in the primary register */
/*      at the address on the top of the stack */
putstk(typeobj)
        char typeobj;
{       zpop();
        if(typeobj==CCHAR) pseudoins(9);
                else pseudoins(8);
        }
/* Fetch the specified object type indirect through the */
/*      primary register into the primary register  */
indirect(typeobj)
        char typeobj;
{       if(typeobj==CCHAR) pseudoins(5);
                else pseudoins(4);
}
/* Print partial instruction to get an immediate value */
/*      into the primary register */
immed()
{       pseudoins(0); defword();
        }
/* Push the primary register onto the stack */
zpush()
{       pseudoins(10);
        Zsp=Zsp-2;
}
/* Pop the top of the stack  */
zpop()
{       Zsp=Zsp+2;
}
/* Swap the primary register and the top of the stack */
swapstk()
{       pseudoins(11);
        }
/* Call the specified subroutine name */
zcall(sname)
        char *sname;
{       pseudoins(14); defword();
        outstr(sname);
        nl();
}

/* Return from subroutine */
zret()
{       pseudoins(16);
}
/* Perform subroutine call to value on top of stack */
callstk()
{       pseudoins(15);
        Zsp=Zsp-2;
        }
/* Jump to specified internal label number */
jump(label)
        int label;
{       pseudoins(12); defword();
        printlabel(label);
        nl();
        }
/* Test the primary register and jump if false to label */
testjump(label)
        int label;
{       pseudoins(13); defword();
        printlabel(label);
        nl();
        }
/* Print a pseudo-instruction for interpreter  */
pseudoins(k)
        int k;
{       defbyte();
        outdec(k+k); nl();
        }
/* Print pseudo-op to define a byte */
defbyte()
{       ot("FCB ");
}
/*Print pseudo-op to define storage */
defstorage()
{       ot("RMB ");
}
/* Print pseudo-op to define a word */
defword()
{       ot("FDB ");
}
/* Modify the stack pointer to the new value indicated */
modstk(newsp)
        int newsp;
 {      int k;
        k=newsp-Zsp;
        if(k==0) return(newsp);
        pseudoins(17); defword();
        outdec(k); nl();
        return (newsp);
}
/* Double the primary register */
doublereg()
{       pseudoins(18);
        }
/* Add the primary and top of stack (pop) */
/*      (results in primary) */
zadd()
{       pseudoins(19);
        }
/* Subtract the primary register from the top of stack */
/*      (results in primary) */
zsub()
{       pseudoins(20);
        }
/* Multiply the primary register and top of stack */
/*      (results in primary */
mult()
{       pseudoins(21);
        }
/* Divide the top of stack by the primary register */
/*      (quotient in primary) */
div()
{       pseudoins(22);
        }
/* Compute remainder (mod) of top of stack divided */
/*      by the primary */
/*      (remainder in primary)  */
zmod()
{       pseudoins(23);
        }
/* Inclusive 'or' the primary and the top of stack */
/*      (results in primary) */
zor()
{       pseudoins(24);
        }
/* Exclusive 'or' the primary and the top of stack */
/*      (results in primary) */
zxor()
{       pseudoins(25);
        }
/* 'And' the primary and the top of stack */
/*      (results in primary) */
zand()
{       pseudoins(26);
        }
/* Arithmetic shift right the top of stack number of */
/*      times in primary (results in primary) */
asr()
{       pseudoins(27);
        }
/* Arithmetic left shift the top of stack number of */
/*      times in primary (results in primary) */
asl()
{       pseudoins(28);
        }
/* Form two's complement of primary register */
neg()
{       pseudoins(29);
        }
/* Form one's complement of primary register */
com()
{       pseudoins(30);
        }
/* Increment the primary register by one */
inc()
{       pseudoins(31);
        }
/* Decrement the primary register by one */
dec()
{       pseudoins(32);
        }

/* Following are the conditional operators */
/* They compare the top of the stack against the primary */
/* and put a literal 1 in the primary if the condition is */
/* true, otherwise they clear the primary register */

/* Test for equal */
zeq()
{       pseudoins(33);
        }
/* Test for not equal */
zne()
{       pseudoins(34);
        }
/* Test for less than (signed) */
zlt()
{       pseudoins(35);
        }
/* Test for less than or equal to (signed) */
zle()
{       pseudoins(36);
        }
/* Test for greater than (signed) */
zgt()
{       pseudoins(37);
        }
/* Test for greater than or equal to (signed) */
zge()
{       pseudoins(38);
        }
/* Test for less than (unsigned) */
ult()
{       pseudoins(39);
        }
/* Test for less than or equal to (unsigned) */
ule()
{       pseudoins(40);
        }
/* Test for greater than (unsigned) */
ugt()
{       pseudoins(41);
        }
/* Test for greater than or equal to (unsigned) */
uge()
{       pseudoins(42);
        }

#asm
*   This stuff is recoded in Assembler to speed things up
*
*       Test if given char is alpha     *
* alpha(c)
*       char c;
* {     c=c&127;
*       return(((c>='a')&(c<='z'))|
*               ((c>='A')&(c<='Z'))|
*               (c='_'));
* }
*
alpha EQU isalpha
*
*       Test if given char is numeric   *
*
* numeric(c)
*       char c;
* {     c=c&127;
*       return((c>='0')&(c<='9'));
* }
*
numeric EQU isdigit
*
*       Test if given char is alphanumeric      *
*
* an(c)
*       char c;
* {     return((alpha(c)|(numeric(c)));
* }
*
an EQU isalnum
*
*
* ch()
* {     return(line[lptr]&127); }
*
ch      FCB     86      switch to assembly
        CLR A
        LDX     #line
        LDA B   lptr+1
        STA B   chXXX+1
chXXX   LDA B   0,X     self modifying code !!!!
        AND B   #$7F
        JMP     RTSC
*
* nch()
* {     if(ch()==0)return(0);
*               else return(line[lptr+1]&127);
* }
*
nch     FCB     86
        CLR A
        LDX     #line
        LDA B   lptr+1
        STA B   nchXXX+1
        INC B
        STA B   nchYYY+1
nchXXX  LDA B   0,X     self modifying code !!!!
        AND B   #$7F
        BNE     nchYYY
        JMP     RTSC
nchYYY  LDA B   0,X     self modifying code !!!!
        AND B   #$7F
        JMP     RTSC
*
* gch()
* {     if(ch()==0)return(0);
*               else return(line[lptr++]&127);
* }
*
gch     FCB     86
        CLR A
        LDX     #line
        LDA B   lptr+1
        STA B   gchXXX+1
gchXXX  LDA B   0,X     self modifying code !!!!
        AND B   #$7F
        BEQ     gchYYY
        INC     lptr+1  bump the pointer
gchYYY  JMP     RTSC
*
*  direct output to console   *
*
* toconsole()
* {     saveout=output;
*       output=0;       }
*
toconsole FCB   86
        LDX     output
        STX     saveout
        CLR     output
        CLR     output+1
        JMP     RTSC
*
*  direct output back to file   *
*
* tofile()
* {     if(saveout)
*               output=saveout;
*       saveout=0;      }
*
tofile  FCB     86
        LDX     saveout
        BEQ     *+5
        STX     output
        CLR     saveout
        CLR     saveout+1
        JMP     RTSC
*
**************************
*  Compare two strings:  if equal return length,  else NULL
*
* streq(str1,str2)
*       char str1[], str2[];
*{      int k;
*       k=0;
*       while (str2[k])
*               {if ((str1[k])!=(str2[k])) return 0;
*               k++;
*               }
*       return k;
* }
*
streq   FCB     86      switch to assembly
        TSX             point to data
        LDX     4,X     pointer to str1
        STX     stX1    save
        CLR A           preset NULL
        CLR B
        PSH A
        TSX
        LDX     3,X     pointer to str2
*
stLooP  LDA A   0,X     get char from str2
        BEQ     stRETR  if NULL then done
        INX
        STX     stX2
        LDX     stX1    pointer to str1
        CMP A   0,X     compare chars
        BNE     stRET0  if not same return NULL
        INX
        STX     stX1
        LDX     stX2    point to str2
        ADD B   #1      increment count
        BCC     stLooP  check for overflow
        PUL A
        INC A           adjust MSB'S
        PSH A
        BRA     stLooP
*
stRETR  PUL A           return length of string
        JMP     RTSC
*
stRET0  INS             clean stack
        CLR A           return NULL
        CLR B
        JMP     RTSC    back to interpreter
*
stX1    FDB     0
stX2    FDB     0
*
*****************************************************
*   compare strings of a given length
*
* astreq(str1,str2,len)
*       char str1[], str2[]; int len;
* {
*       int k;
*       k=0;
*       while (k<len)
*               { if ((str1[k])!=(str2[k])) break;
*               if (str1[k]==0) break;
*               if (str2[k]==0) break;  /*  seems and is redundant !!!! */
*               k++;
*               }
*       return k;
* }
*
astreq  FCB     86
        TSX
        LDX     4,X     point to str2
        STX     stX2
        TSX
        LDX     6,X     point to str1
        STX     stX1
        TSX
        LDX     2,X     get len
        CLR A           preset NULL
        CLR B
        PSH A
        INX
*
astrLP  DEX
        BEQ     astrBR  end of loop ?
        STX     astrLN  save counter
        LDX     stX1
        LDA A   0,X     get char from str1
        BEQ     astrBR  NULL ?
        LDX     stX2
        CMP A   0,X     compare to str2
        BNE     astrBR  exit on inequality
        INX
        STX     stX2
        LDX     stX1
        INX
        STX     stX1
        LDX     astrLN
        ADD B   #1      bump length counter
        BCC     astrLP
        PUL A
        INC A
        PSH A
        BRA     astrLP
*
astrBR  PSH B           save length
        LDX     stX1
        LDA B   0,X     get str1 char
        JSR     alPHA
        PSH B           save result
        LDA B   0,X
        JSR     nuMERIC
        TSX
        ORA B   0,X     set an() flag
        INS             clear stack
        BNE     astrR0  if set, return NULL
        LDX     stX2
        LDA B   0,X     get str2 char
        JSR     alPHA
        PSH B
        LDA B   0,X
        JSR     nuMERIC
        TSX
        ORA B   0,X     set an() flag
        INS
        BNE     astrR0
*
        PUL B           return length count
        PUL A
        JMP     RTSC
*
astrR0  INS             clean stack
        INS
        CLR A           return NULL
        CLR B
        JMP     RTSC
*
astrLN  FDB     0
*
*
*
alPHA   CLR A
        AND B   #$7F
        CMP B   #'a
        BLT     alPHA1
        CMP B   #'z
        BLE     alYES
alPHA1  CMP B   #'A
        BLT     alPHA2
        CMP B   #'Z
        BLE     alYES
alPHA2  CMP B   #'_
        BEQ     alYES
*
alNO    CLR B
        RTS
*
alYES   LDA B   #1
        RTS
*
*
nuMERIC CLR A
        AND B   #$7F
        CMP B   #'0
        BLT     alNO
        CMP B   #'9
        BLE     alYES
        BRA     alNO
#endasm
/*     <<<<< End of Small-C compiler >>>>>      */
