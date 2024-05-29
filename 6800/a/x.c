xputs(s)
     char *s;
{
    int *ACIADA;
    &ACIADA = 0xF001;

    while(*s) {
        ACIADA = *s;
        s++;
    }
}

main() {
    /* char c; */
    /* int  i; */
    char *str;

    str = "A long string";

    puts(str);
    exit(0);
}
