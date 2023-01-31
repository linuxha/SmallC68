/*
 *      contributed by A. T. Schreiner
 *                     Sektion Informatik
 *                     University of Ulm
 *                     West Germany
 *
 */

/*
 *      p -- Small-C preprocessor
 *      ats 6/83
 */

/*
 *      define...
 *
 *      verbose         to support -v for debugging
 */

char usage[] =
#ifdef verbose
       "p [-d n[=v]] [-e] [-i d:] [-u n] [-v] [in [out]]",
       vflag;          /* set by -v */
#else
       "p [-d n[=v]] [-e] [-i d:] [-u n] [in [out]]";
#endif

                       /* command names:               */
#define DEFINE  1      /* # define name text           */
                       /* # define name(name,...) text */
#define ELSE    2      /* # else                       */
#define ENDIF   3      /* # endif                      */
#define IFDEF   4      /* # ifdef name                 */
#define IFNDEF  5      /* # ifndef name                */
#define INCLUDE 6      /* # include "file"             */
                       /* # include <file>             */
#define LINE    7      /* # line number name           */
#define UNDEF   8      /* # undef name                 */
#define DEFAULT 0      /* any others passed on         */

/*
 *      "FEATURES":
 *
 *              Macro calls must be fully contained on one source
 *              line -- all lines can be continued with \, however.
 *
 *              Recursive definitions are not detected as such.
 *              p' will report as per NEST.
 *
 *              #else can be used (to reverse the current
 *              #if condition) arbitrarily often.
 */

#include <stdio.h>

/*
 *      i/o header file

#define FILE    ???     type to represent files (used as FILE*)
#define stdin   ???     pre-opened standard input file
#define stdout  ???     pre-opened standard output file
#define stderr  ???     pre-opened diagnostic output file
#define NULL    0       null pointer, false
#define EOF     ???     end of file indication

 */

#include <ctype.h>

/*
 *      character classification macros header file
 *
 *      isascii(i)      i is ASCII character
 *      isalnum(c)      c is letter or digit
 *      isalpha(c)      c is letter
 *      isdigit(c)      c is digit
 *      islower(c)      c is lower case letter
 *      isspace(c)      c is white space
 *      isupper(c)      c is upper case letter
 *      isxdigit(c)     c is base 16 digit
 *
 *      i can be any integer, isascii(c) must be true for c
 */

#define INCR    80      /* line buffer increment */
#define HASH    128     /* hash table size (power of 2) */
#define NEST    10      /* limit for reprocessing - -1 is "infinite" */

                        /* cmode states */
#define CMcmt   1       /* in comment */
#define CMstr   2       /* in string */
#define CMchr   3       /* in character constant */

#define PARM    0x80    /* flag macro parameter number */
#define PARMNO  0x7f    /* extract parameter number */
/*
 *      special data types
 */

#define LIST      int           /* list of word or string values */
#define l_next(x) (*(x))        /* -> next element */
#define l_word(x) ((x)[1])      /* word value */
#define l_str(x)  ((x)+1)       /* -> string value */
#define sz_WORD   4             /* size of word list element */
#define sz_STR(s) (3+strlen(s)) /* size of string list element */
#define SYMBOL    int           /* list of symbol table elements */
#define s_next(x) (*(x))        /* -> next element */
#define s_val(x)  ((x)[1])      /* -> defined value */
#define s_name(x) ((x)+2)       /* -> name */
#define sz_SYM(n) (5+strlen(n)) /* size of symbol table element */

/*
 *      runtime support routines
 */

extern _drive(),        /* BDOS function 25: current drive number */
       _narg(),         /* number of arguments passed in this call */
       calloc(),        /* (n,l) return NULL or -> n elements of length l */
       cfree(),         /* (p) free area at p, returned by calloc() */
       exit(),          /* terminate program execution */
       fclose(),        /* (f) close file described by f */
       fgetc(),         /* (f) return EOF or next character from file f */
       fopen(),         /* (n,m) return NULL or descriptor for file "n"
                           opened to read (m == "r"), write ("w"),
                           or append ("a") */
       fputc(),         /* (c,f) write c on file f, return EOF or c */
       fputs(),         /* (s,f) write string s on file f */
       freopen(),       /* (n,m,f) like fopen(), but close and reuse f */
       index(),         /* (s,c) find c in string s, return NULL or -> to it.
                           '\0' is always found */
       itod(),          /* (i) return -> (static) string with i in decimal */
       strcmp(),        /* (a,b) <, ==, > 0 as string a is <, ==, > string b */
       strcpy(),        /* (a,b) copy string b to string a */
       strlen(),        /* (s) return number of characters in string s */
       strncmp(),       /* (a,b,n) like strcmp(), but for n bytes at most */
       strncpy();       /* (a,b,n) like strcpy(), but for n bytes at most */
 /*
 *     global data
 */

int    parmno,          /* current number of parameters */
       linelen,         /* current maximum usable length */
       olinelen,
       lineno,          /* current line number */
       olineno,
       iflevel,         /* depth of open #if */
       skip;            /* non-0: iflevel to skip to */

char   eflag,           /* set by -e: prevent position stamps */
       cmode,           /* comment() mode */
       *line,           /* dynamic input line buffer */
       *lp,             /* current position in line */
       *oline,          /* dynamic output line buffer */
       *olp;            /* current position in oline */

LIST   *drive,          /* include prefixes */
       *filenms,        /* open file names */
       *files,          /* open file pointers */
       *lines,          /* line numbers */
       *parms;          /* parameters */

SYMBOL *symbol;         /* list of symbol table elements */
#ifdef HASH             /* hash feature (optional) */
                        /* symbol set by find() to -> hashtab at s */
int    hashtab[HASH];   /* really SYMBOL *: begin of chains */
#endif

FILE   *infile;         /* current input file */

main(argc, argv)
        int argc;
        int *argv;
{       char *cp, *vp;
#ifdef verbose
        LIST *ip;
#endif

        /* current drive is last include prefix */
        vp = "a:";
        *vp += _drive();
        drive = pushs(drive, vp);

        /* predefine "cpm" */
        define("cpm", "");

        /* process arguments, values may be joined or separate */
        while (--argc)
        {       cp = *++argv;
                if (*cp != '-')
                        break;
                switch (cp[1]) {
                case 'd':
                        if (*(cp += 2))
                                ;
                        else if (--argc == 0)
                                goto error;
                        else
                                cp = *++argv;
                        if (vp = index(cp, '='))
                                *vp++ = '\0';
                        else
                                vp = "";
                        define(cp, vp);
                        break;
                case 'e':
                        eflag = 1;
                        break;
                case 'i':
                        /* explicit prefixes in order right to left */
                        if (*(cp += 2))
                                ;
                        else if (--argc == 0)
                                goto error;
                        else
                                cp = *++argv;
                        drive = pushs(drive, cp);
                        break;
                case 'u':
                        if (*(cp += 2))
                                ;
                        else if (--argc == 0)
                                goto error;
                        else
                                cp = *++argv;
                        undefine(cp);
                        break;
#ifdef verbose
                case 'v':
                        vflag = 1;
                        break;
#endif
                default:
                        goto error;
                }
        }

        /* input file drive is first include prefix */
        vp = "a:";
        *vp += _drive();
        /* allow input and output files */
        switch (argc) {
        case 2:         /* use input, drive(input), output */
        case 1:         /* use input, drive(input) */
                if (cp[1] == ':')
                {       vp = "?:";
                        *vp = cp[0];
                }
                if (freopen(*argv, "r", stdin) == NULL)
                {       where("cannot read", *argv);
                        exit();
                }
                filenms = pushs(filenms, *argv);
#ifdef verbose
                if (vflag)
                        where("reading");
#endif
                if (--argc)
                {       if (freopen(*++argv, "w", stdout) == NULL)
                        {       filenms = NULL;
                                where("cannot write", *argv);
                                exit();
                        }
#ifdef verbose
                        if (vflag)
                                where("writing", *argv);
#endif
                }
        case 0:         /* use stdin, current drive */
                break;
        default:
error:          where(usage);
                exit();
        }

        /* set first include prefix */
        drive = pushs(drive, vp);

#ifdef verbose
        if (vflag)
        {       for (ip = drive; ip; ip = l_next(ip))
                        where("drive", l_str(ip));
        }
#endif

        /* start reading on stdin */
        infile = stdin;

        /* allocate first buffers */
        if ((line = calloc(INCR, 1)) == NULL
            || (oline = calloc(INCR, 1)) == NULL)
        {       where("no room");
                exit();
        }
        olinelen = linelen = INCR;

        /* make sure, we first get a position stamp */
        olineno = lineno - 3;

        /* main loop */
        while (getline())
                if (! comment() && ! command())
                        process();
}

getline()               /* line = complete line, ascii */
                        /* return false on EOF */
{       int c;          /* current character */

        /* move to lp, concatenating continued lines */
        for (lp = line; ; )
        {       switch (c = fgetc(infile)) {
                case '\\':
                        switch (c = fgetc(infile)) {
                        case EOF:
                                where("trailing \\");
                        case '\n':
                                ++lineno;
                                continue;
                        }
                        in('\\');
                default:
                        if (! isascii(c))
                                where("illegal character");
                        else
                                in(c);
                        continue;
                case EOF:
                        ++ lineno;
                        if (lp != line)
                                break;
                        else if (files)
                        {       fclose(infile);
#ifdef verbose
                                if (vflag)
                                        where("end include");
#endif
                                infile = pop(&files);
                                lineno = pop(&lines);
                                olineno = lineno - 3; /* stamp! */
                                pop(&filenms);
                                continue;
                        }
                        return 0;
                case '\n':
                        ++lineno;
                        if (lp == line)
                                continue;
                }
                break;  /* got a nonempty line */
        }
        *lp = '\0';
#ifdef verbose
        if (vflag)
                where("getline", line);
#endif
        return 1;
}

comment()               /* line = line w/out comments, lead white space */
                        /* return true if comment line */
{       char c;

        /* move from olp to lp, eliminating comments */
        for (lp = olp = line; ; )
        {       switch (c = *olp++) {
                case '\\':
                        if (cmode != CMstr && cmode != CMchr)
                                break;
                        in(c);
                        if (c = *olp++)
                                break;
                case '\0':
                        if (cmode == CMstr)
                        {       if (! skip)
                                        where("unbalanced \"");
                                in('\"');
                                cmode = 0;
                        }
                        else if (cmode == CMchr)
                        {       if (! skip)
                                        where("unbalanced \'");
                                in('\");
                                cmode = 0;
                        }
                        *lp = '\0';
#ifdef verbose
                        if (vflag)
                                where("comment", line);
#endif
                        return lp == line;
                case '/':
                        if (cmode == 0 && *olp == '*')
                        {       cmode = CMcmt;
                                ++olp;
                                if (lp != line)
                                        in(' ');
                                continue;
                        }
                        break;
                case '*':
                        if (cmode == CMcmt && *olp == '/')
                        {       cmode = O;
                                ++olp;
                                continue;
                        }
                        break;
                case '\"':
                        switch (cmode) {
                        case 0:
                                cmode = CMstr;
                                break;
                        case CMstr:
                                cmode = 0;
                        }
                        break;
                case '\'':
                        switch (cmode) {
                        case 0:
                                cmode = CMchr;
                                break;
                        case CMchr:
                                cmode = O;
                        }
                }
                if (cmode != CMcmt && (! isspace(c) || lp != line))
                        in(c);
        }
}

command()               /* process commands */
                        /* return true if done (i.e., to skip) */
{       int k;
        LIST *dp;
        FILE *fp;
        char *cp;
/*
 *      #if algorithm
 *
 *      skip    if non-zero, knows #if-level to which to skip;
 *              while skipping, comment() is executed, but not process().
 *
 *      iflevel current nesting depth of #if;
 *              counted even while skipping (of course).
 *
 *      #else   if skipping to current #if-level, stop skipping;
 *              if not skipping, start skipping to current level.
 *
 *      In order to limit #else to at most one per #if, we
 *      would need a stack; why bother??
 */

        if (*line != '#' || (k = kind(&lp)) == DEFAULT)
                return skip;

        /* process the command */
        switch (k) {
        case DEFINE:
                if (! skip)
                        define(lp, marknm(lp));
                break;
        case ELSE:
                if (! skip && iflevel == O)
                        where("#else without #if");
                else if (skip == iflevel)
                        skip = 0;
                else if (skip == 0)
                        skip = iflevel;
                break;
        case ENDIF:
                if (! skip && iflevel == 0)
                        where("#endif without #if");
                else
                {       if (skip == iflevel)
                                skip = O;
                        --iflevel;
                }
                break;
        case IFDEF:
        case IFNDEF:
                ++iflevel;
                if (! skip)
                {       marknm(lp);
                        if (isname(lp,""))
                                if (find(lp))
                        {       if (k == IFNDEF)
                                        skip = iflevel;
                        }
                        else
                        {       if (k == IFDEF)
                                        skip = iflevel;
                        }
                }
                break;
        case INCLUDE:
                if (! skip)
                        if (! markfl(lp))
                                where("#include?");
                        else if (lp[2] == ':')
                        {       ++lp;
                                if (fp = fopen(lp, "r"))
                                {
pushfile:                               files = pushw(files, infile);
                                        lines = pushw(lines, lineno);
                                        lineno = O;
                                        olineno = lineno - 3; /* stamp ! */
                                        filenms = pushs(filenms, lp);
                                        infile = fp;
#ifdef verbose
                                        if (vflag)
                                                where("including", lp);
#endif
                                }
                                else
                                        where("cannot open include file", lp);
                        }
                        else
                        {       dp = drive;
                                if (*lp == '<')
                                        dp = l_next(dp);
                                *lp-- = ':';
                                for (; dp; dp = l_next(dp))
                                {       *lp = *l_str(dp);
                                        if (fp = fopen(lp, "r"))
                                                goto pushfile;
                                }
                                where("cannot find include file", lp+2);
                        }
                break;
        case LINE:
                if (! skip)
                {       if (isdigit(*lp))
                        {       for (k = *lp - '0'; isdigit(*++lp); )
                                        k = k*10 + *lp-'O';
                                while (isspace(*lp))
                                        ++lp;
                                cp = lp;
                                while (*lp && ! isspace(*lp))
                                        ++lp;
                                if (lp != cp)
                                {       *lp = '\0';
                                        lineno = k;
                                        if (filenms)
                                                pop(&filenms);
                                        filenms = pushs(filenms, cp);
                                        break;
                                }
                        }
                        where("#line?");
                }
                break;
        case UNDEF:
                if (! skip)
                {       marknm(lp);
                        undefine(lp);
                }
        }
        return 1;
}

process()               /* process regular input line */
{       char expand;    /* reprocess flag */
        char c;         /* current input character */
        SYMBOL *sp;     /* -> found symbol */
        char *name;     /* -> begin of name */
        int i;
        int nest;

        /* expand one buffer into the other */
        nest = NEST+1;
        do
        {       lp = line;
                *(olp = oline) = '\0';
                expand = O;
                while(c = *lp++)
                        if (isalpha(c) || c == '_')
                        {       name = out(c);
                                while ((c = *lp) && (isalnum(c) || c == '_'))
                                {       out(c);
                                        ++lp;
                                }
                                if (sp = find(name))
                                {       expand = 1;
                                        outmacro(name, sp);
                                }
                        }
                        else if (isdigit(c))
                                lp = outnumber(--lp);
                        else if (c == '\'' || c == 34)
                                lp = outdelim(--lp);
                        else
                                out(c);
                /* if something changed, flip buffers */
                if (expand)
                {       lp = line;
                        line = oline;
                        oline = lp;
                        i = linelen;
                        linelen = olinelen;
                        olinelen = i;
                }
        } while (expand && --nest);
        if (expand)
                where("#define nested too deep");
        output(oline);
}

/*
 *      symbol table routines
 */

define(s, v)            /* #define s v */
        char *s;        /* name of symbol ?? */
        char *v;        /* value */
{       SYMBOL *r;
        int f;
        char *cp, *p, c, *name;

        if (! ismacro(s))
                return;

        /* prune and parametrize value*/
        while (isspace(*v))
                ++v;
        if (*v)
        {       for (cp = v + strlen(v); cp != v; )
                        if (! isspace(*--cp))
                                break;
                        else
                                *cp = '\0';
                /* if we have parameters, replace names by positions */
                if (parmno)
                {       p = cp = v;
                        while (c = *cp++)
                                if (isalpha(c) || c == '_')
                                {       *(name = p++) = c;
                                        while ((c = *cp)
                                                && (isalnum(c) || c == '_'))
                                        {       *p++ = c;
                                                ++cp;
                                        }
                                        if (f = findparm(name, p-name))
                                        {       *(p = name): f | PARM;
                                                ++p;
                                        }
                                }
                                else
                                {       *p++ = c;
                                        /* name as trailer of a constant?? */
                                        if (c == '0'
                                                 && (*cp == 'x' || *cp == 'X'))
                                                do
                                                        *p++ = *cp++;
                                                while (isxdigit(*cp));
                                        else if (isdigit(c))
                                                while (isdigit(*cp))
                                                        *p++ = *cp++;
                                        else if (c == '\\')
                                                if (*cp)
                                                        *p++ = *cp++;
                                }
                        *p = '\0';
                }
        }

        /* check if (different) redefinition */
        if (r = find(s))
        {       if (strcmp(s_val(r), v) != O)
                        where("redefining", s);
                undefine(s); /* parmno may change */
#ifdef verbose
                if (vflag)
                        fputs("redefine ", stderr);
#endif
        }

        /* if parametrized, save count */
        if (parmno)
        {       cp = s + strlen(s);
                *cp = '(';
                *++cp = parmno;
                *++cp = '\0';
        }
        /* ready to make new entry */
        if ((r = calloc(sz_SYM(s), 1)) == NULL)
        {       where("no room");
                exit();
        }
        else
        {
#ifdef HASH     /* find() sets symbol -> hashtab at s */
                s_next(r) = *symbol;
                *symbol = r;
#else
                s_next(r) = symbol;
                symbol = r;
#endif
                s_val(r) = NULL;
                strcpy(s_name(r), s);
#ifdef verbose
                if (vflag)
                        fputs("define ", stderr);
#endif
        }
        /* save new value */
        if ((s_val(r) = calloc(strlen(v)+1, 1)) == NULL)
        {       where("no room");
                exit();
        }
        strcpy(s_val(r), v);
#ifdef verbose
        if (vflag)
        {       fputs(s, stderr);
                fputc(' ', stderr);
                fputs(s_val(r), stderr);
                fputc('\n', stderr);
        }
#endif
}

undefine(s)             /* #undef s */
        char *s;        /* name of symbol ?? */
{       SYMBOL *r, *p;

        if (isname(s, "") && ( r = find(s)))
        {       cfree(s_val(r));
                /* need to unlink symbol descriptor from chain */
#ifdef HASH     /* find() sets symbol -> hashtab at s */
                if (r == *symbol)
                        *symbol = s_next(*symbol);
                else
                {       for (p = *symbol;
#else
                if (r == symbol)
                        symbol = s_next(symbol);
                else
                {       for (p = symbol;
#endif
                                        s_next(p) != r; p = s_next(p))
                                ;
                        s_next(p) = s_next(r);
                }
                cfree(r);
#ifdef verbose
                if (vflag)
                        where("undefine", s);
#endif
        }
}

  find(s)                 /* locate s in symbol table */
                          /* return NULL or -> entry */
          char *s;        /* name to find */
  {       SYMBOL *r;
          char *sp, *rp, c;
  #ifdef  HASH
          int h;

          /* symbol table chains start in hashtab[] */
          /* compute hash address as sum of letters */
          for (h = O, sp = s; c = *sp; ++sp)
                  h += c;
          symbol = hashtab + (h & (HASH-1));

          /* run down the chain */
          for (r = *symbol;
  #else
          /* symbol table chain is one linear list */
          /* run down the chain */
          for (r = symbol;
  #endif
                          r; r = s_next(r))
          {       for (sp = s, rp = s_name(r); (c = *sp) && *rp == c; ++sp, ++rp)
                          ;
                  if (c == '\0' && (*rp == '\0' || *rp == '('))
                          return r;
          }
          return NULL;
  }

  findparm(s, l)          /* return 0 or parameter number */
          char *s;        /* -> begin of possible parameter name */
          int l;          /* length of name */
  {       int f;
          LIST *p;

          for (f = 0, p = parms; p; ++f, p = l_next(p))
                  if (strncmp(l_str(p), s, l) == 0)
                          return parmno - f;
          return 0;
  }

  isname(s,d)             /* true, if s is a name */
                          /* return -> delimeter or NULL */
          char *s;        /* -> begin of name */
          char *d;        /* chars in which name may also end */
  {       char *cp, c;

          for (cp = s; index(d, c = *cp) == NULL; ++cp)
                  if (! isalnum(c) && c != '_')
                          goto error;
          if (cp == s || isdigit(*s))
          {

  error:          where("illegal name", s);
                  return NULL;
          }

          return cp;      /* return -> delimeter */
  }

  ismacro(s)              /* true, if s is a macro header */
          char *s;        /* -> begin of name or header */
  {       char *cp, c;
          while (parms)   /* free old parameter list */
                  pop(&parms);
          parmno = 0;
          if ((s = isname(s, "(")) == NULL)
                  return 0;
          if (*s)         /* we have a new macro */
          {       *s = '\0';              /* delimit name */
                  do                      /* and parse parameters */
                  {       while (isspace(*++s))

                          if (cp = isname(s, ",) \t"))
                          {       c = *cp;
                                  *cp = '\0';
                                  parms = pushs(parms, s);
                                  ++ parmno;
                          }
                          else
                                  return 0;
                          while (isspace(c))
                                  c = *++cp;
                          s = cp;
                 } while (c == ',');
                 if (c != ')')
                 {        where("illegal macro header");
                          return 0;
                  }
          }
          return 1;
  }

  marknm(s)               /* bypass and terminate macro header */
                          /* return -> value */
          char *s;        /* ->begin of name */
  {       char c;

          /* find white space or ( */
          while ((c = *s) && ! isspace(c) && c != '(')
                  ++s;

          /* if (, there must be names, white space and then ) */
          if (c == '(')
          {       while ((c = *++s) && c != ')')
                          ;
                  /* after ) there must be \0 or white space */
                  if (c && (s[1] == '\0' || isspace(s[1])))
                          ++s;
          }

          /* terminate in place of white space */
          if (*s)
                  *s++ = '\0';

          /* this is a rough draft -- see ismacro/isname */
          return s;
  }

  /*
   *      input and output routines
   */

  in(c)                   /* store incoming character */
          char c;         /* to be stored at lp */
  {
          *lp++ = c;
          if (lp >= line+linelen)
                  rebuff(&lp, &line, &linelen);
  }

  out(c)                  /* store a character, return -> stored char */
          char c;         /* to be stored at olp */
  {
          *olp++ = c;
          if (olp >= oline+olinelen)
                  rebuff(&olp, &oline, &olinelen);
          *op = '\0';     /* maintain trailer */
          return olp-1;
  }

  rebuff(p, buf, len)     /* make buffer longer */
          int *p;         /* & current pointer */
          int *buf;       /* & buffer pointer */
          int *len;       /* & maximum length */
  {
          if ((*p = calloc(*len + INCR, 1)) == NULL)
          {       where("no room");
                  exit( );
          }
          strncpy(*p, *buf, *len);
          cfree(*buf);
          *buf = *p;
          *p = *buf + *len;
          *len += INCR;
  }

  output(s)               /* write a string */
          char *s;        /* to write as a line */
  {
          /* synchronize output linecount */
          if (! eflag && ++olineno != lineno)
          {       if (++olineno != lineno)
                  {       fputc('#', stdout);
                          fputs(itod(olineno = lineno), stdout);
                          if (filenms)
                          {       fputc(' ', stdout);
                                  fputs(l_str(filenms), stdout);
                          }
                  }
                  fputc('\n', stdout);
          }
          /* emit string as a line */
          fputs(s, stdout);
          if (fputc('\n', stdout) == EOF)
          {       where("output file full");
                  exit();
          }
  }

  /*
   *      C constant processing:
   *
   *      digits          decimal
   *      0digits         octal
   *      0xdigits        hexadecimal
   *      'c'             character value (escapes ok)
   */

  outnumber(cp)           /* store a C constant in decimal */
                          /* return -> past it */
          char *cp;       /* -> constant text (digit) */
  {       char c, *p;
          int base;
          int i;

          base = 10;
          i = 0;
          if ((c = *cp) == '0')
          {       base = 010;
                  if ((c = *++cp) == 'x' || c == 'X')
                  {       base = 0x10;
                          c = *++cp;
                  }
          }
          for ( ; c; c = *++cp)
          {       if (isdigit(c))
                          c -= '0';
                  else if (isxdigit(c))
                          if (isupper(c))
                                  c -= 'A' - 10;
                          else
                                  c -= 'a' - 10;
                  else
                          break;
                  if (c < base)
                          i = i*base + c;
                  else
                          break;
          }
          for (p = itod(i); c = *p; ++p)
                  out(c);
          return cp;
  }

  outdelim(cp)            /* store a delimited string, return -> past trailer /*
          char *cp;       /* -> delimeter */
  {       char c, *p;

          if ((c = *cp) == '\"')
          {       out(c);
                  while (c = *++cp)
                  {       out(c);
                          if (c == '\"')
                                  return cp+l;
                          if (c == '\\')
                                  if (c = *++cp)
                                          out(c);
                                  else
                                          break;
                  }
          }
          else    /* it must be character constant */
                  switch (c = *++cp) {
                  case 0:
                  case '\'':
                          goto error;
                  case '\\':
                          switch (c = *++cp) {
                          case 'b':  c = '\b'; break;
                          case 'f':  c = '\f'; break;
                          case 'n':  c = '\n'; break;
                          case 'r':  c = '\r'; break;
                          case 't':  c = '\t';
                          case '\'':
                          case '\\':
                          case '\"':           break;
                          default:
                                  if (! isdigit(c) || (c -= '0') > 7)
                                          goto error;
                                  if (isdigit(cp[1]) && cp[1] <= '7')
                                  {       c = (c << 3) + *++cp - '0';
                                          if (isdigit(cp[1]) && cp[1] <= '7')
                                                  c = (c << 3) + *++cp - '0';
                                  }
                          }
                  default:
                          if (*++cp != '\'')
                          {
  error:                          where("illegal character constant");
                                  while (*cp && *cp != '\'')
                                          ++cp;
                                  if (*cp)
                                          ++cp;
                                  break;
                          }
                          for (p = itod(c); *p; ++p)
                                  out(*p);
                          out(' ');
                          ++cp;
                  }
          return cp;
  }

  /*
   *      macro processing
   */

  outmacro(at, s)         /* replace string by macro value */
          char *at;       /* replace from here on */
          SYMBOL *s;      /* using this definition */
  {       char *vp, c;

          /* set output up for replacement */
          olp = at;

          /* force white space around replacement */
          if (olp > oline && (! isspace(olp[-1])))
                  out(' ');

          /* if parametrized, collect arguments */
          if (vp = index(s._name(s), '('))
                  markarg(*++vp);
          else
                  parmno = 0;

          /* emit replacement */
          for (vp = s_val(s); c = *vp++; )
                  if (c & PARM)
                          outarg(c & PARMNO);
                  else
                          out(c);

          /* white space */
          out(' ');
  }

  markarg(n)              /* mark and collect arguments */
          int n;          /* number to find */
  {       char c, cmode;  /* cmode during argument collection only */
          int lpar;

          /* release parameter/argument list, if any */
          while (parms)
                  pop(&parms);
          parmno = 0;

          /* find ( */
          while (isspace(c = *lp))
                  ++lp;

          /* collect */
          if (c == '(')
          {       do
                  {       parms = pushw(parms, ++lp);
                          ++ parmno;
                          lpar = cmode = 0;
                          for ( ; c = *lp; ++lp)
                          {       switch (c) {
                                  case '(':
                                          if (cmode == 0)
                                                  ++ lpar;
                                          continue;
                                  case ',':
                                          if (cmode || lpar)
                                                  continue;
                                          break;
                                  case ')':
                                          if (cmode || lpar--)
                                                  continue;
                                  case '\'':
                                          switch (cmode) {
                                          case 0:
                                                  cmode = CMchr;
                                          case CMstr:
                                                  continue;
                                          }
                                          cmode = 0;
                                          continue;
                                  case '\"':
                                          switch (cmode) {
                                          case 0:
                                                  cmode = CMstr;
                                          case CMchr:
                                                  continue;
                                          }
                                          cmode = 0;
                                                  continue;
                                  case '\\':
                                          if (*++lp == '\0')
                                                  break;
                                  default:
                                          continue;
                                  }
                                  *lp = '\0';
                                  break;
                          }
                  } while (c == ',');
                  if (c == ')')
                          ++lp;
                  else
                          where("incomplete macro call");
          }

          /* check and fill argument count */
          if (parmno) != n)
                  where("wrong number of arguments");
          for ( ; parmno < n; ++parmno)
                  parms = pushw(parms,"");
  }

  outarg(i)               /* emit argument */
          int i;          /* number to emit */
  {       LIST *p;
          char *cp, c;

          /* play double safe */
          if (i > parmno)
          {       where("outarg(>>)??");
                  exit();
          }

          /* locate */
          for (i = parmno-i, p = parms; i && p; --i, p = l_next(p))
                  ;
          if (p == NULL)
          {       where("outarg(NULL)??");
                  exit();
          }

          /* emit, no white space */
          for (cp = l_word(p); c = *cp; ++cp)
                  out(c);
  }

  /*
   *      stack routines
   */

  pushw(l, w)             /* push word, return -> new list */
          LIST *l;        /* list */
          int w;          /* word to push */
  {       LIST *r;

          if ((r = calloc(s_WORD, 1)) == NULL)
          {       where("no room");
                  exit();
          }
          l_next(r) = l;
          l_word(r) = w;
          return r;
  }

  pushs(l, s)             /* push string, return -> new list */
          LIST *l;        /* list */
          char *s;        /* string to push */
  {       LIST *r;

          if ((r = calloc(sz_STR(s), 1)) == NULL)
          {       where("no room");
                  exit();
          }
          l_next(r) = l;
          strcpy(l_str(r), s);
          return r;
  }

  pop(l)                  /* pop list, return word */
          LIST *l;        /* really**: list header */
  {       LIST *r;
          int i;

          if (*l == NULL)
          {       where("pop(NULL)??");
                  exit();
          }
          r = *l;         /* element to pop */
          i = l_word(r);  /* result */
          r = l_next(r);  /* following element */
          cfree(*l);
          *l = r;
          return i;       /* nonsense for a string list */
  }

  /*
   * other utilities
   */

  where(vararg)           /* error message writer */
          int vararg;     /* arbitrarily many strings */
  {       int narg, *argv;

          narg = _narg();
          argv = &vararg;
          argv += narg;
          if (filenms)
          {       fputs(l_str(filenms), stderr);
                  if (lineno)
                          fputs(", ", stderr);
                  else
                          fputs(": ", stderr);
          }
          if (lineno)
          {       fputs("line ", stderr);
                  fputs(itod(lineno), stderr);
                  fputs(": ", stderr);
          }
          while (narg)
          {       fputs(*--argv, stderr);
                  if (--narg)
                          fputc(' ', stderr);
          }
          fputc('\n', stderr);
  }

  kind(plp)               /* determine command */
                          /* move line pointer past it and white space */
          int *plp;       /* char**, -> line pointer; NULLed or advanced */
  {       char *s;

          for (s = line+1; isspace(*s); ++s)
                  ;
          if (*plp = cmd(s, "define"))    return DEFINE;
          if (*plp = cmd(s, "else"))      return ELSE;
          if (*plp = cmd(s, "endif"))     return ENDIF;
          if (*plp = cmd(s, "ifdef"))     return IFDEF;
          if (*plp = cmd(s, "ifndef"))    return IFNDEF;
          if (*plp = cmd(s, "include"))   return INCLUDE;
          if (*plp = cmd(s, "line"))      return LINE;
          if (*plp = cmd(s, "undef"))     return UNDEF;
          return DEFAULT; /* *plp is NULL */
  }

  cmd(l, c)               /* parse keyword */
                          /* return NULL or -> past it and white space */
          char *l;        /* -> begin of possible keyword */
          char *c;        /* -> keyword */
  {
          /* compare *
          while (*l++ == *c++ && *c)
                  ;
          if (*c)
                  return 0;       /* incomplete keyword */
          if (*l == '\0')
                  return l;       /* just keyword */
          if (! isspace(*l))
                  return 0;       /* keyword plus trash */
          while (isspace(*++l))
                  ;
          return l;               /* bypassed white space */
  }

  markfl(sp)              /* bypass and terminate file name */
                          /* return true if found */
          char *sp;       /* -> begin delimeter, " or < */
  {       char s, *cp, c;

          if ((s = *(cp = sp)) && (s == '\"' || s == ' <'))
                  while (c = *++cp)
                          if (c == '\"' && s == '\"'
                                  || c == && s == '<')
                          {               *cp = '\0';
                                          return cp-sp > 1;
                          }
          return 0;
  }


LISTING 8-2

  /****
   ****   UN*X compatible dynamic memory allocation
   ****/
  /*
   *      calloc return pointer to vector of 0, or NULL
   *      cfree free previously allocated area
   *
   *      The heap starts at _end and runs upward toward the stack.
   *      Each area in the heap is preceded by a word at an even address:
   *      a pointer chain runs from _end through these words to NULL:
   *      The low bit in each word is 1 if the following area is free.
   *      There is a blind, allocated element at the front of the chain.
   *
   *      BUG:    very unreasonable demands (e.g., wraparound)
   *              will corrupt memory.
   */

  #define SLACK   1024    /* at least 1KB stack to be free */
  #define NULL    0

  word(wp)
          int *wp;
  {
          return *wp;
  }

  char * calloc(n, len)
          int n;          /* number of elements */
          int len;        /* length of element */
  {       int cell;       /* current allocation chain cell */
          char *p;        /* -> cell */
          char *np;       /* pointer in cell */
          int *ip, *wp;   /* for casting */

          len = (len*n + 1) & ~1;     /* even */
          if (len == 0)
                  return NULL;
          for (ip = p = word(_end+1l & ~1) & ~1;
               np = (cell = *ip) & ~1;
               ip = p = np)
                  if (cell & 1)           /* lowbit == 1 means free */
                  {       if ((n = np-p - 2) > len+2)
                          {       wp = p + len+2;
                                  *wp = cell;
                                  *ip = wp;
                                  }
                                  else if (n >= len)
                                          *ip = np;
                                  else
                                          continue;
                                  for (wp = p+2; len; len -= 2)
                                          *wp++ = 0;
                                  return p+2;
                          }
                  if {(wp = p + len+2) > &n - SLACK)
                          return NULL;
                  *ip = wp;
                  *wp = NULL;
                  for (wp = p+2; len; len -= 2)
                          *wp++ = 0;
                  return p+2;
  }
  cfree(fp)
                  int *fp;        /* to be freed */
  {               int *p, *np;

                  --fp;                           /* to cell */
                  for (p = _end+1 & ~1;
                       np = word(p) & ~1;
                       p = np)                    /* p-> previous cell */
                          if (np == fp)           /* fp-> cell to free */
                          {       np = *fp;       /* np-> following cell */
                                  if ((*fp & 1) II np == NULL)
                                          break;  /* he does not own it */
                                  if (*p & 1)
                                          if (*np & 1)
                                                  *p = *np;
                                          else if (*np == NULL)
                                                  *p = NULL;
                                          else
                                          {       *p = np;
                                                  *p I= 1;
                                          }
                                  else if (*np & 1)
                                          *fp =*np;
                                  else if (*np == NULL)
                                          *fp = NULL;
                                  else
                                          *fp I= 1;
                                  return;
                          }
                  fputs("cfree botch", stderr);
                  exit();
  }
