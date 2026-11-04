/*
 * Runlink module 42: the on-screen keyboard of the front end (entering a
 * name).
 */
#include "module.h"

/* One key of the keyboard (in .data) */
typedef struct M042Key {
    /* 0x0 */ u8 unk0[4];
    /* 0x4 */ f32 unk4;
} M042Key;

/* What D_800FF780 holds (only the fields used here) */
typedef struct M042Ui {
    /* 0x000 */ u8 pad0[0x3E6];
    /* 0x3E6 */ s16 unk3E6;
    /* 0x3E8 */ u8 pad3E8[0x660 - 0x3E8];
    /* 0x660 */ s16 unk660;
    /* 0x662 */ u8 pad662[0x680 - 0x662];
    /* 0x680 */ s16 unk680;
} M042Ui;

extern M042Ui D_800FF780;
extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */
extern void mark_read_all_save_files(void *buf);
extern void music_play(s32 seq);

static s16 sAssets[] = { 149, 150, 151, -1 };
static s16 sImages[] = { 51, 52, 31, -1 };
static s32 sLoaded = FALSE;
static s32 sUnk2CB4[2] = { 0, 0 };
/* The keys: their number, another key's number and the character */
static M042Key sKeys[] = {
    { { 0x1A, 0x0C, '1', 0 }, 0.0f }, { { 0x1B, 0x0B, '2', 0 }, 0.0f }, { { 0x1C, 0x0A, '3', 0 }, 0.0f },
    { { 0x1D, 0x09, '4', 0 }, 0.0f }, { { 0x1E, 0x08, '5', 0 }, 0.0f }, { { 0x1F, 0x07, '6', 0 }, 0.0f },
    { { 0x20, 0x06, '7', 0 }, 0.0f }, { { 0x21, 0x05, '8', 0 }, 0.0f }, { { 0x22, 0x04, '9', 0 }, 0.0f },
    { { 0x23, 0x03, '0', 0 }, 0.0f }, { { 0x00, 0x16, 'A', 0 }, 0.0f }, { { 0x01, 0x15, 'B', 0 }, 0.0f },
    { { 0x02, 0x14, 'C', 0 }, 0.0f }, { { 0x03, 0x13, 'D', 0 }, 0.0f }, { { 0x04, 0x12, 'E', 0 }, 0.0f },
    { { 0x05, 0x11, 'F', 0 }, 0.0f }, { { 0x06, 0x10, 'G', 0 }, 0.0f }, { { 0x07, 0x0F, 'H', 0 }, 0.0f },
    { { 0x08, 0x0E, 'I', 0 }, 0.0f }, { { 0x09, 0x0D, 'J', 0 }, 0.0f }, { { 0x0A, 0x20, 'K', 0 }, 0.0f },
    { { 0x0B, 0x1F, 'L', 0 }, 0.0f }, { { 0x0C, 0x1E, 'M', 0 }, 0.0f }, { { 0x0D, 0x1D, 'N', 0 }, 0.0f },
    { { 0x0E, 0x1C, 'O', 0 }, 0.0f }, { { 0x0F, 0x1B, 'P', 0 }, 0.0f }, { { 0x10, 0x1A, 'Q', 0 }, 0.0f },
    { { 0x11, 0x19, 'R', 0 }, 0.0f }, { { 0x12, 0x18, 'S', 0 }, 0.0f }, { { 0x13, 0x17, 'T', 0 }, 0.0f },
    { { 0x14, 0x2A, 'U', 0 }, 0.0f }, { { 0x15, 0x29, 'V', 0 }, 0.0f }, { { 0x16, 0x28, 'W', 0 }, 0.0f },
    { { 0x17, 0x27, 'X', 0 }, 0.0f }, { { 0x18, 0x26, 'Y', 0 }, 0.0f }, { { 0x19, 0x25, 'Z', 0 }, 0.0f },
    { { 0x26, 0x24, '-', 0 }, 0.0f }, { { 0x27, 0x23, ' ', 0 }, 0.0f }, { { 0x28, 0x22, '.', 0 }, 0.0f },
    { { 0x25, 0x21, 1, 0 }, 0.0f },   { { 0x24, 0x2B, 2, 0 }, 0.0f },
};
#define KEY_COUNT 41
static s32 sUnk2E04[] = { 800, 1000, 0, 0x8000000, 0, 0, 0 };
static s32 sUnk2E20[] = { 2, 4, 1, 8, 1, 4, 2, 8, 0x100, 0x400, 0x200, 0x800 };

static s32 sUnk0;
static s32 sUnk4;
static s32 sList[10]; /* func_m042_02C80 */
static char sName[0x18];
static char *sCursor;
static s16 sUnk4C;
static s8 sUnk4E;
static s8 sUnk4F;
static s8 sUnk50;
static s8 sUnk51;
static s16 sUnk52;
static s32 sUnk54;
static s32 sUnk58;
static s32 sUnk5C;
static s32 sUnk60[12];
static s8 sUnk90;
static s8 sUnk91;
static s8 sUnk92;
static s8 sUnk93;
static f32 sUnk94;
static u8 sBuffer[0x1FE8 - 0x98];
static s16 sUnk1FE8;
static s16 sUnk1FEA;
static s32 sUnk1FEC;
static s8 sPairs[6][2];
static s32 sUnk1FFC;

void func_m042_00000(void) {
    s32 i;

    if (sLoaded == 0) {
        sLoaded = 1;
        menu_assetgroup_load(sAssets);
        menu_assetgroup_reset(sImages);
        D_800FF780.unk3E6 = 0x97;
        D_800FF780.unk660 = -0x8000;
        sUnk4C = 0x7FF0;
        D_800FF780.unk680 = -0x8000;
        for (i = 0; i < KEY_COUNT; i++) {
            sKeys[i].unk4 = 0.0f;
        }
        sCursor = sName;
        *sCursor = '\0';
        sUnk90 = 0;
        sUnk94 = -20.0f;
        sUnk4E = 0;
        sUnk4F = 0;
        sUnk50 = 0;
        sUnk1FE8 = 0;
        mark_read_all_save_files(sBuffer);
        for (i = 0; i < 6; i++) {
            sPairs[i][0] = -1;
            sPairs[i][1] = 0;
        }
        music_play(2);
        sUnk0 = 0;
    }
}

void func_m042_00150(void) {
    menu_assetgroup_free(sAssets);
    sLoaded = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m042", func_m042_00178);

s32 *func_m042_02C80(void) {
    return sList;
}

/* Does nothing (called from 00178) */
void func_m042_02C8C(UNUSED s32 arg0) {
}

INCLUDE_ASM("asm/nonmatchings/modules/m042", rodata);
