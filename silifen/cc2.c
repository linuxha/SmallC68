/* cc2.c9 R1.1 3/7/85 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"

/*                                                                   */
/*invoked when declared variable is followed by a "[". This routine  */
/*makes subscript the absolute size of the array                     */
/*                                                                   */
int
needsub() {
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
void
newfunc() {
    char    n[NAMESIZE];
    char    *ptr;
    int     *temp;

    if ( symname(n) == 0 ) {
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
void
getarg(int t) {    /*t = CCHAR or CINT*/
    char    n[NAMESIZE];
    int     j,type;

    while (1) {
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
int
statement() {
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
void
ns() {
    if ( match(";") == 0 )
        error("missing semicolon");
}

/* { statement; statement; ...... } */
/*allow any number of statements to fall between {}  */
void
compound() {
    ++ncmp;                                 /*new level open*/
    while ( match("}") == 0 )
        if (eof)
            return;
        else statement();
    --ncmp;                                 /*close current level*/
}

/*   if (expr) statement; else statement; */
void
doif() {
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
void
dowhile() {
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
void
dofor() {
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
void
dodo() {
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
void
doreturn() {
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
void
dobreak() {
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
void
docont() {
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
void
doasm() {
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
