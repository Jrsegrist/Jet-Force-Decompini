#include "common.h"

extern s32 *D_801047C8;
extern s32 D_801047CC;
extern s32 D_801047D0;

void func_80076020(s32 *arg0) {
    D_801047C8 = arg0;
    D_801047CC = *arg0;
    D_801047D0 = 0x80;
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076044);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800760C0);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076154);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076198);

extern s8 D_800A74F0[];

s8 func_800761C8(u16 *arg0) {
    return D_800A74F0[(u8) (arg0[1] >> 8)];
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_800761E8);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800762A0);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800762F0);

INCLUDE_ASM("asm/nonmatchings/76C20", func_8007637C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800766A8);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800767EC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_8007688C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076928);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800769DC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076A1C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076A5C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076ADC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076B44);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076D6C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076EA0);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077004);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077060);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800770A4);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077158);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800772C4);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800773AC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800773EC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077468);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077614);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077ABC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079890);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079930);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079AA4);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079B6C);

extern s32 D_800A74D4;

void func_80079BA8(void) {
    D_800A74D4 = 0x1;
}

extern s32 D_800A74D4;

void func_80079BB8(void) {
    D_800A74D4 = 0;
}
