/*
 * Runlink module 96: a thrown flare. It flies under gravity, spinning, and
 * sticks to what it hits (following that object if it moves), bounces off
 * one kind of surface, and is destroyed by another (with a sound). It is
 * freed when its time runs out.
 */
#include "module.h"

typedef struct M096Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 y_rotation;
    /* 0x0C */ s16 x_rotation;
    /* 0x0E */ u8 padE;
    /* 0x0F */ u8 time; /* in tenths of a second */
    /* 0x10 */ f32 speed;
    /* 0x14 */ s32 unk14;
} M096Setup;

/* The flare's data (its object's unk68) */
typedef struct M096Flare {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 light;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ Object *stuck; /* the object it sticks to */
    /* 0x10 */ f32 localX;    /* where, in that object's space */
    /* 0x14 */ f32 localY;
    /* 0x18 */ f32 localZ;
    /* 0x1C */ s16 hit;
    /* 0x1E */ s16 timer;
} M096Flare;

/* What a hit object's unk5C points at (the parts the flare uses) */
typedef struct M096Model {
    /* 0x000 */ f32 mtx[2][4][4];
    /* 0x080 */ f32 invMtx[2][4][4];
    /* 0x100 */ u8 pad100[0x124 - 0x100];
    /* 0x124 */ s16 z_rotation;
    /* 0x126 */ s16 x_rotation;
    /* 0x128 */ s16 y_rotation;
    /* 0x12A */ u8 pad12A;
    /* 0x12B */ u8 mtxIndex;
    /* 0x12C */ s16 unk12C;
} M096Model;

/* func_80018808's result and the surface word after it */
typedef struct M096Hit {
    /* 0x00 */ HitResult h;
    /* 0x28 */ s32 surface;
} M096Hit;

extern void runlink_unload_timer_set(s32, s32, s32);
extern void player_drop_held_object(s32);
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz);
extern f32 func_80097140(f32); /* square root */
extern void func_80048B60(f32 *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz);
extern void track_light_set_position(s32 light, f32 x, f32 y, f32 z);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_m105_01474(Object *obj, f32 *x, f32 *y, f32 *z, f32 *ox, f32 *oy, f32 *oz);

s32 D_m096_00620 = 96; /* the module's number */

void func_m096_00000(Object *obj, M096Setup *setup) {
    M096Flare *f = obj->unk68;

    obj->velocity.z = -setup->speed;
    obj->trans.rotation.y_rotation = setup->y_rotation;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->trans.rotation.z_rotation = 0;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    f->light = 0;
    f->unk8 = 0;
    f->hit = 0;
    f->timer = setup->time * 6;
    f->unk0 = setup->unk14;
}

#ifdef NON_MATCHING
/*
 * Equivalent, and the frame matches; IDO swaps two caller-saved registers
 * (the original keeps f in t0 and updateRate / o in v1, this gives f v1).
 */
void func_m096_00088(Object *obj, s32 updateRate) {
    M096Hit hit;
    Vec3f from;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 len;
    f32 dt;
    M096Flare *f = obj->unk68;
    Object *o;
    M096Model *m;

    runlink_unload_timer_set(D_m096_00620, 2, 2);
    if (f->timer == 0) {
        if (f->unk0 != 0) {
            player_drop_held_object(f->unk0);
        }
        func_800079B8(obj);
        return;
    }
    if (updateRate >= f->timer) {
        dt = f->timer;
        f->timer = 0;
    } else {
        f->timer -= updateRate;
        dt = updateRate;
    }
    if (f->unk0 != 0) {
        return;
    }
    if (f->hit == 0) {
        from.x = obj->trans.x_position;
        from.y = obj->trans.y_position;
        from.z = obj->trans.z_position;
        obj->trans.rotation.x_rotation += updateRate << 10;
        obj->trans.rotation.y_rotation += updateRate << 11;
        dx = obj->velocity.x * dt;
        dy = obj->velocity.y * dt + -0.05f * dt * dt;
        dz = obj->velocity.z * dt;
        func_80009A04(obj, dx, dy, dz);
        obj->velocity.y += -0.1f * dt;
        if ((f->hit = func_80018808(obj, &from, (Vec3f *) &obj->trans.x_position, &hit.h, 0, 0)) != 0) {
            len = obj->velocity.x * obj->velocity.x + obj->velocity.y * obj->velocity.y +
                  obj->velocity.z * obj->velocity.z;
            if (len > 0.0f) {
                len = 1.0f / func_80097140(len);
            }
            dx = hit.h.pos.x - obj->velocity.x * len - obj->trans.x_position;
            dy = hit.h.pos.y - obj->velocity.y * len - obj->trans.y_position;
            dz = hit.h.pos.z - obj->velocity.z * len - obj->trans.z_position;
            func_80009A04(obj, dx, dy, dz);
            o = hit.h.obj;
            f->stuck = o;
            if (o != NULL) {
                if (o->behaviorId == 7) {
                    f->stuck = NULL;
                    f->timer = 10;
                } else {
                    m = (M096Model *) o->unk5C;
                    func_80048B60(m->mtx[m->mtxIndex][0], obj->trans.x_position, obj->trans.y_position,
                                  obj->trans.z_position, &f->localX, &f->localY, &f->localZ);
                }
            } else if (hit.h.flags & 0x2000) {
                obj->trans.rotation.x_rotation = 0;
                obj->trans.rotation.z_rotation = 0;
                obj->particleEmittersEnabled = 6;
                func_8005F5AC(obj, updateRate);
                func_800079B8(obj);
                func_80003410(0x214, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                return;
            } else if (hit.surface == 3) {
                obj->velocity.x = hit.h.normal.x * 8.0f;
                obj->velocity.y = hit.h.normal.y;
                obj->velocity.z = hit.h.normal.z * 8.0f;
                f->hit = 0;
            }
        }
        if (f->light != 0) {
            track_light_set_position(f->light, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        }
        obj->particleEmittersEnabled = 1;
        func_8005F5AC(obj, updateRate);
    } else {
        o = f->stuck;
        if (o != NULL) {
            m = (M096Model *) o->unk5C;
            if (o->behaviorId == 0x37) {
                func_m105_01474(o, &f->localX, &f->localY, &f->localZ, &obj->trans.x_position,
                                &obj->trans.y_position, &obj->trans.z_position);
            } else if (o->behaviorId == 0x4E) {
                if (*OBJ_STATE(o, s32) <= 0) {
                    f->stuck = NULL;
                    f->hit = 0;
                    obj->velocity.x = 0.0f;
                    obj->velocity.y = 0.0f;
                    obj->velocity.z = 0.0f;
                }
            } else if (m->unk12C != 0) {
                func_80048B60(m->invMtx[m->mtxIndex][0], f->localX, f->localY, f->localZ, &dx, &dy, &dz);
                obj->trans.rotation.y_rotation += o->trans.rotation.y_rotation - m->y_rotation;
                obj->trans.rotation.x_rotation += o->trans.rotation.x_rotation - m->x_rotation;
                obj->trans.rotation.z_rotation += o->trans.rotation.z_rotation - m->z_rotation;
                func_80009A04(obj, dx - obj->trans.x_position, dy - obj->trans.y_position,
                              dz - obj->trans.z_position);
            }
            if (f->light != 0) {
                track_light_set_position(f->light, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m096", func_m096_00088);
#endif

/* Sets the flare off with the given direction and speed */
void func_m096_005D8(Object *obj, s16 y_rotation, s16 x_rotation, f32 speed) {
    M096Flare *f = obj->unk68;

    f->unk0 = 0;
    obj->trans.rotation.x_rotation = x_rotation;
    obj->velocity.z = -speed;
    obj->trans.rotation.y_rotation = y_rotation;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m096", rodata);
#endif
