/* Small C Demo Program */

/*
No include in simpliest Small C compiler

#include "run9.c"               /* Must use double quotes not <> * /
#include "io9.c"
*/
#include run0.c

main() {
        nl();
        /*
        xputs ("Hello World");
        nl();
        */
}

nl() {
    #asm
    ldaa  #13
    staa  $F001
    #endasm
}

#ifdef NOPE
putchar(i)
     char i;
{
    #asm
    ldaa  zREG
    staa  $F001
    #endasm
}

xnl() {
    putchar (13);
}

xputs(s)
     char *s;
{
    /* int k; */
    /* k = 0; */
    /* while(putchar(s[k++])); */
    while(putchar(s++));
}
#endif
