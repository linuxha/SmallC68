#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "smallc.h"

#ifndef __linux
/*                                      */
/*      Get options from user           */
/*                                      */
void
ask() {
    int k,num[1];
    kill();                 /* clear input line */

    outbyte(CLS);           /* clear the screen */
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
    myGets(line);           /* get answer */
    ctext=0;                /* assume no */
    if((ch()=='Y') || (ch()=='y'))
        ctext=1;        /* user said yes */

    /* see if the user is compiling everything at once */
    /*      (as is usually the case)                   */
    pl("Are you compiling the whole program at once (Y,n) ? ");
    myGets(line);
    if((ch()!='N') && (ch()!='n')) {   /* single file - assume... */
        glbflag=1;      /* define globals */
        mainflg=1;      /* first file to assembler */
        nxtlab =0;      /* start numbers at lowest possible */
    } else {          /* one of many - ask everything */
        /* see if user wants us to allocate static */
        /*  variables by name in this module    */
        /*      (pseudo external capability)    */
        pl("Do you want the globals to be defined (y,N) ? ");
        myGets(line);
        glbflag=0;
        if((ch()=='Y') || (ch()=='y'))
            glbflag=1;      /* user said yes */
        /* see if we should put out the stuff   */
        /*      needed for the first assembler  */
        /*      file.                           */
        pl("Is the output file the first one the assembler ");
        pl("will see (y,N) ? ");
        myGets(line);
        mainflg=0;
        if((ch()=='Y') || (ch()=='y'))
            mainflg=1;      /* indeed it is */
        /* get first allowable number for compiler-generated */
        /*      labels (in case user will append modules) */
        while(1) {
            pl("Starting number for labels (0) ? ");
            myGets(line);
            if(ch()==0){num[0]=0;break;}
            if(k=number(num))break;
        }
        nxtlab=num[0];
    }

    /* see if user wants to be sure to see all errors */
    pl("Should I pause after an error (y,N) ? ");
    myGets(line);
    errstop=0;
    if((ch()=='Y') || (ch()=='y'))
        errstop=1;

    litlab=getlabel();      /* first label=literal pool */ 
    kill();                 /* erase line */
}

/*                                      */
/*      Get output filename             */
/*                                      */
void
openout() {
    kill();                              /* erase line */
    output=0;                            /* start with none */
    pl("Output filename? ");             /* ask...*/
    myGets(line);                        /* get a filename */
    if(ch()==0) return;                  /* none given... */
    if((output=fopen((char *) line,"w"))==NULL) { /* if given, open */
        output=0;                        /* can't open */
        printf("Opps %x\n", output);
        error("Open failure!");
    }
    printf("0x%x\n", output);
    kill();                              /* erase line */
}

/*                                      */
/*      Get (next) input file           */
/*                                      */
void
openin() {
    input=0;                    /* none to start with */

    while(input==0) {           /* any above 1 allowed */
        kill();                 /* clear line */
        if(eof) break;          /* if user said none */
        pl("Input filename? ");
        myGets(line);           /* get a name */
        if(ch() == 0) {
            eof=1;
            break;
        }                       /* none given... */

        if((input=fopen(line,"r"))!=NULL) {
            newfile();
        } else {
            input = stdin;      /* can't open it */
            pl("An Open failure");
        }
    } 
    kill();                     /* erase line */
}
#else
// Flex version

/*                                      */
/*      Get options from user           */
/*                                      */
/*
1. Output intro ( -q to quiet)
2. Whole program or individuals (-c) (<nom>.p)
3. Do you want globals to be defined?
4. Is this main()?
5. Starting number for labels?
6. ask for output file
7. ask for input file
*/
void
ask() {
    ctext=1;                /* user said yes */

    glbflag=1;      /* define globals */
    mainflg=1;      /* first file to assembler */
    nxtlab =0;      /* start numbers at lowest possible */

    /* see if user wants to be sure to see all errors */
    errstop=0;

    litlab = getlabel();    /* first label=literal pool */ 
}

/*                                      */
/*      Get output filename             */
/*                                      */
void
openout() {
    return ;
}

/*                                      */
/*      Get (next) input file           */
/*                                      */
void
openin(char *filename) {
    exit(0);
    /*
    if(filename != NULL) {
        if((input=fopen(filename,"r"))!=NULL) {
            newfile();
        } else {
            fprintf(stderr, "File not found: %s\n", filename);
            exit(2);
        }
    }
    */
}

#endif
