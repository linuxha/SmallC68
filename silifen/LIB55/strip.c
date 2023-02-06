/* strip.c, R0.1 strip out spaces */

#include "stdio.h"

main(argc,argv)
        int     argc;
        char    *argv[];
        {
        FILE *input,*output;
        int  c,nospace;
        if (argc<3)
                {
                perror("useage: strip infile outfile");
                exit(1);
                }
        if ((input=fopen(argv[1],"r"))==0)
                {
                perror("input file not found");
                exit(1);
                }
        if ((output=fopen(argv[2],"w"))==0)
                {
                perror("output file not found");
                exit(1);
                }
        nospace = YES;
        while((c=fgetc(input))!=EOF)
                {
                if (c==' ')
                        {
                        if (nospace)
                                {
                                fputc(c,output);
                                nospace = NO;
                                }
                        }
                else
                        {
                        nospace = YES;
                        fputc(c,output);
                        }
                }
        fclose(input);
        fclose(output);
        }

