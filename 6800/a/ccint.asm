*[ Start ]**********************************************************************
;* This is 6800 code
	ORG  0                ;* This is a huge waste of the Zero Page
	NAM  SMALL-C          ;* INTERPRETER
	OPT  NOP,NOG          ;* 
*
*   LAST UPDATE   9-SEP-82
*
	JMP  BEGIN              ;* START THE INTERPRETER

*  I have no idea what this is??? Is this something Flex uses?
*  AN INDIRECT CALL TABLE
	NOP                     ;* PUT ON A BOUNDARY OF 4
	FCB  86                 ;* 
	JMP  fclose+1           ;* 
	FCB  86                 ;* 
	JMP  fopen+1            ;* 
	FCB  86                 ;* 
	JMP  getc+1             ;* 
	FCB  86                 ;* 
	JMP  getchar+1          ;* 
	FCB  86                 ;* 
	JMP  gets+1             ;* 
	FCB  86                 ;* 
	JMP  putc+1             ;* 
	FCB  86                 ;* 
	JMP  putchar+1          ;* 
	FCB  86                 ;* 
	JMP  puts+1             ;* 
	NOP                     ;* 
	JMP  RTSC               ;* 
	FCB  86                 ;* 
	JMP  isalpha+1          ;* 
	FCB  86                 ;* 
	JMP  isdigit+1          ;* 
	FCB  86                 ;* 
	JMP  isalnum+1          ;* 
	FCB  86                 ;* 
	JMP  islower+1          ;* 
	FCB  86                 ;* 
	JMP  isupper+1          ;* 
	FCB  86                 ;* 
	JMP  isspace+1          ;* 
	FCB  86                 ;* 
	JMP  toupper+1          ;* 
	FCB  86                 ;* 
	JMP  tolower+1          ;* 
	FCB  86                 ;* 
	JMP  strclr+1           ;* 
	FCB  86                 ;* 
	JMP  strlen+1           ;* 
	FCB  86                 ;* 
	JMP  strcpy+1           ;* 
	FCB  86                 ;* 
	JMP  strcat+1           ;* 
	FCB  86                 ;* 
	JMP  strcmp+1           ;* 
	RMB  4*4                ;* ROOM FOR 4 MORE

	LIB  FLEXPTRS           ;* 

NFILES	EQU  4                  ;* MAX NO OF DISK FILES OPEN AT ONCE
PC	RMB  2                  ;* PSEUDO PROGRAM COUNTER
R1A	RMB  1                  ;* WORKING 16 BIT
R1B	RMB  1                  ;* --REGISTER
DFLAG	FCB  NFILES-1           ;* DIVIDE ROUTINE FLAG
STEMP	RMB  2                  ;* TEMP STORAGE FOR STACK POINTER
X1TMP	RMB  2                  ;* TEMP STORAGE FOR X REG
X2TMP	RMB  2                  ;* ... DITTO ...
FCBPTR	RMB  2                  ;* POINTER INTO FCB TABLE
FCBTBL	FDB  FCB                ;* TABLE OF FCB POINTERS
	RMB  NFILES*2           ;* ROOM FOR THE REST



***************************************************

BEGIN	LDX   #FCBTBL+2         ;* POINT TO FCB ADDRESSES XXXXXXXX
	LDAA  #NFILES-1
	STAA  DFLAG             ;* INIT COUNTER
	LDAA  $AC2B             ;* GET TOP OF MEMORY
	LDAB  $AC2C

BLOOP	SUBB                    ;* #$40 SUBTR 320 (SIZE OF FCB) XXXXXXXX
	SBCA  #1
	STAA  0,X               ;* SAVE FCB ADDRESS
	INX                     ;* 
	STAB  0,X
	INX                     ;* 
	DEC   DFLAG             ;* DONE ???
	BNE   BLOOP             ;* 

	CLR   0,X               ;* MARK END OF TABLE
	CLR   1,X               ;* 
	STAA  STEMP             ;* TOP OF STACK AREA
	STAB  STEMP+1
	LDX   STEMP             ;* 
	TXS                     ;* SET STACK POINTER

	LDX   #FCBTBL           ;* POINT TO TABLE OF FCB ADDRESSES
Init	STX   FCBPTR            ;*
	LDX   0,X               ;* GET FCB ADDRESS
	BEQ   Initend           ;* QUIT IF END OF TABLE
	CLR   2,X               ;* MARK AS NOT IN USE
	LDX   FCBPTR            ;* 
	INX                     ;* 
	INX                     ;* 
	BRA   Init              ;* 

Initend	LDX  #$800              ;* I think this is CODE
	BRA  NEXT2              ;* START THE INTERPRETATION

**************************************************************
*
*  THE HEART OF THE INTERPRETER--- NEXT INSTRUCTION FETCHER.
*
BUMP2	LDX  PC                 ;* GET PROG COUNTER XXXXXXXX
BUMP2A	INX  INCR               ;* BY 2 XXXXXXXX
	INX                     ;* 
	BRA  NEXT1              ;* FETCH NEXT INSTRUCTION

NEXT	LDX    PC               ;*
NEXT1	STAA                    ;* R1A SAVE THE WORK XXXXXXXX
	STAB   R1B              ;* --REGISTER
NEXT2	LDAB   0,X              ;* GET THE PSEUDO-INSTRUCTION XXXXXXXX
	INX                     ;* (B CONTAINS A TABLE OFFSET)
	STX    PC               ;* SAVE NEW PC
	STAB   JJJ+2            ;* SAVE AS PAGE OFFSET
	LDAB   R1B              ;* RESTORE
JJJ	LDX    JTABLE           ;* POINT TO ROUTINE (SELF MODIFYING CODE) XXXXXXXX
	JMP    0,X              ;* GO EXECUTE THE PSEUDO-INSTR.

**************************************************************
*                  THE JUMP TABLE                            *
**************************************************************

	ORG  *+255/256*256	;* MUST START ON A PAGE BOUNDARY

JTABLE	FDB  LD1IM              ;* #0
	FDB  LD1SOFF            ;* #1
	FDB  LD1                ;* #2
	FDB  LDB1               ;* #3
	FDB  LD1R               ;* #4
	FDB  LDB1R              ;* #5
	FDB  ST1                ;* #6
	FDB  STB1               ;* #7
	FDB  ST1SP              ;* #8
	FDB  STB1SP             ;* #9
	FDB  PUSHR1             ;* #10
	FDB  EXG1               ;* #11
	FDB  JMPL               ;* #12
	FDB  BRZL               ;* #13
	FDB  JSRL               ;* #14
	FDB  JSRSP              ;* #15
	FDB  RTSC               ;* #16
	FDB  MODSP              ;* #17
	FDB  DBL1               ;* #18
	FDB  ADDS               ;* #19
	FDB  SUBFST             ;* #20
	FDB  MUL1               ;* #21
	FDB  DIV1               ;* #22
	FDB  MOD                ;* #23
	FDB  ORS                ;* #24
	FDB  XORS               ;* #25
	FDB  ANDS               ;* #26
	FDB  ASRS               ;* #27
	FDB  ASLS               ;* #28
	FDB  NEGR               ;* #29
	FDB  NOTR               ;* #30
	FDB  INCR               ;* #31
	FDB  DECR               ;* #32
	FDB  ZEQ                ;* #33
	FDB  ZNE                ;* #34
	FDB  ZLT                ;* #35
	FDB  ZLE                ;* #36
	FDB  ZGT                ;* #37
	FDB  ZGE                ;* #38
	FDB  ULT                ;* #39
	FDB  ULE                ;* #40
	FDB  UGT                ;* #41
	FDB  UGE                ;* #42
	FDB  ASMC               ;* #43

*************************************************************
*-------------------------
* #0 LOAD REG WITH IMMED. VALUE
LD1IM	LDX    PC               ;*
	LDAA   0,X              ;* HIGH BYTE
	LDAB   1,X              ;* LOW BYTE
	JMP    BUMP2A           ;* 

*-------------------------
* #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF	STS    R1A              ;* SAVE STACK VALUE XXXXXXXX
	LDX    PC               ;* 
	LDAA   0,X              ;* GET OFFSET
	LDAB   1,X              ;* -- VALUE
	SEC                     ;* 
	ADCB   R1B              ;* ADD OFFSET + 1
	ADCA   R1A
	JMP    BUMP2A           ;*

*-------------------------
* #2  LOAD WORD @ ADDRESS
LD1	LDX    PC               ;*
	LDX    0,X              ;* GET ADDRESS
LD1A	LDAA                    ;* 0,X GET WORD XXXXXXXX
	LDAB   1,X              ;* 
	JMP    BUMP2            ;* 

*-------------------------
* #3  LOAD BYTE @ ADDRESS
LDB1	LDX     PC              ;*
	LDX     0,X             ;* GET ADDRESS
	CLRA                    ;* 
	LDAB    0,X             ;* GET BYTE
	BPL     LDB1A           ;* 
	COMA                    ;* SIGN EXTEND
LDB1A	JMP     BUMP2           ;*

*-------------------------
* #4  LOAD WORD INDIRECT (ADDR IN REG)
LD1R	LDX     R1A             ;* GET ADDRESS XXXXXXXX
	LDAA    0,X             ;* GET WORD
	LDAB    1,X             ;* 
	JMP     NEXT            ;* 

*-------------------------
* #5  LOAD BYTE INDIRECT (ADDR IN REG)
LDB1R	LDX     R1A             ;*
	CLRA                    ;* 
	LDAB    0,X             ;* GET BYTE
	BPL     LDB1RA          ;* 
	COMA                    ;* 
LDB1RA	JMP     NEXT            ;*

*-------------------------
* #6  STORE WORD @ ADDRESS
ST1	LDX     PC              ;*
	LDX     0,X             ;* GET ADDRESS
	STAA    0,X             ;* STORE WORD
	STAB    1,X             ;* 
	JMP     BUMP2           ;* 

*-------------------------
* #7  STORE BYTE @ ADDRESS
STB1	LDX     PC              ;*
	LDX     0,X             ;* GET ADDR
	STAB    0,X             ;* STORE BYTE
	JMP     BUMP2           ;* 

*-------------------------
* #8  STORE WORD @ ADDRESS ON STACK
ST1SP	TSX     STACK           ;* TO INDEX XXXXXXXX
	LDX     0,X             ;* GET ADDRESS
	STAA    0,X             ;* STORE WORD
	STAB    1,X             ;* 
	INS                     ;* 
	INS                     ;* POP STACK
	JMP     NEXT            ;* 

*-------------------------
* #9  STORE BYTE @ ADDRESS ON STACK
STB1SP	TSX                     ;* (LABEL/NM only?)
	LDX     0,X             ;* 
	STAB    0,X             ;* STORE BYTE
	INS                     ;* POP ...
	INS                     ;* 
	JMP     NEXT            ;* 

*-------------------------
* #10  PUSH WORD ON STACK
PUSHR1	PSHB                    ;*
	PSHA                    ;* 
	LDX     PC              ;* 
	JMP     NEXT2           ;* 

*-------------------------
* #11  SWAP REG AND TOP OF STACK
EXG1	TSX                     ;* (LABEL/NM only?)
	LDX     0,X             ;* GET VALUE ON STACK
	STX     R1A             ;* SAVE
	INS                     ;* 
	INS                     ;* 
	PSHB                    ;* 
	PSHA                    ;* REG ON STACK
	LDAA    R1A             ;* NEW REG
	LDAB    R1B             ;* 
	LDX     PC              ;* 
	JMP     NEXT2           ;* 

*-------------------------
* #12  JUMP TO LABEL
JMPL	LDX     PC              ;*
JMP1	LDX     0,X             ;* GET ADDRESS (NEW PC) XXXXXXXX
	JMP     NEXT2           ;* 

*-------------------------
* #13  JUMP TO LABEL IF FALSE
BRZL	ORAA                    ;* R1B SET FLAGS XXXXXXXX
	BEQ     JMPL            ;* IF REG=0 -- JUMP
	JMP     BUMP2           ;* ELSE, PROCEED

*-------------------------
* #14  CALL TO LABEL
JSRL	LDX     PC              ;*
	INX                     ;* ADJUST RETURN
	INX                     ;* -- ADDRESS
	DES                     ;* 
	STS     *+5             ;* *** SELF MODIFYING CODE ***
	DES                     ;* 
	STX     $FFFF           ;* PUSH RETURN ADDRESS
	BRA     JMPL            ;* 

*-------------------------
* #15  CALL TO TOP OF STACK
JSRSP	TSX     POINT           ;* TO STACK XXXXXXXX
	LDX     0,X             ;* GET ADDRESS (NEW PC)
	INS                     ;* POP
	INS                     ;* 
	LDAB    PC+1            ;* GET RETURN ADDRESS
	PSHB                    ;* 
	LDAB    PC              ;* 
	PSHB                    ;* SAVE RETURN ADDRESS
	JMP     NEXT2           ;* 

*-------------------------
* #16  RETURN TO CALLER
RTSC	TSX                     ;* (LABEL/NM only?)
	LDX     0,X             ;* GET ADDRESS
	INS                     ;* POP
	INS                     ;* 
	JMP     NEXT1           ;* 

*-------------------------
* #17  MODIFY THE STACK POINTER
MODSP	LDX     PC              ;*
	LDAA    0,X             ;* GET VALUE
	LDAB    1,X             ;*
	STS     STEMP           ;* 
	ADDB    STEMP+1         ;* ADD STACK POINTER
	ADCA    STEMP           ;* 
	STAA    STEMP           ;* 
	STAB    STEMP+1         ;* 
	LDS     STEMP           ;* NEW STACK POINTER
	LDAA    R1A             ;* RESTORE REGISTER
	LDAB    R1B             ;* 
	JMP     BUMP2A          ;* 

*---------------------------
* #18  DOUBLE THE PRIMARY REGISTER
DBL1	ASLB                    ;*
	ROLA                    ;* 
	JMP     NEXT            ;* 

*---------------------------
* #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS	TSX                     ;* (LABEL/NM only?)
	ADDB    1,X             ;* DO THE ADD
	ADCA    0,X             ;* 
	JMP     POPS            ;* POP & RETURN

*---------------------------
* #20  SUBTRACT REG FROM TOP OF STACK
SUBFST	PULA                    ;* GET VALUE OFF STACK XXXXXXXX
	PULB                    ;* 
	SUBB    R1B             ;* SUBTRACT REGISTER
	SBCA    R1A             ;* 
	JMP     NEXT            ;* 

*---------------------------
* #21  MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
MUL1	PSHB                    ;*
	PSHA                    ;* REG ON STACK
	LDAA    #16             ;* 
	PSHA                    ;* SET COUNTER
	CLRA                    ;* 
	CLRB                    ;* 
	TSX                     ;* POINT TO DATA

M2	ROR     3,X             ;* SHIFT MULTIPLIER XXXXXXXX
	ROR     4,X             ;* 
	DEC     0,X             ;* DONE ?
	BMI     M4              ;* 
	BCC     M3              ;* 
	ADDB    2,X             ;* 
	ADCA    1,X             ;* 

M3	RORA                    ;*
	RORB                    ;* SHIFT RESULT
	BRA     M2              ;* AND LOOP

M4	INS     CLEAN           ;* STACK XXXXXXXX
	INS                     ;* 
	INS                     ;* 
	PULA                    ;* GET RESULT
	PULB                    ;* 
	JMP     NEXT            ;* 

*-----------------------------
* #22  DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
DIV1	BSRB    DIV             ;* DO THE BASIC DIVIDE XXXXXXXX
	LDAA    DFLAG           ;* GET SIGN FLAG
	ANDA    #1              ;* MASK OFF BIT ZERO
	PULA                    ;* GET RESULT
	PULB                    ;* 
	BEQ     DIV1R           ;* 

DIV1N	BSR     NEGATE          ;* NEGATE THE VALUE IN A,B XXXXXXXX

DIV1R	JMP     NEXT            ;*

*-----------------------------
* #23  DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
MOD	BSRB    DIV             ;*
	INS                     ;* CLEAN STACK
	INS                     ;* 
	PSHA                    ;* TEMP SAVE
	LDAA    DFLAG           ;* GET SIGN FLAG
	BPL     MOD1            ;* 
	COMA                    ;* 

MOD1	ANDA                    ;* #1 MASK OFF BIT 0 XXXXXXXX
	PULA                    ;* 
	BNE     DIV1N           ;* IF BIT 0 SET, NEGATE

	JMP     NEXT            ;* 

*****************************************************
*   BASIC 16 BIT DIVIDE ROUTINE
* ENTER WITH: DIVIDEND ON STACK
*             DIVISOR IN A,B
* EXIT WITH:  QUOTIENT ON STACK
*             REMAINDER IN A,B
*             SIGN FLAG IN DFLAG
*
BDIV	CLR     DFLAG           ;*
	TSTA                    ;* CHECK DIVISOR SIGN
	BPLB    DIV1            ;* 

	INC     DFLAG           ;* ADJUST SIGN FLAG
	BSR     NEGATE          ;* TAKE ABSOLUTE VALUE

BDIV1	PSHB                    ;* FORCE ON STACK XXXXXXXX
	PSHA                    ;* 
	LDAA    #17             ;* BIT COUNTER
	PSHA                    ;* 
	TSX                     ;* POINT TO DATA
	LDAA    5,X             ;* CHECK SIGN
	BPLB    DIV2            ;* -- OF DIVIDEND

	COM     DFLAG           ;* ADJUST FLAG
	LDAB    6,X             ;* 
	BSR     NEGATE          ;* 
	STAA    5,X             ;* 
	STAB    6,X             ;* 

BDIV2	CLRA                    ;*
	CLRB                    ;* 

* MAIN DIVIDE LOOP (UNSIGNED)

UDIV1	CMPA    1,X             ;* XXXXXXXX
	BHI     UDIV3           ;* 
	BCS     UDIV2           ;* 
	CMPB    2,X             ;* 
	BCC     UDIV3           ;* 

UDIV2	CLC                     ;* (LABEL/NM only?)
	BRA     UDIV4           ;* 

UDIV3	SUBB    2,X             ;* XXXXXXXX
	SBCA    1,X             ;* 
	SEC                     ;* 

UDIV4	ROL     6,X             ;*
	ROL     5,X             ;* 
	DEC     0,X             ;* 
	BEQ     UDIV5           ;* 

	ROLB                    ;* 
	ROLA                    ;* 
	BCC     UDIV1           ;* 
	BRA     UDIV3           ;* 

UDIV5	INS                     ;* (LABEL/NM only?)
	INS                     ;* 
	INS                     ;* 
	RTS                     ;* 

*----------------------------------------
* NEGATE THE VALUE IN A,B
NEGATE	COMA                    ;*
	COMB                    ;* 
	ADDB    #1              ;*
	ADCA    #0              ;*
	RTS                     ;* 

*----------------------------------
* #24  INCLUSIVE OR THE TOP OF STACK AND REG.
ORS	TSX                     ;* (LABEL/NM only?)
	ORAA    0,X             ;* 
	ORAB    1,X             ;* 
POPS	INS     POP             ;* THE STACK XXXXXXXX
	INS                     ;* 
	JMP     NEXT            ;* 

*----------------------------------
* #25  EXCLUSIVE OR ......
XORS	TSX                     ;* (LABEL/NM only?)
	EORA    0,X             ;* 
	EORB    1,X             ;* 
	BRA     POPS            ;* 

*----------------------------------
* #26  AND .........
ANDS	TSX                     ;* (LABEL/NM only?)
	ANDA    0,X             ;* 
	ANDB    1,X             ;* 
	BRA     POPS            ;* 

*----------------------------------
* #27  ARITH. SHIFT RIGHT THE TOP OF STACK
ASRS	TSX                     ;* (LABEL/NM only?)
	ANDB    #$1F            ;* MAX REASONABLE SHIFT
	BEQ     ASRS2           ;* 

ASRS1	ASR     0,X             ;*
	ROR     1,X             ;* 
	DECB                    ;* 
	BNE     ASRS1           ;* 

ASRS2	PULA                    ;* GET THE RESULT XXXXXXXX
	PULB                    ;* 
	JMP     NEXT            ;* 

*--------------------------------
* #28  ARITH. SHIFT LEFT THE TOP OF STACK
ASLS	TSX                     ;* (LABEL/NM only?)
	ANDB    #$1F            ;* 
	BEQ     ASRS2           ;* 

ASLS1	ASL     1,X             ;*
	ROL     0,X             ;* 
	DECB                    ;* 
	BNE     ASLS1           ;* 

	BRAA    SRS2            ;* 

*--------------------------------
* #29  NEGATE THE REGISTER
NEGR	BSR     NEGATE          ;*
	JMP     NEXT            ;* 

*--------------------------------
* #30  COMPLEMENT THE REGISTER
NOTR	COMA                    ;*
	COMB                    ;* 
	JMP     NEXT            ;* 

*--------------------------------
* #31  ADD 1 TO REG
INCR	ADDB    #1              ;* XXXXXXXX
	ADCA    #0              ;*
	JMP     NEXT            ;* 

*--------------------------------
* #32 SUBTRACT 1 FROM REG
DECR	SUBB    #1              ;* XXXXXXXX
	SBCA    #0              ;*
	JMP     NEXT            ;* 

*****************************************************
*
*   BASIC COMPARE INSTRUCTION SUBROUTINE
*   Compare the top of Stack to Register and set Condition codes
*
*  Signed compare -- Carry reflects the sign of difference
*         (set means: top of stack < A,B )
*
SCMP	TSX                     ;* (LABEL/NM only?)
	LDAA    2,X             ;* GET TOP OF STACK
	LDAB    3,X             ;* 
	SUBB    R1B             ;* SET CONDITION
	SBCA    R1A             ;* ... FLAGS
	BPL     STCMP1          ;* SKIP IF PLUS

	STAB    R1B             ;* TEMP SAVE
	ORAA    R1B             ;* SET/RESET ZERO FLAG
	SEC                     ;* AND SET CARRY
	RTS                     ;* 

STCMP1	STAB    R1B             ;* XXXXXXXX
	ORAA    R1B             ;* 
	CLC                     ;* CLEAR THE CARRY
	RTS                     ;* 
*
*  Unsigned compare, Carry set if top of stack < A,B
*
BCMP	TSX                     ;* (LABEL/NM only?)
	LDAA    2,X             ;* GET TOP OF STACK
	LDAB    3,X             ;* 
	CMPA    R1A             ;* CHECK TOP BYTE
	BNE     BCMP1           ;* 
	CMPB    R1B             ;* 
BCMP1	RTS                     ;* (LABEL/NM only?)


*-------------------------------
* #33  TEST FOR EQUALITY
ZEQ	BSRB    CMP             ;*
	BEQ     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #34  TEST FOR NOT-EQUAL
ZNE	BSRB    CMP             ;*
	BNE     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #35  TEST FOR LESS THAN
ZLT	BSR     SCMP            ;*
	BCS     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #36  TEST FOR LESS THAN OR EQUAL
ZLE	BSR     SCMP            ;*
	BLS     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #37  TEST FOR GREATER THAN
ZGT	BSR     SCMP            ;*
	BHI     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #38  TEST FOR GREATER THAN OR EQUAL
ZGE	BSR     SCMP            ;*
	BCC     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #39 TEST FOR LESS THAN (UNSIGNED)
ULT	BSRB    CMP             ;*
	BCS     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #40  TEST FOR LESS THAN OR EQUAL (UNSIGNED)
ULE	BSRB    CMP             ;*
	BLS     TRUE            ;* 
	BRA     FALSE           ;* 

*-------------------------------
* #41  TEST FOR GREATER THAN (UNSIGNED)
UGT	BSRB    CMP             ;*
	BHI     TRUE            ;* 
	BRA     FALSE           ;* 

*------------------------------
* #42  TEST FOR GREATER THAN OR EQUAL (UNSIGNED)
UGE	BSRB    CMP             ;*
	BCC     TRUE            ;* 

FALSE	CLRB                    ;* RETURN FALSE XXXXXXXX
	BRA     TRUE1           ;* 

TRUE	LDAB    #1              ;* RETURN TRUE XXXXXXXX

TRUE1	CLRA                    ;*
	JMP     POPS            ;* POP STACK AND PROCEED

*-------------------------------------
* #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC	LDX     PC              ;* POINT TO CODE XXXXXXXX
	JMP     0,X             ;* GO EXECUTE IT

***********************************************************
*
*        RUN-TIME SUBROUTINE LIBRARY
*
***********************************************************

*   fopen(file-name, "type")
*   Open a File..........
fopen	FCB     86              ;* SWITCH TO INLINE CODE
	LDX     #FCBTBL-2       ;* POINT TO FCB ADDRESSES TABLE

NXTFIL	INX                     ;* (LABEL/NM only?)
	INX                     ;* 
	STX     FCBPTR          ;* SAVE POINTER
	LDX     0,X             ;* GET FCB ADDRESS
	BEQ     NOFILE          ;* 
	TST     2,X             ;* BUSY ?
	BEQ     GODOIT          ;* NO,
	LDX     FCBPTR          ;* ELSE, NEXT IN LINE
	BRA     NXTFIL          ;* 

NOFILE	LDX     #FMSG           ;* POINT TO MESSAGE XXXXXXXX
	JSR     PSTRNG          ;* PRINT IT
	JMP     WARMS           ;* --AND BACK TO FLEX

GODOIT	STX     R1A             ;* SAVE FCB ADDRESS XXXXXXXX
	TSX                     ;* 
	LDX     4,X             ;* POINT TO FILE NAME
	STX     $AC14           ;* SAVE IN LINE BUFFER POINTER
	LDX     R1A             ;* GET FCB POINTER
	JSR     GETFIL          ;* GET FILE SPEC
	BCS     FERROR          ;* REPORT IF ERROR
	TSX                     ;* 
	LDX     2,X             ;* POINT TO MODE
	LDAA   	#1
	LDAB   	#'w'            ;* OPEN FOR WRITE
	CMPB   	0,X             ;* -- ????
	BEQ     OWRITE          ;* YES,

*  DEFAULT TO OPEN FOR READ

	LDX     R1A             ;* POINT TO FCB
	STAA    0,X             ;* STORE IN FCB
	JSR     FMS             ;* DO THE OPEN
	BEQ     FEXIT           ;* 

FERROR	JSR     RPTERR          ;* REPORT THE TYPE OF ERROR XXXXXXXX
	JSR     FMSCLS          ;* CLOSE ALL OPEN FILES
	JMP     WARMS           ;* RETURN TO FLEX

* OK, OPEN FOR WRITE

OWRITE	LDAA    #2              ;* CODE FOR WRITE XXXXXXXX
	LDX     R1A             ;* GET FCB ADDRESS
	STAA   	0,X
	JSR     FMS             ;* TRY AN OPEN
	BEQ     FEXIT           ;* IF SUCCESSFULL--DONE

	LDAA   	1,X             ;* GET ERROR STATUS
	CMPA    #3              ;* ALREADY EXISTS ?
	BNE     FERROR          ;* NO--SOME OTHER ERROR

	LDAA    #12             ;* DELETE THE EXISTING FILE
	STAA    0,X             ;*
	JSR     FMS             ;* 
	BNE     FERROR          ;* 
	LDAA    36,X            ;* FIX NAME
	STAA    4,X             ;*
	BRA     OWRITE          ;* 

FEXIT	TSX                     ;* (LABEL/NM only?)
	LDX     2,X             ;* POINT TO MODE AGAIN
	LDAB    1,X             ;* GET OPTIONAL CHAR
	CMPB    #'u'            ;* UNCOMPRESSED (BINARY) ???
	BNE     FEXIT1          ;* NO, SO SKIP

	LDAB    #$FF            ;*
	LDX     R1A             ;* 
	STAB    59,X            ;* SET FLAG IN FCB

FEXIT1	LDAA                    ;* R1A RETURN THE FCB POINTER XXXXXXXX
	LDAB                    ;* R1B

	JMP     RTSC            ;* RETURN TO INTERPRETER

*-------------------------------------------------

*  fclose(unit)
*  CLOSE A FILE
fclose	FCB     86              ;* SWITCH TO IN-LINE
	TSX                     ;* 
	LDX     2,X             ;* POINT TO FCB
	LDAA    #4              ;* CLOSE CODE
	STAA    0,X             ;*
	JSR     FMS             ;* DO THE CLOSE
	BNE     FERROR          ;* 
	CLRA                    ;* 
	LDAB    #1              ;* OK CODE
	JMP     RTSC            ;* RETURN TO INTERPRETER....

*--------------------------------------------------

*  getc(unit)  read a byte from file
*        return a char, else a -1 if EOF

getc	FCB     86              ;* 
	TSX                     ;* 
	LDX     2,X             ;* POINT TO FCB
	JSR     FMS             ;* GET BYTE
	BEQ     CHOK            ;* 

	LDAA    1,X             ;* GET ERROR
	CMPA    #8              ;* EOF ? ^H ?
	BNE     FERROR          ;* 

	LDAA    #$FF            ;* LOAD EOF INDICATOR

CHOK	TAB     COPY            ;* CHAR IN A XXXXXXXX
CHOK1	CLRA                    ;*
	TSTB                    ;* 
	BPL     GETC1           ;* 

	COMA                    ;* SIGN EXTEND

GETC1	JMP     RTSC            ;*

*----------------------------------------------

*  putc(c,unit)   write to file

putc	FCB     86              ;* 
	TSX                     ;* 
	LDAA    5,X             ;* GET CHAR
	LDX     2,X             ;* GET FCB ADDR
	PSHA                    ;* SAVE CHAR
	JSR     FMS             ;* 
	BNE     FERROR          ;* 
	PULB                    ;* GET CHAR
	BRA     CHOK1           ;* 

*-----------------------------------------------
FMSG	FCC     'NO MORE FILES MAY BE OPENED.' ;* XXXXXXXX
	FCB     $0D,$0A,4                      ;* 
*-----------------------------------------------

*   getchar()    get a char from standard input

getchar	FCB     86              ;* 
	JSR     GETCHR          ;* 
	CMPA    #$0D            ;* CR ???
	BEQ     GETCH1          ;* SKIP IF TRUE

	CMPA    #$1A            ;* COMPARE TO CNTRL-Z (EOF)
	BNE     CHOK            ;* NO
	LDAB                    ;* #$FF YES...
	BRA     CHOK1           ;* RETURN -1

GETCH1	LDAA    #$0A            ;* LOAD A LF XXXXXXXX
	JSR     PUTCHR          ;* ECHO IT
	LDAB    #$0D            ;*
	BRA     CHOK1           ;* 

*-----------------------------------------------

*   putchar(c)   write a char to standard output

putchar	FCB     86              ;* 
	TSX                     ;* 
	LDAA    3,X             ;* GET THE CHAR
	PSHA                    ;* SAVE CHAR
	CMPA    #$0D            ;* IS IT A CR ?
	BEQ     PUTC2           ;* YES, SKIP
	JSR     PUTCHR          ;* ELSE, OUTPUT IT
PUTC1	PULB                    ;* RESTORE CHAR XXXXXXXX
	BRA     CHOK1           ;* 

PUTC2	JSR     PCRLF           ;* OUTPUT CR/LF PAIR XXXXXXXX
	BRA     PUTC1           ;* 

*----------------------------------------------

*   gets(buffer)  get a char string into buffer

gets	FCB     86              ;* 
	TSX                     ;* 
	LDX     2,X             ;* GET START OF BUFFER
	CLRB                    ;* 

GETS1	JSR     GETCHR          ;* READ A CHAR XXXXXXXX
	CMPA    $AC00           ;* BACKSPACE ?
	BNE     GETS2           ;* 

	LDAA    #$20            ;*
	JSR     PUTCHR          ;* 
	LDAA    #$08            ;*
	JSR     PUTCHR          ;* 
	TSTB                    ;* BEGINNING OF LINE ?
	BEQ     GETS1           ;* YES,

	DECB                    ;* ELSE,
	DEX                     ;* ADJUST LINE POINTER
	BRA     GETS1           ;* 

GETS2	CMPA    $AC01           ;* DELETE LINE CHAR ? XXXXXXXX
	BNE     GETS3           ;* 

	LDAA    #$0D            ;* CR...
	JSR     PUTCHR          ;* 
	LDAA    #$0A            ;* LF...
	JSR     PUTCHR          ;* 
	BRA     gets+1          ;* GO TRY AGAIN....

GETS3	STAA    0,X             ;* GOOD CHAR--STORE IN BUFFER XXXXXXXX
	INX                     ;* AND BUMP POINTER
	INCB                    ;* AND COUNTER

	CMPA    #$0D            ;* IS IT A CR ?
	BNE     GETS1           ;* NO

	DEX                     ;* 
	CLR     0,X             ;* MARK END OF STRING WITH A NULL
	STX     R1A             ;* SAVE POINTER VALUE
	LDAA                    ;* R1A GET IT INTO WORK
	LDAB                    ;* R1B .. REG
	TSX                     ;* 
	SUBB    3,X             ;* RETURN LENGTH
	SBCA    2,X             ;* --OF BUFFER
GETS4	JMP     RTSC            ;*

*----------------------------------------------
*  puts(string)  print a string on the terminal
puts	FCB     86              ;* 
	TSX                     ;* 
	LDX     2,X             ;* GET STRING ADDRESS
PLOOP	LDAA    0,X             ;* GET THE CHAR XXXXXXXX
	BEQ     GETS4           ;* IF END OF STRING--QUIT
	CMPA    #'\\'           ;* SPECIAL CHAR ?
	BNE     PLOOP1          ;* NO, SKIP
	BSR     SPECIAL         ;* YES, INTERPRET
	CMPA    #$0D            ;* IS IT A CR (NEWLINE)
	BNE     PLOOP1          ;* NO--SKIP
	JSR     PCRLF           ;* YES PRINT CR/LF PAIR
	BRA     PLOOP2          ;* 
PLOOP1	JSR     PUTCHR          ;* PRINT IT XXXXXXXX
PLOOP2	INX     BUMP            ;* POINTER XXXXXXXX
	BRA     PLOOP           ;* 


*  This subroutine interprets the backslash (\) sequence.

SPECIAL	INX                     ;* (LABEL/NM only?)
	LDAA    0,X             ;*  GET NEXT CHAR
	CMPA    #'b'
	BNE     SP1             ;* 
	LDAA    #08             ;* BACKSPACE
	BRA     SPEXIT          ;* 
SP1	CMPA    #'f'            ;* XXXXXXXX
	BNE     SP2             ;* 
	LDAA    #$0C            ;* FORMFEED
	BRA     SPEXIT          ;* 
SP2	CMPA    #'n'            ;* XXXXXXXX
	BNE     SP3             ;* 
	LDAA    #$0D            ;* NEWLINE
	BRA     SPEXIT          ;* 
SP3	CMPA    #'\'            ;* BACKSLASH XXXXXXXX
	BEQ     SPEXIT          ;* 
	CMPA    #'\''           ;* SINGLE QUOTE
	BEQ     SPEXIT          ;* 
	CMPA    #'"'            ;* DOUBLE QUOTE "
	BEQ     SPEXIT          ;* 
	CMPA    #'x'            ;* START OF HEX SEQUENCE
	BEQ     SPHEX           ;* 
	CMPA    #'0'            ;* OCTAL SEQUENCE ?
	BLT     SPERR           ;* 
	CMPA    #'7'
	BLE     SPOCTAL         ;* YES

SPERR	DEX     BACKUP          ;* THE POINTER XXXXXXXX
	LDAA    0,X             ;* RESTORE CHAR
SPEXIT	RTS                     ;* RETURN

SPOCTAL	LDAB    #3              ;* XXXXXXXX
	STAB    DFLAG           ;* SAVE COUNTER
	CLRB                    ;* 

SPOCT1	SUBA    #'0'            ;* CONVERT TO DIGIT XXXXXXXX
	ASLB                    ;* SHIFT ACCUM
	ASLB                    ;* 
	ASLB                    ;* 
	ABA                     ;* ADD IN NEW DIGIT
	TAB                     ;* SAVE
	DEC     DFLAG           ;* 
	BEQ     SPBYE           ;* IF MAX COUNT--EXIT
	INX                     ;* 
	LDAA    0,X             ;* GET NEXT CHAR
	CMPA    #'0'            ;* VERIFY IF OCTAL
	BLT     SPFINI          ;* 
	CMPA    #'7'
	BLE     SPOCT1          ;* YES, CONTINUE

SPFINI	DEX     BACKUP          ;*
SPBYE	TBA                     ;* GET ACCUM CHAR XXXXXXXX
	RTS                     ;* 

SPHEX	LDAB    #2              ;* XXXXXXXX
	STAB    DFLAG           ;*
	CLRB                    ;* 

SPHEXL	INX                     ;* (LABEL/NM only?)
	LDAA    0,X             ;* GET NEXT CHAR
	CMPA    #'0'            ;* VERIFY IF HEX
	BLT     SPFINI          ;* ..
	CMPA    #'9'            ;* ..
	BLE     SPHEX2          ;* ..
	CMPA    #'A'            ;* ..
	BLT     SPFINI          ;* ..
	CMPA    #'F'            ;* ..
	BLE     SPHEX1          ;* ..
	CMPA    #'a'            ;* ..
	BLT     SPFINI          ;* ..
	CMPA    #'f'            ;* ..
	BGT     SPFINI          ;* ..
	SUBA    #$20            ;* YES IT IS HEX
SPHEX1	SUBA    #7              ;* XXXXXXXX
SPHEX2	SUBA    #'0'            ;* XXXXXXXX
	ASLB                    ;* 
	ASLB                    ;* 
	ASLB                    ;* 
	ASLB                    ;* 
	ABA                     ;* 
	TAB                     ;* 
	DEC     DFLAG           ;* 
	BEQ     SPBYE           ;* MAX COUNT REACHED ???
	BRA     SPHEXL          ;* NO, LOOP


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
isalpha	FCB     86              ;* switch to assembly
	TSX                     ;* 
	LDAB    3,X             ;* get char
	BSR     alPHA           ;* 
	JMP     RTSC            ;* 
*
*---------------------------------------------
*       Test if given char is numeric   *
*
* isdigit(c)
*       char c                    ;
* {     c=c&127                   ;
*       return((c>='0')&(c<='9')) ;
* }
*
isdigit	FCB     86              ;* 
	TSX                     ;* 
	LDAB    3,X             ;*
	BSR     nuMERIC         ;* 
	JMP     RTSC            ;* 
*
*----------------------------------------------
*       Test if given char is alphanumeric      *
*
* isalnum(c)
*       char c;
* {     return((alpha(c)|(numeric(c)));
* }
*
isalnum	FCB     86              ;* 
	TSX                     ;* 
	LDAB    3,X             ;* get char
	BSR     alPHA           ;* check if alpha
	TSX                     ;* 
	PSHB                    ;* save result
	LDAB    3,X             ;* get char again
	BSR     nuMERIC         ;* check if decimal
	TSX                     ;* 
	ORAB    0,X             ;* fix flag
	INS                     ;* clean stack
	JMP     RTSC            ;* 
*
*
alPHA	CLRA                    ;*
	ANDB    #$7F            ;*
	CMPB    #'a'            ;*
	BLT     alPHA1          ;* 
	CMPB    #'z'            ;* x
	BLE     alYES           ;* 
alPHA1	CMPB    #'A'            ;* XXXXXXXX
	BLT     alPHA2          ;* 
	CMPB    #'Z'            ;* x
	BLE     alYES           ;* 
alPHA2	CMPB    #'_'            ;* XXXXXXXX
	BEQ     alYES           ;* 
*
alNO	CLRB                    ;*
	RTS                     ;* 
*
alYES	LDAB    #1              ;* XXXXXXXX
	RTS                     ;* 
*
*
nuMERIC	CLRA                    ;*
	ANDB    #$7F            ;*
	CMPB    #'0'
	BLT     alNO            ;* 
	CMPB    #'9'
	BLE     alYES           ;* 
	BRA     alNO            ;* 
*
*-----------------------------------------------
*  islower(c)
*       char c;        returns TRUE if c is lower case alpha,
*                      FALSE otherwise.
*
islower	FCB     86              ;* SWITCH TO IN-LINE
	TSX                     ;* 
	LDAB    3,X             ;* GET CHAR
	ANDB    #$7F            ;*
	CLRA                    ;* 
	CMPB    #'a'
	BLT     ISNO            ;* 
	CMPB    #'z'
	BLE     ISYES           ;* 
*
ISNO	CLRB                    ;*
	JMP     RTSC            ;* RETURN FALSE
*
ISYES	LDAB                    ;* #1 XXXXXXXX
	JMP     RTSC            ;* RETURN TRUE
*
*----------------------------------------------
*  isupper(c)
*      char c;      return TRUE if c is upper case alpha.
*
isupper	FCB     86              ;* 
	TSX                     ;* 
	LDAB    3,X             ;*
	ANDB    #$7F            ;*
	CLRA                    ;* 
	CMPB    #'A'            ;*
	BLT     ISNO            ;* 
	CMPB    #'Z'            ;*
	BLE     ISYES           ;* 
	BRA     ISNO            ;* 
*
*-----------------------------------------------
*  isspace(c)
*      char c;       return TRUE if a "white space" char
*
isspace	FCB     86              ;* 
	TSX                     ;* 
	LDAB    3,X             ;* GET CHAR
	ANDB    #$7F            ;*
	CLRA                    ;* 
	CMPB    #' '            ;* SPACE ?
	BEQ     ISYES           ;* 
	CMPB    #$0D            ;* CR ???
	BEQ     ISYES           ;* 
	CMPB    #$0A            ;* LF ???
	BEQ     ISYES           ;* 
	CMPB    #$09            ;* HOR TAB ???
	BEQ     ISYES           ;* 
	BRA     ISNO            ;* 
*
*----------------------------------------------
*  toupper(c)
*      char c;   make c an upper case char if lower
*                          case alpha
*
toupper	FCB     86              ;* 
	TSX                     ;* 
	LDAB    3,X             ;*
	LDAA    2,X             ;*
	BNE     TOUPP1          ;* SKIP IF MSB'S NOT ZERO
	CMPB    #'a'            ;*
	BLT     TOUPP1          ;* 
	CMPB    #'z'            ;*
	BGT     TOUPP1          ;* 
	SUBB    #$20            ;* CONVERT TO UPPER CASE
TOUPP1	JMP     RTSC            ;*
*
*---------------------------------------------
*  tolower(c)
*      char c;   convert to lower case if upper case alpha.
*
tolower	FCB     86              ;* 
	TSX                     ;* 
	LDAB    3,X             ;*
	LDAA    2,X             ;*
	BNE     TOLOW1          ;* 
	CMPB    #'A'            ;*
	BLT     TOLOW1          ;* 
	CMPB    #'Z'            ;*
	BGT     TOLOW1          ;* 
	ADDB    #$20            ;* CONVERT TO LOWER CASE
TOLOW1	JMP     RTSC            ;*
*
*---------------------------------------------
*  strclr(s,n)
*      char *s;  int n;  clear a string of n bytes.
*
strclr	FCB     86              ;* 
	TSX                     ;* 
	LDAA    2,X             ;* GET LENGTH OF STRING
	LDAB    3,X             ;*
	LDX     4,X             ;* POINT TO STRING
	TSTB                    ;* 
	BEQ     SCLR2           ;* 

SCLR1	CLR     0,X             ;*
	INX                     ;* 
	DECB                    ;* 
	BNE     SCLR1           ;* 
SCLR2	TSTA                    ;*
	BEQ     SCLR3           ;* 
	DECA                    ;* 
	BRA     SCLR1           ;* 

SCLR3	JMP     RTSC            ;*
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
strlen	FCB     86              ;* 
	TSX                     ;* 
	LDX     2,X             ;* point to string
	CLRA                    ;* preset counter
	CLRB                    ;* 
*
strlLP	TST     0,X             ;* look for NULL XXXXXXXX
	BEQ     strlRT          ;* found !!
	INX                     ;* 
	ADDB    #1              ;* bump counter
	ADCA    #0
	BRA     strlLP          ;* 
*
strlRT	JMP     RTSC            ;*
*
*------------------------------------------------
*  strcpy(s1,s2)
*      char *s1, *s2;    copy s2 into s1.
*
strcpy	FCB     86              ;* 
	TSX                     ;* 
	LDX     4,X             ;* POINT TO S1
	STX     X1TMP           ;* SAVE POINTER
	TSX                     ;* 
	LDX     2,X             ;* POINT TO S2

SCPY1	LDAB    0,X             ;* XXXXXXXX
	INX                     ;* 
	STX     X2TMP           ;* 
	LDX     X1TMP           ;* 
	STAB    0,X             ;*
	BEQ     SCLR3           ;* END OF STRING ???
	INX                     ;* 
	STX     X1TMP           ;* 
	LDX     X2TMP           ;* 
	BRA     SCPY1           ;* 
*
*------------------------------------------------
*  strcat(s1,s2)
*      char *s1, *s2;   s2 is concatenated onto s1.
*
strcat	FCB     86              ;* 
	TSX                     ;* 
	LDX     2,X             ;* 
	STX     X2TMP           ;* SAVE POINTER TO S2
	TSX                     ;* 
	LDX     4,X             ;* POINT TO S1

SCAT1	TST     0,X             ;* LOOK FOR END OF STRING XXXXXXXX
	BEQ     SCAT2           ;* 
	INX                     ;* 
	BRA     SCAT1           ;* 

SCAT2	STX     X1TMP           ;* SAVE POINTER XXXXXXXX
	LDX     X2TMP           ;* 
	LDAB    0,X             ;*
	INX                     ;* 
	STX     X2TMP           ;* 
	LDX     X1TMP           ;* 
	STAB    0,X             ;*
	BEQ     SCAT3           ;* 
	INX                     ;* 
	BRA     SCAT2           ;* 

SCAT3	JMP     RTSC            ;*
*
*--------------------------------------------
*  strcmp(s1,s2)
*        char *s1, *s2;       returns:   0  if s1 = s2
*                                       <0  if s1 < s2
*                                       >0  if s1 > s2
*
strcmp	FCB     86              ;* 
	TSX                     ;* 
	LDX     2,X             ;* POINT TO S2
	STX     X2TMP           ;* 
	TSX                     ;* 
	LDX     4,X             ;* POINT TO S1

SCMP1	LDAA    0,X             ;* GET S1 CHAR XXXXXXXX
	BEQ     SCMP3           ;* 
	INX                     ;* 
	STX     X1TMP           ;* 
	LDX     X2TMP           ;* 
	LDAB    0,X
	BEQ     SCMP2           ;* 
	SBA                     ;* COMPARE BY SUBTRACTING
	BNE     SCMP3           ;* 
	INX                     ;* 
	STX     X2TMP           ;* 
	LDX     X1TMP           ;* 
	BRA     SCMP1           ;* 

SCMP2	CLRA                    ;*

SCMP3	TAB                     ;* (LABEL/NM only?)
	BMI     SCMP4           ;* 
	CLRA                    ;* 
	JMP     RTSC            ;* 

SCMP4	LDAA    $FF             ;* #$FF SIGN EXTEND XXXXXXXX
	JMP     RTSC            ;* 
*
*
*****************************************************
*
HERE	EQU  *                  ;* END OF INTERPRETER
* 
	END                     ;* 
*[ Fini ]***********************************************************************
