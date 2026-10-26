#ifndef COMMON_H
#define COMMON_H

#include "ultra64.h"

/*
 * INCLUDE_ASM("asm/nonmatchings/<file>", func_XXXXXXXX); pulls in a function
 * that is not decompiled yet. With IDO, tools/asm-processor replaces the line
 * before the compiler sees it; this definition only serves other compilers
 * (syntax checks, editors).
 */
#ifndef INCLUDE_ASM
#define INCLUDE_ASM(dir, name) extern int __include_asm_##name
#endif

#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))

#define UNUSED

/* Debug prints compiled out of the retail build (as in the DKR decompilation). */
#ifdef __GNUC__
#define stubbed_printf(...)
#else
#define stubbed_printf
#endif

#endif
