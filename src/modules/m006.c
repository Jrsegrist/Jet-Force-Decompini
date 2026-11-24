/*
 * Runlink module 6: the in-game HUD's health meter and message bar. It builds
 * the meter's ring of vertices, places the HUD elements for one, two or four
 * players, keeps a small queue of messages per player that slide in and out
 * of a bar, and formats play times.
 * Family: menus and screens (docs/runlink.md, "Module families").
 */
#include "module.h"
#include "gfx.h"

/* A player's message queue (D_800FF1D0, one per player) */
typedef struct M006Queue {
    /* 0x00 */ s16 timer;
    /* 0x02 */ s16 current;
    /* 0x04 */ u8 count;
    /* 0x05 */ u8 head;
    /* 0x06 */ u8 tail;
    /* 0x07 */ u8 locked;
    /* 0x08 */ s16 messages[5];
    /* 0x12 */ u8 pad12[0x4E - 0x12];
    /* 0x4E */ s16 unk4E;
} M006Queue;

/* The menu items (D_800FF780, size 0x20) */
typedef struct M006MenuItem {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ f32 unk18;
    /* 0x1C */ u8 pad1C[4];
} M006MenuItem;

extern M006Queue D_800FF1D0[];
extern M006MenuItem D_800FF780[];
extern u8 D_800A4FD0; /* the number of players */
extern s8 D_800FF310;
extern s32 D_800FECC0; /* the TV refresh rate */
extern char rl_m006_01AC0[]; /* the module's .rodata (still assembly) */

extern void *mempool_alloc_safe(s32 size, u32 colour);
extern void mempool_free(void *data);
extern s32 normalise_time(s32 time);
extern s32 fb_mode(void);
extern s32 options_split_vertical_get(s32 player);
extern void fb_scale_coords(s32 *x, s32 *y);
extern void func_80054EA0(u32 *width, u32 *height); /* the frame buffer size */
extern void func_80098360(void *data, s32 size);  /* clears */

Triangle D_m006_019A0[] = {
    { 0x40, 0, 1, 2, { { 0x260, 0x1E0 }, { 0x200, 0 }, { 0, 0x1E0 } } },
    { 0x40, 1, 3, 2, { { 0x200, 0 }, { 0x60, 0 }, { 0, 0x1E0 } } },
    { 0x40, 2, 3, 4, { { 0x260, 0x1E0 }, { 0x200, 0 }, { 0, 0x1E0 } } },
    { 0x40, 3, 5, 4, { { 0x200, 0 }, { 0x60, 0 }, { 0, 0x1E0 } } },
    { 0x40, 4, 5, 6, { { 0x260, 0x1E0 }, { 0x200, 0 }, { 0, 0x1E0 } } },
    { 0x40, 5, 7, 6, { { 0x200, 0 }, { 0x60, 0 }, { 0, 0x1E0 } } },
    { 0x40, 6, 7, 8, { { 0x260, 0x1E0 }, { 0x200, 0 }, { 0, 0x1E0 } } },
    { 0x40, 7, 9, 8, { { 0x200, 0 }, { 0x60, 0 }, { 0, 0x1E0 } } },
    { 0x40, 8, 9, 10, { { 0x260, 0x1E0 }, { 0x200, 0 }, { 0, 0x1E0 } } },
    { 0x40, 9, 11, 10, { { 0x200, 0 }, { 0x60, 0 }, { 0, 0x1E0 } } },
    { 0x40, 10, 11, 12, { { 0x260, 0x1E0 }, { 0x200, 0 }, { 0, 0x1E0 } } },
    { 0x40, 11, 13, 12, { { 0x200, 0 }, { 0x60, 0 }, { 0, 0x1E0 } } },
};
Vertex *D_m006_01A60 = NULL;
/* Where the HUD's corners are, for four players, two, and two on a wide screen */
s16 D_m006_01A64[] = { -40, -25, 50, -25, -50, 35, 50, 25 };
s16 D_m006_01A74[] = { -100, -25, 100, -25, -100, 35, 100, 25 };
s16 D_m006_01A84[] = { -40, -85, 50, -85, -50, 95, 50, 85 };
/* Each player's screen centre: two players, two on a wide screen, four players */
s16 D_m006_01A94[][2] = { { 0, 60 }, { 0, -60 }, { -80, 0 }, { 80, 0 }, { -80, 60 }, { 80, 60 }, { -80, -60 }, { 80, -60 } };

static s8 sBarY[8];               /* per player: the message bar's position */
static s16 sBarWidth[4];          /* per player: how far the message bar is open */
static s32 sMeterSegments[4][12]; /* per player */
static s16 sMeterTimer[4];
static s16 sMeterTimer2[4];
static u8 sMeterAlpha[4];

extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern void *D_800FF3F8[]; /* the loaded menu images */
extern s32 D_800A51CC;     /* the text opacity */
extern void func_80055B80(Gfx **dl);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3); /* sets a texture */
extern void func_8005A0D0(s32 item);                                 /* draws a menu item */
extern void func_80042134(MtxF *mtx);                                /* identity */
extern void func_80049874(f32 x, f32 y, f32 z, MtxF *mtx);           /* scale */
extern void func_800498E8(f32 x, f32 y, f32 z, MtxF *mtx);           /* translation */
extern void func_80048D84(MtxF *mtx, Mtx *out);                      /* to a fixed-point Mtx */
extern void func_80040D64(Gfx **dl, Mtx **mtx);
void func_m006_00F30(s32 corner, s32 player, s32 *x, s32 *y);

/*
 * Draws a player's health meter: the ring of 12 segments (two triangles each, from 00C74's vertices) round the
 * HUD corner, fading in for a while after the health changes and out after. With one player each segment is a
 * unit of health (max of them, the empty ones dimmed) and item 2 shows the count of full rings; with more, five
 * units make a segment and the last one fades with what is left of it. Each segment eases towards its place.
 */
void func_m006_00000(Object *obj, s32 health, s32 max, s32 updateRate) {
    s32 count;
    s32 i;
    s32 n;
    s32 target;
    s32 x;
    s32 y;
    s8 *player;
    Triangle *tri;
    s32 left;
    s32 seg;
    s32 c;
    M006MenuItem *item;
    UNUSED s32 pad1;
    UNUSED s32 pad2;
    UNUSED s32 pad3;
    UNUSED s32 pad4;
    MtxF mtx;
    u32 width;
    u32 height;

    if (health < 0) {
        health = 0;
    }
    item = &D_800FF780[2];
    player = obj->unk68;
    tri = D_m006_019A0;
    left = 12;
    if (health != sMeterTimer2[*player]) {
        sMeterTimer2[*player] = health;
        sMeterTimer[*player] = normalise_time(900);
    }
    if (updateRate < sMeterTimer[*player]) {
        sMeterTimer[*player] -= updateRate;
        if (sMeterTimer[*player] > 700) {
            for (i = 0; i < updateRate; i++) {
                sMeterAlpha[*player] += (255 - sMeterAlpha[*player]) / 8;
            }
        } else if (sMeterTimer[*player] < 200) {
            for (i = 0; i < updateRate; i++) {
                sMeterAlpha[*player] -= sMeterAlpha[*player] / 8;
            }
        }
        func_80055B80(&D_800FF398);
        func_80042134(&mtx);
        func_80054EA0(&width, &height);
        mtx[3][3] = width >> 1; /* (as the ROM does: the matrix's w is set to half the screen width) */
        if (D_800A4FD0 == 1) {
            func_800498E8(item->y + 1.0f, item->unk10 - 2.0f, 0.0f, &mtx);
        } else {
            func_m006_00F30(0, *player, &x, &y);
            func_800498E8(x, y, 0.0f, &mtx);
            func_80049874(*(f32 *) (rl_m006_01AC0 + 0x34), *(f32 *) (rl_m006_01AC0 + 0x34), 1.0f, &mtx);
        }
        func_80048D84(&mtx, D_800FF39C);
        {
            Gfx *_g = D_800FF398++;
            _g->words.w0 = 0x01000040;
            _g->words.w1 = (u32) D_800FF39C + 0x80000000;
        }
        D_800FF39C = (Mtx *) ((u8 *) D_800FF39C + 0x40);
        gSPVertexJFG(D_800FF398++, (u32) D_m006_01A60 + 0x80000000, 15, 0);
        if (D_800A4FD0 != 1) {
            n = health;
            count = (max * 5 + 4) / 5;
            for (i = 0; i < count; i++) {
                target = i << 17;
                if (n < 5) {
                    c = (sMeterAlpha[*player] * (n * 92 / 5 + 100)) >> 8;
                    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, c);
                    sMeterSegments[*player][i] = ((5 - n) << 16) / 5 + target;
                } else {
                    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, (sMeterAlpha[*player] * 192) >> 8);
                }
                seg = sMeterSegments[*player][i];
                seg = (target - seg) / 8 + seg;
                sMeterSegments[*player][i] = seg;
                if (n >= 6) {
                    n -= 5;
                } else {
                    n = 0;
                }
                func_80055C00(&D_800FF398, D_800FF3F8[2], 5, seg);
                gDPPipeSync(D_800FF398++);
                c = (seg >> 8) & 0xFF;
                gDPSetEnvColor(D_800FF398++, c, c, c, c);
                gSPPolygon(D_800FF398++, (u8 *) tri + 0x80000000, 2, 1);
                left -= 2;
                tri += 2;
                /* the second half of the ring's vertices, unless this was the last segment */
                if (left == 0 && count - 1 != i) {
                    gSPVertexJFG(D_800FF398++, (u32) D_m006_01A60 + 0x8000008C, 15, 0);
                    tri = D_m006_019A0;
                    left = 12;
                }
            }
        } else {
            gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, (sMeterAlpha[*player] * 192) >> 8);
            n = health % (max + 1);
            for (i = 0; i < max; i++) {
                target = i << 17;
                if (n <= 0) {
                    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, (sMeterAlpha[*player] * 128) >> 8);
                    target += 0x10000;
                }
                sMeterSegments[*player][i] += (target - sMeterSegments[*player][i]) >> 2;
                seg = sMeterSegments[*player][i];
                func_80055C00(&D_800FF398, D_800FF3F8[2], 5, seg);
                gDPPipeSync(D_800FF398++);
                c = (seg >> 8) & 0xFF;
                gDPSetEnvColor(D_800FF398++, c, c, c, c);
                gSPPolygon(D_800FF398++, (u8 *) tri + 0x80000000, 2, 1);
                left -= 2;
                tri += 2;
                if (left == 0) {
                    if (i != 11) {
                        gSPVertexJFG(D_800FF398++, (u32) D_m006_01A60 + 0x8000008C, 15, 0);
                        tri = D_m006_019A0;
                        left = 12;
                    }
                }
                n--;
            }
            func_80040D64(&D_800FF398, &D_800FF39C);
            if (D_800FF780[2].unk18 == -1000.0f) {
                item->unk18 = health / (max + 1);
            } else {
                for (i = 0; i < updateRate; i++) {
                    item->unk18 += ((f32) (health / (max + 1)) - item->unk18) * 0.125f;
                }
            }
            D_800A51CC = (sMeterAlpha[*player] * 164) >> 8;
            func_8005A0D0(2);
            D_800A51CC = 255;
        }
    }
}

void func_m006_00C74(void) {
    Vertex *v;
    s32 i;
    s16 angle;
    s32 value;
    s32 j;
    f32 inner;
    f32 outer;

    if ((D_m006_01A60 = mempool_alloc_safe(0x118, -1)) != NULL) {
        angle = 0;
        v = D_m006_01A60;
        if (D_800A4FD0 == 1) {
            inner = 20.0f;
        } else {
            inner = 14.0f;
        }
        outer = 38.0f;
        for (i = 0; i != 14; i++) {
            v->x = func_80049C34(angle) * outer;
            v->y = func_80049C30(angle) * outer;
            v->z = 0;
            v->r = 255;
            v->g = 255;
            v->b = 255;
            v->a = 255;
            v++;
            v->x = func_80049C34(angle) * inner;
            v->y = func_80049C30(angle) * inner;
            v->z = 0;
            v->r = 255;
            v->g = 255;
            v->b = 255;
            v->a = 255;
            v++;
            if (i != 6) {
                angle -= 0x1555;
            }
        }
        for (j = 0; j < 4; j++) {
            value = 0x10000;
            for (i = 0; i < 12; i++) {
                sMeterSegments[j][i] = value;
                value += 0x20000;
            }
            sMeterAlpha[j] = 255;
            sMeterTimer[j] = normalise_time(900);
            sMeterTimer2[j] = 60;
        }
    }
}

void func_m006_00EFC(void) {
    if (D_m006_01A60 != NULL) {
        mempool_free(D_m006_01A60);
        D_m006_01A60 = NULL;
    }
}

/*
 * Where one corner of a player's HUD is on the screen. The offset rows D_m006_01A94[player + n] are read with
 * the row index pre-shifted onto the s16 scaling (two s16 per row)
 */
void func_m006_00F30(s32 corner, s32 player, s32 *x, s32 *y) {
    s32 hiRes;
    s32 cornerX;
    s32 cornerY;
    s32 offsetX;
    s32 offsetY;
    s16 *coords;

    hiRes = fb_mode();
    corner *= 2;
    if (D_800A4FD0 == 2) {
        if ((hiRes & 1) || options_split_vertical_get(player)) {
            offsetX = ((s16 *) D_m006_01A94 + (player << 1))[4];
            offsetY = ((s16 *) D_m006_01A94 + (player << 1))[5];
            coords = D_m006_01A84;
        } else {
            offsetX = ((s16 *) D_m006_01A94 + (player << 1))[0];
            offsetY = ((s16 *) D_m006_01A94 + (player << 1))[1];
            coords = D_m006_01A74;
        }
    } else {
        offsetX = ((s16 *) D_m006_01A94 + (player << 1))[8];
        offsetY = ((s16 *) D_m006_01A94 + (player << 1))[9];
        coords = D_m006_01A64;
    }
    fb_scale_coords(&offsetX, &offsetY);
    cornerX = coords[corner];
    cornerY = coords[corner + 1];
    fb_scale_coords(&cornerX, &cornerY);
    *x = cornerX + offsetX;
    *y = cornerY + offsetY;
}

void func_m006_0107C(s32 *x, s32 *y) {
    u32 width;
    u32 height;

    func_80054EA0(&width, &height);
    *x += width >> 1;
    *y = (height >> 1) - *y;
}

void func_m006_010D8(s32 *x, s32 *y) {
    u32 width;
    u32 height;

    func_80054EA0(&width, &height);
    *x -= width >> 1;
    *y = (height >> 1) - *y;
}

void func_m006_01134(void) {
    s32 i;

    M006Queue *queue = D_800FF1D0;

    for (i = 0; i < 4; i++) {
        func_80098360(queue, sizeof(M006Queue));
        queue->current = -1;
        queue++;
        sBarY[i] = 75;
        sBarWidth[i] = 0;
    }
}

s8 *func_m006_011C8(void) {
    return sBarY;
}

s16 *func_m006_011D4(void) {
    return sBarWidth;
}

void func_m006_011E0(void *player, s32 message) {
    M006Queue *queue = &D_800FF1D0[*(s8 *) player];

    if (queue->count == 5) {
        if (++queue->tail >= 5) {
            queue->tail = 0;
        }
        queue->count--;
    }
    queue->messages[queue->head] = message;
    if (++queue->head >= 5) {
        queue->head = 0;
    }
    queue->count++;
}

void func_m006_01284(s8 *player, s32 message) {
    M006Queue *queue = &D_800FF1D0[*player];

    queue->locked = 1;
    queue->current = message;
}

void func_m006_012B0(s8 *player) {
    M006Queue *queue = &D_800FF1D0[*player];

    queue->locked = 0;
}

/*
 * Slides a player's message bar in and out and moves on through the queue. (The empty test of
 * queue->locked at the end of the second arm gives the ROM's load of it at the end of each arm.)
 */
void func_m006_012D4(s8 *player, s32 updateRate) {
    M006Queue *queue = &D_800FF1D0[*player];

    if (queue->timer != 0 || queue->count != 0 || queue->locked) {
        if (D_800A4FD0 == 1) {
            if (D_800FF780[5].y < 122.0f) {
                D_800FF780[5].y += updateRate * 8;
                if (D_800FF780[5].y > 122.0f) {
                    D_800FF780[5].y = 122.0f;
                }
            }
        } else {
            if (sBarY[*player] >= 31) {
                sBarY[*player] -= updateRate * 2;
                if (sBarY[*player] < 30) {
                    sBarY[*player] = 30;
                }
            } else {
                sBarWidth[*player] += updateRate * 4;
                if (sBarWidth[*player] >= 87) {
                    sBarWidth[*player] = 86;
                }
            }
            if (queue->locked) {
            }
        }
        if (!queue->locked) {
            if (D_800A4FD0 == 1 || (D_800A4FD0 != 1 && sBarY[*player] == 30)) {
                queue->timer -= updateRate;
            }
            if (queue->timer < 0) {
                if (queue->count == 0) {
                    queue->timer = 0;
                } else {
                    queue->timer += normalise_time(120);
                    queue->current = queue->messages[queue->tail++];
                    queue->unk4E = 0;
                    if (queue->tail >= 5) {
                        queue->tail = 0;
                    }
                    queue->count--;
                }
            }
        }
    } else if (D_800A4FD0 == 1) {
        D_800FF780[5].y -= updateRate * 8;
        if (D_800FF780[5].y < -78.0f) {
            D_800FF780[5].y = -78.0f;
        }
    } else {
        sBarWidth[*player] -= updateRate * 4;
        if (sBarWidth[*player] < 0) {
            sBarWidth[*player] = 0;
            sBarY[*player] += updateRate * 2;
            if (sBarY[*player] >= 76) {
                sBarY[*player] = 75;
            }
        }
    }
}

void func_m006_015E0(s32 hide) {
    D_800FF310 = hide;
}

/* The 50 Hz time scales: doubles in the module's .rodata (still assembly), each read by its own address */
extern f64 rl_m006_01AF8;
extern f64 rl_m006_01B00;

/* Formats a time in frames as minutes, seconds and hundredths */
void func_m006_015EC(s32 time, char *minutes, char *seconds, char *hundredths) {
    s32 min;
    s32 sec;
    s32 rest;

    if (D_800FECC0 == 50) {
        time = (f32) time * rl_m006_01AF8;
    }
    min = time / 3600;
    if (min < 10) {
        sprintf(minutes, rl_m006_01AC0, min);
    } else {
        sprintf(minutes, rl_m006_01AC0 + 4, min);
    }
    rest = min * 3600;
    sec = (time - rest) / 60;
    if (sec < 10) {
        sprintf(seconds, rl_m006_01AC0 + 8, sec);
    } else {
        sprintf(seconds, rl_m006_01AC0 + 0xC, sec);
    }
    if (((time - sec * 60 - rest) * 100) / 60 < 10) {
        sprintf(hundredths, rl_m006_01AC0 + 0x10, ((time - sec * 60 - rest) * 100) / 60);
    } else {
        sprintf(hundredths, rl_m006_01AC0 + 0x14, ((time - sec * 60 - rest) * 100) / 60);
    }
}

/* The same with hours */
void func_m006_017AC(s32 time, char *hours, char *minutes, char *seconds, char *hundredths) {
    s32 min;
    s32 sec;
    s32 rest;
    s32 total;

    if (D_800FECC0 == 50) {
        time = (f32) time * rl_m006_01B00;
    }
    total = time / 3600;
    min = total % 60;
    if (min < 10) {
        sprintf(minutes, rl_m006_01AC0 + 0x18, min);
    } else {
        sprintf(minutes, rl_m006_01AC0 + 0x1C, min);
    }
    rest = total * 3600;
    sec = (time - rest) / 60;
    if (sec < 10) {
        sprintf(seconds, rl_m006_01AC0 + 0x20, sec);
    } else {
        sprintf(seconds, rl_m006_01AC0 + 0x24, sec);
    }
    rest = ((time - sec * 60 - rest) * 100) / 60;
    if (rest < 10) {
        sprintf(hundredths, rl_m006_01AC0 + 0x28, rest);
    } else {
        sprintf(hundredths, rl_m006_01AC0 + 0x2C, rest);
    }
    sprintf(hours, rl_m006_01AC0 + 0x30, total / 60);
}

INCLUDE_ASM("asm/nonmatchings/modules/m006", rodata);
