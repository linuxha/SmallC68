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
BUGRAM  EQU     $90

RAM     EQU     $1000
ROM     EQU     $F100           ;* Simulator (Okay not really ROM)

;*   LAST UPDATE   20240702 ncherry@linuxha.com

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
        clra                    ;* Zero out everything
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
        ldx     #MESG
        jsr     PDATA1
        jsr     PCRLF
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
NEXT1   STAA    zREG            ;* SAVE THE WORK
        STAB    zREG+1          ;* --REGISTER
NEXT2   LDAB    0,X             ;* GET THE PSEUDO-INSTRUCTION
        INX                     ;* (B CONTAINS A TABLE OFFSET)
        STX     zPC             ;* SAVE NEW zPC
;* ==============================================================================
;*                                                                              *
        STAB    JJJ+2           ;* SAVE AS PAGE OFFSET                          *
;*                                                                              *
;* ==============================================================================
        LDAB    zREG+1          ;* RESTORE                                      *
JJJ     LDX     JTABLE          ;* POINT TO ROUTINE (SELF MODIFYING CODE)
        JMP     0,X             ;* GO EXECUTE THE PSEUDO-INSTR.

;**************************************************************
;*                  THE JUMP TABLE                            *
;**************************************************************

        ORG     ((*+255)/256)*256   ;* MUST START ON A PAGE BOUNDARY

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
        ldaa    0,X             ;* HIGH BYTE
        ldab    1,X             ;* HIGH BYTE
        jmp     BUMP2A

;*-------------------------
;* #1 LOAD STACK ADDRESS + OFFSET INTO REG
LD1SOFF tsx
        stx     zREG            ;* SAVE STACK VALUE
        ldx     zPC
        ldaa    0,X             ;* GET OFFSET
        ldab    1,X             ;* GET OFFSET
        addb    zREG+1          ;* ADD OFFSET
        adca    zREG            ;* ADD OFFSET
        jmp     BUMP2A

;*-------------------------
;* #2  LOAD WORD @ ADDRESS
LD1     ldx     zPC
        ldx     0,X             ;* GET ADDRESS
LD1A    ldaa    0,X             ;* GET WORD
        ldab    1,X             ;* GET WORD
        jmp     BUMP2

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
        ldx     0,X             ;* GET ADDRESS
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
PUSHR1  PSHB
        PSHA
        LDX     zPC
        JMP     NEXT2

;*-------------------------
;* #11  SWAP REG AND TOP OF STACK
EXG1    TSX
        LDX     0,X             ;* GET VALUE ON STACK
        STX     zREG            ;* SAVE
        INS
        INS
        PSHB 
        PSHA                    ;* REG ON STACK
        LDAA    zREG            ;* NEW REG
        LDAB    zREG+1
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
        DES
;* ==============================================================================
;*                                                                              *
        STS     *+5             ;*  *** SELF MODIFYING CODE ***                 *
;*                                                                              *
;* ==============================================================================
        DES
        STX     $FFFF           ;* PUSH RETURN ADDRESS
        BRA     JMPL

;*-------------------------
;* #15  CALL TO TOP OF STACK
JSRSP   tsx                     ;* POINT TO STACK
        ldx     0,X             ;* GET ADDRESS (NEW zPC)
        ins                     ;* POP
        ins
        ldab    zPC+1           ;* GET RETURN ADDRESS
        pshb 
        ldab    zPC
        pshb                    ;* SAVE RETURN ADDRESS
        jmp     NEXT2

;*-------------------------
;* #16  RETURN TO CALLER
RTSC    TSX
        LDX     0,X             ;* GET ADDRESS
        INS                     ;* POP
        INS
        JMP     NEXT1

;*-------------------------
;* #17  MODIFY THE STACK POINTER
MODSP   LDX     zPC
        LDAA    0,X             ;* GET VALUE
        LDAB    1,X
        STS     STEMP
        ADDB    STEMP+1         ;* ADD STACK POINTER
        ADCA    STEMP
        STAA    STEMP
        STAB    STEMP+1
        LDS     STEMP           ;* NEW STACK POINTER
        LDAA    zREG            ;* RESTORE REGISTER
        LDAB    zREG+1
        JMP     BUMP2A

;*---------------------------
;* #18  DOUBLE THE PRIMARY REGISTER
;BL1    ASLD
DBL1    asla                    ;* May not match the asld flag settings
        rolb
        JMP     NEXT

;*---------------------------
;* #19  ADD REG AND TOP OF STACK (THEN POP)
ADDS    tsx
        addb    1,X             ;* DO THE ADD
        adca    0,X             ;* DO THE ADD
        jmp     POPS            ;* POP & RETURN

;*---------------------------
;* #20  SUBTRACT REG FROM TOP OF STACK
SUBFST  pula                    ;* GET VALUE OFF STACK
        pulb
        subb    zREG+1          ;* SUBTRACT REGISTER
        sbca    zREG            ;* SUBTRACT REGISTER
        jmp     NEXT

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
BDIV    clr     DFLAG
        tsta                    ;* CHECK DIVISOR SIGN
        bpl     BDIV1

        inc     DFLAG           ;* ADJUST SIGN FLAG
        bsr     NEGATE          ;* TAKE ABSOLUTE VALUE

BDIV1   pshb                    ;* FORCE ON STACK
        psha
        ldaa    #17             ;* BIT COUNTER
        psha
        tsx                     ;* POINT TO DATA
        ldaa    5,X             ;* CHECK SIGN
;* This trip might not be necessary! @FIXME:
        ldab    6,X             ;* CHECK SIGN @FIXME: <- Do I need this?
        bpl     BDIV2           ;* -- OF DIVIDEND

        com     DFLAG           ;* ADJUST FLAG
        bsr     NEGATE
        staa    5,X
        stab    6,X

BDIV2   clra
        clrb

;* MAIN DIVIDE LOOP (UNSIGNED)

UDIV1   CMPA    1,X
        BHI     UDIV3
        BCS     UDIV2
        CMPB    2,X
        BCC     UDIV3

UDIV2   clc
        bra     UDIV4

UDIV3   subb    2,X
        sbca    1,X
        sec

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
;*  Signed compare -- Carry reflects the sign of difference
;*         (set means: top of stack < A,B )
;*
SCMP    TSX
        LDAA    2,X             ;* GET TOP OF STACK
        LDAB    3,X
        SUBB    zREG+1          ;* SET CONDITION
        SBCA    zREG            ;* ... FLAGS
        BPL     STCMP1          ;* SKIP IF PLUS

        STAB    zREG+1          ;* TEMP SAVE
        ORAA    zREG+1          ;* SET/RESET ZERO FLAG
        SEC                     ;* AND SET CARRY
        RTS

STCMP1  STAB    zREG+1
        ORAA    zREG+1
        CLC                     ;* CLEAR THE CARRY
        RTS
;*
;*  Unsigned compare, Carry set if top of stack < A,B
;*
BCMP    TSX
        LDAA    2,X             ;* GET TOP OF STACK
        LDAB    3,X
        CMPA    zREG            ;* CHECK TOP BYTE
        BNE     BCMP1
        CMPB    zREG+1
BCMP1   RTS


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
ZLT     BSR     SCMP
        BCS     T
        BRA     F

;*-------------------------------
;* #36  TEST FOR LESS THAN OR EQUAL
ZLE     BSR     SCMP
        BLS     T
        BRA     F

;*-------------------------------
;* #37  TEST FOR GREATER THAN
ZGT     BSR     SCMP
        BHI     T
        BRA     F

;*-------------------------------
;* #38  TEST FOR GREATER THAN OR EQUAL
ZGE     BSR     SCMP
        BCC     T
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

F       CLRB                    ;* RETURN False
        BRA     TRUE1

T       LDAB    #1              ;* RETURN True

TRUE1   CLRA 
        JMP     POPS            ;* POP STACK AND PROCEED

;*-------------------------------------
;* #43  SWITCH TO EXECUTABLE (ASSEMBLY) CODE
ASMC    LDX     zPC             ;* POINT TO CODE
        JMP     0,X             ;* GO EXECUTE IT

;* For the simulator
;*      ADDRESS
ACIACS  EQU     $F000
ACIADA  EQU     $F001

putchar FCB     86
        tsx
        ldaa    3,X             ;* GET THE CHAR
        ;bra     OUTEEE
;* -----------------------------------------------------------------------------
;*
;*      OUTPUT ONE CHAR 
PUTC
OUTEEE  psha
        inc     BUGRAM+0
OUTEEE1 ldaa    ACIACS
        asra
        asra
        bcc     OUTEEE1
        pula
        staa    ACIADA
        rts

;*
;*      PRINT DATA POINTED AT BY X-REG
PDATA2  bsr     OUTEEE
        inx
PDATA1  ldaa    0,X
        cmpa    #0
        bne     PDATA2
        rts             ;* STOP ON EOT


PCRLF   ldx     #CRLFST         ; ($771F)
        bsr     PDATA1
        rts

;*----------------------------------------------
;*  puts(string)  print a string on the terminal
puts    fcb     86
        TSX
        inc     BUGRAM+1
        LDX     2,X             ;* GET STRING ADDRESS
PLOOP   LDAA    0,X             ;* GET THE CHAR
        BEQ     GETS4           ;* IF END OF STRING--QUIT
        CMPA    #'\\'           ;* SPECIAL CHAR ?
        BNE     PLOOP1          ;* NO, SKIP
        BSR     SPECIAL         ;* YES, INTERPRET
        CMPA    #$0D            ;* IS IT A CR (NEWLINE)
        BNE     PLOOP1          ;* NO--SKIP
        bsr     PCRLF           ;* YES PRINT CR/LF PAIR
        BRA     PLOOP2
PLOOP1  JSR     PUTC            ;* PRINT IT
PLOOP2  INX                     ;* POINTER BUMP
        BRA     PLOOP

GETS4   JMP     RTSC

;*  This subroutine interprets the backslash (\) sequence.

SPECIAL INX
        LDAA    0,X             ;* GET NEXT CHAR
        CMPA    #'b'            ;* \b = ^H
        BNE     SP1
        LDAA    #08             ;* BACKSPACE
        BRA     SPEXIT
SP1     CMPA    #'f'            ;* \f = FF
        BNE     SP2
        LDAA    #$0C            ;* FORMFEED
        BRA     SPEXIT
SP2     CMPA    #'n'            ;* Newline 0x0D
        BNE     SP3
        LDAA    #$0D            ;* NEWLINE
        BRA     SPEXIT
SP3     CMPA    #'\\'           ;* BACKSLASH
        BEQ     SPEXIT
        CMPA    #'\''           ;* SINGLE QUOTE
        BEQ     SPEXIT
        CMPA    #'\"'           ;* DOUBLE QUOTE
        BEQ     SPEXIT
        CMPA    #'x'            ;* START OF HEX SEQUENCE
        BEQ     SPHEX
        CMPA    #'0'            ;* OCTAL SEQUENCE ?
        BLT     SPERR
        CMPA    #'7'
        BLE     SPOCTAL         ;* YES

SPERR   DEX                     ;* BACKUP THE POINTER
        LDAA    0,X             ;* RESTORE CHAR
SPEXIT  RTS                     ;* RETURN

SPOCTAL LDAB    #3
        STAB    DFLAG           ;* SAVE COUNTER
        CLRB 

SPOCT1  SUBA    #'0'            ;* CONVERT TO DIGIT
        ASLB                    ;* SHIFT ACCUM
        ASLB 
        ASLB 
        ABA                     ;* ADD IN NEW DIGIT
        TAB                     ;* SAVE
        DEC     DFLAG
        BEQ     SPBYE           ;* IF MAX COUNT--EXIT
        INX
        LDAA    0,X             ;* GET NEXT CHAR
        CMPA    #'0'            ;* VERIFY IF OCTAL
        BLT     SPFINI
        CMPA    #'7'
        BLE     SPOCT1          ;* YES, CONTINUE

SPFINI  DEX                     ;* BACKUP
SPBYE   TBA                     ;* GET ACCUM CHAR
        RTS

SPHEX   LDAB    #2
        STAB    DFLAG
        CLRB 

SPHEXL  INX
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
SPHEX1  SUBA    #7
SPHEX2  SUBA    #'0'
        ASLB 
        ASLB 
        ASLB 
        ASLB 
        ABA
        TAB
        DEC    DFLAG
        BEQ    SPBYE            ;* MAX COUNT REACHED ???
        BRA    SPHEXL           ;* NO, LOOP

CRLFST  fcc     '\n\r\0'        ;* CR, LF, NUL, NUL, EOT
MESG    fcc     'Small C v1.1.1\n\r'

WARMS   BRA     *

CODE    EQU     *

;* byte codes follow

#endasm
