/*
 * Runlink module 121: an object that breaks another object's model into
 * flying triangles. Init copies the triangles of the model's batches in a
 * group range into shards (one per triangle, each with its own position and
 * velocity in 16.16 fixed point), control moves them under gravity, bounces
 * them off a floor height and fades the object out, and the third function
 * frees the buffers.
 */
#include "module.h"
#include "gfx.h"

/* A batch of a model's triangles (size 0x10) */
typedef struct M121Batch {
    /* 0x00 */ u8 textureIndex; /* 0xFF: none */
    /* 0x01 */ s8 group;        /* also the bone of the first vertices */
    /* 0x02 */ s8 bone1;
    /* 0x03 */ s8 bone2;
    /* 0x04 */ s8 limit1; /* vertices below this use group's bone */
    /* 0x05 */ s8 limit2; /* below this bone1, else bone2 */
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA;
    /* 0x0B */ u8 texOffset;
    /* 0x0C */ u32 flags;
} M121Batch;

typedef struct M121Texture {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 flags; /* 0x40: animated */
    /* 0x08 */ u8 pad8[0x12 - 0x8];
    /* 0x12 */ u16 frames; /* frame count << 8 */
    /* 0x14 */ u8 pad14[0x18 - 0x14];
    /* 0x18 */ u8 loops;
} M121Texture;

typedef struct M121TextureRef {
    /* 0x00 */ M121Texture *texture;
    /* 0x04 */ u32 pad4;
} M121TextureRef;

/* A model triangle (size 0x10) */
typedef struct M121Triangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vi[3];
    /* 0x04 */ s16 uv[6];
} M121Triangle;

/* A texture animation slot (size 8) */
typedef struct M121TexAnim {
    /* 0x00 */ s16 frame;
    /* 0x02 */ s16 next;
    /* 0x04 */ u32 flags;
} M121TexAnim;

typedef struct M121Model {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 animatesTextures;
    /* 0x12 */ u8 pad12[0x16 - 0x12];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ M121TextureRef *textures;
    /* 0x1C */ u8 pad1C[0x20 - 0x1C];
    /* 0x20 */ M121Triangle *triangles;
    /* 0x24 */ M121Batch *batches;
    /* 0x28 */ M121TexAnim *texAnims;
} M121Model;

typedef struct M121Instance {
    /* 0x00 */ M121Model *model;
    /* 0x04 */ Vertex *vertices;
    /* 0x08 */ u8 pad8[0xB - 0x8];
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *matrices[2];
} M121Instance;

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
    /* 0x08 */ M121Triangle *tris; /* one per shard */
    /* 0x0C */ Gfx *displayList;
    /* 0x10 */ Object *target;
    /* 0x14 */ s32 floor; /* 16.16 */
    /* 0x18 */ u16 count;
    /* 0x1A */ s16 timer;
    /* 0x1C */ u8 pad1C[0x20 - 0x1C];
    /* 0x20 */ Vec3f centre;
} M121State;

extern void *func_8004A60C(s32 size, s32 colour); /* allocates */
extern void func_8004AA80(void *data);            /* frees */
extern s32 func_8003DDC8(void);
extern Gfx *func_8003F1B0(s32 *size); /* the scratch display list */
extern void func_80055B80(Gfx **dl);  /* resets the render settings */
extern void func_80055C00(Gfx **dl, M121Texture *tex, s32 flags, s32 texOffset); /* DKR material_set */
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern s32 func_80048E2C(s32 min, s32 max); /* random number */
extern f32 func_80097140(f32 x);            /* sqrtf */
extern f32 rl_m121_01030;                   /* shard speed scale (10000) */

#define gSPVertexM121(pkt, v, last, v0) \
    gDma1p(pkt, G_VTX, v, ((last) << 3) + ((last) << 1) + 8, (last) << 3 | ((u32) (v) & 6) | (v0))

#ifdef NON_MATCHING
static void func_m121_004D0(Gfx **dl, Object *obj, Object *target, s32 extraFlags, s32 clearFlags, s32 firstGroup,
                            s32 lastGroup);
#else
extern void rl_m121_004D0(Gfx **dl, Object *obj, Object *target, s32 extraFlags, s32 clearFlags, s32 firstGroup,
                          s32 lastGroup);
#define func_m121_004D0 rl_m121_004D0
#endif

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
                func_m121_004D0(&s->displayList, obj, setup->target, 6, 0, setup->firstGroup, setup->lastGroup);
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

/*
 * Builds the shards and the display list that draws them: every triangle of
 * the target's batches in the group range is transformed by its bone's
 * matrix into a vertex triple, its centre becomes the shard's position, and
 * the shard flies off along the triangle's normal at a random speed.
 */
#ifdef NON_MATCHING
/* equivalent; register allocation differs (the normal's FP temps, an `addiu s3,s3,0` induction artefact) */
static void func_m121_004D0(Gfx **dl, Object *obj, Object *target, s32 extraFlags, s32 clearFlags, s32 firstGroup,
                            s32 lastGroup) {
    M121Instance *inst;
    M121Model *model;
    s32 n;
    M121State *s;
    M121Batch *batch;
    M121Shard *shard;
    M121Triangle *triOut;
    M121Vertex *verts;
    M121Texture *prevTex;
    M121TexAnim *anim;
    s32 prevOffset;
    Gfx *gfx;
    Gfx *start;
    s32 i;
    M121Texture *tex;
    s32 texOffset;
    s32 first;
    s32 texIndex;
    Vertex *v;
    s32 flags;
    s32 vi;
    s32 bone;
    s32 hasTexture;
    s32 k;
    M121Vertex *vtxStart;
    M121Triangle *tri;
    M121Triangle *triCursor;
    s32 sx;
    f32 ox;
    f32 oy;
    f32 oz;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 x3;
    f32 y3;
    f32 z3;
    s32 sy;
    s32 sz;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 len;
    f32 speed;
    s32 count;
    Gfx *src;
    s32 size;
    s32 vtxCount;
    s32 renderFlags;

    inst = ((M121Instance **) target->modelInstances)[target->modelIndex];
    model = inst->model;
    n = model->numberOfBatches;
    s = obj->unk68;
    batch = model->batches;
    verts = s->verts;
    triOut = s->tris;
    shard = s->shards;
    prevTex = (M121Texture *) -1;
    anim = model->texAnims;
    prevOffset = -1;
    flags = 0;
    vtxCount = 0;
    vtxStart = verts;
    triCursor = triOut;
    renderFlags = func_8003DDC8();
    gfx = start = func_8003F1B0(&size);
    clearFlags = ~clearFlags;
    func_80055B80(&gfx);
    while (n--) {
        if (batch->group >= firstGroup && batch->group <= lastGroup && !(batch->flags & 0x400)) {
            texIndex = batch->textureIndex;
            i = 0;
            if (texIndex == 0xFF) {
                texOffset = 0;
                tex = NULL;
                hasTexture = FALSE;
            } else {
                tex = model->textures[texIndex].texture;
                texOffset = batch->texOffset << 14;
                hasTexture = TRUE;
            }
            flags = (flags | extraFlags | renderFlags) & clearFlags;
            if (model->animatesTextures && tex != NULL) {
                if (tex != prevTex || texOffset != prevOffset) {
                    anim->frame = texOffset;
                    anim->next = -1;
                    anim->flags = (batch->flags & ~0xFF) | texIndex;
                    if (tex->flags & 0x40) {
                        anim->next = texOffset + 0x100;
                        if (anim->next >= tex->frames) {
                            if (tex->loops) {
                                anim->next = 0;
                            } else {
                                anim->next = anim->next - 0x100;
                            }
                        }
                    }
                    anim++;
                }
            }
            prevTex = tex;
            prevOffset = texOffset;
            func_80055C00(&gfx, tex, flags, texOffset);
            first = batch->facesOffset;
            tri = &model->triangles[first];
            for (; i < batch[1].facesOffset - first; i++) {
                sx = 0;
                sy = 0;
                sz = 0;
                for (k = 0; k < 3; k++) {
                    vi = tri->vi[k];
                    v = &inst->vertices[vi + batch->verticesOffset];
                    if (vi < batch->limit1) {
                        bone = batch->group;
                    } else if (vi < batch->limit2) {
                        bone = batch->bone1;
                    } else {
                        bone = batch->bone2;
                    }
                    func_80048B60(&inst->matrices[inst->matrixIndex][bone], v->x, v->y, v->z, &ox, &oy, &oz);
                    sx += ox;
                    sy += oy;
                    sz += oz;
                    verts->x = ox;
                    verts->y = oy;
                    verts->z = oz;
                    verts->r = v->r;
                    verts->g = v->g;
                    verts->b = v->b;
                    verts->a = v->a;
                    verts++;
                }
                triOut->flags = 0x40;
                triOut->vi[0] = vtxCount;
                triOut->vi[1] = vtxCount + 1;
                triOut->vi[2] = vtxCount + 2;
                vtxCount += 3;
                triOut->uv[0] = tri->uv[0];
                triOut->uv[1] = tri->uv[1];
                triOut->uv[2] = tri->uv[2];
                triOut->uv[3] = tri->uv[3];
                triOut->uv[4] = tri->uv[4];
                triOut->uv[5] = tri->uv[5];
                triOut++;
                tri++;
                shard->x = (sx << 16) / 3;
                shard->y = (sy << 16) / 3;
                shard->z = (sz << 16) / 3;
                ox = verts[-3].x;
                oy = verts[-3].y;
                oz = verts[-3].z;
                x3 = verts[-1].x;
                y3 = verts[-1].y;
                z3 = verts[-1].z;
                x2 = verts[-2].x;
                y2 = verts[-2].y;
                z2 = verts[-2].z;
                nx = (y2 - oy) * (z3 - z2) - (z2 - oz) * (y3 - y2);
                ny = (z2 - oz) * (x3 - x2) - (x2 - ox) * (z3 - z2);
                nz = (x2 - ox) * (y3 - y2) - (y2 - oy) * (x3 - x2);
                len = func_80097140(nx * nx + ny * ny + nz * nz);
                if (len > 0.0f) {
                    nx /= len;
                    ny /= len;
                    nz /= len;
                }
                speed = func_80048E2C(50, 300) * rl_m121_01030;
                shard->vx = nx * speed;
                shard->vy = ny * speed;
                shard->vz = nz * speed;
                verts -= 3;
                k = 2;
                do {
                    shard->cornerX[k] = verts->x - (shard->x >> 16);
                    shard->cornerY[k] = verts->y - (shard->y >> 16);
                    shard->cornerZ[k] = verts->z - (shard->z >> 16);
                    verts++;
                } while (k--);
                shard++;
                if (vtxCount == 24) {
                    gSPVertexM121(gfx++, (u32) vtxStart & 0x0FFFFFFF, 24, 0);
                    k = 8;
                    while (k--) {
                        gSPPolygon(gfx++, (u32) triCursor & 0x0FFFFFFF, 1, hasTexture);
                        triCursor++;
                    }
                    vtxStart = verts;
                    vtxCount = 0;
                }
            }
            if (vtxCount != 0) {
                gSPVertexM121(gfx++, (u32) vtxStart & 0x0FFFFFFF, vtxCount, 0);
                k = vtxCount / 3;
                vtxCount = 0;
                vtxStart = verts;
                while (k--) {
                    gSPPolygon(gfx++, (u32) triCursor & 0x0FFFFFFF, 1, hasTexture);
                    triCursor++;
                }
            }
        }
        batch++;
    }
    src = gfx++;
    src->words.w0 = 0xE7000000;
    src->words.w1 = 0;
    src = gfx++;
    src->words.w0 = 0xB8000000;
    src->words.w1 = 0;
    count = gfx - start;
    gfx = *dl = func_8004A60C(count * sizeof(Gfx), 0xFF0000FF);
    if (gfx != NULL) {
        src = start;
        while (count--) {
            gfx->words.w0 = src->words.w0;
            gfx->words.w1 = src->words.w1;
            src++;
            gfx++;
        }
    }
}
#else
#undef func_m121_004D0
INCLUDE_ASM("asm/nonmatchings/modules/m121", func_m121_004D0);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m121", data);
