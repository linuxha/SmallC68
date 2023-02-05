# SmallC68XX - Small C for the Motorola 68XX family under Linux 

Ron Cain's 1.1 Small C compiler for the 6800, 6801, 6803 and 6809. Maybe the 68HC11. The original Author of the Flex port is S. Stepanoff

Originally these were Flex OS Compilers but I think I can make this work under Linux and the output code should be OS agnotic. My intention is to set them up to work on Linux as a cross-compiler.

# Notes

```
6800/ ..... Early version of the Flex OS Small C compiler (very limitted, v1.1 1982)
silifen/ .. Later version of the Flex OS Small C compiler (has for(), etc v1.1 1985)
v22/ ...... MSDOS version of the Small C compiler (outputs 8086 code)
LICENSE ... Proposed License (I may need to change this)
Makefile .. Ignore this, make files have moved their respective directories
README.md . This file
ccint.txt . Ignore this file (Small C interpreter used in 6800 version)

```
After reading and poking around the Flex Small C Compiler DSK images I've guessed that these are Ron Cain's Small C V1.1 compiler and that the runXX (00,01,09) are the runtime files that take the psuedo code and make it understandable (assemble) under Flex.

The ccx.c files is the smallc.c file broken into 9 sections so the small C compiler can compile itself on Flex. Since I'll be using Linux to cross compile. I'll not include this files.

```
cc0.c
cc1.c
cc2.c
cc3.c
cc4.c
cc5.c
cc6.c
cc7.c
cc8.c
```

The important file is smallc.c, that's my starting point. The p.c file is a C pre-processor. I'm not sure if I'll use it but decided to keep it here for now. The rest of the files are files needed to assemble the psuedo code generated by the small C compiler. At this time I'm not sure which is which. I'll work on that.

```
ccc.h
ccint.txt
flexptrs.txt
LICENSE
p.c
prtlib.asm
prtlib.c
prtlib.lib
README.md
run1.asm
run9.asm
run9.c
smallc.c
```

2023/02/04 - I've got the compiler hacked together and mostly working. What I've found is that the small c compiler outputs pseudo code and that the run9 (6809) and run1 (6801) code are the asm source to an interpreter. Assemble the code together and you have a program. This small C compiler is still quite limitted but may be useful and as one of the notes files points out C is easier to write than asm code. Anyway I'm posting this mess so I don't lose it and so other might get ideas. Just note that this is terrible C code. It was meant to use the very limmited small c compiler to compile itself. I'm in the process of making it work under Linux as a cross compiler for any of the Motorola preocessors. It will no longer compiler itself.

# License

For the moment I've selected LGPL 2.1, I'm not certain I can do this as the previous programmers had different licenses. I'll adjust as needed.
