*/* C Demo Program */
*/* #include <run9.c>
*#include <io9.c> */
*#include runN.c
#include runN.c
*#asm
 NAM SMALL-C RUN PACK FOR 6809
*   LAST UPDATE   18-JAN-84
ROM EQU $8000
RAM EQU $9000
WARMS EQU $CD03
 ORG RAM
PC RMB 2 PSEUDO PROGRAM COUNTER
R1A RMB 1 WORKING 16 BIT
R1B RMB 1 --REGISTER
DFLAG RMB 1 DIVIDE ROUTINE FLAG
RAM SET *
***************************************************
 SETDP RAM/256
 ORG ROM
RUN LDS $CC2B
 LDA #RAM/256
 TFR A,DP
 LDX #CODE
 BRA NEXT2 START THE INTERPRETATION
**************************************************************
*
*  THE HEART OF THE INTERPRETER--- NEXT INSTRUCTION FETCHER.
*
BUMP2 LDX PC GET PROG COUNTER
 LEAX 2,X INCR BY 2
 BRA NEXT1 FETCH NEXT INSTRUCTION
NEXT LDX PC
NEXT1 STD R1A SAVE THE WORK
NEXT2 BRA *+3 ROOM FOR TRACE
 NOP
 LDB ,X+ GET THE PSEUDO-INSTRUCTION
 STX PC SAVE NEW PC
 LEAX JTABLE,PCR POINT TO ROUTINE
 ABX
 LDB R1B
 JMP [0,X] GO EXECUTE THE PSEUDO-INSTR.
**************************************************************
*                  THE JUMP TABLE                            *
**************************************************************
JTABLE FDB LD1IM #0
 FDB LD1SOFF #1
 FDB LD1 #2
 FDB LDB1 #3
 FDB LD1R #4
 FDB LDB1R #5
 FDB ST1 #6
 FDB STB1 #7
 FDB ST1SP #8
 FDB STB1SP #9
 FDB PUSHR1 #10
 FDB EXG1 #11
 FDB JMPL #12
 FDB BRZL #13
 FDB JSRL #14
 FDB JSRSP #15
 FDB RTSC #16
 FDB MODSP #17
 FDB DBL1 #18
 FDB ADDS #19
 FDB SUBFST #20
 FDB MUL1 #21
 FDB DIV1 #22
 FDB MOD #23
 FDB ORS #24
 FDB XORS #25
 FDB ANDS #26
 FDB ASRS #27
 FDB ASLS #28
 FDB NEGR #29
 FDB NOTR #30
 FDB INCR #31
 FDB DECR #32
 FDB ZEQ #33
 FDB ZNE #34
 FDB ZLT #35
 FDB ZLE #36
 FDB ZGT #37
 FDB ZGE #38
 FDB ULT #39
 FDB ULE #40
 FDB UGT #41
 FDB UGE #42
 FDB ASMC #43
*************************************************************
*-------------------------
* #0 LOAD REG WITH IMMED. VALUE
LD1IM LDD [PC]
 BRA BUMP2
*-------------------------
* #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF TFR S,D
 ADDD [PC]
 BRA BUMP2
*-------------------------
* #2  LOAD WORD @ ADDRESS
LD1 LDX PC
 LDD [,X] GET WORD
 JMP BUMP2
*-------------------------
* #3  LOAD BYTE @ ADDRESS
LDB1 LDX PC
 LDAB [,X] GET BYTE
 SEX SIGN EXTEND
 JMP BUMP2
*-------------------------
* #4  LOAD WORD INDIRECT (ADDR IN REG)
LD1R LDD [R1A] GET WORD
 JMP NEXT
*-------------------------
* #5  LOAD BYTE INDIRECT (ADDR IN REG)
LDB1R LDB [R1A]
 SEX
 JMP NEXT
*-------------------------
* #6  STORE WORD @ ADDRESS
ST1 STD [PC]
 JMP BUMP2
*-------------------------
* #7  STORE BYTE @ ADDRESS
STB1 STB [PC]
 JMP BUMP2
*-------------------------
* #8  STORE WORD @ ADDRESS ON STACK
ST1SP STD [,S++]
 JMP NEXT
*-------------------------
* #9  STORE BYTE @ ADDRESS ON STACK
STB1SP STB [,S++]
 JMP NEXT
*-------------------------
* #10  PUSH WORD ON STACK
PUSHR1 PSHS D
 LDX PC
 JMP NEXT2
*-------------------------
* #11  SWAP REG AND TOP OF STACK
EXG1 PULS X
 EXG D,X
 PSHS X
 LDX PC
 JMP NEXT2
*-------------------------
* #12  JUMP TO LABEL
JMPL LDX [PC]
 JMP NEXT2
*-------------------------
* #13  JUMP TO LABEL IF FALSE
BRZL ORAA R1B SET FLAGS
 BEQ JMPL IF REG=0 -- JUMP
 JMP BUMP2 ELSE, PROCEED
*-------------------------
* #14  CALL TO LABEL
JSRL LDX PC
 LEAX 2,X ADJUST RETURN ADDR
 PSHS X PUSH RETURN ADDRESS
 BRA JMPL
*-------------------------
* #15  CALL TO TOP OF STACK
JSRSP PULS X
 LDD PC GET RETURN ADDRESS
 PSHS D SAVE RETURN ADDRESS
 JMP NEXT2
*-------------------------
* #16  RETURN TO CALLER
RTSC PULS X GET ADDRESS
 JMP NEXT1
*-------------------------
* #17  MODIFY THE STACK POINTER
MODSP LDD [PC]
 LEAS D,S
 LDD R1A RESTORE REGISTER
 JMP BUMP2
*---------------------------
* #18  DOUBLE THE PRIMARY REGISTER
DBL1 ASLD
 JMP NEXT
*---------------------------
* #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS ADDD ,S++ DO THE ADD
 JMP NEXT POP & RETURN
*---------------------------
* #20  SUBTRACT REG FROM TOP OF STACK
SUBFST PULS D GET VALUE OFF STACK
 SUBD R1A SUBTRACT REGISTER
 JMP NEXT
*---------------------------
* #21  MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
MUL1 PSHS D
 LDAA #16
 PSHA SET COUNTER
 CLRA
 CLRB
M2 ROR 3,S SHIFT MULTIPLIER
 ROR 4,S
 DEC 0,S DONE ?
 BMI M4
 BCC M3
 ADDD 1,S
M3 RORA
 RORB SHIFT RESULT
 BRA M2 AND LOOP
M4 LEAS 3,S
 PULS D GET RESULT
 JMP NEXT
*-----------------------------
* #22  DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
DIV1 BSR BDIV DO THE BASIC DIVIDE
 LDAA DFLAG GET SIGN FLAG
 ANDA #1 MASK OFF BIT ZERO
 PULS D GET RESULT
 BEQ DIV1R
DIV1N BSR NEGATE NEGATE THE VALUE IN A,B
DIV1R JMP NEXT
*-----------------------------
* #23  DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
MOD BSR BDIV
 LEAS 2,S CLEAN STACK
 PSHS A TEMP SAVE
 LDA DFLAG GET SIGN FLAG
 BPL MOD1
 COMA
MOD1 ANDA #1 MASK OFF BIT 0
 PULS A
 BNE DIV1N IF BIT 0 SET, NEGATE
 JMP NEXT
*****************************************************
*   BASIC 16 BIT DIVIDE ROUTINE
* ENTER WITH: DIVIDEND ON STACK
*             DIVISOR IN A,B
* EXIT WITH:  QUOTIENT ON STACK
*             REMAINDER IN A,B
*             SIGN FLAG IN DFLAG
*
BDIV CLR DFLAG
 TSTA CHECK DIVISOR SIGN
 BPL BDIV1
 INC DFLAG ADJUST SIGN FLAG
 BSR NEGATE TAKE ABSOLUTE VALUE
BDIV1 PSHS D FORCE ON STACK
 LDAA #17 BIT COUNTER
 PSHA
 LDD 5,S CHECK SIGN
 BPL BDIV2 -- OF DIVIDEND
 COM DFLAG ADJUST FLAG
 BSR NEGATE
 STD 5,S
BDIV2 CLRA
 CLRB
* MAIN DIVIDE LOOP (UNSIGNED)
UDIV1 CMPD 1,S
 BCC UDIV3
UDIV2 CLC
 BRA UDIV4
UDIV3 SUBD 1,S
 SEC
UDIV4 ROL 6,S
 ROL 5,S
 DEC 0,S
 BEQ UDIV5
 ROLB
 ROLA
 BCC UDIV1
 BRA UDIV3
UDIV5 LEAS 3,S
 RTS
*----------------------------------------
* NEGATE THE VALUE IN A,B
NEGATE NEGA
 NEGB
 SBCA #0
 RTS
*----------------------------------
* #24  INCLUSIVE OR THE TOP OF STACK AND REG.
ORS ORA ,S+
 ORB ,S+
 JMP NEXT
*----------------------------------
* #25  EXCLUSIVE OR ......
XORS EORA ,S+
 EORB ,S+
 JMP NEXT
*----------------------------------
* #26  AND .........
ANDS ANDA ,S+
 ANDB ,S+
 JMP NEXT
*----------------------------------
* #27  ARITH. SHIFT RIGHT THE TOP OF STACK
ASRS ANDB #31 MAX REASONABLE SHIFT
 BEQ ASRS2
ASRS1 ASR 0,S
 ROR 1,S
 DECB
 BNE ASRS1
ASRS2 PULS D GET THE RESULT
 JMP NEXT
*--------------------------------
* #28  ARITH. SHIFT LEFT THE TOP OF STACK
ASLS ANDB #31
 BEQ ASRS2
ASLS1 ASL 1,S
 ROL 0,S
 DECB
 BNE ASLS1
 BRA ASRS2
*--------------------------------
* #29  NEGATE THE REGISTER
NEGR BSR NEGATE
 JMP NEXT
*--------------------------------
* #30  COMPLEMENT THE REGISTER
NOTR COMA
 COMB
 JMP NEXT
*--------------------------------
* #31  ADD 1 TO REG
INCR ADDD #1
 JMP NEXT
*--------------------------------
* #32 SUBTRACT 1 FROM REG
DECR SUBD #1
 JMP NEXT
*****************************************************
*
*   BASIC COMPARE INSTRUCTION SUBROUTINE
*   Compare the top of Stack to Register and set Condition codes
*
*  Unsigned compare, Carry set if top of stack < A,B
*
BCMP LDD 2,S GET TOP OF STACK
 SUBD R1A COMPARE
 RTS
*-------------------------------
* #33  TEST FOR EQUALITY
ZEQ BSR BCMP
 BEQ TRUE
 BRA FALSE
*-------------------------------
* #34  TEST FOR NOT-EQUAL
ZNE BSR BCMP
 BNE TRUE
 BRA FALSE
*-------------------------------
* #35  TEST FOR LESS THAN
ZLT BSR BCMP
 BLT TRUE
 BRA FALSE
*-------------------------------
* #36  TEST FOR LESS THAN OR EQUAL
ZLE BSR BCMP
 BLE TRUE
 BRA FALSE
*-------------------------------
* #37  TEST FOR GREATER THAN
ZGT BSR BCMP
 BGT TRUE
 BRA FALSE
*-------------------------------
* #38  TEST FOR GREATER THAN OR EQUAL
ZGE BSR BCMP
 BGE TRUE
 BRA FALSE
*-------------------------------
* #39 TEST FOR LESS THAN (UNSIGNED)
ULT BSR BCMP
 BCS TRUE
 BRA FALSE
*-------------------------------
* #40  TEST FOR LESS THAN OR EQUAL (UNSIGNED)
ULE BSR BCMP
 BLS TRUE
 BRA FALSE
*-------------------------------
* #41  TEST FOR GREATER THAN (UNSIGNED)
UGT BSR BCMP
 BHI TRUE
 BRA FALSE
*------------------------------
* #42  TEST FOR GREATER THAN OR EQUAL (UNSIGNED)
UGE BSR BCMP
 BCC TRUE
FALSE CLRB RETURN FALSE
 BRA TRUE1
TRUE LDAB #1 RETURN TRUE
TRUE1 CLRA
 LEAS 2,S
 JMP NEXT
*-------------------------------------
* #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC JMP [PC] EXECUTE CODE
CODE EQU *
 FCB 28
 FDB main
 FCB 86
 JMP WARMS
*/* END of run-time package */
#end include
*#include ioN.c
#include ioN.c
*/* Small-C Input-Output Library for FLEX9 */
*putchar(c)

====== putchar()putchar 
*         char c;
*{
*#asm
 FCB 86
 LDA 3,S
 CMPA #13
 BNE :1
 JSR $CD24
 BRA :2
:1 JSR $CD18
:2 LDD 2,S
 JMP RTSC
*}
	FCB 32
*getchar()

====== getchar()getchar 
*{
*#asm
 FCB 86
 JSR $CD15
 CMPA #13
 BNE :1
 JSR $CD24
 LDA #13
:1 TFR A,B
 CLRA
 JMP RTSC
*}
	FCB 32
*puts(s)

====== putchar()puts 
*         char s[];
*{
*         int k;
	FCB 34
	FDB -2
*         k=0;
	FCB 2
	FDB 65536
	FCB 20
	FCB 0
	FDB 0
	FCB 16
*         while(putchar(s[k++]));
cc2 
	FCB 4
	FDB putchar
	FCB 20
	FCB 2
	FDB 6
Line 39, putchar + 5: can't subscript
 while(putchar(s[k++]));
                 ^
	FCB 20
	FCB 2
	FDB 65540
	FCB 20
	FCB 8
	FCB 62
	FCB 16
	FCB 64
	FCB 38
	FCB 8
	FCB 22
	FCB 20
	FCB 30
	FCB 34
	FDB 2
	FCB 26
	FDB cc3
	FCB 24
	FDB cc2
cc3 
	FCB 34
	FDB 4
*}
	FCB 34
	FDB 2
	FCB 32
#end include
*main() {

====== main()main 
*        nl();
	FCB 4
	FDB nl
	FCB 20
	FCB 30
*        puts ("Hello World");
	FCB 4
	FDB putchar
	FCB 20
	FCB 0
	FDB cc1+ 0
	FCB 22
	FCB 20
	FCB 30
	FCB 34
	FDB 2
*        nl();
	FCB 4
	FDB nl
	FCB 20
	FCB 30
*}
	FCB 34
	FDB 12
	FCB 32
*nl() {

====== nl()nl 
*        putchar (13);
	FCB 4
	FDB putchar
	FCB 20
	FCB 0
	FDB 13
	FCB 22
	FCB 20
	FCB 30
	FCB 34
	FDB 2
*}
	FCB 34
	FDB 4
	FCB 32
