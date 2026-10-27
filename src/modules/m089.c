/*
 * Runlink module 89: electrocution: arcs that crawl over the vertices of
 * the model of the object being electrocuted, and a lookup of a model
 * vertex's world position.
 */
#include "module.h"

typedef struct M089Batch {
    /* 0x0 */ u8 pad0;
    /* 0x1 */ s8 bones[3];
    /* 0x4 */ s8 limits[2];
    /* 0x6 */ s16 firstVertex;
    /* 0x8 */ s16 firstTriangle;
    /* 0xA */ u8 padA[0xC - 0xA];
    /* 0xC */ s32 flags;
} M089Batch;

typedef struct M089Triangle {
    /* 0x0 */ u8 flags;
    /* 0x1 */ u8 vi[3];
    /* 0x4 */ u8 pad4[0x10 - 0x4];
} M089Triangle;

typedef struct M089Vertex {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ u8 pad6[0xA - 0x6];
} M089Vertex;

typedef struct M089Model {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 unk11;
    /* 0x12 */ s16 numVertices;
    /* 0x14 */ u8 pad14[0x16 - 0x14];
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
    /* 0x20 */ M089Triangle *triangles;
    /* 0x24 */ M089Batch *batches;
    /* 0x28 */ u8 pad28[0x4E - 0x28];
    /* 0x4E */ s8 unk4E;
} M089Model;

typedef struct M089Instance {
    /* 0x00 */ M089Model *model;
    /* 0x04 */ M089Vertex *vertices;
    /* 0x08 */ u8 pad8[0xA - 0x8];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *matrices[2];
} M089Instance;

typedef struct M089Setup {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Object *target;
} M089Setup;

typedef struct M089Shock {
    /* 0x00 */ Object *target;
    /* 0x04 */ u8 pad4[0x10 - 0x4];
    /* 0x10 */ s16 vertex[6];
    /* 0x1C */ s8 step[6];
    /* 0x22 */ u8 timer;
    /* 0x23 */ u8 done;
} M089Shock;

extern void func_800541B0(s32, s32, s32);
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern void func_8001191C(Object *obj, Object *target);
extern void func_8003D21C(M089Instance *, M089Model *, Object *, f32, f32);
extern void func_800221F0(Object *, M089Instance *, ObjectShading *, MtxF *);
extern void func_80009734(Object *, M089Model *, M089Instance *, s32);

#ifdef NON_MATCHING
s32 D_m089_00460 = 89; /* the module's number */
#else
/* the module's number, followed by the format string of the vertex lookup's stubbed print */
extern s32 D_m089_00460;
INCLUDE_ASM("asm/nonmatchings/modules/m089", data);
#endif

#ifdef NON_MATCHING
/*
 * The world position of vertex `index` of obj's model; 0 if it has none.
 * Equivalent; only the evaluation order of the transform call's arguments
 * and the operand order of the index compare differ.
 */
s32 func_m089_00000(Object *obj, s32 index, f32 *x, f32 *y, f32 *z) {
    M089Instance *inst = ((M089Instance **) obj->unk6C)[obj->modelIndex];
    M089Model *model = inst->model;
    s32 n = model->numBatches;
    M089Batch *batch = model->batches;
    M089Triangle *tri;
    M089Vertex *v;
    s32 count;
    s32 i;
    s32 k;
    s32 raw;
    s32 vi;
    s32 bone;

    if (obj->unkA0 != 0) {
        *x = obj->trans.x_position;
        *y = obj->trans.y_position;
        *z = obj->trans.z_position;
        return 0;
    }
    while (n--) {
        if (!(batch->flags & 0x400)) {
            tri = &model->triangles[batch->firstTriangle];
            for (i = 0; i < batch[1].firstTriangle - batch->firstTriangle; i++, tri++) {
                for (k = 0; k < 3; k++) {
                    if ((raw = tri->vi[k]) + batch->firstVertex == index) {
                        vi = raw + batch->firstVertex;
                        v = &inst->vertices[vi];
                        if (raw < batch->limits[0]) {
                            bone = batch->bones[0];
                        } else if (raw < batch->limits[1]) {
                            bone = batch->bones[1];
                        } else {
                            bone = batch->bones[2];
                        }
                        func_80048B60(&inst->matrices[inst->matrixIndex][bone], v->x, v->y, v->z, x, y, z);
                        return 1;
                    }
                }
            }
        }
        batch++;
    }
    stubbed_printf("GVP:%x, %s, %d\n", obj, NULL, index); /* only its format string is in the ROM */
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m089", func_m089_00000);
#endif

void func_m089_001D0(Object *obj, M089Setup *setup) {
    M089Shock *s = obj->unk68;

    s->target = setup->target;
    s->timer = 0;
    func_8001191C(obj, s->target);
}

void func_m089_00200(Object *obj, s32 updateRate) {
    M089Shock *s = obj->unk68;
    Object *target;
    M089Instance *inst;
    M089Model *model;
    s32 i;

    func_800541B0(D_m089_00460, 2, 2);
    if (s->done) {
        func_800079B8(obj);
        return;
    }
    target = s->target;
    inst = ((M089Instance **) target->unk6C)[target->modelIndex];
    model = inst->model;
    if (target->behaviorId != 0x18 && inst->unkA != 0) {
        if (model->unk4E != 0) {
            func_8003D21C(inst, model, obj, 1.0f, 0.0f);
            func_800221F0(obj, inst, obj->shading, inst->matrices[inst->matrixIndex]);
        } else {
            if (model->unk11 != 0) {
                inst->matrixIndex = 1 - inst->matrixIndex;
            }
            func_800221F0(obj, inst, obj->shading, inst->matrices[inst->matrixIndex]);
        }
        if (model->unk11 > 0) {
            func_80009734(obj, model, inst, inst->unkA);
        }
        inst->unkA = 0;
    }
    if (s->timer == 0) {
        s->timer = func_80048E2C(5, 18);
        i = 5;
        do {
            s->vertex[i] = func_80048E2C(0, model->numVertices - 1);
            if (func_80048E2C(0, 1) != 0) {
                s->step[i] = 1;
            } else {
                s->step[i] = -1;
            }
        } while (i--);
    }
    i = 5;
    do {
        s->vertex[i] += s->step[i];
        if (model->numVertices < s->vertex[i]) {
            s->vertex[i] = 0;
        }
        if (s->vertex[i] < 0) {
            s->vertex[i] = model->numVertices - 1;
        }
    } while (i--);
    s->timer--;
}
