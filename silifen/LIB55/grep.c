/* grep.c - print lines from a file that contain a pattern */

#include "stdio.h"

char buf[132], *str, *p;

main(argc,argv)
        int  argc;
        char *argv[];
        {
        FILE *input,*output;
        if (argc<3) 
                perror("useage: grep pattern infile [outfile]");
        else
                {
                if (argc > 3)
                        {
                        if ((output=fopen(argv[3],"w"))==0)
                                {
                                perror("can't open output file");
                                exit(1);
                                }
                        }
                else
                        {
                        output = stdout;
                        putchar(EOL);
                        }
                if ((input=fopen(argv[2],"r"))==0)
                        {
                        perror("can't open input file");
                        exit(1);
                        }
                p = argv[1];
                while(fgets(buf,132,input))
                        {
                        str = buf;
                        while(str=index(str,*p))
                                {
                                if (strncmp(str,p,strlen(p))==0)
                                        fputs(buf,output);
                                ++str;
                                }
                        }
                }
        fclose(input);
        if (output != stdout)
                fclose(output);
        }

