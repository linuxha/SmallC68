/* printf.c R0.2 30/6/85 Silicon Fen Software */

printf(args)
/* formatted output */
        int     args;
        {
        int width,prec,preclen,len,*nxtarg;
        char *ctl,*cx,c,right,str[20],*sptr,pad;
        int i;
#asm
        std     ,s      save argument bytecount in i
#endasm
        nxtarg = &args + (i - 2);
        ctl = *nxtarg;
        while(c = *ctl++)
                {
                if (c != '%')   { putchar(c); continue; }       /* chars */
                if (*ctl == '%'){ putchar(*ctl++);continue;}    /* %'s */
                cx = ctl;
                if (*cx == '-') { right=0; ++cx; } else right = 1;
                if (*cx == '0') { pad='0'; ++cx; } else pad = ' ';
                prec = preclen = width = 0;
                cx = cx + btoi(cx,8,&width,10);
/*              putdec(width,1);puts(" width"); */
                width = abs(width);
                if (*cx == '.')
                        {
                        preclen = btoi(++cx,8,&prec,10);
                        cx = cx + preclen;
                        }
/*              putdec(prec,1);putdec(preclen,1);puts(" prec preclen"); */
                prec = abs(prec);
                sptr = str;
                c = *cx++;
                i = *(--nxtarg);
                if (c == 'd') str[itob(str,i,-10)]=0;
                else if (c == 'x') str[itob(str,i,16)]=0;
                else if (c == 'c') {str[0] = i; str[1] = 0;}
                else if (c == 's') sptr = i;
                else if (c == 'u') str[itob(str,i,10)]=0;
                else { putchar(c); continue; }
                ctl = cx;
                len = strlen(sptr);
                if ((c == 's')&(len>prec)&(preclen>0)) len = prec;
  /*            putdec(len,1);puts(" len"); */
                if (right) while(((width--)-len)>0) putchar(pad);
                while(len) { putchar(*sptr++); --len; --width; }
                while(((width--)-len)>0) putchar(pad);
                }
        }

