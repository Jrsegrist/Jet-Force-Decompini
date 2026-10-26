#include "common.h"

extern s32 D_800FB084;

void func_800426D0(void) {
    D_800FB084 = 0;
}

extern s32 D_800FB084;

void func_800426DC(s32 arg0) {
    D_800FB084 = arg0;
}

INCLUDE_ASM("asm/nonmatchings/432D0", func_800426E8);

extern s32 D_800FB080;

s32 func_8004270C(void) {
    return D_800FB080;
}

extern s32 D_800FB084;

s32 func_8004271C(void) {
    return D_800FB084;
}

INCLUDE_ASM("asm/nonmatchings/432D0", func_8004272C);

INCLUDE_ASM("asm/nonmatchings/432D0", func_800427E8);

INCLUDE_ASM("asm/nonmatchings/432D0", func_80042A74);

INCLUDE_ASM("asm/nonmatchings/432D0", func_80042F48);
