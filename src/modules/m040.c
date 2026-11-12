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
static s16 sUnk8[8]; /* per view (sUnk4): the menu text shown as its title, -1: none */
static u8 sAlpha[8]; /* per view: the title's opacity */
static s16 sUnk20;   /* frames before the screen takes input */
static s16 sTiltList[12]; /* the rotation of the model 0019C draws (func_m040_024C0) */
static s32 sTurn[1];   /* the model's turn */
static s32 sTurnTo;    /* the turn it heads for */
static s8 sFrame[1];   /* the picture on the model */
static s8 sFrameNext;  /* the one it changes to when it turns */
static s16 sUnk4A;
static s16 sUnk4C;
static s16 sUnk4E;
static u16 sMarks[4]; /* the players' bonus marks (007F0) */
static s32 sUnk58;
static s32 sUnk5C;

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
    /* 0x94 */ u16 unk94[4]; /* by rank */
    /* 0x9C */ u16 bonus; /* the bonus marks earned (up to 50000) */
    /* 0x9E */ u8 pad9E[0xA0 - 0x9E];
} M040Stats;

/* The parts of the model 0019C draws that it animates */
typedef struct M040Part {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 unk16;
} M040Part;
typedef struct M040PartRef {
    /* 0x0 */ M040Part *part;
    /* 0x4 */ u8 pad4[4];
} M040PartRef;
typedef struct M040Model {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M040PartRef *parts;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ u8 attachCount;
} M040Model;
typedef struct M040Attach {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2; /* < 0: none */
    /* 0x4 */ s32 kind; /* the low byte: the part */
} M040Attach;
typedef struct M040Instance {
    /* 0x00 */ M040Model *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ u8 padC[0x74 - 0xC];
    /* 0x74 */ M040Attach *attach;
    /* 0x78 */ s16 *unk78[2]; /* indexed by unkB */
} M040Instance;

extern void *D_800FF3F8[];      /* the menu's images */
extern u8 D_800A4FD0;           /* the number of players */
extern M040Stats *D_800FF1C0[]; /* the players, sorted */
extern s32 D_800FECCC;
extern void menu_assetgroup_load(s16 *assets);
extern void menu_assetgroup_reset(s16 *assets);
extern s32 menu_mode_get();
extern void func_m061_00F50(void);
extern void func_m061_010D4(s32 arg0);

void func_m040_00000(void) {
    s32 i;
    M040Stats *stats;
    s32 rank;
    s32 bonus;

    menu_assetgroup_load(sAssets);
    menu_assetgroup_reset(sImages);
    menu_assetgroup_load(sAssets2);
    sUnk8[0] = -1;
    sUnk8[5] = -1;
    sUnk4 = 1;
    sUnk5 = 0;
    sPos[0] = -220;
    sPos[1] = 380;
    sUnk20 = 30;
    switch (menu_mode_get()) {
        case 0x12:
        case 0x13:
            func_m061_00F50();
            break;
        case 0x14:
            func_m061_010D4(4);
            break;
        case 0x15:
            func_m061_010D4(4);
            break;
        case 0x16:
            func_m061_010D4(4);
            break;
    }
    for (i = 0; i < D_800A4FD0; i++) {
        stats = D_800FF1C0[i];
        rank = (stats->rank - 1) & 3;
        bonus = sOrder[rank] + stats->bonus;
        if (bonus > 50000) {
            bonus = 50000;
        }
        stats->bonus = bonus;
        D_800FF1C0[i]->unk94[rank]++;
    }
    obj_anim_set(D_800FF3F8[0x84], 0, 1.0f);
    D_800FECCC = 1;
}

extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern char **gMenuText;
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_m038_00000(void);
extern void func_80055B80(Gfx **dl);
extern void set_active_camera(s32 camera);
extern void viewport_apply(Gfx **dl, s32 arg1);
extern void cam_fov_scale_set(f32 x, f32 y); /* scales the field of view */
extern void func_8003F7F4(f32 fov, s32 arg1); /* sets the field of view */
extern void func_80040E64(Gfx **dl, Mtx **mtx);
extern void func_8005A0D0(s32 item);          /* draws a menu item */
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity); /* the text colour */
extern void func_8006FCA8(Gfx **dl, s32 textX, s32 textY, char *text, s32 flags);
extern void func_8004665C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
static void func_m040_007F0(s32 updateRate);
static void func_m040_00DF8(s32 updateRate);
static void func_m040_0124C(s32 updateRate);
static void func_m040_0193C(s32 updateRate);
static void func_m040_01D60(s32 updateRate);

/* The statistics screen's frame: runs the view shown, turns the model to it and fades the view titles */
void func_m040_0019C(s32 updateRate) {
    UNUSED s32 pad;
    s32 quit;
    M040Instance *inst;
    M040Model *model;
    M040Attach *a;
    M040Part *part;
    Object *obj;
    s16 *out;
    s32 i;
    s32 step;
    s32 prev;
    s32 oldView;
    s32 oldNext;

    quit = FALSE;
    func_80040D64(&D_800FF398, &D_800FF39C);
    func_m038_00000();
    func_80055B80(&D_800FF398);
    sUnk58 = menu_mode_get();
    if (sUnk20 >= 0) {
        sUnk20 -= updateRate;
        if (sUnk20 < 0) {
            func_80001248(0x1C4, 0);
        }
    } else {
        oldView = sUnk4;
        oldNext = sUnk5;
        switch (sUnk4) {
            case 1:
                switch (sUnk58) {
                    case 0x12:
                    case 0x13:
                        sFrameNext = 0;
                        func_m061_00F50();
                        func_m040_007F0(updateRate);
                        break;
                    case 0x14:
                        sFrameNext = 0;
                        func_m061_010D4(4);
                        func_m040_0124C(updateRate);
                        break;
                    case 0x15:
                        sFrameNext = 1;
                        func_m061_010D4(4);
                        func_m040_0124C(updateRate);
                        break;
                    case 0x16:
                        sFrameNext = 2;
                        func_m061_010D4(4);
                        func_m040_007F0(updateRate);
                        break;
                }
                break;
            case 2:
                sFrameNext = 0;
                func_m040_00DF8(updateRate);
                break;
            case 3:
                sFrameNext = 4;
                func_m040_0193C(updateRate);
                break;
            case 4:
                sFrameNext = 6;
                func_m061_010D4(3);
                func_m040_01D60(updateRate);
                if (sUnk4 >= 5) {
                    quit = TRUE;
                }
                break;
        }
        if (sUnk5 != 0 && oldNext != sUnk5) {
            func_80001248(0x1C4, 0);
        }
        if (oldView != sUnk4) {
            func_80001248(0x1C4, 0);
            sUnk4A = 12;
            sTurnTo = 0x10000;
            if (sTurn[0] >= 0xB700) {
                sTurn[0] -= 0x10000;
            }
        }
    }
    prev = sTurn[0];
    sTurn[0] += (sTurnTo - sTurn[0]) >> 3;
    if (sTurn[0] >= 0xB700 && prev < 0xB700) {
        sFrame[0] = sFrameNext;
    }
    set_active_camera(0);
    viewport_apply(&D_800FF398, 0);
    cam_fov_scale_set(1.0f, 1.0f);
    func_8003F7F4(52.0f, 1);
    func_80040E64(&D_800FF398, &D_800FF39C);
    sTiltList[0] = 0x2006;
    sTiltList[1] = sTurn[0];
    sTiltList[2] = 0x1000;
    obj = D_800FF3F8[0x84];
    func_8001138C(obj, -0.02f, updateRate);
    inst = *(M040Instance **) obj->modelInstances;
    model = inst->model;
    inst->unkA = updateRate;
    a = inst->attach;
    out = inst->unk78[inst->unkB];
    for (i = 0; i < model->attachCount; i++, a++) {
        step = a->kind & 0xFF;
        part = model->parts[step].part;
        if (step == 4) {
            a->unk0 = sFrame[0] << 8;
        }
        *out++ = (a->unk0 >> 8) * part->unk16;
        if (a->unk2 >= 0) {
            *out++ = (a->unk2 >> 8) * part->unk16;
        }
    }
    func_8005A0D0(0x2E);
    func_8005A0D0(0x1F);
    func_8003F7F4(52.0f, 1);
    /* the fade step written out at each use (a `step` local changes the temps) */
    for (i = 0; i < 8 && sUnk20 < 0; i++) {
        if (i == sUnk4) {
            if (sAlpha[i] < 255 - updateRate * 8) {
                sAlpha[i] += updateRate * 8;
            } else {
                sAlpha[i] = 255;
            }
        } else {
            if (sAlpha[i] < updateRate * 8) {
                sAlpha[i] = 0;
            } else {
                sAlpha[i] -= updateRate * 8;
            }
        }
        if (sUnk8[i] != -1 && sAlpha[i] > 0) {
            func_8006FC68(0, 0, 128, 255, sAlpha[i]);
            func_8006FCA8(&D_800FF398, 125, 49, gMenuText[sUnk8[i]], 0);
        }
    }
    func_8006FC68(255, 255, 255, 255, 255);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    if (quit) {
        func_8004665C(0, 0, 0, 8, 1, 0);
    }
}

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

extern Vertex *D_800FF3A0;
extern M040Stats D_800FED50[];
extern void render_ortho_triangle_image(Gfx **dl, Mtx **mtx, Vertex **vtx, M040Sprite *sprite, void *tex, s32 arg5,
                                        s32 alpha);
extern void func_m006_010D8(s32 *x, s32 *y);
extern void func_m006_0107C(s32 *x, s32 *y);
extern void func_8006FC54(s32 fontId);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern s32 func_80043510(s32 player);               /* the buttons pressed */

/* Slides the table of the players' places in or out, draws it, and counts up their bonus marks (called from 0019C) */
static void func_m040_007F0(s32 updateRate) {
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
        func_800639E8(buf, "%d", score);
        func_8006FCA8(&D_800FF398, x - 2, rowY, buf, 1);
        func_8006FCA8(&D_800FF398, x + 2, rowY, gMenuText[334], 0);
        x += 42;
        func_800639E8(buf, "+%d", sMarks[i]);
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
static void func_m040_00DF8(s32 updateRate) {
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
            func_800639E8(buf, "%d", p->kills[j]);
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

/* Slides the table of the players' times in or out, draws it, and counts up their bonus marks (called from 0019C) */
static void func_m040_0124C(s32 updateRate) {
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
        func_8006FCA8(&D_800FF398, x, rowY, ":", 4);
        x += 6;
        func_8006FCA8(&D_800FF398, x, rowY, sec, 4);
        x += 8;
        func_8006FCA8(&D_800FF398, x, rowY, sec2, 4);
        x += 6;
        func_8006FCA8(&D_800FF398, x, rowY, ".", 4);
        x += 6;
        func_8006FCA8(&D_800FF398, x, rowY, cs, 4);
        x += 8;
        func_8006FCA8(&D_800FF398, x, rowY, cs2, 4);
        x += 34;
        func_800639E8(buf, "+%d", sMarks[i]);
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
static void func_m040_0193C(s32 updateRate) {
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
            func_800639E8(buf, "%d", p->unk94[j]);
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


/* Slides the table of the players' bonus marks in or out and draws them in rows of four (called from 0019C) */
static void func_m040_01D60(s32 updateRate) {
    s32 i;
    s32 j;
    s32 x;
    s32 rowY;
    s32 y;
    s32 place;
    s32 done;
    s32 rows;
    s32 count;
    s32 maxed;
    u32 *colour;
    UNUSED u8 pad[0x84];
    M040Sprite sprite;
    M040Stats *stats;
    char buf[0x100];

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
        } else {
            sMarks[0] = 0;
            sMarks[1] = 0;
            sMarks[2] = 0;
            sMarks[3] = 0;
        }
    }
    maxed = FALSE;
    if (sMarks[0] < D_800FF1C0[0]->bonus && sPos[0] == 80 && sPos[1] == 80) {
        sUnk4A -= updateRate;
        if (sUnk4A < 0) {
            sUnk4A += 7;
            for (i = 0; i < D_800A4FD0; i++) {
                if (sMarks[i] < D_800FF1C0[i]->bonus) {
                    sMarks[i]++;
                    if (sMarks[i] > 20) {
                        maxed = TRUE;
                        sMarks[i] = D_800FF1C0[i]->bonus;
                    }
                }
            }
            func_80001248(0x1C3, 0);
        }
        if (maxed) {
            func_80001248(0x47, 0);
        }
    }
    func_80055B80(&D_800FF398);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    func_8006FC54(2);
    rowY = 100;
    place = 0;
    for (i = 0; i < D_800A4FD0; rowY += 26) {
        y = rowY;
        stats = D_800FF1C0[i];
        x = sPos[i & 1] + 30;
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.y_position = y;
        sprite.frame = place;
        sprite.trans.scale = 0.75f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x82], 0, 255);
        x += 30;
        sprite.trans.x_position = x;
        sprite.frame = stats->player;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x83], 0, 255);
        func_m006_0107C(&x, &y);
        x += 15;
        func_800639E8(buf, "%d", sMarks[i]);
        func_8006FCA8(&D_800FF398, x, rowY, buf, 0);
        rows = stats->bonus / 4;
        if (stats->bonus % 4) {
            rows++;
        }
        if (rows >= 6) {
            rows = 5;
            x += 35;
        }
        count = sMarks[i];
        x += rows * 9 + 21;
        y = rowY + 5;
        func_m006_010D8(&x, &y);
        sprite.trans.x_position = x;
        sprite.trans.scale = 1.0;
        sprite.trans.y_position = y;
        if (rows != 0) {
            colour = &sColours[rows];
            do {
                gDPSetColor(D_800FF398++, G_SETPRIMCOLOR, *colour);
                j = 0;
                sprite.frame = 0.0f;
                sprite.trans.y_position = y;
                if (count > 0) {
                    do {
                        if (j == 3 || count == 1) {
                            sprite.frame = 1.0f;
                        }
                        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x85],
                                                    0, 255);
                        j++;
                        sprite.trans.y_position += 4.0f;
                        count--;
                    } while (j < 4 && count > 0);
                }
                sprite.trans.x_position -= 9.0f;
                colour--;
            } while (colour != sColours);
        }
        if (sMarks[i] > 20) {
            sprite.trans.y_position -= 10.0f;
            sprite.trans.x_position -= 4.0f;
            sprite.trans.scale = 0.87f;
            render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x86], 0, 255);
        }
        if (!done) {
            if (func_80043510(stats->player) & 0x8000) {
                sUnk5 = 5;
            }
            if (func_80043510(stats->player) & 0x4000) {
                sUnk5 = 3;
            }
        }
        i++;
        if (i < D_800A4FD0 && stats->bonus != D_800FF1C0[i]->bonus) {
            place = i;
        }
    }
}

s16 *func_m040_024C0(void) {
    return sTiltList;
}

