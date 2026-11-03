/*
 * Runlink module 53: a vehicle the players ride (its init, control and camera; still mostly
 * assembly).
 */
#include "module.h"

/* The module's .rodata and .bss (still assembly) */
extern f32 rl_m053_03710;
extern f32 rl_m053_03828[4];
extern f32 rl_m053_03838[4];
extern f32 rl_m053_03848[4];

/* Resets player `player`'s camera values */
void func_m053_00000(s32 player) {
    rl_m053_03828[player] = rl_m053_03710;
    rl_m053_03838[player] = 0.0f;
    rl_m053_03848[player] = 0.0f;
}

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_00038);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_00EA0);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_014F4);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_01850);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_019B0);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_01BB4);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_021E4);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_0230C);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_023D0);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_02708);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_0271C);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_028CC);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_028DC);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_02B48);

INCLUDE_ASM("asm/nonmatchings/modules/m053", data);

INCLUDE_ASM("asm/nonmatchings/modules/m053", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m053", bss);
