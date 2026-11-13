/*
 * osSyncPrintf in the game's form: it formats through _Printf with an output
 * function that discards the text. Written from the assembly; built like the
 * game's other 2.0I os/io objects (-O1 -mips2).
 */
#include "stdarg.h"
#include "PR/os_internal.h"
#include "../lib/ultralib/src/libc/xstdio.h"

static void *syncprintf_discard(void *str, const char *buf, size_t n) {
    return (void *) 1;
}

void osSyncPrintf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    _Printf(syncprintf_discard, NULL, fmt, ap);
    va_end(ap);
}
