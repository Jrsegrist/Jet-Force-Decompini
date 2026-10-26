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

void func_80076154(void) {
    if (D_801047D0 != 0x80) {
        D_801047CC++;
    }
    *D_801047C8 = D_801047CC;
}

extern s32 D_800A74C8;
extern s32 D_800A74CC;
extern s32 D_800A74D0;

void func_80076198(void) {
    if (D_800A74C8) {
        D_800A74C8 = 0;
        D_800A74CC = 0;
        D_800A74D0 = 0;
    }
}

extern s8 D_800A74F0[];

s8 func_800761C8(u16 *arg0) {
    return D_800A74F0[(u8) (arg0[1] >> 8)];
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_800761E8);

extern s32 *D_800A74BC;
s32 func_800761E8(s32 *arg0);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800762A0);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800762F0);

INCLUDE_ASM("asm/nonmatchings/76C20", func_8007637C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800766A8);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800767EC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_8007688C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076928);

typedef struct AnimPath {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ u8 flags;
} AnimPath;
extern AnimPath **D_801045B0;

void func_800769DC(u8 index) {
    AnimPath *path = D_801045B0[index];
    if (path != NULL) {
        path->flags |= 8;
    }
}

void func_80076A1C(u8 index) {
    AnimPath *path = D_801045B0[index];
    if (path != NULL) {
        path->flags &= ~8;
    }
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076A5C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076ADC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076B44);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076D6C);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80076EA0);

extern s32 D_801045C8[16];
void func_80001228(s32 soundHandle);

void func_80077004(void) {
    s32 i;
    for (i = 0; i < 16; i++) {
        if (D_801045C8[i] != 0) {
            func_80001228(D_801045C8[i]);
            D_801045C8[i] = 0;
        }
    }
}

extern void *D_800A74B0;
extern s32 D_800A74B8;
void mempool_free(void *data);
void func_80077158(void);

void func_80077060(void) {
    if (D_800A74B0 != NULL) {
        mempool_free(D_800A74B0);
        D_800A74B0 = NULL;
        D_800A74B8 = -1;
        func_80077158();
    }
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_800770A4);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077158);

INCLUDE_ASM("asm/nonmatchings/76C20", func_800772C4);

void func_8007637C(u8 index);

void func_800773AC(void) {
    s32 i;
    for (i = 0; i < 256; i++) {
        func_8007637C(i);
    }
}

INCLUDE_ASM("asm/nonmatchings/76C20", func_800773EC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077468);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077614);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80077ABC);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079890);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079930);

INCLUDE_ASM("asm/nonmatchings/76C20", func_80079AA4);

s32 func_80079B6C(void) {
    s32 i = 256;
    while (i--) {
        if (D_801045B0[i] == NULL) {
            return i;
        }
    }
    return -1;
}

extern s32 D_800A74D4;

void func_80079BA8(void) {
    D_800A74D4 = 0x1;
}

extern s32 D_800A74D4;

void func_80079BB8(void) {
    D_800A74D4 = 0;
}
