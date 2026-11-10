/*
 * Runlink module 40: the multiplayer statistics screen of the front end.
 */
#include "module.h"

#include "gfx.h"

extern s32 D_800FECCC;
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */

static s16 sAssets2[] = { 0x43, 0x44, -1, 0 };
static s16 sImages[] = { 0x1F, 0x2E, -1, 0 };
static s16 sAssets[] = { 0x65, 0x82, 0x83, 0x84, 0x85, 0x86, -1, 0 };
static u32 sColours[] = { 0xFFFFFFFF, 0xDFDFDFFF, 0xBFBFBFFF, 0x9F9F9FFF, 0x7F7F7FFF, 0x5F5F5FFF };
static s8 sOrder[] = { 4, 2, 1, 0 };

/* The start of the .bss (the assembly passes its address to menu_mode_get as a box) */
static s16 sPos[2]; /* where the score table is (0193C slides it) */
static s8 sUnk4;
static s8 sUnk5;     /* set: the table slides out, then becomes sUnk4 */
static s16 sUnk6;
static s16 sUnk8[12]; /* indexed by sUnk4 */
static s16 sUnk20;
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
    menu_assetgroup_free(sAssets);
    D_800FECCC = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_007F0);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_00DF8);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_0124C);

/* What render_ortho_triangle_image draws an image with */
typedef struct M040Sprite {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 frame;
} M040Sprite;

/* A player's multiplayer scores (D_800FED50, 0xA0 each) */
typedef struct M040Stats {
    /* 0x00 */ u8 pad0[0x8C];
    /* 0x8C */ u8 player; /* the controller */
    /* 0x8D */ u8 pad8D[0x94 - 0x8D];
    /* 0x94 */ u16 unk94[4]; /* one per player */
    /* 0x9C */ u8 pad9C[0xA0 - 0x9C];
} M040Stats;

extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern Vertex *D_800FF3A0;
extern void *D_800FF3F8[]; /* the menu's images */
extern u8 D_800A4FD0;      /* the number of players */
extern M040Stats D_800FED50[];
extern char rl_m040_02528[]; /* "%d" (.rodata, still assembly) */
extern void func_80055B80(Gfx **dl);
extern void render_ortho_triangle_image(Gfx **dl, Mtx **mtx, Vertex **vtx, M040Sprite *sprite, void *tex, s32 arg5,
                                        s32 alpha);
extern void func_m006_010D8(s32 *x, s32 *y);
extern void func_m006_0107C(s32 *x, s32 *y);
extern void func_8006FC54(s32 fontId);
extern void func_8006FCA8(Gfx **dl, s32 textX, s32 textY, char *text, s32 flags);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern s32 func_80043510(s32 player);               /* the buttons pressed */

/* Slides the table of the players' scores in or out and draws it (called from 0019C) */
void func_m040_0193C(s32 updateRate) {
    UNUSED s32 pad0[2];
    s32 x;
    s32 rowY;
    s32 y;
    s32 done;
    s32 i;
    s32 j;
    M040Stats *p;
    UNUSED u8 pad[0x7C];
    M040Sprite sprite;
    char buf[0x104];

    sUnk8[sUnk4] = 0x17;
    sprite.trans.rotation.z_rotation = 0;
    sprite.trans.rotation.x_rotation = 0;
    sprite.trans.rotation.y_rotation = 0;
    done = FALSE;
    sprite.trans.z_position = 0.0f;
    sprite.trans.scale = 0.75f;
    if (sUnk5 != 0) {
        sPos[0] -= updateRate * 16;
        sPos[1] += updateRate * 16;
        if (sPos[0] < -220) {
            sUnk4 = sUnk5;
            done = TRUE;
            sUnk5 = 0;
        }
    } else {
        sPos[0] += updateRate * 16;
        if (sPos[0] > 80) {
            sPos[0] = 80;
        }
        sPos[1] -= updateRate * 16;
        if (sPos[1] < 80) {
            sPos[1] = 80;
        }
    }
    func_80055B80(&D_800FF398);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    x = sPos[1] + 41;
    y = 84;
    func_m006_010D8(&x, &y);
    sprite.trans.y_position = y;
    for (i = 0; i < D_800A4FD0; i++) {
        sprite.trans.x_position = x;
        sprite.frame = i;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x82], 0, 255);
        x += 40;
    }
    rowY = 110;
    func_8006FC54(2);
    for (i = 0; i < D_800A4FD0; i++) {
        p = &D_800FED50[i];
        x = sPos[i & 1];
        y = rowY;
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.y_position = y;
        sprite.frame = p->player;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x83], 0, 255);
        func_m006_0107C(&x, &y);
        x += 40;
        for (j = 0; j < D_800A4FD0; j++) {
            func_800639E8(buf, rl_m040_02528, p->unk94[j]);
            func_8006FCA8(&D_800FF398, x, rowY - 6, buf, 4);
            x += 40;
        }
        if (!done) {
            if (func_80043510(p->player) & 0x8000) {
                sUnk5 = 4;
            }
            if (func_80043510(p->player) & 0x4000) {
                if (sUnk58 == 0x15 || sUnk58 == 0x16) {
                    sUnk5 = 1;
                } else {
                    sUnk5 = 2;
                }
            }
        }
        rowY += 26;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_01D60);

s32 *func_m040_024C0(void) {
    return sTiltList;
}

INCLUDE_ASM("asm/nonmatchings/modules/m040", rodata);
