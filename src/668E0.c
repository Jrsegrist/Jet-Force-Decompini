#include "common.h"

INCLUDE_ASM("asm/nonmatchings/668E0", func_80065CE0);

extern s32 D_800A64B8;
extern s32 D_801012A0[2];

void func_80065D24(void) {
    D_800A64B8 = 1 - D_800A64B8;
    D_801012A0[D_800A64B8] = 0;
}

INCLUDE_ASM("asm/nonmatchings/668E0", func_80065D50);

INCLUDE_ASM("asm/nonmatchings/668E0", func_80065E28);
