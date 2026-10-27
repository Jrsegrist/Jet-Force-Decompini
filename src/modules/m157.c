/* Runlink module 157: draws a window frame as a set of rectangles from a shape table. */
#include "module.h"

/*
 * One rectangle of a frame shape, in pixels from the window's top left
 * corner; a set *rel flag measures that coordinate from the right (x) or
 * bottom (y) edge instead. A shape ends with a piece whose x0rel is negative.
 */
typedef struct WindowPiece {
    /* 0x0 */ s8 x0rel;
    /* 0x1 */ s8 x0;
    /* 0x2 */ s8 y0rel;
    /* 0x3 */ s8 y0;
    /* 0x4 */ s8 x1rel;
    /* 0x5 */ s8 x1;
    /* 0x6 */ s8 y1rel;
    /* 0x7 */ s8 y1;
    /* 0x8 */ s8 alpha; /* scales the colour's alpha, 0x80 = 1 */
} WindowPiece;

typedef struct WindowRect {
    /* 0x0 */ s16 x0;
    /* 0x2 */ s16 y0;
    /* 0x4 */ s16 x1;
    /* 0x6 */ s16 y1;
    /* 0x8 */ u32 colour;
} WindowRect;

extern void func_800595F4(s32, s32, WindowRect *, s32); /* draws rectangles */

static WindowPiece sWindowPieces[] = {
    { 0, -4, 0, -4, 1, 4, 0, -3, 0x1A },  { 0, -3, 0, -3, 1, 3, 0, -2, 0x33 },
    { 0, -2, 0, -2, 1, 2, 0, -1, 0x4D },  { 0, -1, 0, -1, 1, 1, 0, 0, 0x66 },
    { 0, -4, 0, -3, 0, -3, 1, 3, 0x1A },  { 0, -3, 0, -2, 0, -2, 1, 2, 0x33 },
    { 0, -2, 0, -1, 0, -1, 1, 1, 0x4D },  { 0, -1, 0, 0, 0, 0, 1, 0, 0x66 },
    { 1, 0, 0, 0, 1, 1, 1, 0, 0x66 },     { 1, 1, 0, -1, 1, 2, 1, 1, 0x4D },
    { 1, 2, 0, -2, 1, 3, 1, 2, 0x33 },    { 1, 3, 0, -3, 1, 4, 1, 3, 0x1A },
    { 0, -1, 1, 0, 1, 1, 1, 1, 0x66 },    { 0, -2, 1, 1, 1, 2, 1, 2, 0x4D },
    { 0, -3, 1, 2, 1, 3, 1, 3, 0x33 },    { 0, -4, 1, 3, 1, 4, 1, 4, 0x1A },
    { 0, 0, 0, 0, 1, 0, 1, 0, 0x7F },     { -1, -1, -1, -1, -1, -1, -1, -1, 0 },
};

static WindowPiece *sWindowShapes[] = { sWindowPieces };

void func_m157_00000(s32 arg0, u8 shape, s32 x, s32 y, s32 width, s32 height, u32 colour) {
    WindowRect *r;
    WindowPiece *p;
    WindowRect rects[24];
    s32 n;
    u32 alpha;

    p = sWindowShapes[shape];
    alpha = colour & 0xFF;
    r = rects;
    n = 0;
    colour &= ~0xFF;
    for (; p->x0rel >= 0 && n < 24; n++) {
        r->x0 = p->x0 + x;
        if (p->x0rel) {
            r->x0 += width;
        }
        r->y0 = p->y0 + y;
        if (p->y0rel) {
            r->y0 += height;
        }
        r->x1 = p->x1 + x;
        if (p->x1rel) {
            r->x1 += width;
        }
        r->y1 = p->y1 + y;
        if (p->y1rel) {
            r->y1 += height;
        }
        r->colour = (s32) (p->alpha * alpha) >> 7 | colour;
        p++;
        r++;
    }
    func_800595F4(arg0, n, rects, 0);
}
