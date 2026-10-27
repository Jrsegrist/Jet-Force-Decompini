#include "common.h"

INCLUDE_ASM("asm/nonmatchings/50300", func_8004F700);

INCLUDE_ASM("asm/nonmatchings/50300", func_8004F85C);

INCLUDE_ASM("asm/nonmatchings/50300", func_8004F8C0);

/* The scheduler's command queue (at 0x78 in its state) (0x8004F950). */
void *osScGetCmdQ(u8 *sc) {
    return sc + 0x78;
}

/* The scheduler's interrupt queue (at 0x40 in its state) (0x8004F958). */
void *osScGetInterruptQ(u8 *sc) {
    return sc + 0x40;
}

void func_8004F960(f32 *a, f32 *b, f32 *c) {
    *a = 0.0f;
    *b = 0.0f;
    *c = 0.0f;
}

INCLUDE_ASM("asm/nonmatchings/50300", func_8004F978);

INCLUDE_ASM("asm/nonmatchings/50300", func_8004FB30);

INCLUDE_ASM("asm/nonmatchings/50300", func_8004FBD8);

void func_8004FC64(s32 arg0) {
}

INCLUDE_ASM("asm/nonmatchings/50300", func_8004FC6C);

INCLUDE_ASM("asm/nonmatchings/50300", func_8004FF64);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050154);

INCLUDE_ASM("asm/nonmatchings/50300", func_800507F8);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050924);

INCLUDE_ASM("asm/nonmatchings/50300", func_800509B0);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050A08);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050B08);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050B70);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050C3C);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050C90);
