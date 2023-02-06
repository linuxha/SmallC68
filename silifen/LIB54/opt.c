/* optimiser for small-C */
/* macro version 12/7/84 */
/* uppercase macros 22/8/84 R0.1 */
/* multiple INCS 17/1/85 R0.3 */
/* "getw" to "getw " so that it doesnt match getwi by mistake R0.4 */
/* NTEST from CCEQ, TEST etc R1.0  28/5/85 */
/* bug in CC?? TEST fixed R1.1 3/7/85 */

/* transformations :
 getloc n       =       getws n         use ldd n,s
 getwi

 getloc n       =       getbs n         use ldb n,s
 getbi                                      sex

 getloc n       =       getwsi n        use ldd [n,s]
 getwi
 getwi

 getloc n       =       getbsi n        use ldb [n,s]
 getwi                                      sex
 getbi

 getw label     =       gwwi label      use ldd [label]
 getwi

 getw label     =       gwbi label      use ldb [label]
 getbi                                      sex

 INCS n         =       INCS n+m+...    use single leas n,s
 INCS m ...

 CCEQ           =       CCEQ            use result in cc rather than cmpd
 TEST label             NTEST label
*/

#include "stdio.h"

#define LINELEN 132

char    line[LINELEN], save[LINELEN], save2[LINELEN], save3[LINELEN];
int     i;
char    *match1,*match2,*match3,*match4,*match5,*match6,*matchcc,*matchtst;

#define LENMAT1 7
#define LENMAT2 6
#define LENMAT3 6
#define LENMAT4 6
#define LENMAT5 5
#define LENMAT6 5
#define LENMATCC 3
#define LENMATTST 5

main(argc,argv)
        int      argc;
        char     *argv[];
        {
        FILE    *input,*output;
        match1 = " GETLOC";
        match2 = " GETWI";
        match3 = " GETBI";
        match4 = " GETW ";
        match5 = " PUSH";
        match6 = " INCS";
        matchcc = " CC";
        matchtst = " TEST ";
        /* open files specified on command line */
        if (argc < 2 )
            { /* not enough arguments */
            perror("Optimiser R1.1 3/7/85 useage: opt infile [outfile]");
            exit(1);
            }
        else
            {
            if ((input=fopen(argv[1],"r")) == 0)
                {
                perror("can't open input file");
                exit(1);
                }
            if (argc == 2)
                output = stdout; /* output to screen */
            else
                {
                if ((output=fopen(argv[2],"w")) == 0)
                    {
                    perror("can't open output file");
                    exit(1);
                    }
                }
            }
        /* process input file a line at a time */
        while( fgets(line,LINELEN,input) ) /* read till end of file */
            {
            if ( strncmp( line, match6, LENMAT6 ) == 0 )
                { /* matched an INCS */
                strcpy( save, line);
                fgets( line, LINELEN, input );
                while( strncmp( line, match6, LENMAT6 ) == 0 )
                    { /* keep accumulating INCS n's onto save */
                    save[strlen(save)-1] = NULL; /* remove return */
                    if ( line[6] == '-' )
                        strcat( save, line+6 ); /* copy number and - */
                    else
                        {
                        strcat( save, "+" );     /* need a plus */
                        strcat( save, line+6 );  /* and number */
                        }
                    fgets( line, LINELEN, input );    /* get another line */
                    }
                fputs( save, output );   /* output INCS n+m+k-j... */
                }       /* leave line for other matches */
            if( strncmp( line, match1, LENMAT1 ) == 0 )
                {       /* it matched a getloc */
                strcpy( save, line );
                fgets(line,LINELEN,input);
                if ( strncmp( line, match2, LENMAT2 ) == 0 )
                    { /* it matched getloc, getwi */
                    /* can junk line so */
                    fgets(line,LINELEN,input);
                    if ( strncmp( line, match2, LENMAT2 ) == 0 )
                        { /* matched getloc,getwi,getwi */
                        /* edit getloc -> getwsi */
                        save[4] = 'W';
                        save[5] = 'S';
                        save[6] = 'I';
                        fputs(save,output);
                        continue;
                        }
                    else if ( strncmp( line, match3, LENMAT3 ) == 0 )
                        { /* matched getloc,getwi,getbi */
                        /* edit getloc -> getbsi */
                        save[4] = 'B';
                        save[5] = 'S';
                        save[6] = 'I';
                        fputs(save,output);
                        continue;
                        }
                    else    /* no extra match */
                        {
                        /* edit saved line */
                        save[4] = 'W'; /* getloc -> getws */
                        save[5] = 'S';
                        save[6] = ' ';
                        fputs( save, output );
                        }
                    }
                else if ( strncmp( line, match3, LENMAT3 ) == 0 )
                    { /* it matched getloc, getbi */
                    /* edit saved line */
                    save[4] = 'B'; /* getloc -> getbs */
                    save[5] = 'S';
                    save[6] = ' ';
                    fputs( save ,output);
                    continue;     /* try another line */
                    }
                else                /* second line didnt match so */
                    fputs( save ,output);
                }
             else if ( strncmp( line, match4, LENMAT4 ) == 0 )
                { /* it matched a getw */
                strcpy( save, line );
                fgets(line,LINELEN,input);
                if ( strncmp( line, match2, LENMAT2 ) == 0 )
                    { /* it matched getw, getwi */
                    /* edit getw -> gwwi */
                    save[2] = 'W';
                    save[3] = 'W';
                    save[4] = 'I';
                    fputs(save,output);
                    continue;
                    }
                if ( strncmp( line, match3, LENMAT3 ) == 0 )
                    { /* matched getw, getbi */
                    /* edit getw -> gwbi */
                    save[2] = 'W';
                    save[3] = 'B';
                    save[4] = 'I';
                    fputs(save,output);
                    continue;
                    }
                /* second line didnt match so */
                fputs( save ,output);
                }
            if ( strncmp( line, matchcc, LENMATCC ) == 0 )
                {       /* matched " CC??" */
                fputs( line, output );  /* keep CC?? */
                fgets( line, LINELEN, input ); /* get next line */
                if ( strncmp( line, matchtst, LENMATTST ) == 0 )
                    { /* matched TEST */
                    line[1] = 'N'; /* TEST to NTST */
                    line[2] = 'T';
                    }
                fputs( line, output );  /* save result, modified or not */
                }
            else
                fputs( line ,output);   /* no matches at all */
            }
        }
