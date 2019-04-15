/*      >>>>>> start of cc7 <<<<<<      */

heir11(lval)
        int *lval;
{       int k;char *ptr;
        k=primary(lval);
        ptr=lval[0];
        blanks();
        if((ch()=='[')|(ch()=='('))
        while(1)
                {if(match("["))
                        {if(ptr==0)
                                {error("can't subscript");
                                junk();
                                needbrack("]");
                                return (0);
                                }
                        else if(ptr[IDENT]==POINTER)rvalue(lval);
                        else if(ptr[IDENT]!=ARRAY)
                                {error("can't subscript");
                                k=0;
                                }
                        zpush();
                        expression();
                        needbrack("]");
                        if(ptr[TYPE]==CINT)doublereg();
                        zpop();
                        zadd();
                        lval[1]=ptr[TYPE];
                        k=1;
                        }
                else if(match("("))
                        {if(ptr==0)
                                {callfunction(0);
                                }
                        else if(ptr[IDENT]!=FUNCTION)
                                {rvalue(lval);
                                callfunction(0);
                                }
                        else callfunction(ptr);
                        k=lval[0]=0;
                        }
                else return (k);
                }
        if(ptr==0)return (k);
        if(ptr[IDENT]==FUNCTION)
                {immed();
                outstr(ptr);
                nl();
                return (0);
                }
        return (k);
}
primary(lval)
        int *lval;
{       char *ptr,sname[NAMESIZE];int num[1];
        int k;
        if(match("("))
                {k=heir1(lval);
                needbrack(")");
                return (k);
                }
        if(symname(sname))
                {if(ptr=findloc(sname))
                        {getloc(ptr);
                        lval[0]=ptr;
                        lval[1]=ptr[TYPE];
                        if(ptr[IDENT]==POINTER)lval[1]=CINT;
                        if(ptr[IDENT]==ARRAY)return (0);
                                else return (1);
                        }
                if(ptr=findglb(sname))
                        if(ptr[IDENT]!=FUNCTION)
                        {lval[0]=ptr;
                        lval[1]=0;
                        if(ptr[IDENT]!=ARRAY)return (1);
                        immed();
                        outstr(ptr);nl();
                        lval[1]=ptr[TYPE];
                        return (0);
                        }
                ptr=addglb(sname,FUNCTION,CINT,0);
                lval[0]=ptr;
                lval[1]=0;
                return (0);
                }
        if(constant(num))
                return(lval[0]=lval[1]=0);
        else
                {error("invalid expression");
                immed();outdec(0);nl();
                junk();
                return (0);
                }
        }
store(lval)
        int *lval;
{       if (lval[1]==0)putmem(lval[0]);
        else putstk(lval[1]);
}
rvalue(lval)
        int *lval;
{       if((lval[0] != 0) & (lval[1] == 0))
                getmem(lval[0]);
                else indirect(lval[1]);
}
test(label)
        int label;
{
        needbrack("(");
        expression();
        needbrack(")");
        testjump(label);
}
constant(val)
        int val[];
{
        if (number(val))
                immed();
        else if (pstr(val))
                immed();
        else if (qstr(val))
                {immed();printlabel(litlab);outbyte("+ ");}
        else return(0);
        outdec(val[0]);
        nl();
          return(1);
}
number(val)
        int val[];
{
        int k, minus; char c;
        k = minus = 1;
        while(k)
                {k = 0;
                if (match("+")) k = 1;
                if (match("-")) {minus = (-minus); k = 1;}
                }
        if (numeric(ch()) == 0) return(0);
        while (numeric(ch()))
                {c = inbyte();
                k = k * 10 + (c - '0');
                }
        if (minus < 0) k = (-k);
        val[0] = k;
        return(1);
}
pstr(val)
        int val[];
{
        int k; char c;
        k = 0;
        if (match("'") == 0) return(0);
        while ((c = gch()) != 39)
                k = (k & 255) * 256 + (c & 127);
        val[0] = k;
        return(1);
}
qstr(val)
        int val[];
{
        char c;
        if (match(quote) == 0) return(0);
        val[0] = litptr;
        while (ch() != '"')
                {if (ch() == 0) break;
                if (litptr >= LITMAX)
                        {error("string space exhausted");
                        while (match(quote) == 0)
                                if (gch() == 0) break;
                        return(1);
                        }
                litq[litptr++]=gch();
                }
        gch();
        litq[litptr++]=0;
        return (1);
}
