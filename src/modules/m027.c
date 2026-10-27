/*
 * Runlink module 27: animated vertex lighting. The init builds a 256-entry
 * colour table (four 64-step gradients between three colours from its
 * parameters); the update recolours the vertices of the flagged model
 * segments from that table, by a phase that advances each frame, either
 * modulating their base colours or taking the brighter of the two.
 */
#include "module.h"

/* The game's 10-byte vertex */
typedef struct M027Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} M027Vertex;

typedef struct M027Colours {
    /* 0x00 */ u8 *base; /* r, g, b per vertex */
    /* 0x04 */ s16 *bits; /* one bit per vertex, cleared */
} M027Colours;

typedef struct M027Segment {
    /* 0x00 */ M027Vertex *verts;
    /* 0x04 */ u8 pad4[0x20];
    /* 0x24 */ s16 numVerts;
    /* 0x26 */ u8 pad26[0xC];
    /* 0x32 */ u8 unk32;
    /* 0x33 */ u8 pad33;
    /* 0x34 */ M027Colours *colours;
    /* 0x38 */ u8 pad38[0x10];
} M027Segment;

typedef struct M027Model {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ M027Segment *segments;
    /* 0x08 */ u8 pad8[0x12];
    /* 0x1A */ s16 numSegments;
} M027Model;

typedef struct M027Params {
    /* 0x000 */ u8 pad0[0xF7];
    /* 0x0F7 */ u8 mode;
    /* 0x0F8 */ u8 colourA[3];
    /* 0x0FB */ u8 colourB[3];
    /* 0x0FE */ u8 colourC[3];
} M027Params;

extern void *func_8004A57C(s32 size, s32 colour); /* allocates */
extern void func_8004AA80(void *data);             /* frees */

static u8 *sTable = NULL;
static s32 sMode = 1;
static s32 sPhase = 0;

static void makeGradient(u8 *dst, s32 r, s32 g, s32 b, s32 r2, s32 g2, s32 b2) {
    s32 i;

    r2 -= r;
    g2 -= g;
    b2 -= b;
    r <<= 6;
    g <<= 6;
    b <<= 6;
    i = 64;
    while (i--) {
        dst[0] = r >> 6;
        dst[1] = g >> 6;
        dst[2] = b >> 6;
        r += r2;
        g += g2;
        b += b2;
        dst += 3;
    }
}

void func_m027_0008C(M027Params *params) {
    sTable = func_8004A57C(0x300, -1);
    if (sTable != NULL) {
        makeGradient(sTable, params->colourC[0], params->colourC[1], params->colourC[2], params->colourB[0],
                     params->colourB[1], params->colourB[2]);
        makeGradient(sTable + 0xC0, params->colourB[0], params->colourB[1], params->colourB[2], params->colourA[0],
                     params->colourA[1], params->colourA[2]);
        makeGradient(sTable + 0x180, params->colourA[0], params->colourA[1], params->colourA[2], params->colourB[0],
                     params->colourB[1], params->colourB[2]);
        makeGradient(sTable + 0x240, params->colourB[0], params->colourB[1], params->colourB[2], params->colourC[0],
                     params->colourC[1], params->colourC[2]);
        sPhase = 0;
        sMode = params->mode;
    }
}

/* The module's shutdown function (runlink fini) */
void func_m027_001A8(void) {
    if (sTable != NULL) {
        func_8004AA80(sTable);
        sTable = NULL;
    }
}
#pragma weak __rl_fini = func_m027_001A8

void func_m027_001DC(u8 *flags, M027Model *model, s32 delta) {
    u8 *table;
    s32 phase;
    M027Segment *seg;
    s32 i;
    M027Colours *colours;
    s16 *bits;
    s32 j;
    u8 *base;
    M027Vertex *v;
    s32 idx;
    u8 *c;
    s32 br;
    s32 bg;
    s32 bb;
    s32 cr;
    s32 cg;
    s32 cb;

    table = sTable;
    if (table != NULL) {
        sPhase = (sPhase + delta) & 0xFF;
        phase = sPhase;
        seg = model->segments;
        i = model->numSegments;
        while (i--) {
            if (*flags) {
                colours = seg->colours;
                if (colours != NULL) {
                    bits = colours->bits;
                    j = (seg->numVerts + 15) >> 4;
                    while (j--) {
                        *bits++ = 0;
                    }
                    base = colours->base;
                    v = seg->verts;
                    j = seg->numVerts;
                    if (sMode == 1) {
                        while (j--) {
                            idx = (v->z + v->x + v->y + phase) & 0xFF;
                            c = table + idx + idx + idx;
                            v->r = (base[0] * c[0]) >> 8;
                            v->g = (base[1] * c[1]) >> 8;
                            v->b = (base[2] * c[2]) >> 8;
                            base += 3;
                            v++;
                        }
                    } else {
                        while (j--) {
                            idx = (v->z + v->x + v->y + phase) & 0xFF;
                            c = table + idx + idx + idx;
                            br = base[0];
                            bg = base[1];
                            bb = base[2];
                            cr = c[0];
                            cg = c[1];
                            cb = c[2];
                            base += 3;
                            if (cr < br) {
                                v->r = br;
                            } else {
                                v->r = cr;
                            }
                            if (cg < bg) {
                                v->g = bg;
                            } else {
                                v->g = cg;
                            }
                            if (cb < bb) {
                                v->b = bb;
                            } else {
                                v->b = cb;
                            }
                            v++;
                        }
                    }
                }
                seg->unk32 = 0;
            }
            flags++;
            seg++;
        }
    }
}
