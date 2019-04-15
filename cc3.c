/*      >>>>> start of cc3 <<<<<<<<<    */

/*                                      */
/*      Perform a function call         */
/*                                      */
/* called from heir11, this routine will either call */
/*      the named function, or if the supplied ptr is */
/*      zero, will call the contents of primary register */
callfunction(ptr)
        char *ptr;      /* symbol table entry (or 0) */
{       int nargs;
        nargs=0;
        blanks();       /* already saw open paren */
        if(ptr==0)zpush();      /* calling primary register */
        while(streq(line+lptr,")")==0)
                {if(endst())break;
                expression();   /* get an argument */
                if(ptr==0)swapstk(); /* don't push addr */
                zpush();        /* push argument */
                nargs=nargs+2;  /* count args*2 */
                if (match(",")==0) break;
                }
        needbrack(")");
        if(ptr)zcall(ptr);
        else callstk();
        Zsp=modstk(Zsp+nargs);  /* clean up arguments */
}
junk()
{       if(an(inbyte()))
                while(an(ch()))gch();
        else while(an(ch())==0)
                {if(ch()==0)break;
                gch();
                }
        blanks();
}
endst()
{       blanks();
        return ((streq(line+lptr,";")|(ch()==0)));
}
illname()
{       error("illegal symbol name");junk();}
multidef(sname)
        char *sname;
{       error("already defined");
        comment();
        outstr(sname);nl();
}
needbrack(str)
        char *str;
{       if (match(str)==0)
                {error("missing bracket");
                comment();outstr(str);nl();
                }
}
needlval()
{       error("must be lvalue");
}
findglb(sname)
        char *sname;
{       char *ptr;
        ptr=STARTGLB;
        while(ptr!=glbptr)
                {if(astreq(sname,ptr,NAMEMAX))return (ptr);
                ptr=ptr+SYMSIZ;
                }
        return (0);
}
findloc(sname)
        char *sname;
{       char *ptr;
        ptr=STARTLOC;
        while(ptr!=locptr)
                {if(astreq(sname,ptr,NAMEMAX))return (ptr);
                ptr=ptr+SYMSIZ;
                }
        return (0);
}
addglb(sname,id,typ,value)
        char *sname,id,typ;
        int value;
{       char *ptr;
        if(cptr=findglb(sname))return (cptr);
        if(glbptr>=ENDGLB)
                {error("global symbol table overflow");
                return (0);
                }
        cptr=ptr=glbptr;
        while(an(*ptr++ = *sname++));   /* copy name */
        cptr[IDENT]=id;
        cptr[TYPE]=typ;
        cptr[STORAGE]=STATIK;
        cptr[OFFSET]=value>>8;
        cptr[OFFSET+1]=value&255;
        glbptr=glbptr+SYMSIZ;
        return (cptr);
}
addloc(sname,id,typ,value)
        char *sname,id,typ;
        int value;
{       char *ptr;
        if(cptr=findloc(sname))return (cptr);
        if(locptr>=ENDLOC)
                {error("local symbol table overflow");
                return (0);
                }
        cptr=ptr=locptr;
        while(an(*ptr++ = *sname++));   /* copy name */
        cptr[IDENT]=id;
        cptr[TYPE]=typ;
        cptr[STORAGE]=STKLOC;
        cptr[OFFSET]=value>>8;
        cptr[OFFSET+1]=value&255;
        locptr=locptr+SYMSIZ;
        return (cptr);
}
/* Test if next input string is legal symbol name */
symname(sname)
        char *sname;
{       int k;char c;
        blanks();
        if(alpha(ch())==0)return (0);
        k=0;
        while(an(ch()))sname[k++]=gch();
        sname[k]=0;
        return (1);
        }
/* Return next avail internal label number */
getlabel()
{       return(++nxtlab);
}
/* Print specified number as label */
printlabel(label)
        int label;
{       outstr("cc");
        outdec(label);
}
/* Print a carriage return and a string only to console */
pl(str)
        char *str;
{       int k;
        k=0;
        putchar(EOL);
        while(str[k])putchar(str[k++]);
}
addwhile(ptr)
        int ptr[];
 {
        int k;
        if (wqptr==WQMAX)
                {error("too many active whiles");return;}
        k=0;
        while (k<WQSIZ)
                {*wqptr++ = ptr[k++];}
}
delwhile()
        {if(readwhile()) wqptr=wqptr-WQSIZ;
        }
readwhile()
 {
        if (wqptr==wq){error("no active whiles");return (0);}
        else return (wqptr-WQSIZ);
 }
kill()
{       lptr=0;
        line[lptr]=0;
}
inbyte()
{
        while(ch()==0)
                {if (eof) return (0);
                inline();
                preprocess();
                }
        return (gch());
}
inchar()
{
        if(ch()==0)inline();
        if(eof)return (0);
        return(gch());
}
inline()
{
        int k,unit;
        while(1)
                {if (input==0)openin();
                if(eof)return;
                if((unit=input2)==0)unit=input;
                kill();
                while((k=getc(unit))>0)
                        {if((k==EOL)|(lptr>=LINEMAX))break;
                        line[lptr++]=k;
                        }
                line[lptr]=0;   /* append null */
                lineno++;       /* read one more line */
                if(k<=0)
                        {fclose(unit);
                        if(input2)endinclude();
                                else input=0;
                        }
                if(lptr)
                        {if((ctext)&(cmode))
                                {comment();
                                outstr(line);
                                nl();
                                }
                        lptr=0;
                        return;
                        }
                }
}
