/*
 * Runlink module 86: a tumbling shot with its own light that flies straight
 * until it hits something, runs out of time or is shot down. Where it hits a
 * suitable surface it leaves another object behind; otherwise it bursts.
 * (Module 130 is the same shot pointing along its flight.)
 */
#include "module.h"

typedef struct M086Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 strength;
    /* 0x0B */ u8 lifetime; /* in seconds / 10 */
    /* 0x0C */ s16 vx;
    /* 0x0E */ s16 vy;
    /* 0x10 */ s16 vz;
    /* 0x12 */ u8 pad12[0x14 - 0x12];
    /* 0x14 */ s32 unk14;
} M086Setup;

typedef struct M086Shot {
    /* 0x00 */ f32 x; /* where it hit */
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ s32 hitFlags;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s16 timer;
    /* 0x16 */ u8 hit; /* 0: flying, 1: hit something, 2: hit a floor */
} M086Shot;

typedef struct M086State {
    /* 0x0 */ s32 health; /* at the start */
    /* 0x4 */ s32 light;
} M086State;

typedef struct M086Interaction {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ Vec3f oldPos;
} M086Interaction;

/* The level entry of the object it leaves behind */
typedef struct M086Spawn {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ f32 unkC;
} M086Spawn;

extern void func_800541B0(s32, s32, s32);
extern s32 func_80005064(s32);
extern s32 func_80049C84(f32, f32); /* the angle of a vector */
extern f32 sqrtf(f32);
extern f32 *func_80013E0C(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, s32 arg5, s32 arg6, s32 arg7); /* adds a light */
extern void func_80013F08(f32 *light);
extern void func_80013FE0(f32 *light, f32 x, f32 y, f32 z);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern Object *spawn_object(M086Spawn *entry, s32 arg1); /* DKR */
extern void func_800285B0(s32, s16, s16, s16, s32, s32);
extern void func_80052734(s32, s16, s16, s16, u8, u8, u8);
extern void func_8005F5AC(Object *obj, s32 updateRate);

s32 D_m086_005C0 = 86; /* the module's number */

void func_m086_00000(Object *obj, M086Setup *setup) {
    M086Shot *s = obj->unk68;

    obj->velocity.x = setup->vx;
    obj->velocity.y = setup->vy;
    obj->velocity.z = setup->vz;
    obj->trans.rotation.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
    obj->trans.rotation.x_rotation = func_80048E2C(-0x8000, 0x7FFF);
    obj->trans.rotation.z_rotation = 0;
    s->hit = 0;
    s->hitFlags = 0;
    s->x = obj->trans.x_position;
    s->y = obj->trans.y_position;
    s->z = obj->trans.z_position;
    s->timer = func_80005064(setup->lifetime * 6);
    s->unk10 = setup->unk14;
    func_80079D1C(obj, (setup->strength + 1) << 8, 1, 4, 0, 0, 0, 2.0f, 2.0f, -1.0f, 1.0f, 0x11);
    OBJ_STATE(obj, M086State)->health = obj->interactObj->unk6;
    OBJ_STATE(obj, M086State)->light = (s32) func_80013E0C(obj->trans.x_position, obj->trans.y_position,
                                                           obj->trans.z_position, 500.0f, 300.0f, 0xFF, 0xE0, 0);
}

void func_m086_00188(Object *obj, s32 updateRate) {
    M086Shot *s;
    s32 pad1;
    HitResult hit;
    Object *spawned;
    s32 pad2;
    M086Spawn entry;

    s = obj->unk68;
    func_800541B0(D_m086_005C0, 2, 2);
    s->timer -= updateRate;
    if (s->timer > 0 && obj->interactObj->unk6 > 0 && s->hit == 0 &&
        obj->interactObj->unk6 == OBJ_STATE(obj, M086State)->health) {
        if (s->hit == 0) {
            func_80009A04(obj, obj->velocity.x, obj->velocity.y, obj->velocity.z);
            if ((s->hit = func_80018808(obj, &((M086Interaction *) obj->interactObj)->oldPos,
                                        (Vec3f *) &obj->trans.x_position, &hit, 0, 0)) != 0) {
                s->x = hit.pos.x;
                s->y = hit.pos.y;
                s->z = hit.pos.z;
                s->hitFlags = hit.flags;
                func_80009A04(obj, s->x - obj->trans.x_position, s->y - obj->trans.y_position,
                              s->z - obj->trans.z_position);
                if (s->hit == 1 &&
                    (s16) func_80049C84(hit.normal.y, sqrtf(hit.normal.z * hit.normal.z + hit.normal.x * hit.normal.x)) <
                        0x2000) {
                    s->hit = 2;
                }
            }
            if (OBJ_STATE(obj, M086State)->light != 0) {
                func_80013FE0((f32 *) OBJ_STATE(obj, M086State)->light, obj->trans.x_position,
                              obj->trans.y_position, obj->trans.z_position);
            }
        }
    } else {
        if ((s->hitFlags & 0x2000) && !(s->hitFlags & 0x8000)) {
            func_80003410(0xD, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            entry.objectID = 0xC0;
            entry.size = 0x10;
            entry.x = s->x;
            entry.y = s->y;
            entry.z = s->z;
            entry.unkA = 0x100;
            entry.unkC = 20.0f;
            spawned = spawn_object(&entry, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
        } else {
            func_80003410(0x96, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            func_800285B0(2, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 100, 0);
            func_80052734(0, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 5, 1, 0x96);
        }
        if (OBJ_STATE(obj, M086State)->light != 0) {
            func_80013F08((f32 *) OBJ_STATE(obj, M086State)->light);
        }
        func_800079B8(obj);
        s = NULL;
    }
    if (s != NULL) {
        obj->particleEmittersEnabled = 3;
        func_8005F5AC(obj, updateRate);
    }
}
