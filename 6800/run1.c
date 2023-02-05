#asm
        NAM  SMALL-C RUN PACK FOR 6801/6301

RAM     EQU  $1000
ROM     EQU  $F000

;*   LAST UPDATE   16-JAN-84

        ORG  $80
;*
PPC     RMB  2                  ;* PSEUDO PROGRAM COUNTER
R1A     RMB  1                  ;* WORKING 16 BIT
R1B     RMB  1                  ;* --REGISTER
DFLAG   RMB  1                  ;* DIVIDE ROUTINE FLAG
STEMP   RMB  2                  ;* TEMP STORAGE FOR STACK POINTER

;***************************************************

 	ORG  ROM

RUN     LDS  #$FF
        STS  STEMP
        LDX  #CODE
        BRA  NEXT2              ;* START THE INTERPRETATION

;**************************************************************
;*
;*  THE HEART OF THE INTERPRETER--- NEXT INSTRUCTION FETCHER.
;*
BUMP2   LDX  PPC                ;* GET PROG COUNTER
BUMP2A  INX  INCR               ;* BY 2
        INX
        BRA  NEXT1              ;* FETCH NEXT INSTRUCTION

NEXT    LDX  PPC
NEXT1   STD  R1A                ;* SAVE THE WORK
NEXT2   LDAB 0,X                ;* GET THE PSEUDO-INSTRUCTION
        INX                     ;* (B CONTAINS A TABLE OFFSET)
        STX  PPC                ;* SAVE NEW PPC
        LDX  #JTABLE            ;* POINT TO ROUTINE
        ABX
        LDAB R1B
        LDX  0,X
        JMP  0,X                ;* GO EXECUTE THE PSEUDO-INSTR.

;**************************************************************
;*                  THE JUMP TABLE                            *
;**************************************************************

JTABLE  FDB  LD1IM              ;* #0  LOAD REG WITH IMMED. VALUE
        FDB  LD1SOFF            ;* #1  LOAD STACK ADDRESS + OFFSET INTO REG
        FDB  LD1                ;* #2  LOAD WORD @ ADDRESS
        FDB  LDB1               ;* #3  LOAD BYTE @ ADDRESS
        FDB  LD1R               ;* #4  LOAD WORD INDIRECT (ADDR IN REG)
        FDB  LDB1R              ;* #5  LOAD BYTE INDIRECT (ADDR IN REG)
        FDB  ST1                ;* #6  STORE WORD @ ADDRESS
        FDB  STB1               ;* #7  STORE BYTE @ ADDRESS
        FDB  ST1SP              ;* #8  STORE WORD @ ADDRESS ON STACK
        FDB  STB1SP             ;* #9  STORE BYTE @ ADDRESS ON STACK
        FDB  PUSHR1             ;* #10 PUSH WORD ON STACK
        FDB  EXG1               ;* #11 SWAP REG AND TOP OF STACK
        FDB  JMPL               ;* #12 JUMP TO LABEL
        FDB  BRZL               ;* #13 JUMP TO LABEL IF FALSE
        FDB  JSRL               ;* #14 CALL TO LABEL
        FDB  JSRSP              ;* #15 CALL TO TOP OF STACK
        FDB  RTSC               ;* #16 RETURN TO CALLER
        FDB  MODSP              ;* #17 MODIFY THE STACK POINTER
        FDB  DBL1               ;* #18 DOUBLE THE PRIMARY REGISTER
        FDB  ADDS               ;* #19 ADD REG AND TOP OF STACK (THEN POP)
        FDB  SUBFST             ;* #20 SUBTRACT REG FROM TOP OF STACK
        FDB  MUL1               ;* #21 MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
        FDB  DIV1               ;* #22 DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
        FDB  MOD                ;* #23 DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
        FDB  ORS                ;* #24 INCLUSIVE OR THE TOP OF STACK AND REG.
        FDB  XORS               ;* #25 EXCLUSIVE OR ......
        FDB  ANDS               ;* #26 AND .........
        FDB  ASRS               ;* #27 ARITH. SHIFT RIGHT THE TOP OF STACK
        FDB  ASLS               ;* #28 ARITH. SHIFT LEFT THE TOP OF STACK
        FDB  NEGR               ;* #29 NEGATE THE REGISTER
        FDB  NOTR               ;* #30 COMPLEMENT THE REGISTER
        FDB  INCR               ;* #31 ADD 1 TO REG
        FDB  DECR               ;* #32 SUBTRACT 1 FROM REG
        FDB  ZEQ                ;* #33 TEST FOR EQUALITY
        FDB  ZNE                ;* #34 TEST FOR NOT-EQUAL
        FDB  ZLT                ;* #35 TEST FOR LESS THAN
        FDB  ZLE                ;* #36 TEST FOR LESS THAN OR EQUAL
        FDB  ZGT                ;* #37 TEST FOR GREATER THAN
        FDB  ZGE                ;* #38 TEST FOR GREATER THAN OR EQUAL
        FDB  ULT                ;* #39 TEST FOR LESS THAN (UNSIGNED)
        FDB  ULE                ;* #40 TEST FOR LESS THAN OR EQUAL (UNSIGNED)
        FDB  UGT                ;* #41 TEST FOR GREATER THAN (UNSIGNED)
        FDB  UGE                ;* #42 TEST FOR GREATER THAN OR EQUAL (UNSIGNED)
        FDB  ASMC               ;* #43 SWITCH TO EXECUTABLE (ASSEMBLY) CODE

;*************************************************************
;*-------------------------
;* #0 LOAD REG WITH IMMED. VALUE
LD1IM   LDX  PPC
        LDD  0,X                ;* HIGH BYTE
        JMP  BUMP2A

;*-------------------------
;* #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF TSX
        STX  R1A                ;* SAVE STACK VALUE
        LDX  PPC                ;*
        LDD  0,X                ;* GET OFFSET
        ADDD R1A                ;* ADD OFFSET
        JMP  BUMP2A

;*-------------------------
;* #2  LOAD WORD @ ADDRESS
LD1     LDX  PPC
        LDX  0,X                ;* GET ADDRESS
LD1A    LDD  0,X                ;* GET WORD
        JMP  BUMP2

;*-------------------------
;* #3  LOAD BYTE @ ADDRESS
LDB1    LDX  PPC
        LDX  0,X                ;* GET ADDRESS
        CLRA
        LDAB 0,X                ;* GET BYTE
        BPL  LDB1A
        COMA                    ;* SIGN EXTEND
LDB1A   JMP  BUMP2

;*-------------------------
;* #4  LOAD WORD INDIRECT (ADDR IN REG)
LD1R    LDX  R1A                ;* GET ADDRESS
        LDD  0,X                ;* GET WORD
        JMP  NEXT

;*-------------------------
;* #5  LOAD BYTE INDIRECT (ADDR IN REG)
LDB1R   LDX  R1A
        CLRA
        LDAB 0,X                ;* GET BYTE
        BPL  LDB1RA
        COMA
LDB1RA  JMP  NEXT

;*-------------------------
;* #6  STORE WORD @ ADDRESS
ST1     LDX  PPC
        LDX  0,X                 ;* GET ADDRESS
        STD  0,X                 ;* STORE WORD
        JMP  BUMP2

;*-------------------------
;* #7  STORE BYTE @ ADDRESS
STB1    LDX  PPC
        LDX  0,X                ;* GET ADDR
        STAB 0,X                ;* STORE BYTE
        JMP  BUMP2

;*-------------------------
;* #8  STORE WORD @ ADDRESS ON STACK
ST1SP   TSX                     ;* STACK TO INDEX
        LDX  0,X                ;* GET ADDRESS
        STD  0,X                ;* STORE WORD
        INS
        INS                     ;* POP STACK
        JMP  NEXT

;*-------------------------
;* #9  STORE BYTE @ ADDRESS ON STACK
STB1SP  TSX
        LDX  0,X
        STAB 0,X                ;* STORE BYTE
        INS                     ;* POP ...
        INS
        JMP  NEXT

;*-------------------------
;* #10  PUSH WORD ON STACK
PUSHR1  PSHB
        PSHA
        LDX  PPC
        JMP  NEXT2

;*-------------------------
;* #11  SWAP REG AND TOP OF STACK
EXG1    PULX
        STX  R1A                ;* SAVE
        PSHB
        PSHA REG                ;* ON STACK
        LDD  R1A                ;* NEW REG
        LDX  PPC
        JMP  NEXT2

;*-------------------------
;* #12  JUMP TO LABEL
JMPL    LDX  PPC
JMP1    LDX  0,X                ;* GET ADDRESS (NEW PPC)
        JMP  NEXT2

;*-------------------------
;* #13  JUMP TO LABEL IF FALSE
BRZL    ORAA R1B                ;* SET FLAGS
        BEQ  JMPL               ;* IF REG=0 -- JUMP
        JMP  BUMP2              ;* ELSE, PROCEED

;*-------------------------
;* #14  CALL TO LABEL
JSRL    LDX  PPC
        INX                     ;* ADJUST RETURN
        INX                     ;* -- ADDRESS
        PSHX                    ;* PUSH RETURN ADDRESS
        BRA  JMPL

;*-------------------------
;* #15  CALL TO TOP OF STACK
JSRSP   PULX
        LDD  PPC                ;* GET RETURN ADDRESS
        PSHB SAVE               ;* RETURN ADDRESS
        PSHA
        JMP  NEXT2

;*-------------------------
;* #16  RETURN TO CALLER
RTSC    PULX                    ;* GET ADDRESS
        JMP  NEXT1

;*-------------------------
;* #17  MODIFY THE STACK POINTER
MODSP   LDX  PPC
        LDD  0,X                ;* GET VALUE
        STS  STEMP
        ADDD STEMP              ;* ADD STACK POINTER
        STD  STEMP
        LDS  STEMP              ;* NEW STACK POINTER
        LDD  R1A                ;* RESTORE REGISTER
        JMP  BUMP2A

;*---------------------------
;* #18  DOUBLE THE PRIMARY REGISTER
DBL1    ASLD
        JMP  NEXT

;*---------------------------
;* #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS    TSX
        ADDD 0,X                ;* DO THE ADD
        JMP  POPS               ;* POP & RETURN

;*---------------------------
;* #20  SUBTRACT REG FROM TOP OF STACK
SUBFST  PULA                    ;* GET VALUE OFF STACK
        PULB
        SUBD R1A                ;* SUBTRACT REGISTER
        JMP  NEXT

;*---------------------------
;* #21  MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
MUL1    PSHB
        PSHA                    ;* REG ON STACK
        LDAA #16                ;*
        PSHA                    ;* SET COUNTER
        CLRA
        CLRB
        TSX                     ;* POINT TO DATA

M2      ROR  3,X                 ;* SHIFT MULTIPLIER
        ROR  4,X
        DEC  0,X                ;* DONE ?
        BMI  M4
        BCC  M3
        ADDD 1,X

M3      RORA
        RORB                    ;* SHIFT RESULT
        BRA  M2                 ;* AND LOOP

M4      INS                     ;* CLEAN STACK
        INS
        INS
        PULA                    ;* GET RESULT
        PULB
        JMP  NEXT

;*-----------------------------
;* #22  DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
DIV1    BSR  BDIV               ;* DO THE BASIC DIVIDE
        LDAA DFLAG              ;* GET SIGN FLAG
        ANDA #1                 ;* MASK OFF BIT ZERO
        PULA                    ;* GET RESULT
        PULB
        BEQ  DIV1R

DIV1N   BSR  NEGATE             ;* NEGATE THE VALUE IN A,B

DIV1R   JMP NEXT

;*-----------------------------
;* #23  DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
MOD     BSR  BDIV
        INS                     ;* CLEAN STACK
        INS
        PSHA                    ;* TEMP SAVE
        LDAA DFLAG              ;* GET SIGN FLAG
        BPL  MOD1
        COMA

MOD1    ANDA #1                 ;* MASK OFF BIT 0
        PULA
        BNE  DIV1N              ;* IF BIT 0 SET, NEGATE

        JMP  NEXT

;*****************************************************
;*   BASIC 16 BIT DIVIDE ROUTINE
;* ENTER WITH: DIVIDEND ON STACK
;*             DIVISOR IN A,B
;* EXIT WITH:  QUOTIENT ON STACK
;*             REMAINDER IN A,B
;*             SIGN FLAG IN DFLAG
;*
BDIV    CLR  DFLAG
        TSTA                    ;* CHECK DIVISOR SIGN
        BPL  BDIV1

        INC  DFLAG              ;* ADJUST SIGN FLAG
        BSR  NEGATE             ;* TAKE ABSOLUTE VALUE

BDIV1   PSHB                    ;* FORCE ON STACK
        PSHA
        LDAA #17                ;* BIT COUNTER
        PSHA
        TSX                     ;* POINT TO DATA
        LDD  5,X                ;* CHECK SIGN
        BPL  BDIV2              ;* -- OF DIVIDEND

        COM  DFLAG              ;* ADJUST FLAG
        BSR  NEGATE
        STD  5,X

BDIV2   CLRA
        CLRB

;* MAIN DIVIDE LOOP (UNSIGNED)

UDIV1   CMPA 1,X
        BHI  UDIV3
        BCS  UDIV2
        CMPB 2,X
        BCC  UDIV3

UDIV2   CLC
        BRA  UDIV4

UDIV3   SUBD 1,X
        SEC

UDIV4   ROL  6,X
        ROL  5,X
        DEC  0,X
        BEQ  UDIV5

        ROLB
        ROLA
        BCC  UDIV1
        BRA  UDIV3

UDIV5   INS
        INS
        INS
        RTS

;*----------------------------------------
;* NEGATE THE VALUE IN A,B
NEGATE  NEGA
        NEGB
        SBCA #0
        RTS

;*----------------------------------
;* #24  INCLUSIVE OR THE TOP OF STACK AND REG.
ORS     TSX
        ORAA 0,X
        ORAB 1,X
POPS    INS                     ;* POP THE STACK
        INS
        JMP  NEXT

;*----------------------------------
;* #25  EXCLUSIVE OR ......
XORS    TSX
        EORA 0,X
        EORB 1,X
        BRA  POPS

;*----------------------------------
;* #26  AND .........
ANDS    TSX
        ANDA 0,X
        ANDB 1,X
        BRA  POPS

;*----------------------------------
;* #27  ARITH. SHIFT RIGHT THE TOP OF STACK
ASRS    TSX
        ANDB #$1F               ;* MAX REASONABLE SHIFT
        BEQ  ASRS2

ASRS1   ASR  0,X
        ROR  1,X
        DECB
        BNE  ASRS1

ASRS2   PULA                    ;* GET THE RESULT
        PULB
        JMP  NEXT

;*--------------------------------
;* #28  ARITH. SHIFT LEFT THE TOP OF STACK
ASLS    TSX
        ANDB #$1F
        BEQ  ASRS2

ASLS1   ASL  1,X
        ROL  0,X
        DECB
        BNE  ASLS1

        BRA  ASRS2

;*--------------------------------
;* #29  NEGATE THE REGISTER
NEGR    BSR  NEGATE
        JMP  NEXT

;*--------------------------------
;* #30  COMPLEMENT THE REGISTER
NOTR    COMA
        COMB
        JMP  NEXT

;*--------------------------------
;* #31  ADD 1 TO REG
INCR    ADDD #1
        JMP  NEXT

;*--------------------------------
;* #32  SUBTRACT 1 FROM REG
DECR    SUBD #1
        JMP  NEXT

;*****************************************************
;*
;*   BASIC COMPARE INSTRUCTION SUBROUTINE
;*   Compare the top of Stack to Register and set Condition codes
;*
;*  Unsigned compare, Carry set if top of stack < A,B
;*
BCMP    TSX
        LDD  2,X                ;* GET TOP OF STACK
        SUBD R1A                ;* COMPARE
        RTS


;*-------------------------------
;* #33  TEST FOR EQUALITY
ZEQ     BSR  BCMP
        BEQ  TRUE
        BRA  FALSE

;*-------------------------------
;* #34  TEST FOR NOT-EQUAL
ZNE     BSR  BCMP
        BNE  TRUE
        BRA  FALSE

;*-------------------------------
;* #35  TEST FOR LESS THAN
ZLT     BSR  BCMP
        BLT  TRUE
        BRA  FALSE

;*-------------------------------
;* #36  TEST FOR LESS THAN OR EQUAL
ZLE     BSR  BCMP
        BLE  TRUE
        BRA  FALSE

;*-------------------------------
;* #37  TEST FOR GREATER THAN
ZGT     BSR  BCMP
        BGT  TRUE
        BRA  FALSE

;*-------------------------------
;* #38  TEST FOR GREATER THAN OR EQUAL
ZGE     BSR  BCMP
        BGE  TRUE
        BRA  FALSE

;*-------------------------------
;* #39  TEST FOR LESS THAN (UNSIGNED)
ULT     BSR  BCMP
        BCS  TRUE
        BRA  FALSE

;*-------------------------------
;* #40  TEST FOR LESS THAN OR EQUAL (UNSIGNED)
ULE     BSR  BCMP
        BLS  TRUE
        BRA  FALSE

;*-------------------------------
;* #41  TEST FOR GREATER THAN (UNSIGNED)
UGT     BSR  BCMP
        BHI  TRUE
        BRA  FALSE

;*------------------------------
;* #42  TEST FOR GREATER THAN OR EQUAL (UNSIGNED)
UGE     BSR  BCMP
        BCC  TRUE

FALSE   CLRB                    ;* RETURN FALSE
        BRA  TRUE1

TRUE    LDAB #1                 ;* RETURN TRUE

TRUE1   CLRA
        JMP                     ;* POPS POP STACK AND PROCEED

;*-------------------------------------
;* #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC    LDX  PPC                ;* POINT TO CODE
        JMP  0,X                ;* GO EXECUTE IT

WARMS 	BRA *

CODE 	EQU *

#endasm

/* -*- mode: asm-mode; -*- */
