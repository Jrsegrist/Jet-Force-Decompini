/*
 * Runlink module 153: a blob thrown by an enemy: it flies and spins, sticks
 * to its thrower's model while held, leaves blood where it lands, and fades
 * out when its time is up.
 */
#include "module.h"

typedef struct M153Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 size;
    /* 0x0B */ u8 lifetime; /* in seconds / 10 */
    /* 0x0C */ s16 y_rotation;
    /* 0x0E */ s16 x_rotation;
    /* 0x10 */ u8 pad10[0x14 - 0x10];
    /* 0x14 */ Object *thrower;
    /* 0x18 */ u8 attachPoint;
} M153Setup;

typedef struct M153Blob {
    /* 0x0 */ Object *thrower;
    /* 0x4 */ u8 pad4[0x8 - 0x4];
    /* 0x8 */ s16 timer;
    /* 0xA */ u8 size;
    /* 0xB */ s8 spin;
    /* 0xC */ u8 held;
    /* 0xD */ u8 attachPoint;
} M153Blob;

/* What obj->unk7C points at for this object: its collision result */
typedef struct M153Collision {
    /* 0x00 */ u8 flags; /* 1: on the ground */
    /* 0x01 */ u8 pad1;
    /* 0x02 */ u16 hit;
    /* 0x04 */ Vec3f normal;
    /* 0x10 */ f32 height;
    /* 0x14 */ f32 speed;
} M153Collision;

typedef struct M153Model {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s8 animated;
    /* 0x0B */ u8 padB[0x68 - 0xB];
    /* 0x68 */ Vec3f *points;
} M153Model;

extern void runlink_unload_timer_set(s32, s32, s32);
extern s32 func_80005064(s32);
extern s32 func_80049C84(f32, f32); /* the angle of a vector */
extern f32 sqrtf(f32);
extern void func_m003_058A4(Object *);
extern void func_m008_001CC(f32 x, f32 y, f32 z, f32 nx, f32 ny, f32 nz, s32 alpha, s32 arg7, f32 arg8);

s32 D_m153_00450 = 153; /* the module's number */

void func_m153_00000(Object *obj, M153Setup *setup) {
    M153Blob *b = obj->unk68;

    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    obj->velocity.z = 0.0f;
    obj->trans.rotation.y_rotation = setup->y_rotation;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->trans.rotation.z_rotation = func_80048E2C(0, 0xFFFF);
    obj->unk28 = 0.0f;
    func_80079D1C(obj, setup->size << 8, 1, 2, 0, 0, 0, 16.0f, 16.0f, 1.0f, 0.75f, 0x11);
    b->timer = func_80005064(setup->lifetime * 6);
    b->size = setup->size;
    b->thrower = setup->thrower;
    b->spin = func_80048E2C(-128, 127);
    b->attachPoint = setup->attachPoint;
    b->held = 1;
    *obj->unk7C &= ~2;
}

void func_m153_0012C(Object *obj, s32 updateRate) {
    M153Blob *b = obj->unk68;
    M153Collision *c = (M153Collision *) obj->unk7C;
    M153Model *model;

    runlink_unload_timer_set(D_m153_00450, 2, 2);
    if (b->timer > 0 && !(c->flags & 2)) {
        b->timer -= updateRate;
    } else {
        if (updateRate * 4 < obj->opacity) {
            obj->opacity -= updateRate * 4;
        } else {
            func_800079B8(obj);
        }
        return;
    }
    obj->trans.rotation.y_rotation = func_80049C84(obj->velocity.x, obj->velocity.z) - 0x8000;
    obj->trans.rotation.x_rotation =
        func_80049C84(obj->velocity.y, sqrtf(obj->velocity.x * obj->velocity.x + obj->velocity.z * obj->velocity.z));
    obj->trans.rotation.z_rotation += b->spin * 14;
    if (b->held) {
        model = ((M153Model **) b->thrower->modelInstances)[b->thrower->modelIndex];
        obj->particleEmittersEnabled = 0;
        if (model == NULL) {
            return;
        }
        if (model->animated) {
            func_m003_058A4(b->thrower);
        }
        obj->trans.x_position = model->points[b->attachPoint].x;
        obj->trans.y_position = model->points[b->attachPoint].y;
        obj->trans.z_position = model->points[b->attachPoint].z;
        return;
    }
    if (c->hit != 0 && !(c->flags & 1)) {
        func_m008_001CC(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, -c->normal.x, -c->normal.y,
                        -c->normal.z, obj->shading->unk0 * 255.0f, 1, 1.0f);
    }
    if (!(c->flags & 1) && 2.0f < c->speed) {
        obj->particleEmittersEnabled = 1;
        obj_spawn_particle(obj, updateRate);
        return;
    }
    if ((c->flags & 1) && 10.0f < c->height - obj->trans.y_position) {
        obj->particleEmittersEnabled = 8;
        obj_spawn_particle(obj, updateRate);
        return;
    }
    obj->particleEmittersEnabled = 0;
}
