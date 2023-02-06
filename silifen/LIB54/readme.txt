

              Notes for use of CRASMB with SFS small-C R1.1b
              ----------------------------------------------

For 6809 use to generate a FLEX command.

Make sure that LIBDEF09.TXT has CRASMB set 1 in it.

+++exec goto9

+++cc sieve.c -o sieve.m -d             normal useage

+++opt sieve.m sieve.om                 normal useage

+++crasmb sieve.om,sieve.cmd,i6809+ls

+++sieve


See Manual Appendix B for details of 6801 code generation, this
appendix does not cover CRASMB use, it assumes Compusense XASM6801.

To generate 6801 code but assemble into 6809 FLEX command for testing.

Make sure that LIBDEF91.TXT has CRASMB set 1 in it.

+++exec goto91

If sieve.om is still around from 6809 use then it can be used,
otherwise make it as above.

+++cc sieve.c -o sieve.m -d             no difference from 6809

+++opt sieve.om sieve.cmd               no difference from 6809

+++crasmb sieve.om,sieve91.cmd,i6809+ls

+++sieve91

The above version of sieve, sieve91, uses 6801 assembly language
definitions of the macro's used to generate code but the 6809
assembler turns these into a subset of the 6809 instruction set.
The code is larger and slower than pure 6809 and is a bit larger
and slower than it would be on a real 6801.


To generate code for a 6801/6811/6301

Make sure that LIBDEF01.TXT has CRASMB set 1 in it.

Check that LIBM01.TXT has the processor type set up. One of
M6801, M6811 or H6301 should be SET 1.

Check that LIBF01.TXT has the correct memory map set up using
CCODE, CDATA and CSTACK for the target system.















The program starts at the label START in LIBC01.TXT, this needs
to be jumped to in some way on the target system. To set up the
reset vector the CSEG macro in LIBM01.TXT could be changed to:

CSEG macro
     org $FFFE
     fdb START
     org CCODE
     endm

Terminal I/O primitives need to be put into LIBH.TXT
if any character based I/O is required. Any other hardware
access routines can be placed in LIBH.TXT. A C header file
is provided, HARDWARE.H and this should be #included or
compiled in with the program otherwise LIBH.TXT will not
be loaded in by the assembler. Alternatively LIBLOAD1.TXT could
be modified to always load LIBH.TXT.


Finally -

+++exec goto1

If the .om file is note left over from above then recompile,
there is no change as far as the compiler is concerned for 6801.

+++cc sieve.c -o sieve.m -d             normal compile

+++opt sieve.m sieve.om                 normal opt

+++crasmb sieve.om,sieve1.bin,i6801+ls  (or use i6811,i6301 as required)

Then get sieve1.bin into the target processor somehow.


A word of warning
-----------------
This compiler is tested and believed bug free for 6809 use, the 6801
use has been tested using a 6809 and has been assembled into 6801 binaries
successfully. The 6801 binaries have never been tested on a real 6801
(to the authors knowledge) although some users have had the 6801 product
since september 1985 and have not complained yet. Please report problems
to

        Silicon Fen Software
        39, Mason Road,
        Burwell,
        Cambs,
        CB5 OBG

