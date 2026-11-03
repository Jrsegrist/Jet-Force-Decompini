/*
 * Runlink module 122: a thrown spinning blade. Init (at most ten of them
 * exist: an eleventh frees the oldest) places it at the object, loads its
 * model and sets its hit box; control flies it at its target (aimed at by
 * 00374); the matrix function puts it in its target's hand or at its own
 * rotation, and the bounce function knocks it back with a random spin.
 */
#include "module.h"

/*
 * The holder's obj->unk5C as the matrix function sees it: joint matrices
 * from 0x80, and at 0x12B the index of the one the blade is held by.
 */
typedef struct M122Joints {
    /* 0x00 */ u8 pad0[0x80];
    /* 0x80 */ MtxF mtx[4];
} M122Joints;

/* The same, as control sees it */
typedef struct M122Joints2 {
    /* 0x000 */ MtxF mtxA[2];
    /* 0x080 */ MtxF mtxB[2];
    /* 0x100 */ u8 pad100[0x12A - 0x100];
    /* 0x12A */ s8 unk12A;
    /* 0x12B */ u8 index;
    /* 0x12C */ s16 active;
    /* 0x12E */ u8 pad12E[0x130 - 0x12E];
    /* 0x130 */ s16 damage;
} M122Joints2;

/* What func_80016208 fills in */
typedef struct M122Hit {
    /* 0x00 */ Object *obj;
    /* 0x04 */ Vec3f normal;
    /* 0x10 */ u8 pad10[0x20 - 0x10];
    /* 0x20 */ s32 flags;
    /* 0x24 */ u8 kind;
    /* 0x25 */ u8 pad25[0x30 - 0x25];
} M122Hit;

/* The joint a target is aimed at by (module 3's func_m003_0D2C0) */
typedef struct M122Aim {
    /* 0x00 */ u8 pad0[0x36];
    /* 0x36 */ u8 joint;
} M122Aim;

/* What a target's interactObj points at */
typedef struct M122Interaction {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ f32 height;
} M122Interaction;

typedef struct M122Instance {
    /* 0x00 */ void *model;
    /* 0x04 */ u8 pad4[0xB - 0x4];
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *matrices[2];
} M122Instance;

/* Kept in obj->unk68 */
typedef struct M122State {
    /* 0x00 */ s32 modelId;
    /* 0x04 */ void *model;
    /* 0x08 */ Vec3f pos;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ u8 pad20[0x24 - 0x20];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x30 - 0x2C];
    /* 0x30 */ Object *holder;
    /* 0x34 */ MtxF mtx; /* relative to the holder's hand */
    /* 0x74 */ s16 unk74;
    /* 0x76 */ s16 prevYaw;
    /* 0x78 */ s16 angle;
    /* 0x7A */ s16 spin[3];
    /* 0x80 */ u16 timer;
    /* 0x82 */ u8 flags;
    /* 0x83 */ u8 pad83;
    /* 0x84 */ u32 age;
} M122State;

/* What init gets */
typedef struct M122Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ s32 modelId;
} M122Setup;

extern void *chain_slot_find_shortest(s32 id, u8 *arg1); /* loads a model */
extern void obj_list_insert_after(Object *obj, void *model);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out); /* out = a * b */
extern void func_80049DC0(Object *obj, MtxF *out);
extern void func_80049AE8(s32 angle, MtxF *mtx);
extern void mtx_model_push(Gfx **dl, Mtx **mtx, MtxF *m);
extern M122Aim *func_m003_0D2C0(Object *obj);
extern void func_m003_058A4(Object *obj);
extern void func_m105_01474(Object *obj, f32 *x, f32 *y, f32 *z, f32 *ox, f32 *oy, f32 *oz);
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz);                       /* moves the object */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void obj_list_unlink(Object *obj);
extern f32 point_distance(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2); /* distance */
extern s16 func_80049C84(f32 x, f32 z);
extern s32 func_80049D80(s32 from, s32 to); /* the difference between two angles */
extern s32 D_m122_010B0;                    /* the module's number */
extern f32 func_80097140(f32 x);        /* sqrtf */
void func_m122_00FE4(Object *obj, M122State *s);

void func_m122_00000(Object *obj, M122Setup *setup) {
    M122State *s;
    s32 first;
    s32 last;
    Object **objects;
    s32 count;
    u32 maxAge;
    u8 unused;
    s32 i;
    Object *other;
    Object *oldest;

    s = obj->unk68;
    count = 0;
    maxAge = 0;
    objects = func_80005E98(&first, &last);
    for (i = first; i < last; i++) {
        other = objects[i];
        if (other->behaviorId == 0x99 && !other->freed) {
            count++;
            if (maxAge < ((M122State *) other->unk68)->age) {
                oldest = other;
                maxAge = ((M122State *) other->unk68)->age;
            }
        }
    }
    if (count >= 11) {
        func_800079B8(oldest);
    }
    s->flags = 2;
    s->pos.x = obj->trans.x_position;
    s->pos.y = obj->trans.y_position;
    s->pos.z = obj->trans.z_position;
    s->unk74 = 0;
    s->unk14 = 0.0f;
    s->unk18 = 0.0f;
    s->unk1C = 0.0f;
    s->unk24 = 0.0f;
    s->unk28 = 0.0f;
    s->modelId = setup->modelId;
    s->timer = func_80005064(600);
    s->age = 0;
    func_80079D1C(obj, 0xA00, 1, 2, 0, 0, 0, 16.0f, 16.0f, 1.0f, 0.75f, 0x11);
    obj->unk7C[0] |= 2;
    if ((s->model = chain_slot_find_shortest(s->modelId, &unused)) != NULL) {
        obj_list_insert_after(obj, s->model);
    } else {
        s->flags |= 8;
    }
}

/* Where on the target to aim: its aim joint, or a bit above its position */
static s32 func_m122_00374(Object *target, f32 *x, f32 *y, f32 *z) {
    M122Aim *aim;
    M122Instance *inst;

    if (target == NULL || target->freed) {
        stubbed_printf("GNL:NULL Target\n");
        return FALSE;
    }
    if (target->behaviorId == 0x18) {
        if ((aim = func_m003_0D2C0(target)) != NULL) {
            func_m003_058A4(target);
            inst = ((M122Instance **) target->modelInstances)[target->modelIndex];
            /* (the no-op mask is what gives the ROM's temp registers) */
            func_80048B60(&inst->matrices[inst->matrixIndex][aim->joint & 0xFFFF], 0.0f, 0.0f, 0.0f, x, y, z);
            return TRUE;
        }
        *x = target->trans.x_position;
        *y = target->trans.y_position + 40.0f;
        *z = target->trans.z_position;
    } else if (target->behaviorId == 1 || target->behaviorId == 0x5A) {
        *x = target->trans.x_position;
        *y = target->trans.y_position + ((M122Interaction *) target->interactObj)->height * 0.75f;
        *z = target->trans.z_position;
    } else {
        *x = target->trans.x_position;
        *y = target->trans.y_position;
        *z = target->trans.z_position;
    }
    return TRUE;
}

/*
 * Control: sticks with what it hit, spins after a bounce, or flies: homes
 * in on its target (turning at most 1500 a frame), and sticks into or
 * bounces off what it hits; when its time runs out it heads back.
 */
void func_m122_004EC(Object *obj, s32 updateRate) {
    M122State *s;
    Object *holder;
    M122Joints2 *joints;
    Vec3f *relPos;
    f32 fRate;
    M122Hit hit;
    f32 nvx;
    f32 dy;
    f32 nvz;
    f32 ax;
    f32 az;
    s16 yaw1;
    s16 yaw0;
    s16 roll;
    f32 speed;
    f32 tx;
    f32 ty;
    f32 tz;
    MtxF m;

    s = obj->unk68;
    fRate = updateRate;
    runlink_unload_timer_set(D_m122_010B0, 2, 2);
    s->age++;
    if (obj->interactObj->hitPoints != 0xA00) {
        obj->interactObj->hitPoints = 0xA00;
        s->flags &= ~1;
        s->flags |= 0x20;
    }
    relPos = (Vec3f *) s->mtx[3];
    if (s->flags & 1) {
        if (s->holder != NULL) {
            holder = s->holder;
            joints = (M122Joints2 *) holder->unk5C;
            if (holder->behaviorId == 0x37) {
                func_m105_01474(holder, &relPos->x, &relPos->y, &relPos->z, &obj->trans.x_position,
                                &obj->trans.y_position, &obj->trans.z_position);
            } else if (joints->active != 0) {
                func_80048B60(&joints->mtxB[joints->index], relPos->x, relPos->y, relPos->z, &tx, &ty, &tz);
                func_80009A04(obj, tx - obj->trans.x_position, ty - obj->trans.y_position, tz - obj->trans.z_position);
            }
        }
        return;
    }
    if (s->flags & 0x20) {
        if (!(obj->unk7C[0] & 2)) {
            obj->trans.rotation.y_rotation += s->spin[0];
            obj->trans.rotation.x_rotation += s->spin[1];
            obj->trans.rotation.z_rotation += s->spin[2];
        }
        return;
    }
    s->angle += 0x1D6B;
    s->pos.x = obj->trans.x_position;
    s->pos.y = obj->trans.y_position;
    s->pos.z = obj->trans.z_position;
    func_80009A04(obj, obj->velocity.x * fRate, obj->velocity.y * fRate, obj->velocity.z * fRate);
    func_8001CE10(1, &s->pos, (Vec3f *) &obj->trans.x_position, &obj->header->unk50->radius, 0xCE000000, 0, 0);
    joints = (M122Joints2 *) obj->header->unk50;
    if (func_80016208(obj, &s->pos, (Vec3f *) &obj->trans.x_position, ((ObjectHeaderUnk50 *) joints)->radius,
                      ((ObjectHeaderUnk50 *) joints)->unk10, &hit)) {
        if (hit.obj != NULL && (*(u16 *) ((u8 *) hit.obj->header + 0x14) & 0x8000)) {
            func_m122_00FE4(obj, s);
        } else if (hit.obj != NULL && hit.obj->behaviorId == 0x4E) {
            OBJ_STATE(hit.obj, s32)[0] = 1;
            ((M122Joints2 *) hit.obj->unk5C)->damage = 10;
            ((M122Joints2 *) hit.obj->unk5C)->unk12A = 1;
        } else if (hit.obj != NULL && hit.obj->behaviorId == 0x57) {
            hit.obj->interactObj->hitPoints = 0;
            obj_list_unlink(obj);
            s->model = (void *) s->modelId;
            s->flags |= 4;
        } else if (hit.flags & 0x400) {
            func_m122_00FE4(obj, s);
            obj->velocity.x = 0.0f;
            obj->velocity.y = 0.0f;
            obj->velocity.z = 0.0f;
        } else if (hit.flags & 0x2000) {
            s->model = NULL;
            s->flags |= 0x20;
            obj->unk7C[0] &= ~2;
            s->spin[0] = func_80048E2C(-1000, 1000);
            s->spin[1] = func_80048E2C(-1000, 1000);
            s->spin[2] = func_80048E2C(-1000, 1000);
        } else if (hit.kind == 3) {
            s->model = NULL;
            s->flags |= 0x20;
            obj->unk7C[0] &= ~2;
            s->spin[0] = func_80048E2C(-5000, 5000);
            s->spin[1] = func_80048E2C(-5000, 5000);
            s->spin[2] = func_80048E2C(-5000, 5000);
            obj->velocity.x = hit.normal.x * 3.0f;
            obj->velocity.y = hit.normal.y * 3.0f;
            obj->velocity.z = hit.normal.z * 3.0f;
        } else {
            obj->velocity.x = 0.0f;
            obj->velocity.y = 0.0f;
            obj->velocity.z = 0.0f;
            if ((s->holder = hit.obj) != NULL) {
                joints = (M122Joints2 *) s->holder->unk5C;
                func_80049DC0(obj, &m);
                func_80049AE8(s->angle, &m);
                func_80048C7C(&m, &joints->mtxA[joints->index], &s->mtx);
            }
            if (!(s->flags & 1)) {
                func_80003410(0x9A, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            }
            s->flags |= 1;
            s->flags &= ~8;
        }
    }
    if (s->model == NULL) {
        obj->particleEmittersEnabled = 1;
        func_8005F5AC(obj, updateRate);
        if (!(s->flags & 1)) {
            obj->trans.rotation.y_rotation = func_80049C84(obj->velocity.x, obj->velocity.z);
            obj->trans.rotation.x_rotation = -func_80049C84(
                obj->velocity.y, func_80097140(obj->velocity.x * obj->velocity.x + obj->velocity.z * obj->velocity.z));
        }
        return;
    }
    if (!func_m122_00374(s->model, &tx, &ty, &tz)) {
        obj_list_unlink(obj);
        s->flags |= 4;
        s->model = (void *) s->modelId;
        tx = ((Object *) s->model)->trans.x_position;
        ty = ((Object *) s->model)->trans.y_position +
             ((M122Interaction *) ((Object *) s->model)->interactObj)->height * 0.75f;
        tz = ((Object *) s->model)->trans.z_position;
    }
    speed = point_distance(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, tx, ty, tz);
    if (!(s->flags & 8)) {
        nvx = tx - obj->trans.x_position;
        dy = ty - obj->trans.y_position;
        nvz = tz - obj->trans.z_position;
        if (speed != 0.0f) {
            nvx /= speed;
            dy /= speed;
            nvz /= speed;
        }
        nvx *= 5.5f;
        dy *= 5.5f;
        nvz *= 5.5f;
        ax = (nvx - obj->velocity.x) * 0.27f;
        az = (nvz - obj->velocity.z) * 0.27f;
        s->unk14 += (ax - s->unk14) * 0.17f;
        s->unk1C += (az - s->unk1C) * 0.17f;
        yaw0 = func_80049C84(obj->velocity.x, obj->velocity.z);
        nvx = obj->velocity.x + s->unk14;
        nvz = obj->velocity.z + s->unk1C;
        yaw1 = func_80049C84(nvx, nvz);
        speed = func_80097140(nvx * nvx + nvz * nvz);
        roll = func_80049D80(yaw0, yaw1);
        if (roll > 1500) {
            roll = 1500;
        } else if (roll < -1500) {
            roll = -1500;
        }
        yaw1 = yaw0 + roll;
        if (speed < 4.0f) {
            speed = 4.0f;
        }
        obj->velocity.x = func_80049C34(yaw1) * speed;
        obj->velocity.z = func_80049C30(yaw1) * speed;
        if (s->flags & 2) {
            roll = 0;
        } else {
            roll = func_80049D80(yaw1, s->prevYaw);
        }
        obj->trans.rotation.y_rotation = func_80049C84(obj->velocity.x, obj->velocity.z);
        obj->trans.rotation.z_rotation = roll * 10;
        obj->trans.rotation.x_rotation = -func_80049C84(
            obj->velocity.y, func_80097140(obj->velocity.x * obj->velocity.x + obj->velocity.z * obj->velocity.z));
        if (roll < 0) {
            s->unk24 = -roll;
        } else {
            s->unk24 = roll;
        }
        if (s->flags & 2) {
            s->unk18 = 0.0f;
        } else {
            s->unk24 = s->unk28 + (s->unk24 - s->unk28) * 0.5f;
            speed = func_80049D80(s->unk28, s->unk24) * 0.01f;
            s->unk18 = (dy - obj->velocity.y) + speed;
        }
        if (s->unk18 >= 0.0f) {
            s->unk18 = func_80097140(s->unk18);
        } else {
            s->unk18 = -func_80097140(-s->unk18);
        }
        obj->velocity.y += s->unk18;
        s->prevYaw = yaw1;
        s->flags &= ~2;
        s->unk28 = s->unk24;
        if (updateRate >= s->timer) {
            obj_list_unlink(obj);
            s->model = (void *) s->modelId;
            s->flags |= 4;
        } else {
            s->timer -= updateRate;
        }
    }
    obj->particleEmittersEnabled = 1;
    func_8005F5AC(obj, updateRate);
}

void func_m122_00F38(Gfx **dl, Mtx **mtx, Object *obj) {
    M122State *s;
    Object *holder;
    MtxF m;

    s = obj->unk68;
    holder = s->holder;
    if (holder != NULL && holder->behaviorId != 0x37) {
        func_80048C7C(&s->mtx, &((M122Joints *) holder->unk5C)->mtx[((u8 *) holder->unk5C)[0x12B]], &m);
        mtx_model_push(dl, mtx, &m);
    } else {
        func_80049DC0(obj, &m);
        func_80049AE8(s->angle, &m);
        mtx_model_push(dl, mtx, &m);
    }
}

void func_m122_00FE4(Object *obj, M122State *s) {
    s->model = (void *) s->modelId;
    s->flags |= 0x20;
    obj->unk7C[0] &= ~2;
    obj->velocity.x *= -0.5f;
    obj->velocity.y = func_80048E2C(0, 5);
    obj->velocity.z *= -0.5f;
    s->spin[0] = func_80048E2C(-5000, 5000);
    s->spin[1] = func_80048E2C(-5000, 5000);
    s->spin[2] = func_80048E2C(-5000, 5000);
}

INCLUDE_ASM("asm/nonmatchings/modules/m122", data);
