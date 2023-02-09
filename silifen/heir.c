#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"

/* cc5.c9 R1.0 10/3/85  recursive descent expression evaluator */

/* lval[0] - symbol table address - 0 for constant */
/* lval[1] - type of indirect obj to fetch - 0 for static */
/* this file contains assignment, logical operators and comparisons */

// For 64 bit Linux char * == 8 bytes long
// For 64 bit Linux int    == 8 bytes long
void
expression() {
    long    lval[2];

    if ( heir1(lval) )
        rvalue(lval);
}

long
heir1(long *lval) {
    long    k;
    long    lval2[2];

    k = heir2(lval);
    if ( match("=") )          /* need an lvalue to put result in */
        {
            if ( k == 0 )
                {
                    needlval();
                    return 0;
                }
            if ( lval[1] )
                push();
            if ( heir1(lval2) )
                rvalue(lval2);
            store(lval);
            return 0;
        }
    else return k;
}

long
heir2(long *lval) {
    long    k;
    long    lval2[2];

    k = heir3(lval);
    blanks();

    if ( ch != '|' )
        return k;
    if ( k )
        rvalue(lval);
    while ( 1 )
        {
            if ( match("|") )
                {
                    push();
                    if ( heir3(lval2) )
                        rvalue(lval2);
                    or();              /* auto pop in or */
                }
            else return 0;
        }
}

long
heir3(long *lval) {
    long    k;
    long    lval2[2];

    k = heir4(lval);
    blanks();

    if ( ch != '^' )
        return k;
    if ( k )
        rvalue(lval);

    while ( 1 )
        {
            if ( match("^") )
                {
                    push();
                    if ( heir4(lval2) )
                        rvalue(lval2);
                    xor();             /* auto pop in xor */
                }
            else return 0;
        }
}

long
heir4(long *lval) {
    long    k;
    long    lval2[2];

    k = heir5(lval);
    blanks();

    if ( ch != '&' )
        return k;
    if ( k )
        rvalue(lval);

    while ( 1 )
        {
            if ( match("&") )
                {
                    push();
                    if ( heir5(lval2) )
                        rvalue(lval2);
                    and();             /* auto pop in and */
                }
            else return 0;
        }
}

long
heir5(long *lval) {
    long    k;
    long    lval2[2];

    k = heir6(lval);
    blanks();

    if((streq(line+lptr,"==")==0) & (streq(line+lptr,"!=") == 0 ))
        return k;
    if ( k )
        rvalue(lval);

    while ( 1 ) {
            if ( match("==") )
                {
                    push();
                    if ( heir6(lval2))
                        rvalue(lval2);
                    eq();  /* takes one param in D and other on stack */
                }
            else if ( match("!=") )
                {
                    push();
                    if ( heir6(lval2) )
                        rvalue(lval2);
                    ne();
                }
            else return 0;
        }
}

long
heir6(long *lval) {
    long    k;
    long    lval2[2];

    k = heir7(lval);
    blanks();

    if ( (streq(line + lptr,"<")     == 0 ) &
         (streq(line + lptr,">" )    == 0 ) &
         (streq(line + lptr , "<=" ) == 0 ) &
         (streq(line + lptr , ">=" ) == 0 ) )
        return k;
    if ( streq( line + lptr , ">>" ) )
        return k;
    if ( streq( line + lptr , "<<" ) )
        return k;
    if ( k )
        rvalue(lval);

    while(1) {
            if ( match("<=") )
                {
                    push();
                    if ( heir7(lval2) )
                        rvalue(lval2);
                    if ( cptr = *lval )
                        if (cptr[IDENT] == POINTER )
                            {
                                ule();
                                continue;
                            }
                    if ( cptr = *lval2 )
                        if ( cptr[IDENT] == POINTER )
                            {
                                ule();
                                continue;
                            }
                    le();
                }
            else if ( match(">=") )
                {
                    push();
                    if ( heir7(lval2) )
                        rvalue(lval2);
                    if ( cptr = *lval )
                        if ( cptr[IDENT] == POINTER )
                            {
                                uge();
                                continue;
                            }
                    if ( cptr = *lval2 )
                        if ( cptr[IDENT] == POINTER )
                            {
                                uge();
                                continue;
                            }
                    ge();
                }
            else if((streq(line+lptr,"<"))&(streq(line+lptr,"<<")==0))
                {
                    inbyte();
                    push();
                    if ( heir7(lval2) )
                        rvalue(lval2);
                    if ( cptr = *lval )
                        if ( cptr[IDENT] == POINTER )
                            {
                                ult();
                                continue;
                            }
                    if ( cptr = *lval2 )
                        if ( cptr[IDENT] == POINTER )
                            {
                                ult();
                                continue;
                            }
                    lt();
                }
            else if((streq(line+lptr,">"))&(streq(line+lptr,">>")==0))
                {
                    inbyte();
                    push();
                    if ( heir7(lval2) )
                        rvalue(lval2);
                    if ( cptr = *lval )
                        if ( cptr[IDENT] == POINTER )
                            {
                                ugt();
                                continue;
                            }
                    if ( cptr = *lval2 )
                        if ( cptr[IDENT] == POINTER )
                            {
                                ugt();
                                continue;
                            }
                    gt();
                }
            else return 0;
        }
}
/*  cc6.c9 R1.1 24/4/85  */

long
heir7(long *lval) {
    long    k;
    long    lval2[2];

    k = heir8(lval);
    blanks();

    if ((streq(line+lptr,">>")== 0)&(streq(line+lptr,"<")==0 ))
        return k;
    if ( k )
        rvalue(lval);

    while(1)
        {
            if ( match(">>") )
                {
                    push();
                    if ( heir8(lval2) )
                        rvalue(lval2);
                    casr();   /* auto stack pop */
                }
            else if ( match("<<") )
                {
                    push();
                    if ( heir8(lval2) )
                        rvalue(lval2);
                    casl();
                }
            else return 0;
        }
}

long
heir8(long *lval) {
    long    k;
    long    lval2[2];

    k = heir9(lval);
    blanks();

    if ( (ch != '-') & (ch != '+') )
        return k;
    if ( k )
        rvalue(lval);

    while(1) {
            if ( match("+") )
                {
                    push();
                    if ( heir9(lval2) )
                        rvalue(lval2);
                    if ( cptr = *lval )
                        if((cptr[IDENT]==POINTER)&(cptr[TYPE]==CINT))
                            scale(intwidth);
                    cadd();             /* auto pull in add */
                }
            else if ( match("-") )
                {
                    push();
                    if ( heir9(lval2) )
                        rvalue(lval2);
                    if ( cptr = *lval )
                        if((cptr[IDENT]==POINTER)&(cptr[TYPE]==CINT))
                            scale(intwidth);
                    csub();             /* auto pull in sub */
                }
            else return 0;
        }
}

long
heir9(long *lval) {
    long    k;
    long    lval2[2];

    k = heir10(lval);
    blanks();

    if ( (ch != '*') & (ch != '/') & (ch != '%') )
        return k;
    if ( k )
        rvalue(lval);

    while(1) {
            if ( match("*") )
                {
                    push();
                    if ( heir9(lval2) )     /**should it be heir10?**/
                        rvalue(lval2);
                    mult();            /* cleans up its own stack */
                }
            else if ( match("/") )
                {
                    push();
                    if ( heir10(lval2) )
                        rvalue(lval2);
                    myDiv();
                }
            else if ( match("%") )
                {
                    push();
                    if ( heir10(lval2) )
                        rvalue(lval2);
                    mod();
                }
            else return 0;
        }
}

long
heir10(long *lval) {
    long    k;
    char    *ptr;

    if (match("++") ) {
            if ((k=heir10(lval)) == 0 )
                {
                    needlval();
                    return 0;
                }
            if ( lval[1] )
                push();
            rvalue(lval);
            ptr = *lval;
            if ( (ptr[IDENT] == POINTER ) & (ptr[TYPE] == CINT ))
                cinc( intwidth );
            else
                cinc( 1 );
            store(lval);
            return 0;
        }
    if (match("--") )
        {
            if ((k=heir10(lval)) == 0 )
                {
                    needlval();
                    return 0;
                }
            if ( lval[1] )
                push();
            rvalue(lval);
            ptr = *lval;
            if ( (ptr[IDENT] == POINTER ) & (ptr[TYPE] == CINT ))
                cdec( intwidth );
            else
                cdec( 1 );
            store(lval);
            return 0;
        }
    if (match("-") )
        {
            k = heir10(lval);
            if ( k )
                rvalue(lval);
            cneg();
            return 0;
        }
    else if ( match("*") )   /* take contents of pointer */
        {
            k = heir10(lval);
            if ( k )
                rvalue(lval);
            lval[1] = CINT;
            if ( ptr = *lval )
                lval[1] = ptr[TYPE];
            *lval = 0;
            return 1;
        }
    else if ( match("&") )     /* take address of variable */
        {
            k = heir10(lval);
            if ( k == 0 )
                {
                    error("illegal address");
                    return 0;
                }
            else if ( lval[1] )
                return 0;
            else
                {
                    getglb();
                    outstr(ptr = *lval );
                    nl();
                    lval[1] = ptr[TYPE];
                    return 0;
                }
        }
    else
        {
            k = heir11(lval);
            if ( match("++") )       /* post increment */
                {
                    if ( k == 0 )
                        {
                            needlval();
                            return 0;
                        }
                    if ( lval[1] )
                        push();
                    rvalue(lval);
                    ptr = *lval;
                    if((ptr[IDENT]==POINTER)&(ptr[TYPE]==CINT))
                        {
                            cinc( intwidth );
                            store(lval);
                            cdec( intwidth );
                        }
                    else
                        {
                            cinc( 1 );
                            store(lval);
                            cdec( 1 );
                        }
                    /* the value is incremented and stored then decremented before use */
                    return 0;
                }
            else if (match("--") )   /* post decrement */
                {
                    if ( k == 0 )
                        {
                            needlval();
                            return 0;
                        }
                    if ( lval[1] )
                        push();
                    rvalue(lval);
                    ptr = *lval;
                    if((ptr[IDENT]==POINTER)&(ptr[TYPE]==CINT))
                        {
                            cdec( intwidth );
                            store(lval);
                            cinc( intwidth );
                        }
                    else
                        {
                            cdec( 1 );
                            store(lval);
                            cinc( 1 );
                        }
                    return 0;
                }
            else return k;
        }
}
/*  cc7.c9 R1.1 24/4/85  */

long
heir11(long *lval) {
    long    k;
    char    *ptr;

    k = primary(lval);
    ptr = *lval;
    blanks();

    if ( (ch == '[' ) | ( ch == '(' ) )
        while(1)
            {
                if ( match("[") )
                    {
                        if (ptr)
                            {
                                if ( ptr[IDENT] == POINTER )
                                    rvalue(lval);
                            }
                        push();
                        expression();
                        needbrack("]");
                        if (ptr)
                            if ( ptr[TYPE] == CINT )
                                scale(intwidth);
                        cadd();             /* auto pull */
                        *lval = 0;
                        if (ptr)
                            lval[1] = ptr[TYPE];
                        else
                            lval[1] = 0;
                        k = 1;
                    }
                else if ( match("(") )
                    {
                        if ( ptr == 0 )
                            {
                                callfunction(0);
                            }
                        else if ( ptr[IDENT] != FUNCTION )
                            {
                                rvalue(lval);
                                callfunction(0);
                            }
                        else callfunction(ptr);
                        k = *lval =0;
                    }
                else return k;
            }
    if ( ptr == 0 )
        return k;
    if ( ptr[IDENT] == FUNCTION ) {
        getglb();
        outstr(ptr);
        nl();
        return 0;
    }

    return k;
}
