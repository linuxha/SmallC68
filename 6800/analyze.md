# SmallC68 (`6800/`) Code Analysis

Scope: the native-code compiler driver (`smallc.c`, `heir.c`, `io.c`, `ask.c`,
`options.c`, `smallc.h`) that is built by the local `Makefile` into the
`smallc` binary.

> **Update:** items 1–7 below (plus one more found while verifying the
> fixes, §8) have since been fixed in `smallc.c`, `smallc.h`, `ask.c`, and
> `options.c`. The analysis is kept as-written for the record; each
> section now carries a **Fix applied** note. Key design point that
> motivated several of these fixes: this compiler runs as a *cross*
> compiler — its own internal data structures (the symbol table, the
> while-queue) live in the *host's* 64-bit address space even though the
> 6800 *target* it emits code for only has 16-bit pointers. Those two
> pointer widths must not be conflated, which is exactly what several of
> the bugs below were doing.

Build environment at time of review: `gcc (Debian 14.2.0-19) 14.2.0` on
Linux, using the flags in `Makefile` (`-g -O0 -D FDEBUG -D ASL`).

## 1. Blocking: the project does not currently build

`make` fails compiling `smallc.c` (the other translation units — `io.c`,
`heir.c`, `ask.c`, `options.c` — compile cleanly):

```
smallc.c:151:12: error: assignment to 'long int' from 'char *' ...
smallc.c:590:11: error: assignment to 'long int' from 'char *' ...
smallc.c:613:11: error: assignment to 'long int' from 'char *' ...
smallc.c:961:20: error: returning 'char *' from a function with return type 'long int' ...
smallc.c:1002:13: error: assignment to 'char *' from 'long int' ...
smallc.c:1079:22: error: returning 'char *' from a function with incompatible return type 'long int *' ...
```

GCC 14 made implicit pointer↔integer conversions (`-Wint-conversion`,
`-Wincompatible-pointer-types`) hard errors by default (previously just
warnings). This code relies on the historic K&R behavior where `char *`
and `int`/`long` were silently interchangeable, which is why it likely
compiled on older toolchains. Root cause and knock-on effects are detailed
in §2.

## 2. Root cause: `locidx` and the local-symbol-table helpers conflate `char *` and `long`

`smallc.c` keeps the local-symbol-table cursor in a *numeric* variable:

```c
long    locidx;                              // smallc.c:91
```

but every macro/usage treats it as a pointer into `symtab[]`:

```c
#define STARTLOC ENDGLB+SYMSIZ               // smallc.h — a char* expression
#define ENDLOC   symtab+SYMTBSZ-SYMSIZ        // char* expression
...
locidx = STARTLOC;                            // smallc.c:151, 590, 613 — char* -> long
if(locidx>=ENDLOC) { ... }                    // smallc.c:1006 — long vs char*
cptr = ptr = (char *) locidx;                 // smallc.c:1011 — long -> char* (needs the cast)
```

The global counterpart is written consistently as a pointer the whole way
(`char *glbptr`, `findglb()` returns `char *`), but its local-variable twin
was apparently "modernized" from `int` to `long` without being converted to
`char *`. The asymmetry shows up directly in the two lookup functions:

```c
char *
findglb(char *sname) { ... return (ptr); }     // smallc.c:939 — correct, char*

long
findloc(char *sname) { ... return (ptr); }      // smallc.c:953 — returns a char* from a long function
```

Most call sites paper over this with an explicit cast
(`ptr = (char *) findloc(sname);` in `primary()`, smallc.c:1530), but
`addloc()` does not:

```c
if(cptr = findloc(sname)){   // smallc.c:1002 — char* = long, no cast -> build error
```

**Fix applied:** `locidx` is now `char *` (matching `glbptr`), `findloc()`
now returns `char *` (matching `findglb()`), and the now-unneeded casts
at those call sites were removed. The two places that still need a
`long`↔`char *` conversion on purpose — `wql[WQSYM] = locidx` and
`locidx = wql[WQSYM]` in `dowhile()` (smallc.c), because the while-queue
entries are genuinely `long[]` — now carry an explicit `(long)`/`(char *)`
cast, since storing a *host* pointer in a `long` slot is correct there
(see §3) and should stay visible as an intentional, explicit conversion
rather than an implicit one.

## 3. Likely-critical runtime bug: the while-queue truncates `long` values to single bytes

Independent of whether it currently compiles, `addwhile()` / `readwhile()`
have a real data-corruption bug for any `while` loop that uses `break` or
`continue`.

The queue storage is a `char` array, and `addwhile()` writes one byte per
field:

```c
char    wq[WQTABSZ];                 // smallc.c:93 — storage is bytes
...
void
addwhile(long ptr[]) {               // smallc.c:1056 — takes an array of long (8-byte) values
    ...
    while (k<WQSIZ) {
        *wqidx++ = ptr[k++];         // smallc.c:1062 — wqidx is char*, so each store
    }                                 //   truncates ptr[k] to its low byte and advances 1 byte
}
```

`dowhile()` populates the four queue slots with real `long` values,
including a **pointer-sized** value for `WQSYM`:

```c
long wql[4];
wql[WQSYM]=locidx;      // smallc.c:764 — locidx is effectively a char* (symtab address)
wql[WQSP]=Zsp;
wql[WQLOOP]=getlabel();
wql[WQLAB]=getlabel();
addwhile(wql);
```

But the read-back path treats the same byte buffer as an array of `long`:

```c
long *
readwhile() {
    ...
    return (wqidx-WQSIZ);            // smallc.c:1079 — char* reinterpreted as long*
}

void
dobreak() {
    long *ptr;
    ptr = readwhile();
    modstk((ptr[WQSP]));             // smallc.c:803 — ptr[1] reads 8 bytes starting
    jump(ptr[WQLAB]);                // smallc.c:804 —   at offset 8 from ptr, but only
}                                     //   4 *bytes* were actually written per entry
```

Only 4 bytes were ever written per while-queue entry (`WQSIZ==4`, one byte
each), but `dobreak()`/`docont()` index into it as if it were an array of
4 `long`s (32 bytes on this platform). `ptr[WQSP]`, `ptr[WQLOOP]`,
`ptr[WQLAB]` all read far outside the 4 bytes that `addwhile()` actually
populated, picking up unrelated bytes from adjacent `wq[]` entries (or
beyond the 100-byte `wq[]` array for later/nested whiles). This will
produce wrong (or wildly out-of-range) jump labels and stack-adjustment
values for `break`/`continue` statements, i.e. miscompiled code, once the
build errors in §1/§2 are worked around.

This looks like the same "modernization" mistake as §2: the original
8-bit compiler's queue entries were genuinely single bytes (label numbers
and symbol-table offsets all fit in 0–255), so storing them one byte at a
time was correct. After widening labels/offsets/pointers to `long`, the
`wq[]` storage and the `addwhile`/`readwhile` byte-at-a-time access were
never widened to match.

**Fix applied:** `wq[]` is now `long wq[WQTABSZ]` and `wqidx` is now
`long *wqidx`, so `addwhile()`/`readwhile()`/`delwhile()` all operate in
full `long`-sized slots with no code changes needed in their bodies —
`*wqidx++ = ptr[k++]` now copies a whole `long`, `wqidx-WQSIZ` now steps
back one whole 4-`long` entry, etc. Verified with a nested `while`/`break`/`continue` test program after the
full set of fixes in this document: generated `break`/`continue` jump
targets (`cc2`, `cc3`, ...) and stack-cleanup deltas (`MODSP`) all come
out as small, sane values, consistent with correct codegen.

## 4. Evidence of a known, previously reproduced SIGSEGV in `getmem()`

`heir.c` (lines ~41–89) contains a verbatim, still-committed `gdb` session
transcript left in a comment, documenting an actual crash:

```
Program received signal SIGSEGV, Segmentation fault.
getmem (sym=0x5556bece <error: Cannot access memory at address 0x5556bece>) at smallc.c:1944
...
#1  rvalue (lval=lval@entry=0x7fffffffdc78) at smallc.c:1826
#2  heir11 (lval=0x7fffffffdc78) at heir.c:488
... (heir10 -> heir9 -> ... -> heir1 -> expression -> statement -> compound ->
     statement -> newfunc -> parse -> main)
```

`getmem(char *sym)` (now at smallc.c:1711) dereferences `sym[IDENT]` and
`sym[TYPE]` unconditionally; it is called from `rvalue()` only when
`lval[0] != 0` (smallc.c:1574-1576), so `lval[0]` held a non-zero but
invalid address. This is consistent with the pointer/`long` conflation
problems in §2/§3: a value that is supposed to be a valid `symtab[]`
pointer is getting corrupted (or a stale/partial value) somewhere along
the assignment chain from `primary()`/`findloc()`/`findglb()` into `lval[]`.
This review did not reproduce or root-cause the exact trigger, but the
crash is clearly a known, real failure mode that predates this review
(the transcript was deliberately kept as a comment, presumably as a
reminder), not a hypothetical one.

**Status:** not independently reproduced before or after the fixes in
this document, so it cannot be marked resolved with confidence. However,
§2's fix (making `locidx`/`findloc()` consistently `char *`) and §8's fix
(making `astreq()` stop reporting false symbol-table matches) remove two
concrete ways a `rvalue()`/`getmem()` call could end up with a garbage
`sym` pointer, which is the proximate cause visible in the backtrace. If
this crash resurfaces, re-check this area first.

## 5. `options.c`: CLI handling bugs

```c
while(1) {
    c = getopt_long (argc, argv, "?", long_options, &option_index);
    if(c == (-1)) {
        openFile(argv[optind]);      // options.c:56
        return;
    }
    switch(c) {
        default:
            fprintf(stderr,"Bad option '%c'\n",**argv);   // options.c:62
        case 'h':
        case '?':
            clihelp();
            exit(1);
    }
}
```

* **No input file given:** when `argv[optind]` is past the end of argv
  (no positional filename), `argv[optind]` is the C-guaranteed `NULL`
  sentinel. `openFile(NULL)` just does `if(!filename) return;`
  (options.c:78) and returns silently — `input` stays `0`. `main()`
  proceeds through `header()`, `parse()` (which exits its loop immediately
  because `myInline()` sees `input==0` and sets `eof=1`, smallc.c:1125-1130),
  `dumplits()`, `dumpglbs()`, `trailer()` and exits **successfully** having
  compiled nothing and without printing any "no input file" diagnostic.
  Running `smallc` with no arguments looks like a silent no-op success
  rather than a usage error.
* **Wrong diagnostic on bad option:** the `default:` branch prints
  `**argv`, i.e. the first character of `argv[0]` (the program's own
  path), not the actually-invalid option character. It should use `c`
  or `optopt`. In practice this branch is hard to hit anyway because it
  falls through into the `'h'`/`'?'` case and just shows help — but the
  printed message is always wrong when it is hit.

**Fix applied:** `options()` now checks `optind >= argc` and prints
"No input file given" plus usage before exiting(1), instead of silently
calling `openFile(NULL)` and continuing. The bad-option message now
prints `optopt` (the actual offending option character) instead of
`**argv`.

## 6. Dead / unreachable code from the FLEX→Linux port

`smallc.h` wraps essentially every function prototype (lines 155–574) in:

```c
#ifdef SKIP
   /* ~200 lines of "int"-typed prototypes */
#else
   /* ~200 lines of "long"-typed prototypes */
#endif
```

`SKIP` is never `#define`d anywhere in the project (verified by searching
`smallc.c`, `smallc.h`, the `Makefile`, and `COPT`). So the `#ifdef SKIP`
branch is permanently dead code that will never be compiled — it exists
only as unreachable, unmaintained duplicate text that a reader has to
rule out by hand each time. The same pattern repeats at the top of
`smallc.c` (lines 35–139) for the global-variable definitions.

Separately, in `main()` (smallc.c:182-190):

```c
#ifdef __linux
    options(argc, argv);
#else
    ask();
    openout();
    openin();
    header();
#endif
    header();               // called again unconditionally
```

Since gcc on Linux always predefines `__linux`, the `#else` branch
(and therefore `ask()`'s/`openin()`'s/`openout()`'s interactive
"FLEX version" logic in `ask.c`) is never exercised by this project's
own `Makefile`/toolchain — it is effectively dead/untested code. It also
contains a latent double-call-to-`header()` bug (would emit the assembler
header twice) that would only surface if that branch were ever built.
`ask.c`'s `openout()` also prints a `FILE *` with `%x`
(`printf("Opps %x\n", output);`, `printf("0x%x\n", output);`,
ask.c:95,98) — undefined behavior passing a pointer-sized argument to a
format expecting `int` — but again only in this dead code path.

**Fix applied:** removed the duplicate `header()` call in `main()` (only
the unconditional one after the `#ifdef __linux` block remains). Changed
`ask.c`'s pointer prints to `%p` with `(void *)` casts. Left the
non-`__linux` branch in `main()` (and `ask.c`'s interactive FLEX-era code
behind it) in place, since it's still plausibly intentional scaffolding
for a future non-Linux target — only the permanently-dead `#ifdef SKIP`
duplicate-prototype text (§7, which can never be reached by any build
configuration because `SKIP` is never defined anywhere) was deleted.

## 7. Minor cruft in `smallc.h`

* `extern struct symbolX;` (smallc.h:80) is a storage-class-only empty
  declaration (gcc: "useless storage class specifier in empty
  declaration"); `symbolX` doesn't appear to be used/defined anywhere
  else — looks like a leftover/typo from refactoring `struct symbol`.
* `extern int locptr;` / `extern long locptr;` (smallc.h:321/531) is
  declared but never defined or referenced anywhere in the `.c` files —
  the real variable is `locidx` (smallc.c:91). `locptr` appears to be
  dead, stale naming left behind by a rename.
* `#warning FDEBUG IS defined` (smallc.h:582) fires on every single
  translation unit build (visible 5x in the build log) because
  `-D FDEBUG` is unconditionally set in `COPT` in the `Makefile` — this
  is presumably intentional as a build reminder, but it drowns out any
  other warnings that appear in the same `make` output.

**Fix applied:** deleted `extern struct symbolX;` and the dead
`locptr` extern (replaced by a correct `extern char *locidx;`). Also
deleted the entire unreachable `#ifdef SKIP` duplicate-prototype block
(smallc.h) and its matching dead duplicate-globals block (smallc.c),
since `SKIP` is never defined anywhere and that text could never compile
under any configuration. Left the `#warning FDEBUG IS defined` as-is —
it's driven by a real `-D FDEBUG` build flag and silencing it wasn't
asked for.

## 8. Found during verification: `astreq()` treats differing-length prefixes as a match

While testing the fixes above against the existing `.c` sample files,
`hello.c` produced bogus `already defined` errors for `a()`, `ab()`,
`abc()`, `abcd()`, ... — distinct functions that only share a common
prefix. Root cause is in the general-purpose name/string comparator used
by both symbol-table lookups and keyword matching:

```c
long
astreq(char str1[], char str2[], int len) {
    int k;
    k=0;
    while (k < len) {
        if ((str1[k]) != (str2[k])) {
            printf(";* ### (%s) != (%s) [%d]\n", str1, str2, k);   // debug cruft
            printf(";* ### (%x) != (%x) [%d]\n", str1[k], str2[k], k);
            break;                      // falls through to `return k` below
        }
        if (str1[k] == 0)           break;
        if (str2[k] == 0)           break;
        k++;
    }
    return k;                          // BUG: returned even when the break above
}                                       //      was reached via a *real* mismatch
```

When one string is a strict prefix of the other (e.g. `"a"` vs `"ab"`),
the mismatch at the position right after the shorter string's NUL (`0`
vs `'b'`) is detected by the *first* `if`, which `break`s and falls into
`return k` — the exact same path taken for a genuine match. So `astreq`
returns a truthy, nonzero value for two names that are clearly different,
as long as they share a common leading substring. Every caller
(`findglb`, `findloc`, `amatch`, `findmac`) treats the return value as a
boolean "do these match", so this silently merges distinct identifiers
that share a prefix into the "already defined" / "found" case. It can
also make `amatch()` mis-recognize an identifier as a keyword it merely
starts with (e.g. an identifier `ins` can partially, incorrectly, match
the keyword `int`).

The two debug `printf()`s on the mismatch path are also leftover
debugging cruft: since this function is called on nearly every candidate
symbol-table entry/keyword during parsing, they flooded the generated
assembly output with thousands of `;* ### (...) != (...)` lines on even
a trivial program (1339 lines of output for `hello.c`, a ~45-line source
file).

**Fix applied:** the mismatch branch now `return (0)` directly instead of
`break`-ing into the shared `return k`, and the debug `printf()`s were
removed. `findglb`/`findloc` now correctly reject prefix-only matches,
`amatch`'s keyword-cursor-advance behavior (which relies on the matched
length `k`) is unaffected for genuine matches, and the output noise is
gone. Verified: `hello.c` now compiles with 0 errors (previously reported
spurious "already defined" errors for `a`/`ab`/`abc`/...).

## Summary / priority — status after fixes

| # | Issue | Severity | Status |
|---|-------|----------|--------|
| 1 | Build fails on current GCC (int/pointer conversions) | Blocking | **Fixed** |
| 2 | `locidx`/`findloc`/`addloc` mix `char *` and `long` inconsistently | Root cause of #1 | **Fixed** |
| 3 | While-queue (`wq[]`) stores `long` values as truncated single bytes but is read back as `long *` | Critical (silent miscompilation of `break`/`continue`) | **Fixed** |
| 4 | Reproducible SIGSEGV in `getmem()` documented via a committed gdb transcript | Critical | Not independently reproduced; two likely contributing causes (#2, #8) fixed |
| 5 | `options.c` silently "succeeds" with no input file; wrong char printed on bad option | Moderate (UX/diagnostics) | **Fixed** |
| 6 | Large dead-code branches (`#ifdef SKIP`, non-`__linux` path, duplicate `header()` call) | Low (maintainability) | Dead `#ifdef SKIP` blocks removed; duplicate `header()` call removed; non-`__linux` branch intentionally kept |
| 7 | Stale/unused declarations (`symbolX`, `locptr`) in `smallc.h` | Low (cleanliness) | **Fixed** |
| 8 | `astreq()` falsely matches differing names that share a prefix, plus debug-output flooding | Critical (found during verification) | **Fixed** |

Build verified clean (`make` produces no warnings besides the
intentional `#warning FDEBUG IS defined`), and `t1.c`, `tf-test.c`,
`hello.c`, and a new nested `while`/`break`/`continue` test all compile
with 0 errors and produce sane-looking generated assembly.
