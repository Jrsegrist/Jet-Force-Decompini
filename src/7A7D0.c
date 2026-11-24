/*
 * Object collision: collision spheres, pushing objects apart, line tests and box cells.
 */
#include "common.h"

typedef struct Vec3f {
    f32 x, y, z;
} Vec3f;

typedef struct Obj7EHeader {
    /* 0x00 */ u8 pad0[0x71];
    /* 0x71 */ s8 unk71;
} Obj7EHeader;

typedef struct Obj7EData {
    /* 0x000 */ f32 mtx[2][4][4];  // JFG: double-buffered inverse transform
    /* 0x080 */ f32 mtx2[2][4][4]; // JFG: double-buffered transform
    /* 0x100 */ f32 bboxMin[3];
    /* 0x10C */ f32 bboxMax[3];
    /* 0x118 */ f32 prevX;
    /* 0x11C */ f32 prevY;
    /* 0x120 */ f32 prevZ;
    /* 0x124 */ s16 prevRoll;
    /* 0x126 */ s16 prevPitch;
    /* 0x128 */ s16 prevYaw;
    /* 0x12A */ u8 unk12A;
    /* 0x12B */ u8 mtxIndex;
    /* 0x12C */ s16 unk12C;
    /* 0x12E */ s16 unk12E;
    /* 0x130 */ s16 unk130;
    /* 0x132 */ u8 unk132;
    /* 0x133 */ u8 modelIndex;
} Obj7EData;

typedef struct Obj7E {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ s16 pitch;
    /* 0x04 */ s16 roll;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x2E - 0x18];
    /* 0x2E */ s16 segmentID;
    /* 0x30 */ u8 pad30[0x3A - 0x30];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B[0x40 - 0x3B];
    /* 0x40 */ Obj7EHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 type;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ struct Obj7EFlags *flags;
    /* 0x50 */ u8 pad50[0x5C - 0x50];
    /* 0x5C */ Obj7EData *data;
    /* 0x60 */ u8 pad60[0x68 - 0x60];
    /* 0x68 */ struct Obj7EState *state;
    /* 0x6C */ struct Obj7EModel **modelInstances;
} Obj7E;

/* JFG: one collision sphere of a model: centred on a vertex (0xC bytes) */
typedef struct ModelSphereDef {
    /* 0x00 */ u16 vertex;
    /* 0x02 */ u8 pad2[0x8 - 0x2];
    /* 0x08 */ f32 radius;
} ModelSphereDef;

/* JFG: a model triangle (0x10 bytes, as DKR's Triangle) */
typedef struct Triangle7E {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vi[3];
    /* 0x04 */ s16 uv[3][2];
} Triangle7E;

/* JFG: a model's triangle batch (0x10 bytes; only the fields used here) */
typedef struct ModelBatch7E {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset; // the batch's first triangle (DKR facesOffset)
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ u32 flags;
} ModelBatch7E;

/* JFG: a 14-byte record per triangle of a model's collision edge info (only the plane index is used here) */
typedef struct EdgeRecord7E {
    /* 0x00 */ u16 plane; // the triangle's plane in ModelEdges7E.planes
    /* 0x02 */ u8 pad2[12];
} EdgeRecord7E;

/* JFG: a plane a*x + b*y + c*z + d = 0 */
typedef struct Plane7E {
    /* 0x00 */ f32 a, b, c, d;
} Plane7E;

/* JFG: a model's collision edge info (only the fields used here) */
typedef struct ModelEdges7E {
    /* 0x00 */ EdgeRecord7E *records; // per triangle
    /* 0x04 */ u32 *cellMasks;        // JFG: per triangle, the box cells it touches (as box_range_cell_mask)
    /* 0x08 */ u8 *edgeFlags; // JFG: per triangle, bit n = edge n is a collision edge
    /* 0x0C */ Plane7E *planes;
} ModelEdges7E;

typedef struct ModelHeader7E {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
    /* 0x20 */ Triangle7E *triangles;
    /* 0x24 */ ModelBatch7E *batches;
    /* 0x28 */ u8 pad28[0x38 - 0x28];
    /* 0x38 */ ModelSphereDef *spheres;
    /* 0x3C */ u8 pad3C[0x4E - 0x3C];
    /* 0x4E */ s8 animated; // JFG: sphere positions come from the animated vertex list
    /* 0x4F */ u8 pad4F[0x60 - 0x4F];
    /* 0x60 */ ModelEdges7E *edges;
} ModelHeader7E;

/* JFG: a vertex (0xA bytes) */
typedef struct Vertex7E {
    /* 0x00 */ s16 x, y, z;
    /* 0x06 */ u8 pad6[4];
} Vertex7E;

typedef struct Obj7EModel {
    /* 0x00 */ ModelHeader7E *header;
    /* 0x04 */ Vertex7E *vertices;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ ModelEdges7E *edges; // JFG: this instance's own edge info (else the header's)
    /* 0x10 */ u8 pad10[0x70 - 0x10];
    /* 0x70 */ f32 *animVerts; // JFG: x, y, z per sphere
    /* 0x74 */ u8 pad74[0x8C - 0x74];
    /* 0x8C */ s16 bboxMin[3];
    /* 0x92 */ s16 bboxMax[3];
} Obj7EModel;

typedef struct Obj7EFlags {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags;
} Obj7EFlags;

typedef struct Obj7EState {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 pad2[0x1C - 0x2];
    /* 0x1C */ s32 unk1C;
} Obj7EState;

extern s32 D_801047E0;
extern Obj7E **D_801047E4;
extern void *D_801047E8;
extern void *D_801047EC;
extern s32 *D_801047F4; // JFG: per entry, an object address (> 0) or 0 for a triangle
extern s16 *D_801047F8; // JFG: per entry, the batch of the triangle
extern s16 *D_801047FC; // JFG: per entry, the triangle
extern struct ObjEdge *D_80104800; // JFG: the object edge list
void *mempool_alloc_safe(s32 size, u32 colourTag);
void mempool_free(void *data);

void collision_buffers_alloc(void) {
    D_801047E4 = mempool_alloc_safe(0x140, 0xFFFF);
    D_801047E8 = mempool_alloc_safe(0x400, 0xFFFF);
    D_801047EC = mempool_alloc_safe(0x400, 0xFFFF);
}

/* DKR include/level.h LevelHeader, reduced (JFG offsets) */
typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0xEE];
    /* 0xEE */ s16 unkEE;
    /* 0xF0 */ s16 unkF0;
} LevelHeader;
LevelHeader *level_header(void);

void collision_level_buffers_alloc(void) {
    LevelHeader *header;

    D_801047E0 = 0;
    header = level_header();
    D_80104800 = mempool_alloc_safe(header->unkF0 * 0x30, 0xFFFF);
    D_801047F4 = mempool_alloc_safe(header->unkEE * 4, 0xFFFF);
    D_801047F8 = mempool_alloc_safe(header->unkEE * 2, 0xFFFF);
    D_801047FC = mempool_alloc_safe(header->unkEE * 2, 0xFFFF);
}

void collision_level_buffers_free(void) {
    mempool_free(D_80104800);
    mempool_free(D_801047F4);
    mempool_free(D_801047F8);
    mempool_free(D_801047FC);
}

/* Size: 0x34 bytes */
typedef struct CollisionSphere {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags;
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ Vec3f prevPos;
    /* 0x24 */ f32 radius;
    /* 0x28 */ f32 radius2;
    /* 0x2C */ u8 pad2C[0x30 - 0x2C];
    /* 0x30 */ u32 reset : 1;
    /* 0x30 */ u32 pad30 : 31;
} CollisionSphere;

typedef struct SphereSet {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 active : 1;
    /* 0x08 */ u8 unk8_6 : 1;
    /* 0x08 */ u8 unk8_rest : 6;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u16 unkA;
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ Vec3f prevPos;
    /* 0x24 */ f32 radius;
    /* 0x28 */ f32 radius2;
    /* 0x2C */ s16 unk2C;
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 unk2F;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ s16 count;
    /* 0x34 */ f32 unk34;
    /* 0x38 */ f32 unk38;
    /* 0x3C */ u8 pad3C[0x40 - 0x3C];
    /* 0x40 */ CollisionSphere *spheres;
} SphereSet;

void func_800492DC(Obj7E *obj, Vec3f *v);

/* JFG: set up an object's hit set (at obj+0x4C): its centre offset by the rotated (rx, ry, rz) times the
   scale, the scaled radii and the other values, then place its collision spheres on the model as
   obj_update_collision_spheres does, with fresh radii. */
void obj_collision_spheres_init(Obj7E *obj, s32 arg1, s32 arg2, s32 arg3, s16 rx, s16 ry, s16 rz, f32 radius, f32 radius2,
                   f32 arg9, f32 arg10, u16 argB) {
    f32 pos[3];
    SphereSet *set;
    s32 i;
    CollisionSphere *sphere;
    Obj7EModel *model;
    ModelHeader7E *header;
    f32 *anim;
    ModelSphereDef *def;
    Vertex7E *vert;

    set = (SphereSet *) obj->flags;
    if (set == NULL) {
        return;
    }
    set->unk6 = arg1;
    set->unk30 = arg2;
    set->unk31 = 0;
    set->unk9 = arg3;
    set->rotation[0] = rx;
    set->rotation[1] = ry;
    set->rotation[2] = rz;
    set->prevPos.x = obj->x;
    set->prevPos.y = obj->y;
    set->prevPos.z = obj->z;
    if (rx | ry | rz) {
        pos[0] = rx;
        pos[1] = ry;
        pos[2] = rz;
        func_800492DC(obj, (Vec3f *) pos);
        set->prevPos.x += pos[0] * obj->scale;
        set->prevPos.y += pos[1] * obj->scale;
        set->prevPos.z += pos[2] * obj->scale;
    }
    set->pos.x = set->prevPos.x;
    set->pos.y = set->prevPos.y;
    set->pos.z = set->prevPos.z;
    set->radius = obj->scale * radius;
    set->radius2 = obj->scale * radius2;
    set->unk34 = arg9;
    set->unk38 = arg10;
    set->unk2E = 100;
    set->unk2F = 100;
    set->active = TRUE;
    set->unk2C = 0;
    set->unk8_6 = FALSE;
    set->unkA = argB;
    if (set->count == 0) {
        return;
    }
    if (obj->unk4A == 0xE6) {
        return;
    }
    i = set->count;
    sphere = set->spheres;
    model = obj->modelInstances[0];
    header = model->header;
    anim = model->animVerts;
    def = header->spheres;
    do {
        if (header->animated) {
            sphere->prevPos.x = *anim++;
            sphere->prevPos.y = *anim++;
            sphere->prevPos.z = *anim++;
        } else {
            vert = &model->vertices[def->vertex];
            pos[0] = vert->x;
            pos[1] = vert->y;
            pos[2] = vert->z;
            func_800492DC(obj, (Vec3f *) pos);
            sphere->prevPos.x = pos[0] + obj->x;
            sphere->prevPos.y = pos[1] + obj->y;
            sphere->prevPos.z = pos[2] + obj->z;
        }
        i--;
        sphere->radius2 = sphere->radius = def->radius * obj->scale;
        sphere++;
        def++;
    } while (i > 0);
}

f32 sqrtf(f32 x);

/* JFG: fit the hit set around its collision spheres (at the model's animated positions): the centre of their
   bounding box, a radius reaching its corner (in 3D when unk30 is 1, else on XZ with the half height in
   radius2), and the centre's offset from the object. */
void obj_hit_set_fit(Obj7E *obj, SphereSet *set) {
    s32 i;
    f32 minX;
    f32 minY;
    f32 minZ;
    f32 maxX;
    f32 maxY;
    f32 maxZ;
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 cx;
    f32 cy;
    f32 cz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 *anim;
    CollisionSphere *sphere;

    anim = obj->modelInstances[obj->modelIndex]->animVerts;
    sphere = set->spheres;
    minX = anim[0] - sphere->radius;
    minY = anim[1] - sphere->radius2;
    minZ = anim[2] - sphere->radius;
    maxX = anim[0] + sphere->radius;
    maxY = anim[1] + sphere->radius2;
    maxZ = anim[2] + sphere->radius;
    anim += 3;
    sphere++;
    for (i = 1; i < set->count; i++) {
        x0 = anim[0] - sphere->radius;
        y0 = anim[1] - sphere->radius2;
        z0 = anim[2] - sphere->radius;
        x1 = anim[0] + sphere->radius;
        y1 = anim[1] + sphere->radius2;
        z1 = anim[2] + sphere->radius;
        if (x0 < minX) {
            minX = x0;
        }
        if (y0 < minY) {
            minY = y0;
        }
        if (z0 < minZ) {
            minZ = z0;
        }
        if (maxX < x1) {
            maxX = x1;
        }
        if (maxY < y1) {
            maxY = y1;
        }
        if (maxZ < z1) {
            maxZ = z1;
        }
        anim += 3;
        sphere++;
    }
    cx = (minX + maxX) * 0.5f;
    cz = (minZ + maxZ) * 0.5f;
    dx = maxX - cx;
    dz = maxZ - cz;
    d = dx * dx + dz * dz;
    if (set->unk30 == 1) {
        cy = (minY + maxY) * 0.5f;
        dy = maxY - cy;
        set->radius = set->radius2 = sqrtf(dy * dy + d);
    } else {
        set->radius = sqrtf(d);
        cy = (minY + maxY) * 0.5f;
        set->radius2 = maxY - cy;
    }
    set->pos.x = cx;
    set->pos.y = cy;
    set->pos.z = cz;
    set->rotation[0] = cx - obj->x;
    set->rotation[1] = cy - obj->y;
    set->rotation[2] = cz - obj->z;
}

void obj_hit_set_fit(Obj7E *obj, SphereSet *set);

/* JFG: move an object's collision spheres to its current model vertices (keeping the previous positions). */
void obj_update_collision_spheres(Obj7E *obj, SphereSet *set) {
    s32 i;
    CollisionSphere *sphere;
    f32 pos[3];
    Obj7EModel *model;
    ModelHeader7E *header;
    f32 *anim;
    ModelSphereDef *def;
    Vertex7E *vert;

    if (set->count == 0) {
        return;
    }
    if (obj->unk4A == 0xE6) {
        return;
    }
    i = set->count;
    sphere = set->spheres;
    model = obj->modelInstances[0];
    header = model->header;
    anim = model->animVerts;
    def = header->spheres;
    do {
        sphere->flags &= ~0x60;
        sphere->prevPos.x = sphere->pos.x;
        sphere->prevPos.y = sphere->pos.y;
        sphere->prevPos.z = sphere->pos.z;
        if (header->animated) {
            sphere->pos.x = *anim++;
            sphere->pos.y = *anim++;
            sphere->pos.z = *anim++;
        } else {
            vert = &model->vertices[def->vertex];
            pos[0] = vert->x;
            pos[1] = vert->y;
            pos[2] = vert->z;
            func_800492DC(obj, (Vec3f *) pos);
            sphere->pos.x = pos[0] + obj->x;
            sphere->pos.y = pos[1] + obj->y;
            sphere->pos.z = pos[2] + obj->z;
        }
        i--;
        if (sphere->reset) {
            sphere->radius2 = sphere->radius = def->radius * obj->scale;
            sphere->reset = FALSE;
            sphere->prevPos.x = sphere->pos.x;
            sphere->prevPos.y = sphere->pos.y;
            sphere->prevPos.z = sphere->pos.z;
        }
        sphere++;
        def++;
    } while (i > 0);
    if (obj->type == 1) {
        obj_hit_set_fit(obj, set);
    }
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A534);

typedef struct CollObjHeader {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ u16 flags; // JFG: 0x40 = solid for line tests
} CollObjHeader;

typedef struct CollInteract {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ u8 kind;
} CollInteract;

typedef struct CollObj {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ Vec3f pos;
    /* 0x1C */ u8 pad1C[0x40 - 0x1C];
    /* 0x40 */ CollObjHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 type;
    /* 0x4A */ u8 pad4A[0x4C - 0x4A];
    /* 0x4C */ CollInteract *interactObj;
} CollObj;

/* JFG: a line test start point (with two scales) */
typedef struct LinePoint {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 pad14[0x30 - 0x14];
} LinePoint;

/* JFG: what the level line test reports (0x28 bytes) */
typedef struct LineHit {
    /* 0x00 */ u8 pad0[0x28];
} LineHit;

s32 func_80018808(s32 arg0, LinePoint *from, Vec3f *to, LineHit *hit, s32 flags, s32 arg5);
CollObj **objGetObjList(s32 *first, s32 *last);
/* JFG: object line/sphere query; start/end are filled in by the caller (0x54 bytes) */
typedef struct CollQuery {
    /* 0x00 */ Vec3f unk0;
    /* 0x0C */ Vec3f end;
    /* 0x18 */ Vec3f start;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x54 - 0x2C];
} CollQuery;

s32 func_8007C2E8(s32 arg0, void *query, s32 count, CollObj **objs, CollObj **hitObj, CollObj *self);

/* JFG: whether the line from (x1, y1, z1) to (x2, y2, z2) hits the level (1) or how many solid objects other
 * than self and other it hits. */
s32 collision_line_test_objects(CollObj *self, CollObj *other, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
    UNUSED s32 pad;
    LineHit hit;
    s32 first;
    s32 last;
    LinePoint from;
    Vec3f to;
    Vec3f result;
    CollObj *index;
    CollObj **objs;
    CollObj *obj;
    CollObj **list;
    s32 count = 0;

    from.unkC = 1.0f;
    from.unk10 = 1.0f;
    from.pos.x = x1;
    from.pos.y = y1;
    from.pos.z = z1;
    to.x = x2;
    to.y = y2;
    to.z = z2;
    if (func_80018808(0, &from, &to, &hit, 0x2000, 0)) {
        return 1;
    }
    objs = objGetObjList(&first, &last);
    list = D_801047EC;
    while (first < last) {
        obj = objs[first++];
        if (obj != self && obj != other && obj->interactObj != NULL && (obj->header->flags & 0x40)) {
            list[count++] = obj;
        }
    }
    if (count > 0) {
        count = func_8007C2E8(0, &result, count, list, &index, self);
    }
    return count;
}

/* JFG: how many solid objects (interaction kind 1 or 2, type not skipType1/2) the line from start to end hits;
 * *dist gets the distance from start to the hit object. */
s32 line_count_solid_hits(CollObj *self, Vec3f *start, Vec3f *end, f32 *dist, s32 skipType1, s32 skipType2) {
    s32 first;
    s32 last;
    CollQuery query;
    CollObj **objs;
    CollObj *hitObj;
    CollObj *obj;
    CollObj **list;
    s32 count = 0;

    query.start.x = start->x;
    query.start.y = start->y;
    query.start.z = start->z;
    query.end.x = end->x;
    query.end.y = end->y;
    query.end.z = end->z;
    query.unk24 = 1.0f;
    query.unk28 = 1.0f;
    objs = objGetObjList(&first, &last);
    list = D_801047EC;
    while (first < last) {
        obj = objs[first++];
        if (obj->interactObj != NULL && (obj->header->flags & 0x40) &&
            (obj->interactObj->kind == 1 || obj->interactObj->kind == 2) && skipType1 != obj->type &&
            skipType2 != obj->type) {
            list[count++] = obj;
        }
    }
    if (count > 0) {
        count = func_8007C2E8(0, &query, count, list, &hitObj, self);
        if (count > 0 && dist != NULL) {
            *dist = sqrtf((query.start.x - hitObj->pos.x) * (query.start.x - hitObj->pos.x) +
                          (query.start.y - hitObj->pos.y) * (query.start.y - hitObj->pos.y) +
                          (query.start.z - hitObj->pos.z) * (query.start.z - hitObj->pos.z));
        }
    }
    return count;
}

/* JFG: an object hit by the player's aim line, as func_8007BCD0 reads it */
typedef struct AimObj {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ CollObjHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 type;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ struct AimInteract *interact;
    /* 0x50 */ u8 pad50[0x93 - 0x50];
    /* 0x93 */ u8 unk93[4]; // JFG: per player
    /* 0x97 */ u8 unk97[4]; // JFG: per player: the part index, or 0xFF
    /* 0x9B */ u8 unk9B[4]; // JFG: per player
} AimObj;

typedef struct AimInteract {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 unk6;
} AimInteract;

/* JFG: one part of a hit object (0x34 bytes) */
typedef struct AimPart {
    /* 0x00 */ u8 pad0[0x2F];
    /* 0x2F */ u8 enabled;
    /* 0x30 */ u32 unk30_31 : 1;
    /* 0x30 */ u32 unk30_30 : 1;
    /* 0x30 */ u32 unk30_29 : 1;
    /* 0x30 */ u32 unk30_rest : 29;
} AimPart;

typedef struct AimTarget {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ u32 unk8_31 : 1;
    /* 0x08 */ u32 unk8_30 : 1;
    /* 0x08 */ u32 unk8_rest : 30;
    /* 0x0C */ u8 padC[0x2F - 0xC];
    /* 0x2F */ u8 enabled;
    /* 0x30 */ u8 pad30[0x40 - 0x30];
    /* 0x40 */ AimPart *parts;
} AimTarget;

/* JFG: what func_8007C2E8 reports for the object hit */
typedef struct AimHit {
    /* 0x00 */ AimObj *obj;
    /* 0x04 */ AimTarget *target;
    /* 0x08 */ AimPart *part;
    /* 0x0C */ u8 padC[4];
    /* 0x10 */ Vec3f pos;
} AimHit;

/* JFG: the line query as func_8007BCD0 lays it out on its stack */
typedef struct AimQuery {
    /* 0x00 */ Vec3f unk0;
    /* 0x0C */ Vec3f end;
    /* 0x18 */ Vec3f start;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x44 - 0x2C];
} AimQuery;

typedef struct AimRacer {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0xC4 - 0x2];
    /* 0x0C4 */ s32 soundHandle;
    /* 0x0C8 */ u8 padC8[0x1A8 - 0xC8];
    /* 0x1A8 */ AimObj *targets[6];
    /* 0x1C0 */ u8 targetParts[6];
    /* 0x1C6 */ s16 unk1C6; // JFG: the aim angles
    /* 0x1C8 */ s16 unk1C8;
} AimRacer;

typedef struct AimChar {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ u8 unk70;
} AimChar;

typedef struct AimModelHeader {
    /* 0x00 */ u8 pad0[0x2F];
    /* 0x2F */ u8 unk2F;
} AimModelHeader;

typedef struct AimModel {
    /* 0x00 */ AimModelHeader *header;
} AimModel;

typedef struct AimSelf {
    /* 0x00 */ u8 pad0[0x3A];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B[0x68 - 0x3B];
    /* 0x68 */ AimRacer *racer;
    /* 0x6C */ AimModel **modelInstances;
} AimSelf;

AimChar *char_record_lookup(s32 player, s32 character);
void player_aim_origin_get(AimSelf *obj, AimRacer *racer, f32 *x, f32 *y, f32 *z);
void func_8004944C(s16 *rotation, f32 *vec); // DKR: vec3f_rotate_py
u16 input_held(s32 player);
void sound_play(u16 soundID, s32 *handlePtr);

/**
 * JFG: the line from the player's aim origin, range units along its aim angles, against the solid objects;
 * *dist (if given) gets the distance to the object hit. Returns whether it hits one (0 if that object or part is
 * one the aim ignores), and records the object (and part) in the racer's target slots.
 * As in the ROM, only vec[2] is set before func_8004944C reads vec, and the count is cleared (to return 0) where
 * the part is to be ignored.
 */
s32 func_8007BCD0(AimSelf *self, f32 range, f32 *dist) {
    s32 first;
    s32 last;
    AimRacer *racer;
    s32 count;
    s32 i;
    s16 rot[3];
    f32 vec[3];
    AimQuery query;
    AimHit *hit;
    AimObj *obj;
    AimModel *model;
    AimObj **list;
    u8 part;
    u8 bit30;
    u8 enabled;
    u8 bit29;
    UNUSED s32 pad;
    AimChar *ch;
    AimObj **objs;

    racer = self->racer;
    ch = char_record_lookup(racer->player, racer->character);
    count = 0;
    player_aim_origin_get(self, racer, &query.start.x, &query.start.y, &query.start.z);
    rot[0] = racer->unk1C6;
    rot[1] = racer->unk1C8;
    rot[2] = 0;
    vec[2] = -range;
    func_8004944C(rot, vec);
    query.end.x = vec[0] + query.start.x;
    query.end.y = vec[1] + query.start.y;
    query.end.z = vec[2] + query.start.z;
    query.unk24 = 1.0f;
    query.unk28 = 1.0f;
    objs = (AimObj **) objGetObjList(&first, &last);
    list = D_801047EC;
    while (first < last) {
        obj = objs[first++];
        if (obj->interact != NULL && (obj->header->flags & 0x40)) {
            list[count++] = obj;
        }
    }
    if (count > 0) {
        count = func_8007C2E8(0, &query, count, (CollObj **) list, (CollObj **) &hit, (CollObj *) self);
        if (count > 0 && dist != NULL) {
            vec[0] = query.start.x - hit->pos.x;
            vec[1] = query.start.y - hit->pos.y;
            vec[2] = query.start.z - hit->pos.z;
            *dist = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        }
    }
    if (count != 0) {
        obj = hit->obj;
        if (ch->unk70 == 9) {
            if (obj->type != 1 && obj->type != 0x5A && obj->type != 0x18) {
                return FALSE;
            }
        }
        if (obj->type == 0x18 && obj->unk4A == 0xE6) {
            return FALSE;
        }
        if (obj->type == 1 && obj->interact != NULL && obj->interact->unk6 < 0x100) {
            return FALSE;
        }
        model = self->modelInstances[self->modelIndex];
        part = 0xFF;
        if ((AimPart *) hit->target == hit->part || model->header == NULL || !model->header->unk2F) {
            bit30 = hit->target->unk8_30;
            enabled = hit->target->enabled;
            bit29 = FALSE;
        } else {
            part = hit->part - hit->target->parts;
            enabled = hit->part->enabled;
            bit30 = hit->part->unk30_30;
            bit29 = hit->part->unk30_29;
        }
        if (bit29) {
            count = 0;
        }
        if (!bit29 && enabled) {
            if (!bit30 && (ch->unk70 == 1 || ch->unk70 == 9) && (input_held(0) & 0x10)) {
                i = 5;
                do {
                    if (racer->targets[i] == NULL ||
                        ((obj->header->flags & 0x4000) && obj == racer->targets[i] && part != racer->targetParts[i])) {
                        if (racer->soundHandle == 0) {
                            sound_play(0x1D, &racer->soundHandle);
                        }
                        racer->targets[i] = obj;
                        racer->targetParts[i] = part;
                        if (ch->unk70 == 9) {
                            obj->unk93[racer->player] = 3;
                            obj->unk9B[racer->player] = 1;
                            obj->unk97[racer->player] = 0xFF;
                        } else {
                            obj->unk93[racer->player] = 4;
                            obj->unk9B[racer->player] = 1;
                            obj->unk97[racer->player] = part;
                        }
                        break;
                    }
                    if (obj == racer->targets[i]) {
                        break;
                    }
                } while (i--);
            } else if (obj != racer->targets[5]) {
                if (racer->targets[5] != NULL) {
                    racer->targets[5]->unk93[racer->player] = 0;
                }
                if (racer->soundHandle == 0) {
                    sound_play(0x1D, &racer->soundHandle);
                }
                racer->targets[5] = obj;
                racer->targetParts[5] = part;
                switch (ch->unk70) {
                    case 0:
                    case 2:
                    case 3:
                    case 5:
                    case 7:
                    case 8:
                        obj->unk93[racer->player] = 2;
                        break;
                    case 4:
                    case 6:
                    case 11:
                    case 12:
                    case 14:
                        obj->unk93[racer->player] = 1;
                        break;
                    case 1:
                        obj->unk93[racer->player] = 4;
                        break;
                    case 9:
                        obj->unk93[racer->player] = 3;
                        break;
                    case 10:
                    case 13:
                        obj->unk93[racer->player] = 5;
                        break;
                }
                obj->unk97[racer->player] = 0xFF;
                obj->unk9B[racer->player] = 1;
                if (obj->type == 0x6C || obj->type == 0x82) {
                    obj->unk97[racer->player] = part;
                    racer->targetParts[5] = part;
                }
            }
        }
    }
    return count != 0;
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007C2E8);

f32 sqrtf(f32 x);

typedef struct PushPlayer {
    /* 0x000 */ u8 pad0[0x190];
    /* 0x190 */ u8 flags190;
    /* 0x191 */ u8 pad191[0x1F9 - 0x191];
    /* 0x1F9 */ u8 unk1F9;
    /* 0x1FA */ u8 pad1FA[0x354 - 0x1FA];
    /* 0x354 */ f32 pushX;
    /* 0x358 */ f32 pushY;
    /* 0x35C */ f32 pushZ;
} PushPlayer;

typedef struct PushInfo {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 flags;
} PushInfo;

typedef struct PushObj {
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ s16 type;
    /* 0x4A */ u8 pad4A[0x68 - 0x4A];
    /* 0x68 */ void *data;
} PushObj;

/* JFG: push the player (horizontally) out of every sphere of another object's set its spheres overlap (0x8007CCF0). */
void player_spheres_push_out(PushObj *obj, SphereSet *set, PushObj *other, SphereSet *otherSet) {
    PushPlayer *player = obj->data;
    s32 flag = 0;
    s32 i;
    s32 j;
    CollisionSphere *a;
    CollisionSphere *b;
    PushInfo *info;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 r;
    f32 d;
    f32 f;

    if (other->type == 0x18) {
        info = other->data;
        if (!(info->flags & 1)) {
            flag = 0x80;
        }
    } else if (other->type == 0x5A && player->unk1F9) {
        return;
    }
    a = set->spheres;
    for (i = 0; i < set->count; i++, a++) {
        b = otherSet->spheres;
        for (j = 0; j < otherSet->count; j++, b++) {
            dx = (a->pos.x + player->pushX) - b->pos.x;
            dy = a->pos.y - b->pos.y;
            dz = (a->pos.z + player->pushZ) - b->pos.z;
            r = a->radius + b->radius;
            d = sqrtf(dx * dx + dz * dz);
            if (-r < dy && dy < r && d < r) {
                if (d != 0.0f) {
                    f = (r - d) / d;
                    player->pushX += dx * f;
                    player->pushZ += dz * f;
                } else {
                    player->pushZ -= d;
                }
                player->flags190 |= flag;
            }
        }
    }
}

/* JFG: push a sphere (position and radius) out of every sphere of the set it overlaps. */
void spheres_push_out(f32 *x, f32 *y, f32 *z, f32 radius, SphereSet *set) {
    s32 i = set->count;
    f32 px = *x;
    f32 py = *y;
    f32 pz = *z;
    CollisionSphere *sphere = set->spheres;
    f32 dx, dy, dz, r, d2, d, f;

    while (i--) {
        dx = px - sphere->pos.x;
        dy = py - sphere->pos.y;
        dz = pz - sphere->pos.z;
        r = sphere->radius + radius;
        d2 = dx * dx + dy * dy + dz * dz;
        if (d2 < r * r && d2 != 0.0f) {
            d = sqrtf(d2);
            f = (r - d) / d;
            px += dx * f;
            py += dy * f;
            pz += dz * f;
        }
        sphere++;
    }
    *x = px;
    *y = py;
    *z = pz;
}

/* JFG: TRUE if the sphere overlaps any sphere of the set (0x8007D0B4). */
s32 sphere_overlaps_set(CollisionSphere *sphere, SphereSet *set) {
    s32 i = set->count;
    CollisionSphere *other = set->spheres;
    f32 x = sphere->pos.x;
    f32 y = sphere->pos.y;
    f32 z = sphere->pos.z;
    f32 radius = sphere->radius;
    f32 dx, dy, dz, r;

    while (i--) {
        dx = other->pos.x - x;
        dy = other->pos.y - y;
        dz = other->pos.z - z;
        r = other->radius + radius;
        r *= r;
        other++;
        if (dx * dx + dy * dy + dz * dz < r) {
            return TRUE;
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007D168);

/* JFG: a pair of points moved together with an object (only the fields used here) */
typedef struct PointPair {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 x2;
    /* 0x1C */ f32 y2;
    /* 0x20 */ f32 z2;
} PointPair;

typedef struct SegmentBox {
    /* 0x00 */ s16 x1, y1, z1;
    /* 0x06 */ s16 x2, y2, z2;
} SegmentBox;

SegmentBox *block_boundbox(s32 segmentID);
s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos);

/* JFG: move the pair's points to (x2, y2, z2) and (x, y, z), the object's state points with them (type 2) and
   the object by the first point's x offset on all three axes, then update its level segment. */
void obj_move_with_points(Obj7E *obj, PointPair *pair, f32 x2, f32 y2, f32 z2, f32 x, f32 y, f32 z) {
    s32 ix;
    s32 iy;
    s32 iz;
    f32 dx2;
    f32 dy2;
    f32 dz2;
    f32 dx;
    f32 dy;
    f32 dz;
    SegmentBox *box;
    f32 *pts;

    dx2 = x2 - pair->x2;
    dy2 = y2 - pair->y2;
    dz2 = z2 - pair->z2;
    dx = x - pair->x;
    dy = y - pair->y;
    dz = z - pair->z;
    pair->x2 += dx2;
    pair->y2 += dy2;
    pair->z2 += dz2;
    pair->x += dx;
    pair->y += dy;
    pair->z += dz;
    if (obj->type == 2) {
        pts = (f32 *) obj->state;
        pts[0] += dx;
        pts[1] += dy;
        pts[2] += dz;
        pts[3] += dx2;
        pts[4] += dy2;
        pts[5] += dz2;
    }
    /* (the game's own quirk: dx on all three axes) */
    obj->x += dx;
    obj->y += dx;
    obj->z += dx;
    if (obj->segmentID == -1) {
        obj->segmentID = get_level_segment_index_from_position(obj->x, obj->y, obj->z);
    } else {
        ix = obj->x;
        iy = obj->y;
        iz = obj->z;
        box = block_boundbox(obj->segmentID);
        if (box->x2 < ix || ix < box->x1 || box->y2 < iy || iy < box->y1 || box->z2 < iz || iz < box->z1) {
            obj->segmentID = get_level_segment_index_from_position(obj->x, obj->y, obj->z);
        }
    }
}

typedef struct RacerInfo {
    /* 0x00 */ s8 playerIndex;
} RacerInfo;

typedef struct RacerObj {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x68 - 0x18];
    /* 0x68 */ RacerInfo *racer;
} RacerObj;

RacerObj **get_racer_objects(s32 *numRacers);

/* JFG: DKR's obj_dist_racer: the racers within radius of (x, y, z) (2D when is2dCheck), sorted nearest first. */
s32 obj_dist_racer(f32 x, f32 y, f32 z, f32 radius, s32 is2dCheck, RacerObj **sortObj) {
    f32 distances[8];
    s32 i;
    s32 j;
    RacerObj **racers;
    RacerObj *racerObj;
    RacerInfo *racer;
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    s32 numRacers;
    f32 dist;
    s32 result;
    f32 swapf;

    result = 0;
    radius *= radius;
    racers = get_racer_objects(&numRacers);
    if (numRacers > 0) {
        do {
            racerObj = *racers++;
            racer = racerObj->racer;
            if (racer->playerIndex >= 0 && racer->playerIndex < 4) {
                xDiff = racerObj->x - x;
                zDiff = racerObj->z - z;
                dist = xDiff * xDiff + zDiff * zDiff;
                if (!is2dCheck) {
                    yDiff = racerObj->y - y;
                    dist += yDiff * yDiff;
                }
                if (dist < radius) {
                    distances[result] = sqrtf(dist);
                    sortObj[result] = racerObj;
                    result++;
                }
            }
        } while (--numRacers > 0);
        i = result - 1;
        while (i > 0) {
            j = 0;
            do {
                if (distances[j + 1] < distances[j]) {
                    swapf = distances[j];
                    racerObj = sortObj[j];
                    distances[j] = distances[j + 1];
                    sortObj[j] = sortObj[j + 1];
                    distances[j + 1] = swapf;
                    sortObj[j + 1] = racerObj;
                }
                j++;
            } while (j < i);
            i--;
        }
    }
    return result;
}

/* JFG: one player's multiplayer scores (0xA0 bytes; 45750.c has the whole record) */
typedef struct MultiPlayerScore {
    /* 0x00 */ u16 kills[4];  // JFG: by victim
    /* 0x08 */ u16 deaths[4]; // JFG: by killer
    /* 0x10 */ s16 score;
    /* 0x12 */ u8 pad12;
    /* 0x13 */ s8 lives;
    /* 0x14 */ u8 pad14[0xA0 - 0x14];
} MultiPlayerScore;
extern MultiPlayerScore D_800FED50[4];
extern u8 D_800A4FC4; // JFG: a multiplayer game
extern u8 D_800A4FD8; // JFG: the multiplayer game type

typedef struct PlayerState {
    /* 0x00 */ s8 player;
} PlayerState;

typedef struct PlayerObject {
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ u8 pad4A[0x68 - 0x4A];
    /* 0x68 */ PlayerState *state;
} PlayerObject;

void func_80054530(); // a module function (patched in by the runtime linker)

/* JFG: in a multiplayer game, score killer's kill of victim (both players); TRUE if it was scored. */
s32 multiplayer_score_kill(PlayerObject *killer, PlayerObject *victim) {
    PlayerState *k;
    PlayerState *v;

    if (D_800A4FC4 && killer->behaviorId == 1 && victim->behaviorId == 1) {
        k = killer->state;
        v = victim->state;
        D_800FED50[k->player].kills[v->player]++;
        D_800FED50[v->player].deaths[k->player]++;
        if ((D_800A4FD8 & 0x30) == 0x30) {
            D_800FED50[v->player].lives--;
        }
        if (killer != victim) {
            D_800FED50[k->player].score++;
        } else {
            D_800FED50[k->player].score--;
        }
        func_80054530(k, 0x9F);
        return TRUE;
    }
    return FALSE;
}

Obj7E **collision_objects_get(s32 *count) {
    *count = D_801047E0;
    return D_801047E4;
}

void obj_update_matrices(Obj7E *obj, s32 arg1);
void func_8007E794(Obj7E *obj);

void func_8007E4A0(Obj7E *obj) {
    Obj7EData *data = obj->data;
    s8 value = obj->header->unk71;

    if (value >= 0) {
        data->modelIndex = value;
    } else {
        data->modelIndex = obj->modelIndex;
    }
    obj_update_matrices(obj, 1);
    obj_update_matrices(obj, 1);
    func_8007E794(obj);
}

void func_8007E504(void) {
    s32 i;
    Obj7E *obj;
    Obj7EData *data;
    s8 value;

    for (i = 0; i < D_801047E0; i++) {
        obj = D_801047E4[i];
        data = obj->data;
        value = obj->header->unk71;
        if (value >= 0) {
            data->modelIndex = value;
        } else {
            data->modelIndex = obj->modelIndex;
        }
        obj_update_matrices(obj, 0);
        if (data->unk12C != 0) {
            func_8007E794(obj);
        }
        data->unk12E = 0;
        data->unk132 = 0;
        data->unk130 = 0;
        data->unk12A = 0;
    }
}

/* DKR's ObjectTransform layout */
typedef struct Obj7ETransform {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ s16 pitch;
    /* 0x04 */ s16 roll;
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} Obj7ETransform;

void func_8004A014(Obj7ETransform *trans, f32 mtx[4][4]); // JFG: matrix from an (inverse-order) transform
void func_80049DC0(Obj7E *obj, f32 mtx[4][4]);            // JFG: matrix from an object's transform

/* JFG: if the object moved or turned (or force is set), rebuild its transform and inverse matrices (double-buffered). */
void obj_update_matrices(Obj7E *obj, s32 force) {
    Obj7EData *data = obj->data;
    Obj7ETransform trans;

    if (obj->x != data->prevX || obj->y != data->prevY || obj->z != data->prevZ || obj->yaw != data->prevYaw ||
        obj->pitch != data->prevPitch || obj->roll != data->prevRoll) {
        data->unk12C = 1;
    } else {
        data->unk12C = 0;
    }
    if (data->unk12C || force) {
        data->mtxIndex ^= 1;
        obj->yaw &= 0xFFF0;
        obj->pitch &= 0xFFF0;
        obj->roll &= 0xFFF0;
        trans.x = -obj->x;
        trans.y = -obj->y;
        trans.z = -obj->z;
        trans.scale = 1.0f / obj->scale;
        trans.roll = -obj->roll;
        trans.pitch = -obj->pitch;
        trans.yaw = -obj->yaw;
        func_8004A014(&trans, data->mtx[data->mtxIndex]);
        func_80049DC0(obj, data->mtx2[data->mtxIndex]);
    }
}

#ifdef NON_MATCHING
/* JFG: the object's world bounding box: its model's box transformed by its current matrix
 * (same pattern as func_8007E9E0).
 * NON_MATCHING: ROM length and frame, 10 diffs: i, the column pointer and the min pointer are in a0/v0/a2 (the ROM
 * a2/a0/v1). The column walked by its own pointer (p, from xform + i) gives the ROM's i counter and loop shape; the
 * frame needs one more word above min/max (pad). */
void func_8007E794(Obj7E *obj) {
    Obj7EData *data;
    f32 (*xform)[4];
    Obj7EModel *model;
    s32 i;
    s32 j;
    f32 *lo;
    f32 *hi;
    f32 a;
    f32 b;
    UNUSED s32 pad;
    s16 min[3];
    s16 max[3];
    f32 *p;

    data = obj->data;
    model = obj->modelInstances[data->modelIndex];
    xform = data->mtx2[data->mtxIndex];
    data->bboxMin[0] = xform[3][0];
    data->bboxMin[1] = xform[3][1];
    data->bboxMin[2] = xform[3][2];
    data->bboxMax[0] = data->bboxMin[0];
    data->bboxMax[1] = data->bboxMin[1];
    data->bboxMax[2] = data->bboxMin[2];
    min[0] = (f32) model->bboxMin[0];
    min[1] = (f32) model->bboxMin[1];
    min[2] = (f32) model->bboxMin[2];
    max[0] = (f32) model->bboxMax[0];
    max[1] = (f32) model->bboxMax[1];
    max[2] = (f32) model->bboxMax[2];
    lo = data->bboxMin;
    hi = data->bboxMax;
    for (i = 0; i < 3; i++) {
        p = (f32 *) xform + i;
        for (j = 0; j < 3; j++, p += 4) {
            a = *p * min[j];
            b = *p * max[j];
            if (a < b) {
                lo[i] += a;
                hi[i] += b;
            } else {
                lo[i] += b;
                hi[i] += a;
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E794);
#endif

/* JFG: transform a bounding box (min and max corners) by a matrix, giving the box around the result. The element
 * mtx[i][j] is read as a byte offset, (i * 4 + j) * 4 from mtx: that keeps j as the outer counter (mtx + j * 4
 * recomputed each pass) and gives the inner loop its i * 4 counter and walking column pointer. */
void func_8007E9E0(f32 mtx[4][4], f32 *min, f32 *max) {
    f32 newMin[4];
    s32 i;
    f32 newMax[4];
    s32 j;
    f32 a;
    f32 b;
    UNUSED s32 pad;

    newMin[0] = 0.0f;
    newMin[1] = 0.0f;
    newMin[2] = 0.0f;
    newMax[0] = 0.0f;
    newMax[1] = 0.0f;
    newMax[2] = 0.0f;
    newMin[3] = 1.0f;
    newMax[3] = 1.0f;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            a = *(f32 *) ((u8 *) mtx + (i * 4 + j) * 4) * min[i];
            b = *(f32 *) ((u8 *) mtx + (i * 4 + j) * 4) * max[i];
            if (a < b) {
                newMin[j] += a;
                newMax[j] += b;
            } else {
                newMin[j] += b;
                newMax[j] += a;
            }
        }
    }
    min[0] = newMin[0];
    min[1] = newMin[1];
    min[2] = newMin[2];
    max[0] = newMax[0];
    max[1] = newMax[1];
    max[2] = newMax[2];
}

extern s32 D_801047F0; // JFG: the number of entries
s32 box_range_cell_mask(s16 *box, s32 minX, s32 minY, s32 minZ, s32 maxX, s32 maxY, s32 maxZ);

#ifdef NON_MATCHING
/**
 * JFG: collect the collision objects whose boxes the box from a to b touches (not exclude; at most 16, nearest
 * first), then list each object and those of its triangles in the cells the box covers (in batches matching mask1
 * unless they match mask2; mask1 -1: every batch, triangles with an edge flag 0x38), up to the level's limit.
 * NON_MATCHING: a first body written from the ROM, at the ROM's length and frame (453 words, 0x1C8), 24 diffs: register
 * choice only. In the object loop the ROM has i in a3 (here t0, and the other loop values one register on), and the
 * cell-mask call's stack arguments in v0/v1/t0 (here v1/t0/t1).
 */
void func_8007EB6C(f32 *a, f32 *b, s32 mask1, s32 mask2, Obj7E *exclude) {
    LevelHeader *level;
    Obj7E *objs[16];
    s32 count;
    s32 i;
    s32 j;
    s32 k;
    s32 inside;
    f32 dists[16];
    f32 dx;
    f32 dy;
    f32 dz;
    Obj7E *obj;
    f32 boxA[4];
    f32 boxB[4];
    Obj7EData *data;
    Obj7EModel *model;
    ModelHeader7E *header;
    ModelEdges7E *edges;
    s32 n;
    s32 batch;
    s32 tri;
    s32 start;
    s32 end;
    s32 useAll;
    s32 flags;
    u32 mask;
    s32 minX;
    s32 minY;
    s32 minZ;
    s32 maxX;
    s32 maxY;
    s32 maxZ;
    u32 masks[16];

    level = level_header();
    count = 0;
    for (i = 0; i < D_801047E0; i++) {
        obj = D_801047E4[i];
        if (obj == exclude) {
            continue;
        }
        if (!(obj->flags->flags & 1)) {
            continue;
        }
        data = obj->data;
        inside = TRUE;
        for (k = 0; k < 3 && inside; k++) {
            if (a[k] < data->bboxMin[k] && b[k] < data->bboxMin[k]) {
                inside = FALSE;
            } else if (data->bboxMax[k] < a[k] && data->bboxMax[k] < b[k]) {
                inside = FALSE;
            }
        }
        if (inside && count < 16) {
            dx = data->bboxMin[0] - a[0];
            dy = data->bboxMin[1] - a[1];
            dz = data->bboxMin[2] - a[2];
            objs[count] = obj;
            dists[count] = dx * dx + dy * dy + dz * dz;
            j = count;
            while (j > 0 && dists[j] < dists[j - 1]) {
                dx = dists[j]; // dx as the swap temporary
                dists[j] = dists[j - 1];
                dists[j - 1] = dx;
                obj = objs[j];
                objs[j] = objs[j - 1];
                objs[j - 1] = obj;
                j--;
            }
            count++;
            if (count >= 16) {
                i = D_801047E0;
            }
        }
    }
    for (n = 0; n < count; n++) {
        obj = objs[n];
        data = obj->data;
        model = obj->modelInstances[data->modelIndex];
        boxA[0] = a[0];
        boxA[1] = a[1];
        boxA[2] = a[2];
        boxA[3] = 1.0f;
        boxB[0] = b[0];
        boxB[1] = b[1];
        boxB[2] = b[2];
        boxB[3] = 1.0f;
        func_8007E9E0(data->mtx[data->mtxIndex], boxA, boxB);
        minX = boxA[0];
        minY = boxA[1];
        minZ = boxA[2];
        maxX = boxB[0];
        maxY = boxB[1];
        maxZ = boxB[2];
        masks[n] = box_range_cell_mask(model->bboxMin, minX, minY, minZ, maxX, maxY, maxZ);
    }
    if (mask1 == -1) {
        useAll = TRUE;
    } else {
        useAll = FALSE;
    }
    D_801047F0 = 0;
    for (n = 0; n < count; n++) {
        obj = objs[n];
        data = obj->data;
        model = obj->modelInstances[data->modelIndex];
        header = model->header;
        if (model->edges != NULL) {
            edges = model->edges;
        } else {
            edges = header->edges;
        }
        mask = masks[n];
        D_801047F4[D_801047F0] = (s32) obj & 0x7FFFFFFF;
        D_801047F0++;
        for (batch = 0; batch < header->numBatches; batch++) {
            flags = header->batches[batch].flags;
            start = header->batches[batch].facesOffset;
            end = header->batches[batch + 1].facesOffset;
            if (!(flags & mask2) && !useAll && (flags & mask1)) {
                start = end;
            }
            for (tri = start; tri < end; tri++) {
                start = edges->cellMasks[tri] & mask; // start (dead here) holds the triangle's cells
                if (useAll) {
                    if ((edges->edgeFlags[tri] & 0x38) && (start & 0xFF) && (start & 0xFF00)) {
                        D_801047F4[D_801047F0] = (s32) &edges->records[tri];
                        D_801047F8[D_801047F0] = batch;
                        D_801047FC[D_801047F0] = tri;
                        D_801047F0++;
                    }
                } else if (obj->type == 0x37) {
                    if ((start & 0xFF) && (start & 0xFF00)) {
                        D_801047F4[D_801047F0] = (s32) &edges->records[tri];
                        D_801047F8[D_801047F0] = batch;
                        D_801047FC[D_801047F0] = tri;
                        D_801047F0++;
                    }
                } else if ((start & 0xFF) && (start & 0xFF00) && (start & 0xFF0000)) {
                    D_801047F4[D_801047F0] = (s32) &edges->records[tri];
                    D_801047F8[D_801047F0] = batch;
                    D_801047FC[D_801047F0] = tri;
                    D_801047F0++;
                }
                if (D_801047F0 >= level->unkEE) {
                    batch = header->numBatches;
                    tri = end;
                    n = count;
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007EB6C);
#endif

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007F280);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007FC0C);

/* JFG: a collision edge of an object's model (0x30 bytes, as 12D50's TrackEdge) */
typedef struct ObjEdge {
    /* 0x00 */ f32 x0, y0, z0;
    /* 0x0C */ f32 x1, y1, z1;
    /* 0x18 */ f32 dirX, dirY, dirZ; // the unit vector from the first end to the second
    /* 0x24 */ f32 length;
    /* 0x28 */ Obj7E *obj;
    /* 0x2C */ s16 batch;
    /* 0x2E */ s16 unk2E; // JFG: set from the triangle's flag bit (edge << 2)
} ObjEdge;

extern s32 D_801047F0; // JFG: the number of entries
extern s32 D_80104804; // JFG: the number of object edges
void func_80048B60(f32 *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz);

#ifdef NON_MATCHING
/**
 * NON_MATCHING: i lives on the stack for the whole loop (the target keeps it in a3 and spills it only around the
 * triangle loop), which renumbers the temporaries. 135 aligned diffs at the ROM's length and frame (0xB0, with
 * the pad after mtx; 139 at frame 0xA8).
 * JFG: build the object edge list from the entries, as track_edges_build does for the level: an object entry
 * selects the object (its current model and transform), a triangle entry adds the model triangle's marked
 * edges, transformed to world space (zero-length edges are skipped), up to the level header's limit.
 */
void func_800805F4(void) {
    Obj7E *obj;
    UNUSED s32 pad[3];
    Obj7EModel *model;
    ModelHeader7E *header;
    ModelEdges7E *edges;
    UNUSED s32 pad2[3];
    LevelHeader *level;
    f32 *mtx;
    UNUSED s32 pad3[2];
    s32 i;
    s32 j;
    s32 k;
    s32 bit;
    s32 tri;
    Triangle7E *triangle;
    Vertex7E *verts;
    Vertex7E *va;
    Vertex7E *vb;
    ObjEdge *edge;
    f32 dx;
    f32 dy;
    f32 dz;

    level = level_header();
    D_80104804 = 0;
    for (i = 0; i < D_801047F0; i++) {
        if (D_801047F4[i] > 0) {
            obj = (Obj7E *) (D_801047F4[i] | 0x80000000);
            model = obj->modelInstances[obj->data->modelIndex];
            header = model->header;
            if (model->edges == NULL) {
                edges = header->edges;
            } else {
                edges = model->edges;
            }
            mtx = obj->data->mtx2[obj->data->mtxIndex][0];
        } else {
            tri = D_801047FC[i];
            triangle = &header->triangles[tri];
            verts = &model->vertices[header->batches[D_801047F8[i]].verticesOffset];
            for (j = 0; j < 3; j++) {
                bit = 1 << j;
                if (edges->edgeFlags[tri] & bit) {
                    edge = &D_80104800[D_80104804];
                    k = j + 1;
                    if (k >= 3) {
                        k = 0;
                    }
                    va = &verts[triangle->vi[j]];
                    vb = &verts[triangle->vi[k]];
                    func_80048B60(mtx, va->x, va->y, va->z, &edge->x0, &edge->y0, &edge->z0);
                    func_80048B60(mtx, vb->x, vb->y, vb->z, &edge->x1, &edge->y1, &edge->z1);
                    dx = edge->x1 - edge->x0;
                    dy = edge->y1 - edge->y0;
                    dz = edge->z1 - edge->z0;
                    edge->length = sqrtf(dx * dx + dy * dy + dz * dz);
                    if (edge->length > 0.0f) {
                        edge->dirX = dx / edge->length;
                        edge->dirY = dy / edge->length;
                        edge->dirZ = dz / edge->length;
                        edge->obj = obj;
                        edge->unk2E = 0;
                        edge->batch = D_801047F8[i];
                        if (header->triangles[tri].flags & (bit << 2)) {
                            edge->unk2E = 1;
                        }
                        if (++D_80104804 >= level->unkF0) {
                            i = D_801047F0;
                            j = 3;
                        }
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800805F4);
#endif

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800809F4);

/* JFG: an object edge crossed by a line (func_80081658) */
typedef struct EdgeHit7E {
    /* 0x00 */ f32 x0, y0, z0; // the edge's ends, in the object's space
    /* 0x0C */ f32 x1, y1, z1;
    /* 0x18 */ f32 along;      // where the line crosses it, 0..1 from the first end
    /* 0x1C */ Plane7E *plane; // the plane of the edge's triangle
} EdgeHit7E;

#ifdef NON_MATCHING
/**
 * JFG: the object twin of func_80019888: the first object edge (among the entries' triangle edges marked in
 * edgeFlags >> 3) that the line from..to, taken into the object's space by its inverse transform, passes through,
 * taking each edge as a wall 10 units high: its ends, the crossing point along it and its triangle's plane go to
 * out, the object to outObj. Returns 1 or -1 for the side the line crosses from, 0 if none. As in the ROM, the
 * object values are only set by an earlier object entry.
 * NON_MATCHING: a first body written from the ROM, at the ROM's length and frame (408 words, 0x158), 279 aligned
 * diffs: the t-registers from the loop top on and the FP temporaries of the triangle code (the ROM's 0.0/1.0
 * constants are f22/f24, here f24/f26, and t / d is in f0 there).
 */
s32 func_80081658(f32 *from, f32 *to, EdgeHit7E *out, Obj7E **outObj) {
    s32 result;
    s32 j;
    s32 k;
    s32 side;
    ModelHeader7E *header;
    Obj7EModel *model;
    ModelEdges7E *edges;
    Obj7E *obj;
    f32 *mtx;
    s32 tri;
    f32 p[3];
    f32 q[3];
    f32 dir[3];
    f32 n[3];
    f32 v[3];
    f32 a[3];
    f32 b[3];
    f32 e[3];
    s32 batch;
    Triangle7E *triangle;
    Vertex7E *verts;
    u8 flags;
    s32 i;
    Vertex7E *va;
    Vertex7E *vb;
    f32 d;
    f32 t;
    s32 addr;
    UNUSED s32 pad1;

    result = 0;
    for (i = 0; i < D_801047F0 && !result; i++) {
        addr = D_801047F4[i];
        if (addr > 0) {
            obj = (Obj7E *) (addr | 0x80000000);
            model = obj->modelInstances[obj->data->modelIndex];
            header = model->header;
            if (model->edges != NULL) {
                edges = model->edges;
            } else {
                edges = header->edges;
            }
            mtx = obj->data->mtx[obj->data->mtxIndex][0];
            func_80048B60(mtx, from[0], from[1], from[2], &p[0], &p[1], &p[2]);
            func_80048B60(mtx, to[0], to[1], to[2], &q[0], &q[1], &q[2]);
            dir[0] = q[0] - p[0];
            dir[1] = q[1] - p[1];
            dir[2] = q[2] - p[2];
        } else {
            batch = D_801047F8[i];
            tri = D_801047FC[i];
            triangle = &header->triangles[tri];
            verts = &model->vertices[header->batches[batch].verticesOffset];
            flags = (u32) edges->edgeFlags[tri] >> 3;
            j = 0;
            if (flags & 7) {
                for (; j < 3; j++) {
                    if (flags & (1 << j)) {
                        k = j + 1;
                        if (k >= 3) {
                            k = 0;
                        }
                        va = &verts[triangle->vi[j]];
                        vb = &verts[triangle->vi[k]];
                        a[0] = va->x;
                        a[1] = va->y;
                        a[2] = va->z;
                        b[0] = vb->x;
                        b[1] = vb->y;
                        b[2] = vb->z;
                        e[0] = b[0] - a[0];
                        e[1] = b[1] - a[1];
                        e[2] = b[2] - a[2];
                        v[0] = 0.0f;
                        v[1] = 10.0f;
                        v[2] = 0.0f;
                        n[0] = v[1] * e[2] - v[2] * e[1];
                        n[1] = v[2] * e[0] - v[0] * e[2];
                        n[2] = v[0] * e[1] - v[1] * e[0];
                        d = dir[0] * n[0] + dir[1] * n[1] + dir[2] * n[2];
                        if (d != 0.0f) {
                            side = 1;
                            if (d < 0.0f) {
                                side = -1;
                            }
                            v[0] = n[0] * (a[0] - p[0]);
                            v[1] = n[1] * (a[1] - p[1]);
                            v[2] = n[2] * (a[2] - p[2]);
                            t = (v[0] + v[1] + v[2]) / d;
                            if (t >= 0.0f && t <= 1.0f) {
                                v[0] = dir[0] * t + p[0];
                                v[1] = dir[1] * t + p[1];
                                v[2] = dir[2] * t + p[2];
                                d = e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
                                v[0] = e[0] * (v[0] - a[0]);
                                v[1] = e[1] * (v[1] - a[1]);
                                v[2] = e[2] * (v[2] - a[2]);
                                t = v[0] + v[1] + v[2];
                                if (t / d >= 0.0f && t / d <= 1.0f) {
                                    result = side;
                                    out->x0 = a[0];
                                    out->y0 = a[1];
                                    out->z0 = a[2];
                                    out->x1 = b[0];
                                    out->y1 = b[1];
                                    out->z1 = b[2];
                                    out->plane = &edges->planes[edges->records[tri].plane];
                                    out->along = t / d;
                                    *outObj = obj;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081658);
#endif

#ifdef NON_MATCHING
/* JFG: find the object edge (flag bits 3-5 of its triangle) that continues the line: one ending at
   the line's start (line[6] < 0.5) or starting at its end; copies its ends to out and returns 1.
   NON_MATCHING: the right length and stack; the entry count sits in v0 (target a3), which
   renumbers the loop's temporaries. k carrying the model index and the batch index (each dead by its next
   assignment) took the aligned distance 96 -> 65. */
s32 func_80081CB8(f32 *line, f32 *out) {
    s32 found;
    s32 i;
    s32 j;
    s32 k;
    ModelHeader7E *header;
    Obj7EModel *model;
    ModelEdges7E *edges;
    Obj7E *obj;
    s32 tri;
    s32 start[3];
    s32 end[3];
    Triangle7E *triangle;
    Vertex7E *verts;
    Vertex7E *va;
    Vertex7E *vb;
    u8 flags;

    start[0] = line[0];
    start[1] = line[1];
    start[2] = line[2];
    end[0] = line[3];
    end[1] = line[4];
    end[2] = line[5];
    found = 0;
    for (i = 0; i < D_801047F0 && !found; i++) {
        if (D_801047F4[i] > 0) {
            obj = (Obj7E *) (D_801047F4[i] | 0x80000000);
            k = obj->data->modelIndex;
            model = obj->modelInstances[k];
            header = model->header;
            if (model->edges != NULL) {
                edges = model->edges;
            } else {
                edges = header->edges;
            }
        } else {
            tri = D_801047FC[i];
            triangle = &header->triangles[tri];
            k = D_801047F8[i];
            verts = &model->vertices[header->batches[k].verticesOffset];
            flags = edges->edgeFlags[tri] >> 3;
            if (flags & 7) {
                for (j = 0; j < 3; j++) {
                    if (flags & (1 << j)) {
                        k = j + 1;
                        if (k >= 3) {
                            k = 0;
                        }
                        va = &verts[triangle->vi[j]];
                        vb = &verts[triangle->vi[k]];
                        if (line[6] < 0.5f) {
                            if (start[0] == vb->x && start[1] == vb->y && start[2] == vb->z) {
                                found = 1;
                                out[0] = va->x;
                                out[1] = va->y;
                                out[2] = va->z;
                                out[3] = vb->x;
                                out[4] = vb->y;
                                out[5] = vb->z;
                            }
                        } else if (end[0] == va->x && end[1] == va->y && end[2] == va->z) {
                            found = 1;
                            out[0] = va->x;
                            out[1] = va->y;
                            out[2] = va->z;
                            out[3] = vb->x;
                            out[4] = vb->y;
                            out[5] = vb->z;
                        }
                    }
                }
            }
        }
    }
    return found;
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081CB8);
#endif

/* JFG: every moving type-0x5B object (other than self) that is moving down and whose box overlaps
 * the box spanned by a and b gets state->unk0 = -32000 and flag bit 0 cleared. */
void falling_objects_hit_box(f32 *a, f32 *b, Obj7E *self) {
    s32 i;
    s32 j;
    Obj7E *obj;
    Obj7EData *data;
    Obj7EState *state;
    s32 hit;
    f32 dy;

    for (i = 0; i < D_801047E0; i++) {
        obj = D_801047E4[i];
        data = obj->data;
        if (obj->type == 0x5B && obj != self && data->unk12C && (obj->flags->flags & 1)) {
            dy = obj->y - data->prevY;
            if (dy < 0.0f) {
                j = 0;
                hit = TRUE;
                do {
                    if (a[j] < data->bboxMin[j] && b[j] < data->bboxMin[j]) {
                        hit = FALSE;
                    } else if (data->bboxMax[j] < a[j] && data->bboxMax[j] < b[j]) {
                        hit = FALSE;
                    }
                    j++;
                } while (j < 3 && hit);
                if (hit) {
                    state = obj->state;
                    state->unk1C = 1;
                    state->unk0 = -32000;
                    obj->flags->flags &= ~1;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800822D8);

/* JFG: an object height under a point: the plane's height there, its scaled a/b/c and the object */
typedef struct ObjHeight {
    /* 0x00 */ f32 y;
    /* 0x04 */ f32 a;
    /* 0x08 */ f32 b;
    /* 0x0C */ f32 c;
    /* 0x10 */ Obj7E *obj;
} ObjHeight;

typedef struct ModelPlanes7E {
    /* 0x00 */ u8 *faces; // JFG: 14 bytes per triangle, the plane index first
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ f32 (*normals)[4];
} ModelPlanes7E;

typedef struct ModelHeader2D8 {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
    /* 0x20 */ Triangle7E *triangles;
    /* 0x24 */ struct ModelBatch2D8 *batches;
    /* 0x28 */ u8 pad28[0x60 - 0x28];
    /* 0x60 */ ModelPlanes7E *planes;
} ModelHeader2D8;

typedef struct ModelBatch2D8 {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ u32 flags;
} ModelBatch2D8;

typedef struct ObjModel2D8 {
    /* 0x00 */ ModelHeader2D8 *header;
    /* 0x04 */ Vertex7E *vertices;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ ModelPlanes7E *planes; // JFG: this instance's own planes (else the header's)
} ObjModel2D8;

extern ObjHeight D_80104C68[20];
extern ObjHeight *D_80104DF8[20];
void func_8004A2FC(f32 *mtx, f32 *in, f32 *out);

#ifdef NON_MATCHING
/**
 * JFG: every object height under (x, z), as func_8001B078 (12D50) finds the level's: the moving objects whose box
 * holds the point (up to 16), then each up-facing triangle of their current model (batches with any of flags, or
 * 0x880, are skipped) whose transformed corners hold the point in x/z, up to 20; sorted from the highest down,
 * *heights gets the sorted list and the count is returned. inside doubles as the sort's done flag.
 * NON_MATCHING: a first body written from the ROM: the ROM's frame (0x198) and length (536 words; the plane's
 * a/b/c are read before the up-facing test, as the ROM loads them). The ROM keeps the model header in s6 and the
 * total in s7 and scales the vertex indices by shifts; IDO here keeps 10 in a saved register and multiplies,
 * which moves the saved registers round (written out as shifts, the indices give the ROM's s2-s6).
 */
s32 func_80082D80(f32 x, f32 z, s32 flags, ObjHeight ***heights) {
    Obj7E *list[16];
    Obj7E *obj;
    s32 i;
    Obj7EData *data;
    ObjModel2D8 *model;
    ModelPlanes7E *planes;
    ModelHeader2D8 *header;
    s32 inside;
    s32 total;
    f32 (*normals)[4];
    f32 *pointMtx;
    f32 *mtx;
    ModelBatch2D8 *batch;
    s32 vOff;
    s32 end;
    f32 plane[4];
    f32 scale;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 d;
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    Vertex7E *verts;
    Triangle7E *tri;
    Vertex7E *v0;
    s32 m;
    s32 count;
    Vertex7E *v1;
    Vertex7E *v2;
    s32 test1;
    s32 test2;
    ObjHeight **waves;
    s32 b;
    s32 f;
    ObjHeight *wave;

    *heights = NULL;
    count = 0;
    for (i = 0; i < D_801047E0; i++) {
        obj = D_801047E4[i];
        if (obj->flags->flags & 1) {
            data = obj->data;
            inside = FALSE;
            if (data->bboxMin[0] <= x && x <= data->bboxMax[0] && data->bboxMin[2] <= z && z <= data->bboxMax[2]) {
                inside = TRUE;
            }
            if (inside) {
                list[count++] = obj;
            }
            if (count >= 16) {
                i = D_801047E0;
            }
        }
    }
    if (count == 0) {
        return 0;
    }
    flags |= 0x880;
    total = 0;
    for (m = 0; m < count; m++) {
        obj = list[m];
        data = obj->data;
        model = (ObjModel2D8 *) obj->modelInstances[data->modelIndex];
        header = model->header;
        if (model->planes != NULL) {
            planes = model->planes;
        } else {
            planes = header->planes;
        }
        normals = planes->normals;
        mtx = data->mtx[data->mtxIndex][0];
        pointMtx = data->mtx2[data->mtxIndex][0];
        scale = obj->scale;
        for (b = 0; b < header->numberOfBatches; b++) {
            batch = &header->batches[b];
            f = batch->facesOffset;
            vOff = batch->verticesOffset;
            end = batch[1].facesOffset;
            if (batch->flags & flags) {
                f = end;
            }
            for (; f < end; f++) {
                func_8004A2FC(mtx, normals[*(u16 *) (planes->faces + f * 14)], plane);
                inside = FALSE;
                nx = plane[0];
                ny = plane[1];
                nz = plane[2];
                if (0.0f < ny) {
                    verts = &model->vertices[vOff];
                    tri = &header->triangles[f];
                    v0 = verts + tri->vi[0];
                    v1 = verts + tri->vi[1];
                    v2 = verts + tri->vi[2];
                    nx *= scale;
                    ny *= scale;
                    nz *= scale;
                    d = plane[3] * scale;
                    func_80048B60(pointMtx, v0->x, v0->y, v0->z, &x0, &y0, &z0);
                    func_80048B60(pointMtx, v1->x, v1->y, v1->z, &x1, &y1, &z1);
                    func_80048B60(pointMtx, v2->x, v2->y, v2->z, &x2, &y2, &z2);
                    test1 = (x2 - x1) * (z - z1) <= (z2 - z1) * (x - x1);
                    test2 = (x1 - x0) * (z - z0) <= (z1 - z0) * (x - x0);
                    if (test2 == test1) {
                        test2 = (x0 - x2) * (z - z2) <= (z0 - z2) * (x - x2);
                        if (test1 == test2) {
                            inside = TRUE;
                        }
                    }
                    if (inside) {
                        D_80104C68[total].a = nx;
                        D_80104C68[total].b = ny;
                        D_80104C68[total].c = nz;
                        D_80104C68[total].obj = obj;
                        D_80104C68[total].y = -((nx * x + nz * z + d) / ny);
                        total++;
                    }
                    if (total >= 20) {
                        f = end;
                        b = header->numberOfBatches;
                        m = count;
                    }
                }
            }
        }
    }
    waves = D_80104DF8;
    wave = D_80104C68;
    // clang-format off
    for (m = 0; m < total; m++) { waves[m] = &wave[m]; }
    // clang-format on
    do {
        m = 0;
        inside = TRUE;
        for (; m < total - 1; m++) {
            if (waves[m]->y < waves[m + 1]->y) {
                wave = waves[m];
                waves[m] = waves[m + 1];
                waves[m + 1] = wave;
                inside = FALSE;
            }
        }
    } while (!inside);
    *heights = D_80104DF8;
    return total;
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80082D80);
#endif

/* JFG: the C form of func_8001D464 for three axes: clamp the range to the box, split each axis of the box into
 * 8 cells and return a 24-bit mask of the cells the range overlaps (x in bits 0-7, z in 8-15, y in 16-23). */
s32 box_range_cell_mask(s16 *box, s32 minX, s32 minY, s32 minZ, s32 maxX, s32 maxY, s32 maxZ) {
    s32 mask;
    s32 bit;
    u32 i;
    s16 step;
    s16 cellEnd;
    s16 cellStart;

    if (box == NULL) {
        return 0;
    }
    if (maxX < box[0]) {
        maxX = box[0];
    }
    if (minX < box[0]) {
        minX = box[0];
    }
    if (maxY < box[1]) {
        maxY = box[1];
    }
    if (minY < box[1]) {
        minY = box[1];
    }
    if (maxZ < box[2]) {
        maxZ = box[2];
    }
    if (minZ < box[2]) {
        minZ = box[2];
    }
    if (box[3] < maxX) {
        maxX = box[3];
    }
    if (box[3] < minX) {
        minX = box[3];
    }
    if (box[4] < maxY) {
        maxY = box[4];
    }
    if (box[4] < minY) {
        minY = box[4];
    }
    if (box[5] < maxZ) {
        maxZ = box[5];
    }
    if (box[5] < minZ) {
        minZ = box[5];
    }
    mask = 0;
    bit = 1;
    step = ((box[3] - box[0]) >> 3) + 1;
    cellEnd = box[0];
    cellEnd = step + cellEnd;
    cellStart = box[0];
    for (i = 0; i < 8; i++) {
        if (cellEnd >= minX && maxX >= cellStart) {
            mask |= bit;
        }
        cellEnd += step;
        cellStart += step;
        bit <<= 1;
    }
    step = ((box[5] - box[2]) >> 3) + 1;
    cellEnd = box[2];
    cellEnd = step + cellEnd;
    cellStart = box[2];
    for (i = 0; i < 8; i++) {
        if (cellEnd >= minZ && maxZ >= cellStart) {
            mask |= bit;
        }
        cellEnd += step;
        cellStart += step;
        bit <<= 1;
    }
    step = ((box[4] - box[1]) >> 3) + 1;
    cellEnd = box[1];
    cellEnd = step + cellEnd;
    cellStart = box[1];
    for (i = 0; i < 8; i++) {
        if (cellEnd >= minY && maxY >= cellStart) {
            mask |= bit;
        }
        cellEnd += step;
        cellStart += step;
        bit <<= 1;
    }
    return mask;
}

typedef struct MassInfo {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ f32 mass;
} MassInfo;

typedef struct MassObject {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ Vec3f velocity;
    /* 0x28 */ u8 pad28[0x4C - 0x28];
    /* 0x4C */ MassInfo *info;
} MassObject;

/* JFG: the velocity of two objects together, each weighted by its share of the total mass. */
void velocity_mass_weighted(MassObject *a, MassObject *b, f32 *x, f32 *y, f32 *z) {
    f32 massA;
    f32 massB;
    f32 shareA;
    f32 shareB;
    f32 total;

    if (a != NULL && b != NULL) {
        massA = a->info->mass;
        massB = b->info->mass;
        if (massA <= 0.0f) {
            if (massB <= 0.0f) {
                shareA = 0.5f;
                shareB = 0.5f;
            } else {
                shareA = 1.0f;
                shareB = 0.0f;
            }
        } else if (massB <= 0.0f) {
            shareA = 0.0f;
            shareB = 1.0f;
        } else {
            total = massA + massB;
            shareA = massA / total;
            shareB = massB / total;
        }
        *x = a->velocity.x * shareA + b->velocity.x * shareB;
        *y = a->velocity.y * shareA + b->velocity.y * shareB;
        *z = a->velocity.z * shareA + b->velocity.z * shareB;
    } else if (a == NULL && b == NULL) {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    } else if (a == NULL) {
        *x = b->velocity.x;
        *y = b->velocity.y;
        *z = b->velocity.z;
    } else {
        *x = a->velocity.x;
        *y = a->velocity.y;
        *z = a->velocity.z;
    }
}
