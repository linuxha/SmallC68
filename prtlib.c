 /*
        This file contains the source for the following
        library functions:
 
        printf          fprintf         sprintf         fmspr
        fputs
 
        These functions differ from the ones defined in K&R in
        one important aspect only -- they require a parameter
        count as the very last argument in the calling routine.

        For example:
               printf("Hello World !");      in K&R
        becomes:
               printf("Hello World !", 1);   for Small-C

        This abberation is caused by the fact that Small-C
        pushes its calling arguments in the order encountered,
        i.e. left to right, instead of the normal C conven-
        tion of right to left.  Therefore, the called routine
        must be somehow told the number of arguments present.

        Note that all the upper-level formatted I/O functions
        ("printf", "fprintf", and "sprintf") use
        "fmspr" for doing conversions.
 
        Note that temporary work space is declared within
        each of the high-level functions as a one-dimensional
        character array. The length limit on this array is
        presently set to 132 by the #define MAXLINE statement;
        if you intend to create longer lines through printf
        or fprintf calls, be SURE to raise this
        limit by changing the #define statement.
 
  */
 
  #define MAXLINE 132
  #define ERROR -1
  #define OK 0
 
  /*
        printf
 
        usage:
                printf(format, arg1, arg2, ...);
        
        Note that since the "fmspr" function is used to
        form the output string, and then "puts" is used to
        actually print it out, care must be taken to
        avoid generating null (zero) bytes in the output,
        since such a byte will terminate printing of the
        string by puts. Thus, a statement such as:
 
                printf("%c foo",'\0',2);
 
        would print nothing at all.
 
  */
 
  printf(count)
  int count;
  {
        char line[MAXLINE], *format;
        format = &count + count +count; /* point to 1st argument */
        fmspr(line,format);             /* use "fmspr" to form the output */
        puts(line);                     /* and print out the line        */
  }
 
 
  /*
        fprintf:
        Like printf, except that the first argument is
        a pointer to a buffered I/O buffer, and the text
        is written to the file described by the buffer:
        ERROR (-1) returned on error.
 
        usage:
                fprintf(iobuf, format, arg1, arg2, ..., argn, count);
                         where count = n + 2
  */
 
  fprintf(count)
  int count;
  {
        int iobuf, *format;
        char text[MAXLINE];
        format = &count + count + count; /* point to 1st argument */
        iobuf = *format--;
        fmspr(text,format);
        return fputs(text,iobuf);
  }
 
 
  /*
        sprintf:
        Like fprintf, except a string pointer is specified
        instead of a buffer pointer. The text is written
        directly into memory where the string pointer points.
 
        Usage:
                sprintf(string,format,arg1, arg2, ..., argN, count);
  */
 
  sprintf(count)
  int count;
  {
        char *buffer;
        int *format;
        format = &count + count + count;
        buffer = *format--;
        fmspr(buffer,format);  /* call fmspr to do all the work */
  }
 
 
  /*
        General formatted output conversion routine, used by
        fprintf and sprintf..."line" is where the output is
        written, and "fmt" is a pointer to an argument list
        which must consist of a format string pointer and
        subsequent list of (optional) values.
  */
 
 
  fmspr(line,fmt)
  char *line;
  int *fmt;
  {
        char c, base, *sptr, *format, uc, tf;
        char wbuf[MAXLINE], *wptr, pf, ljflag, zfflag;
        int width, precision,  *args;

        format = *fmt--;    /* fmt first points to the format string    */
        args = fmt;         /* now fmt points to the first arg value    */

        while (c = *format++) {
          if (c == '%') {
            wptr = wbuf;
            precision = 6;
            base = tf = ljflag = pf = zfflag = 0;
 
            if (*format == '-') {
                    format++;
                    ljflag++;
             }
 
 
            if (*format == '0') zfflag++;       /* zero-fill feature test */
 
            if (isdigit(*format))
                width = gv2(&format);
            else
                width = 0;
 
            if ((c = *format++) == '.') {
                    precision = gv2(&format);
                    pf++;
                    c = *format++;
             }
 
            uc = toupper(c);
            while(1) {
                if (uc == 'D') {
                    if (*args < 0) {
                        *wptr++ = '-';
                        *args = -*args;
                        width--;
                    }
                    base = 10;
                }
                else if (uc == 'U')
                    base = 10;
                else if (uc == 'X')
                    base = 16;
                else if (uc == 'O')
                    base = 8;

                if (base) {
                    width = width - uspr(&wptr, *args--, base);
                    tf++;
                }
 
                if (uc == 'C') {
                    *wptr++ = *args--;
                    width--;  tf++;
                }
                else if (uc == 'S') {
                    if (pf == 0) precision =200;
                    sptr = *args--;
                    while (*sptr && precision) {
                        *wptr++ = *sptr++;
                        precision--;
                        width--;
                    }
                    tf++;
                }
 
                if (tf) {
                    *wptr = 0;
                    wptr = wbuf;
                    if (ljflag == 0) {
                        while (width-- > 0) {
                            if (zfflag)
                                *line++ = '0';
                            else
                                *line++ = ' ';
                        }
                    }
                    while (*line = *wptr++)
                        line++;
                    if (ljflag)
                        while (width-- > 0)
                            *line++ = ' ';
                }
                else
                    *line++ = c;
 
                break;
             }
          }
          else *line++ = c;
        }
        *line = 0;
  }
 
  /*
        Internal routine used by "fmspr" to perform ascii-
        to-decimal conversion and update an associated pointer:
  */
 
  gv2(sptr)
  int *sptr;
  {
        int n;
        char *np;
        np = *sptr;
        n = 0;
        while (isdigit(*np)) n = 10 * n + *np++ - '0';
        *sptr = np;
        return n;
  }
 
 
  /*
        Internal function which converts n into an ASCII
        base `base' representation and places the text
        at the location pointed to by the pointer pointed
        to by `string'. Yes, you read that correctly.
  */

  uspr(string, n, base)
  int *string, n, base;
  {
      char length, *sp;
      int temp[6], i, old;
      i = 0;
      if (n < 0 )   {
          temp[i++] = n;      /* if neg, fake "unsigned" */
          n = (n >> 1) / (base >> 1);
          }
      while (n >= base)   {
          temp[i++] = n;
          n = n / base;
          }
      temp[i++] = n;
      length = i;             /* save return parameter */
      sp = *string;
      old = 0;
      while (i--)   {
          n = temp[i] - old * base;
          if(n < 10)
              *sp++ = n + '0';
          else
              *sp++ = n + 55;
          old = temp[i];
          }
      *string = sp;
      return length;
  }

  /*
        fputs:
        This function writes a string out to a buffered
        output file.
        ERROR (-1) returned on error.
  */
 
  fputs(s,iobuf)
  char *s;
  int iobuf;
  {
        char c;
        while (c = *s++) {
                if (putc(c,iobuf) == ERROR) return ERROR;
        }
        return OK;
  }
