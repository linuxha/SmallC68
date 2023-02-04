#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "smallc.h"

/* Print a carriage return and a string only to console */
void
pl(char *str) {
    int k;

    k=0;
    putchar(EOL);
    while(str[k]) putchar(str[k++]);
    fflush(stderr);
    fflush(stdout);
}

long
outbyte(char c) {
    if(c==0) return (0);
    if(output) {
        int i = (int) c;
        if((fputc(i, output)) == EOF) {
            closeout();
            error("Output file error");
            myAbort();
        }
    }
    else putchar(c);
    /*
    fflush(output);
    fflush(stderr);
    fflush(stdout);
    */
    return (c);
}

/*  direct output to console   */
void
toconsole() {
    saveout=output;
    output=stdout;
}

/*  direct output back to file   */
void
tofile() {
    if(saveout)
        output=saveout;
    saveout=stdout;
}

/*                                      */
/*      Close the output file           */
/*                                      */
void
closeout() {
    tofile();                   /* if diverted, return to file */
    if(output) fclose(output);  /* if open, close it */
    output=0;                   /* mark as closed */
}

long
myFclose(FILE *fp) {
    return(fclose(fp));
}
