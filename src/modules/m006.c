/*
 * Runlink module 6: the in-game HUD's health meter and message bar. It builds
 * the meter's ring of vertices, places the HUD elements for one, two or four
 * players, keeps a small queue of messages per player that slide in and out
 * of a bar, and formats play times.
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
    /* 0x10 */ u8 pad10[0x10];
} M006MenuItem;

extern M006Queue D_800FF1D0[];
extern M006MenuItem D_800FF780[];
extern u8 D_800A4FD0; /* the number of players */
extern s8 D_800FF310;
extern s32 D_800FECC0; /* the TV refresh rate */
extern char rl_m006_01AC0[]; /* the module's .rodata (still assembly) */

extern void *mempool_alloc_safe(s32 size, s32 colour);
extern void mempool_free(void *data);
extern s32 normalise_time(s32 time);
extern s32 fb_mode(void);
extern s32 func_8005AA7C(s32 player);
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

INCLUDE_ASM("asm/nonmatchings/modules/m006", func_m006_00000);

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

#ifdef NON_MATCHING
/* Where one corner of a player's HUD is on the screen; the temporaries' registers differ */
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
        if ((hiRes & 1) || func_8005AA7C(player)) {
            offsetX = D_m006_01A94[player + 2][0];
            offsetY = D_m006_01A94[player + 2][1];
            coords = D_m006_01A84;
        } else {
            offsetX = D_m006_01A94[player][0];
            offsetY = D_m006_01A94[player][1];
            coords = D_m006_01A74;
        }
    } else {
        offsetX = D_m006_01A94[player + 4][0];
        offsetY = D_m006_01A94[player + 4][1];
        coords = D_m006_01A64;
    }
    fb_scale_coords(&offsetX, &offsetY);
    cornerX = coords[corner];
    cornerY = coords[corner + 1];
    fb_scale_coords(&cornerX, &cornerY);
    *x = cornerX + offsetX;
    *y = cornerY + offsetY;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m006", func_m006_00F30);
#endif

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

#ifdef NON_MATCHING
/* Slides a player's message bar in and out and moves on through the queue; register allocation differs */
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
                    queue->current = queue->messages[queue->tail];
                    queue->unk4E = 0;
                    if (++queue->tail >= 5) {
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
#else
INCLUDE_ASM("asm/nonmatchings/modules/m006", func_m006_012D4);
#endif

void func_m006_015E0(s32 hide) {
    D_800FF310 = hide;
}

#ifdef NON_MATCHING
/*
 * The original multiplied by a double literal 1.2 from the module's .rodata, which stays assembly
 * until func_m006_00000 (which has the rest of it) is C; read from there the load is scheduled
 * differently. 015EC's saved register also differs.
 */
/* Formats a time in frames as minutes, seconds and hundredths */
void func_m006_015EC(s32 time, char *minutes, char *seconds, char *hundredths) {
    s32 min;
    s32 sec;
    s32 rest;
    s32 frac;

    if (D_800FECC0 == 50) {
        time = (f32) time * *(f64 *) (rl_m006_01AC0 + 0x38);
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
    frac = ((time - sec * 60 - rest) * 100) / 60;
    if (frac < 10) {
        sprintf(hundredths, rl_m006_01AC0 + 0x10, frac);
    } else {
        sprintf(hundredths, rl_m006_01AC0 + 0x14, frac);
    }
}

/* The same with hours */
void func_m006_017AC(s32 time, char *hours, char *minutes, char *seconds, char *hundredths) {
    s32 min;
    s32 sec;
    s32 rest;
    s32 total;

    if (D_800FECC0 == 50) {
        time = (f32) time * *(f64 *) (rl_m006_01AC0 + 0x40);
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
#else
INCLUDE_ASM("asm/nonmatchings/modules/m006", func_m006_015EC);

INCLUDE_ASM("asm/nonmatchings/modules/m006", func_m006_017AC);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m006", rodata);
