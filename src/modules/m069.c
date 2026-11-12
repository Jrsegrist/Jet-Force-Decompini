/*
 * Runlink module 69: a target-range minigame. It spawns one target object per
 * player, draws each player's crosshair and the score lines (shots, ants,
 * bears and ammo), and at the end a banner in the winner's colour.
 */
#include "module.h"
#include "gfx.h"

/* A level entry for spawning a target (size 0x12) */
typedef struct M069Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ u8 player;
    /* 0x11 */ u8 unk11;
} M069Entry;

/* A target's state (its object's unk68) */
typedef struct M069Score {
    /* 0x00 */ u8 player;
    /* 0x01 */ u8 pad1;
    /* 0x02 */ u16 shots;
    /* 0x04 */ u16 bears;
    /* 0x06 */ u16 ants;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ s16 ammo; /* < 0: none */
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ f32 x; /* the crosshair, on screen */
    /* 0x14 */ f32 y;
} M069Score;

/* A text position */
typedef struct M069Point {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} M069Point;

extern Object *func_80005F14(M069Entry *entry, s32 count); /* DKR spawn_object */
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void screen_line_add_in_window(s16 x0, s16 y0, s16 x1, s16 y1, s32 colour, s32 arg5); /* draws a line */
extern void func_80055C00(Gfx **dList, void *tex, s32 flags, s32 texOffset);
extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern void func_80054EA0(s32 *width, s32 *height); /* the frame buffer size */
extern void func_8006F550(Gfx **dl, s32 image, s32 width, s32 height, s32 x, s32 y, s32 w, s32 h, u32 colour);
extern void func_8004F02C(Gfx **dl, void *image, f32 x, f32 y, f32 xScale, f32 yScale, u32 colour, s32 flags);
extern void menu_fill_box(Gfx **dl, s32 x, s32 y, s32 w, s32 h, s32 arg5);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */

extern s32 D_800A4FDC;
extern s32 gMenuTimer;
extern u8 D_800A4FEC;  /* the game is over */
extern char **gMenuText; /* the game's texts */
extern s32 gVideoLastFramebuffer;
extern u16 D_800FF1B8;

/* .data, still assembly */
extern u32 rl_m069_01D48[4];          /* the players' colours (RGBA) */
extern M069Point *rl_m069_01DD0[4];   /* the score lines' positions, by the number of players */
extern u32 rl_m069_01EF8[4];          /* the crosshair colours */
extern s8 rl_m069_01F08[14][5];       /* the crosshair lines: x0, y0, x1, y1, colour */
extern s8 rl_m069_01F50[12][2];       /* the crosshair vertices */
extern Triangle rl_m069_01F68[4];     /* the crosshair triangles */

/* .rodata, still assembly */

/* .bss, still assembly */
extern Object *rl_m069_02000[4]; /* the targets */
extern s32 rl_m069_02010;        /* the winner, or < 0 */
extern s32 rl_m069_02014;        /* the banner's opacity */

/* Spawns one target per player */
void func_m069_00000(s32 count) {
    s32 i;
    M069Entry entry;

    i = 0;
    while (i < count) {
        entry.objectID = 0x23E;
        entry.size = sizeof(M069Entry);
        entry.x = 0;
        entry.y = 0;
        entry.z = 0;
        entry.unkA = 0;
        entry.unkC = 0;
        entry.unkE = 0;
        entry.player = i;
        entry.unk11 = 0;
        rl_m069_02000[i] = func_80005F14(&entry, 1);
        i++;
    }
    gMenuTimer = D_800A4FDC;
}

/* The results of a player (D_800FED50, size 0xA0), as used here */
typedef struct M069Player {
    /* 0x00 */ u16 ants;
    /* 0x02 */ u8 pad2[6];
    /* 0x08 */ u16 bears;
    /* 0x0A */ u8 padA[6];
    /* 0x10 */ s16 points; /* ants - bears */
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ u8 rank;
    /* 0x15 */ u8 pad15[0x8C - 0x15];
    /* 0x8C */ u8 player;
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ u32 accuracy; /* ants per 100 shots */
    /* 0x94 */ u8 pad94[0xC];
} M069Player;

extern M069Player D_800FED50[4];
extern M069Player *D_800FF1C0[4]; /* the players' results, best first */
extern ObjectTransform *func_80041DE0(void); /* the camera */
extern void func_80079890(s32 arg0, ObjectTransform *cam);
extern void *func_80055694(s32 id); /* loads a texture */
extern void func_80055A00(void *tex); /* frees it */
extern void func_800496B0(s32 arg0, s16 *angles, f32 *in, f32 *out);
extern s16 func_80015510(f32 x, f32 y, f32 z); /* the level segment at a position */
extern s8 input_stick_x_raw(s32 player); /* the stick, x */
extern s8 input_stick_y_raw(s32 player); /* the stick, y */
extern u16 func_800434E4(s32 player); /* the buttons held */
extern s32 func_80043510(s32 player); /* the buttons pressed */
extern void music_volume_fade(f32 volume, s32 arg1);
extern void func_80047608(s32 arg0, s32 arg1, s32 arg2);
extern void rl_m069_00A84(Object *obj, M069Score *score, s32 x, s32 y, s32 held, s32 pressed, s32 updateRate);
extern f32 *rl_m069_01D38[4]; /* the targets' positions in front of the camera, by the number of players */
extern void *rl_m069_01DE4;   /* the banner image */
extern s16 rl_m069_01DFC[4];  /* the banner images, by winner */

/* Runs the game: moves the targets with the camera, then the results, the banner and the way out */
void func_m069_000A8(s32 count, s32 updateRate) {
    s32 i;
    s32 rank;
    s32 sorted;
    s16 angles[3];
    f32 *offsets;
    M069Score *score;
    M069Player *a;
    M069Player *b;
    M069Player *prev;
    Object *obj;
    Object **targets;
    ObjectTransform *cam;
    UNUSED s16 pad;
    u16 held;
    s8 stickY;
    s8 stickX;

    cam = func_80041DE0();
    func_80079890(0, cam);
    if (D_800A4FEC == 0) {
            if (gMenuTimer != 0) {
                gMenuTimer -= updateRate;
                if (gMenuTimer <= 0) {
                    gMenuTimer = 0;
                    for (i = 0; i < count; i++) {
                        score = rl_m069_02000[i]->unk68;
                        D_800FED50[i].player = score->player;
                        D_800FED50[i].ants = score->ants;
                        D_800FED50[i].bears = score->bears;
                        D_800FED50[i].points = score->ants - score->bears;
                        a = &D_800FED50[i];
                        if (score->shots != 0) {
                            a->accuracy = score->ants * 100 / score->shots;
                        } else {
                            a->accuracy = 0;
                        }
                        D_800FF1C0[i] = a;
                    }
                    do {
                        sorted = TRUE;
                        for (i = 0; i < count - 1; i++) {
                            a = D_800FF1C0[i];
                            b = D_800FF1C0[i + 1];
                            if (a->points < b->points || (a->points == b->points && a->accuracy < b->accuracy)) {
                                D_800FF1C0[i] = b;
                                D_800FF1C0[i + 1] = a;
                                sorted = FALSE;
                            }
                        }
                    } while (!sorted);
                    prev = NULL;
                    for (i = 0; i < count;) {
                        a = D_800FF1C0[i];
                        i++;
                        if (prev == NULL) {
                            rank = i;
                        } else if (a->points != prev->points ||
                                   (a->points == prev->points && a->accuracy != prev->accuracy)) {
                            rank = i;
                        }
                        a->rank = rank;
                        prev = a;
                    }
                    if (D_800FF1C0[0]->rank != D_800FF1C0[1]->rank) {
                        rl_m069_02010 = D_800FF1C0[0]->player;
                    } else {
                        rl_m069_02010 = -1;
                    }
                    D_800A4FEC = 1;
                    rl_m069_02014 = 0;
                    if (rl_m069_02010 >= 0 && rl_m069_02010 < 4) {
                        rl_m069_01DE4 = func_80055694(rl_m069_01DFC[rl_m069_02010]);
                    }
                } else {
                    offsets = rl_m069_01D38[count - 1];
                    angles[0] = -0x8000 - cam->rotation.y_rotation;
                    angles[1] = -cam->rotation.x_rotation;
                    i = 0;
                    targets = rl_m069_02000;
                    angles[2] = -cam->rotation.z_rotation;
                    for (; i < count; i++) {
                        obj = *targets++;
                        func_800496B0(1, angles, offsets, &obj->trans.x_position);
                        obj->trans.x_position += cam->x_position;
                        obj->trans.y_position += cam->y_position;
                        obj->trans.z_position += cam->z_position;
                        obj->segmentID =
                            func_80015510(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
                        stickX = input_stick_x_raw(i);
                        stickY = input_stick_y_raw(i);
                        held = func_800434E4(i);
                        rl_m069_00A84(obj, obj->unk68, stickX, stickY, held, func_80043510(i), updateRate);
                        offsets += 3;
                    }
                }
            }
    } else {
    switch (D_800A4FEC) {
        case 1:
            rl_m069_02014 += updateRate;
            if (rl_m069_02014 >= 255) {
                rl_m069_02014 = 254;
            }
            D_800FF1B8 += updateRate;
            if (D_800FF1B8 >= 256) {
                D_800FF1B8 = 255;
                D_800A4FEC = 2;
            }
            break;
        case 2:
            for (i = 0; i < count; i++) {
                if (func_80043510(i) & 0x9000) {
                    music_volume_fade(1.0f, 0);
                    D_800A4FEC = 3;
                    break;
                }
            }
            break;
        case 3:
            rl_m069_02014 -= updateRate * 8;
            if (rl_m069_02014 <= 0) {
                rl_m069_02014 = 0;
                func_80047608(0x16, 0, 0);
                func_80055A00(rl_m069_01DE4);
                rl_m069_01DE4 = NULL;
                D_800A4FEC = 4;
            }
            break;
    }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m069/ro", func_m069_00A84);

/* The query that func_8007C2E8 takes, as used here */
typedef struct M069Ray {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 to[3];
    /* 0x18 */ f32 from[3];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x54 - 0x2C];
} M069Ray;

extern Object **func_80005E98(s32 *first, s32 *count); /* the objects */
extern s32 func_8007C2E8(s32 arg0, M069Ray *ray, s32 count, Object **objs, Object **hit, s32 arg5);
extern f32 func_80097140(f32 x); /* sqrtf */

#ifdef NON_MATCHING
/* 27 diffs: stack offsets only (the frame is 8 bytes bigger; the code is the same) */
/*
 * Of the objects with a hit box of kind 1 or 2 (and header flag 0x40), how many the line from `from`
 * to `to` meets; the distance to the first goes to *dist (as the original computes it)
 */
s32 func_m069_0135C(s32 arg0, f32 *from, f32 *to, f32 *dist) {
    s32 i;
    s32 first;
    s32 count;
    M069Ray ray;
    s32 n;
    Object *hit;
    Object **list;
    Object *obj;
    Object *objs[256];

    n = 0;
    ray.from[0] = from[0];
    ray.from[1] = from[1];
    ray.from[2] = from[2];
    ray.to[0] = to[0];
    ray.to[1] = to[1];
    ray.to[2] = to[2];
    ray.unk24 = 1.0f;
    ray.unk28 = 1.0f;
    list = func_80005E98(&first, &count);
    list += first;
    i = count - first;
    while (i--) {
        obj = *list++;
        if (obj->interactObj != NULL && (((u16 *) obj->header)[0x14 / 2] & 0x40)) {
            if (((u8 *) obj->interactObj)[0x30] == 1 || ((u8 *) obj->interactObj)[0x30] == 2) {
                objs[n++] = obj;
            }
        }
    }
    if (n > 0) {
        n = func_8007C2E8(0, &ray, n, objs, &hit, arg0);
    }
    if (n > 0) {
        *dist = func_80097140((ray.from[0] - hit->trans.y_position) * (ray.from[0] - hit->trans.y_position) +
                              (ray.from[0] - hit->trans.y_position) * (ray.from[0] - hit->trans.y_position) +
                              (ray.from[0] - hit->trans.y_position) * (ray.from[0] - hit->trans.y_position));
    }
    return n;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m069", func_m069_0135C);
#endif

/* Draws player `player`'s crosshair at (x, y) */
static void func_m069_014D0(Gfx **dl, Vertex **verts, s32 player, s32 x, s32 y) {
    s32 i;
    s8 *line;
    s8 *point;
    Vertex *v;

    if (player >= 0 && player < 4) {
        line = rl_m069_01F08[0];
        i = 14;
        while (i--) {
            screen_line_add_in_window(line[0] + x, line[1] + y, line[2] + x, line[3] + y, line[4], 0);
            line += 5;
        }
        v = *verts;
        point = rl_m069_01F50[0];
        x -= 160;
        y = 120 - y;
        i = 12;
        while (i--) {
            v->x = point[0] + x;
            v->y = point[1] + y;
            v->z = 1;
            v->r = 255;
            v->g = 255;
            v->b = 255;
            v->a = 255;
            v++;
            point += 2;
        }
        func_80055C00(dl, NULL, 5, 0);
        gDPSetColor((*dl)++, G_SETPRIMCOLOR, rl_m069_01EF8[player]);
        gSPVertexJFG((*dl)++, (u32) *verts + 0x80000000, 13, 0);
        gSPPolygon((*dl)++, (u8 *) rl_m069_01F68 + 0x80000000, 4, 0);
        gDPPipeSync((*dl)++);
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        *verts = v;
    }
}

/* Draws the crosshairs, the end banner and the score lines */
void func_m069_016F4(Gfx **dl, Mtx **mtx, Vertex **verts, s32 count) {
    s32 i;
    s32 y;
    s32 fade;
    char *text;
    u8 *colour;
    char buf[32];
    s32 width;
    s32 height;
    M069Score *score;
    M069Point *pos;
    void *image;
    Object *obj;

    func_80040D64(dl, mtx);
    for (i = 0; i < count; i++) {
        obj = rl_m069_02000[i];
        score = obj->unk68;
        func_m069_014D0(dl, verts, i, score->x, score->y);
    }
    if (D_800A4FEC) {
        func_8006FC54(1);
        func_8006FC8C(0, 0, 0, 0);
        func_80054EA0(&width, &height);
        fade = (0xFF - D_800FF1B8) << 8;
        fade = (fade << 8) | fade | 0xFF000000;
        func_8006F550(dl, gVideoLastFramebuffer, width, height, 0, 0, width, height, fade);
        if (rl_m069_02010 >= 0 && rl_m069_02010 < 4) {
            y = 64;
            text = gMenuText[520];
            image = &rl_m069_01DE4;
            func_8004F02C(dl, image, 161.0f, 81.0f, 1.5f, 1.5f, rl_m069_02014, 1);
            func_8004F02C(dl, image, 160.0f, 80.0f, 1.5f, 1.5f,
                          (rl_m069_01D48[rl_m069_02010] & ~0xFF) | rl_m069_02014, 1);
        } else {
            y = 72;
            text = gMenuText[519];
        }
        func_8006FC68(0, 0, 0, 255, rl_m069_02014);
        func_8006FCA8(dl, 161, y + 1, text, 12);
        func_8006FC68(255, 255, 0, 255, rl_m069_02014);
        func_8006FCA8(dl, 159, y - 1, text, 12);
    }
    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    menu_fill_box(dl, 0, 192, 320, 240, 128);
    pos = rl_m069_01DD0[count - 1];
    for (i = 0; i < count; pos += 3, i++) {
        obj = rl_m069_02000[i];
        score = obj->unk68;
        colour = (u8 *) &rl_m069_01D48[i];
        func_8006FC68(colour[0], colour[1], colour[2], 255, 240);
        func_8006FCA8(dl, pos[0].x, pos[0].y, "Shots : ", 1);
        func_800639E8(buf, "%d\n", score->shots);
        func_8006FCA8(dl, pos[0].x, pos[0].y, buf, 0);
        func_8006FCA8(dl, pos[1].x, pos[1].y, "Ants : ", 1);
        func_800639E8(buf, "%d\n", score->ants);
        func_8006FCA8(dl, pos[1].x, pos[1].y, buf, 0);
        func_8006FCA8(dl, pos[2].x, pos[2].y, "Bears : ", 1);
        func_800639E8(buf, "%d\n", score->bears);
        func_8006FCA8(dl, pos[2].x, pos[2].y, buf, 0);
        if (score->ammo >= 0) {
            func_8006FCA8(dl, pos[2].x, pos[2].y + 10, "Ammo : ", 1);
            func_800639E8(buf, "%d\n", score->ammo);
            func_8006FCA8(dl, pos[2].x, pos[2].y + 10, buf, 0);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m069", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m069", bss);
