#include "common.h"

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001CE10);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D344);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D464);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D500);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D800);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D938);

extern s32 D_800A0EC8;

/* JFG: flip between two buffers. */
void func_8001D9B4(void) {
    D_800A0EC8 ^= 1;
}

extern s32 D_800A0E80[4];
extern s32 D_800A0E90[4];
extern s32 D_800A0EA0[4];

void func_8001D9D0(s32 flags, s32 *a, s32 *b, s32 *c) {
    s32 index = D_800A0EC8;
    if (flags & 2) {
        index += 2;
    }
    *a = D_800A0E80[index];
    *b = D_800A0E90[index];
    *c = D_800A0EA0[index];
}

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001DA1C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E17C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E8B8);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001EE1C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001F4BC);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001FA24);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001FFE0);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_800203C4);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020890);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020954);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_800209E4);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020AF8);
