/* Local Variables: */
/* mode: asm        */
/* End:             */
#asm
	MACEXP  on

        CPU     6800            ; That's what asl has

;;;
;;; WIP, need to convert this tfrom 6801 to 6800 @FIXME: replace 6801 op codes
;;;
;        NAM SMALL-C RUN PACK FOR 6800/6802

STACK   EQU     $FF             ;* Our stack
BASERAM EQU     $80             ;* VM Ram

RAM     EQU     $1000
ROM     EQU     $F100           ;* Simulator

;*   LAST UPDATE   20240518 ncherry@linuxha.com

        ORG     BASERAM
;*
zPC     RMB     2               ;* PSEUDO PROGRAM COUNTER
zREG    RMB     2               ;* WORKING 16 BIT (was R1A)
;1B     RMB     1               ;* --REGISTER     (was R1B)
DFLAG   RMB     1               ;* DIVIDE ROUTINE FLAG
STEMP   RMB     2               ;* TEMP STORAGE FOR STACK POINTER

XTEMP   RMB     2
ATEMP   RMB     1
BTEMP   RMB     1

;***************************************************

        ORG     ROM

RUN     LDS     #STACK          ;* @FIXME: Not sure 0 page is a good idea yet
        STS     STEMP
        ;; 
        clra
        clrb
        staa    zPC
        staa    zPC+1
        staa    zREG
        staa    zREG+1
        staa    DFLAG
        staa    STEMP
        staa    STEMP+1
        staa    XTEMP
        staa    XTEMP+1
        ;; 
        LDX     #CODE
        BRA     NEXT2           ;* START THE INTERPRETATION

;**************************************************************
;*
;*  THE HEART OF THE INTERPRETER--- NEXT INSTRUCTION FETCHER.
;*
BUMP2   LDX     zPC             ;* GET PROG COUNTER
BUMP2A  INX                     ;* INCR BY 2
        INX
        BRA     NEXT1           ;* FETCH NEXT INSTRUCTION

NEXT    LDX     zPC
;EXT1   std     zREG            ;* SAVE THE WORK
NEXT1   staa    zREG            ;* SAVE THE WORK
        stab    zREG+1          ;*
NEXT2   LDAB    0,X             ;* GET THE PSEUDO-INSTRUCTION (B CONTAINS A TABLE OFFSET)
        INX                     ;* INC zPC
        STX     zPC             ;* SAVE NEW zPC
        LDX     #JTABLE         ;* POINT TO ROUTINE
;*
;* ABX -> X <- X + B
;*
        stx     XTEMP           ;* Save X to XTEMP
        clra
        addb    XTEMP+1         ;* add X lo to B -> B
        adca    XTEMP           ;* add the X hi + CC to A ->
        staa    XTEMP           ;* A -> XTEMP
        stab    XTEMP+1         ;* B -> XTEMP+1
        ldx     XTEMP           ;* retore new X (JTable+B offset)
;*
        LDAB    zREG+1          ;* What does B hold?
        LDX     0,X
        JMP     0,X             ;* Jump to function

;**************************************************************
;*                  THE JUMP TABLE                            *
;**************************************************************

;*              zOP Code           Pos  BYTE value
JTABLE  FDB     LD1IM           ;*  #0  #0  immediate
        FDB     LD1SOFF         ;*  #1  #2  getloc
        FDB     LD1             ;*  #2  #4  getmem
        FDB     LDB1            ;*  #3  #6  getmem
        FDB     LD1R            ;*  #4  #8  indirect
        FDB     LDB1R           ;*  #5  #10 indirect
        FDB     ST1             ;*  #6  #12 putmem
        FDB     STB1            ;*  #7  #14 putmem
        FDB     ST1SP           ;*  #8  #16 putstk
        FDB     STB1SP          ;*  #9  #18 putstk
        FDB     PUSHR1          ;*  #10 #20 zpush
        FDB     EXG1            ;*  #11 #22 swapstk
        FDB     JMPL            ;*  #12 #24 jump
        FDB     BRZL            ;*  #13 #26 testjump
        FDB     JSRL            ;*  #14 #28 zcall
        FDB     JSRSP           ;*  #15 #30 callstk
        FDB     RTSC            ;*  #16 #32 zret
        FDB     MODSP           ;*  #17 #34 modstk
        FDB     DBL1            ;*  #18 #36 doublereg
        FDB     ADDS            ;*  #19 #38 zadd
        FDB     SUBFST          ;*  #20 #40 zsub
        FDB     MUL1            ;*  #21 #42 mult
        FDB     DIV1            ;*  #22 #44 myDiv
        FDB     MOD             ;*  #23 #46 zmod
        FDB     ORS             ;*  #24 #48 zor
        FDB     XORS            ;*  #25 #50 zxor
        FDB     ANDS            ;*  #26 #52 zand
        FDB     ASRS            ;*  #27 #54 asr
        FDB     ASLS            ;*  #28 #56 asl
        FDB     NEGR            ;*  #29 #58 neq
        FDB     NOTR            ;*  #30 #60 com
        FDB     INCR            ;*  #31 #62 inc
        FDB     DECR            ;*  #32 #64 dec
        FDB     ZEQ             ;*  #33 #66 
        FDB     ZNE             ;*  #34 #68
        FDB     ZLT             ;*  #35 #70
        FDB     ZLE             ;*  #36 #72
        FDB     ZGT             ;*  #37 #74
        FDB     ZGE             ;*  #38 #76
        FDB     ULT             ;*  #39 #78
        FDB     ULE             ;*  #40 #80
        FDB     UGT             ;*  #41 #82
        FDB     UGE             ;*  #42 #84
        FDB     ASMC            ;*  #43 #86

;*************************************************************
;*-------------------------
;* #0 LOAD REG WITH IMMED. VALUE
;* zPC is the immediate value
LD1IM   LDX     zPC
;       LDD     0,X             ;* HIGH BYTE
        ldaa    0,X             ;* HIGH BYTE
        ldab    1,X             ;* HIGH BYTE
        JMP     BUMP2A

;*-------------------------
;* #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF TSX
        STX     zREG            ;* SAVE STACK VALUE
        LDX     zPC
;       LDD     0,X             ;* GET OFFSET
        ldaa    0,X             ;* GET OFFSET
        ldab    1,X             ;* GET OFFSET
;       ADDD    zREG            ;* ADD OFFSET
        addb    zREG+1           ;* ADD OFFSET
        adca    zREG            ;* ADD OFFSET
        JMP     BUMP2A

;*-------------------------
;* #2  LOAD WORD @ ADDRESS
LD1     LDX     zPC
        LDX     0,X             ;* GET ADDRESS
;D1A    LDD     0,X             ;* GET WORD
LD1A    ldaa    0,X             ;* GET WORD
        ldab    1,X             ;* GET WORD
        JMP     BUMP2

;*-------------------------
;* #3  LOAD BYTE @ ADDRESS
LDB1    LDX     zPC
        LDX     0,X             ;* GET ADDRESS
        CLRA
        LDAB    0,X             ;* GET BYTE
        BPL     LDB1A
        COMA                    ;* SIGN EXTEND
LDB1A   JMP     BUMP2

;*-------------------------
;* #4  LOAD WORD INDIRECT (ADDR IN REG)
LD1R    LDX     zREG            ;* GET ADDRESS
;       LDD     0,X             ;* GET WORD
        ldaa    0,X             ;* GET WORD
        ldab    1,X             ;* GET WORD
        JMP     NEXT

;*-------------------------
;* #5  LOAD BYTE INDIRECT (ADDR IN REG)
LDB1R   LDX     zREG
        CLRA
        LDAB    0,X             ;* GET BYTE
        BPL     LDB1RA
        COMA
LDB1RA  JMP     NEXT

;*-------------------------
;* #6  STORE WORD @ ADDRESS
ST1     LDX     zPC
        LDX     0,X             ;* GET ADDRESS
;       STD     0,X             ;* STORE WORD
        staa    0,X             ;* STORE WORD
        stab    1,X             ;* STORE WORD
        JMP     BUMP2

;*-------------------------
;* #7  STORE BYTE @ ADDRESS
STB1    LDX     zPC
        LDX     0,X             ;* GET ADDR
        STAB    0,X             ;* STORE BYTE
        JMP     BUMP2

;*-------------------------
;* #8  STORE WORD @ ADDRESS ON STACK
ST1SP   TSX                     ;* STACK TO INDEX
        LDX     0,X             ;* GET ADDRESS
;       STD     0,X             ;* STORE WORD
        staa    0,X             ;* STORE WORD
        stab    1,X             ;* STORE WORD
        INS
        INS                     ;* POP STACK
        JMP     NEXT

;*-------------------------
;* #9  STORE BYTE @ ADDRESS ON STACK
STB1SP  TSX
        LDX     0,X
        STAB    0,X             ;* STORE BYTE
        INS                     ;* ... POP
        INS
        JMP     NEXT

;*-------------------------
;* #10  PUSH WORD ON STACK
PUSHR1  PSHB                    ;* A & B on the stack
        PSHA
        LDX     zPC
        JMP     NEXT2

;*-------------------------
;* #11  SWAP REG AND TOP OF STACK
;XG1    PULX                    ;* @FIXME: Need a proper pull
EXG1    staa    ATEMP           ;* GET ADDRESS
        stab    BTEMP           ;* Save B
        pula                    ;* Get Address of stack
        pulb                    ;*
        staa    XTEMP           ;* Save A to XTEMP
        stab    XTEMP+1         ;* Save B to XTEMP+1
        ldx     XTEMP           ;* Get Address and put it in X
        ldaa    ATEMP           ;* Restore A
        ldab    BTEMP           ;* Restore B
;
        STX     zREG            ;* SAVE
        PSHB
        PSHA                    ;* REG ON STACK
;       LDD     zREG            ;* NEW REG
        ldaa    zREG            ;* NEW REG
        ldab    zREG+1          ;* NEW REG
        LDX     zPC
        JMP     NEXT2

;*-------------------------
;* #12  JUMP TO LABEL
JMPL    LDX     zPC
JMP1    LDX     0,X             ;* GET ADDRESS (NEW zPC)
        JMP     NEXT2

;*-------------------------
;* #13  JUMP TO LABEL IF FALSE
BRZL    ORAA    zREG+1          ;* SET FLAGS
        BEQ     JMPL            ;* IF REG=0 -- JUMP
        JMP     BUMP2           ;* ELSE, PROCEED

;*-------------------------
;* #14  CALL TO LABEL
JSRL    LDX     zPC
        INX                     ;* ADJUST RETURN
        INX                     ;* -- ADDRESS
;       PSHX                    ;* PUSH RETURN ADDRESS @FIXME: Need a proper push
        staa    ATEMP           ;* Save D [A:B]
        stab    BTEMP           ;* 
        stx     XTEMP           ;* Copy X to XTEMP
        ldaa    XTEMP           ;* Load XTEMP into D [A:B]
        ldab    XTEMP+1         ;* 
        pshb                    ;* Push X [A:B] onto the stack
        psha
        ldaa    ATEMP           ;* Restore D [A:B]
        ldab    BTEMP
;       PSHX                    ;* PUSH RETURN ADDRESS @FIXME: Need a proper push
        BRA     JMPL

;*-------------------------
;* #15  CALL TO TOP OF STACK
;SRSP   PULX                    ;*
JSRSP   staa    ATEMP           ;* Save A
        stab    BTEMP           ;* Save B
        pula                    ;* Get X off the stack
        pulb                    ;*
        staa    XTEMP           ;* Save A to XTEMP
        stab    XTEMP+1         ;* Save B to XTEMP+1
        ldx     XTEMP           ;* Get address off the stack
        ldaa    ATEMP           ;* Restore A
        ldab    BTEMP           ;* Restore B
;                               ;*
;       LDD     zPC             ;* GET RETURN ADDRESS
        ldaa    zPC             ;* GET RETURN ADDRESS
        ldab    zPC+1           ;* 
        PSHB                    ;* SAVE RETURN ADDRESS
        PSHA
        JMP     NEXT2

;*-------------------------
;* #16  RETURN TO Small C CALLER
;TSC    PULX                    ;* GET ADDRESS
RTSC    staa    ATEMP           ;* GET ADDRESS
        stab    BTEMP           ;* Save B
        pula                    ;* Get Address of stack
        pulb                    ;*
        staa    XTEMP           ;* Save A to XTEMP
        stab    XTEMP+1         ;* Save B to XTEMP+1
        ldx     XTEMP           ;* Get Address and put it in X
        ldaa    ATEMP           ;* Restore A
        ldab    BTEMP           ;* Restore B
;*
        JMP     NEXT1

;*-------------------------
;* #17  MODIFY THE STACK POINTER
MODSP   LDX     zPC
;       LDD     0,X             ;* GET VALUE
        ldaa    0,X             ;* GET VALUE
        ldab    1,X             ;* GET VALUE
        STS     STEMP
;       ADDD    STEMP           ;* ADD STACK POINTER
        addb    STEMP+1         ;* ADD STACK POINTER
        adca    STEMP           ;* ADD STACK POINTER
;       STD     STEMP           ;*
        staa    STEMP
        stab    STEMP+1
        LDS     STEMP           ;* NEW STACK POINTER
;       LDD     zREG            ;* RESTORE REGISTER
        ldaa    zREG            ;* RESTORE REGISTER
        ldab    zREG+1          ;* RESTORE REGISTER
        JMP     BUMP2A

;*---------------------------
;* #18  DOUBLE THE PRIMARY REGISTER
;BL1    ASLD
DBL1    asla                    ;* May not match the asld flag settings
        rolb
        JMP     NEXT

;*---------------------------
;* #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS    TSX
;       ADDD    0,X              ;* DO THE ADD
        addb    1,X             ;* DO THE ADD
        adca    0,X             ;* DO THE ADD
        JMP     POPS            ;* POP & RETURN

;*---------------------------
;* #20  SUBTRACT REG FROM TOP OF STACK
SUBFST  PULA                    ;* GET VALUE OFF STACK
        PULB
;       SUBD    zREG            ;* SUBTRACT REGISTER
        subb    zREG+1          ;* SUBTRACT REGISTER
        sbca    zREG            ;* SUBTRACT REGISTER
        JMP     NEXT

;*---------------------------
;* #21  MULTIPLY TOP OF STACK BY REG (RESULT IN REG)
MUL1    PSHB
        PSHA                    ;* REG ON STACK
        LDAA    #16
        PSHA                    ;* SET COUNTER
        CLRA
        CLRB
        TSX                     ;* POINT TO DATA

M2      ROR     3,X             ;* SHIFT MULTIPLIER
        ROR     4,X
        DEC     0,X             ;* DONE ?
        BMI     M4
        BCC     M3
;       ADDD    1,X
        addb    2,X
        adca    1,X

M3      RORA
        RORB                    ;* SHIFT RESULT
        BRA     M2              ;* AND LOOP

M4      INS                     ;* CLEAN STACK
        INS
        INS
        PULA                    ;* GET RESULT
        PULB
        JMP     NEXT

;*-----------------------------
;* #22  DIVIDE THE TOP OF STACK BY REG --- RESULT IN REG.
DIV1    BSR     BDIV            ;* DO THE BASIC DIVIDE
        LDAA    DFLAG           ;* GET SIGN FLAG
        ANDA    #1              ;* MASK OFF BIT ZERO
        PULA                    ;* GET RESULT
        PULB
        BEQ     DIV1R

DIV1N   BSR     NEGATE          ;* NEGATE THE VALUE IN A,B

DIV1R   JMP NEXT

;*-----------------------------
;* #23  DIVIDE TOP OF STACK BY REG --- REMAINDER IN REG
MOD     BSR     BDIV
        INS                     ;* CLEAN STACK
        INS
        PSHA                    ;* TEMP SAVE
        LDAA    DFLAG           ;* GET SIGN FLAG
        BPL     MOD1
        COMA

MOD1    ANDA    #1              ;* MASK OFF BIT 0
        PULA
        BNE     DIV1N           ;* IF BIT 0 SET, NEGATE

        JMP     NEXT

;*****************************************************
;*   BASIC 16 BIT DIVIDE ROUTINE
;* ENTER WITH: DIVIDEND ON STACK
;*             DIVISOR IN A,B
;* EXIT WITH:  QUOTIENT ON STACK
;*             REMAINDER IN A,B
;*             SIGN FLAG IN DFLAG
;*
BDIV    CLR     DFLAG
        TSTA                    ;* CHECK DIVISOR SIGN
        BPL     BDIV1

        INC     DFLAG           ;* ADJUST SIGN FLAG
        BSR     NEGATE          ;* TAKE ABSOLUTE VALUE

BDIV1   PSHB                    ;* FORCE ON STACK
        PSHA
        LDAA    #17             ;* BIT COUNTER
        PSHA
        TSX                     ;* POINT TO DATA
;       LDD     5,X             ;* CHECK SIGN
        ldaa    5,X             ;* CHECK SIGN
;* This trip might not be necessary! @FIXME:
        ldab    6,X             ;* CHECK SIGN @FIXME: <- Do I need this?
        BPL     BDIV2           ;* -- OF DIVIDEND

        COM     DFLAG           ;* ADJUST FLAG
        BSR     NEGATE
;       STD     5,X
        staa    5,X
        stab    6,X

BDIV2   CLRA
        CLRB

;* MAIN DIVIDE LOOP (UNSIGNED)

UDIV1   CMPA    1,X
        BHI     UDIV3
        BCS     UDIV2
        CMPB    2,X
        BCC     UDIV3

UDIV2   CLC
        BRA     UDIV4

;DIV3   SUBD    1,X
UDIV3   subb    2,X
        sbca    1,X
        SEC

UDIV4   ROL     6,X
        ROL     5,X
        DEC     0,X
        BEQ     UDIV5

        ROLB
        ROLA
        BCC     UDIV1
        BRA     UDIV3

UDIV5   INS
        INS
        INS
        RTS

;*----------------------------------------
;* NEGATE THE VALUE IN A,B
NEGATE  NEGA
        NEGB
        SBCA    #0
        RTS

;*----------------------------------
;* #24  INCLUSIVE OR THE TOP OF STACK AND REG.
ORS     TSX
        ORAA    0,X
        ORAB    1,X
POPS    INS                     ;* POP THE STACK
        INS
        JMP     NEXT

;*----------------------------------
;* #25  EXCLUSIVE OR ......
XORS    TSX
        EORA    0,X
        EORB    1,X
        BRA     POPS

;*----------------------------------
;* #26  AND .........
ANDS    TSX
        ANDA    0,X
        ANDB    1,X
        BRA     POPS

;*----------------------------------
;* #27  ARITH. SHIFT RIGHT THE TOP OF STACK
ASRS    TSX
        ANDB    #$1F            ;* MAX REASONABLE SHIFT
        BEQ     ASRS2

ASRS1   ASR     0,X
        ROR     1,X
        DECB
        BNE     ASRS1

ASRS2   PULA                    ;* GET THE RESULT
        PULB
        JMP     NEXT

;*--------------------------------
;* #28  ARITH. SHIFT LEFT THE TOP OF STACK
ASLS    TSX
        ANDB    #$1F
        BEQ     ASRS2

ASLS1   ASL     1,X
        ROL     0,X
        DECB
        BNE     ASLS1
        BRA     ASRS2

;*--------------------------------
;* #29  NEGATE THE REGISTER
NEGR    BSR     NEGATE
        JMP     NEXT

;*--------------------------------
;* #30  COMPLEMENT THE REGISTER
NOTR    COMA
        COMB
        JMP     NEXT

;*--------------------------------
;* #31  ADD 1 TO REG
;NCR    ADDD    #1
INCR    addb    #$01
        adca    #$00
        JMP     NEXT

;*--------------------------------
;* #32 SUBTRACT 1 FROM REG
;ECR    SUBD    #1
DECR    subb    #$01
        sbca    #$00
        JMP     NEXT

;*****************************************************
;*
;*   BASIC COMPARE INSTRUCTION SUBROUTINE
;*   Compare the top of Stack to Register and set Condition codes
;*
;*  Unsigned compare, Carry set if top of stack < A,B
;*
BCMP    TSX
;       ldd     2,X             ;* GET TOP OF STACK
        ldaa    2,X             ;* GET TOP OF STACK
        ldab    3,X             ;* GET TOP OF STACK
;       subd    zREG            ;* COMPARE
        subb    zREG+1          ;* COMPARE
        sbca    zREG            ;* COMPARE
        RTS


;*-------------------------------
;* #33  TEST FOR EQUALITY
ZEQ     BSR     BCMP
        BEQ     T
        BRA     F

;*-------------------------------
;* #34  TEST FOR NOT-EQUAL
ZNE     BSR     BCMP
        BNE     T
        BRA     F

;*-------------------------------
;* #35  TEST FOR LESS THAN
ZLT     BSR     BCMP
        BLT     T
        BRA     F

;*-------------------------------
;* #36  TEST FOR LESS THAN OR EQUAL
ZLE     BSR     BCMP
        BLE     T
        BRA     F

;*-------------------------------
;* #37  TEST FOR GREATER THAN
ZGT     BSR     BCMP
        BGT     T
        BRA     F

;*-------------------------------
;* #38  TEST FOR GREATER THAN OR EQUAL
ZGE     BSR     BCMP
        BGE     T
        BRA     F

;*-------------------------------
;* #39 TEST FOR LESS THAN (UNSIGNED)
ULT     BSR     BCMP
        BCS     T
        BRA     F

;*-------------------------------
;* #40  TEST FOR LESS THAN OR EQUAL (UNSIGNED)
ULE     BSR     BCMP
        BLS     T
        BRA     F

;*-------------------------------
;* #41  TEST FOR GREATER THAN (UNSIGNED)
UGT     BSR     BCMP
        BHI     T
        BRA     F

;*------------------------------
;* #42  TEST FOR GREATER THAN OR EQUAL (UNSIGNED)
UGE     BSR     BCMP
        BCC     T
F       CLRB                    ;* RETURN FALSE
        BRA     TRUE1
T       LDAB    #1              ;* RETURN TRUE
TRUE1   CLRA
        JMP     POPS            ;* POP STACK AND PROCEED

;*-------------------------------------
;* #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC    LDX     zPC             ;* POINT TO CODE
        JMP     0,X             ;* GO EXECUTE IT

WARMS   NOP                     ;* Makes it easier to pop a SWI in here
        BRA     WARMS

;* -[ Libraries ]---------------------------------------------------------------
xnl      ldaa    #13             ;* ^M
        staa    $F001
        ;;
        ;; Okay how are we called?

CODE    EQU     *
;* byte codes follow
#endasm
