# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html#zz-2.3
COPT = -g -O3
OBJS = smallcv22.o cc2.o cc3.o cc4.o

#mallcv22: %.o cc2.o cc3.o cc4.o
smallcv22: ${OBJS}
	$(CC) $(LOPT) -o $@  $^

%.o: %.c cc.h notice.h
	$(CC) $(COPT) -o $@ -c $<

cc2.c: cc.h
cc3.c: cc.h
cc4.c: cc.h

smallc: smallc.c smallc.h
	gcc -g -O3 smallc.c -o smallc

.PHONY: clean

clean:
	rm -rf *~ *.ihx *.lst *.map *.mem *.rel *.rst   \
	       *.sym foo bar *.d51 *.hex *.a51 *.lnk  \
	       smallc *.o

# for i in AR.C CC1.C CC2.C CC3.C CC4.C CC.H NOTICE.H; do
# nom=$(tr '[:upper:]' '[:lower:]' <<< $i); cp $i ~/dev/git/SmallC68/${nom}; done

# gdb -q --args ./smallcv22 ar.c
