/*
 * Runlink module 37: the options page of the front end (sound, music, screen
 * and the cheats), with its volume bars.
 */
#include "module.h"

typedef struct M037Words4 {
    s32 w[4];
} M037Words4;
typedef struct M037Words2 {
    s32 w[2];
} M037Words2;

/* What gMenuText points at (only the fields used here) */
typedef struct M037Settings {
    /* 0x000 */ M037Words4 unk0;
    /* 0x010 */ u8 pad10[0x264 - 0x10];
    /* 0x264 */ M037Words2 unk264;
    /* 0x26C */ u8 pad26C[0x274 - 0x26C];
    /* 0x274 */ M037Words2 unk274;
    /* 0x27C */ u8 pad27C[0x4BC - 0x27C];
    /* 0x4BC */ M037Words2 unk4BC;
} M037Settings;

extern M037Settings *gMenuText;
extern void func_80059E1C(s16 *assets); /* loads menu textures */
extern void func_80059FE8(s16 *assets);
extern void func_80059CCC(s16 *assets); /* frees menu textures */

/*
 * The module's .data and .rodata are still assembly: they are reached from
 * their first symbol (the ROM's relocations are section-relative).
 */
extern s16 rl_m037_01EB0[]; /* .data */
extern f32 rl_m037_01F70[]; /* .rodata */
#define sAssets (&rl_m037_01EB0[0x18])      /* 0x01EE0 */
#define sCheatAssets (&rl_m037_01EB0[0x1E]) /* 0x01EEC */
#define sImages (&rl_m037_01EB0[0x22])      /* 0x01EF4 */

/* The page's state */
static s32 sSaved0[4]; /* copies of settings */
static s32 sSaved1[2];
static s32 sSaved2[4];
static s32 sSaved3[2];
static s32 sUnk30;
static s32 sUnk34;
static s16 sUnk38;
static s16 sUnk3A;
static f32 sUnk3C;
static s32 sUnk40;
static s32 sUnk44;
static s32 sUnk48;
static s32 sNoCheats; /* func_m037_00000's argument */
static s32 sUnk50;
static s32 sUnk54;
static s32 sUnk58;
static f32 sUnk5C;
static f32 sUnk60;

void func_m037_00000(s32 arg0) {
    func_80059E1C(sAssets);
    func_80059FE8(sImages);
    if (arg0 == 0) {
        func_80059E1C(sCheatAssets);
    }
    sUnk5C = rl_m037_01F70[0x14]; /* 0.01f */
    sUnk40 = 0;
    sUnk54 = 1;
    sUnk50 = 0;
    sUnk44 = -1;
    sUnk38 = 0x36;
    sUnk3C = 7.0f;
    sUnk48 = 0;
    sSaved0[0] = gMenuText->unk0.w[0];
    sSaved0[1] = gMenuText->unk0.w[1];
    sSaved0[2] = gMenuText->unk0.w[2];
    sSaved0[3] = gMenuText->unk0.w[3];
    sSaved1[0] = gMenuText->unk4BC.w[0];
    sSaved1[1] = gMenuText->unk4BC.w[1];
    sSaved2[0] = gMenuText->unk264.w[0];
    sSaved2[1] = gMenuText->unk264.w[1];
    sSaved2[2] = gMenuText->unk264.w[0];
    sSaved2[3] = gMenuText->unk264.w[1];
    sSaved3[0] = gMenuText->unk274.w[0];
    sSaved3[1] = gMenuText->unk274.w[1];
    sNoCheats = arg0;
    sUnk30 = 0;
    sUnk3A = 10;
    sUnk60 = 0.0f;
    sUnk58 = 0;
}

void func_m037_00160(void) {
    func_80059CCC(sAssets);
    if (sNoCheats == 0) {
        func_80059CCC(sCheatAssets);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m037", func_m037_001A0);

INCLUDE_ASM("asm/nonmatchings/modules/m037", func_m037_01ACC);

INCLUDE_ASM("asm/nonmatchings/modules/m037", data);

INCLUDE_ASM("asm/nonmatchings/modules/m037", rodata);
