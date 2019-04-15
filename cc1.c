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
