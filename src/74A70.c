/*
 * __osLeoInterrupt as the game has it: a stub that reports no 64DD interrupt
 * (the game has no 64DD support). Written from the assembly.
 */
#include "PR/os_internal.h"

s32 __osLeoInterrupt(void) {
    return 0;
}
