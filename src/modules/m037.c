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
extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */

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
    menu_assetgroup_load(sAssets);
    menu_assetgroup_reset(sImages);
    if (arg0 == 0) {
        menu_assetgroup_load(sCheatAssets);
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
    menu_assetgroup_free(sAssets);
    if (sNoCheats == 0) {
        menu_assetgroup_free(sCheatAssets);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m037", func_m037_001A0);

/* A colour cycle (55130.c's) */
typedef struct M037ColourCycle {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4];
    /* 0x0C */ void *source;
} M037ColourCycle;

extern s8 D_800FF3E0; /* the stick's left/right */
extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern void func_m006_010D8(s32 *x, s32 *y);
extern void func_m006_0107C(s32 *x, s32 *y);
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_8005A0D0(s32 item); /* draws a menu item */
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3); /* sets a texture */
extern void colour_cycle_init(M037ColourCycle *cycle, s32 assetID);
extern void update_colour_cycle(M037ColourCycle *cycle, s32 updateRate);

/* A menu item (D_800FF780, size 0x20; the main segment's MenuItem), as used here */
typedef struct M037MenuItem {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 posY;
    /* 0x14 */ u8 pad14[0x20 - 0x14];
} M037MenuItem;

extern M037MenuItem D_800FF780[];
#define sBarDL ((u8 *) rl_m037_01EB0 + 0x10)            /* 0x01EC0 */
extern u32 rl_m037_01F40[3]; /* .data (still assembly): the steps' colours */
#define sStepColours rl_m037_01F40

#ifdef NON_MATCHING
/* 6 diffs: the ROM loads the cycle's colour bytes green, red, blue (C: red, blue, green) */
/*
 * Draws a volume bar for `value` (0..256, moved by 8 with the stick): a strip coloured along colour
 * cycle 0x39, then up to three steps in sStepColours; returns the new value
 */
s32 func_m037_01ACC(s32 value, s32 arg1, s32 top) {
    s32 x;
    s32 y;
    s32 i;
    s32 n;
    s32 y2;
    s32 m;
    M037ColourCycle cycle;

    y = top;
    x = 160;
    if (D_800FF3E0 < 0) {
        value -= 8;
        if (value < 0) {
            value = 0;
        }
    }
    if (D_800FF3E0 > 0) {
        value += 8;
        if (value > 256) {
            value = 256;
        }
    }
    func_m006_010D8(&x, &y);
    D_800FF780[42].posY = y - 23;
    func_80040D64(&D_800FF398, &D_800FF39C);
    func_8005A0D0(42);
    n = (f32) value * 38.0f * (1.0f / 256.0f);
    func_80055C00(&D_800FF398, NULL, 0, 0);
    gDma1p(D_800FF398++, 7, sBarDL + 0x80000000, 16, 2);
    x -= 18;
    y -= 25;
    func_m006_0107C(&x, &y);
    for (i = 0; i < n; i += 3) {
        colour_cycle_init(&cycle, 0x39);
        update_colour_cycle(&cycle, func_80005064(60) * i / 38);
        gDPSetPrimColor(D_800FF398++, 0, 0, cycle.colour[0], cycle.colour[1], cycle.colour[2], 0xFF);
        gDPFillRectangle(D_800FF398++, x, y - 8, x + 1, y - 3);
        x += 3;
    }
    x = 178;
    y += 16;
    m = (f32) value * 21.0f * (1.0f / 256.0f);
    for (i = 0; i < 3 && m > 0; i++, m -= 7) {
        gDPSetColor(D_800FF398++, G_SETPRIMCOLOR, sStepColours[i]);
        if (m >= 8) {
            y2 = y - 7;
        } else {
            y2 = y - m;
        }
        gDPFillRectangle(D_800FF398++, x + 1, y2, x + 2, y);
        x += 4;
    }
    return value;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m037", func_m037_01ACC);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m037", data);

INCLUDE_ASM("asm/nonmatchings/modules/m037", rodata);
