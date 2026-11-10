/*
 * Runlink module 105: an object made of a row of segments that sags under the
 * player's weight (its model's vertices are moved up and down along its
 * length), with its collision data rebuilt as it moves.
 */
#include "module.h"

/* One point along the object's length (0x28 bytes) */
typedef struct M105Point {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ s32 vx;
} M105Point;

/* The object's data (its unk68) */
typedef struct M105Data {
    /* 0x00 */ M105Point *points;
    /* 0x04 */ s32 sound;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ s16 spacing;
    /* 0x2E */ s8 state;
    /* 0x2F */ u8 count; /* the last point */
    /* 0x30 */ M105Point pointData[1];
} M105Data;

/* What the object's unk5C points at */
typedef struct M105Model {
    /* 0x000 */ f32 mtx[2][4][4];
    /* 0x080 */ f32 invMtx[2][4][4];
    /* 0x100 */ u8 pad100[0x104 - 0x100];
    /* 0x104 */ f32 minY;
    /* 0x108 */ u8 pad108[0x110 - 0x108];
    /* 0x110 */ f32 maxY;
    /* 0x114 */ u8 pad114[0x12B - 0x114];
    /* 0x12B */ u8 mtxIndex;
    /* 0x12C */ u8 pad12C[0x132 - 0x12C];
    /* 0x132 */ u8 ridden;
    /* 0x133 */ u8 pad133;
    /* 0x134 */ struct M105Rider *rider;
} M105Model;

/* What stands on the object (the parts used here) */
typedef struct M105Rider {
    /* 0x00 */ f32 x;
    /* 0x04 */ u8 pad4[0x10 - 0x4];
    /* 0x10 */ f32 vy;
} M105Rider;

/* The object's set-up */
typedef struct M105Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 scale; /* in 64ths */
    /* 0x0D */ u8 yRotation; /* in 256ths of a turn */
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 padF;
    /* 0x10 */ u8 spacing;
} M105Setup;

/* A model vertex (10 bytes) */
typedef struct M105Vertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 pad6[4];
} M105Vertex;

typedef struct M105Joint {
    /* 0x00 */ u16 vertex;
    /* 0x02 */ u8 pad2[2];
} M105Joint;

/* The model's header */
typedef struct M105ModelHeader {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ s16 numVertices;
    /* 0x14 */ u8 pad14[0x16 - 0x14];
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ u8 pad18[0x1C - 0x18];
    /* 0x1C */ M105Vertex *vertices;
    /* 0x20 */ struct M105Triangle *triangles;
    /* 0x24 */ struct M105Batch *batches;
    /* 0x28 */ u8 pad28[0x2D - 0x28];
    /* 0x2D */ u8 numJoints;
    /* 0x2E */ u8 pad2E[0x30 - 0x2E];
    /* 0x30 */ M105Joint *joints;
} M105ModelHeader;

/* A model triangle (16 bytes) */
typedef struct M105Triangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vertices[3]; /* relative to the batch's first vertex */
    /* 0x04 */ u8 pad4[0xC];
} M105Triangle;

/* A batch of triangles (16 bytes); the next batch's start ends it */
typedef struct M105Batch {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 vertex; /* the first vertex */
    /* 0x08 */ s16 start;  /* the first triangle */
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ s32 flags;
} M105Batch;

/* The collision data: per triangle, its plane and the triangles on its edges */
typedef struct M105Edges {
    /* 0x00 */ u16 plane;
    /* 0x02 */ u16 edge[3]; /* the plane of each edge, | 0x8000 once linked back */
    /* 0x08 */ u16 next[3];
} M105Edges;

typedef struct M105Plane {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 d;
} M105Plane;

typedef struct M105Hit {
    /* 0x00 */ M105Edges *edges;
    /* 0x04 */ u32 *cells;
    /* 0x08 */ u8 *flags;
    /* 0x0C */ M105Plane *planes;
} M105Hit;

/* A model instance */
typedef struct M105ModelInstance {
    /* 0x00 */ M105ModelHeader *model;
    /* 0x04 */ M105Vertex *vertices; /* the set drawn */
    /* 0x08 */ u8 pad8[0xB - 0x8];
    /* 0x0B */ s8 vertexSet; /* the one drawn */
    /* 0x0C */ void *hit;
    /* 0x10 */ u8 pad10[0x18 - 0x10];
    /* 0x18 */ M105Vertex *vertexSets[2];
    /* 0x20 */ u8 pad20[0x8E - 0x20];
    /* 0x8E */ s16 minY;
    /* 0x90 */ u8 pad90[0x92 - 0x90];
    /* 0x92 */ s16 maxX;
    /* 0x94 */ s16 maxY;
} M105ModelInstance;

/* The object's f32 at 0x34 */
#define OBJ_F34(obj) (*(f32 *) &(obj)->pad30[0x34 - 0x30])

extern void func_80048B60(f32 *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* transforms a point */
extern f32 func_80049BEC(f32 base, s32 exponent);
extern void func_80003614(s32 handle);           /* stops a positioned sound */
extern void audspat_point_set_pitch(s32 handle, u8 pitch); /* sets a positioned sound's pitch */
extern f32 func_80097140(f32 x); /* square root */
#ifdef NON_MATCHING
static void collision_rebuild(M105ModelInstance *inst, M105ModelHeader *model, M105Hit *hit);
#else
extern void rl_m105_015F4(M105ModelInstance *inst, M105ModelHeader *model, M105Hit *hit);
#define collision_rebuild rl_m105_015F4
#endif

static f32 D_m105_01F00 = 1.0f / 255.0f;
static f32 D_m105_01F04 = 9.82f;
static f32 D_m105_01F08 = 0.9f;
static f32 D_m105_01F0C = 6.283185f;

#ifdef NON_MATCHING
/* FP registers: the ROM keeps mid in f16 with a copy in f12, so every float temp differs */
void func_m105_00000(Object *obj, M105Setup *setup) {
    M105ModelInstance *inst;
    M105Data *d;
    M105ModelHeader *model;
    M105Vertex *v;
    M105Vertex *va;
    M105Vertex *vb;
    s16 i;
    s16 j;
    s16 k;
    s16 maxX;
    s16 minY;
    s32 lo;
    s32 hi;
    s32 base;
    f32 mid;
    f32 f;

    f = setup->scale / 64.0f;
    obj->trans.scale = obj->header->scale * f;
    obj->trans.rotation.y_rotation = setup->yRotation << 8;
    obj->trans.rotation.x_rotation = 0;
    obj->trans.rotation.z_rotation = 0;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    maxX = 0;
    inst = ((M105ModelInstance **) obj->modelInstances)[obj->modelIndex];
    d = obj->unk68;
    model = inst->model;
    d->count = model->numJoints - 1;
    d->unk20 = setup->unkB;
    d->unk8 = setup->unkE * D_m105_01F00;
    d->unkC = (d->unk20 * D_m105_01F04) / d->count;
    d->state = 0;
    d->unk24 = setup->unkA;
    d->sound = 0;
    d->count = model->numJoints - 1;
    d->points = d->pointData;
    d->spacing = setup->spacing;
    for (i = 0; i < model->numJoints; i++) {
        M105Vertex *w = model->vertices + model->joints[i].vertex;

        d->points[i].vx = w->x;
        d->points[i].x = w->x;
        d->points[i].y = w->y;
        d->points[i].z = w->z;
    }
    lo = d->points[0].vx;
    hi = d->points[d->count].vx;
    base = d->spacing * d->count + lo;
    v = model->vertices;
    va = inst->vertexSets[0];
    vb = inst->vertexSets[1];
    for (j = 0; j < model->numVertices; j++, v++, va++, vb++) {
        if (hi < v->x) {
            vb->x = va->x = v->x - hi + base;
            if (maxX < va->x) {
                maxX = va->x;
            }
        }
    }
    for (i = 1; i <= d->count; i++) {
        v = model->vertices;
        va = inst->vertexSets[0];
        vb = inst->vertexSets[1];
        k = d->spacing * i + lo;
        for (j = 0; j < model->numVertices; j++, v++, va++, vb++) {
            if (v->x == d->points[i].vx) {
                va->x = k;
                vb->x = k;
            }
        }
        d->points[i].vx = k;
        d->points[i].x = k;
    }
    mid = (d->points[d->count].x - d->points[0].x) * 0.5f + d->points[0].x;
    f = d->count;
    for (i = 1; d->points[i].x < mid; i++) {
        d->points[i].unkC = d->unkC * f;
        d->points[i].unk10 = d->points[i].unkC / d->unk24 + d->points[i - 1].unk10;
        f -= 2.0f;
    }
    f = d->count;
    for (i = d->count - 1; mid <= d->points[i].x; i--) {
        d->points[i].unkC = d->unkC * f;
        d->points[i].unk10 = d->points[i].unkC / d->unk24 + d->points[i + 1].unk10;
        f -= 2.0f;
    }
    for (k = 0; k <= d->count; k++) {
        d->points[k].unk18 = d->points[k].y - d->points[k].unk10;
        d->points[k].unk1C = d->points[k].unk18;
    }
    minY = inst->minY;
    lo = d->points[0].vx;
    hi = d->points[d->count].vx;
    va = inst->vertexSets[0];
    v = model->vertices;
    vb = inst->vertexSets[1];
    for (i = 0; i < model->numVertices; i++, v++, va++, vb++) {
        if (va->x >= lo && va->x <= hi) {
            k = (va->x - lo) / d->spacing;
            va->y = v->y - (s32) d->points[k].unk10;
            vb->y = v->y - (s16) d->points[k].unk10;
            if (va->y < minY) {
                minY = va->y;
            }
        }
    }
    inst->vertices = inst->vertexSets[0];
    inst->minY = minY;
    inst->maxX = maxX;
    OBJ_F34(obj) = maxX * obj->trans.scale;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m105", func_m105_00000);
#endif

void func_m105_007E4(Object *obj, s32 updateRate) {
    s32 pad;
    M105ModelInstance *inst;
    M105Data *d;
    void *hit;
    M105Model *m;
    M105ModelHeader *model;
    M105Rider *rider;
    M105Vertex *v;
    s32 lo;
    s32 hi;
    f32 s;
    f32 speed;
    f32 dist;
    f32 r;
    s16 i;
    s16 k;
    s16 minY;
    s16 maxY;
    s16 pad3;
    s16 j;
    s32 pad2;

    d = obj->unk68;
    inst = ((M105ModelInstance **) obj->modelInstances)[obj->modelIndex];
    model = inst->model;
    hit = inst->hit;
    m = (M105Model *) obj->unk5C;
    rider = m->rider;
    for (i = 0; i <= d->count; i++) {
        d->points[i].unk1C = d->points[i].unk18;
    }
    if (d->state == 0 && m->ridden != 0 && d->points[0].x < rider->x && rider->x < d->points[d->count].x &&
        rider->vy < 0.0f) {
        d->state = 1;
        speed = -rider->vy / updateRate;
        d->unk10 = 0.0f;
        d->unk14 = 50.0f / d->unk20 * -rider->vy;
        d->unk18 = 0.0f;
        d->unk1C = 1.0f;
        s = (rider->x - d->points[0].x) / d->spacing;
        r = 50.0f * speed;
        for (i = 1; i < d->count; i++) {
            dist = (s16) s - i;
            if (dist < 0.0f) {
                dist = -dist;
            } else if (dist == 0.0f) {
                dist = 1.0f;
            }
            d->points[i].unkC = func_80049BEC(D_m105_01F08, dist) * r;
            d->points[i].unk14 = d->points[i].unk10 * d->points[i].unkC / d->unk24;
        }
        if (d->sound != 0) {
            func_80003614(d->sound);
        }
        func_80003410(0xBE, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, &d->sound);
        if (d->sound != 0) {
            d->unk28 = d->unk14 + d->unk14;
            audspat_point_set_pitch(d->sound, 100.0f - d->unk28);
        }
    }
    if (d->state == 1 || d->state == 2) {
        if (d->unk1C == 0.0f) {
            d->state = 3;
            return;
        }
        inst->vertexSet ^= 1;
        inst->vertices = inst->vertexSets[inst->vertexSet];
        d->unk10 += updateRate / 60.0f;
        d->unk1C = 0.0f;
        s = func_80049C34(d->unk14 * d->unk10 / D_m105_01F0C * 65536.0f);
        for (j = 1; j < d->count; j++) {
            if (s >= 0.0f && d->state == 2) {
                d->points[j].unk14 *= func_80049BEC(d->unk8, updateRate);
            }
            if (d->points[j].unk14 < 1.0f) {
                d->points[j].unk14 = 0.0f;
            }
            if (d->unk1C < d->points[j].unk14) {
                d->unk1C = d->points[j].unk14;
            }
            d->points[j].unk20 = d->points[j].unk10 + d->points[j].unk14 * s;
            d->points[j].unk18 = d->points[j].y - d->points[j].unk20;
        }
        minY = inst->minY;
        maxY = inst->maxY;
        hi = d->points[d->count].vx;
        lo = d->points[0].vx;
        for (i = 0; i < model->numVertices; i++) {
            v = &inst->vertices[i];
            if (v->x >= lo && v->x <= hi) {
                k = (v->x - lo) / d->spacing;
                v->y = model->vertices[i].y - (s16) d->points[k].unk20;
                if (v->y < minY) {
                    minY = v->y;
                } else if (maxY < v->y) {
                    maxY = v->y;
                }
            }
        }
        m->minY = minY * obj->trans.scale + obj->trans.y_position;
        m->maxY = maxY * obj->trans.scale + obj->trans.y_position;
        if (4.0f < d->unk1C) {
            if (s >= 0.0f && d->unk18 < 0.0f) {
                if (d->sound != 0) {
                    func_80003614(d->sound);
                }
                func_80003410(0xBE, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4,
                              &d->sound);
                if (d->sound != 0) {
                    d->unk28 *= func_80049BEC(d->unk8, updateRate);
                    audspat_point_set_pitch(d->sound, 100.0f - d->unk28);
                }
            } else if (s < d->unk18 && d->state == 1) {
                if (d->sound != 0) {
                    func_80003614(d->sound);
                }
                func_80003410(0xBF, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4,
                              &d->sound);
                if (d->sound != 0) {
                    d->unk28 *= func_80049BEC(d->unk8, updateRate);
                    audspat_point_set_pitch(d->sound, 100.0f - d->unk28);
                }
            }
        }
        if (s <= d->unk18) {
            d->state = 2;
        } else if (d->unk18 < s) {
            d->state = 1;
        }
        d->unk18 = s;
        collision_rebuild(inst, model, hit);
    } else if (d->state == 3) {
        if (m->ridden == 0) {
            d->state = 0;
        }
    }
}

#ifdef NON_MATCHING
/* the ROM keeps first in a stack slot and sets up the stack arguments before the loads */
/* Transforms a point into the object's space, moved with the object's sag */
void func_m105_01260(Object *obj, f32 *x, f32 *y, f32 *z) {
    M105Model *m;
    M105Data *d;
    f32 t;
    f32 a;
    M105Point *p;
    f32 ox;
    f32 oy;
    f32 oz;
    s32 i;
    f32 b;
    f32 spacing;
    f32 first;

    d = obj->unk68;
    m = (M105Model *) obj->unk5C;
    spacing = d->spacing;
    func_80048B60(m->mtx[m->mtxIndex][0], *x, *y, *z, &ox, &oy, &oz);
    first = d->points[0].x;
    if (first < ox && ox < d->points[d->count].x) {
        i = (ox - first) / spacing;
        p = &d->points[i];
        t = (ox - p->x) / spacing;
        a = p->unk1C + t * (p[1].unk1C - p->unk1C);
        b = p->unk18 + t * (p[1].unk18 - p->unk18);
        if (a < oy && d->state == 2) {
            oy += (b - a) + 1.0f;
        } else if (oy < a && d->state == 1) {
            oy += (b - a) - 1.0f;
        }
    }
    func_80048B60(m->invMtx[m->mtxIndex][0], ox, oy, oz, x, y, z);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m105", func_m105_01260);
#endif

#ifdef NON_MATCHING
/* the ROM keeps the two heights in stack slots (0x48/0x4C); this gives registers */
/* Moves a point held by the object with the object's sag */
void func_m105_01474(Object *obj, f32 *x, f32 *y, f32 *z, f32 *ox, f32 *oy, f32 *oz) {
    M105Model *m;
    M105Data *d;
    f32 a;
    f32 b;
    f32 t;
    M105Point *p;
    s32 i;
    f32 spacing;
    s32 pad1;
    s32 pad2;
    f32 first;

    d = obj->unk68;
    m = (M105Model *) obj->unk5C;
    spacing = d->spacing;
    first = d->points[0].x;
    if (first < *x && *x < d->points[d->count].x) {
        i = (*x - first) / spacing;
        p = &d->points[i];
        t = (*x - p->x) / spacing;
        a = p->unk1C + t * (p[1].unk1C - p->unk1C);
        b = p->unk18 + t * (p[1].unk18 - p->unk18);
        *y += b - a;
    }
    func_80048B60(m->invMtx[m->mtxIndex][0], *x, *y, *z, ox, oy, oz);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m105", func_m105_01474);
#endif

#ifdef NON_MATCHING
/* register allocation and induction variables differ throughout */
/* Rebuilds the collision data's planes and edge links from the moved vertices */
static void collision_rebuild(M105ModelInstance *inst, M105ModelHeader *model, M105Hit *hit) {
    f32 xa;
    f32 ya;
    f32 za;
    f32 xc;
    f32 yc;
    f32 zc;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 sz;
    s32 i;
    s32 j;
    s32 k;
    s32 k2;
    s32 m;
    s32 n;
    s32 numPlanes;
    s32 end;
    s32 start;
    s32 base;
    u16 self;
    u16 other;
    s32 a;
    s32 b;
    f32 len;
    f32 d;
    f32 xb;
    f32 yb;
    f32 zb;
    f32 px;
    f32 py;
    f32 pz;
    f32 qx;
    f32 qy;
    f32 qz;
    M105Vertex *v;
    M105Plane *ps;
    M105Plane *po;

    n = 0;
    for (i = 0; i < model->numBatches; i++) {
        start = model->batches[i].start;
        end = model->batches[i + 1].start;
        base = model->batches[i].vertex;
        for (j = start; j < end; j++) {
            v = &inst->vertices[model->triangles[j].vertices[0] + base];
            xa = v->x;
            ya = v->y;
            za = v->z;
            v = &inst->vertices[model->triangles[j].vertices[1] + base];
            xb = v->x;
            yb = v->y;
            zb = v->z;
            v = &inst->vertices[model->triangles[j].vertices[2] + base];
            xc = v->x;
            yc = v->y;
            zc = v->z;
            nx = (yb - ya) * (zc - zb) - (zb - za) * (yc - yb);
            ny = (zb - za) * (xc - xb) - (xb - xa) * (zc - zb);
            nz = (xb - xa) * (yc - yb) - (yb - ya) * (xc - xb);
            len = func_80097140(nx * nx + ny * ny + nz * nz);
            if (len > 0.0f) {
                nx /= len;
                ny /= len;
                nz /= len;
            }
            hit->planes[n].x = nx;
            hit->planes[n].y = ny;
            hit->planes[n].z = nz;
            hit->planes[n].d = -(xa * nx + ya * ny + za * nz);
            n++;
            hit->edges[j].edge[0] = hit->edges[j].next[0];
            hit->edges[j].edge[1] = hit->edges[j].next[1];
            hit->edges[j].edge[2] = hit->edges[j].next[2];
            hit->flags[j] &= 0xF8;
        }
    }
    numPlanes = n;
    for (i = 0; i < model->numBatches; i++) {
        start = model->batches[i].start;
        base = model->batches[i].vertex;
        end = model->batches[i + 1].start;
        if (model->batches[i].flags & 0x880) {
            end = start - 1;
        }
        for (j = start; j < end; j++) {
            self = hit->edges[j].plane;
            ps = &hit->planes[self];
            px = ps->x;
            py = ps->y;
            pz = ps->z;
            for (k = 0; k < 3; k++) {
                k2 = k + 1;
                if (k2 >= 3) {
                    k2 = 0;
                }
                other = hit->edges[j].edge[k];
                a = model->triangles[j].vertices[k] + base;
                b = model->triangles[j].vertices[k2] + base;
                if (other < numPlanes) {
                    po = &hit->planes[other];
                    nx = po->x + px;
                    ny = po->y + py;
                    sz = po->z + pz;
                    v = &inst->vertices[a];
                    ya = v->y;
                    xa = v->x;
                    za = v->z;
                    v = &inst->vertices[b];
                    xb = v->x;
                    zb = v->z;
                    yb = v->y;
                    xc = nx * 10.0f + xa;
                    yc = ny * 10.0f + ya;
                    zc = sz * 10.0f + za;
                    nx = (yb - ya) * (zc - zb) - (zb - za) * (yc - yb);
                    ny = (zb - za) * (xc - xb) - (xb - xa) * (zc - zb);
                    sz = (xb - xa) * (yc - yb) - (yb - ya) * (xc - xb);
                    len = func_80097140(nx * nx + ny * ny + sz * sz);
                    if (len > 0.0f) {
                        sz /= len;
                        nx /= len;
                        ny /= len;
                    }
                    d = -(xa * nx + ya * ny + za * sz);
                    if (other != self) {
                        for (m = 0; m < 3; m++) {
                            if (hit->edges[other].edge[m] == self) {
                                hit->edges[other].edge[m] = n | 0x8000;
                            }
                        }
                    }
                    hit->edges[j].edge[k] = n;
                    if (ps->x != po->x || ps->y != po->y || ps->z != po->z) {
                        qx = po->x * 5.0f + xa;
                        qy = po->y * 5.0f + ya;
                        qz = po->z * 5.0f + za;
                        xa += ps->x * 5.0f;
                        ya += ps->y * 5.0f;
                        za += ps->z * 5.0f;
                        if (xa * nx + ya * ny + za * sz + d < 0.0f && -(qx * nx + qy * ny + qz * sz + d) < 0.0f) {
                            hit->flags[j] |= 1 << k;
                        }
                    } else if (ps == po) {
                        hit->flags[j] |= 1 << k;
                    }
                    hit->planes[n].x = nx;
                    hit->planes[n].y = ny;
                    hit->planes[n].z = sz;
                    hit->planes[n].d = d;
                    n++;
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m105", func_m105_015F4);
#endif
