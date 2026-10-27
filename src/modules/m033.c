/*
 * Runlink module 33: loads the explosion definitions (asset 0x40, indexed by
 * the offset table in asset 0x3F), turns their offsets into pointers and
 * sets their headers, then builds the triangles of the explosion's ring
 * mesh: a band of 20 vertices (two per step) closed into a loop.
 */
#include "module.h"
#include "gfx.h"

typedef struct M033Header {
    /* 0x00 */ s16 type;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 flags;
} M033Header;

typedef struct M033Part1 {
    /* 0x00 */ M033Header header;
    /* 0x04 */ u8 pad4[0x22 - 0x4];
} M033Part1;

typedef struct M033Part2 {
    /* 0x00 */ M033Header header;
    /* 0x04 */ u8 pad4[0x1C - 0x4];
} M033Part2;

typedef struct M033Explosion {
    /* 0x00 */ M033Header header;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ u8 *unkC;
    /* 0x10 */ u8 *unk10;
    /* 0x14 */ u8 *unk14;
    /* 0x18 */ M033Part1 *parts1;
    /* 0x1C */ u8 *unk1C;
    /* 0x20 */ u8 *unk20;
    /* 0x24 */ u8 *unk24;
    /* 0x28 */ M033Part2 *parts2;
    /* 0x2C */ u8 pad2C[0x31 - 0x2C];
    /* 0x31 */ u8 numParts1;
    /* 0x32 */ u8 numParts2;
} M033Explosion;

extern s32 D_800F6B08;             /* number of explosions */
extern u8 *D_800F6B00;             /* the explosion data (asset 0x40) */
extern M033Explosion **D_800F6B04; /* the explosions */
extern Triangle D_800F2D58[20];    /* the ring's triangles */

extern s32 *func_8004D37C(s32 asset);               /* DKR asset_table_load */
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);            /* DKR mempool_free */

#define RELOC(e, field) ((e)->field = (void *) ((u8 *) (e) + (s32) (e)->field))

void func_m033_00000(void) {
    s32 *table;
    s32 i;
    s32 j;
    s32 k;
    M033Explosion *e;
    M033Part1 *p1;
    M033Part2 *p2;
    Triangle *tri;
    s16 u0;
    s16 u1;
    s16 tmp;

    table = func_8004D37C(0x3F);
    D_800F6B08 = 0;
    while (table[D_800F6B08] != -1) {
        D_800F6B08++;
    }
    D_800F6B08--;
    D_800F6B00 = (u8 *) func_8004D37C(0x40);
    D_800F6B04 = func_8004A57C(D_800F6B08 * 4, 0xC0C040FF);
    for (i = 0; i < D_800F6B08; i++) {
        e = (M033Explosion *) (table[i] + D_800F6B00);
        D_800F6B04[i] = e;
        RELOC(e, unkC);
        RELOC(e, unk10);
        RELOC(e, unk14);
        RELOC(e, parts1);
        RELOC(e, unk1C);
        RELOC(e, unk20);
        RELOC(e, unk24);
        RELOC(e, parts2);
        e->header.type = 0x4B;
        e->header.flags = 0;
        e->header.size = 0x58;
        p1 = e->parts1;
        for (j = 0; j < e->numParts1; j++, p1++) {
            p1->header.type = 0x73;
            p1->header.size = 0x22;
            p1->header.flags = 0;
        }
        p2 = e->parts2;
        for (j = 0; j < e->numParts2; j++, p2++) {
            p2->header.type = 0x80;
            p2->header.size = 0x1C;
            p2->header.flags = 0;
        }
    }
    func_8004AA80(table);

    tri = D_800F2D58;
    u0 = 0;
    u1 = 0x800;
    for (k = 0; k < 18; k += 2) {
        tri->vi0 = k;
        tri->vi1 = k + 3;
        tri->vi2 = k + 1;
        tri->uv[0][0] = u0;
        tri->uv[0][1] = 0x400;
        tri->uv[1][1] = 0;
        tri->uv[2][1] = 0;
        tri->flags = 0x40;
        tri++;
        tri->vi0 = k;
        tri->vi1 = k + 2;
        tri->vi2 = k + 3;
        tri->uv[0][0] = u0;
        tri->uv[0][1] = 0x400;
        tri->uv[1][0] = u1;
        tri->uv[1][1] = 0x400;
        tri->uv[2][1] = 0;
        tri->flags = 0x40;
        tri++;
        tmp = u0;
        u0 = u1;
        u1 = tmp;
    }
    tri->vi0 = k;
    tri->vi1 = 1;
    tri->vi2 = k + 1;
    tri->uv[0][0] = 0;
    tri->uv[0][1] = 0x400;
    tri->uv[1][1] = 0;
    tri->uv[2][1] = 0;
    tri->flags = 0x40;
    tri++;
    tri->vi0 = k;
    tri->vi1 = 0;
    tri->vi2 = 1;
    tri->uv[0][0] = 0;
    tri->uv[0][1] = 0x400;
    tri->uv[1][0] = 0x800;
    tri->uv[1][1] = 0x400;
    tri->uv[2][1] = 0;
    tri->flags = 0x40;
    tri++;
}
