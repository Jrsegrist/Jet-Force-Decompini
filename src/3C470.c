#include "common.h"

void func_8003B870(u16 *src, u16 *dst, s32 size) {
    size = (size + 1) >> 1;
    while (size--) {
        *dst++ = *src++;
    }
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003B8AC);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003B9E8);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C0C4);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C12C);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C1B4);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C7C4);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C92C);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CB04);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CDAC);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CF24);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CFCC);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003D150);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003D21C);

extern s32 D_800A2EC0;

void func_8003DDBC(s32 arg0) {
    D_800A2EC0 = arg0;
}

extern s32 D_800A2EC0;

s32 func_8003DDC8(void) {
    return D_800A2EC0;
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003DDD8);

extern s8 D_800F6F48[3];
extern s16 D_800F6F4C[3];

void func_8003E35C(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        D_800F6F48[i] = -1;
        D_800F6F4C[i] = 1000;
    }
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003E398);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003E638);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003EF60);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003F0E4);

extern s32 D_800F6F58;

s32 func_8003F1B0(s32 *size) {
    *size = 0x400;
    return D_800F6F58;
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003F1C8);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003F284);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003F468);
