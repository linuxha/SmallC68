#asm
        NAM  SMALL-C RUN PACK FOR 6809

;*   LAST UPDATE   18-JAN-84

ROM     EQU  $8000
RAM     EQU  $9000
WARMS   EQU  $CD03

        ORG  RAM

PPC     RMB  2                  ;* PSEUDO PROGRAM COUNTER
R1A     RMB  1                  ;* WORKING 16 BIT
R1B     RMB  1                  ;* --REGISTER
DFLAG   RMB  1                  ;* DIVIDE ROUTINE FLAG

RAM     SET  *

;***************************************************

 	SETDP RAM/256

 	ORG  ROM

RUN     LDS  $CC2B
        LDA  #RAM/256
        TFR  A,DP
        LDX  #CODE
        BRA  NEXT2              ;* START THE INTERPRETATION

;**************************************************************
;*
;*  THE HEART OF THE INTERPRETER--- NEXT INSTRUCTION FETCHER.
;*
BUMP2   LDX  PPC                ;* GET PROG COUNTER
        LEAX 2,X                ;* INCR BY 2
        BRA  NEXT1              ;* FETCH NEXT INSTRUCTION

NEXT    LDX  PPC
NEXT1   STD  R1A                ;* SAVE THE WORK
NEXT2   BRA  *+3                ;* ROOM FOR TRACE
        NOP
        LDB  ,X+                ;* GET THE PSEUDO-INSTRUCTION
        STX  PPC                ;* SAVE NEW PPC
        LEAX JTABLE,PCR         ;* POINT TO ROUTINE (PC Relative)
        ABX
        LDB  R1B
        JMP  [0,X]              ;* GO EXECUTE THE PSEUDO-INSTR.

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
LD1IM   LDD  [PPC]
        BRA  BUMP2

;*-------------------------
;* #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF TFR S,D
        ADDD [PPC]
        BRA  BUMP2

;*-------------------------
;* #2  LOAD WORD @ ADDRESS
LD1     LDX  PPC
        LDD  [,X]               ;* GET WORD
        JMP  BUMP2

;*-------------------------
;* #3  LOAD BYTE @ ADDRESS
LDB1    LDX  PPC
        LDAB [,X]               ;* GET BYTE
        SEX                     ;* SIGN EXTEND
        JMP  BUMP2

;*-------------------------
;* #4  LOAD WORD INDIRECT (ADDR IN REG)
LD1R    LDD  [R1A]              ;* GET WORD
        JMP  NEXT

;*-------------------------
;* #5  LOAD BYTE INDIRECT (ADDR IN REG)
LDB1R   LDB  [R1A]
        SEX
        JMP  NEXT

;*-------------------------
;* #6  STORE WORD @ ADDRESS
ST1     STD  [PPC]
        JMP  BUMP2

;*-------------------------
;* #7  STORE BYTE @ ADDRESS
STB1    STB  [PPC]
        JMP  BUMP2

;*-------------------------
;* #8  STORE WORD @ ADDRESS ON STACK
ST1SP   STD  [,S++]
        JMP  NEXT

;*-------------------------
;* #9  STORE BYTE @ ADDRESS ON STACK
STB1SP  STB  [,S++]
        JMP  NEXT

;*-------------------------
;* #10  PUSH WORD ON STACK
PUSHR1  PSHS D
        LDX  PPC
        JMP  NEXT2

;*-------------------------
;* #11  SWAP REG AND TOP OF STACK
EXG1    PULS X
        EXG  D,X
        PSHS X
        LDX  PPC
        JMP  NEXT2

;*-------------------------
;* #12  JUMP TO LABEL
JMPL    LDX  [PPC]
        JMP  NEXT2

;*-------------------------
;* #13  JUMP TO LABEL IF FALSE
BRZL    ORAA R1B                ;* SET FLAGS
        BEQ  JMPL               ;* IF REG=0 -- JUMP
        JMP  BUMP2              ;* ELSE, PROCEED

;*-------------------------
;* #14  CALL TO LABEL
JSRL    LDX  PPC
        LEAX 2,X                ;* ADJUST RETURN ADDR
        PSHS X                  ;* PUSH RETURN ADDRESS
        BRA  JMPL

;*-------------------------
;* #15  CALL TO TOP OF STACK
JSRSP   PULS X
        LDD  PPC                ;* GET RETURN ADDRESS
        PSHS D                  ;* SAVE RETURN ADDRESS
        JMP  NEXT2

;*-------------------------
;* #16  RETURN TO CALLER
RTSC    PULS X                  ;* GET ADDRESS
        JMP  NEXT1

;*-------------------------
;* #17  MODIFY THE STACK POINTER
MODSP   LDD  [PPC]
        LEAS D,S
        LDD  R1A                ;* RESTORE REGISTER
        JMP  BUMP2

;*---------------------------
;* #18  DOUBLE THE PRIMARY REGISTER
DBL1    ASLD
        JMP  NEXT

;*---------------------------
;* #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS    ADDD ,S++               ;* DO THE ADD
        JMP  NEXT               ;* POP & RETURN

;*---------------------------
;* #20  SUBTRACT REG FROM TOP OF STACK
SUBFST  PULS D                  ;* GET VALUE OFF STACK
        SUBD R1A                ;* SUBTRACT REGISTER
        JMP  NEXT

;*---------------------------
;* #21  MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
MUL1    PSHS D
        LDAA #16
        PSHA                    ;* SET COUNTER
        CLRA
        CLRB

M2      ROR  3,S                ;* SHIFT MULTIPLIER
        ROR  4,S
        DEC  0,S                ;* DONE ?
        BMI  M4
        BCC  M3
        ADDD 1,S

M3      RORA
        RORB                    ;* SHIFT RESULT
        BRA  M2                 ;* AND LOOP

M4      LEAS 3,S
        PULS D                  ;* GET RESULT
        JMP  NEXT

;*-----------------------------
;* #22  DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
DIV1    BSR  BDIV               ;* DO THE BASIC DIVIDE
        LDAA DFLAG              ;* GET SIGN FLAG
        ANDA #1                 ;* MASK OFF BIT ZERO
        PULS D                  ;* GET RESULT
        BEQ  DIV1R

DIV1N   BSR  NEGATE             ;* NEGATE THE VALUE IN A,B

DIV1R   JMP  NEXT

;*-----------------------------
;* #23  DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
MOD     BSR  BDIV
        LEAS 2,S                ;* CLEAN STACK
        PSHS A                  ;* TEMP SAVE
        LDA  DFLAG              ;* GET SIGN FLAG
        BPL  MOD1
        COMA

MOD1    ANDA #1                 ;* MASK OFF BIT 0
        PULS A
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

BDIV1   PSHS D                  ;* FORCE ON STACK
        LDAA #17                ;* BIT COUNTER
        PSHA
        LDD  5,S                ;* CHECK SIGN
        BPL  BDIV2              ;* -- OF DIVIDEND

        COM  DFLAG              ;* ADJUST FLAG
        BSR  NEGATE
        STD  5,S

BDIV2   CLRA
        CLRB

;* MAIN DIVIDE LOOP (UNSIGNED)

UDIV1   CMPD 1,S
        BCC  UDIV3

UDIV2   CLC
        BRA  UDIV4

UDIV3   SUBD 1,S
        SEC

UDIV4   ROL  6,S
        ROL  5,S
        DEC  0,S
        BEQ  UDIV5

        ROLB
        ROLA
        BCC  UDIV1
        BRA  UDIV3

UDIV5   LEAS 3,S
        RTS

;*----------------------------------------
;* NEGATE THE VALUE IN A,B
NEGATE  NEGA
        NEGB
        SBCA #0
        RTS

;*----------------------------------
;* #24  INCLUSIVE OR THE TOP OF STACK AND REG.
ORS     ORA  ,S+
        ORB  ,S+
        JMP  NEXT

;*----------------------------------
;* #25  EXCLUSIVE OR ......
XORS    EORA ,S+
        EORB ,S+
        JMP  NEXT

;*----------------------------------
;* #26  AND .........
ANDS    ANDA ,S+
        ANDB ,S+
        JMP  NEXT

;*----------------------------------
;* #27  ARITH. SHIFT RIGHT THE TOP OF STACK
ASRS    ANDB #31                ;* MAX REASONABLE SHIFT
        BEQ  ASRS2

ASRS1   ASR  0,S
        ROR  1,S
        DECB
        BNE  ASRS1

ASRS2   PULS D                  ;* GET THE RESULT
        JMP NEXT

;*--------------------------------
;* #28  ARITH. SHIFT LEFT THE TOP OF STACK
ASLS    ANDB #31
        BEQ  ASRS2

ASLS1   ASL  1,S
        ROL  0,S
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
BCMP    LDD  2,S                ;* GET TOP OF STACK
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

FALSE   CLRB RETURN FALSE
        BRA  TRUE1

TRUE    LDAB #1 RETURN TRUE

TRUE1   CLRA
        LEAS 2,S
        JMP  NEXT

;*-------------------------------------
;* #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC    JMP  [PPC]              ;* EXECUTE CODE

CODE    EQU  *

        FCB  28                 ;* ASL
        FDB  main
        FCB  86                 ;* ???
        JMP  WARMS

#endasm

/* END of run-time package */
/* -*- mode: asm-mode; -*- */
