/* Runlink module 60: the object table for the multiplayer arenas. */
#include "module.h"

/* The module's data: tables of object entries (kept as assembly for now) */
extern u8 D_m060_00010[];

/* Returns the table of objects for the arenas */
void *func_m060_00000(void) {
    return D_m060_00010 + 0x900;
}

INCLUDE_ASM("asm/nonmatchings/modules/m060", data);
