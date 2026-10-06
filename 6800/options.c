/*
**
** Author:	Neil Cherry
** Date:	2023/02/03
** Version:	1.1 (sub alpha)
**
**
*/

#include <getopt.h>             /* as referenced in the getop man page */
#include <stdlib.h>             /* used by atoi */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "smallc.h"

extern void openFile(char *filename);

void
clihelp() {
    fprintf(stderr,"smallc [-i interp.asm] filename.c\n");
    fprintf(stderr,"  -i, --interp file   include file instead of interp.asm\n");
    fprintf(stderr,"  -h, -?, --help      this help\n");
}

void
options(int argc, char **argv) {
    int c, option_index = 0;

    static struct option long_options[] = {
        {"help",     0, 0, '?'},
        {"help",     0, 0, 'h'},
        {"interp",   1, 0, 'i'},
        {0,          0, 0, 0}
    };

    ctext=1;                /* user said yes */

    glbflag=1;      /* define globals */
    mainflg=1;      /* first file to assembler */
    nxtlab =0;      /* start numbers at lowest possible */

    /* see if user wants to be sure to see all errors */
    errstop=0;

    litlab = getlabel();    /* first label=literal pool */ 

    /*
    ** handle options here before we fork
    **
    ** The following options should be available:
    ** --help -h -?  Issues help
    ** --interp -i   Use a different interpreter file (default interp.asm)
    */

    while(1) {                    /* Forever loop */
        c = getopt_long (argc, argv, "?hi:", long_options, &option_index);
        if(c == (-1)) {
            if(optind >= argc) {
                fprintf(stderr,"No input file given\n");
                clihelp();
                exit(1);
            }
            openFile(argv[optind]);
            return;
        }

        switch(c) {
            case 'i':                 /* Interpreter file */
                interpfile = optarg;
                break;

            default:                  /* Bad option */
                fprintf(stderr,"Bad option '%c'\n",optopt);
            case 'h':                 /* Help screen */
            case '?':
                clihelp();
                exit(1);
                break;
        }
    }
}

/*                                      */
/*      Get (next) input file           */
/*                                      */
void
openFile(char *filename) {
    //fprintf("* filename '%s'\n)
    if(!filename) return;

    // input is global
    if((input = fopen(filename,"r")) == NULL) {
        fprintf(stderr, "X File not found: %s\n", filename);
        exit(2);
    } else {
        newfile();
    }
}
