#!/usr/bin/python3

# http://www.8bit-era.cz/6800.html
import os, sys
import re

# Read in asm file, reformat

help = """
HELP
"""

if(len(sys.argv) != 2) :
    # put help here
    print(help, file=sys.stderr)
    exit(-1);
#

# Examples
#       	NAM RT68-V2
# SYSMOD	RMB	1	* RT MODE 0=USER 1=EXEC
# BEGADR	RMB	2
# STACK		EQU	*	* MONITOR STACK
# LOAD		LDAB	#$3C	* TAPE ON CONSTANTS
# 		LDAA	#$11	* READER ON CODE
#		BSR	INCH	*
#		RTS		*
def func(s):
    asmOps(s)
#
def asmOps(s):
    s  = s.strip()
    sp = s.split(None, 3)
    #if(len(sp[0]) < 6):
    #    sp[0] = sp[0] + '\t'
    #
    l  = len(sp)
    if(l == 4):
        if(len(sp[1]) < 4):
            sp[1] = sp[1] + ' '
        #
        if(len(sp[2]) < 4):
            sp[2] = sp[2] + '  '
        #
        print("{0}\t{1} {2}\t* {3}".format( sp[0], sp[1], sp[2], sp[3]))
    elif(l == 3):
        if(len(sp[1]) < 4):
            sp[1] = sp[1] + ' '
        #
        if(len(sp[2]) < 4):
            sp[2] = sp[2] + '  '
        #
        print("{0}\t{1} {2}\t* ".format( sp[0], sp[1], sp[2]))
    else:
        print("1?:{0}".format(s))
    #
#

# 	RTS	*
# 	RTS	* Comment
def func2(s):
    s = s.strip()
    sp = s.split(None, 1)
    l  = len(sp)
    if(l == 2):
        print("\t{0}\t\t* {1}".format( sp[0], sp[1]))
    elif(l == 1):
        print("\t{0}\t\t* ".format( sp[0]))
    else:
        print("2?:\t{0}\t\t* {1} ({2})".format(s, l, sp[0]))
    #
#

# @2
# P:LOAD3 LDAB #$34 TAPE OFF CONSTANTS (3)
# 3:LOAD3 LDAB    * #$34 TAPE OFF CONSTANTS
# #3
# P:LOAD3 LDAB #$34 TAPE OFF CONSTANTS (3)
# 3:LOAD3 LDAB    * #$34 TAPE OFF CONSTANTS
def func3(s):
    s = s.strip()
    sp = s.split(None, 3)       # This causes issues
    l  = len(sp)
    if(l == 4):
        if(len(sp[1]) < 4):
            sp[1] = sp[1] + ' '
        #
        if(len(sp[2]) < 3):
            sp[2] = sp[2] + '\t'
        #
        print("{0}\t{1} {2}\t* {3} XXXXXXXX".format( sp[0], sp[1], sp[2], sp[3]))
    elif(l == 3):
        if(len(sp[1]) < 4):
            sp[1] = sp[1] + ' '
        #
        if(len(sp[2]) < 4):
            sp[2] = sp[2] + '  '
        #
        print("{0}\t{1} {2}\t*".format( sp[0], sp[1], sp[2]))
    elif(l == 2):
        #if(len(sp[0]) < 4):
        #    sp[0] = sp[0] + ' '
        #
        print("{0}\t{1}\t\t* (LABEL/NM only?)".format( sp[0], sp[1]))
    else:
        print("3?:{0}\t* {1} ({2})".format(s, l, sp[0]))
    #
#

#
# 	BRA	LOAD2	*
# 	BRA	LOAD2	* Branch always
def func4(s):
    s = s.strip()
    sp = s.split(None, 2)
    l  = len(sp)
    if(l == 3):
        if(len(sp[0]) < 4):
            sp[0] = sp[0] + ' '
        #
        if(len(sp[1]) < 4):
            sp[1] = sp[1] + ' '
        #
        print("\t{0} {1}\t* {2}".format( sp[0], sp[1], sp[2]))
    elif(l == 2):
        if(len(sp[0]) < 4):
            sp[0] = sp[0] + ' '
        #
        if(len(sp[1]) < 4):
            sp[1] = sp[1] + ' '
        #
        print("\t{0} {1}\t* ".format( sp[0], sp[1]))
    elif(l == 1):
        print("\t{0}\t\t* ".format( sp[0]))
    else:
        print("4?:\t{0}\t\t* {1} ({2})".format(s, l, sp[0]))
    #
#

def comments(s):
    # Handle blank and lines that start with comments
    # line with line ending
    print(s, end = '')
#

def labels(s):
    # Handle labels (and equates)
    # Label OP VALUE Comment...
    op = s.split()[1]
    # run the function
    if(op in OP):
        OP[op](s)
    else:
        func3(s)
    #
#

def instructions(s):
    # Eat the whitespace
    s  = s.lstrip()
    nm = s.split()[0]
    if(nm in NM):
        NM[nm](s)
    else:
        #print("P:\t" + line, end = '')
        func4(s)
    #
#

filename = sys.argv[1];
# @FIXME: Still needs the Asm Instructions
# LABEL	LSRA
# LABEL LSRA		*
# LABEL ADDA #$30
# LABEL ADDA #$30	*
OP = {'RMB': asmOps,
      'EQU': asmOps,
      'FDB': asmOps,
      'FCB': asmOps,
      'x': asmOps
}

# No labels
# mnemonics
NM = {'RMB': func,
      'ORG': func4,
      'END': func2,
      'TTL': func,
      'RTS': func2,
      'ABA': func2,
      'ASLA': func2,
      'ASLB': func2,
      'ASRA': func2,
      'ASRB': func2,
      'CBA': func2,
      'CLC': func2,
      'CLI': func2,
      'CLRA': func2,
      'CLRB': func2,
      'CLV': func2,
      'COMA': func2,
      'COMB': func2,
      'DAA': func2,
      'DECA': func2,
      'DECB': func2,
      'DES': func2,
      'DEX': func2,
      'INCA': func2,
      'INCB': func2,
      'INS': func2,
      'INX': func2,
      'LSRA': func2,
      'LSRB': func2,
      'NEGA': func2,
      'NEGB': func2,
      'NOP': func2,
      'PSHA': func2,
      'PSHB': func2,
      'PULA': func2,
      'PULB': func2,
      'ROLA': func2,
      'ROLB': func2,
      'RORA': func2,
      'RORB': func2,
      'RTI': func2,
      'RTS': func2,
      'SBA': func2,
      'SEC': func2,
      'SEI': func2,
      'SEV': func2,
      'SWI': func2,
      'TAB': func2,
      'TAP': func2,
      'TBA': func2,
      'TPA': func2,
      'TSTA': func2,
      'TSTB': func2,
      'TSX': func2,
      'TXS': func2,
      'WAI': func2
}
# NM[idx](str)
try:
    with open(filename) as f:
        print('*[ Start ]**********************************************************************')

        while True:
            # Get next line from file
            line = f.readline()
  
            # if line is empty
            # end of file is reached
            if not line:
                print('*[ Fini ]***********************************************************************')
                break
            #
            # The difference is that if line is an empty string, line[:1]
            # will evaluate to an empty string while line[0] will
            # raise an IndexError
            #
            #myRe  = re.compile(r"^\s+|^\s+\*")
            myRe  = re.compile(r"^\s*\*|^\s*\n")
            match = re.search(myRe, line)
            if match:
                # Handle blank and comment lines
                # Handle blank and lines that start with comments
                comments(line)
            elif(line[:1].isalpha()):
                # Handle labels (and equates)
                # Label OP VALUE Comment...
                labels(line)
            else:
                #print("### '" + line + "' ###")
                instructions(line)
            #
        #
    #
except FileNotFoundError:
    msg = "Sorry, the file "+ filename + "does not exist."
    print(msg) # Sorry, the file John.txt does not exist.
#

# =[ Fini ]=====================================================================
"""
      'PSHB': func2,
      'ASLA': func2,
      'ASLB': func2,
      'LSRA': func2,
      'LSRB': func2,
      'TAB': func2,
      'ABA': func2,
      'PULB': func2,
      'PSHA': func2,
      'PULA': func2,
      'INX': func2,
      'DECA': func2,
      'DECB': func2,
      'DEX': func2,
      'INCA': func2,
      'INCB': func2,
      'INX': func2,
      'INS': func2,
      'CLRA': func2,
      'CLRB': func2,
      'TSX': func2,
      'TXS': func2,
"""
