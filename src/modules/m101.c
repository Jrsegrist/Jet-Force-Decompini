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

/* The pane's level entry */
typedef struct M101Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 zRotation;
    /* 0x10 */ u8 scale;
    /* 0x11 */ u8 pad11;
    /* 0x12 */ u8 strength;
} M101Setup;

/* The model the pane's texture comes from */
typedef struct M101Model {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M101Texture **textures;
} M101Model;

extern f32 rl_m101_01174;       /* a .rodata float: the scale factor */
extern Vertex rl_m101_01134[];  /* the quad's vertices (.data) */

#ifdef NON_MATCHING
/* Registers: the ROM loads the texture through a0/a3 and computes pane->tris early in v1 */
void func_m101_009F8(Object *obj, M101Setup *setup) {
    M101Pane *pane;
    Triangle *t;
    s32 i;

    obj->trans.rotation.y_rotation = setup->yRotation;
    pane = obj->unk68;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->trans.rotation.z_rotation = setup->zRotation;
    obj->trans.scale = obj->header->scale * setup->scale * rl_m101_01174;
    PANE_STATE(obj)->strength = setup->strength << 8;
    if (PANE_STATE(obj)->strength <= 0) {
        PANE_STATE(obj)->strength = 0x100;
    }
    PANE_STATE(obj)->buffer = 1;
    pane->texture = (*(M101Model **) obj->modelInstances[0])->textures[0];
    pane->quadTris[0] = pane->tris[0];
    pane->quadTris[1] = pane->tris[1];
    pane->quadVerts = rl_m101_01134;
    for (i = 0; i < 2; i++) {
        t = pane->quadTris[i];
        t->flags = 0x40;
        t->vi0 = 0;
        t->vi1 = 1;
        t->vi2 = 2;
        t++;
        t->flags = 0x40;
        t->vi0 = 0;
        t->vi1 = 2;
        t->vi2 = 3;
    }
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m101", func_m101_009F8);
#endif

/* What obj->unk5C points at for the pane */
typedef struct M101Hits {
    /* 0x000 */ u8 pad0[0x12A];
    /* 0x12A */ s8 hit;
    /* 0x12B */ u8 pad12B[0x130 - 0x12B];
    /* 0x130 */ s16 damage;
} M101Hits;

extern ObjectTransform *func_80041DE0(void); /* the camera */
extern u8 D_800A4FC4;
extern void rl_m101_00000(Object *obj, M101Pane *pane);  /* breaks the pane */
extern void rl_m101_0056C(Object *obj, M101Pane *pane, s32 updateRate); /* moves the shards */

#ifdef NON_MATCHING
/*
 * Before it breaks: scrolls the quad's texture with the pane's and the camera's angles and takes
 * damage. Once broken: moves the shards and, after three seconds, frees itself or stops drawing.
 * Frame and length match; the texture, its width and height are in other registers (a1/a0/a2).
 */
void func_m101_00B64(Object *obj, s32 updateRate) {
    s32 u;
    s32 v;
    s32 w;
    s32 h;
    ObjectTransform *cam;
    M101Pane *pane;
    M101Hits *hits;
    M101Texture *tex;
    Triangle *t;
    s16 yaw;
    s16 pitch;

    runlink_unload_timer_set(D_m101_01130, 2, 2);
    if (obj->trans.flags & 0x400) {
        return;
    }
    pane = obj->unk68;
    hits = (M101Hits *) obj->unk5C;
    cam = func_80041DE0();
    PANE_STATE(obj)->buffer = 1 - PANE_STATE(obj)->buffer;
    if (PANE_STATE(obj)->strength <= 0) {
        PANE_STATE(obj)->strength -= updateRate;
        if (-PANE_STATE(obj)->strength >= 181) {
            if (obj->level_entry != NULL && (((u8 *) obj->level_entry)[3] & 1)) {
                obj->trans.flags |= 0x400;
            } else {
                func_800079B8(obj);
            }
        } else {
            rl_m101_0056C(obj, pane, updateRate);
        }
        return;
    }
    t = pane->quadTris[PANE_STATE(obj)->buffer];
    tex = pane->texture;
    yaw = -obj->trans.rotation.y_rotation;
    pitch = -obj->trans.rotation.x_rotation;
    u = tex->width << 4;
    w = u;
    v = tex->height << 4;
    h = v;
    if (!D_800A4FC4) {
        yaw += cam->rotation.y_rotation;
        pitch -= cam->rotation.x_rotation;
    }
    u += (tex->width * yaw) >> 11;
    v += (tex->height * pitch) >> 11;
    t->uv[0][0] = u + w;
    t->uv[0][1] = v - h;
    t->uv[1][0] = u + w;
    t->uv[1][1] = v + h;
    t->uv[2][0] = u - w;
    t->uv[2][1] = v + h;
    t++;
    t->uv[0][0] = u + w;
    t->uv[0][1] = v - h;
    t->uv[1][0] = u - w;
    t->uv[1][1] = v + h;
    t->uv[2][0] = u - w;
    t->uv[2][1] = v - h;
    if (hits->hit != 0) {
        PANE_STATE(obj)->strength -= hits->damage;
        if (PANE_STATE(obj)->strength <= 0) {
            PANE_STATE(obj)->strength = 0;
            obj->interactObj->flags &= ~1;
            rl_m101_00000(obj, pane);
            func_80003410(0xD3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            if (pane->shards == NULL) {
                func_800079B8(obj);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m101", func_m101_00B64);
#endif

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
