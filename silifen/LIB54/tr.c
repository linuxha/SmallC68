/* tr.c  V0.7  4/7/85    anc      translated on the fly from ratfor */

/*
Characters in 'from' are changed into the equivalent character in 'to'.
If 'to' contains less characters than 'from' then the excess in 'from' is
Collapsed onto the last char in 'to'.

e.g.    +++tr a b infile outfile        change all 'a' in infile to 'b'

        +++tr abc d infile outfile      change all 'a', 'b' and 'c' in
                                        infile to 'd'

As a shorthand, ranges of characters can be used using '-'.

e.g.    +++tr A-Z a-z infile outfile    change infile to lower case

        +++tr 0-9 # infile outfile      change all numbers to #

Special characters can be specified using '\' as an escape character.

        \n      ascii 13 (flex) newline
        \t      ascii 9 tab
        \f      ascii 12 formfeed
        \b      ascii 8 backspace
        \l      ascii 10 linefeed
        \r      ascii 13 return
        \s      ascii 32 space
        \\      backslash

The set of chars can be complemented using '~'.
*/

#include "stdio.h"

#define MAXSET 1000

#define NOT '~'         /* special char to invert characters in from */
#define DELETE "~"      /* special string to delete chars in to */
#define ESCAPE '\\'     /* to embed \n and \t etc */
#define DASH '-'        /* subrange   a-z  etc */

main(argc,argv)
        int argc;
        char *argv[];
        {
        char from[MAXSET], to[MAXSET];
        char *s;
        int  c,allbut,collap,i,lastto;
        FILE *infile, *outfile;

        if (argc < 4)
                error("\nTransliterate characters V0.7 4/7/85 - useage: tr from to infile [outfile]\n");
        if ((infile = fopen(argv[3], "r")) == NULL)
                error("\nCan't open input file\n");
        if (argc > 4)
                {
                if ((outfile = fopen(argv[4], "w")) == NULL)
                        error("\nCan't open output file\n");
                }
        else
                outfile = stdout;

        s = argv[1];
        if ( *s == NOT )
                {
                allbut = YES;
                if (makeset(s+1,from,MAXSET) == NO)
                        error("\nfrom: too large\n");
                }
        else
                {
                allbut = NO;
                if (makeset(s,from,MAXSET) == NO)
                        error("\nfrom: too large\n");
                }
        if (strcmp(argv[2],DELETE)==0)     /* delete char */
                to[0] = NULL;
        else
                if (makeset(argv[2],to,MAXSET) == NO)
                        error("\nto: too large\n");

        lastto = strlen(to);
        if ((strlen(from) > lastto) | (allbut == YES))
                collap = YES;
        else
                collap = NO;
        --lastto;               /* change from count to index */

        printf("From: %s\nTo:   %s\n",from,to);
        while(1)
                {
                i = xindex(from, c = fgetc(infile), allbut, lastto);
                if ((collap == YES) & (i >= lastto) & (lastto >= 0))
                        {       /* collapse */
                        fputc(to[lastto], outfile);
                        do
                                i = xindex(from,c=fgetc(infile),allbut,lastto);
                        while(i >= lastto);
                        }
                if (c == EOF)
                        break;
                if ((i >= 0) & (lastto >= 0))     /* translate */
                        putc(to[i], outfile);
                else if (i < 0)                /* copy */
                        putc(c, outfile);

                /* else delete */
                }
        }


error(s)
/* give up after dumping error message */
        char *s;
        {
        while(*s) putchar(*s++);
        exit(1);
        }

makeset(s, set, size)
/* make set from string */
        char *s, *set;
        int size;
        {
        int j;
        j = 0;
        filset(NULL, s, set, &j, size);
        return addset( NULL, set, &j, size);
        }

addset(c, set, pj, maxsize)
        char c,*set;
        int *pj, maxsize;
        {
        if (*pj >= maxsize)
                return NO;
        else
                {
                set[*pj] = c;
                *pj = *pj + 1;
                return YES;
                }
        }

/* expand s into set stop at delim */
filset(delim, s, set, pj, maxset)
        char delim, *s, *set;
        int *pj, maxset;
        {
        char *digits,*lowalf,*upalf;

        digits = "0123456789";
        lowalf = "abcdefghijklmnopqrstuvwxyz";
        upalf = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        while((*s != delim) & (*s != NULL))
                {
                if (*s == ESCAPE)
                        addset( esc(&s), set, pj, maxset);
                else if (*s != DASH)
                        addset(*s, set, pj, maxset);
                else if ((*pj <= 0) | (s[1] == NULL))
                        addset( DASH, set, pj, maxset);
                else if (index(digits, set[(*pj)-1]) > 0)
                        {
                        ++s;
                        dodash(digits, &s, set, pj, maxset);
                        }
                else if (index(lowalf, set[*pj-1]) > 0)
                        {
                        ++s;
                        dodash(lowalf, &s, set, pj, maxset);
                        }
                else if (index(upalf, set[*pj-1]) > 0)
                        {
                        ++s;
                        dodash(upalf, &s, set, pj, maxset);
                        }
                else
                        addset(DASH, set, pj, maxset);
                ++s;
                }
        }


/* map *s onto escaped character */
esc(ps)
        int *ps;          /* actually char **s; but small-C cant cope */
        {
        char *s;
        s = *ps;
        if (*s != ESCAPE)
                return *s;
        else if (s[1] == NULL)
                return ESCAPE;
        else
                {
                *ps = ++s;      /* update internal and external pointers */
                if (*s == 'n')
                        return '\n';
                else if (*s == 't')
                        return '\t';
                else if (*s == 'f')
                        return '\f';
                else if (*s == 'b')
                        return '\b';
                else if (*s == 'l')
                        return '\l';
                else if (*s == 'r')
                        return '\r';
                else if (*s == 's')
                        return ' ';
                else
                        return *s;
                }
        }


/* expand dashed ranges */
dodash(valid,ps,set,pj,maxset)
        char *valid;
        int *ps;        /* actually char **ps */
        char *set;
        int *pj,maxset;
        {
        char *k, *limit;

        --*pj;
        limit = index(valid, esc(ps));
        for(k = index(valid, set[*pj]); k <= limit; ++k)
                addset(*k,set,pj,maxset);
        }

/* reversed index */
xindex(s,c,allbut,lastto)
        char *s;
        int c, allbut, lastto;
        {
        char *p;
        if (c == EOF)
                return -1;
        else if (allbut == NO)
                {
                p = index(s,c);
                if (p == 0)
                        return -1;
                else
                        return p - s;
                }
        else if (index(s,c) > 0)
                return -1;
        else
                return lastto + 1;
        }

/* end of tr.c */

