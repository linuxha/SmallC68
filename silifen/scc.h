#ifndef SCC_H
#define SCC_H
/************************************************/
/*                                              */
/*              small-c compiler                */
/*                                              */
/*                by Ron Cain                   */
/*                                              */
/************************************************/

#define BANNER  "** Ron Cain's Small-C V1.1.0 **"

#define VERSION " FLEX Version 2.1, 13 Aug 1982\n* Linux Version 0.0.1, Jan31 2023\n* Motorola 68xx"

#define AUTHOR "       By S. Stepanoff & Neil Cherry"

/*      Define system dependent parameters      */

extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stderr;

/*      Stand-alone definitions                 */

#ifndef NULL
#define NULL	0
#endif
#ifdef __linux
#define EOL 	'\n'            // \n = ^J 0x0A
#else
#define EOL 	13              // \r = ^M 0x0D
#endif

#define TAB	0x09
#define CLS	0x0C            // ^L
#define SQUOTE	0x27
#define DQUOTE	0x22

/*      UNIX definitions (if not stand-alone)   */

#ifndef FDEBUG
#warning FDEBUG not defined
#define fdebugf(b,c) { }
#else
#warning FDEBUG IS defined
#define fdebugf(b,c) fprintf(stderr, b, c)
#endif

#endif  /* SCC_H */

