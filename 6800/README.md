# Small C v1.1 for the Motorola 68XX processor

Small C v1.1 to pseudo code compiler.

# Status

Currently this compiles under 64bit Linux (with a huge number of warnings). When done this will be a cross compiler under Linux/Windows. You won't be able to compile this under Flex. And if done correctly you should be able to use it with Motorola 68XX embedded boards with no OS.

Interesting note, this compiler outputs psuedo code which gets assembled with an interpreter in assembler code appropriate to the processor. Currently I have run1.c (6801/6803) and run9.c (6809). I haven't attempted to create a 6800 version but it should be possible. One additional note, I lack documentions on how the pseudo code works. I will come back to this at a later date as it's an interesting approach and may be useful for future projects on more than the 68XX processors.
