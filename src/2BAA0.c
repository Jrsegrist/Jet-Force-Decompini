#include "common.h"

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002AEA0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B064);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B1D4);

/* Size: 0x2C bytes */
typedef struct CameraBlend {
    /* 0x00 */ f32 blend;
    /* 0x04 */ u8 pad4[0x28];
} CameraBlend;
extern CameraBlend D_800F6E70[];

f32 func_8002B3F0(s32 index) {
    return D_800F6E70[index].blend;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B418);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B960);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002BD04);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002C070);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002C118);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002C908);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002C9A0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002CC70);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002CF6C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002EB6C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002ED94);

extern s32 D_800A2D38;
extern s32 D_800F6DF8[24];

void func_8002F014(s32 obj) {
    if (D_800A2D38 < 24) {
        D_800F6DF8[D_800A2D38] = obj;
        D_800A2D38++;
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F04C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F0DC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F2B0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F450);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F50C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F5F0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F728);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031090);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800319C0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031A80);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031D00);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031F44);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031F9C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031FE8);

void func_80032128(s32 arg0) {
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80032130);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80032A48);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033920);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800339D8);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033BB8);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033D74);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033ED4);

s32 func_80033F4C(s32 weapon) {
    if (weapon == 8 || weapon == 10 || weapon == 11 || weapon == 12 || weapon == 13 || weapon == 14) {
        return 1;
    }
    if (weapon == 9) {
        return 2;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033FA4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80034018);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800340A4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80034164);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800341B4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800344B4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800346C4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800349AC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80034BB0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035514);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035628);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035894);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035AEC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035B30);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035E14);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800364D4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80036A30);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80036DBC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800370D0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80037194);

extern s32 D_800F6DBC;

void func_80037244(s32 arg0) {
    D_800F6DBC = arg0;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80037250);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003733C);

s32 func_800373E4(u8 *chr, s32 weapon) {
    switch (weapon) {
        case 6:
            return 110;
        case 4:
            return 30;
    }
    return (chr + weapon)[0x54] * 32;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80037420);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800374C8);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80037550);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038324);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038430);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038528);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038630);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800386B0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038F04);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038F7C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039098);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039308);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039544);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800396CC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800397E0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039918);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039A38);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039B98);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039D08);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039ECC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039FCC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003AABC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B114);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B16C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B290);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B418);

/* JFG: addresses of two tables inside an object's state block (at 0x68). */
u8 *func_8003B4D8(u8 **obj) {
    u8 *state = obj[0x68 / 4];
    return state + 0x240;
}

u8 *func_8003B4E4(u8 **obj) {
    u8 *state = obj[0x68 / 4];
    return state + 0x96;
}

extern f32 D_800F6DC4;

f32 func_8003B4F0(void) {
    return D_800F6DC4;
}

extern u8 D_800A1490[][0x30];

/* JFG: an entry of a table of 0x30-byte records. */
u8 *func_8003B500(s32 index) {
    return D_800A1490[index];
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B51C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B590);

extern s16 D_800F6DEC;
extern s16 D_800F6DEE;
extern s16 D_800F6DF0;
extern s16 D_800F6DF2;
extern u8 D_800A2EB0;

void func_8003B620(s16 a, s16 b, s16 c, s16 d) {
    D_800F6DEC = a;
    D_800F6DEE = b;
    D_800F6DF0 = c;
    D_800F6DF2 = d;
    D_800A2EB0 = 1;
}

s32 func_8003B660(s16 *a, s16 *b, s16 *c, s16 *d) {
    if (D_800A2EB0 != 0) {
        *a = D_800F6DEC;
        *b = D_800F6DEE;
        *c = D_800F6DF0;
        *d = D_800F6DF2;
        D_800A2EB0 = 0;
        return 1;
    }
    return 0;
}

extern u8 D_800A2EB0;

void func_8003B6BC(void) {
    D_800A2EB0 = 0;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B6C8);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B740);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B820);
