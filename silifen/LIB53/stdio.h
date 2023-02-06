/* standard i/o header file for small-C with FLEX-9 */
/* Silicon Fen Software August 1984 */
/* get FILE i/o routines for assembler */
#asm
LIBS SET 1
#endasm
#define stdin        1
#define stdout       2
#define stderr       3
#define putc         fputc
#define getc         fgetc
#define YES          1
#define NO           0
#define NULL         0
#define BUFSIZE      256
#define EOF          -1
/* EOL = cr for FLEX lf for unix */
#define EOL          13
#define BYTMASK      255
#define FILE         char
#define extern
