/*
 * Runlink module 102: a thrown bomb. It is either held at a point of its
 * thrower's model until thrown, or launched along its set-up direction from a
 * spot checked against the scenery. It spins and tumbles as it rolls, and
 * goes off when its time runs out: it spawns an object from the level's list
 * where it lies and plays a sound.
 */
#include "module.h"

typedef struct M102Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 size;
    /* 0x0B */ u8 lifetime; /* in seconds / 10 */
    /* 0x0C */ s16 yRotation;
    /* 0x0E */ s16 xRotation;
    /* 0x10 */ f32 speed;
    /* 0x14 */ Object *thrower;
    /* 0x18 */ u8 held;
    /* 0x19 */ u8 attachPoint;
} M102Setup;

typedef struct M102Bomb {
    /* 0x0 */ Object *thrower;
    /* 0x4 */ s32 anim;
    /* 0x8 */ s16 timer;
    /* 0xA */ s16 spin;
    /* 0xC */ u8 size;
    /* 0xD */ u8 held;
    /* 0xE */ u8 attachPoint;
} M102Bomb;

/* What obj->unk7C points at for this object: its collision result */
typedef struct M102Collision {
    /* 0x00 */ u8 flags; /* 1: on the ground, 2: in water? */
    /* 0x01 */ u8 pad1;
    /* 0x02 */ u16 hit;
    /* 0x04 */ Vec3f normal;
    /* 0x10 */ f32 height;
    /* 0x14 */ f32 speed;
} M102Collision;

typedef struct M102Model {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s8 animated;
    /* 0x0B */ u8 padB[0x68 - 0xB];
    /* 0x68 */ Vec3f *points;
} M102Model;

extern void runlink_unload_timer_set(s32, s32, s32);
extern s32 func_80005064(s32);
extern void spawn_object_from_list(s16 kind, s16 x, s16 y, s16 z, s32 arg4, Object *thrower);
extern void func_80052734(Object *obj, s16 x, s16 y, s16 z, s32, s32, s32);
extern void func_800577A8(void *model, s32 *anim, s32 arg2, f32 *arg3, s32 updateRate);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_m003_058A4(Object *);

void func_m102_002B4(Object *obj);

s32 D_m102_00830 = 102; /* the module's number */

void func_m102_00000(Object *obj, M102Setup *setup) {
    M102Bomb *b;
    u8 hit[0x28];
    Vec3f from;
    Vec3f to;
    f32 radius;

    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    b = obj->unk68;
    if (setup->held) {
        obj->velocity.z = 0.0f;
        obj->trans.rotation.y_rotation = 0;
        obj->trans.rotation.x_rotation = 0;
        obj->trans.rotation.z_rotation = 0;
    } else {
        obj->velocity.z = -1.0f;
        obj->trans.rotation.y_rotation = setup->yRotation;
        obj->trans.rotation.x_rotation = setup->xRotation;
        obj->trans.rotation.z_rotation = func_80048E2C(0, 0xFFFF);
        obj->unk28 = func_80048E2C(0, *(u8 *) obj->modelInstances[0] - 1);
        func_8004944C(&obj->trans.rotation, &obj->velocity);
        radius = 32.0f;
        from.x = obj->trans.x_position - obj->velocity.x * radius;
        from.y = obj->trans.y_position - obj->velocity.y * radius;
        from.z = obj->trans.z_position - obj->velocity.z * radius;
        to.x = obj->trans.x_position + obj->velocity.x * radius;
        to.y = obj->trans.y_position + obj->velocity.y * radius;
        to.z = obj->trans.z_position + obj->velocity.z * radius;
        radius = 16.0f;
        func_8001CE10(1, &from, &to, &radius, 0, 0, 0);
        if (func_80016208(NULL, &from, &to, radius, 1, hit) != 0) {
            obj->trans.x_position = to.x;
            obj->trans.y_position = to.y;
            obj->trans.z_position = to.z;
        }
        obj->velocity.x *= setup->speed;
        obj->velocity.y *= setup->speed;
        obj->velocity.z *= setup->speed;
    }
    func_80079D1C(obj, (setup->size + 1) << 8, 1, 2, 0, 0, 0, 16.0f, 16.0f, 1.0f, 0.75f, 0x11);
    b->timer = func_80005064(setup->lifetime * 6);
    b->size = setup->size;
    b->thrower = setup->thrower;
    b->anim = 0xD;
    b->held = setup->held;
    b->attachPoint = setup->attachPoint;
    *obj->unk7C &= ~2;
}

void func_m102_002B4(Object *obj) {
    M102Collision *c = (M102Collision *) obj->unk7C;
    s32 kind = 0;
    Object *thrower;

    if (c != NULL && (c->flags & 1) && c->height - obj->trans.y_position > 10.0f) {
        kind = 4;
    }
    thrower = ((M102Bomb *) obj->unk68)->thrower;
    if (thrower != NULL && thrower->behaviorId != 1) {
        thrower = NULL;
    }
    spawn_object_from_list(kind, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 8, thrower);
    func_80003410(0x22, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
    func_80052734(NULL, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 5, 1, 0x22);
    func_800079B8(obj);
}

void func_m102_00490(Object *obj, s32 updateRate) {
    M102Bomb *b = obj->unk68;
    s32 pad;
    M102Collision *c = (M102Collision *) obj->unk7C;
    M102Model *model;
    f32 scale;

    runlink_unload_timer_set(D_m102_00830, 2, 2);
    b->timer -= updateRate;
    if (b->timer <= 0) {
        func_m102_002B4(obj);
        return;
    }
    if (b->held) {
        model = ((M102Model **) b->thrower->modelInstances)[b->thrower->modelIndex];
        obj->particleEmittersEnabled = 0;
        if (model == NULL) {
            return;
        }
        if (model->animated && b->thrower->behaviorId == 0x18) {
            func_m003_058A4(b->thrower);
        }
        obj->trans.x_position = model->points[b->attachPoint].x;
        obj->trans.y_position = model->points[b->attachPoint].y;
        obj->trans.z_position = model->points[b->attachPoint].z;
        return;
    }
    if (c->flags & 2) {
        func_80052734(obj, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, 2, 0);
    } else {
        scale = c->speed * 0.25f;
        if (scale > 1.0f) {
            scale = 1.0f;
        }
        b->spin += 256.0f * scale * updateRate;
        func_800577A8(obj->modelInstances[0], &b->anim, 10.0f * scale, &obj->unk28, updateRate);
    }
    obj->trans.rotation.z_rotation = b->spin;
    if (b->anim & 2) {
        obj->trans.rotation.z_rotation += 0x8000;
    }
    if (!(c->flags & 1) && 2.0f < c->speed) {
        obj->particleEmittersEnabled = 1;
        func_8005F5AC(obj, updateRate);
        return;
    }
    if ((c->flags & 1) && 10.0f < c->height - obj->trans.y_position) {
        obj->particleEmittersEnabled = 8;
        func_8005F5AC(obj, updateRate);
        return;
    }
    obj->particleEmittersEnabled = 0;
}
