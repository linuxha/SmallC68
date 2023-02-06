# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html#zz-2.3
CC      = gcc
COPT    = -g -O3 -D FDEBUG 
OBJS    = scc.o itob.o ask.o heir.o cc2.o cc3.o cc4.o cc8.o cc9.o dump.o
OOBJS   = opt.c

scc: $(OBJS)
	$(CC) $(COPT) -o $@  $^

%.o: %.c
	$(CC) $(COPT) -o $@ -c $<

scc.o:	scc.c scc.h cc.h
ask.o:	ask.c cc.h
heir.o:	heir.c cc.h
cc2.o:	cc2.c cc.h
cc3.o:	cc3.c cc.h
cc4.o:	cc4.c cc.h
cc8.o:	cc8.c cc.h
cc9.o:	cc9.c cc.h
dump.o:	dump.c cc.h

opt: $(OOBJS)
	$(CC) $(COPT) -o $@  $^

.PHONY: clean

clean:
	rm -rf *~ *.ihx *.lst *.map *.mem *.rel *.rst   \
	       *.sym foo bar *.d51 *.hex *.a51 *.lnk  \
	       a.out scc smallc smallcv22 *.o zap

# for i in AR.C CC1.C CC2.C CC3.C CC4.C CC.H NOTICE.H; do
# nom=$(tr '[:upper:]' '[:lower:]' <<< $i); cp $i ~/dev/git/SmallC68/${nom}; done

# gdb -q --args ./smallcv22 ar.c
