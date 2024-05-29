main() {
  char c;
  c = 32;
  putc(c);
}

putc(c)
char c;
{
#asm
ACIA    equ  0xC000

        ldab c          ;*
        stab ACIA       ;*
#endasm
}
