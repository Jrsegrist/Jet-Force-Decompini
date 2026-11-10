/*
 * Runlink module 30: builds the collision data of an object's models. For
 * the model the object's header names (or every model it has), it allocates
 * one 32 KiB block, fills it in, then shrinks the block to what was used and
 * keeps it in the model instance or in the model itself.
 */
#include "module.h"

typedef struct M030Header {
    /* 0x00 */ u8 pad0[0x1F];
    /* 0x1F */ s8 numberOfModelIds;
    /* 0x20 */ u8 pad20[0x71 - 0x20];
    /* 0x71 */ s8 modelIndex; /* the model to set up, or -1 for all of them */
} M030Header;

/* The collision data: a header followed by its tables in one block */
typedef struct M030Hit {
    /* 0x00 */ void *unk0;
    /* 0x04 */ void *unk4;
    /* 0x08 */ void *unk8;
    /* 0x0C */ void *unkC;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
} M030Hit;

/* The collision data's per-triangle entry (unk0): its plane and the planes on its edges */
typedef struct M030Edges {
    /* 0x00 */ u16 plane;
    /* 0x02 */ u16 edge[3]; /* | 0x8000 once linked back */
    /* 0x08 */ u16 next[3]; /* the triangle on each edge (itself if none) */
} M030Edges;

#define HIT_EDGES(hit) ((M030Edges *) (hit)->unk0)
#define HIT_FLAGS(hit) ((u8 *) (hit)->unk8)

/* A model vertex (10 bytes) */
typedef struct M030Vertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 pad6[4];
} M030Vertex;

/* A model triangle (16 bytes) */
typedef struct M030Triangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vertices[3]; /* relative to the batch's first vertex */
    /* 0x04 */ u8 pad4[0xC];
} M030Triangle;

/* A batch of triangles (16 bytes); the next batch's start ends it */
typedef struct M030Batch {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 vertex; /* the first vertex */
    /* 0x08 */ s16 start;  /* the first triangle */
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ s32 flags;
} M030Batch;

typedef struct M030Model {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ u8 pad18[0x1C - 0x18];
    /* 0x1C */ M030Vertex *vertices;
    /* 0x20 */ M030Triangle *triangles;
    /* 0x24 */ M030Batch *batches;
    /* 0x28 */ u8 pad28[0x4E - 0x28];
    /* 0x4E */ s8 unk4E;
    /* 0x4F */ u8 pad4F[0x60 - 0x4F];
    /* 0x60 */ M030Hit *hit;
} M030Model;

typedef struct M030ModelInstance {
    /* 0x00 */ M030Model *model;
    /* 0x04 */ M030Vertex *vertices;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ M030Hit *hit;
    /* 0x10 */ u8 pad10[0x8C - 0x10];
    /* 0x8C */ s16 minX; /* the model's bounds */
    /* 0x8E */ s16 minY;
    /* 0x90 */ s16 minZ;
    /* 0x92 */ s16 maxX;
    /* 0x94 */ s16 maxY;
    /* 0x96 */ s16 maxZ;
} M030ModelInstance;

extern void *func_8004A60C(s32 size, s32 colour); /* allocates */
extern void func_8004AA80(void *data);            /* frees */
extern void *func_8004A828(s32 size, void *address, s32 colour); /* allocates at an address */
extern u8 *func_8004AFE0(u8 *address);            /* aligns */
extern s32 mempool_free_timer_get(s32 size);
extern void func_8004AA08(s32 state);

#ifdef NON_MATCHING
static void model_hit_build(M030ModelInstance *inst, M030Model *model, M030Hit **slot);
static void hit_planes_build(M030ModelInstance *inst, M030Model *model, M030Hit *hit);
static void hit_edges_link(M030ModelInstance *inst, M030Model *model, M030Hit *hit);
static s32 hit_edges_compare(M030Vertex *v, s32 a, s32 b, s32 c, s32 d);
static void hit_cells_build(M030ModelInstance *inst, M030Model *model, M030Hit *hit);
static s32 hit_edge_find(M030ModelInstance *inst, M030Model *model, s32 tri, M030Vertex *p, M030Vertex *q);
s32 func_m030_01134(M030ModelInstance *inst, M030Model *model, s32 skip, s32 a, s32 b, s32 *outBatch,
                    s32 *outEdge);
#else
extern void rl_m030_000F4(M030ModelInstance *inst, M030Model *model, M030Hit **slot);
#define model_hit_build rl_m030_000F4
extern s32 rl_m030_012F0(M030Vertex *v, s32 a, s32 b, s32 c, s32 d);
#define hit_edges_compare rl_m030_012F0
#endif

void func_m030_00000(Object *obj, s32 flags) {
    M030ModelInstance *inst;
    M030Model *model;
    s32 i;

    if (((M030Header *) obj->header)->modelIndex >= 0) {
        inst = ((M030ModelInstance **) obj->modelInstances)[((M030Header *) obj->header)->modelIndex];
        model = inst->model;
        if (flags & 4) {
            model_hit_build(inst, model, &inst->hit);
        } else {
            model_hit_build(inst, model, &model->hit);
        }
        return;
    }
    for (i = 0; i < ((M030Header *) obj->header)->numberOfModelIds; i++) {
        inst = ((M030ModelInstance **) obj->modelInstances)[i];
        model = inst->model;
        if (flags & 4) {
            model_hit_build(inst, model, &inst->hit);
        } else {
            model_hit_build(inst, model, &model->hit);
        }
    }
}

#ifdef NON_MATCHING
/* Register choice: the ROM keeps ptr in a3 (as end), this gives v1 */
static void model_hit_build(M030ModelInstance *inst, M030Model *model, M030Hit **slot) {
    M030Hit *block;
    s32 n;
    u8 *end;
    s32 p;
    M030Hit *hit;
    u8 *ptr;
    s32 state;

    if (model->unk4E == 0 && *slot == NULL) {
        block = func_8004A60C(0x8000, 0xFF0000FF);
        if (block != NULL) {
            p = (s32) block;
            n = 0x2000;
            while (n != 0) {
                n--;
                *(u32 *) p = 0;
                p += 4;
            }
            *slot = block;
            block->unk0 = (u8 *) block + 0x14;
            ptr = (u8 *) block + model->unk14 * 14;
            ptr = func_8004AFE0(ptr + 0x14);
            (*slot)->unk4 = ptr;
            ptr += model->unk14 * 4;
            (*slot)->unk8 = ptr;
            end = func_8004AFE0(ptr + model->unk14);
            hit_edges_link(inst, model, *slot);
            hit = *slot;
            hit->unkC = end;
            p = hit->unk10 + hit->unk12;
            end += p * 16;
            if (end - (u8 *) block > 0x8000) {
                func_8004AA80(*slot);
                *slot = NULL;
                return;
            }
            state = mempool_free_timer_get(end - (u8 *) block);
            func_8004AA08(0);
            func_8004AA80(block);
            func_8004A828(end - (u8 *) block, block, 0xFF0000FF);
            func_8004AA08(state);
            hit_planes_build(inst, model, *slot);
            hit_cells_build(inst, model, *slot);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_000F4);
#endif

/* A plane of the collision data (unkC) */
typedef struct M030Plane {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 d;
} M030Plane;

#define HIT_PLANES(hit) ((M030Plane *) (hit)->unkC)

extern f32 func_80097140(f32 x); /* square root */

#ifdef NON_MATCHING
/* register allocation (the ROM keeps more of the coordinates in stack slots) */
/*
 * The triangles' planes, then one plane along each edge shared with another
 * triangle (and the edge flags).
 */
static void hit_planes_build(M030ModelInstance *inst, M030Model *model, M030Hit *hit) {
    s32 i;
    s32 j;
    s32 k;
    s32 k1;
    s32 k2;
    s32 m;
    s32 n;
    s32 numPlanes;
    s32 start;
    s32 end;
    s32 base;
    u16 self;
    u16 other;
    s32 c;
    f32 xa;
    f32 ya;
    f32 za;
    f32 xb;
    f32 yb;
    f32 zb;
    f32 xc;
    f32 yc;
    f32 zc;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 len;
    f32 s;
    M030Vertex *v;
    M030Plane *ps;
    M030Plane *po;

    n = 0;
    for (i = 0; i < model->numBatches; i++) {
        start = model->batches[i].start;
        end = model->batches[i + 1].start;
        base = model->batches[i].vertex;
        for (j = start; j < end; j++) {
            if (model->triangles[j].flags & 0x80) {
                continue;
            }
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
            dy = (yb - ya) * (zc - zb) - (zb - za) * (yc - yb);
            nx = dy;
            dx = xb - xa;
            dz = (zb - za) * (xc - xb) - dx * (zc - zb);
            ny = dz;
            nz = dx * (yc - yb) - (yb - ya) * (xc - xb);
            len = func_80097140(dy * dy + dz * dz + nz * nz);
            if (len > 0.0f) {
                nx = dy / len;
                ny = dz / len;
                nz /= len;
            }
            HIT_PLANES(hit)[n].x = nx;
            HIT_PLANES(hit)[n].y = ny;
            HIT_PLANES(hit)[n].z = nz;
            HIT_PLANES(hit)[n].d = -(xa * nx + ya * ny + za * nz);
            n++;
            HIT_EDGES(hit)[j].edge[0] = HIT_EDGES(hit)[j].next[0];
            HIT_EDGES(hit)[j].edge[1] = HIT_EDGES(hit)[j].next[1];
            HIT_EDGES(hit)[j].edge[2] = HIT_EDGES(hit)[j].next[2];
            HIT_FLAGS(hit)[j] &= 0xF8;
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
            if (model->triangles[j].flags & 0x80) {
                continue;
            }
            self = HIT_EDGES(hit)[j].plane;
            ps = &HIT_PLANES(hit)[self];
            nx = ps->x;
            ny = ps->y;
            nz = ps->z;
            for (k = 0; k < 3; k++) {
                k1 = k + 1;
                if (k1 >= 3) {
                    k1 = 0;
                }
                k2 = k1 + 1;
                if (k2 >= 3) {
                    k2 = 0;
                }
                other = HIT_EDGES(hit)[j].edge[k];
                c = model->triangles[j].vertices[k2] + base;
                if (other < numPlanes) {
                    po = &HIT_PLANES(hit)[other];
                    v = &inst->vertices[model->triangles[j].vertices[k] + base];
                    ya = v->y;
                    xa = v->x;
                    za = v->z;
                    v = &inst->vertices[model->triangles[j].vertices[k1] + base];
                    xb = v->x;
                    yb = v->y;
                    zb = v->z;
                    xc = (po->x + nx) * 10.0f + xa;
                    yc = (po->y + ny) * 10.0f + ya;
                    zc = (po->z + nz) * 10.0f + za;
                    dy = yb - ya;
                    dz = zb - za;
                    dx = xb - xa;
                    xc -= xb;
                    zc -= zb;
                    {
                        f32 ex = dy * zc - dz * (yc - yb);
                        f32 ey = dz * xc - dx * zc;
                        f32 ez = dx * (yc - yb) - dy * xc;

                        len = func_80097140(ex * ex + ey * ey + ez * ez);
                        if (len > 0.0f) {
                            ez /= len;
                            ex /= len;
                            ey /= len;
                        }
                        if (other != self) {
                            for (m = 0; m < 3; m++) {
                                if (HIT_EDGES(hit)[other].edge[m] == self) {
                                    HIT_EDGES(hit)[other].edge[m] = n | 0x8000;
                                }
                            }
                            v = &inst->vertices[c];
                            if (po->d + (v->x * po->x + v->y * po->y + v->z * po->z) < 0.0f) {
                                HIT_FLAGS(hit)[j] |= 1 << k;
                            }
                            if (po->z * ps->z + (ps->x * po->x + ps->y * po->y) < 0.0f) {
                                s = dy;
                                if (yb < ya) {
                                    s = -dy;
                                }
                                if (s <= func_80097140(dx * dx + dz * dz)) {
                                    model->triangles[j].flags |= (1 << k) << 2;
                                }
                            }
                        } else {
                            HIT_FLAGS(hit)[j] |= 1 << k;
                        }
                        HIT_EDGES(hit)[j].edge[k] = n;
                        HIT_PLANES(hit)[n].x = ex;
                        HIT_PLANES(hit)[n].y = ey;
                        HIT_PLANES(hit)[n].z = ez;
                        HIT_PLANES(hit)[n].d = -(xa * ex + ya * ey + za * ez);
                        n++;
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_00288);
#endif


#ifdef NON_MATCHING
/* frame and register allocation (the ROM spills j * 14 and keeps the out pointers in s2/s4) */
/* Links each triangle to the triangles on its edges and numbers the edge planes */
static void hit_edges_link(M030ModelInstance *inst, M030Model *model, M030Hit *hit) {
    s32 n;
    s32 i;
    s32 j;
    s32 base;
    s32 k;
    s32 end;
    s32 k2;
    s32 start;
    s32 r;
    s32 m;
    u16 self;
    u16 other;
    s32 outEdge;
    s32 outBatch;
    s32 numPlanes;

    n = 0;
    for (i = 0; i < model->numBatches; i++) {
        start = model->batches[i].start;
        end = model->batches[i + 1].start;
        base = model->batches[i].vertex;
        for (j = start; j < end; j++) {
            if (!(model->triangles[j].flags & 0x80)) {
                HIT_EDGES(hit)[j].plane = j;
                HIT_FLAGS(hit)[j] = 0;
                for (k = 0; k < 3; k++) {
                    k2 = k + 1;
                    if (k2 >= 3) {
                        k2 = 0;
                    }
                    r = func_m030_01134(inst, model, j, model->triangles[j].vertices[k] + base,
                                        model->triangles[j].vertices[k2] + base, &outBatch, &outEdge);
                    if (r != -1) {
                        HIT_EDGES(hit)[j].next[k] = r;
                    } else {
                        HIT_EDGES(hit)[j].next[k] = j;
                    }
                }
                n++;
                HIT_EDGES(hit)[j].edge[0] = HIT_EDGES(hit)[j].next[0];
                HIT_EDGES(hit)[j].edge[1] = HIT_EDGES(hit)[j].next[1];
                HIT_EDGES(hit)[j].edge[2] = HIT_EDGES(hit)[j].next[2];
            }
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
            if (!(model->triangles[j].flags & 0x80)) {
                self = HIT_EDGES(hit)[j].plane;
                for (k = 0; k < 3; k++) {
                    k2 = k + 1;
                    if (k2 >= 3) {
                        k2 = 0;
                    }
                    other = HIT_EDGES(hit)[j].edge[k];
                    if (other < numPlanes) {
                        if ((model->triangles[j].flags & 1) && (model->triangles[other].flags & 2)) {
                            r = hit_edge_find(inst, model, other,
                                                &model->vertices[model->triangles[j].vertices[k] + base],
                                                &model->vertices[model->triangles[j].vertices[k2] + base]);
                            model->triangles[j].flags &= 0xFFFE;
                            HIT_FLAGS(hit)[other] |= 1 << (r + 3);
                        } else if ((model->triangles[j].flags & 2) && (model->triangles[other].flags & 1)) {
                            model->triangles[other].flags &= ~1;
                            HIT_FLAGS(hit)[j] |= 1 << (k + 3);
                        }
                        if (other != self) {
                            for (m = 0; m < 3; m++) {
                                if (HIT_EDGES(hit)[other].edge[m] == self) {
                                    HIT_EDGES(hit)[other].edge[m] = n | 0x8000;
                                }
                            }
                        }
                        HIT_EDGES(hit)[j].edge[k] = n;
                        n++;
                    }
                }
            }
        }
    }
    hit->unk10 = numPlanes;
    hit->unk12 = n - numPlanes;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_00C14);
#endif


/*
 * Finds the triangle (other than skip) that has the edge (a, b): returns it
 * and gives its batch and the edge, or returns -1.
 */
s32 func_m030_01134(M030ModelInstance *inst, M030Model *model, s32 skip, s32 a, s32 b, s32 *outBatch,
                    s32 *outEdge) {
    s32 i;
    s32 end;
    s32 j;
    s32 k;
    s32 k2;
    s32 start;
    s32 base;
    M030Batch *pad; /* frame */
    s32 v2;

    for (i = 0; i < model->numBatches; i++) {
        start = model->batches[i].start;
        base = model->batches[i].vertex;
        end = model->batches[i + 1].start;
        if (model->batches[i].flags & 0x880) {
            end = start - 1;
        }
        for (j = start; j < end; j++) {
            if (!(model->triangles[j].flags & 0x80) && j != skip) {
                for (k = 0; k < 3; k++) {
                    k2 = k + 1;
                    if (k2 >= 3) {
                        k2 = 0;
                    }
                    if (hit_edges_compare(inst->vertices, a, b, model->triangles[j].vertices[k] + base,
                                        v2 = model->triangles[j].vertices[k2] + base)) {
                        *outEdge = k;
                        *outBatch = i;
                        return j;
                    }
                }
            }
        }
    }
    return -1;
}

#ifdef NON_MATCHING
/* load order: the ROM loads the second vertex's y and z first */
/*
 * Compares edge (a, b) with edge (c, d): 1 if they use the same vertex
 * indices, 2 if their vertices are at the same places, else 0.
 */
static s32 hit_edges_compare(M030Vertex *v, s32 a, s32 b, s32 c, s32 d) {
    M030Vertex *pa;
    M030Vertex *pb;
    M030Vertex *pc;
    M030Vertex *pd;

    if ((a == c && b == d) || (a == d && b == c)) {
        return 1;
    }
    pc = &v[c];
    pa = &v[a];
    if (pc->x - 1 < pa->x && pc->x >= pa->x && pc->y <= pa->y && pc->y >= pa->y && pc->z <= pa->z &&
        pc->z >= pa->z) {
        pb = &v[b];
        pd = &v[d];
        if (pd->x <= pb->x && pd->x >= pb->x && pd->y <= pb->y && pd->y >= pb->y && pd->z <= pb->z &&
            pd->z >= pb->z) {
            return 2;
        }
    } else {
        pd = &v[d];
        if (pd->x <= pa->x && pd->x >= pa->x && pd->y <= pa->y && pd->y >= pa->y && pd->z <= pa->z &&
            pd->z >= pa->z) {
            pb = &v[b];
            if (pc->x - 1 < pb->x && pc->x >= pb->x && pc->y <= pb->y && pc->y >= pb->y && pc->z <= pb->z &&
                pc->z >= pb->z) {
                return 2;
            }
        }
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_012F0);
#endif

#ifdef NON_MATCHING
/* register choice: the ROM gives lo/hi/n a0/v0/v1 and the mask t0 */
/*
 * For each triangle, a mask of the cells of an 8x8x8 grid over the model's
 * bounds that its bounding box touches (x in bits 0-7, z in 8-15, y in 16-23).
 */
static void hit_cells_build(M030ModelInstance *inst, M030Model *model, M030Hit *hit) {
    M030Triangle *t;
    M030Vertex *v;
    u32 mask;
    s32 end;
    u32 bit;
    s32 i;
    s32 j;
    s32 k;
    s32 start;
    s32 base;
    s16 maxX;
    s16 minX;
    s16 maxY;
    s16 minY;
    s16 maxZ;
    s16 minZ;
    s16 x;
    s16 y;
    s16 z;
    s16 lo;
    s16 hi;
    s16 step;
    s16 n;

    for (i = 0; i < model->numBatches; i++) {
        start = model->batches[i].start;
        base = model->batches[i].vertex;
        end = model->batches[i + 1].start;
        if (model->batches[i].flags & 0x880) {
            end = start - 1;
        }
        for (j = start; j < end; j++) {
            if (model->triangles[j].flags & 0x80) {
                ((u32 *) hit->unk4)[j] = 0;
                continue;
            }
            maxX = -32000;
            maxY = -32000;
            maxZ = -32000;
            minX = 32000;
            minY = 32000;
            minZ = 32000;
            t = &model->triangles[j];
            mask = 0;
            for (k = 0; k < 3; k++) {
                v = &inst->vertices[t->vertices[k] + base];
                x = v->x;
                y = v->y;
                z = v->z;
                if (maxX < x) {
                    maxX = x;
                }
                if (x < minX) {
                    minX = x;
                }
                if (maxY < y) {
                    maxY = y;
                }
                if (y < minY) {
                    minY = y;
                }
                if (maxZ < z) {
                    maxZ = z;
                }
                if (z < minZ) {
                    minZ = z;
                }
            }
            bit = 1;
            lo = inst->minX;
            step = ((inst->maxX - inst->minX) >> 3) + 1;
            hi = step + inst->minX;
            for (n = 0; n < 8; n++) {
                if (hi >= minX && maxX >= lo) {
                    mask |= bit;
                }
                hi += step;
                lo += step;
                bit <<= 1;
            }
            lo = inst->minZ;
            step = ((inst->maxZ - inst->minZ) >> 3) + 1;
            hi = step + inst->minZ;
            for (n = 0; n < 8; n++) {
                if (hi >= minZ && maxZ >= lo) {
                    mask |= bit;
                }
                hi += step;
                lo += step;
                bit <<= 1;
            }
            lo = inst->minY;
            step = ((inst->maxY - inst->minY) >> 3) + 1;
            hi = step + inst->minY;
            for (n = 0; n < 8; n++) {
                if (hi >= minY && maxY >= lo) {
                    mask |= bit;
                }
                hi += step;
                lo += step;
                bit <<= 1;
            }
            ((u32 *) hit->unk4)[j] = mask;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_01508);
#endif

#ifdef NON_MATCHING
/* register allocation: the ROM keeps the result in ra and the batch vertex in t5 */
/* Finds which edge of triangle tri runs from p to q (either way round), or -1 */
static s32 hit_edge_find(M030ModelInstance *inst, M030Model *model, s32 tri, M030Vertex *p, M030Vertex *q) {
    M030Batch *batch;
    M030Triangle *t;
    M030Vertex *verts;
    M030Vertex *v1;
    M030Vertex *v2;
    s32 i;
    s32 k;
    s32 k2;
    s32 base;
    s32 result;
    s32 start;
    s32 end;
    s16 px;
    s16 py;
    s16 pz;
    s16 qx;
    s16 qy;
    s16 qz;
    s16 x1;
    s16 y1;
    s16 z1;
    s16 x2;
    s16 y2;
    s16 z2;

    px = p->x;
    py = p->y;
    pz = p->z;
    qx = q->x;
    qy = q->y;
    qz = q->z;
    result = -1;
    batch = model->batches;
    for (i = 0; i < model->numBatches; i++, batch++) {
        start = batch->start;
        end = batch[1].start;
        if (!(batch->flags & 0x880) && start <= tri && tri < end) {
            t = &model->triangles[tri];
            verts = inst->vertices;
            base = batch->vertex;
            for (k = 0; k < 3; k++) {
                k2 = k + 1;
                if (k2 >= 3) {
                    k2 = 0;
                }
                v1 = &verts[t->vertices[k] + base];
                v2 = &verts[t->vertices[k2] + base];
                x1 = v1->x;
                y1 = v1->y;
                z1 = v1->z;
                x2 = v2->x;
                y2 = v2->y;
                z2 = v2->z;
                if ((px == x1 && py == y1 && pz == z1 && qx == x2 && qy == y2 && qz == z2) ||
                    (px == x2 && py == y2 && pz == z2 && qx == x1 && qy == y1 && qz == z1)) {
                    result = k;
                    k = 3;
                }
            }
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m030", func_m030_018D0);
#endif
