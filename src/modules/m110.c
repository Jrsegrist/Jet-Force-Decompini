/*
 * Runlink module 110: a box that marks the level's vertices inside it as
 * exceptions to the track fade, saving their colours so that they can be
 * put back.
 */
#include "module.h"

typedef struct M110Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 sizeX;
    /* 0x0C */ s16 sizeY;
    /* 0x0E */ s16 sizeZ;
} M110Setup;

/* A level vertex, with its colour read as two halves */
typedef struct M110Vertex {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ s16 colour[2];
} M110Vertex;

typedef struct M110Segment {
    /* 0x00 */ M110Vertex *vertices;
    /* 0x04 */ u8 pad4[0x24 - 0x4];
    /* 0x24 */ s16 numVertices;
    /* 0x26 */ u8 pad26[0x48 - 0x26];
} M110Segment;

typedef struct M110Model {
    /* 0x0 */ void *unk0;
    /* 0x4 */ M110Segment *segments;
} M110Model;

typedef struct M110Exception {
    /* 0x0 */ s16 *colour;
    /* 0x4 */ s16 saved[2];
} M110Exception;

extern M110Exception *D_800A0D88; /* 0x80 entries */
extern s32 D_800A0D84;

extern s32 func_80015714(s32 *segments, s16 minX, s16 minY, s16 minZ, s16 maxX, s16 maxY, s16 maxZ);
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern M110Model *func_8001B650(void);

static void func_m110_00054(s16 x, s16 y, s16 z, s16 sizeX, s16 sizeY, s16 sizeZ);

void func_m110_00000(Object *obj, M110Setup *setup, s32 arg2) {
    if (obj->objectID == 0x1C8) {
        func_m110_00054(setup->common.x, setup->common.y, setup->common.z, setup->sizeX, setup->sizeY, setup->sizeZ);
    }
}

/* Marks the level vertices inside the box (centre, half sizes) as exceptions, saving their colours */
static void func_m110_00054(s16 x, s16 y, s16 z, s16 sizeX, s16 sizeY, s16 sizeZ) {
    s32 n;
    M110Model *model;
    s32 i;
    s32 segments[0x20];
    s32 minX;
    s32 minY;
    s32 minZ;
    s32 maxX;
    s32 maxY;
    s32 maxZ;
    M110Vertex *v;
    s32 k;
    M110Segment *seg;

    minX = x - sizeX;
    minY = y - sizeY;
    minZ = z - sizeZ;
    maxX = x + sizeX;
    maxY = y + sizeY;
    maxZ = z + sizeZ;
    n = func_80015714(segments, minX, minY, minZ, maxX, maxY, maxZ);
    if (n == 0) {
        return;
    }
    if (D_800A0D88 == NULL) {
        D_800A0D88 = func_8004A57C(0x400, 0xFFFF);
        D_800A0D84 = 0;
    }
    if (D_800A0D88 == NULL) {
        return;
    }
    model = func_8001B650();
    if (model == NULL) {
        return;
    }
    for (i = 0; i < n && D_800A0D84 < 0x80; i++) {
        seg = &model->segments[segments[i]];
        k = seg->numVertices;
        v = seg->vertices;
        while (k--) {
            x = v->x;
            y = v->y;
            z = v->z;
            if (x >= minX && x <= maxX && y >= minY && y <= maxY && z >= minZ && z <= maxZ) {
                D_800A0D88[D_800A0D84].colour = v->colour;
                D_800A0D88[D_800A0D84].saved[0] = v->colour[0];
                D_800A0D88[D_800A0D84].saved[1] = v->colour[1];
                if (++D_800A0D84 >= 0x80) {
                    break;
                }
            }
            v++;
        }
    }
}

/* Puts the saved colours back */
void func_m110_00320(void) {
    M110Exception *e;
    s32 n;

    if (D_800A0D88 != NULL) {
        n = D_800A0D84;
        e = D_800A0D88;
        while (n--) {
            e->colour[0] = e->saved[0];
            e->colour[1] = e->saved[1];
            e++;
        }
    }
}
