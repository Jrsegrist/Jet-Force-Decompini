/*
 * Runlink module 66: a model whose vertices are bent and faded towards the
 * nearest light source every frame. It can follow another object along an
 * animation path; it keeps two copies of the model's vertices and writes one
 * while the other is rendered.
 */
#include "module.h"
#include "gfx.h"

/* An object field that object.h does not name yet */

/* A model's triangle batch (like DKR's TriangleBatchInfo, structs.h, but 0x10 bytes) */
typedef struct M066Batch {
    /* 0x00 */ u8 textureIndex; /* 0xFF: no texture */
    /* 0x01 */ u8 pad1[0x6 - 0x1];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ u32 flags; /* 0x400: hidden */
} M066Batch;

typedef struct M066Texture {
    /* 0x00 */ void *texture;
    /* 0x04 */ u8 pad4[4];
} M066Texture;

typedef struct M066Model {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ s16 numVertices;
    /* 0x14 */ u8 pad14[0x16 - 0x14];
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ M066Texture *textures;
    /* 0x1C */ Vertex *vertices;
    /* 0x20 */ Triangle *triangles;
    /* 0x24 */ M066Batch *batches;
} M066Model;

/* What obj->unk6C[obj->modelIndex] points at */
typedef struct M066ModelInstance {
    /* 0x00 */ M066Model *model;
    /* 0x04 */ Vertex *vertices; /* the vertices to render */
} M066ModelInstance;

typedef struct M066Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 y_rotation;
    /* 0x0C */ s16 x_rotation;
    /* 0x0E */ s16 z_rotation;
    /* 0x10 */ u8 pathIndex; /* 0xFF: none */
    /* 0x11 */ u8 alpha;
} M066Setup;

/* obj->unk68 */
typedef struct M066Data {
    /* 0x00 */ Vertex *vertices[2];
    /* 0x08 */ u8 buffer; /* the copy written last */
    /* 0x09 */ u8 pathIndex;
    /* 0x0A */ u8 alpha;
} M066Data;

/* An animation path (D_801045B0) */
typedef struct M066Path {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ Object *obj;
    /* 0x0C */ u8 padC[0x16 - 0xC];
    /* 0x16 */ u8 flags;
} M066Path;

/* What func_800218C4 returns: the light nearest to a point */
typedef struct M066Light {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ f32 x;
    /* 0x1C */ f32 y;
    /* 0x20 */ f32 z;
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0x44 - 0x28];
    /* 0x44 */ f32 intensity;
} M066Light;

extern M066Path **D_801045B0;

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern M066Light *func_800218C4(f32 x, f32 y, f32 z);
extern f32 func_80097140(f32 x); /* sqrtf */
extern void func_80041CA0(Gfx **dList, Mtx **mtx, Object *obj, f32 scaleY, f32 offsetY); /* DKR camera_push_model_mtx */
extern void func_80041DA0(Gfx **dList);
extern void func_80055C00(Gfx **dList, void *texture, s32 flags, s32 texOffset); /* DKR load_and_set_texture */

void func_m066_002AC(Object *obj, f32 x, f32 y, f32 z, f32 outer, f32 inner, f32 level);

void func_m066_00000(Object *obj, M066Setup *setup, s32 arg2) {
    M066Data *data = obj->unk68;
    M066ModelInstance *inst;
    s32 size;
    s32 i;
    s32 n;
    s16 *src;
    s16 *dst;

    obj->trans.rotation.y_rotation = setup->y_rotation;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->trans.rotation.z_rotation = setup->z_rotation;
    if (arg2 == 0) {
        inst = obj->unk6C[obj->modelIndex];
        if (inst != NULL) {
            size = inst->model->numVertices * sizeof(Vertex);
            i = 2;
            while (i--) {
                data->vertices[i] = func_8004A57C(size, -1);
                if (data->vertices[i] != NULL) {
                    n = (size + 1) >> 1;
                    src = (s16 *) inst->model->vertices;
                    dst = (s16 *) data->vertices[i];
                    while (n--) {
                        *dst++ = *src++;
                    }
                }
            }
        }
        data->buffer = 1;
    }
    data->pathIndex = setup->pathIndex;
    data->alpha = setup->alpha;
}

void func_m066_00120(Object *obj, s32 updateRate) {
    f32 x;
    f32 y;
    f32 z;
    f32 radius;
    f32 inner;
    f32 level;
    M066Data *data = obj->unk68;
    M066Path *path;
    Object *target;
    M066Light *light;

    if (data->pathIndex != 0xFF) {
        obj->trans.flags |= 0x400;
        path = D_801045B0[data->pathIndex];
        if (path != NULL && (path->flags & 1)) {
            target = path->obj;
            if (target != NULL) {
                obj->trans.rotation.y_rotation = target->trans.rotation.y_rotation;
                obj->trans.rotation.x_rotation = target->trans.rotation.x_rotation;
                obj->trans.rotation.z_rotation = target->trans.rotation.z_rotation;
                obj->trans.scale = obj->header->scale * target->trans.scale / target->header->scale;
                obj->trans.x_position = target->trans.x_position;
                obj->trans.y_position = target->trans.y_position;
                obj->trans.z_position = target->trans.z_position;
                obj->segmentID = target->segmentID;
                obj->trans.flags &= ~0x400;
            }
        }
    }
    if (!(obj->trans.flags & 0x400)) {
        light = func_800218C4(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        if (light != NULL) {
            radius = light->radius;
            x = light->x;
            y = light->y;
            z = light->z;
            inner = radius * 0.25f;
            level = light->intensity;
        } else {
            x = obj->trans.x_position;
            y = obj->trans.y_position;
            z = obj->trans.z_position;
            radius = 0.0f;
            inner = 0.0f;
            level = 0.0f;
        }
        func_m066_002AC(obj, x, y, z, radius, inner, level);
    }
}

#ifdef NON_MATCHING
/* Frame and spill slots match; the float register allocation of the rotation (ROM keeps tx/ty/z in
   caller-saved registers spilled to the stack, IDO here gives them f20-f30) and the loop differ. */
void func_m066_002AC(Object *obj, f32 x, f32 y, f32 z, f32 outer, f32 inner, f32 level) {
    f32 cosA;
    f32 sinA;
    f32 tx;
    f32 ty;
    f32 tz;
    f32 scale;
    f32 d;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distSq;
    f32 dist;
    f32 f;
    f32 nearFactor;
    f32 midFactor;
    f32 unused1;
    f32 unused2;
    f32 fadeFactor;
    f32 unused3;
    f32 unused4;
    M066ModelInstance *inst;
    M066Model *model;
    M066Data *data;
    Vertex *src;
    Vertex *dst;
    s32 n;

    data = obj->unk68;
    inst = obj->unk6C[obj->modelIndex];
    if (inst != NULL) {
        model = inst->model;
        if (model != NULL) {
            x -= obj->trans.x_position;
            y -= obj->trans.y_position;
            z -= obj->trans.z_position;
            cosA = func_80049C30(-obj->trans.rotation.y_rotation);
            sinA = func_80049C34(-obj->trans.rotation.y_rotation);
            tx = x * cosA + z * sinA;
            tz = z * cosA - x * sinA;
            cosA = func_80049C30(-obj->trans.rotation.x_rotation);
            sinA = func_80049C34(-obj->trans.rotation.x_rotation);
            ty = y * cosA - tz * sinA;
            tz = tz * cosA + y * sinA;
            cosA = func_80049C30(-obj->trans.rotation.z_rotation);
            sinA = func_80049C34(-obj->trans.rotation.z_rotation);
            x = tx * cosA - ty * sinA;
            y = ty * cosA + tx * sinA;
            z = tz;
            scale = obj->trans.scale;
            nearFactor = data->alpha / scale;
            if (inner < outer) {
                d = outer - inner;
                midFactor = nearFactor / d;
                fadeFactor = level / d;
            } else {
                fadeFactor = 0.0f;
                midFactor = 0.0f;
            }
            data->buffer ^= 1;
            dst = data->vertices[data->buffer];
            n = model->numVertices;
            src = model->vertices;
            inst->vertices = dst;
            obj->curVertData = dst;
            while (n--) {
                dz = 0.0f - z;
                dx = src->x * scale - x;
                dy = src->y * scale - y;
                distSq = dx * dx + dy * dy + dz * dz;
                if (src->z == 0) {
                    if (distSq < inner * inner) {
                        dst->a = level;
                    } else if (distSq < outer * outer) {
                        dst->a = (s32) ((outer - func_80097140(distSq)) * fadeFactor);
                    } else {
                        dst->a = 0;
                    }
                } else if (distSq < outer * outer) {
                    dist = func_80097140(distSq);
                    f = dist;
                    if (dist > 0.0f) {
                        if (dist < inner) {
                            f = nearFactor / dist;
                        } else {
                            f = (outer - dist) * midFactor / dist;
                        }
                    }
                    dst->x = src->x + (s32) (dx * f);
                    dst->y = src->y + (s32) (dy * f);
                    dst->z = dz * f;
                } else {
                    dst->x = src->x;
                    dst->z = 0;
                    dst->y = src->y;
                }
                src++;
                dst++;
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m066", func_m066_002AC);
#endif

/* gSPVertexJFG with the length field written the way this module's code computes it */
#define VTX_CMD(pkt, v, n, v0) \
    gDma1p(pkt, G_VTX, v, (((n) - 1) << 3) + (((n) - 1) << 1) + 8, ((n) - 1) << 3 | ((u32) (v) & 6) | (v0))

void func_m066_007B0(Gfx **dl, Mtx **mtx, Object *obj) {
    s32 numBatches;
    M066Data *data = obj->unk68;
    M066Model *model;
    Vertex *verts;
    void *texture;
    s32 texEnabled;
    M066Batch *batch;
    M066ModelInstance *inst;
    Vertex *v;
    Triangle *tris;
    s32 vOffset;
    s32 fOffset;

    inst = obj->unk6C[obj->modelIndex];
    model = inst->model;
    func_80041CA0(dl, mtx, obj, 1.0f, 0.0f);
    gDPNoParam((*dl)++, 0xBF);
    numBatches = model->numBatches;
    verts = data->vertices[data->buffer];
    batch = model->batches;
    if (verts == NULL) {
        verts = model->vertices;
    }
    if (batch->textureIndex == 0xFF) {
        texture = NULL;
        texEnabled = 0;
    } else {
        texture = model->textures[batch->textureIndex].texture;
        texEnabled = 1;
    }
    func_80055C00(dl, texture, 0x16, 0);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 0);
    while (numBatches--) {
        if (!(batch->flags & 0x400)) {
            vOffset = batch->verticesOffset;
            fOffset = batch->facesOffset;
            v = &verts[vOffset];
            tris = &model->triangles[fOffset];
            /* from here on: the counts minus one / the triangle count */
            vOffset = batch[1].verticesOffset - vOffset;
            fOffset = batch[1].facesOffset - fOffset;
            VTX_CMD((*dl)++, OS_K0_TO_PHYSICAL(v), vOffset + 1, 0);
            gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(tris), fOffset, texEnabled);
        }
        batch++;
    }
    func_80041DA0(dl);
    gDPPipeSync((*dl)++);
}
