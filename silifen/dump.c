#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "cc.h"

/* print out compiler statistics */
dumpstats() {
    if (dump) {
        pl("compilation errors ");
        outdec(errcnt);
        pl("global symbols     ");
        outdec((glbptr-STARTGLB)/SYMSIZ);
        pl("literal characters ");
        outdec(litptr);
        pl("define characters  ");
        outdec(macptr);
        nl();
    }
}
