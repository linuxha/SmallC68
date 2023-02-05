# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html#zz-2.3
CC      = gcc
COPT    = -g -O3 -D FDEBUG 
OBJS    = scc.c scc.h

# smallc.c smallc.h # smallc.c -o smallc
scc: $(OBJS)
	$(CC) $(COPT) -o $@  $^

%.o: %.c
	$(CC) $(COPT) -o $@ -c $<

.PHONY: clean

clean:
	rm -rf *~ *.ihx *.lst *.map *.mem *.rel *.rst   \
	       *.sym foo bar *.d51 *.hex *.a51 *.lnk  \
	       smallc smallcv22 *.o zap

# for i in AR.C CC1.C CC2.C CC3.C CC4.C CC.H NOTICE.H; do
# nom=$(tr '[:upper:]' '[:lower:]' <<< $i); cp $i ~/dev/git/SmallC68/${nom}; done

# gdb -q --args ./smallcv22 ar.c
