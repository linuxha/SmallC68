/* itob and btoi derived from WANB's discompiled versions of itob and btoi */

/* ANC 22/8/84 */
/*
  #asm
  IF ITOB
  *itob(s,i,base)
  * integer to ascii, s=buffer, i=integer, base is -36..36, -ve for signed
  *      char    *s;
  *      int     i,base;
  #endasm
*/
/* integer to ascii, s=buffer, i=integer, base is -36..36, -ve for signed */
char *
itob(char *s, int i, int base) { 
    int     val, t;
    char    *qbuf;

    qbuf = s;

    /* correct for sign of i */
    if ( (i < 0) & (base <= 0) ) {
        val = -i;
        *qbuf++ = '-';
    } else {
        val = i;
    }
    /* check base */
    if (base == 0) {
        base = 10;
    } else {
        if (base < 0) {
            base = -base;
        }
    }
    /* pick digits off recursively */
    if ( val >= base ) {
        t    = itob(qbuf,val/base,base);
        qbuf = qbuf[t];
        //qbuf = qbuf + itob(qbuf,val/base,base);
    }

    if ((*qbuf = (val % base) + '0') > '9' )
        *qbuf = *qbuf + 7; /* ( 'A'-'9'-1 ) */

    /* return number of digits converted */
    return (qbuf - s + 1);
}
/*
  #asm
  ENDIF

  IF BTOI
  * btoi(s,n,pinum,base)
  * ascii to integer, s=buffer,n=field width,pinum=*int for result,base to 36 
  *      char    *s;
  *      int     n, *pinum, base;
  #endasm
*/
/* ascii to integer, s=buffer,n=field width,pinum=*int for result,base to 36 */
char *
btoi(char *s, int n, int *pinum, int base) {
    int     answer, digit, minus, c;
    char    *p;

    p = s;
    answer = minus = 0;      /* assume positive */
    /* skip white */
    while( (n > 0) & ( *s == ' ') ) {
        --n;
        ++s;
    }

    /* look for + or - */
    if (n > 0) {
        if (*s == '-') {
            minus = 1;
            ++s;
            --n;
        } else {
            if (*s == '+') {
                ++s;
                --n;
            }
        }
    }

    /* check for 0x or 0X */
    if ((base==16) && (n>=2)) {
        if ((*s=='0') && ((s[1]=='x') || (s[1]=='X'))) {
            s = s+2;
            n = n-2;
        }
    }

    /* convert chars */
    while( n > 0 ) {
        c = *s;
        if ((c >= '0')&(c <= '9')) {
            if ( (digit = c - '0')  >= base)
                break;
        } else if ((c >= 'A')&(c <= 'Z')) {        /* 55 == 'A' - 10 */
            if ( (digit = c - 55 ) >= base) {
                break;
            }
        } else if ((c >= 'a')&(c <= 'z')) {        /* 87 == 'a' - 10 */
            if ( (digit = c - 87) >= base) {
                break;
            }
        } else {
            break;
        }

        answer = answer*base + digit;
        --n;
        ++s;
    }
    /* store answer */
    if (minus)
        *pinum = -answer;
    else
        *pinum = answer;

    /* return number of chars converted */
    return (s-p);
}
/*
  #asm
  ENDIF
  #endasm
*/
