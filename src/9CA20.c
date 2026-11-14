/*
 * The formatted-output engine behind sprintf and osSyncPrintf, in the form
 * the game was built with: an older _Printf than ultralib's (the literal text
 * before each conversion is scanned to and including its terminating byte),
 * with the field formatter compiled ahead of it.
 *
 * Written from the assembly (user ruling 2026-11-13: the object is not
 * ultralib's xprintf.o, so it can't be #included). Built like the rest of the
 * game's libc: -O3 -mips2, without asm-processor (it has no -O3), so the
 * field formatter gets interprocedural registers and comes first.
 */
#include "stdarg.h"
#include "PR/ultratypes.h"
#include "../lib/ultralib/src/libc/xstdio.h"

char *strchr(const char *s, int c);
size_t strlen(const char *s);

/* Padding sources: up to 32 characters are handed to the output at a time */
static char sPadSpaces[] = "                                ";
static char sPadZeroes[] = "00000000000000000000000000000000";

#define PAD_CHUNK (sizeof(sPadSpaces) - 1)

/* Hand LEN bytes from STR to the output function; stop on a NULL result */
#define OUTPUT(str, len)                                     \
    if ((len) > 0) {                                         \
        if ((dest = (*out)(dest, (str), (len))) != NULL) {   \
            fld.nchar += (len);                              \
        } else {                                             \
            return fld.nchar;                                \
        }                                                    \
    }

/* Output COUNT copies of the padding character in SRC */
#define OUTPUT_PAD(src, count)                                          \
    if ((count) > 0) {                                                  \
        int chunk;                                                      \
        int left = (count);                                             \
        for (; left > 0; left -= chunk) {                               \
            chunk = ((unsigned int) left > PAD_CHUNK) ? (int) PAD_CHUNK : left; \
            OUTPUT(src, chunk);                                         \
        }                                                               \
    }

#define IS_DIGIT(ch) ((ch) >= '0' && (ch) <= '9')

/* Formats one conversion into buf (prefix) and fld->s (body) */
static void _Putfld(_Pft *fld, va_list *args, char conv, char *buf) {
    fld->n0 = fld->nz0 = fld->n1 = fld->nz1 = fld->n2 = fld->nz2 = 0;

    switch (conv) {
        case 'c':
            buf[fld->n0++] = va_arg(*args, int);
            break;

        case 'd':
        case 'i':
            if (fld->qual == 'l') {
                fld->v.ll = va_arg(*args, long);
            } else if (fld->qual == 'L') {
                fld->v.ll = va_arg(*args, long long);
            } else {
                fld->v.ll = va_arg(*args, int);
            }
            if (fld->qual == 'h') {
                fld->v.ll = (short) fld->v.ll;
            }
            if (fld->v.ll < 0) {
                buf[fld->n0++] = '-';
            } else if (fld->flags & FLAGS_PLUS) {
                buf[fld->n0++] = '+';
            } else if (fld->flags & FLAGS_SPACE) {
                buf[fld->n0++] = ' ';
            }
            fld->s = (unsigned char *) &buf[fld->n0];
            _Litob(fld, conv);
            break;

        case 'o':
        case 'u':
        case 'x':
        case 'X':
            if (fld->qual == 'l') {
                fld->v.ll = va_arg(*args, long);
            } else if (fld->qual == 'L') {
                fld->v.ll = va_arg(*args, long long);
            } else {
                fld->v.ll = va_arg(*args, int);
            }
            if (fld->qual == 'h') {
                fld->v.ll = (unsigned short) fld->v.ll;
            } else if (fld->qual == '\0') {
                fld->v.ll = (unsigned int) fld->v.ll;
            }
            if (fld->flags & FLAGS_HASH) {
                buf[fld->n0++] = '0';
                if (conv == 'x' || conv == 'X') {
                    buf[fld->n0++] = conv;
                }
            }
            fld->s = (unsigned char *) &buf[fld->n0];
            _Litob(fld, conv);
            break;

        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G':
            if (fld->qual == 'L') {
                fld->v.ld = va_arg(*args, ldouble);
            } else {
                fld->v.ld = va_arg(*args, double);
            }
            /* sign bit of the double's top halfword */
            if (*(u16 *) &fld->v.ld & 0x8000) {
                buf[fld->n0++] = '-';
            } else if (fld->flags & FLAGS_PLUS) {
                buf[fld->n0++] = '+';
            } else if (fld->flags & FLAGS_SPACE) {
                buf[fld->n0++] = ' ';
            }
            fld->s = (unsigned char *) &buf[fld->n0];
            _Ldtob(fld, conv);
            break;

        case 'n':
            if (fld->qual == 'h') {
                *va_arg(*args, unsigned short *) = fld->nchar;
            } else if (fld->qual == 'l') {
                *va_arg(*args, unsigned long *) = fld->nchar;
            } else if (fld->qual == 'L') {
                *va_arg(*args, unsigned long long *) = fld->nchar;
            } else {
                *va_arg(*args, unsigned int *) = fld->nchar;
            }
            break;

        case 'p':
            fld->v.ll = (long) va_arg(*args, void *);
            fld->s = (unsigned char *) &buf[fld->n0];
            _Litob(fld, 'x');
            break;

        case 's':
            fld->s = va_arg(*args, unsigned char *);
            fld->n1 = strlen((char *) fld->s);
            if (fld->prec >= 0 && fld->prec < fld->n1) {
                fld->n1 = fld->prec;
            }
            break;

        case '%':
            buf[fld->n0++] = '%';
            break;

        default:
            buf[fld->n0++] = conv;
            break;
    }
}

int _Printf(void *out(void *, const char *, size_t), void *dest, const char *fmt, va_list ap) {
    _Pft fld;

    fld.nchar = 0;

    while (TRUE) {
        static const char sFlagChars[] = { ' ', '+', '-', '#', '0', '\0' };
        static const unsigned int sFlagBits[] = {
            FLAGS_SPACE, FLAGS_PLUS, FLAGS_MINUS, FLAGS_HASH, FLAGS_ZERO, 0,
        };
        const char *p;
        const char *found;
        int ch;
        char buf[32];
        int pct;
        int unused; /* an unused word in the original frame */

        /* the literal text, up to a '%' or through the terminator */
        pct = '%';
        p = fmt;
        while ((ch = *(unsigned char *) p++) > 0) {
            if (pct == ch) {
                p--;
                break;
            }
        }
        OUTPUT(fmt, p - fmt);
        if (ch == 0) {
            return fld.nchar;
        }
        p++;

        fld.flags = 0;
        while ((found = strchr(sFlagChars, *p)) != NULL) {
            fld.flags |= sFlagBits[found - sFlagChars];
            p++;
        }

        if (*p == '*') {
            fld.width = va_arg(ap, int);
            p++;
            if (fld.width < 0) {
                fld.width = -fld.width;
                fld.flags |= FLAGS_MINUS;
            }
        } else {
            for (fld.width = 0; IS_DIGIT(*p); p++) {
                if (fld.width < 999) {
                    fld.width = *p + fld.width * 10 - '0';
                }
            }
        }

        if (*p != '.') {
            fld.prec = -1;
        } else if (*++p == '*') {
            fld.prec = va_arg(ap, int);
            p++;
        } else {
            for (fld.prec = 0; IS_DIGIT(*p); p++) {
                if (fld.prec < 999) {
                    fld.prec = *p + fld.prec * 10 - '0';
                }
            }
        }

        if (strchr("hlL", *p) != NULL) {
            fld.qual = *p++;
        } else {
            fld.qual = '\0';
        }
        if (fld.qual == 'l' && *p == 'l') {
            fld.qual = 'L';
            p++;
        }

        _Putfld(&fld, &ap, *p, buf);
        fld.width = fld.width - fld.n0 - fld.nz0 - fld.n1 - fld.nz1 - fld.n2 - fld.nz2;

        if (!(fld.flags & FLAGS_MINUS)) {
            OUTPUT_PAD(sPadSpaces, fld.width);
        }
        OUTPUT(buf, fld.n0);
        OUTPUT_PAD(sPadZeroes, fld.nz0);
        OUTPUT((char *) fld.s, fld.n1);
        OUTPUT_PAD(sPadZeroes, fld.nz1);
        OUTPUT((char *) fld.s + fld.n1, fld.n2);
        OUTPUT_PAD(sPadZeroes, fld.nz2);
        if (fld.flags & FLAGS_MINUS) {
            OUTPUT_PAD(sPadSpaces, fld.width);
        }
        fmt = p + 1;
    }
}
