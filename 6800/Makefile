# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html#zz-2.3
CC      = gcc
COPT    = -g -O3 -D FDEBUG 
V22OBJS = smallc.o cc2.o cc3.o cc4.o
OBJS    = smallc.o io.o heir.o ask.o options.o

# smallc.c smallc.h # smallc.c -o smallc
smallc: $(OBJS)
	$(CC) $(COPT) -o $@  $^

smallcv22: ${V22OBJS}
	$(CC) $(LOPT) -o $@  $^

%.o: %.c smallc.h
	$(CC) $(COPT) -o $@ -c $<

cc2.c: cc.h
cc3.c: cc.h
cc4.c: cc.h

.PHONY: clean

clean:
	rm -rf *~ *.ihx *.lst *.map *.mem *.rel *.rst   \
	       *.sym foo bar *.d51 *.hex *.a51 *.lnk  \
	       smallc smallcv22 *.o zap

# for i in AR.C CC1.C CC2.C CC3.C CC4.C CC.H NOTICE.H; do
# nom=$(tr '[:upper:]' '[:lower:]' <<< $i); cp $i ~/dev/git/SmallC68/${nom}; done

# gdb -q --args ./smallcv22 ar.c
