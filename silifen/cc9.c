/* cc9.c9 R1.1 24/4/85 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"

/*  get output file name  */
openout()
{
    char *p;
    int i;
    kill();
    if (gargc>1)            /* command line parameters in use */
        {
            i = 1;
            while(i<gargc)
                {
                    p = gargv[i];
                    if (*p == '-')
                        if (*++p == 'o')
                            {       /* output file indicator found */
                                if (*++p == 0)
                                    if (++i<gargc)
                                        p = gargv[i];
                                    else
                                        {
                                            perror("output file not specified after -o");
                                            exit(1);
                                        }
                                if ((output=fopen(p,"w"))==NULL)
                                    {
                                        fputc(EOL,stderr);
                                        fputs("Can't open output file: ",stderr);
                                        fputs(p,stderr);
                                        exit(1);    /* back to flex to try again */
                                    }
                                return; /* file now open */
                            }
                    ++i;
                }
        }
    pl("output filename: ");         /*ask*/
    getl(line);                      /*get filename*/
    if ( *line == 0 )                 /*none given...*/
        {
            pl("screen output\n");
            output = stdout;
            return;
        }
    if ( ( output = fopen(line, "w" ) ) == NULL )
        {
            error("can't open file");
            pl(line);
            output = stdout;
        }
    kill();                          /*erase line*/
}

/* get (next) input file  */
openin()
{
    char *p;
    input = 0;                      /*none to start with*/
    while ( input == 0 )            /*any above 1 allowed*/
        {
            kill();
            if (eof)
                break;
            if (gargc>1)    /* take from command line */
                {
                    while(curr_arg<gargc)
                        {
                            p = gargv[curr_arg];
                            if (*p == '-')
                                {   /* option not wanted */
                                    ++curr_arg;
                                    if (*++p == 'o')
                                        {/* output file indicator found */
                                            if (*++p == 0) /* -o file */
                                                ++curr_arg;
                                        }
                                    continue;
                                }
                            if ((input=fopen(p,"r"))==NULL)
                                {
                                    fputc(EOL,stderr);
                                    fputs("Can't open input file: ",stderr);
                                    fputs(p,stderr);
                                    ++curr_arg;
                                    continue;
                                }
                            strncpy(infnam,p,FNAMSIZ); /* remember name */
                            pl(infnam);   /* tell user current file */
                            if (output==stdout) nl();
                            ++curr_arg;     /* move on for next file */
                            infline = 0;    /* zero line number */
                            return; /* file now open */
                        } /* above temp fix for codegen error */
                    eof = 1;
                    break;      /* no more input files */
                }
            else
                {   /* get files interactively */
                    pl("input filename: ");
                    getl(line);             /*get name*/
                    if ( *line == 0 )
                        {
                            eof = 1;        /*none given*/
                            break;
                        }
                    if (( input = fopen( line,"r")) == NULL )
                        pl("can't open file");
                    else
                        {
                            strncpy(infnam,line,FNAMSIZ);
                            infline = 0;
                        }
                }
        }
    kill();                         /*erase line*/
}

/* open an include file */
doinclude() {
    char buffer[LINESIZE];           /* buffer for include file name*/
    char *pointer;                   /* pointer to buffer*/

    blanks();                        /* skip over to name*/

    if ( input2 != 0 ) {
        error("can't nest include files");
    } else {
        pointer = buffer;            /* reset buffer pointer*/
        while ( gch() != '"' ) {     /* look for first ocurrence of '"'*/
            if ( ch == 0 ) {         /* eol no file*/
                error("bad include file specification");
                return;              /* give up*/
            }
        }

        while (( *pointer++ = gch() ) != '"' ) {
            /*strip off file name*/
            if ( ch == 0 ) {         /*eol*/
                error("bad include file specification");
                return;              /*give up*/
            }
        }

        *--pointer = 0;              /*append a null*/

        if ( ( input2 = fopen(buffer , "r" )) == NULL ) {
            error("can't open include file");
        } else {
            strncpy(incfnam,buffer,FNAMSIZ);
            incfline = 0;
        }
    }

    kill();                        /*clear rest of line so that */
    /*next read will come from new */
    /*file (if open)*/
}

/* close the output file  */
closeout()
{
    if ( output)                     /*if there is an output file*/
        fclose( output );        /*close it*/
    output = stdout;
}

closein(unit)
/**this function closes the input file identified by "unit"
   unit is a copy of input or input2 */
     FILE *unit;               /*indicates which file is to be closed*/
{
    if ( unit )       /*if input is open*/
        {
            fclose( unit );
            if ( unit == input )
                input = 0;      /*mark as closed*/
            else
                input2 = 0;
        }
}

#if 0
getl(line)
/*copy a line from stdin to line*/
/* note that line is 132 chars long and that FLEX line buf is 128 so ok */
     char    *line;
{
    gets(line);
}
#else
void
getl(char *buf) {
    // Really should put the null at the end of the buf
    if (fgets(buf, sizeof(buf), stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    }
}
#endif
