*[ Start ]**********************************************************************
;* This is 6800 code
	ORG  0 		;* 
	NAM  SMALL-C	;* INTERPRETER
	OPT  NOP,NOG	;* 
*
*   LAST UPDATE   9-SEP-82
*
	JMP  BEGIN	;* START THE INTERPRETER

*  AN INDIRECT CALL TABLE
	NOP		;* PUT ON A BOUNDARY OF 4
	FCB  86 	;* 
	JMP  fclose+1	;* 
	FCB  86 	;* 
	JMP  fopen+1	;* 
	FCB  86 	;* 
	JMP  getc+1	;* 
	FCB  86 	;* 
	JMP  getchar+1	;* 
	FCB  86 	;* 
	JMP  gets+1	;* 
	FCB  86 	;* 
	JMP  putc+1	;* 
	FCB  86 	;* 
	JMP  putchar+1	;* 
	FCB  86 	;* 
	JMP  puts+1	;* 
	NOP		;* 
	JMP  RTSC	;* 
	FCB  86 	;* 
	JMP  isalpha+1	;* 
	FCB  86 	;* 
	JMP  isdigit+1	;* 
	FCB  86 	;* 
	JMP  isalnum+1	;* 
	FCB  86 	;* 
	JMP  islower+1	;* 
	FCB  86 	;* 
	JMP  isupper+1	;* 
	FCB  86 	;* 
	JMP  isspace+1	;* 
	FCB  86 	;* 
	JMP  toupper+1	;* 
	FCB  86 	;* 
	JMP  tolower+1	;* 
	FCB  86 	;* 
	JMP  strclr+1	;* 
	FCB  86 	;* 
	JMP  strlen+1	;* 
	FCB  86 	;* 
	JMP  strcpy+1	;* 
	FCB  86 	;* 
	JMP  strcat+1	;* 
	FCB  86 	;* 
	JMP  strcmp+1	;* 
	RMB  4*4        ;* ROOM FOR 4 MORE

	LIB  FLEXPTRS	;* 

NFILES	EQU  4  	;* MAX NO OF DISK FILES OPEN AT ONCE
PC	RMB  2  	;* PSEUDO PROGRAM COUNTER
R1A	RMB  1  	;* WORKING 16 BIT
R1B	RMB  1  	;* --REGISTER
DFLAG	FCB  NFILES-1	;* DIVIDE ROUTINE FLAG
STEMP	RMB  2  	;* TEMP STORAGE FOR STACK POINTER
X1TMP	RMB  2  	;* TEMP STORAGE FOR X REG
X2TMP	RMB  2  	;* ... DITTO ...
FCBPTR	RMB  2  	;* POINTER INTO FCB TABLE
FCBTBL	FDB  FCB  	;* TABLE OF FCB POINTERS
	RMB  NFILES*2   ;* ROOM FOR THE REST



***************************************************

BEGIN	LDX  #FCBTBL+2	;* POINT TO FCB ADDRESSES XXXXXXXX
	LDA  A #NFILES-1
	STA  A DFLAG    ;* INIT COUNTER
	LDA  A $AC2B    ;* GET TOP OF MEMORY
	LDA  B $AC2C

BLOOP	SUB  B		;* #$40 SUBTR 320 (SIZE OF FCB) XXXXXXXX
	SBC  A #1
	STA  A 0,X      ;* SAVE FCB ADDRESS
	INX		;* 
	STA  B 0,X
	INX		;* 
	DEC  DFLAG	;* DONE ???
	BNE  BLOOP	;* 

	CLR  0,X 	;* MARK END OF TABLE
	CLR  1,X 	;* 
	STA  A STEMP    ;* TOP OF STACK AREA
	STA  B STEMP+1
	LDX  STEMP	;* 
	TXS		;* SET STACK POINTER

	LDX  #FCBTBL	;* POINT TO TABLE OF FCB ADDRESSES
Init	STX  FCBPTR	;*
	LDX  0,X 	;* GET FCB ADDRESS
	BEQ  Initend	;* QUIT IF END OF TABLE
	CLR  2,X 	;* MARK AS NOT IN USE
	LDX  FCBPTR	;* 
	INX		;* 
	INX		;* 
	BRA  Init	;* 

Initend	LDX  #$800	;*
	BRA  NEXT2	;* START THE INTERPRETATION

**************************************************************
*
*  THE HEART OF THE INTERPRETER--- NEXT INSTRUCTION FETCHER.
*
BUMP2	LDX  PC		;* GET PROG COUNTER XXXXXXXX
BUMP2A	INX  INCR	;* BY 2 XXXXXXXX
	INX		;* 
	BRA  NEXT1	;* FETCH NEXT INSTRUCTION

NEXT	LDX  PC  	;*
NEXT1	STA  A		;* R1A SAVE THE WORK XXXXXXXX
	STA  B R1B      ;* --REGISTER
NEXT2	LDA  B 0,X      ;* GET THE PSEUDO-INSTRUCTION XXXXXXXX
	INX		;* (B CONTAINS A TABLE OFFSET)
	STX  PC 	;* SAVE NEW PC
	STA  B JJJ+2    ;* SAVE AS PAGE OFFSET
	LDA  B R1B      ;* RESTORE
JJJ	LDX  JTABLE	;* POINT TO ROUTINE (SELF MODIFYING CODE) XXXXXXXX
	JMP  0,X 	;* GO EXECUTE THE PSEUDO-INSTR.

**************************************************************
*                  THE JUMP TABLE                            *
**************************************************************

	ORG  *+255/256*256	;* MUST START ON A PAGE BOUNDARY

JTABLE	FDB  LD1IM	;* #0
	FDB  LD1SOFF	;* #1
	FDB  LD1 	;* #2
	FDB  LDB1	;* #3
	FDB  LD1R	;* #4
	FDB  LDB1R	;* #5
	FDB  ST1 	;* #6
	FDB  STB1	;* #7
	FDB  ST1SP	;* #8
	FDB  STB1SP	;* #9
	FDB  PUSHR1	;* #10
	FDB  EXG1	;* #11
	FDB  JMPL	;* #12
	FDB  BRZL	;* #13
	FDB  JSRL	;* #14
	FDB  JSRSP	;* #15
	FDB  RTSC	;* #16
	FDB  MODSP	;* #17
	FDB  DBL1	;* #18
	FDB  ADDS	;* #19
	FDB  SUBFST	;* #20
	FDB  MUL1	;* #21
	FDB  DIV1	;* #22
	FDB  MOD 	;* #23
	FDB  ORS 	;* #24
	FDB  XORS	;* #25
	FDB  ANDS	;* #26
	FDB  ASRS	;* #27
	FDB  ASLS	;* #28
	FDB  NEGR	;* #29
	FDB  NOTR	;* #30
	FDB  INCR	;* #31
	FDB  DECR	;* #32
	FDB  ZEQ 	;* #33
	FDB  ZNE 	;* #34
	FDB  ZLT 	;* #35
	FDB  ZLE 	;* #36
	FDB  ZGT 	;* #37
	FDB  ZGE 	;* #38
	FDB  ULT 	;* #39
	FDB  ULE 	;* #40
	FDB  UGT 	;* #41
	FDB  UGE 	;* #42
	FDB  ASMC	;* #43

*************************************************************
*-------------------------
* #0 LOAD REG WITH IMMED. VALUE
LD1IM	LDX  PC  	;*
	LDA  A 0,X      ;* HIGH BYTE
	LDA  B 1,X      ;* LOW BYTE
	JMP  BUMP2A	;* 

*-------------------------
* #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF	STS  R1A	;* SAVE STACK VALUE XXXXXXXX
	LDX  PC 	;* 
	LDA  A 0,X      ;* GET OFFSET
	LDA  B 1,X      ;* -- VALUE
	SEC		;* 
	ADC  B R1B      ;* ADD OFFSET + 1
	ADC  A R1A
	JMP  BUMP2A     ;*

*-------------------------
* #2  LOAD WORD @ ADDRESS
LD1	LDX  PC  	;*
	LDX  0,X 	;* GET ADDRESS
LD1A	LDA  A		;* 0,X GET WORD XXXXXXXX
	LDA  B 1,X      ;* 
	JMP  BUMP2	;* 

*-------------------------
* #3  LOAD BYTE @ ADDRESS
LDB1	LDX  PC  	;*
	LDX  0,X 	;* GET ADDRESS
	CLR  A          ;* 
	LDA  B 0,X      ;* GET BYTE
	BPL  LDB1A	;* 
	COM  A          ;* SIGN EXTEND
LDB1A	JMP  BUMP2	;*

*-------------------------
* #4  LOAD WORD INDIRECT (ADDR IN REG)
LD1R	LDX  R1A	;* GET ADDRESS XXXXXXXX
	LDA  A 0,X      ;* GET WORD
	LDA  B 1,X      ;* 
	JMP  NEXT	;* 

*-------------------------
* #5  LOAD BYTE INDIRECT (ADDR IN REG)
LDB1R	LDX  R1A  	;*
	CLR  A          ;* 
	LDA  B 0,X      ;* GET BYTE
	BPL  LDB1RA	;* 
	COM  A          ;* 
LDB1RA	JMP  NEXT	;*

*-------------------------
* #6  STORE WORD @ ADDRESS
ST1	LDX  PC  	;*
	LDX  0,X 	;* GET ADDRESS
	STA  A 0,X      ;* STORE WORD
	STA  B 1,X      ;* 
	JMP  BUMP2	;* 

*-------------------------
* #7  STORE BYTE @ ADDRESS
STB1	LDX  PC  	;*
	LDX  0,X 	;* GET ADDR
	STA  B 0,X      ;* STORE BYTE
	JMP  BUMP2	;* 

*-------------------------
* #8  STORE WORD @ ADDRESS ON STACK
ST1SP	TSX  STACK	;* TO INDEX XXXXXXXX
	LDX  0,X 	;* GET ADDRESS
	STA  A 0,X      ;* STORE WORD
	STA  B 1,X      ;* 
	INS		;* 
	INS		;* POP STACK
	JMP  NEXT	;* 

*-------------------------
* #9  STORE BYTE @ ADDRESS ON STACK
STB1SP	TSX		;* (LABEL/NM only?)
	LDX  0,X 	;* 
	STA  B 0,X      ;* STORE BYTE
	INS		;* POP ...
	INS		;* 
	JMP  NEXT	;* 

*-------------------------
* #10  PUSH WORD ON STACK
PUSHR1	PSH  B  	;*
	PSH  A          ;* 
	LDX  PC 	;* 
	JMP  NEXT2	;* 

*-------------------------
* #11  SWAP REG AND TOP OF STACK
EXG1	TSX		;* (LABEL/NM only?)
	LDX  0,X 	;* GET VALUE ON STACK
	STX  R1A 	;* SAVE
	INS		;* 
	INS		;* 
	PSH  B          ;* 
	PSH  A          ;* REG ON STACK
	LDA  A R1A      ;* NEW REG
	LDA  B R1B      ;* 
	LDX  PC 	;* 
	JMP  NEXT2	;* 

*-------------------------
* #12  JUMP TO LABEL
JMPL	LDX  PC  	;*
JMP1	LDX  0,X	;* GET ADDRESS (NEW PC) XXXXXXXX
	JMP  NEXT2	;* 

*-------------------------
* #13  JUMP TO LABEL IF FALSE
BRZL	ORA  A		;* R1B SET FLAGS XXXXXXXX
	BEQ  JMPL	;* IF REG=0 -- JUMP
	JMP  BUMP2	;* ELSE, PROCEED

*-------------------------
* #14  CALL TO LABEL
JSRL	LDX  PC  	;*
	INX		;* ADJUST RETURN
	INX		;* -- ADDRESS
	DES		;* 
	STS  *+5 	;* *** SELF MODIFYING CODE ***
	DES		;* 
	STX  $FFFF	;* PUSH RETURN ADDRESS
	BRA  JMPL	;* 

*-------------------------
* #15  CALL TO TOP OF STACK
JSRSP	TSX  POINT	;* TO STACK XXXXXXXX
	LDX  0,X 	;* GET ADDRESS (NEW PC)
	INS		;* POP
	INS		;* 
	LDA  B PC+1     ;* GET RETURN ADDRESS
	PSH  B          ;* 
	LDA  B PC       ;* 
	PSH  B          ;* SAVE RETURN ADDRESS
	JMP  NEXT2	;* 

*-------------------------
* #16  RETURN TO CALLER
RTSC	TSX		;* (LABEL/NM only?)
	LDX  0,X 	;* GET ADDRESS
	INS		;* POP
	INS		;* 
	JMP  NEXT1	;* 

*-------------------------
* #17  MODIFY THE STACK POINTER
MODSP	LDX  PC  	;*
	LDA  A 0,X      ;* GET VALUE
	LDA  B 	;* 1,X 
	STS  STEMP	;* 
	ADD  B STEMP+1  ;* ADD STACK POINTER
	ADC  A STEMP    ;* 
	STA  A STEMP    ;* 
	STA  B STEMP+1  ;* 
	LDS  STEMP	;* NEW STACK POINTER
	LDA  A R1A      ;* RESTORE REGISTER
	LDA  B R1B      ;* 
	JMP  BUMP2A	;* 

*---------------------------
* #18  DOUBLE THE PRIMARY REGISTER
DBL1	ASL  B  	;*
	ROL  A          ;* 
	JMP  NEXT	;* 

*---------------------------
* #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS	TSX		;* (LABEL/NM only?)
	ADD  B 1,X      ;* DO THE ADD
	ADC  A 0,X      ;* 
	JMP  POPS	;* POP & RETURN

*---------------------------
* #20  SUBTRACT REG FROM TOP OF STACK
SUBFST	PUL  A		;* GET VALUE OFF STACK XXXXXXXX
	PUL  B          ;* 
	SUB  B R1B      ;* SUBTRACT REGISTER
	SBC  A R1A      ;* 
	JMP  NEXT	;* 

*---------------------------
* #21  MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
MUL1	PSH  B  	;*
	PSH  A          ;* REG ON STACK
	LDA  A #16      ;* 
	PSH  A          ;* SET COUNTER
	CLR  A          ;* 
	CLR  B          ;* 
	TSX		;* POINT TO DATA

M2	ROR  3,X	;* SHIFT MULTIPLIER XXXXXXXX
	ROR  4,X 	;* 
	DEC  0,X 	;* DONE ?
	BMI  M4 	;* 
	BCC  M3 	;* 
	ADD  B 2,X      ;* 
	ADC  A 1,X      ;* 

M3	ROR  A  	;*
	ROR  B          ;* SHIFT RESULT
	BRA  M2 	;* AND LOOP

M4	INS  CLEAN	;* STACK XXXXXXXX
	INS		;* 
	INS		;* 
	PUL  A          ;* GET RESULT
	PUL  B          ;* 
	JMP  NEXT	;* 

*-----------------------------
* #22  DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
DIV1	BSR  BDIV	;* DO THE BASIC DIVIDE XXXXXXXX
	LDA  A DFLAG    ;* GET SIGN FLAG
	AND  A #1       ;* MASK OFF BIT ZERO
	PUL  A          ;* GET RESULT
	PUL  B          ;* 
	BEQ  DIV1R	;* 

DIV1N	BSR  NEGATE	;* NEGATE THE VALUE IN A,B XXXXXXXX

DIV1R	JMP  NEXT	;*

*-----------------------------
* #23  DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
MOD	BSR  BDIV	;*
	INS		;* CLEAN STACK
	INS		;* 
	PSH  A          ;* TEMP SAVE
	LDA  A DFLAG    ;* GET SIGN FLAG
	BPL  MOD1	;* 
	COM  A          ;* 

MOD1	AND  A		;* #1 MASK OFF BIT 0 XXXXXXXX
	PUL  A          ;* 
	BNE  DIV1N	;* IF BIT 0 SET, NEGATE

	JMP  NEXT	;* 

*****************************************************
*   BASIC 16 BIT DIVIDE ROUTINE
* ENTER WITH: DIVIDEND ON STACK
*             DIVISOR IN A,B
* EXIT WITH:  QUOTIENT ON STACK
*             REMAINDER IN A,B
*             SIGN FLAG IN DFLAG
*
BDIV	CLR  DFLAG	;*
	TST  A          ;* CHECK DIVISOR SIGN
	BPL  BDIV1	;* 

	INC  DFLAG	;* ADJUST SIGN FLAG
	BSR  NEGATE	;* TAKE ABSOLUTE VALUE

BDIV1	PSH  B		;* FORCE ON STACK XXXXXXXX
	PSH  A          ;* 
	LDA  A #17      ;* BIT COUNTER
	PSH  A          ;* 
	TSX		;* POINT TO DATA
	LDA  A 5,X      ;* CHECK SIGN
	BPL  BDIV2	;* -- OF DIVIDEND

	COM  DFLAG	;* ADJUST FLAG
	LDA  B 6,X      ;* 
	BSR  NEGATE	;* 
	STA  A 5,X      ;* 
	STA  B 6,X      ;* 

BDIV2	CLR  A  	;*
	CLR  B          ;* 

* MAIN DIVIDE LOOP (UNSIGNED)

UDIV1	CMP  A		;* 1,X XXXXXXXX
	BHI  UDIV3	;* 
	BCS  UDIV2	;* 
	CMP  B 2,X      ;* 
	BCC  UDIV3	;* 

UDIV2	CLC		;* (LABEL/NM only?)
	BRA  UDIV4	;* 

UDIV3	SUB  B 2,X      ;* XXXXXXXX
	SBC  A 1,X      ;* 
	SEC		;* 

UDIV4	ROL  6,X  	;*
	ROL  5,X 	;* 
	DEC  0,X 	;* 
	BEQ  UDIV5	;* 

	ROL  B          ;* 
	ROL  A          ;* 
	BCC  UDIV1	;* 
	BRA  UDIV3	;* 

UDIV5	INS		;* (LABEL/NM only?)
	INS		;* 
	INS		;* 
	RTS		;* 

*----------------------------------------
* NEGATE THE VALUE IN A,B
NEGATE	COM  A  	;*
	COM  B          ;* 
	ADD  B          ;* #1
	ADC  A          ;* #0
	RTS		;* 

*----------------------------------
* #24  INCLUSIVE OR THE TOP OF STACK AND REG.
ORS	TSX		;* (LABEL/NM only?)
	ORA  A 0,X      ;* 
	ORA  B 1,X      ;* 
POPS	INS  POP	;* THE STACK XXXXXXXX
	INS		;* 
	JMP  NEXT	;* 

*----------------------------------
* #25  EXCLUSIVE OR ......
XORS	TSX		;* (LABEL/NM only?)
	EOR  A 0,X      ;* 
	EOR  B 1,X      ;* 
	BRA  POPS	;* 

*----------------------------------
* #26  AND .........
ANDS	TSX		;* (LABEL/NM only?)
	AND  A 0,X      ;* 
	AND  B 1,X      ;* 
	BRA  POPS	;* 

*----------------------------------
* #27  ARITH. SHIFT RIGHT THE TOP OF STACK
ASRS	TSX		;* (LABEL/NM only?)
	AND  B #$1F     ;* MAX REASONABLE SHIFT
	BEQ  ASRS2	;* 

ASRS1	ASR  0,X  	;*
	ROR  1,X 	;* 
	DEC  B          ;* 
	BNE  ASRS1	;* 

ASRS2	PUL  A		;* GET THE RESULT XXXXXXXX
	PUL  B          ;* 
	JMP  NEXT	;* 

*--------------------------------
* #28  ARITH. SHIFT LEFT THE TOP OF STACK
ASLS	TSX		;* (LABEL/NM only?)
	AND  B #$1F     ;* 
	BEQ  ASRS2	;* 

ASLS1	ASL  1,X  	;*
	ROL  0,X 	;* 
	DEC  B          ;* 
	BNE  ASLS1	;* 

	BRA  ASRS2	;* 

*--------------------------------
* #29  NEGATE THE REGISTER
NEGR	BSR  NEGATE	;*
	JMP  NEXT	;* 

*--------------------------------
* #30  COMPLEMENT THE REGISTER
NOTR	COM  A  	;*
	COM  B          ;* 
	JMP  NEXT	;* 

*--------------------------------
* #31  ADD 1 TO REG
INCR	ADD  B		;* #1 XXXXXXXX
	ADC  A          ;* #0
	JMP  NEXT	;* 

*--------------------------------
* #32 SUBTRACT 1 FROM REG
DECR	SUB  B		;* #1 XXXXXXXX
	SBC  A          ;* #0
	JMP  NEXT	;* 

*****************************************************
*
*   BASIC COMPARE INSTRUCTION SUBROUTINE
*   Compare the top of Stack to Register and set Condition codes
*
*  Signed compare -- Carry reflects the sign of difference
*         (set means: top of stack < A,B )
*
SCMP	TSX		;* (LABEL/NM only?)
	LDA  A 2,X      ;* GET TOP OF STACK
	LDA  B 3,X      ;* 
	SUB  B R1B      ;* SET CONDITION
	SBC  A R1A      ;* ... FLAGS
	BPL  STCMP1	;* SKIP IF PLUS

	STA  B R1B      ;* TEMP SAVE
	ORA  A R1B      ;* SET/RESET ZERO FLAG
	SEC		;* AND SET CARRY
	RTS		;* 

STCMP1	STA  B R1B      ;* XXXXXXXX
	ORA  A R1B      ;* 
	CLC		;* CLEAR THE CARRY
	RTS		;* 
*
*  Unsigned compare, Carry set if top of stack < A,B
*
BCMP	TSX		;* (LABEL/NM only?)
	LDA  A 2,X      ;* GET TOP OF STACK
	LDA  B 3,X      ;* 
	CMP  A R1A      ;* CHECK TOP BYTE
	BNE  BCMP1	;* 
	CMP  B R1B      ;* 
BCMP1	RTS		;* (LABEL/NM only?)


*-------------------------------
* #33  TEST FOR EQUALITY
ZEQ	BSR  BCMP	;*
	BEQ  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #34  TEST FOR NOT-EQUAL
ZNE	BSR  BCMP	;*
	BNE  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #35  TEST FOR LESS THAN
ZLT	BSR  SCMP	;*
	BCS  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #36  TEST FOR LESS THAN OR EQUAL
ZLE	BSR  SCMP	;*
	BLS  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #37  TEST FOR GREATER THAN
ZGT	BSR  SCMP	;*
	BHI  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #38  TEST FOR GREATER THAN OR EQUAL
ZGE	BSR  SCMP	;*
	BCC  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #39 TEST FOR LESS THAN (UNSIGNED)
ULT	BSR  BCMP	;*
	BCS  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #40  TEST FOR LESS THAN OR EQUAL (UNSIGNED)
ULE	BSR  BCMP	;*
	BLS  TRUE	;* 
	BRA  FALSE	;* 

*-------------------------------
* #41  TEST FOR GREATER THAN (UNSIGNED)
UGT	BSR  BCMP	;*
	BHI  TRUE	;* 
	BRA  FALSE	;* 

*------------------------------
* #42  TEST FOR GREATER THAN OR EQUAL (UNSIGNED)
UGE	BSR  BCMP	;*
	BCC  TRUE	;* 

FALSE	CLR  B		;* RETURN FALSE XXXXXXXX
	BRA  TRUE1	;* 

TRUE	LDA  B		;* #1 RETURN TRUE XXXXXXXX

TRUE1	CLR  A  	;*
	JMP  POPS	;* POP STACK AND PROCEED

*-------------------------------------
* #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC	LDX  PC		;* POINT TO CODE XXXXXXXX
	JMP  0,X 	;* GO EXECUTE IT

***********************************************************
*
*        RUN-TIME SUBROUTINE LIBRARY
*
***********************************************************

*   fopen(file-name, "type")
*   Open a File..........
fopen	FCB  86  	;* SWITCH TO INLINE CODE
	LDX  #FCBTBL-2	;* POINT TO FCB ADDRESSES TABLE

NXTFIL	INX		;* (LABEL/NM only?)
	INX		;* 
	STX  FCBPTR	;* SAVE POINTER
	LDX  0,X 	;* GET FCB ADDRESS
	BEQ  NOFILE	;* 
	TST  2,X 	;* BUSY ?
	BEQ  GODOIT	;* NO,
	LDX  FCBPTR	;* ELSE, NEXT IN LINE
	BRA  NXTFIL	;* 

NOFILE	LDX  #FMSG	;* POINT TO MESSAGE XXXXXXXX
	JSR  PSTRNG	;* PRINT IT
	JMP  WARMS	;* --AND BACK TO FLEX

GODOIT	STX  R1A	;* SAVE FCB ADDRESS XXXXXXXX
	TSX		;* 
	LDX  4,X 	;* POINT TO FILE NAME
	STX  $AC14	;* SAVE IN LINE BUFFER POINTER
	LDX  R1A 	;* GET FCB POINTER
	JSR  GETFIL	;* GET FILE SPEC
	BCS  FERROR	;* REPORT IF ERROR
	TSX		;* 
	LDX  2,X 	;* POINT TO MODE
	LDA  A 	;* #1
	LDA  B 	;* #'w' OPEN FOR WRITE
	CMP  B 	;* 0,X  -- ????
	BEQ  OWRITE	;* YES,

*  DEFAULT TO OPEN FOR READ

	LDX  R1A 	;* POINT TO FCB
	STA  A 	;* 0,X STORE IN FCB
	JSR  FMS 	;* DO THE OPEN
	BEQ  FEXIT	;* 

FERROR	JSR  RPTERR	;* REPORT THE TYPE OF ERROR XXXXXXXX
	JSR  FMSCLS	;* CLOSE ALL OPEN FILES
	JMP  WARMS	;* RETURN TO FLEX

* OK, OPEN FOR WRITE

OWRITE	LDA  A		;* #2 CODE FOR WRITE XXXXXXXX
	LDX  R1A 	;* GET FCB ADDRESS
	STA  A 	;* 0,X
	JSR  FMS 	;* TRY AN OPEN
	BEQ  FEXIT	;* IF SUCCESSFULL--DONE

	LDA  A 	;* 1,X GET ERROR STATUS
	CMP  A 	;* #3 ALREADY EXISTS ?
	BNE  FERROR	;* NO--SOME OTHER ERROR

	LDA  A 	;* #12 DELETE THE EXISTING FILE
	STA  A 	;* 0,X
	JSR  FMS 	;* 
	BNE  FERROR	;* 
	LDA  A 	;* 36,X FIX NAME
	STA  A 	;* 4,X
	BRA  OWRITE	;* 

FEXIT	TSX		;* (LABEL/NM only?)
	LDX  2,X 	;* POINT TO MODE AGAIN
	LDA  B 	;* 1,X GET OPTIONAL CHAR
	CMP  B 	;* #'u UNCOMPRESSED (BINARY) ???
	BNE  FEXIT1	;* NO, SO SKIP

	LDA  B 	;* #$FF
	LDX  R1A 	;* 
	STA  B 	;* 59,X SET FLAG IN FCB

FEXIT1	LDA  A		;* R1A RETURN THE FCB POINTER XXXXXXXX
	LDA  B 	;* R1B

	JMP  RTSC	;* RETURN TO INTERPRETER

*-------------------------------------------------

*  fclose(unit)
*  CLOSE A FILE
fclose	FCB  86  	;* SWITCH TO IN-LINE
	TSX		;* 
	LDX  2,X 	;* POINT TO FCB
	LDA  A 	;* #4 CLOSE CODE
	STA  A 	;* 0,X
	JSR  FMS 	;* DO THE CLOSE
	BNE  FERROR	;* 
	CLR  A 	;* 
	LDA  B 	;* #1 OK CODE
	JMP  RTSC	;* RETURN TO INTERPRETER....

*--------------------------------------------------

*  getc(unit)  read a byte from file
*        return a char, else a -1 if EOF

getc	FCB  86  	;* 
	TSX		;* 
	LDX  2,X 	;* POINT TO FCB
	JSR  FMS 	;* GET BYTE
	BEQ  CHOK	;* 

	LDA  A 	;* 1,X GET ERROR
	CMP  A 	;* #8 EOF ?
	BNE  FERROR	;* 

	LDA  A 	;* #$FF LOAD EOF INDICATOR

CHOK	TAB  COPY	;* CHAR IN A XXXXXXXX
CHOK1	CLR  A  	;*
	TST  B 	;* 
	BPL  GETC1	;* 

	COM  A 	;* SIGN EXTEND

GETC1	JMP  RTSC	;*

*----------------------------------------------

*  putc(c,unit)   write to file

putc	FCB  86  	;* 
	TSX		;* 
	LDA  A 	;* 5,X GET CHAR
	LDX  2,X 	;* GET FCB ADDR
	PSH  A 	;* SAVE CHAR
	JSR  FMS 	;* 
	BNE  FERROR	;* 
	PUL  B 	;* GET CHAR
	BRA  CHOK1	;* 

*-----------------------------------------------
FMSG	FCC  'NO	;* MORE FILES MAY BE OPENED.' XXXXXXXX
	FCB  $0D,$0A,4	;* 
*-----------------------------------------------

*   getchar()    get a char from standard input

getchar	FCB  86  	;* 
	JSR  GETCHR	;* 
	CMP  A 	;* #$0D CR ???
	BEQ  GETCH1	;* SKIP IF TRUE

	CMP  A 	;* #$1A COMPARE TO CNTRL-Z (EOF)
	BNE  CHOK	;* NO
	LDA  B 	;* #$FF YES...
	BRA  CHOK1	;* RETURN -1

GETCH1	LDA  A		;* #$0A LOAD A LF XXXXXXXX
	JSR  PUTCHR	;* ECHO IT
	LDA  B 	;* #$0D
	BRA  CHOK1	;* 

*-----------------------------------------------

*   putchar(c)   write a char to standard output

putchar	FCB  86  	;* 
	TSX		;* 
	LDA  A 	;* 3,X GET THE CHAR
	PSH  A 	;* SAVE CHAR
	CMP  A 	;* #$0D IS IT A CR ?
	BEQ  PUTC2	;* YES, SKIP
	JSR  PUTCHR	;* ELSE, OUTPUT IT
PUTC1	PUL  B		;* RESTORE CHAR XXXXXXXX
	BRA  CHOK1	;* 

PUTC2	JSR  PCRLF	;* OUTPUT CR/LF PAIR XXXXXXXX
	BRA  PUTC1	;* 

*----------------------------------------------

*   gets(buffer)  get a char string into buffer

gets	FCB  86  	;* 
	TSX		;* 
	LDX  2,X 	;* GET START OF BUFFER
	CLR  B 	;* 

GETS1	JSR  GETCHR	;* READ A CHAR XXXXXXXX
	CMP  A 	;* $AC00 BACKSPACE ?
	BNE  GETS2	;* 

	LDA  A 	;* #$20
	JSR  PUTCHR	;* 
	LDA  A 	;* #$08
	JSR  PUTCHR	;* 
	TST  B 	;* BEGINNING OF LINE ?
	BEQ  GETS1	;* YES,

	DEC  B 	;* ELSE,
	DEX		;* ADJUST LINE POINTER
	BRA  GETS1	;* 

GETS2	CMP  A		;* $AC01 DELETE LINE CHAR ? XXXXXXXX
	BNE  GETS3	;* 

	LDA  A 	;* #$0D CR...
	JSR  PUTCHR	;* 
	LDA  A 	;* #$0A LF...
	JSR  PUTCHR	;* 
	BRA  gets+1	;* GO TRY AGAIN....

GETS3	STA  A		;* 0,X GOOD CHAR--STORE IN BUFFER XXXXXXXX
	INX		;* AND BUMP POINTER
	INC  B 	;* AND COUNTER

	CMP  A 	;* #$0D IS IT A CR ?
	BNE  GETS1	;* NO

	DEX		;* 
	CLR  0,X 	;* MARK END OF STRING WITH A NULL
	STX  R1A 	;* SAVE POINTER VALUE
	LDA  A 	;* R1A GET IT INTO WORK
	LDA  B 	;* R1B .. REG
	TSX		;* 
	SUB  B 	;* 3,X RETURN LENGTH
	SBC  A 	;* 2,X --OF BUFFER
GETS4	JMP  RTSC	;*

*----------------------------------------------
*  puts(string)  print a string on the terminal
puts	FCB  86  	;* 
	TSX		;* 
	LDX  2,X 	;* GET STRING ADDRESS
PLOOP	LDA  A		;* 0,X GET THE CHAR XXXXXXXX
	BEQ  GETS4	;* IF END OF STRING--QUIT
	CMP  A 	;* #'\ SPECIAL CHAR ?
	BNE  PLOOP1	;* NO, SKIP
	BSR  SPECIAL	;* YES, INTERPRET
	CMP  A 	;* #$0D IS IT A CR (NEWLINE)
	BNE  PLOOP1	;* NO--SKIP
	JSR  PCRLF	;* YES PRINT CR/LF PAIR
	BRA  PLOOP2	;* 
PLOOP1	JSR  PUTCHR	;* PRINT IT XXXXXXXX
PLOOP2	INX  BUMP	;* POINTER XXXXXXXX
	BRA  PLOOP	;* 


*  This subroutine interprets the backslash (\) sequence.

SPECIAL	INX		;* (LABEL/NM only?)
	LDA  A 	;* 0,X GET NEXT CHAR
	CMP  A 	;* #'b
	BNE  SP1 	;* 
	LDA  A 	;* #08 BACKSPACE
	BRA  SPEXIT	;* 
SP1	CMP  A		;* #'f XXXXXXXX
	BNE  SP2 	;* 
	LDA  A 	;* #$0C FORMFEED
	BRA  SPEXIT	;* 
SP2	CMP  A		;* #'n XXXXXXXX
	BNE  SP3 	;* 
	LDA  A 	;* #$0D NEWLINE
	BRA  SPEXIT	;* 
SP3	CMP  A		;* #'\ BACKSLASH XXXXXXXX
	BEQ  SPEXIT	;* 
	CMP  A 	;* #'' SINGLE QUOTE
	BEQ  SPEXIT	;* 
	CMP  A 	;* #'" DOUBLE QUOTE
	BEQ  SPEXIT	;* 
	CMP  A 	;* #'x START OF HEX SEQUENCE
	BEQ  SPHEX	;* 
	CMP  A 	;* #'0 OCTAL SEQUENCE ?
	BLT  SPERR	;* 
	CMP  A 	;* #'7
	BLE  SPOCTAL	;* YES

SPERR	DEX  BACKUP	;* THE POINTER XXXXXXXX
	LDA  A 	;* 0,X RESTORE CHAR
SPEXIT	RTS  RETURN	;*

SPOCTAL	LDA  B		;* #3 XXXXXXXX
	STA  B 	;* DFLAG SAVE COUNTER
	CLR  B 	;* 

SPOCT1	SUB  A		;* #'0 CONVERT TO DIGIT XXXXXXXX
	ASL  B 	;* SHIFT ACCUM
	ASL  B 	;* 
	ASL  B 	;* 
	ABA		;* ADD IN NEW DIGIT
	TAB		;* SAVE
	DEC  DFLAG	;* 
	BEQ  SPBYE	;* IF MAX COUNT--EXIT
	INX		;* 
	LDA  A 	;* 0,X GET NEXT CHAR
	CMP  A 	;* #'0 VERIFY IF OCTAL
	BLT  SPFINI	;* 
	CMP  A 	;* #'7
	BLE  SPOCT1	;* YES, CONTINUE

SPFINI	DEX  BACKUP	;*
SPBYE	TBA  GET	;* ACCUM CHAR XXXXXXXX
	RTS		;* 

SPHEX	LDA  B		;* #2 XXXXXXXX
	STA  B 	;* DFLAG
	CLR  B 	;* 

SPHEXL	INX		;* (LABEL/NM only?)
	LDA  A 	;* 0,X GET NEXT CHAR
	CMP  A 	;* #'0 VERIFY IF HEX
	BLT  SPFINI	;* ..
	CMP  A 	;* #'9 ..
	BLE  SPHEX2	;* ..
	CMP  A 	;* #'A ..
	BLT  SPFINI	;* ..
	CMP  A 	;* #'F ..
	BLE  SPHEX1	;* ..
	CMP  A 	;* #'a ..
	BLT  SPFINI	;* ..
	CMP  A 	;* #'f ..
	BGT  SPFINI	;* ..
	SUB  A 	;* #$20 YES IT IS HEX
SPHEX1	SUB  A		;* #7 XXXXXXXX
SPHEX2	SUB  A		;* #'0 XXXXXXXX
	ASL  B 	;* 
	ASL  B 	;* 
	ASL  B 	;* 
	ASL  B 	;* 
	ABA		;* 
	TAB		;* 
	DEC  DFLAG	;* 
	BEQ  SPBYE	;* MAX COUNT REACHED ???
	BRA  SPHEXL	;* NO, LOOP


*----------------------------------------------
*
*
*       Test if given char is alpha     *
* isalpha(c)
*       char c;
* {     c=c&127;
*       return(((c>='a')&(c<='z'))|
*               ((c>='A')&(c<='Z'))|
*               (c='_'));
* }
*
isalpha	FCB  86  	;* switch to assembly
	TSX		;* 
	LDA  B 	;* 3,X     get char
	BSR  alPHA	;* 
	JMP  RTSC	;* 
*
*---------------------------------------------
*       Test if given char is numeric   *
*
* isdigit(c)
*       char c;
* {     c=c&127;
*       return((c>='0')&(c<='9'));
* }
*
isdigit	FCB  86  	;* 
	TSX		;* 
	LDA  B 	;* 3,X
	BSR  nuMERIC	;* 
	JMP  RTSC	;* 
*
*----------------------------------------------
*       Test if given char is alphanumeric      *
*
* isalnum(c)
*       char c;
* {     return((alpha(c)|(numeric(c)));
* }
*
isalnum	FCB  86  	;* 
	TSX		;* 
	LDA  B 	;* 3,X     get char
	BSR  alPHA	;* check if alpha
	TSX		;* 
	PSH  B 	;* save result
	LDA  B 	;* 3,X     get char again
	BSR  nuMERIC	;* check if decimal
	TSX		;* 
	ORA  B 	;* 0,X     fix flag
	INS		;* clean stack
	JMP  RTSC	;* 
*
*
alPHA	CLR  A  	;*
	AND  B 	;* #$7F
	CMP  B 	;* #'a
	BLT  alPHA1	;* 
	CMP  B 	;* #'z
	BLE  alYES	;* 
alPHA1	CMP  B		;* #'A XXXXXXXX
	BLT  alPHA2	;* 
	CMP  B 	;* #'Z
	BLE  alYES	;* 
alPHA2	CMP  B		;* #'_ XXXXXXXX
	BEQ  alYES	;* 
*
alNO	CLR  B  	;*
	RTS		;* 
*
alYES	LDA  B		;* #1 XXXXXXXX
	RTS		;* 
*
*
nuMERIC	CLR  A  	;*
	AND  B 	;* #$7F
	CMP  B 	;* #'0
	BLT  alNO	;* 
	CMP  B 	;* #'9
	BLE  alYES	;* 
	BRA  alNO	;* 
*
*-----------------------------------------------
*  islower(c)
*       char c;        returns TRUE if c is lower case alpha,
*                        FALSE otherwise.
*
islower	FCB  86  	;* SWITCH TO IN-LINE
	TSX		;* 
	LDA  B 	;* 3,X GET CHAR
	AND  B 	;* #$7F
	CLR  A 	;* 
	CMP  B 	;* #'a
	BLT  ISNO	;* 
	CMP  B 	;* #'z
	BLE  ISYES	;* 
*
ISNO	CLR  B  	;*
	JMP  RTSC	;* RETURN FALSE
*
ISYES	LDA  B		;* #1 XXXXXXXX
	JMP  RTSC	;* RETURN TRUE
*
*----------------------------------------------
*  isupper(c)
*      char c;      return TRUE if c is upper case alpha.
*
isupper	FCB  86  	;* 
	TSX		;* 
	LDA  B 	;* 3,X
	AND  B 	;* #$7F
	CLR  A 	;* 
	CMP  B 	;* #'A
	BLT  ISNO	;* 
	CMP  B 	;* #'Z
	BLE  ISYES	;* 
	BRA  ISNO	;* 
*
*-----------------------------------------------
*  isspace(c)
*      char c;       return TRUE if a "white space" char
*
isspace	FCB  86  	;* 
	TSX		;* 
	LDA  B 	;* 3,X GET CHAR
	AND  B 	;* #$7F
	CLR  A 	;* 
	CMP  B 	;* #'   SPACE ?
	BEQ  ISYES	;* 
	CMP  B 	;* #$0D  CR ???
	BEQ  ISYES	;* 
	CMP  B 	;* #$0A  LF ???
	BEQ  ISYES	;* 
	CMP  B 	;* #$09  HOR TAB ???
	BEQ  ISYES	;* 
	BRA  ISNO	;* 
*
*----------------------------------------------
*  toupper(c)
*      char c;   make c an upper case char if lower
*                          case alpha
*
toupper	FCB  86  	;* 
	TSX		;* 
	LDA  B 	;* 3,X
	LDA  A 	;* 2,X
	BNE  TOUPP1	;* SKIP IF MSB'S NOT ZERO
	CMP  B 	;* #'a
	BLT  TOUPP1	;* 
	CMP  B 	;* #'z
	BGT  TOUPP1	;* 
	SUB  B 	;* #$20  CONVERT TO UPPER CASE
TOUPP1	JMP  RTSC	;*
*
*---------------------------------------------
*  tolower(c)
*      char c;   convert to lower case if upper case alpha.
*
tolower	FCB  86  	;* 
	TSX		;* 
	LDA  B 	;* 3,X
	LDA  A 	;* 2,X
	BNE  TOLOW1	;* 
	CMP  B 	;* #'A
	BLT  TOLOW1	;* 
	CMP  B 	;* #'Z
	BGT  TOLOW1	;* 
	ADD  B 	;* #$20 CONVERT TO LOWER CASE
TOLOW1	JMP  RTSC	;*
*
*---------------------------------------------
*  strclr(s,n)
*      char *s;  int n;  clear a string of n bytes.
*
strclr	FCB  86  	;* 
	TSX		;* 
	LDA  A 	;* 2,X GET LENGTH OF STRING
	LDA  B 	;* 3,X
	LDX  4,X 	;* POINT TO STRING
	TST  B 	;* 
	BEQ  SCLR2	;* 

SCLR1	CLR  0,X  	;*
	INX		;* 
	DEC  B 	;* 
	BNE  SCLR1	;* 
SCLR2	TST  A  	;*
	BEQ  SCLR3	;* 
	DEC  A 	;* 
	BRA  SCLR1	;* 

SCLR3	JMP  RTSC	;*
*
*-----------------------------------------------
*  return the length of a string
*
* strlen(s)
*       char *s;
* {     char *t;
*       t=s;
*       while (*s) s++;
*       return (s-t);
* }
*
strlen	FCB  86  	;* 
	TSX		;* 
	LDX  2,X 	;* point to string
	CLR  A 	;* preset counter
	CLR  B 	;* 
*
strlLP	TST  0,X	;* look for NULL XXXXXXXX
	BEQ  strlRT	;* found !!
	INX		;* 
	ADD  B 	;* #1      bump counter
	ADC  A 	;* #0
	BRA  strlLP	;* 
*
strlRT	JMP  RTSC	;*
*
*------------------------------------------------
*  strcpy(s1,s2)
*      char *s1, *s2;    copy s2 into s1.
*
strcpy	FCB  86  	;* 
	TSX		;* 
	LDX  4,X 	;* POINT TO S1
	STX  X1TMP	;* SAVE POINTER
	TSX		;* 
	LDX  2,X 	;* POINT TO S2

SCPY1	LDA  B		;* 0,X XXXXXXXX
	INX		;* 
	STX  X2TMP	;* 
	LDX  X1TMP	;* 
	STA  B 	;* 0,X
	BEQ  SCLR3	;* END OF STRING ???
	INX		;* 
	STX  X1TMP	;* 
	LDX  X2TMP	;* 
	BRA  SCPY1	;* 
*
*------------------------------------------------
*  strcat(s1,s2)
*      char *s1, *s2;   s2 is concatenated onto s1.
*
strcat	FCB  86  	;* 
	TSX		;* 
	LDX  2,X 	;* 
	STX  X2TMP	;* SAVE POINTER TO S2
	TSX		;* 
	LDX  4,X 	;* POINT TO S1

SCAT1	TST  0,X	;* LOOK FOR END OF STRING XXXXXXXX
	BEQ  SCAT2	;* 
	INX		;* 
	BRA  SCAT1	;* 

SCAT2	STX  X1TMP	;* SAVE POINTER XXXXXXXX
	LDX  X2TMP	;* 
	LDA  B 	;* 0,X
	INX		;* 
	STX  X2TMP	;* 
	LDX  X1TMP	;* 
	STA  B 	;* 0,X
	BEQ  SCAT3	;* 
	INX		;* 
	BRA  SCAT2	;* 

SCAT3	JMP  RTSC	;*
*
*--------------------------------------------
*  strcmp(s1,s2)
*        char *s1, *s2;       returns:   0  if s1 = s2
*                                       <0  if s1 < s2
*                                       >0  if s1 > s2
*
strcmp	FCB  86  	;* 
	TSX		;* 
	LDX  2,X 	;* POINT TO S2
	STX  X2TMP	;* 
	TSX		;* 
	LDX  4,X 	;* POINT TO S1

SCMP1	LDA  A		;* 0,X GET S1 CHAR XXXXXXXX
	BEQ  SCMP3	;* 
	INX		;* 
	STX  X1TMP	;* 
	LDX  X2TMP	;* 
	LDA  B 	;* 0,X
	BEQ  SCMP2	;* 
	SBA		;* COMPARE BY SUBTRACTING
	BNE  SCMP3	;* 
	INX		;* 
	STX  X2TMP	;* 
	LDX  X1TMP	;* 
	BRA  SCMP1	;* 

SCMP2	CLR  A  	;*

SCMP3	TAB		;* (LABEL/NM only?)
	BMI  SCMP4	;* 
	CLR  A 	;* 
	JMP  RTSC	;* 

SCMP4	LDA  A		;* #$FF SIGN EXTEND XXXXXXXX
	JMP  RTSC	;* 
*
*
*****************************************************
*
HERE	EQU  *  	;* END OF INTERPRETER
* 
	END		;* 
*[ Fini ]***********************************************************************
