/*
 * Runlink module 14: part of the HUD (still mostly assembly).
 */
#include "module.h"
#include "gfx.h"

extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */
extern void func_m006_00EFC();
extern void *D_800FF3F8[]; /* the loaded menu images */
extern void menu_item_free(s32 image);

extern void menu_assetgroup_load(void *images); /* loads a list of images */
extern void menu_assetgroup_reset(void *images);
extern s32 func_m013_00000(void);
extern void func_8004DDAC(s32 arg0, s32 arg1, s32 arg2);
extern void func_m006_00C74(void);
extern void func_m006_01134(void);
extern u8 D_800FF310;
static void func_m014_010D8(void);

/* The module's .data (still assembly) */
extern u8 rl_m014_04440[]; /* images */
extern u8 rl_m014_04460[];

/* The module's .bss (still assembly) */
extern s32 rl_m014_04890;
extern s32 rl_m014_0489C;
extern s16 rl_m014_04894;
extern s16 rl_m014_04896;
extern s16 rl_m014_04898;

void func_m014_00000(void) {
    func_m013_00000();
    func_8004DDAC(0, 0, 0);
    menu_assetgroup_load(rl_m014_04440);
    menu_assetgroup_reset(rl_m014_04460);
    func_m014_010D8();
    func_m006_00C74();
    func_m006_01134();
    rl_m014_04894 = 0;
    rl_m014_04896 = 0;
    rl_m014_04898 = 0;
}

/* The part of the player's state that 00074 reads */
typedef struct M014Player {
    /* 0x000 */ u8 pad0[0x1CA];
    /* 0x1CA */ s16 yaw;
    /* 0x1CC */ u8 pad1CC[0x540 - 0x1CC];
    /* 0x540 */ u16 unk540;
} M014Player;

extern Gfx *D_800FF398;
extern void func_80055B80(Gfx **dl);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3); /* sets a texture */
extern s32 rl_m014_04528;     /* .data (still assembly): the arrow's blink timer */
extern u8 rl_m014_044D8[];    /* its triangle */
extern u8 rl_m014_044E8[];    /* its vertices, pointing left */
extern u8 rl_m014_04508[];    /* and right */

/* Blinks an arrow to the left or right when `angle` is more than 0xA00 off the player's yaw (called from
   00B30) */
void func_m014_00074(s16 *angle, M014Player *p, s32 updateRate) {
    rl_m014_04528 += updateRate;
    if ((p->unk540 & 0xF) && (rl_m014_04528 & 0x10)) {
        s16 d = *angle - p->yaw;

        if (d < -0xA00 || d > 0xA00) {
            func_80055B80(&D_800FF398);
            func_80055C00(&D_800FF398, NULL, 7, 0);
            gDPSetEnvColor(D_800FF398++, 0xFF, 0xFF, 0xFF, 0x00);
            gDPSetPrimColor(D_800FF398++, 0, 0, 0xFF, 0xFF, 0xFF, 0x80);
            if (d < -0x800) {
                gSPVertexJFG(D_800FF398++, rl_m014_044E8 + 0x80000000, 4, 0);
            } else {
                gSPVertexJFG(D_800FF398++, rl_m014_04508 + 0x80000000, 4, 0);
            }
            gSPPolygon(D_800FF398++, rl_m014_044D8 + 0x80000000, 1, 0);
            gDPPipeSync(D_800FF398++);
            gDPSetPrimColor(D_800FF398++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00220);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00B30);

static void func_m014_010D8(void) {
    rl_m014_0489C = 0;
    rl_m014_04890 = 0;
    D_800FF310 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_010F4);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_01328);

/* Frees the images 10 to 25 */
static void free_images(void) {
    s32 i;

    for (i = 0; i < 16; i++) {
        if (D_800FF3F8[i + 10] != NULL) {
            menu_item_free(i + 10);
        }
    }
}

void func_m014_019E4(void) {
    rl_m014_04894 = 0xF0;
}

/* A menu item (D_800FF780, size 0x20; the main segment's MenuItem), as used here */
typedef struct M014MenuItem {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 posX;
    /* 0x10 */ u8 pad10[0x20 - 0x10];
} M014MenuItem;

extern M014MenuItem D_800FF780[];

/* Slides menu item 6 in while 019E4's timer runs, and out again after */
void func_m014_019F4(s32 updateRate) {
    if (rl_m014_04894 != 0) {
        if (D_800FF780[6].posX < 69.0f) {
            D_800FF780[6].posX += updateRate * 8;
            if (D_800FF780[6].posX > 69.0f) {
                D_800FF780[6].posX = 69.0f;
            }
        }
        rl_m014_04894 -= updateRate;
        if (rl_m014_04894 < 0) {
            rl_m014_04894 = 0;
        }
    } else if (D_800FF780[6].posX > -91.0f) {
        D_800FF780[6].posX -= updateRate * 8;
        if (D_800FF780[6].posX < -91.0f) {
            D_800FF780[6].posX = -91.0f;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_01AD8);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_0292C);

void func_m014_02CC0(void) {
    menu_assetgroup_free(rl_m014_04440);
    free_images();
    func_m006_00EFC();
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_02CF4);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_03418);

/* A level's best result (size 8) */
typedef struct M014Record {
    /* 0x0 */ u8 score;
    /* 0x1 */ u8 unk1;
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u32 time;
} M014Record;

/* The saved game, as far as used here */
typedef struct M014Save {
    /* 0x000 */ u8 pad0[3];
    /* 0x003 */ s8 unk3;
    /* 0x004 */ u8 score;
    /* 0x005 */ u8 unk5;
    /* 0x006 */ u8 pad6[0x334 - 0x6];
    /* 0x334 */ M014Record records[32];
    /* 0x434 */ u8 pad434[0x474 - 0x434];
    /* 0x474 */ u32 unk474[16];
    /* 0x4B4 */ u32 unk4B4[16];
    /* 0x4F4 */ u16 unk4F4[16];
    /* 0x514 */ u32 time;
} M014Save;

/* What func_8004470C returns, as used here */
typedef struct M014Level {
    /* 0x000 */ u8 pad0[0x102];
    /* 0x102 */ u8 record;
} M014Level;

extern M014Save *get_settings(void);
extern M014Level *func_8004470C(void);
extern s32 game_flag_get(s32 flag);
extern void func_80047EFC(s32 flag, s32 value);
extern void func_80000510();
extern void func_8004842C(s32 arg0);
extern s8 *D_800FD7B8;

/* .bss, still assembly */
extern s16 rl_m014_04896;
extern s32 rl_m014_048A0;
extern s32 rl_m014_048A4;
extern s32 rl_m014_048A8;
extern s32 rl_m014_048AC;
extern s32 rl_m014_048B0;

/* At the end of a level: finds the best of the stats and updates the level's record */
void func_m014_037F4(void) {
    M014Level *level;
    M014Record *rec;
    s32 i;
    s32 changed;
    s32 maxA;
    u32 maxB;
    f32 best;
    f32 ratio;
    s32 sum;
    M014Save *save;

    save = get_settings();
    level = func_8004470C();
    rl_m014_04896 = 1;
    best = 0.0f;
    maxA = 0;
    maxB = 0;
    rl_m014_048A0 = -1;
    rl_m014_048A4 = -1;
    rl_m014_048A8 = -1;
    sum = 0;
    for (i = 0; i < 16; i++) {
        if (maxB < save->unk474[i]) {
            maxB = save->unk474[i];
            rl_m014_048A8 = i;
        }
        if (save->unk4B4[i] != 0) {
            ratio = (f32) save->unk4B4[i] / (f32) save->unk474[i];
            if (best < ratio) {
                best = ratio;
                rl_m014_048A4 = i;
            }
        }
        if (maxA < save->unk4F4[i]) {
            maxA = save->unk4F4[i];
            rl_m014_048A0 = i;
        }
        sum += save->unk4F4[i];
    }
    changed = 0;
    rec = &save->records[level->record];
    if (rec->unk2 < sum) {
        rec->unk2 = sum;
        changed = 1;
    }
    rl_m014_048AC = 0;
    rl_m014_048B0 = 0;
    if (rec->score < save->score) {
        s32 total;
        s32 target;

        rec->score = save->score;
        changed = 1;
        rec->unk1 = save->unk5;
        rec->time = save->time;
        rl_m014_048AC = 1;
        target = 0;
        total = 0;
        for (i = 0; i < 31; i++) {
            target += D_800FD7B8[i];
            total += save->records[i].score;
        }
        if (target == total && !game_flag_get(0x58)) {
            func_80047EFC(0x58, 1);
        }
    } else if (save->time < rec->time && save->score >= rec->score) {
        rec->time = save->time;
        rl_m014_048B0 = 1;
        changed = 1;
    }
    if (changed) {
        func_80000510(0x2A);
        func_8004842C(save->unk3);
    } else {
        func_80000510(0x2A);
    }
}

extern char **gMenuText; /* the game's texts */
extern s32 func_800706D4(char *text, s32 x, s32 font); /* DKR get_text_width */
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void func_m157_00000(Gfx **dl, u8 shape, s32 x, s32 y, s32 width, s32 height, u32 colour);
extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern char *get_world_name(s32 world);
extern char rl_m014_0487C[]; /* "%d %s\n" */

/* Draws a box with the region's name, its world's name and a numbered line under it */
void func_m014_03AD4(Gfx **dl, s32 region, s32 world, s32 number, s32 alpha) {
    char buf[32];
    char *name;
    s32 width;
    s32 width2;

    name = gMenuText[region + 407];
    width = func_800706D4(name, 2, 1) + 40;
    func_800639E8(buf, rl_m014_0487C, number, gMenuText[458]);
    width2 = func_800706D4(buf, 2, 1) + 40;
    if (width < width2) {
        width = width2;
    }
    func_m157_00000(dl, 0, 160 - (width >> 1), 51, width, 46, (alpha * 164) >> 8);
    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    func_8006FC68(0, 192, 192, 255, alpha);
    func_8006FCA8(dl, 159, 61, name, 12);
    func_8006FCA8(dl, 159, 74, get_world_name(world), 12);
    func_8006FC68(192, 192, 192, 255, alpha);
    func_8006FCA8(dl, 159, 87, buf, 12);
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_03C68);

INCLUDE_ASM("asm/nonmatchings/modules/m014", data);

INCLUDE_ASM("asm/nonmatchings/modules/m014", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m014", bss);

