/* C Demo Program */

#include run0.c

/* Small-C Input-Output Library for sim6800 */

main() {
        nl();
        puts ("Hello World");
        nl();
}

nl() {
        putchar (13);
}

putchar(c)
         char c;
{
#asm
    FCB  86
    nop
    nop                 ;* Check to see where the char c is
    nop
    nop
    tsx
    ldaa  0,X           ;* Char goes into A
    staa  $F001		;* ACIADA
    JMP RTSC
#endasm
}

puts(s)
         char s[];
{
         int k;
         k=0;
         while(putchar(s[k++]));
}

