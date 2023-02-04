#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "smallc.h"

/*
      This manual page lists C operators and their precedence in evaluation.

       Operator                            Associativity   Notes
1      () [] -> . ++ --                    left to right   [1]
2      ! ~ ++ -- + - (type) * & sizeof     right to left   [2]
3      * / %                               left to right
4      + -                                 left to right
5      << >>                               left to right
6      < <= > >=                           left to right
7      == !=                               left to right
8      &                                   left to right
9      ^                                   left to right
10     |                                   left to right
11     &&                                  left to right
12     ||                                  left to right
13     ?:                                  right to left
13     = += -= *= /= %= <<= >>= &= ^= |=   right to left
15     ,                                   left to right

       The following notes provide further information to the above table:

       [1] The ++ and -- operators at this precedence level are the postfix flavors of the
           operators.
       [2] The ++ and -- operators at this precedence level are the prefix flavors of the
           operators.

 */

//
// @FIXME: I don't know what these shoild be
//         These shouldn't be ints they shoud be char or int * ?
//

/*
@@@ In match [
@@@ In blanks
@@@ In ch '('
@@@ In ch '3'
@@@ In ch '('
@@@ In ch '3'
@@@ In ch '('
@@@ In ch '3'
@@@ In streq ();@@@ [
@@@ In match (
@@@ In blanks
@@@ In ch '('
@@@ In ch '3'
@@@ In ch '('
@@@ In ch '3'
@@@ In ch '('
@@@ In ch '3'
@@@ In streq ();@@@ (

Program received signal SIGSEGV, Segmentation fault.
getmem (sym=0x5556bece <error: Cannot access memory at address 0x5556bece>) at smallc.c:1944
1944        if((sym[IDENT]!=POINTER)&(sym[TYPE]==CCHAR))
(gdb) bt
#0  getmem (sym=0x5556bece <error: Cannot access memory at address 0x5556bece>) at smallc.c:1944
#1  rvalue (lval=lval@entry=0x7fffffffdc78) at smallc.c:1826
#2  0x0000555555564915 in heir11 (lval=0x7fffffffdc78) at heir.c:488
#3  0x0000555555564bf5 in heir10 (lval=0x7fffffffdc78) at heir.c:406
#4  heir10 (lval=0x7fffffffdc78) at heir.c:343
#5  0x0000555555565240 in heir9 (lval=0x7fffffffdc78) at heir.c:313
#6  heir8 (lval=0x7fffffffdc78) at heir.c:278
#7  heir7 (lval=0x7fffffffdc78) at heir.c:249
#8  0x00005555555655f9 in heir6 (lval=lval@entry=0x7fffffffdc78) at heir.c:160
#9  0x00005555555659d7 in heir5 (lval=0x7fffffffdc78) at heir.c:132
#10 0x0000555555565c5f in heir4 (lval=0x7fffffffdc78) at heir.c:108
#11 heir3 (lval=0x7fffffffdc78) at heir.c:86
#12 0x0000555555565d50 in heir2 (lval=0x7fffffffdc78) at heir.c:65
#13 0x0000555555565ec3 in heir1 (lval=lval@entry=0x7fffffffdc78) at heir.c:45
#14 0x0000555555556f24 in expression () at smallc.c:1313
#15 0x0000555555560d8c in statement () at smallc.c:596
#16 0x0000555555561bcb in compound () at smallc.c:616
#17 0x0000555555560310 in statement () at smallc.c:579
#18 0x0000555555563adf in newfunc () at smallc.c:509
#19 0x0000555555564221 in parse () at smallc.c:166
#20 0x0000555555555212 in main (argc=<optimized out>, argv=<optimized out>) at smallc.c:115
(gdb) print lval
No symbol "lval" in current context. 
(gdb)
 */
/*
1  Handle =
*/
long
heir1(long lval[]) {
    int k;
    int lval2[2];

    k = heir2(lval);

    if (match("=")) {
        if(k==0) {
            needlval();
            return (0);
        }
        if (lval[1]) {
            zpush();
        }
        if(heir1(lval2)) {
            rvalue(lval2);
        }
        store(lval);
        return (0);
    }
    else
        return (k);
}

/*
2  Handle |
*/
long
heir2(long lval[]) {
    int k;
    int lval2[2];

    k=heir3(lval);

    blanks();
    if(ch()!='|')return (k);
    if(k)rvalue(lval);

    while(1) {
        if (match("|")) {
            zpush();
            if(heir3(lval2)) rvalue(lval2);
            zpop();
            zor();
        }
        else return (0);
    }
}

/*
3  Handle ^
*/
long
heir3(long lval[]) {
    int k;
    int lval2[2];

    k=heir4(lval);

    blanks();
    if(ch()!='^')return (k);
    if(k)rvalue(lval);

    while(1) {
        if (match("^")) {
            zpush();
            if(heir4(lval2))
                rvalue(lval2);
            zpop();
            zxor();
        }
        else return (0);
    }
}

/*
4  Handle &
*/
long
heir4(long lval[]) {
    int k;
    int lval2[2];

    k=heir5(lval);
    blanks();
    if(ch()!='&')
        return (k);
    if(k)
        rvalue(lval);

    while(1) {
        if (match("&")) {
            zpush();
            if(heir5(lval2))rvalue(lval2);
            zpop();
            zand();
        }
        else
            return (0);
    }
}

/*
5  Handle == !=
*/
long
heir5(long lval[]) {
    int k;
    int lval2[2];

    k=heir6(lval);
    blanks();
    if((streq((char *) line+lidx,"==")==0) &&
       (streq((char *) line+lidx,"!=")==0))    return (k);
    if(k)rvalue(lval);

    while(1) {
        if (match("==")) {
            zpush();
            if(heir6(lval2))rvalue(lval2);
            zpop();
            zeq();
        }
        else if (match("!=")) {
            zpush();
            if(heir6(lval2))rvalue(lval2);
            zpop();
            zne();
        }
        else return (0);
    }
}

/*
6  Handle < > <= >= >> <<
*/
long
heir6(long lval[]) {
    int k;
    int lval2[2];

    k=heir7(lval);
    blanks();

    if((streq((char *) line+lidx,"<")==0)   &&
       (streq((char *) line+lidx,">")==0)   &&
       (streq((char *) line+lidx,"<=")==0)  &&
       (streq((char *) line+lidx,">=")==0)) return (k);
    if(streq( (char *) line+lidx,">>"))      return (k);
    if(streq( (char *) line+lidx,"<<"))      return (k);
    if(k)                          rvalue(lval);

    while(1) {
        if (match("<=")) {
            zpush();
            if(heir7(lval2)) rvalue(lval2);
            zpop();
            cptr = lval; //[0]
            if(lval) {
                if(cptr[IDENT]==POINTER) {
                    ule();
                    continue;
                }
            }
            cptr = lval2; //[0]
            if(lval2) {
                if(cptr[IDENT]==POINTER) {
                    ule();
                    continue;
                }
            }
            zle();
        }
        else if (match(">=")) {
            zpush();
            if(heir7(lval2))rvalue(lval2);
            zpop();
            cptr = lval; //[0]
            if(lval) {
                if(cptr[IDENT]==POINTER) {
                    uge();
                    continue;
                }
            }
            cptr = lval2; //[0]
            if(lval2) {
                if(cptr[IDENT]==POINTER) {
                    uge();
                    continue;
                }
            }
            zge();
        }
        else if((streq((char *) line+lidx,"<")) && (streq((char *) line+lidx,"<<")==0)) {
            inbyte();
            zpush();

            if(heir7(lval2))rvalue(lval2);
            zpop();
            cptr = lval; //[0]
            if(lval) {
                if(cptr[IDENT]==POINTER) {
                    ult();
                    continue;
                }
            }
            cptr=lval2; //[0]
            if(lval2)
                if(cptr[IDENT]==POINTER) {
                    ult();
                    continue;
                }
            zlt();
        }
        else if((streq((char *) line+lidx,">")) && (streq((char *) line+lidx,">>")==0)) {
            inbyte();
            zpush();
            if(heir7(lval2))rvalue(lval2);
            zpop();
            cptr = lval; //[0]
            if(lval) {
                if(cptr[IDENT]==POINTER) {
                    ugt();
                    continue;
                }
            }
            cptr = lval2; //[0]
            if(lval2) {
                if(cptr[IDENT]==POINTER) {
                    ugt();
                    continue;
                }
            }
            zgt();
        }
        else return (0);
    }
}
/*      >>>>>> start of cc6 <<<<<<      */

/*
7  Handle >> <<
*/
long
heir7(long lval[]) {
    int  k;
    int lval2[2];

    k = heir8(lval);
    blanks();

    //if((streq(line+lidx,">>")==0) && (streq(line+lidx,"<<")==0))
    if((streq((char *) line+lidx,">>")==0) && (streq((char *) line+lidx,"<<")==0))
        return (k);
    if(k)rvalue(lval);

    while(1) {
        if (match(">>")) {
            zpush();
            if(heir8(lval2))rvalue(lval2);
            zpop();
            asr();
        }
        else if (match("<<")) {
            zpush();
            if(heir8(lval2))rvalue(lval2);
            zpop();
            asl();
        }
        else return (0);
    }
}

/*
8  Handle + -
*/

long
heir8(long lval[]) {
    int k;
    int lval2[2];

    k=heir9(lval);
    blanks();
    if((ch()!='+') && (ch()!='-'))return (k);
    if(k) rvalue(lval);

    while(1) {
        if (match("+")) {
            zpush();
            if(heir9(lval2))rvalue(lval2);
            if(cptr=lval[0])
                if((cptr[IDENT]==POINTER) &&
                   (cptr[TYPE]==CINT))
                    doublereg();
            zpop();
            zadd();
        }
        else if (match("-")) {
            zpush();
            if(heir9(lval2))rvalue(lval2);
            if(cptr=lval[0])
                if((cptr[IDENT]==POINTER) &&
                   (cptr[TYPE]==CINT))
                    doublereg();
            zpop();
            zsub();
        }
        else return (0);
    }
}

/*
9  Handle * / %
 */
long
heir9(long lval[]) {
    int k;
    int lval2[2];

    k=heir10(lval);
    blanks();
    if((ch()!='*')&(ch()!='/')&
       (ch()!='%'))return (k);
    if(k)rvalue(lval);

    while(1) {
        if (match("*")) {
            zpush();
            if(heir9(lval2))rvalue(lval2);
            zpop();
            mult();
        }
        else if (match("/")) {
            zpush();
            if(heir10(lval2))rvalue(lval2);
            zpop();
            myDiv();
        }
        else if (match("%")) {
            zpush();
            if(heir10(lval2))rvalue(lval2);
            zpop();
            zmod();
        }
        else return (0);
    }
}

/*
10 Handle ++ -- - * &
 */
long
heir10(long lval[]) {
    int k;
    long *ptr;

    if(match("++")) {
        if((k=heir10(lval))==0) {
            needlval();
            return (0);
        }
        if(lval[1]) zpush();
        rvalue(lval);
        inc();
        ptr=lval; //[0];
        if((ptr[IDENT]==POINTER) && (ptr[TYPE]==CINT)) {
            inc();
        }
        store(lval);
        return (0);
    } else if(match("--")) {
        if((k=heir10(lval))==0) {
            needlval();
            return (0);
        }
        if(lval[1])zpush();
        rvalue(lval);
        dec();
        ptr=lval; //[0];
        if((ptr[IDENT]==POINTER) && (ptr[TYPE]==CINT)) {
            dec();
        }
        store(lval);
        return (0);
    }
    else if (match("-")) {
        k=heir10(lval);
        if (k) rvalue(lval);
        neg();
        return (0);
    }
    else if(match("*")) {
        k=heir10(lval);
        if(k)rvalue(lval);
        lval[1]=CINT;
        ptr=lval; //[0]
        if(lval) lval[1]=ptr[TYPE];
        lval[0]=0;
        return (1);
    }
    else if(match("&")) {
        k=heir10(lval);
        if(k==0) {
            error("illegal address");
            return (0);
        }
        else if(lval[1])return (0);
        else {
            immed();
            ptr=lval; //[0];
            outstr((char *)lval);
            nl();
            lval[1]=ptr[TYPE];
            return (0);
        }
    }
    else {
        k=heir11(lval);
        if(match("++")) {
            if(k==0) {
                needlval();
                return (0);
            }
            if(lval[1]) zpush();
            rvalue(lval);
            inc();
            ptr=lval; //[0];
            if((ptr[IDENT]==POINTER)&
               (ptr[TYPE]==CINT))
                inc();
            store(lval);
            dec();
            if((ptr[IDENT]==POINTER)&
               (ptr[TYPE]==CINT))
                dec();
            return (0);
        } else if(match("--")) {
            if(k==0) {
                needlval();
                return (0);
            }
            if(lval[1])zpush();
            rvalue(lval);
            dec();
            ptr=lval; //[0];
            if((ptr[IDENT]==POINTER)&
               (ptr[TYPE]==CINT))
                dec();
            store(lval);
            inc();
            if((ptr[IDENT]==POINTER)&
               (ptr[TYPE]==CINT))
                inc();
            return (0);
        }
        else return (k);
    }
}
/*      >>>>>> start of cc7 <<<<<<      */

/*
11 Handle [] () {} FUNCTION
 */

long
heir11(long *lval) {
    int k;

    long *ptr;

    k = primary(lval);
    //ptr = lval[0];
    ptr = lval;
    blanks();

    if((ch() == '[') || (ch() == '(')) {
        while(1) {
            if(match("[")) {
                if(ptr == 0) {
                    error("can't subscript");
                    junk();
                    needbrack("]");
                    return (0);
                }
                else if(ptr[IDENT]==POINTER)
                    rvalue(lval);
                else if(ptr[IDENT]!=ARRAY) {
                    error("can't subscript");
                    k=0;
                }
                zpush();
                expression();
                needbrack("]");
                if(ptr[TYPE] == CINT)
                    doublereg();
                zpop();
                zadd();
                lval[1]=ptr[TYPE];
                k=1;
            }
            else if(match("(")) {
                if(ptr == 0) {
                    callfunction(0);
                }
                else if(ptr[IDENT] != FUNCTION) {
                    rvalue(lval);
                    callfunction(0);
                }
                else callfunction((char *) ptr);
                k = lval[0] = 0;
            }
            else return (k);
        }
    }

    if(ptr==0) return (k);
    if(ptr[IDENT] == FUNCTION) {
        immed();
        outstr((char *) ptr);
        nl();
        return (0);
    }
    return (k);
}
