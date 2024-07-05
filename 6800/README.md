# Small C v1.1 for the Motorola 68XX processor

Small C v1.1 to pseudo code compiler.

# Status

Currently this compiles under 64bit Linux (with a huge number of warnings). When done this will be a cross compiler under Linux/Windows. You won't be able to compile this under Flex. And if done correctly you should be able to use it with Motorola 68XX embedded boards with no OS.

Interesting note, this compiler outputs psuedo code which gets assembled with an interpreter in assembler code appropriate to the processor. Currently I have run1.c (6801/6803) and run9.c (6809). I have mow attempted to create a 6800 version but it remains untested. One additional note, I lack documentions on how the pseudo code works. I will come back to this at a later date as it's an interesting approach and may be useful for future projects on more than the 68XX processors.

Remember this is the limited compiler so it's annoying but it's simple so debugging is less of a chore. Once this is somewhat stable I'll more on to more advanced versions and take what I've learned here to those. 

And one more point, just because it compiles doesn't mean it actually works or works well. I willkeep hacking at it until it works reasonably. I do want to test this out and see what it can do. The interpreter is an interesting idea.
