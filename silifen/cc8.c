/* cc8.c9 R1.1 3/7/85  */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"

void
outsnl(char *s) {
    outstr(s);
    nl();
}

/*begin a comment line for the assembler*/
void
comment() {
#if 0
    outbyte('*');
    outbyte(' ');
#else
    outstr(";* ");
#endif
}

void 
outup(char *s) {
    while(*s)
        outbyte(toupper(*s++));
}

/*print all assembler info before any code is generated*/
void
header() {
    if (glbflag) {
            comment();
            outsnl(version);
            outstr(" lib libdef\n");
        }
    else {
            outstr("/* small-C precompiled module */\n");
            outstr("#asm\n");
            comment();
            outsnl(version);
        }
}

/*print any assembler stuff needed after all code*/
void
trailer() {
    if (glbflag) {
            outstr("end\n");     /* end of uninitialised data segment */
            outstr(" end START\n"); /* xfer address is START */
        }
    else
        outstr("#endasm\n");
}

/*fetch a static memory cell into the primary register*/
void
getmem(char *sym) {
    if ( ( sym[IDENT] != POINTER ) & ( sym[TYPE] == CCHAR ) )
        {
            outstr(" GETB ");
            outsnl(sym + NAME);
        }
    else
        {
            outstr(" GETW ");
            outsnl(sym + NAME);
        }
}

/*fetch the address of the specified local into the primary register*/
void
getloc(char *sym) {
    char *temp;

    outstr(" GETLOC ");
    temp = sym + OFFSET;
    outdec( *temp - sp );
    nl();
}

/* fetch address of a global into the primary register */
void
getglb() {
    outstr(" GETGLB ");
}

/*store the primary register into the specified static memory cell*/
void
putmem(char *sym) {
    if ( ( sym[IDENT] != POINTER ) & ( sym[TYPE] == CCHAR ) )
        outstr(" PUTB ");
    else
        outstr(" PUTW ");
    outsnl(sym + NAME);
}

/*store the specified object type in the primary register at the address*/
/*on top of the stack*/
void
putstk(char typeobj) {
    if ( typeobj == CCHAR )
        outstr(" PUTSB\n");
    else
        outstr(" PUTSW\n");
    /* store value at the address pointed to by s then pull address off stack */
    sp = sp + intwidth;
}

/*fetch the specified object type indirect through the primary register*/
/*into the primary register*/
void
indirect(char typeobj) {
    if ( typeobj == CCHAR )
        outstr(" GETBI\n");
    else
        outstr(" GETWI\n");
}

/*swap primary and secondary registers*/
void
swap() {
    outstr(" SWAP\n");
}

/*print partial instruction to get partial value*/
/*into primary register*/
void
immed() {
    outstr(" IMMED ");
}

/*push the primary register onto the stack*/
void
push() {
    outstr(" PUSH\n");
    sp = sp - intwidth;
}

/*swap the primary register and the top of the stack*/
void
swapstk() {
    outstr(" SWAPS\n");
}

/*call the specified subroutine name*/
void
call(char *sname, int  argcnt) {
    outstr(" CALL ");
    outstr(sname);
    outbyte(',');
    outdec(argcnt);
    nl();
}

/*return from a subroutine*/
void
ret() {
    outstr(" RET\n");
}

/*call subroutine at address in stack*/
void
callstk(int argcnt) {
    outstr(" CALLS ");
    outdec(argcnt);
    nl();
}

/*jump to a specified internal label number*/
void
jump(int  label) {
    outstr(" GOTO ");
    printlabel(label);
    nl();
}

/*test the primary register and jump if false to label*/
void
testjump(int label) {
    outstr(" TEST ");
    printlabel(label);
    nl();
}

/* declare code segment */
void
codeseg() {
    outstr(" CSEG\n");
}

/* declare literal segment */
void
litseg() {
    if (glbflag)
        {
            outstr(" lib libload\n");  /* load runtime library code */
            outstr("etext LSEG\n"); /* label end of text segment for C */
        }
    else
        outstr(" LSEG\n");
}

void
dataseg() /* declare data segment */
{ /* label end of initialised data segment */
    if (glbflag)
        outstr("edata DSEG\n");
    else
        outstr(" DSEG\n");
}

void
extfref(char *ptr) {
    outstr(" EXTF ");
    outstr(ptr);
    outbyte(',');
    outup(ptr);
    nl();
}

void
extvref(char *ptr) {
    outstr(" EXTV ");
    outsnl(ptr);
}

void
pubref(char *ptr) {
    outstr(" PUB ");
    outsnl(ptr);
}

/*print pseudo op to define a byte*/
void
defbyte() {
    outstr(" fcb ");
}

/*print pseudo-op to define storage*/
void
defstorage(int n) {
    outstr(" rmb ");
    outdec( n );
    nl();
}

/*print pseudo-op to define a word*/
void
defword() {
    outstr(" fdb ");
}

/* increment stack pointer by n */
void
incstack( int n ) {
    outstr(" INCS ");
    outdec( n );
    nl();
    sp = sp + n;
}

/*modify stack pointer to new value indicated*/
int
modstk(int newsp) {
    int     k;
    k = newsp - sp;
    if ( k == 0 )
        return newsp;
    incstack( k );
    return newsp;
}

void
scale(int width) {
    outstr(" SCALE ");
    outdec(width);
    nl();
}

/* add the primary register to the stack and pull the stack ( results in*/
/* primary)*/
void
cadd() {
    outstr(" ADDS\n");
    sp = sp + intwidth;
}

/*subtract the primary register from the stack and pull the stack (results*/
/*in primary)*/
void
csub() {
    outstr(" SUBS\n");
    sp = sp + intwidth;
}

/*multiply the value on the stack by d and pull off( results in primary )*/
void
mult() {
    outstr(" MULS\n");
    sp = sp + intwidth;
}

/*divide the value on the stack by the value in d 
  ( quotient in primary,remainder in secondary ) */
void
myDiv() {
    outstr(" DIVS\n");
    sp = sp + intwidth;
}

/*compute the remainder ( mod ) of the value on the stack by the value in d
  (remainder in primary,quotient in secondary*/
void
mod() {
    outstr(" MODS\n");
    sp = sp + intwidth;
}

/*inclusive or the primary reg with the stack and pull the stack */
/*( results in primary )*/
void
or() {
    outstr(" ORS\n");
    sp = sp + intwidth;
}

/*exclusive or the primary reg with the stack and pull */
/*(results in primary )*/
void
xor() {
    outstr(" EORS\n");
    sp = sp + intwidth;
}

/*'and' the primary reg with the stack and pull( results in primary )*/
void
and() {
    outstr(" ANDS\n");
    sp = sp + intwidth;
}

/*arithmetic shift right the value on the stack no of times in d
  (results in primary)*/
void
casr() {
    outstr(" ASRS\n");
    sp = sp + intwidth;
}

/*arithmetic left shift the value on the stack number of times in d
  (results in primary)*/
void
casl() {
    outstr(" ASLS\n");
    sp = sp + intwidth;
}

/*form two's complement of primary register*/
void
cneg() {
    outstr(" NEGD\n");
}

/*form one's complement of primary register*/
void
ccom() {
    outstr(" COMD\n");
}

/*increment the primary register by n */
void
cinc( int n ) {
    outstr(" INCD ");
    outdec( n );
    nl();
}

/*decrement the primary register by n */
void
cdec( int n) {
    outstr(" DECD ");
    outdec( n );
    nl();
}

/*following are the conditional operators*/
/*they compare the value on the stack against the primary and put a literal*/
/*1 in the primary if the condition is true,otherwise they clear the primary*/
/*register. They are pure macro's and pull the stack themselves */

/*test for equal*/
void
eq() {
    outstr(" CCEQ\n");
    sp = sp + intwidth;
}

/*test for not equal*/
void
ne() {
    outstr(" CCNE\n");
    sp = sp + intwidth;
}

/*test for less than ( signed )*/
void
lt() {
    outstr(" CCLT\n");
    sp = sp + intwidth;
}

/*test for less than or equal to (signed)*/
void
le() {
    outstr(" CCLE\n");
    sp = sp + intwidth;
}

/*test for greater than ( signed )*/
void
gt() {
    outstr(" CCGT\n");
    sp = sp + intwidth;
}

/*test for greater than or equal to ( signed )*/
void
ge() {
    outstr(" CCGE\n");
    sp = sp + intwidth;
}

/*test for less than ( unsigned )*/
void
ult() {
    outstr(" CCULT\n");
    sp = sp + intwidth;
}

/*test for less than or equal to ( unsigned )*/
void
ule() {
    outstr(" CCULE\n");
    sp = sp + intwidth;
}

/*test for greater than ( unsigned)*/
void
ugt() {
    outstr(" CCUGT\n");
    sp = sp + intwidth;
}

/*test for greater than or equal to ( unsigned )*/
void
uge() {
    outstr(" CCUGE\n");
    sp = sp + intwidth;
}
/*                   <<<<    end of compiler >>>>                    */
