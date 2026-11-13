/*
 * osMotorStop and osMotorStart as functions. The game's SDK 2.0I headers
 * declare them, but its rumble pak object is the 2.0J one, which has only
 * __osMotorAccess (2.0J makes the two names macros). Written from the
 * assembly; built like the libultra objects around it (-O2 -g3 -mips2).
 */
#include "common.h"

extern s32 __osMotorAccess(OSPfs *pfs, s32 flag);

s32 osMotorStop(OSPfs *pfs) {
    return __osMotorAccess(pfs, 0);
}

s32 osMotorStart(OSPfs *pfs) {
    return __osMotorAccess(pfs, 1);
}
