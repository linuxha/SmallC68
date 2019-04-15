/*      >>>>>>> start of cc5 <<<<<<<    */

expression()
{
        int lval[2];
        if(heir1(lval))rvalue(lval);
}
heir1(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir2(lval);
        if (match("="))
                {if(k==0){needlval();return (0);}
                if (lval[1])zpush();
                if(heir1(lval2))rvalue(lval2);
                store(lval);
                return (0);
                }
        else return (k);
}
heir2(lval)
        int lval[];
{       int k,lval2[2];
        k=heir3(lval);
        blanks();
        if(ch()!='|')return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("|"))
                        {zpush();
                        if(heir3(lval2)) rvalue(lval2);
                        zpop();
                        zor();
                        }
                else return (0);
                }
}
heir3(lval)
        int lval[];
{       int k,lval2[2];
        k=heir4(lval);
        blanks();
        if(ch()!='^')return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("^"))
                        {zpush();
                        if(heir4(lval2))rvalue(lval2);
                        zpop();
                        zxor();
                        }
                else return (0);
                }
}
heir4(lval)
        int lval[];
{       int k,lval2[2];
        k=heir5(lval);
        blanks();
        if(ch()!='&')return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("&"))
                        {zpush();
                        if(heir5(lval2))rvalue(lval2);
                        zpop();
                        zand();
                        }
                else return (0);
                }
}
heir5(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir6(lval);
        blanks();
        if((streq(line+lptr,"==")==0)&
                (streq(line+lptr,"!=")==0))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("=="))
                        {zpush();
                        if(heir6(lval2))rvalue(lval2);
                        zpop();
                        zeq();
                        }
                else if (match("!="))
                        {zpush();
                        if(heir6(lval2))rvalue(lval2);
                        zpop();
                        zne();
                        }
                else return (0);
                }
}
heir6(lval)
        int lval[];
{
        int k,lval2[2];
        k=heir7(lval);
        blanks();
        if((streq(line+lptr,"<")==0)&
                (streq(line+lptr,">")==0)&
                (streq(line+lptr,"<=")==0)&
                (streq(line+lptr,">=")==0))return (k);
                if(streq(line+lptr,">>"))return (k);
                if(streq(line+lptr,"<<"))return (k);
        if(k)rvalue(lval);
        while(1)
                {if (match("<="))
                        {zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {ule();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {ule();
                                continue;
                                }
                        zle();
                        }
                else if (match(">="))
                        {zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {uge();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {uge();
                                continue;
                                }
                        zge();
                        }
                else if((streq(line+lptr,"<"))&
                        (streq(line+lptr,"<<")==0))
                        {inbyte();
                        zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {ult();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {ult();
                                continue;
                                }
                        zlt();
                        }
                else if((streq(line+lptr,">"))&
                        (streq(line+lptr,">>")==0))
                        {inbyte();
                        zpush();
                        if(heir7(lval2))rvalue(lval2);
                        zpop();
                        if(cptr=lval[0])
                                if(cptr[IDENT]==POINTER)
                                {ugt();
                                continue;
                                }
                        if(cptr=lval2[0])
                                if(cptr[IDENT]==POINTER)
                                {ugt();
                                continue;
                                }
                        zgt();
                        }
                else return (0);
                }
}
