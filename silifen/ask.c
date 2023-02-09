#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"

#if 1
void
ask() {
    //return();
}
#else
/*                                                                   */
/*                   get options from user                           */
/*                                                                   */
void
ask() {
    int     k;
    int     num;

    kill();                                 /*clear input line*/
    nl();
    pl(version);                       /* version string defined above */
    nl();

    pl("Interleave source as comments (y/*n): ");
    getl(line);                             /*get answer*/
    if (toupper(*line) == 'Y')
        ctext = 1;                      /*user said yes*/
    else
        ctext = 0;

    /* check for module compilation */
    pl("Declare global variables (*y/n)     : ");
    getl(line);                             /*get answer*/
    if ( toupper(*line) == 'N' )
        glbflag = 0;                    /*user said no*/
    else
        glbflag = 1;

    /* check for 32 bit integers */
    pl("Generate 32 bit macro code (y/*n)   : ");
    getl(line);
    if ( toupper(*line) == 'Y' )
        intwidth = 4;       /* 32 bit for 68000 */
    else
        intwidth = 2;       /* 16 bit for 6809, 6801 */

    /*get first allowable number of compiler generated labels
      in case user will append modules*/
    while ( 1 ) {
        pl("Starting number for labels (*0)     : ");
        getl(line);
        if ( *line == 0 ) {
            num = 0;
            break;
        }
        if ( btoi(line,4,&num,10) )
            break;
    }

    nxtlab = num;                           /*first label = literal pool*/
    litlab = getlabel();                    /*first label = literal pool*/
    kill();                                 /*erase line*/
}
#endif

/* fix options from command line */
void
cl_ask() {
    int i, t;
    char *p;

    ctext = 0;
    i = glbflag = 1;
    intwidth = 2;       /* assume 16 bit */

    while(i<gargc) {   /*look through all args*/
        p = gargv[i];

        if (*p == '-') {  /*this is an option arg*/
            if (*++p == 0) {
                puts("\nNo options given after '-' on command line");
                exit(1);
            }

            while(*p) {
                if (*p == 'i') {                  /* interleave source */
                    ctext = 1;
                } else if (*p == 'm') {           /* compile module only */
                    glbflag = 0;
                } else if (*p == '4') {           /* wide integers */
                    intwidth = 4;
                } else if (*p == '2') {           /* narrow integers */
                    intwidth = 2;
                } else if (*p == 'd') {           /* dump stats */
                    dump = 1;
                } else if (*p == 'n') {           /* label number */
                    ++p;
                    t = -1 + btoi(p,6,&nxtlab,10);
                    p = p[t];                     /*convert into nxtlab*/
                } else if (*p == 'o') {           /* skip output file */
                    break;
                } else {
                    pl("Unknown option: -");
                    putchar(*p);
                    putchar(EOL);
                    exit(1);
                }
                ++p;
            }
        }
        ++i;
    }
    litlab = getlabel();
}
