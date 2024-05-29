#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdint.h>

int sub(int c) {
    return(isalpha(c));
}

void
main() {
    int c = 'a', c1 = 0;

    printf("C  is = 0x%x\n", isalpha(c));
    printf("C1 is = 0x%x\n", isalpha(c1));
    if(sub(c)) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    if(sub(c1)) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    printf("sizeof(Char *) = %d\n", sizeof(char *));
    printf("sizeof(Int)    = %d\n", sizeof(int));
    printf("sizeof(Int *)  = %d\n", sizeof(int *));
    printf("sizeof(long)   = %d\n", sizeof(long));
    printf("sizeof(long *) = %d\n", sizeof(long *));

    printf("sizeof(uint8_t)    = %d\n", sizeof(uint8_t));
    printf("sizeof(uint8_t *)  = %d\n", sizeof(uint8_t *));
    printf("sizeof(uint16_t)   = %d\n", sizeof(uint16_t));
    printf("sizeof(uint16_t *) = %d\n", sizeof(uint16_t *));
    printf("sizeof(uint32_t)   = %d\n", sizeof(uint32_t));
    printf("sizeof(uint32_t *) = %d\n", sizeof(uint32_t *));
    printf("sizeof(uint64_t)   = %d\n", sizeof(uint64_t));
    printf("sizeof(uint64_t *) = %d\n", sizeof(uint64_t *));
}
