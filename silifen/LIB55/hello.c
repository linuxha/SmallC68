
#include "stdio.h"

main(argc,argv)
        int argc;
        char *argv[];
        {
        if (argc==1) printf("\nfilename %s\n",*argv);
        if (argc==2) printf(argv[1],100,0x6ABC,'c',"string",-32000);
        }

