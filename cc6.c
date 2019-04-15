/*      >>>>>> start of cc6 <<<<<<      */

heir7(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir8(lval);
        blanks();
        if((streq(line+lptr,">>")==0)&
                (streq(line+lptr,"<<")==0))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match(">>"))
                        {zpush();
                        if(heir8(lval2))rvalue(lval2);
                        zpop();
                        asr();
                        }
                else if (match("<<"))
                        {zpush();
                        if(heir8(lval2))rvalue(lval2);
                        zpop();
                        asl();
                        }
                else return (0);
                }
}
heir8(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir9(lval);
        blanks();
        if((ch()!='+')&(ch()!='-'))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("+"))
                        {zpush();
                        if(heir9(lval2))rvalue(lval2);
                        if(cptr=lval[0])
                                if((cptr[IDENT]==POINTER)&
                                (cptr[TYPE]==CINT))
                                doublereg();
                        zpop();
                        zadd();
                        }
                else if (match("-"))
                        {zpush();
                        if(heir9(lval2))rvalue(lval2);
                        if(cptr=lval[0])
                                if((cptr[IDENT]==POINTER)&
                                (cptr[TYPE]==CINT))
                                doublereg();
                        zpop();
                        zsub();
                        }
                else return (0);
                }
}
heir9(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir10(lval);
        blanks();
        if((ch()!='*')&(ch()!='/')&
                (ch()!='%'))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("*"))
                        {zpush();
                        if(heir9(lval2))rvalue(lval2);
                        zpop();
                        mult();
                        }
                else if (match("/"))
                        {zpush();
                        if(heir10(lval2))rvalue(lval2);
                        zpop();
                        div();
                        }
                else if (match("%"))
                        {zpush();
                        if(heir10(lval2))rvalue(lval2);
                        zpop();
                        zmod();
                        }
                else return (0);
                }
}
heir10(lval)
        int lval[];
{
        int k;
        char *ptr;
        if(match("++"))
                {if((k=heir10(lval))==0)
                        {needlval();
                        return (0);
                        }
                if(lval[1])zpush();
                rvalue(lval);
                inc();
                ptr=lval[0];
                if((ptr[IDENT]==POINTER)&
                        (ptr[TYPE]==CINT))
                                inc();
                store(lval);
                return (0);
                }
        else if(match("--"))
                {if((k=heir10(lval))==0)
                        {needlval();
                        return (0);
                        }
                if(lval[1])zpush();
                rvalue(lval);
                dec();
                ptr=lval[0];
                if((ptr[IDENT]==POINTER)&
                        (ptr[TYPE]==CINT))
                                dec();
                store(lval);
                return (0);
                }
        else if (match("-"))
                {k=heir10(lval);
                if (k) rvalue(lval);
                neg();
                return (0);
                }
        else if(match("*"))
                {k=heir10(lval);
                if(k)rvalue(lval);
                lval[1]=CINT;
                if(ptr=lval[0])lval[1]=ptr[TYPE];
                lval[0]=0;
                return (1);
                }
        else if(match("&"))
                {k=heir10(lval);
                if(k==0)
                        {error("illegal address");
                        return (0);
                        }
                else if(lval[1])return (0);
                else
                        {immed();
                        outstr(ptr=lval[0]);
                        nl();
                        lval[1]=ptr[TYPE];
                        return (0);
                        }
                }
        else 
                {k=heir11(lval);
                if(match("++"))
                        {if(k==0)
                                {needlval();
                                return (0);
                                }
                        if(lval[1])zpush();
                        rvalue(lval);
                        inc();
                        ptr=lval[0];
                        if((ptr[IDENT]==POINTER)&
                                (ptr[TYPE]==CINT))
                                        inc();
                        store(lval);
                        dec();
                        if((ptr[IDENT]==POINTER)&
                                (ptr[TYPE]==CINT))
                                dec();
                        return (0);
                        }
                else if(match("--"))
                        {if(k==0)
                                {needlval();
                                return (0);
                                }
                        if(lval[1])zpush();
                        rvalue(lval);
                        dec();
                        ptr=lval[0];
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
