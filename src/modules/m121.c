/*
 * Runlink module 121: an object that breaks another object's model into
 * flying triangles. Init copies the triangles of the model's batches in a
 * group range into shards (one per triangle, each with its own position and
 * velocity in 16.16 fixed point), control moves them under gravity, bounces
 * them off a floor height and fades the object out, and the third function
 * frees the buffers.
 */
#include "module.h"

/* A batch of a model's triangles (size 0x10) */
typedef struct M121Batch {
    /* 0x00 */ u8 textureIndex; /* 0xFF: none */
    /* 0x01 */ s8 group;
    /* 0x02 */ s8 unk2;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ s8 unk4;
    /* 0x05 */ s8 unk5;
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ u32 flags;
} M121Batch;

typedef struct M121Model {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ u8 pad18[0x24 - 0x18];
    /* 0x24 */ M121Batch *batches;
} M121Model;

/* What init gets */
typedef struct M121Setup {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u8 firstGroup;
    /* 0x0B */ u8 lastGroup;
    /* 0x0C */ f32 floor;
    /* 0x10 */ Object *target; /* whose model breaks */
} M121Setup;

/* One flying triangle (size 0x2C) */
typedef struct M121Shard {
    /* 0x00 */ s32 x; /* 16.16 */
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
    /* 0x0C */ s32 vx;
    /* 0x10 */ s32 vy;
    /* 0x14 */ s32 vz;
    /* 0x18 */ s16 cornerX[3]; /* the corners, relative to the position */
    /* 0x1E */ s16 cornerY[3];
    /* 0x24 */ s16 cornerZ[3];
} M121Shard;

/* The vertices drawn (size 0xA) */
typedef struct M121Vertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} M121Vertex;

/* Kept in obj->unk68 */
typedef struct M121State {
    /* 0x00 */ M121Shard *shards;
    /* 0x04 */ M121Vertex *verts; /* three per shard */
    /* 0x08 */ void *tris;        /* one 0x10-byte triangle per shard */
    /* 0x0C */ void *displayList;
    /* 0x10 */ Object *target;
    /* 0x14 */ s32 floor; /* 16.16 */
    /* 0x18 */ u16 count;
    /* 0x1A */ s16 timer;
    /* 0x1C */ u8 pad1C[0x20 - 0x1C];
    /* 0x20 */ Vec3f centre;
} M121State;

extern void *func_8004A60C(s32 size, s32 colour); /* allocates */
extern void func_8004AA80(void *data);            /* frees */
extern void rl_m121_004D0(void **dl, Object *obj, Object *target, s32 arg3, s32 arg4, s32 firstGroup, s32 lastGroup);

void func_m121_00000(Object *obj, M121Setup *setup) {
    s32 pad;
    M121State *s;
    M121Model *model;
    M121Batch *batch;
    M121Shard *shard;
    s32 n;
    s32 x;
    s32 y;
    s32 z;
    Object *target;
    void **inst;

    target = setup->target;
    s = obj->unk68;
    inst = target->modelInstances[target->modelIndex];
    model = *inst;
    n = model->numberOfBatches;
    batch = model->batches;
    s->shards = NULL;
    s->verts = NULL;
    s->tris = NULL;
    s->displayList = NULL;
    s->target = setup->target;
    s->floor = setup->floor * 65536.0f;
    s->count = 0;
    s->timer = 200;
    x = 0;
    y = 0;
    z = 0;
    while (n--) {
        if (batch->group >= setup->firstGroup && batch->group <= setup->lastGroup && !(batch->flags & 0x400)) {
            s->count += batch[1].facesOffset - batch->facesOffset;
        }
        batch++;
    }
    if ((s->shards = func_8004A60C(s->count * sizeof(M121Shard), 0xFF0000FF)) != NULL) {
        if ((s->verts = func_8004A60C(s->count * 3 * sizeof(M121Vertex), 0xFF0000FF)) != NULL) {
            if ((s->tris = func_8004A60C(s->count * 0x10, 0xFF0000FF)) != NULL) {
                rl_m121_004D0(&s->displayList, obj, setup->target, 6, 0, setup->firstGroup, setup->lastGroup);
                if (s->displayList != NULL) {
                    shard = s->shards;
                    n = s->count;
                    /* (the first shard's position, count times) */
                    while (n--) {
                        x += shard->x;
                        y += shard->y;
                        z += shard->z;
                    }
                    s->centre.x = (f32) x / (u32) s->count;
                    s->centre.y = (f32) y / (u32) s->count;
                    s->centre.z = (f32) z / (u32) s->count;
                    return;
                }
            }
        }
    }
    func_800079B8(obj);
}

void func_m121_002D4(Object *obj, s32 updateRate) {
    M121State *s;
    M121Shard *shard;
    M121Vertex *v;
    s32 n;
    s32 j;

    s = obj->unk68;
    n = s->count;
    v = s->verts;
    shard = s->shards;
    while (n--) {
        shard->vy -= 30000;
        shard->x += shard->vx;
        shard->y += shard->vy;
        shard->z += shard->vz;
        if (shard->y < s->floor) {
            shard->y = s->floor;
            shard->vy = -(shard->vy >> 1);
            if (shard->vy == 0 || ((shard->x + shard->z) & 1)) {
                shard->vx = 0;
                shard->vy = 0;
                shard->vz = 0;
            }
        }
        j = 2;
        do {
            v->x = shard->cornerX[j] + (shard->x >> 16);
            v->y = shard->cornerY[j] + (shard->y >> 16);
            v->z = shard->cornerZ[j] + (shard->z >> 16);
            v++;
        } while (j--);
        shard++;
    }
    if (updateRate >= s->timer) {
        s->timer = 0;
        if (updateRate * 2 >= obj->opacity) {
            func_800079B8(obj);
        } else {
            obj->opacity -= updateRate * 2;
        }
    } else {
        s->timer -= updateRate * 2;
    }
}

void func_m121_00438(Object *obj) {
    M121State *s = obj->unk68;

    if (s->verts != NULL) {
        func_8004AA80(s->verts);
    }
    if (s->shards != NULL) {
        func_8004AA80(s->shards);
    }
    if (s->tris != NULL) {
        func_8004AA80(s->tris);
    }
    if (s->displayList != NULL) {
        func_8004AA80(s->displayList);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m121", func_m121_004D0);

INCLUDE_ASM("asm/nonmatchings/modules/m121", data);
