/*
 * Runlink module 83: two behaviours of objects launched in a direction (a
 * big one and a small one). Each init turns the object's velocity to the
 * level entry's angles and scales it; the big one also starts from where a
 * sweep along that direction meets the level. The controls (still assembly)
 * move them.
 */
#include "module.h"

/* What the inits get */
typedef struct M083Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 yaw;
    /* 0x0E */ s16 pitch;
    /* 0x10 */ f32 speed;
    /* 0x14 */ s32 unk14;
} M083Setup;

/* The big one's state at 0x88 */
typedef struct M083State {
    /* 0x00 */ s16 flags;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 unk4;
} M083State;

/* The small one's state at 0x88 */
typedef struct M083SmallState {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 unk4;
} M083SmallState;

void func_m083_00000(Object *obj, M083Setup *setup) {
    M083State *state = OBJ_STATE(obj, M083State);
    HitResult hit;
    Vec3f from;
    Vec3f to;
    f32 radius;

    obj->velocity.z = -1.0f;
    obj->trans.rotation.y_rotation = setup->yaw;
    obj->trans.rotation.x_rotation = setup->pitch;
    obj->trans.rotation.z_rotation = 0;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    radius = 16.0f;
    from.x = obj->trans.x_position - obj->velocity.x * radius;
    from.y = obj->trans.y_position - obj->velocity.y * radius;
    from.z = obj->trans.z_position - obj->velocity.z * radius;
    to.x = obj->trans.x_position + obj->velocity.x * radius;
    to.y = obj->trans.y_position + obj->velocity.y * radius;
    to.z = obj->trans.z_position + obj->velocity.z * radius;
    radius = 8.0f;
    func_8001CE10(1, &from, &to, &radius, 0, 0, 0);
    if (func_80016208(NULL, &from, &to, radius, 1, &hit)) {
        obj->trans.x_position = to.x;
        obj->trans.y_position = to.y;
        obj->trans.z_position = to.z;
    }
    obj->velocity.x *= setup->speed;
    obj->velocity.y *= setup->speed;
    obj->velocity.z *= setup->speed;
    state->flags = 0;
    state->unk2 = setup->unkA;
    state->unk4 = setup->unk14;
    func_80079D1C(obj, 0x1800, 1, 4, 0, 0, 0, 8.0f, 8.0f, -1.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m083", func_m083_001EC);

void func_m083_009C8(Object *obj, M083Setup *setup) {
    M083SmallState *state = OBJ_STATE(obj, M083SmallState);

    obj->velocity.z = -setup->speed;
    obj->trans.rotation.y_rotation = setup->yaw;
    obj->trans.rotation.x_rotation = setup->pitch;
    obj->trans.rotation.z_rotation = 0;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->trans.rotation.x_rotation = 0;
    state->unk0 = 0;
    state->unk1 = 0;
    state->unk2 = setup->unkA;
    state->unk4 = setup->unk14;
    func_80079D1C(obj, 0x800, 1, 4, 0, 0, 0, 8.0f, 8.0f, -1.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m083", func_m083_00A88);

INCLUDE_ASM("asm/nonmatchings/modules/m083", data);

INCLUDE_ASM("asm/nonmatchings/modules/m083", rodata);
