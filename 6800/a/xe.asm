*/*  FLEX: Simply echo calling line arguments  */
*/* #include <stdio.h> */
*print(s)

;* ====== print()
print 
*char *s;
*{
*    char c;

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB -1
*    while (*s) {
cc2 

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 3

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* testjump(13)
;* pseudoins(13)
	 FCB 26
	 FDB cc3
*        if ( 0x5C == *s ) {
Line 9, print + 6: illegal symbol name
 if ( 0x5C == *s ) {
    ^

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB -2
Line 9, print + 6: missing semicolon
 if ( 0x5C == *s ) {
      ^

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 0
Line 9, print + 6: missing semicolon
 if ( 0x5C == *s ) {
       ^

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 7

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* zpop()

;* zeq()33
;* pseudoins(33)
	 FCB 66
Line 9, print + 6: missing semicolon
 if ( 0x5C == *s ) {
                 ^
Line 9, print + 6: invalid expression
 if ( 0x5C == *s ) {
                 ^

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 0
*            s = s + 1;
Line 10, print + 7: missing semicolon
 s = s + 1;
 ^

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 5

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 7

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 1

;* zpop()

;* zadd(19)
;* pseudoins(19)
	 FCB 38

;* zpop()

;* putstk(8)
;* pseudoins(8)
	 FCB 16
*            c = *s & 0x1F;
Line 11, print + 8: illegal symbol name
 c = *s & 0x1F;
   ^

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB -1
Line 11, print + 8: missing semicolon
 c = *s & 0x1F;
      ^

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 6

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 0

;* zpop()

;* zand(26)
;* pseudoins(26)
	 FCB 52
Line 11, print + 8: missing semicolon
 c = *s & 0x1F;
           ^
*            putchar(c);

;* getmem(2)
;* pseudoins(2)
	 FCB 4
	 FDB print

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 65541

;* indirect(5)
;* pseudoins(5)
	 FCB 10

;* swapstk(11)
;* pseudoins(11)
	 FCB 22

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* callstk(15)
;* pseudoins(15)
	 FCB 30

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 2
*        } else {

;* jump(15)
;* pseudoins(12)
	 FCB 24
	 FDB cc2
cc3 

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 7
Line 13, print + 10: missing semicolon
 } else {
        ^
*            putchar(*s);

;* getmem(2)
;* pseudoins(2)
	 FCB 4
	 FDB print

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 5

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* swapstk(11)
;* pseudoins(11)
	 FCB 22

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* callstk(15)
;* pseudoins(15)
	 FCB 30

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 2
*        }
*        s++;

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 7

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* inc(31)
;* pseudoins(31)
	 FCB 62

;* zpop()

;* putstk(8)
;* pseudoins(8)
	 FCB 16

;* dec(32)
;* pseudoins(32)
	 FCB 64
*    }

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 5

;* zret(16)
;* pseudoins(16)
	 FCB 32
*}
Line 19, (print) + 16: illegal function or declaration
}
^
*nl() {

;* ====== nl()
nl 
*    putchar(13);

;* getmem(2)
;* pseudoins(2)
	 FCB 4
	 FDB print

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 13

;* swapstk(11)
;* pseudoins(11)
	 FCB 22

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* callstk(15)
;* pseudoins(15)
	 FCB 30

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 2
*}

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 4

;* zret(16)
;* pseudoins(16)
	 FCB 32
*main(carg, varg)

;* ====== main()
main 
*int  carg;
*char *varg[];
*{
*    /* Let's cheat, @ thru DEL get 0110 knocked off */
*    /* so \@ (x40) becomes NULL (x00), A (x41) or a (x61) becomes ^A (x01), etc
* */
*    while (--carg > 0) {
cc4 

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 4

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* dec(32)
;* pseudoins(32)
	 FCB 64

;* zpop()

;* putstk(8)
;* pseudoins(8)
	 FCB 16

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 0

;* zpop()

;* zgt(37)
;* pseudoins(37)
	 FCB 74

;* testjump(13)
;* pseudoins(13)
	 FCB 26
	 FDB cc5
*        ++varg;

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 2

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* inc(31)
;* pseudoins(31)
	 FCB 62

;* zpop()

;* putstk(8)
;* pseudoins(8)
	 FCB 16
*        print(*varg);           /* print the 'word' */

;* getmem(2)
;* pseudoins(2)
	 FCB 4
	 FDB print

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 4

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* indirect(4)
;* pseudoins(4)
	 FCB 8

;* swapstk(11)
;* pseudoins(11)
	 FCB 22

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* callstk(15)
;* pseudoins(15)
	 FCB 30

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 2
*        putchar(' ');

;* getmem(2)
;* pseudoins(2)
	 FCB 4
	 FDB print

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 32

;* swapstk(11)
;* pseudoins(11)
	 FCB 22

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* callstk(15)
;* pseudoins(15)
	 FCB 30

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 2
*    }

;* jump(15)
;* pseudoins(12)
	 FCB 24
	 FDB cc4
cc5 

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 8
*    /* nl(); */
*}

;* zret(16)
;* pseudoins(16)
	 FCB 32
*/*
*-*- mode: c-mode; -*-
**/
