/* Runlink module 92: fish food, a pellet that drifts along a line and shrinks away. */
#include "module.h"

typedef struct M092Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ Vec3s rotation; /* z_rotation is also the lifetime */
    /* 0x10 */ f32 speed;
} M092Setup;

typedef struct M092Food {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
    /* 0xC */ s16 timer;
    /* 0xE */ u8 unkE;
    /* 0xF */ u8 unkF;
} M092Food;

typedef struct M092State {
    /* 0x0 */ s32 timer;
    /* 0x4 */ s32 unk4;
} M092State;

extern void func_800541B0(s32, s32, s32);
extern void func_8001CE10(s32, Vec3f *, Vec3f *, f32 *, s32, s32, s32);
extern s32 func_80016208(s32, Vec3f *, Vec3f *, f32, s32, void *);

s32 D_m092_00330 = 92; /* the module's number */

void func_m092_00000(Object *obj, M092Setup *setup) {
    M092Food *food = obj->unk68;
    u8 hit[0x28];
    Vec3f from;
    Vec3f to;
    f32 radius;

    food->timer = setup->rotation.z_rotation << 8;
    obj->trans.scale = setup->rotation.z_rotation * 0.002f;
    obj->velocity.z = -1.0f;
    func_8004944C(&setup->rotation, &obj->velocity);
    radius = 20.0f;
    from.x = obj->trans.x_position - obj->velocity.x * radius;
    from.y = obj->trans.y_position - obj->velocity.y * radius;
    from.z = obj->trans.z_position - obj->velocity.z * radius;
    to.x = obj->trans.x_position + obj->velocity.x * radius;
    to.y = obj->trans.y_position + obj->velocity.y * radius;
    to.z = obj->trans.z_position + obj->velocity.z * radius;
    radius = 10.0f;
    func_8001CE10(1, &from, &to, &radius, 0, 0, 0);
    if (func_80016208(0, &from, &to, radius, 1, hit) != 0) {
        obj->trans.x_position = to.x;
        obj->trans.y_position = to.y;
        obj->trans.z_position = to.z;
    }
    obj->velocity.x *= setup->speed;
    obj->velocity.y *= setup->speed;
    obj->velocity.z *= setup->speed;
    food->x = obj->trans.x_position;
    food->y = obj->trans.y_position;
    food->z = obj->trans.z_position;
    food->unkE = 0;
    food->unkF = 0;
    OBJ_STATE(obj, M092State)->timer = food->timer;
    OBJ_STATE(obj, M092State)->unk4 = 0;
    *obj->unk7C &= ~2;
}

void func_m092_001F0(Object *obj, s32 updateRate) {
    M092Food *food;
    u8 *flags;

    func_800541B0(D_m092_00330, 2, 2);
    food = obj->unk68;
    flags = obj->unk7C;
    food->timer = OBJ_STATE(obj, M092State)->timer;
    food->timer -= updateRate;
    if (food->timer <= 0) {
        OBJ_STATE(obj, M092State)->timer = 0;
        obj->trans.scale = 0.0f;
        func_800079B8(obj);
        return;
    }
    obj->trans.scale = food->timer * 0.002f;
    OBJ_STATE(obj, M092State)->timer = food->timer;
    if (*flags & 2) {
        OBJ_STATE(obj, M092State)->unk4 = 1;
    } else {
        OBJ_STATE(obj, M092State)->unk4 = 0;
    }
    if (*flags & 1) {
        obj->trans.flags &= ~4;
        obj->trans.flags |= 8;
        food->unkE = 1;
    } else {
        obj->trans.flags &= ~8;
        obj->trans.flags |= 4;
        food->unkE = 0;
    }
}
