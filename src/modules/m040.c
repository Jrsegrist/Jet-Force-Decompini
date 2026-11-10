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
static u16 sMarks[4]; /* the players' bonus marks (007F0) */
static s32 sUnk58;
static s32 sUnk5C;

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m040", func_m040_0019C);

void func_m040_007C8(void) {
    menu_assetgroup_free(sAssets);
    D_800FECCC = 0;
}

/* What render_ortho_triangle_image draws an image with */
typedef struct M040Sprite {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 frame;
} M040Sprite;

/* A player's multiplayer scores (D_800FED50, 0xA0 each) */
typedef struct M040Stats {
    /* 0x00 */ u16 kills[4]; /* by victim */
    /* 0x08 */ u8 pad8[0x10 - 0x8];
    /* 0x10 */ s16 score;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ u8 rank; /* 1 to 4 */
    /* 0x15 */ u8 pad15[0x8C - 0x15];
    /* 0x8C */ u8 player; /* the controller */
    /* 0x8D */ u8 pad8D[0x90 - 0x8D];
    /* 0x90 */ s32 time;
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
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity); /* the text colour */
extern char **gMenuText;
extern char rl_m040_02518[]; /* "%d" (.rodata, still assembly) */

extern M040Stats *D_800FF1C0[]; /* the players, sorted */
extern s32 menu_mode_get();
extern char rl_m040_02510[];    /* "%d" (.rodata, still assembly) */
extern char rl_m040_02514[];    /* "+%d" */

/* Slides the table of the players' places in or out, draws it, and counts up their bonus marks (called from 0019C) */
void func_m040_007F0(s32 updateRate) {
    s32 i;
    s32 j;
    s32 rank;
    s32 score;
    s32 x;
    s32 rowY;
    s32 y;
    s32 done;
    s32 blink;
    M040Stats *stats;
    UNUSED u8 pad[0x84];
    M040Sprite sprite;
    char buf[0x104];

    sprite.trans.rotation.z_rotation = 0;
    sprite.trans.rotation.x_rotation = 0;
    sprite.trans.rotation.y_rotation = 0;
    sprite.trans.z_position = 0.0f;
    switch (menu_mode_get()) {
        default:
            sUnk8[sUnk4] = 0x17B;
            break;
        case 0x14:
            sUnk8[sUnk4] = 0x17C;
            break;
        case 0x16:
            sUnk8[sUnk4] = 0x17E;
            break;
    }
    done = FALSE;
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
        } else {
            sMarks[0] = 0;
            sMarks[1] = 0;
            sMarks[2] = 0;
            sMarks[3] = 0;
        }
    }
    func_80055B80(&D_800FF398);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    func_8006FC54(2);
    blink = FALSE;
    rowY = 100;
    if (sMarks[0] < sOrder[0] && sPos[0] == 80 && sPos[1] == 80) {
        sUnk4A -= updateRate;
        if (sUnk4A < 0) {
            sUnk4A += 12;
            func_80001248(0x1C3, 0);
            blink = TRUE;
        }
    }
    for (i = 0; i < D_800A4FD0; i++) {
        stats = D_800FF1C0[i];
        score = stats->score;
        rank = (stats->rank - 1) & 3;
        if (blink && sMarks[i] < sOrder[rank]) {
            sMarks[i]++;
        }
        y = rowY;
        x = sPos[i & 1];
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.y_position = y;
        sprite.frame = rank;
        sprite.trans.scale = 0.75f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x82], 0, 255);
        sprite.trans.x_position += 40.0f;
        sprite.frame = stats->player;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x83], 0, 255);
        func_m006_0107C(&x, &y);
        x += 88;
        func_800639E8(buf, rl_m040_02510, score);
        func_8006FCA8(&D_800FF398, x - 2, rowY, buf, 1);
        func_8006FCA8(&D_800FF398, x + 2, rowY, gMenuText[334], 0);
        x += 42;
        func_800639E8(buf, rl_m040_02514, sMarks[i]);
        func_8006FCA8(&D_800FF398, x, rowY, buf, 0);
        x += 28;
        y = rowY + 5;
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.y_position = y;
        sprite.frame = 0.0f;
        sprite.trans.scale = 1.0;
        for (j = 0; j < sMarks[i]; j++) {
            if (j == sMarks[i] - 1) {
                sprite.frame = 1.0f;
            }
            render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x85], 0, 255);
            sprite.trans.y_position += 4.0f;
        }
        if (!done) {
            if (func_80043510(stats->player) & 0x8000) {
                if (sUnk58 == 0x16) {
                    sUnk5 = 3;
                } else {
                    sUnk5 = 2;
                }
            }
        }
        rowY += 26;
    }
}

/* Slides the table of the players' kills in or out and draws it (called from 0019C) */
void func_m040_00DF8(s32 updateRate) {
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

    sUnk8[sUnk4] = 0x181;
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
    x = sPos[1] + 80;
    y = 84;
    func_m006_010D8(&x, &y);
    sprite.trans.y_position = y;
    for (i = 0; i < D_800A4FD0; i++) {
        sprite.trans.x_position = x;
        sprite.frame = i;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x83], 0, 255);
        x += 30;
    }
    rowY = 110;
    func_8006FC54(2);
    for (i = 0; i < D_800A4FD0; i++) {
        p = &D_800FED50[i];
        y = rowY;
        x = sPos[i & 1] - 12;
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.y_position = y;
        sprite.frame = p->player;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x83], 0, 255);
        func_m006_0107C(&x, &y);
        x += 30;
        func_8006FCA8(&D_800FF398, x, rowY, gMenuText[334], 0);
        x += 60;
        for (j = 0; j < D_800A4FD0; j++) {
            if (i == j) {
                func_8006FC68(255, 64, 0, 255, 255);
            } else {
                func_8006FC68(255, 255, 255, 255, 255);
            }
            func_800639E8(buf, rl_m040_02518, p->kills[j]);
            func_8006FCA8(&D_800FF398, x, rowY, buf, 4);
            x += 30;
        }
        if (!done) {
            if (func_80043510(p->player) & 0x8000) {
                sUnk5 = 3;
            }
            if (func_80043510(p->player) & 0x4000) {
                sUnk5 = 1;
            }
        }
        rowY += 26;
    }
}

extern void func_m006_015EC(s32 time, char *a, char *b, char *c); /* a time as three two-digit strings */
extern char rl_m040_0251C[]; /* ":" (.rodata, still assembly) */
extern char rl_m040_02520[]; /* "." */
extern char rl_m040_02524[]; /* "+%d" */

/* Slides the table of the players' times in or out, draws it, and counts up their bonus marks (called from 0019C) */
void func_m040_0124C(s32 updateRate) {
    s32 i;
    s32 j;
    s32 x;
    s32 rowY;
    s32 y;
    s32 done;
    UNUSED u8 pad[0x88];
    M040Sprite sprite;
    M040Stats *stats;
    char buf[0x100];
    char min[4];
    char min2[4];
    char sec[4];
    char sec2[4];
    char cs[4];
    char cs2[4];

    if (sUnk58 == 0x14) {
        sUnk8[sUnk4] = 0x17C;
    } else {
        sUnk8[sUnk4] = 0x17D;
    }
    sprite.trans.rotation.z_rotation = 0;
    sprite.trans.rotation.x_rotation = 0;
    sprite.trans.rotation.y_rotation = 0;
    done = FALSE;
    sprite.trans.z_position = 0.0f;
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
        } else {
            sMarks[0] = 0;
            sMarks[1] = 0;
            sMarks[2] = 0;
            sMarks[3] = 0;
        }
    }
    func_80055B80(&D_800FF398);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    rowY = 100;
    if (sMarks[0] < sOrder[0] && sPos[0] == 80 && sPos[1] == 80) {
        sUnk4A -= updateRate;
        if (sUnk4A < 0) {
            sUnk4A += 12;
            for (i = 0; i < D_800A4FD0; i++) {
                if (sMarks[i] < sOrder[i]) {
                    sMarks[i]++;
                }
            }
            func_80001248(0x1C3, 0);
        }
    }
    func_8006FC54(2);
    for (i = 0; i < D_800A4FD0; i++) {
        stats = D_800FF1C0[i];
        y = rowY;
        x = sPos[i & 1];
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.y_position = y;
        sprite.frame = i;
        sprite.trans.scale = 0.75f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x82], 0, 255);
        sprite.trans.x_position += 40.0f;
        sprite.frame = stats->player;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x83], 0, 255);
        func_m006_0107C(&x, &y);
        x += 70;
        func_m006_015EC(stats->time, min, sec, cs);
        min2[0] = min[1];
        min2[1] = 0;
        min[1] = 0;
        sec2[0] = sec[1];
        sec2[1] = 0;
        sec[1] = 0;
        cs2[0] = cs[1];
        cs2[1] = 0;
        cs[1] = 0;
        func_8006FCA8(&D_800FF398, x, rowY, min, 4);
        x += 8;
        func_8006FCA8(&D_800FF398, x, rowY, min2, 4);
        x += 6;
        func_8006FCA8(&D_800FF398, x, rowY, rl_m040_0251C, 4);
        x += 6;
        func_8006FCA8(&D_800FF398, x, rowY, sec, 4);
        x += 8;
        func_8006FCA8(&D_800FF398, x, rowY, sec2, 4);
        x += 6;
        func_8006FCA8(&D_800FF398, x, rowY, rl_m040_02520, 4);
        x += 6;
        func_8006FCA8(&D_800FF398, x, rowY, cs, 4);
        x += 8;
        func_8006FCA8(&D_800FF398, x, rowY, cs2, 4);
        x += 34;
        func_800639E8(buf, rl_m040_02524, sMarks[i]);
        func_8006FCA8(&D_800FF398, x, rowY, buf, 0);
        x += 28;
        y = rowY + 5;
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.y_position = y;
        sprite.frame = 0.0f;
        sprite.trans.scale = 1.0;
        for (j = 0; j < sMarks[i]; j++) {
            if (j == sMarks[i] - 1) {
                sprite.frame = 1.0f;
            }
            render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x85], 0, 255);
            sprite.trans.y_position += 4.0f;
        }
        if (!done) {
            if (func_80043510(stats->player) & 0x8000) {
                if (sUnk58 == 0x15) {
                    sUnk5 = 3;
                } else {
                    sUnk5 = 2;
                }
            }
        }
        rowY += 26;
    }
}

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
