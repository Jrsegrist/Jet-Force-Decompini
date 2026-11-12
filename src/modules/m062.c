/*
 * Runlink module 62: the multiplayer mode select of the front end (the
 * choice of game mode and its settings).
 */
#include "module.h"

extern s32 *gMenuText; /* the settings, read here as words */
extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */
extern void music_play(s32 seq);

#define SETTING(offset) (gMenuText[(offset) / 4])

/* The render mode for the page's images (SetCombine, SetOtherMode) */
static u32 sRenderMode[] = { 0xFC71FEE3, 0xFFFEFB7D, 0xEF000C0F, 0x00504240 };
static s16 sAssets2[] = { 0x43, 0x44, -1, 0 };
static s16 sImages[] = { 0x1F, 0x27, 0x28, 0x2D, -1, 0 };
static s16 sAssets[] = { 0x65, 0x7C, 0x81, -1 };
/* The page's other tables (the modes, their settings and texts) */
static u32 sTables[] = { 0x10101010, 0x10101007, 0x07000000 };
static s32 sPage = 8; /* the setting being changed */
static u32 sTables3C[] = { 0x06000107, 0x0208040A };
static u8 sCursor[8] = { 0 };
static u32 sTables4C[] = {
    0x00010302, 0x00010000, 0x02070308, 0x0409050A, 0x05040405, 0x01030200, 0x04060000, 0x0000006E,
    0x00000091, 0x000000C8, 0x00000164, 0x000100DD, 0x00020090, 0x00040163, 0x0008010E, 0x0149014E,
    0x014C0000, 0x00000010, 0x00000020, 0x00000030, 0x0001010C, 0x00000002, 0x01020100, 0x0004018D,
    0x01000010, 0x00CA0000, 0x01550149,
};
/* The game modes (only those whose bit is in D_800FF318[0x6B] can be picked) */
typedef struct M062Mode {
    /* 0x0 */ u16 mask;
    /* 0x2 */ s16 mode;
    /* 0x4 */ s32 arg;
} M062Mode;
static M062Mode sModes[] = { { 1, 0xFB, 0x1518 }, { 2, 0x16C, 0x189C }, { 4, 0x198, 0xE10 } };
static s16 sTimeLimits[] = { -1, 40, 60, 80, 100, 0 };
static u32 sTablesDC[] = {
    0xFFFF0001, 0x00030005, 0x000A0000, 0x090C0E00, 0x01400149, 0x02090150, 0x0144FFFF, 0x0145017A,
    0x01400156, 0x01440158, 0x020AFFFF, 0x0000017A, 0x0140FFFF, 0x0144FFFF, 0x015FFFFF, 0x0165017A,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
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
    menu_assetgroup_load(sAssets);
    menu_assetgroup_reset(sImages);
    menu_assetgroup_load(sAssets2);
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

INCLUDE_ASM("asm/nonmatchings/modules/m062/ro", func_m062_001E4);

INCLUDE_ASM("asm/nonmatchings/modules/m062/ro", func_m062_01158);

INCLUDE_ASM("asm/nonmatchings/modules/m062/ro", func_m062_01A48);

extern s8 D_800A500C;   /* the game mode picked */
extern u8 D_800FF318[]; /* [0x6B]: the game modes that can be picked */
extern s16 D_800A4FE8;
extern s32 D_800A4FDC;
extern s8 D_800A5010;   /* the time limit picked */
extern s8 D_800A5018;
extern s8 D_800A5020;
extern s8 D_800A5058;
extern s16 D_800FF1B8[4];
extern s32 gMenuTimer;
extern s8 D_800A4FD8;
extern s8 D_800A4FEC;
extern Gfx *D_800FF398;
extern void func_8006FCA8(Gfx **dl, s32 textX, s32 textY, char *text, s32 flags);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void music_volume_fade(f32 volume, s32 arg1);
extern void func_80047A20(void);
extern void func_800468C0(s32 arg0);
extern void func_8004665C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_800468A4(s32 arg0);
extern void func_80047580(s32 arg0);

/* Changes the setting being picked with the stick and draws it; the third button starts the game (called from 001E4) */
void func_m062_0228C(UNUSED s32 arg0) {
    s32 mask;
    s32 old;
    char buf[0x100];

    /* on one line: IDO then schedules the mask load as the ROM does */
    mask = D_800FF318[0x6B]; while (!(sModes[D_800A500C].mask & mask)) {
        D_800A500C++;
        if (D_800A500C >= 3) {
            D_800A500C = 0;
        }
    }
    D_800A4FE8 = sModes[D_800A500C].mode;
    D_800A4FDC = sModes[D_800A500C].arg;
    switch (sPage) {
        case 2:
            old = D_800A500C;
            if (sUnk44) {
                do {
                    D_800A500C--;
                    if (D_800A500C < 0) {
                        D_800A500C = 2;
                    }
                } while (!(sModes[D_800A500C].mask & mask));
            } else if (sUnk48) {
                do {
                    D_800A500C++;
                    if (D_800A500C >= 3) {
                        D_800A500C = 0;
                    }
                } while (!(sModes[D_800A500C].mask & mask));
            }
            if (old != D_800A500C) {
                func_80001248(0x16C, 0);
            }
            D_800A4FE8 = sModes[D_800A500C].mode;
            D_800A4FDC = sModes[D_800A500C].arg;
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSavedC0[D_800A500C], 4);
            break;
        case 4:
            if (sUnk44) {
                D_800A5010--;
                func_80001248(0x16C, 0);
            }
            if (sUnk48) {
                D_800A5010++;
                func_80001248(0x16C, 0);
            }
            if (D_800A5010 < 0) {
                D_800A5010 = 4;
            }
            if (D_800A5010 >= 5) {
                D_800A5010 = 0;
            }
            D_800A5018 = sTimeLimits[D_800A5010];
            if (D_800A5010 == 0) {
                func_800639E8(buf, "%s %s", SETTING(0x580), SETTING(0x584));
            } else {
                func_800639E8(buf, "%3d %s", D_800A5018, SETTING(0x584));
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, buf, 4);
            break;
        case 6:
            if (sUnk44) {
                D_800A5020--;
                func_80001248(0x16C, 0);
            }
            if (sUnk48) {
                D_800A5020++;
                func_80001248(0x16C, 0);
            }
            if (D_800A5020 < 0) {
                D_800A5020 = 2;
            }
            if (D_800A5020 >= 3) {
                D_800A5020 = 0;
            }
            func_8006FCA8(&D_800FF398, sUnk20, sUnk14, (char *) sSavedC8[D_800A5020], 4);
            break;
        case 7:
            if (sUnk28) {
                D_800A5058 = 1;
                music_volume_fade(0.5f, 0);
                for (old = 0; old < 4; old++) {
                    D_800FF1B8[old] = 0;
                }
                gMenuTimer = 0;
                D_800A4FD8 = 6;
                D_800A4FEC = 0;
                func_80047A20();
                func_800468C0(1);
                func_8004665C(D_800A4FE8, 0, 0, 16, 1, 0);
                func_800468A4(0);
                func_80047580(0);
            }
            break;
    }
    sCursor[2] = 6;
    sCursor[4] = 4;
    sCursor[6] = 7;
    sCursor[1] = 0;
    sCursor[3] = 0;
    sCursor[5] = 0;
    if (sUnk2C || sUnk28) {
        func_80001248(0x16D, 0);
        sPage = 8;
    }
}

void func_m062_02784(void) {
    menu_assetgroup_free(sAssets);
    menu_assetgroup_free(sAssets2);
}

/* (.rodata: C, and the assembly functions' own from ro/) */
