#ifndef SCC_H
#define SCC_H
/*   now reserve some storage words  */

char  symtab[SYMTBSZ];          /*symbol table*/
char  *glbptr;
char  *locptr;                  /*pointers to next entry*/
long  wq[WQTABSZ];              /*while queue*/
long  *wqptr;                   /*pointer to next entry*/
char  litq[LITABSZ];            /*literal pool*/
long  litptr;                   /*pointer to next entry*/
char  macq[MACQSIZE];           /*macro string buffer*/
long  macptr;                   /*and its index*/
char  line[LINESIZE];           /*parsing buffer*/
char  mline[LINESIZE];          /*temp macro buffer*/
long  iflevel;                  /* #if.. nest level */
long  skiplevel;                /* level at which #if.. skipping started */
long  lptr;
long  mptr;                     /*pointers into each*/

/*   misc storage    */

char  *version;                 /*pointer to version string */
int   intwidth;                 /*integer width in bytes 2 or 4*/
int   nxtlab;                   /*next available label*/
int   litlab;                   /*label # assigned by literal pool*/
int   sp;                       /*compiler relative stack pointer*/
int   argstk;                   /*function arg sp*/
int   ncmp;                     /*# open compound statements*/
int   errcnt;                   /*# errors in compilation*/
int   eof;                      /*set non zero on final input eof*/
int   glbflag;                  /*non-zero if internal globals*/
int   ctext;                    /*non-zero to intermix c-source*/
int   cmode;
/*non-zero while parsing  c-code zero when parsing assembly code*/
int   lastst;                   /*last executed statement type*/
char  quote[2];                 /*literal string for '"' */
char  *cptr;                    /*work pointer to any char buffer*/
long  *iptr;                    /*work pointer to any int buffer*/
int   ch,nch;                   /*current and next characters as ints*/
char  infnam[FNAMSIZ];          /*buffer for current filename*/
char  incfnam[FNAMSIZ];         /*buffer for include filename*/
int   infline;                  /*line number for input file*/
int   incfline;                 /*line number for include file*/
int   curr_arg;                 /*current place in argument list*/
int   dump;                     /*dump statistics flag*/

int   gargc;                    /*global copy of arg count*/
char  **gargv;                  /*pointer to argument array*/

FILE  *output;                  /*output fcb pointer*/
FILE  *input;                   /*input file fcb pointer*/
FILE  *input2;                  /*include file fcb pointer */
#endif  /* SCC_H */

