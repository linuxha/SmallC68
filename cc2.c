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
