/*
 * Runlink module 83: two behaviours of objects launched in a direction (a
 * big one and a small one). Each init turns the object's velocity to the
 * level entry's angles and scales it; the big one also starts from where a
 * sweep along that direction meets the level. The controls fly them under
 * gravity (damped once they are in water, where entering spawns an effect at the surface) until
 * they hit something, then spawn an impact effect and pieces, play a sound
 * and free the object.
 */
#include "module.h"

/* What the inits get (also the entry of the big one's pieces) */
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
    /* 0x00 */ s16 flags; /* 1: in water */
    /* 0x02 */ s16 unk2;  /* a timer */
    /* 0x04 */ s32 unk4;  /* an object */
} M083State;

/* The small one's state at 0x88 */
typedef struct M083SmallState {
    /* 0x00 */ u8 unk0; /* 1: in water, 2: fell below the level */
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 unk4;
} M083SmallState;

/* The effect the big one spawns at the water surface when it enters water */
typedef struct M083SurfaceEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ u8 pad10[0x18 - 0x10];
} M083SurfaceEntry;

/* The entry of the small one's pieces */
typedef struct M083Piece {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 speed;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ u8 unk1C;
    /* 0x20 */ Object *unk20;
} M083Piece;

extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern s32 track_height_at(f32 x, f32 z, f32 *height, s32 mask); /* the water surface at x, z */
extern void spawn_object_from_list(s32 index, s16 x, s16 y, s16 z, s32 arg4, Object *obj);
extern s32 func_8004966C(s16 angle);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_80049C84(f32 x, f32 z); /* arctangent as a 16-bit angle */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern f32 func_80097140(f32 x); /* square root */
extern u8 D_800A4FC4;

s32 D_m083_01200 = 83; /* the module's number */

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

void func_m083_001EC(Object *obj, s32 updateRate) {
    M083State *state;
    Object *spawned;
    f32 dx;
    f32 dz;
    s32 hitFlag;
    s32 i;
    f32 dy;
    s32 rate;
    f32 waterY;
    f32 radius;
    Vec3f from;
    Vec3f *hitPos;
    f32 rateF;
    f32 damp;
    HitResult hit;
    M083SurfaceEntry surface;
    Object *target;
    M083Setup piece;
    UNUSED s32 pad1;
    UNUSED s32 pad2;

    runlink_unload_timer_set(D_m083_01200, 2, 2);
    hitFlag = 0;
    obj->particleEmittersEnabled = 0;
    radius = 8.0f;
    from.x = obj->trans.x_position;
    from.y = obj->trans.y_position;
    from.z = obj->trans.z_position;
    rate = updateRate;
    state = OBJ_STATE(obj, M083State);
    if (state->unk2 < rate) {
        rate = state->unk2;
    }
    rateF = rate;
    if (state->flags & 1) {
        damp = func_80049BEC(0.875f, rate);
        obj->velocity.x *= damp;
        obj->velocity.y = func_80049BEC(0.7f, rate) * (-0.4f - obj->velocity.y) + obj->velocity.y;
        obj->velocity.z *= damp;
        dy = obj->velocity.y * rateF;
    } else {
        dy = obj->velocity.y * rateF + -0.075f * rateF * rateF;
        obj->velocity.y += -0.15f * rateF;
    }
    dx = obj->velocity.x * rateF;
    dz = obj->velocity.z * rateF;
    func_80009A04(obj, dx, dy, dz);
    if ((track_height_at(obj->trans.x_position, obj->trans.z_position, &waterY, 0x2000) & 0x2000) &&
        obj->trans.y_position < waterY) {
        if (!(state->flags & 1)) {
            surface.common.objectID = 0xC0;
            surface.common.x = obj->trans.x_position;
            surface.common.y = waterY;
            surface.common.z = obj->trans.z_position;
            surface.unkA = 0x100;
            surface.unkC = 7.5f;
            spawned = func_80005F14(&surface, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
            rate = obj->trans.rotation.x_rotation;
            obj->trans.rotation.x_rotation = 0;
            dy = obj->trans.y_position;
            obj->trans.y_position = waterY;
            obj->particleEmittersEnabled |= 1;
            func_8005F5AC(obj, 2);
            obj->trans.y_position = dy;
            obj->trans.rotation.x_rotation = rate;
            func_80003410(0xD, obj->trans.x_position, dy, obj->trans.z_position, 4, 0);
            state->flags |= 1;
        } else if (waterY - obj->trans.y_position > 16.0f) {
            obj->particleEmittersEnabled |= 2;
            func_8005F5AC(obj, 2);
        }
    }
    func_8001CE10(1, &from, (Vec3f *) &obj->trans.x_position, &radius, 0xCE002000, 0, 0);
    if (func_80016208(obj, &from, (Vec3f *) &obj->trans.x_position, radius, 1, &hit)) {
        hitFlag = 2;
        hitPos = (Vec3f *) &obj->trans.x_position;
    } else if (obj->interactObj->hitPoints != 0x1800) {
        hitPos = &from;
        hitFlag = 1;
    } else {
        state->unk2 -= updateRate;
        if (state->unk2 <= 0) {
            hitPos = (Vec3f *) &obj->trans.x_position;
            hitFlag = 1;
        }
    }
    if (hitFlag) {
        target = (Object *) state->unk4;
        if (target != NULL && target->behaviorId != 1) {
            target = NULL;
        }
        if (state->flags & 1) {
            spawn_object_from_list(6, hitPos->x, hitPos->y, hitPos->z, 0xE, target);
        } else {
            spawn_object_from_list(7, hitPos->x, hitPos->y, hitPos->z, 0xE, target);
            piece.common.objectID = 0x136;
            piece.common.x = hitPos->x;
            piece.common.y = hitPos->y;
            piece.common.z = hitPos->z;
            piece.unkA = 300;
            piece.unk14 = state->unk4;
            for (i = 0; i < 3; i++) {
                if (hitFlag == 2) {
                    if (hit.pos.y >= 0.866f) {
                        piece.yaw = i * 0x5555;
                        piece.pitch = func_80048E2C(0x3400, 0x3800);
                        piece.speed = func_80048E2C(65, 75) * 0.1f;
                    } else if (hit.pos.y <= -0.866f) {
                        piece.yaw = i * 0x5555;
                        piece.pitch = func_80048E2C(-0x3800, -0x3400);
                        piece.speed = func_80048E2C(20, 30) * 0.1f;
                    } else {
                        rate = (s16) (func_80049C84(hit.pos.x, hit.pos.z) + 0x8000);
                        piece.yaw = func_80048E2C(-0x2000, 0x2000) + rate;
                        rate = (s16) func_80049C84(hit.pos.y,
                                                   func_80097140(hit.pos.z * hit.pos.z + hit.pos.x * hit.pos.x));
                        piece.pitch = func_80048E2C(-0x2000, 0x2000) + rate;
                        piece.speed = func_80048E2C(30, 50) * 0.1f;
                    }
                } else {
                    piece.yaw = i * 0x5555;
                    piece.pitch = func_80048E2C(0x3400, 0x3800);
                    piece.speed = func_80048E2C(65, 75) * 0.1f;
                }
                spawned = func_80005F14(&piece, 1);
                if (spawned != NULL) {
                    spawned->level_entry = NULL;
                }
            }
        }
        func_80003410(0x96, hitPos->x, hitPos->y, hitPos->z, 4, 0);
        func_800079B8(obj);
    }
}

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

void func_m083_00A88(Object *obj, s32 updateRate) {
    Object *spawned;
    f32 dx;
    f32 dz;
    M083SmallState *state;
    s32 hitFlag;
    s32 i;
    f32 dy;
    s32 rate;
    f32 waterY;
    f32 radius;
    Vec3f from;
    Vec3f *hitPos;
    f32 rateF;
    f32 damp;
    HitResult hit;
    Object *target;
    UNUSED s32 pad1;
    M083Piece piece;
    s32 rotX;

    runlink_unload_timer_set(D_m083_01200, 200, 10);
    hitFlag = 0;
    obj->particleEmittersEnabled = 0;
    radius = 8.0f;
    obj->trans.rotation.y_rotation += updateRate << 8;
    from.x = obj->trans.x_position;
    state = OBJ_STATE(obj, M083SmallState);
    from.y = obj->trans.y_position;
    from.z = obj->trans.z_position;
    rate = updateRate;
    if (state->unk2 < rate) {
        rate = state->unk2;
    }
    rateF = rate;
    if (state->unk0 & 1) {
        damp = func_80049BEC(0.875f, rate);
        obj->velocity.x *= damp;
        obj->velocity.y = func_80049BEC(0.7f, rate) * (-0.4f - obj->velocity.y) + obj->velocity.y;
        obj->velocity.z *= damp;
        dy = obj->velocity.y * rateF;
    } else if (state->unk0 & 2) {
        damp = func_80049BEC(0.95f, rate);
        obj->velocity.x *= damp;
        obj->velocity.y = func_80049BEC(0.8f, rate) * (-0.75f - obj->velocity.y) + obj->velocity.y;
        obj->velocity.z *= damp;
        dy = obj->velocity.y * rateF;
    } else {
        dy = obj->velocity.y * rateF + -0.075f * rateF * rateF;
        obj->velocity.y += -0.15f * rateF;
    }
    dx = obj->velocity.x * rateF;
    dz = obj->velocity.z * rateF;
    func_80009A04(obj, dx, dy, dz);
    if (obj->velocity.y < -3.0f) {
        state->unk0 |= 2;
    }
    if ((track_height_at(obj->trans.x_position, obj->trans.z_position, &waterY, 0x2000) & 0x2000) &&
        obj->trans.y_position < waterY) {
        if (!(state->unk0 & 1)) {
            rotX = obj->trans.rotation.x_rotation;
            dy = obj->trans.y_position;
            obj->trans.y_position = waterY;
            obj->trans.rotation.x_rotation = 0;
            obj->particleEmittersEnabled |= 5;
            func_8005F5AC(obj, 2);
            obj->trans.y_position = dy;
            obj->trans.rotation.x_rotation = rotX;
            func_80003410(0x9E, obj->trans.x_position, dy, obj->trans.z_position, 4, 0);
            state->unk0 |= 1;
        } else if (waterY - obj->trans.y_position > 16.0f) {
            obj->particleEmittersEnabled |= 2;
            func_8005F5AC(obj, 2);
        }
    }
    if (state->unk0 & 1) {
        func_8001138C(obj, -0.0167f, rate);
    } else if (state->unk0 & 2) {
        func_8001138C(obj, 0.0334f, rate);
        state->unk1 += updateRate * 2;
        obj->trans.rotation.z_rotation = func_8004966C(state->unk1 << 8) >> 4;
    }
    func_8001CE10(1, &from, (Vec3f *) &obj->trans.x_position, &radius, 0xCE002000, 0, 0);
    if (func_80016208(obj, &from, (Vec3f *) &obj->trans.x_position, radius, 1, &hit)) {
        hitFlag = 1;
        hitPos = (Vec3f *) &obj->trans.x_position;
    } else if (obj->interactObj->hitPoints != 0x800) {
        hitPos = &from;
        hitFlag = 1;
    } else {
        state->unk2 -= updateRate;
        if (state->unk2 <= 0) {
            hitPos = (Vec3f *) &obj->trans.x_position;
            hitFlag = 1;
        }
    }
    if (hitFlag) {
        target = (Object *) state->unk4;
        if (target != NULL && target->behaviorId != 1) {
            target = NULL;
        }
        if (state->unk0 & 1) {
            spawn_object_from_list(6, hitPos->x, hitPos->y, hitPos->z, 0xE, target);
        } else {
            spawn_object_from_list(8, hitPos->x, hitPos->y, hitPos->z, 0xE, target);
            if (!D_800A4FC4) {
                piece.common.objectID = 0x139;
                piece.common.x = hitPos->x;
                piece.common.y = hitPos->y;
                piece.common.z = hitPos->z;
                piece.unkE = 120;
                piece.unk1C = 0xE;
                piece.unk20 = target;
                piece.unk18 = 0.15f;
                for (i = 0; i < 4; i++) {
                    piece.yaw = obj->trans.rotation.y_rotation + (i << 14);
                    piece.pitch = func_80048E2C(0x1000, 0x2000);
                    piece.speed = func_80048E2C(50, 80) * 0.1f;
                    piece.unk10 = func_80048E2C(15, 25) * 0.01f;
                    spawned = func_80005F14(&piece, 1);
                    if (spawned != NULL) {
                        spawned->level_entry = NULL;
                    }
                }
            }
        }
        func_80003410(0x96, hitPos->x, hitPos->y, hitPos->z, 4, 0);
        func_800079B8(obj);
    }
}
