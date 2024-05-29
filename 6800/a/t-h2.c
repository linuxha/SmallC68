/* Small C Demo Program */

#include run0.c /* This is a very broken compiler */

main() {
        nl();
}

nl() {
    #asm
    FCB   86
    nop
    ldaa  #13		;* NL
    staa  $F001		;* ACIADA
    jmp   RTSC
    #endasm
}

putchar(c)
     char c;
{
    #asm
    FCB   86
    nop
    nop
    nop
    nop
    tsx
    ldaa  0,X
    ; Char goes into A
    staa  $F001		;* ACIADA
    jmp   RTSC
    #endasm
}
