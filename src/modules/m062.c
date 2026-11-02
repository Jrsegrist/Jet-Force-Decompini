/*
 * Runlink module 62: the multiplayer mode select of the front end (the
 * choice of game mode and its settings).
 */
#include "module.h"

extern s32 *gMenuText; /* the settings, read here as words */
extern void func_80059E1C(s16 *assets); /* loads menu textures */
extern void func_80059FE8(s16 *assets);
extern void func_80059CCC(s16 *assets); /* frees menu textures */
extern void music_play(s32 seq);

#define SETTING(offset) (gMenuText[(offset) / 4])

/* The render mode for the page's images (SetCombine, SetOtherMode) */
static u32 sRenderMode[] = { 0xFC71FEE3, 0xFFFEFB7D, 0xEF000C0F, 0x00504240 };
static s16 sAssets2[] = { 0x43, 0x44, -1, 0 };
static s16 sImages[] = { 0x1F, 0x27, 0x28, 0x2D, -1, 0 };
static s16 sAssets[] = { 0x65, 0x7C, 0x81, -1 };
/* The page's other tables (the modes, their settings and texts) */
static u32 sTables[] = {
    0x10101010, 0x10101007, 0x07000000, 0x00000008, 0x06000107, 0x0208040A, 0x00000000, 0x00000000,
    0x00010302, 0x00010000, 0x02070308, 0x0409050A, 0x05040405, 0x01030200, 0x04060000, 0x0000006E,
    0x00000091, 0x000000C8, 0x00000164, 0x000100DD, 0x00020090, 0x00040163, 0x0008010E, 0x0149014E,
    0x014C0000, 0x00000010, 0x00000020, 0x00000030, 0x0001010C, 0x00000002, 0x01020100, 0x0004018D,
    0x01000010, 0x00CA0000, 0x01550149, 0x000100FB, 0x00001518, 0x0002016C, 0x0000189C, 0x00040198,
    0x00000E10, 0xFFFF0028, 0x003C0050, 0x00640000, 0xFFFF0001, 0x00030005, 0x000A0000, 0x090C0E00,
    0x01400149, 0x02090150, 0x0144FFFF, 0x0145017A, 0x01400156, 0x01440158, 0x020AFFFF, 0x0000017A,
    0x0140FFFF, 0x0144FFFF, 0x015FFFFF, 0x0165017A, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000,
};

static s32 sUnk0;
static s16 sUnk4;
static s16 sUnk6;
static f32 sUnk8;
static s32 sUnkC;
static s32 sUnk10;
static s32 sUnk14;
static s32 sUnk18;
static s32 sUnk1C;
static s32 sUnk20;
static s32 sUnk24;
static s32 sUnk28;
static s32 sUnk2C;
static s32 sUnk30;
static s32 sUnk34;
static s32 sUnk38;
static s32 sUnk3C;
static s32 sUnk40;
static s32 sUnk44;
static s32 sUnk48;
static s32 sUnk4C;
/* Copies of settings */
static s32 sSaved50[4];
static s32 sSaved60[4];
static s32 sSaved70[8];
static s32 sSaved90[4];
static s32 sSavedA0[2];
static s32 sSavedA8[4];
static s32 sSavedB8[2];
static s32 sSavedC0[2];
static s32 sSavedC8[3];

void func_m062_00000(void) {
    func_80059E1C(sAssets);
    func_80059FE8(sImages);
    func_80059E1C(sAssets2);
    sSaved50[0] = SETTING(0x5A0);
    sSaved50[1] = SETTING(0x5A4);
    sSaved50[2] = SETTING(0x5A8);
    sSaved60[0] = SETTING(0x504);
    sSaved60[1] = SETTING(0x508);
    sSaved60[2] = SETTING(0x50C);
    sSaved70[0] = SETTING(0x5AC);
    sSaved70[1] = SETTING(0x5B0);
    sSaved70[2] = SETTING(0x5B4);
    sSaved70[3] = SETTING(0x6FC);
    sSaved70[4] = SETTING(0x5B8);
    sSaved70[5] = SETTING(0x5BC);
    sSaved70[6] = SETTING(0x6F8);
    sSaved70[7] = SETTING(0x6F4);
    sSaved90[0] = SETTING(0x518);
    sSaved90[1] = SETTING(0x51C);
    sSaved90[2] = SETTING(0x520);
    sSavedA0[0] = SETTING(0x544);
    sSavedA0[1] = SETTING(0x548);
    sSavedA8[0] = SETTING(0x54C);
    sSavedA8[1] = SETTING(0x550);
    sSavedA8[2] = SETTING(0x82C);
    sSavedA8[3] = SETTING(0x830);
    sSavedB8[0] = SETTING(0x554);
    sSavedB8[1] = SETTING(0x504);
    sSavedC0[0] = SETTING(0x280);
    sSavedC0[1] = SETTING(0x5AC);
    sSavedC8[0] = SETTING(0x544);
    sSavedC8[1] = SETTING(0x598);
    sSavedC8[2] = SETTING(0x59C);
    sUnkC = 0;
    sUnk10 = -1;
    sUnk4 = 0x36;
    sUnk8 = 7.0f;
    sUnk4C = 0;
    music_play(0x10);
    sUnk18 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m062", func_m062_001E4);

INCLUDE_ASM("asm/nonmatchings/modules/m062", func_m062_01158);

INCLUDE_ASM("asm/nonmatchings/modules/m062", func_m062_01A48);

INCLUDE_ASM("asm/nonmatchings/modules/m062", func_m062_0228C);

void func_m062_02784(void) {
    func_80059CCC(sAssets);
    func_80059CCC(sAssets2);
}

INCLUDE_ASM("asm/nonmatchings/modules/m062", rodata);
