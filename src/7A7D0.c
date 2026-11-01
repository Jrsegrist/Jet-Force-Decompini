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
    /* 0x133 */ u8 unk133;
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
    /* 0x18 */ u8 pad18[0x3A - 0x18];
    /* 0x3A */ s8 unk3A;
    /* 0x3B */ u8 pad3B[0x40 - 0x3B];
    /* 0x40 */ Obj7EHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 type;
    /* 0x4A */ u8 pad4A[2];
    /* 0x4C */ struct Obj7EFlags *flags;
    /* 0x50 */ u8 pad50[0x5C - 0x50];
    /* 0x5C */ Obj7EData *data;
    /* 0x60 */ u8 pad60[0x68 - 0x60];
    /* 0x68 */ struct Obj7EState *state;
    /* 0x6C */ struct Obj7EModel **modelInstances;
} Obj7E;

typedef struct Obj7EModel {
    /* 0x00 */ u8 pad0[0x8C];
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
extern void *D_801047F4;
extern void *D_801047F8;
extern void *D_801047FC;
extern void *D_80104800;
void *mempool_alloc_safe(s32 size, u32 colourTag);
void mempool_free(void *data);

void func_80079BD0(void) {
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

void func_80079C20(void) {
    LevelHeader *header;

    D_801047E0 = 0;
    header = level_header();
    D_80104800 = mempool_alloc_safe(header->unkF0 * 0x30, 0xFFFF);
    D_801047F4 = mempool_alloc_safe(header->unkEE * 4, 0xFFFF);
    D_801047F8 = mempool_alloc_safe(header->unkEE * 2, 0xFFFF);
    D_801047FC = mempool_alloc_safe(header->unkEE * 2, 0xFFFF);
}

void func_80079CC4(void) {
    mempool_free(D_80104800);
    mempool_free(D_801047F4);
    mempool_free(D_801047F8);
    mempool_free(D_801047FC);
}

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80079D1C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A018);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007A324);

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

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words near the end (work in progress) */
s32 func_8007BB00(CollObj *self, Vec3f *start, Vec3f *end, f32 *dist, s32 skipType1, s32 skipType2) {
    s32 first;
    s32 last;
    CollQuery query;
    CollObj *hitObj;
    CollObj **objs;
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
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007BB00);
#endif

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007BCD0);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007C2E8);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007CCF0);

/* Size: 0x34 bytes */
typedef struct CollisionSphere {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ u8 pad18[0x24 - 0x18];
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0x34 - 0x28];
} CollisionSphere;

typedef struct SphereSet {
    /* 0x00 */ u8 pad0[0x32];
    /* 0x32 */ s16 count;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ CollisionSphere *spheres;
} SphereSet;

f32 sqrtf(f32 x);

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

#ifdef NON_MATCHING
/* NON_MATCHING: float register allocation (r/dx/dy/dz in f16-f22 rotated against the target) */
/* JFG: TRUE if the sphere overlaps any sphere of the set. */
s32 func_8007D0B4(CollisionSphere *sphere, SphereSet *set) {
    s32 i = set->count;
    CollisionSphere *other = set->spheres;
    f32 x = sphere->pos.x;
    f32 y = sphere->pos.y;
    f32 z = sphere->pos.z;
    f32 radius = sphere->radius;
    f32 dx, dy, dz, r;

    while (i--) {
        r = other->radius + radius;
        r *= r;
        dx = other->pos.x - x;
        dy = other->pos.y - y;
        dz = other->pos.z - z;
        other++;
        if (dx * dx + dy * dy + dz * dz < r) {
            return TRUE;
        }
    }
    return FALSE;
}
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007D0B4);
#endif

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007D168);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007DE98);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E150);

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

Obj7E **func_8007E484(s32 *count) {
    *count = D_801047E0;
    return D_801047E4;
}

void obj_update_matrices(Obj7E *obj, s32 arg1);
void func_8007E794(Obj7E *obj);

void func_8007E4A0(Obj7E *obj) {
    Obj7EData *data = obj->data;
    s8 value = obj->header->unk71;

    if (value >= 0) {
        data->unk133 = value;
    } else {
        data->unk133 = obj->unk3A;
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
            data->unk133 = value;
        } else {
            data->unk133 = obj->unk3A;
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

/* JFG: the object's world bounding box: its model's box transformed by its current matrix
 * (same pattern as func_8007E9E0; frame and loop shape not reproduced yet). */
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E794);

#ifdef NON_MATCHING
/* NON_MATCHING: loop shape only: the target keeps j as a counter (j != 4, mtx + j * 4 recomputed); mine turns it into pointers */
/* JFG: transform a bounding box (min and max corners) by a matrix, giving the box around the result. */
void func_8007E9E0(f32 mtx[4][4], f32 *min, f32 *max) {
    f32 newMin[4];
    s32 i;
    f32 newMax[4];
    s32 j;
    f32 a;
    f32 b;

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
            a = mtx[i][j] * min[i];
            b = mtx[i][j] * max[i];
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
#else
INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007E9E0);
#endif

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007EB6C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007F280);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_8007FC0C);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800805F4);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800809F4);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081658);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80081CB8);

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

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_80082D80);

INCLUDE_ASM("asm/nonmatchings/7A7D0", func_800835E0);

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
