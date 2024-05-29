*main() {

;* ====== main()
main 
*  char c;

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB -1
*  c = 32;
Line 3, main + 2: illegal symbol name
 c = 32;
   ^
Line 3, main + 2: already defined
 c = 32;
     ^
*c

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB -1
Line 3, main + 2: missing semicolon
 c = 32;
     ^

;* imediate(0)
;* pseudoins(0)
	 FCB 0
	 FDB 32
*  putc(c);

;* getmem(2)
;* pseudoins(2)
	 FCB 4
	 FDB putc

;* zpush(10)
;* pseudoins(10)
	 FCB 20

;* getloc(1)
;* pseudoins(1)
	 FCB 2
	 FDB 65539

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
*}

;* modstk(17)
;* pseudoins(17)
	 FCB 34
	 FDB 6

;* zret(16)
;* pseudoins(16)
	 FCB 32
*putc(c)

;* ====== putc()
putc 
*char c;
*{
*#asm
ACIA    equ  0xC000
        ldab c          ;*
        stab ACIA       ;*
*}

;* zret(16)
;* pseudoins(16)
	 FCB 32
