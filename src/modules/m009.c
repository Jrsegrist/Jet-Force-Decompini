/*
 * Runlink module 9: a front-end map screen (still mostly assembly).
 */
#include "module.h"

extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */

extern void menu_assetgroup_load(void *images); /* loads a list of images */
extern void menu_assetgroup_reset(void *images);
extern void func_8005342C(s32 module); /* unloads a runlink module */
extern void func_m005_00010(void);
extern void rl_m009_03D0C(void *icon);
extern s32 D_800FECCC;

/* A world's icon on the map (size 0x30) */
typedef struct M009Icon {
    /* 0x00 */ u8 pad0[0x30];
} M009Icon;

/* The module's .data (still assembly) */
extern u8 rl_m009_043F8[];
extern u8 rl_m009_04400[]; /* the map's icons */
extern u8 rl_m009_04424[];
extern M009Icon rl_m009_0448C[11];
extern s32 rl_m009_04908;

/* The module's .bss (still assembly) */
extern s32 rl_m009_097B0;
extern s32 rl_m009_097B4;

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00000);

void func_m009_003A8(void) {
    func_m005_00010();
    if (rl_m009_097B0) {
        func_8005342C(5);
    }
    menu_assetgroup_free(rl_m009_043F8);
    menu_assetgroup_free(rl_m009_04400);
    D_800FECCC = 0;
    rl_m009_04908 = 0;
}

void func_m009_00408(void) {
    s32 i;
    M009Icon *icon;

    menu_assetgroup_load(rl_m009_04400);
    menu_assetgroup_reset(rl_m009_04424);
    rl_m009_097B4 = 0;
    icon = rl_m009_0448C;
    for (i = 0; i < 11; i++, icon++) {
        rl_m009_03D0C(icon);
    }
}

/* A menu item (D_800FF780, size 0x20), as used here */
typedef struct M009MenuItem {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ f32 posX;
    /* 0x10 */ f32 posY;
    /* 0x14 */ u8 pad14[0x20 - 0x14];
} M009MenuItem;

/* A place on the map (size 0xC; a kind of 0xFF ends the list) */
typedef struct M009Place {
    /* 0x0 */ u8 kind;  /* 0: a world model, else a menu item */
    /* 0x1 */ u8 index;
    /* 0x2 */ u8 world;
    /* 0x3 */ u8 pad3[0xC - 0x3];
} M009Place;

/* A world model drawn on the map (size 0x4E0) */
typedef struct M009Model {
    /* 0x000 */ u8 pad0[0x4CC];
    /* 0x4CC */ s16 x;
    /* 0x4CE */ s16 y;
    /* 0x4D0 */ u8 pad4D0[0x4E0 - 0x4D0];
} M009Model;

extern M009MenuItem D_800FF780[];
extern Gfx *D_800FF398;
extern void *D_800FF39C;
extern void func_80040D64(Gfx **dl, void *mtx);
extern void rl_m009_02978(M009Model *model, s32 updateRate);
extern void rl_m009_030E8(M009Model *model);
extern void rl_m009_03908(s32 index);
extern M009Place rl_m009_0469C[]; /* (.data) */
extern M009Model rl_m009_04990[]; /* (.bss) */
extern f32 rl_m009_09790;
extern f32 rl_m009_09794;

/* Draws the icon of a world on the map at (x, y) */
void func_m009_00478(s32 world, s32 x, s32 y, s32 updateRate) {
    M009Place *p;
    s32 found;
    M009Model *model;

    rl_m009_09790 = 0.0f;
    p = rl_m009_0469C;
    rl_m009_09794 = 0.0f;
    found = 0;
    if (p->kind != 0xFF) {
        do {
            if (world == p->world) {
                found = 1;
                if (p->kind == 0) {
                    model = &rl_m009_04990[p->index];
                    model->x = x;
                    model->y = y;
                    func_80040D64(&D_800FF398, &D_800FF39C);
                    rl_m009_02978(model, updateRate);
                    rl_m009_030E8(model);
                } else {
                    D_800FF780[p->index].posX = x;
                    D_800FF780[p->index].posY = y;
                    D_800FF780[16].x += updateRate * 256;
                    D_800FF780[17].x += updateRate << 7;
                    D_800FF780[44].x += updateRate << 7;
                    D_800FF780[18].x -= updateRate * 160;
                    D_800FF780[14].x += updateRate << 8;
                    D_800FF780[20].x += updateRate << 8;
                    D_800FF780[43].x += updateRate << 8;
                    D_800FF780[43].y += updateRate << 5;
                    D_800FF780[15].x += updateRate << 8;
                    rl_m009_03908(p->index);
                }
            }
            p++;
        } while (p->kind != 0xFF && !found);
    }
}

void func_m009_0066C(void) {
    menu_assetgroup_free(rl_m009_04400);
}

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00690);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02880);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02978);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02FB0);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_030E8);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_0343C);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_037E4);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_03908);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_039F0);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_03D0C);

INCLUDE_ASM("asm/nonmatchings/modules/m009", data);

INCLUDE_ASM("asm/nonmatchings/modules/m009", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m009", bss);

