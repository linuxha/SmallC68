/*  FLEX: Simply echo calling line arguments  */
/* #include <stdio.h> */
print(s)
char *s;
{
    char c;

    while (*s) {
        if ( 0x5C == *s ) {
            s = s + 1;
            c = *s & 0x1F;
            putchar(c);
        } else {
            putchar(*s);
        }

        s++;
    }
}

nl() {
    putchar(13);
}

main(carg, varg)
int  carg;
char *varg[];
{
    /* Let's cheat, @ thru DEL get 0110 knocked off */
    /* so \@ (x40) becomes NULL (x00), A (x41) or a (x61) becomes ^A (x01), etc. */
    while (--carg > 0) {
        ++varg;
        print(*varg);           /* print the 'word' */
        putchar(' ');
    }

    /* nl(); */
}

/*
-*- mode: c-mode; -*-
*/
