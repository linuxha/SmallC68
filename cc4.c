/*      >>>>>> start of cc4 <<<<<<<     */

keepch(c)
        char c;
{       mline[mptr]=c;
        if(mptr<MPMAX)mptr++;
        return (c);
}
preprocess()
{       int k;
        char c,sname[NAMESIZE];
        if(cmode==0)return;
        mptr=lptr=0;
        while(ch())
                {if((ch()==' ')|(ch()==9))
                        {keepch(' ');
                        while((ch()==' ')|
                                (ch()==9))
                                gch();
                        }
                else if(ch()=='"')
                        {keepch(ch());
                        gch();
                        while(ch()!='"')
                                {if(ch()==0)
                                  {error("missing quote");
                                  break;
                                  }
                                keepch(gch());
                                }
                        gch();
                        keepch('"');
                        }
                else if(ch()==39)
                        {keepch(39);
                        gch();
                        while(ch()!=39)
                                {if(ch()==0)
                                  {error("missing apostrophe");
                                  break;
                                  }
                                keepch(gch());
                                }
                        gch();
                        keepch(39);
                        }
                else if((ch()=='/')&(nch()=='*'))
                        {inchar();inchar();
                        while(((ch()=='*')&
                                (nch()=='/'))==0)
                                {if(ch()==0)inline();
                                        else inchar();
                                if(eof)break;
                                }
                        inchar();inchar();
                        }
                else if(an(ch()))
                        {k=0;
                        while(an(ch()))
                                {if(k<NAMEMAX)sname[k++]=ch();
                                gch();
                                }
                        sname[k]=0;
                        if(k=findmac(sname))
                                while(c=macq[k++])
                                        keepch(c);
                        else
                                {k=0;
                                while(c=sname[k++])
                                        keepch(c);
                                }
                        }
                else keepch(gch());
                }
        keepch(0);
        if(mptr>=MPMAX)error("line too long");
        lptr=mptr=0;
        while(line[lptr++]=mline[mptr++]);
        lptr=0;
        }
addmac()
{       char sname[NAMESIZE];
        int k;
        if(symname(sname)==0)
                {illname();
                kill();
                return;
                }
        k=0;
        while(putmac(sname[k++]));
        while(ch()==' ' | ch()==9) gch();
        while(putmac(gch()));
        if(macptr>=MACMAX)error("macro table full");
        }
putmac(c)
        char c;
{       macq[macptr]=c;
        if(macptr<MACMAX)macptr++;
        return (c);
}
findmac(sname)
        char *sname;
{       int k;
        k=0;
        while(k<macptr)
                {if(astreq(sname,macq+k,NAMEMAX))
                        {while(macq[k++]);
                        return (k);
                        }
                while(macq[k++]);
                while(macq[k++]);
                }
        return (0);
}

outbyte(c)
        char c;
{
        if(c==0)return (0);
        if(output)
                {if((putc(c,output))<=0)
                        {closeout();
                        error("Output file error");
                        abort();
                        }
                }
        else putchar(c);
        return (c);
}
outstr(ptr)
        char *ptr;
 {
        while(outbyte(*ptr++));
 }

nl()
        {outbyte(EOL);}
tab()
        {outbyte(' ');}
col()
        {outbyte(' ');}
bell()
        {outbyte(7);}

error(ptr)
char ptr[];
{       int k;
        char yunk[81];

        toconsole();
        bell();
        outstr("Line "); outdec(lineno); outstr(", ");
        if(infunc==0)
                outbyte('(');
        if(currfn==NULL)
                outstr("start of file");
        else    outstr(currfn+NAME);
        if(infunc==0)
                outbyte(')');
        outstr(" + ");
        outdec(lineno-fnstart);
        outstr(": ");  outstr(ptr);  nl();

        outstr(line); nl();

        k=0;    /* skip to error position */
        while(k<lptr){
                if(line[k++]==9)
                        tab();
                else    outbyte(' ');
                }
        outbyte('^');  nl();
        ++errcnt;

        if(errstop){
                pl("Continue (Y,n,g) ? ");
                gets(yunk);             
                k=yunk[0];
                if((k=='N') | (k=='n'))
                        abort();
                if((k=='G') | (k=='g'))
                        errstop=0;
                }
        tofile();
/* end error */}

ol(ptr)
        char ptr[];
{
        ot(ptr);
        nl();
}
ot(ptr)
        char ptr[];
{
        tab();
        outstr(ptr);
}
match(lit)
        char *lit;
{
        int k;
        blanks();
        if (k=streq(line+lptr,lit))
                {lptr=lptr+k;
                return (1);
                }
        return (0);
}
amatch(lit,len)
        char *lit;int len;
 {
        int k;
        blanks();
        if (k=astreq(line+lptr,lit,len))
                {lptr=lptr+k;
                while(an(ch())) inbyte();
                return (1);
                }
        return (0);
 }
blanks()
        {while(1)
                {while(ch()==0)
                        {inline();
                        preprocess();
                        if(eof)break;
                        }
                if(ch()==' ')gch();
                else if(ch()==9)gch();
                else return;
                }
        }
outdec(numb)
        int numb;
 {
        int k,zs;
        char c;
        zs = 0;
        k=10000;
        if (numb<0)
                {numb=(-numb);
                outbyte('-');
                }
        while (k>=1)
                {
                c=numb/k + '0';
                if ((c!='0')|(k==1)|(zs))
                        {zs=1;outbyte(c);}
                numb=numb%k;
                k=k/10;
                }
 }

