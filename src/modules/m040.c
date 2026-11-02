/*
 * Runlink module 40: the multiplayer statistics screen of the front end.
 */
#include "module.h"

/* The box the statistics are drawn in (passed to menu_mode_get) */
typedef struct M040Box {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s8 unk4;
    /* 0x05 */ s8 unk5;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 padC[0x20 - 0xC];
    /* 0x20 */ s16 unk20;
    /* 0x22 */ u8 pad22[0x28 - 0x22];
} M040Box;

extern s32 D_800FECCC;
extern void func_80059CCC(s16 *assets); /* frees menu textures */

static s16 sAssets2[] = { 0x43, 0x44, -1, 0 };
static s16 sImages[] = { 0x1F, 0x2E, -1, 0 };
static s16 sAssets[] = { 0x65, 0x82, 0x83, 0x84, 0x85, 0x86, -1, 0 };
static u32 sColours[] = { 0xFFFFFFFF, 0xDFDFDFFF, 0xBFBFBFFF, 0x9F9F9FFF, 0x7F7F7FFF, 0x5F5F5FFF };
static s8 sOrder[] = { 4, 2, 1, 0 };

static M040Box sBox;
static s32 sTiltList[6]; /* func_m040_024C0 */
static s32 sUnk40;
static s32 sUnk44;
static u8 sUnk48;
static s8 sUnk49;
static s16 sUnk4A;
static s16 sUnk4C;
static s16 sUnk4E;
static u8 sUnk50[8];
static s32 sUnk58;
static s32 sUnk5C;

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_0019C);

void func_m040_007C8(void) {
    func_80059CCC(sAssets);
    D_800FECCC = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_007F0);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_00DF8);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_0124C);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_0193C);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_01D60);

s32 *func_m040_024C0(void) {
    return sTiltList;
}

INCLUDE_ASM("asm/nonmatchings/modules/m040", rodata);
