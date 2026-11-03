/*
 * Runlink module 101: a breakable pane. It is drawn as one textured quad,
 * its texture scrolling with the camera, until it has taken enough damage.
 * Then it breaks into 40 triangular shards that spin, fly apart and fall
 * (each point of the pane keeps its texture coordinate) for a few seconds,
 * after which the object frees itself or, if its level entry says so, just
 * stops drawing.
 */
#include "module.h"
#include "gfx.h"

/*
 * gSPVertexJFG with the count given as the last index (n - 1) and the length
 * field as the code computes it (m064)
 */
#define gSPVertexJFGLast(pkt, v, last, v0) \
    gDma1p(pkt, G_VTX, v, ((last) << 3) + ((last) << 1) + 8, (last) << 3 | ((u32) (v) & 6) | (v0))

/* The start of a texture: its size in pixels */
typedef struct M101Texture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
} M101Texture;

/* What obj->unk68 points at */
typedef struct M101Pane {
    /* 0x00 */ M101Texture *texture;
    /* 0x04 */ Triangle *quadTris[2]; /* double-buffered */
    /* 0x0C */ Vertex *quadVerts;
    /* 0x10 */ struct M101Shard *shards;
    /* 0x14 */ Triangle *shardTris;
    /* 0x18 */ Vertex *shardVerts[2]; /* double-buffered */
    /* 0x20 */ Triangle tris[2][2];
} M101Pane;

/* The pane's state in the object */
typedef struct M101State {
    /* 0x00 */ s32 strength; /* then, once broken, the time since */
    /* 0x04 */ s32 buffer;
} M101State;

#define PANE_STATE(obj) OBJ_STATE(obj, M101State)

extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */
extern void mtx_pop(Gfx **dl);

extern s32 D_m101_01130; /* the module's number */

INCLUDE_ASM("asm/nonmatchings/modules/m101", func_m101_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m101", func_m101_0056C);

INCLUDE_ASM("asm/nonmatchings/modules/m101", func_m101_009F8);

INCLUDE_ASM("asm/nonmatchings/modules/m101", func_m101_00B64);

void func_m101_00DCC(Gfx **dl, s32 arg1, Object *obj) {
    M101Pane *pane;
    u8 *tris;
    Vertex *verts;
    s32 left;
    s32 numVerts;
    s32 numTris;

    runlink_unload_timer_set(D_m101_01130, 2, 2);
    pane = obj->unk68;
    func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
    func_80055C00(dl, pane->texture, 0xF, 0);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
    left = 120;
    if (PANE_STATE(obj)->strength > 0) {
        gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(pane->quadVerts), 5, 0);
        gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(pane->quadTris[PANE_STATE(obj)->buffer]), 2, 1);
    } else {
        verts = pane->shardVerts[PANE_STATE(obj)->buffer];
        tris = (u8 *) pane->shardTris;
        do {
            if (left > 24) {
                numVerts = 24;
                numTris = 8;
                left -= 24;
            } else {
                numVerts = left;
                numTris = left / 3;
                left = 0;
            }
            gSPVertexJFGLast((*dl)++, OS_K0_TO_PHYSICAL(verts), numVerts, 0);
            gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(tris), numTris, 1);
            verts += numVerts;
            tris += numTris * 16; /* sizeof (Triangle) */
        } while (left != 0);
    }
    mtx_pop(dl);
}

INCLUDE_ASM("asm/nonmatchings/modules/m101", data);

INCLUDE_ASM("asm/nonmatchings/modules/m101", rodata);
