/*
 * Runlink module 99: a frog. It hops about at random within a range of its
 * home, hops away from a player who comes near, and when a player touches
 * it either is collected (freed) or squashes flat and recovers. State 5
 * makes it burst (emitting particles) and fade out.
 */
#include "module.h"

typedef struct M099Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 range;
    /* 0x0C */ u8 collect; /* nonzero: freed when touched, and hops faster */
} M099Setup;

/* The frog's data (its object's unk68) */
typedef struct M099Frog {
    /* 0x00 */ f32 homeX;
    /* 0x04 */ f32 homeY;
    /* 0x08 */ f32 homeZ;
    /* 0x0C */ f32 range;
    /* 0x10 */ f32 rangeSq;
    /* 0x14 */ u8 state;
    /* 0x15 */ u8 collect;
    /* 0x16 */ s16 timer;
    /* 0x18 */ s8 hop;      /* counts down through a hop */
    /* 0x19 */ s8 cooldown; /* before it can be touched again */
    /* 0x1A */ s16 angle;
    /* 0x1C */ f32 speed;
    /* 0x20 */ f32 startX;
    /* 0x24 */ f32 startZ;
    /* 0x28 */ f32 vx;
    /* 0x2C */ f32 vz;
    /* 0x30 */ f32 squash;
    /* 0x34 */ f32 burstA;
    /* 0x38 */ f32 burstB;
    /* 0x3C */ s32 fade;
} M099Frog;

extern s32 osTvType;

extern s32 func_8007E150(f32, f32, f32, f32, s32, Object **);
extern void func_800116BC(Object *obj, f32 arg1);
extern s16 func_80049C84(f32 x, f32 z); /* the angle of (x, z) */
extern f32 func_80049C34(s32 angle);   /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle);   /* cosine */
extern void func_800099F4();
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz);
extern s32 func_8001B078(f32 x, f32 z, s32 arg2, f32 ***floor);
extern void func_8005F5AC(Object *obj, s32 updateRate);

void func_m099_00000(Object *obj, M099Setup *setup) {
    M099Frog *f = obj->unk68;

    f->collect = setup->collect;
    f->homeX = obj->trans.x_position;
    f->homeY = obj->trans.y_position;
    f->homeZ = obj->trans.z_position;
    f->range = setup->range;
    f->state = 0;
    f->rangeSq = f->range * f->range;
    f->startX = obj->trans.x_position;
    f->startZ = obj->trans.z_position;
    f->squash = 1.0f;
    f->burstA = 1.0f;
    f->burstB = 1.0f;
    f->cooldown = 0;
    f->fade = 0x100;
    obj->modelIndex = 0;
}

#ifdef NON_MATCHING
/*
 * Equivalent (and its .rodata, the PAL factor, jump table and constants,
 * matches). The original keeps rate, dx and dz in the callee-saved f20/f22
 * (frame 0x90, one more f32 local, 0.0f held in f20 across case 1's calls);
 * this spills them instead.
 */
void func_m099_0008C(Object *obj, s32 updateRate) {
    M099Frog *f;
    s32 i;
    s32 hop;
    s32 ok;
    f32 rate;
    f32 dt;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 s;
    Object *list[8];
    f32 **floor;

    rate = updateRate;
    dt = rate;
    if (osTvType == 0) {
        dt = rate * 1.2f;
    }
    f = obj->unk68;
    switch (f->state) {
        case 0:
            func_800116BC(obj, 0.0f);
            hop = 0;
            if (f->cooldown > 0) {
                f->cooldown -= updateRate;
            }
            if (func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 96.0f, 1, list) > 0) {
                dx = obj->trans.x_position - list[0]->trans.x_position;
                dy = obj->trans.y_position - list[0]->trans.y_position;
                dz = obj->trans.z_position - list[0]->trans.z_position;
                if (f->cooldown <= 0 && dx * dx + dy * dy + dz * dz < 1600.0f) {
                    if (f->collect) {
                        func_800079B8(obj);
                        return;
                    }
                    f->state = 2;
                } else {
                    f->speed = 72.0f;
                    if (f->collect) {
                        f->speed *= 1.34f;
                    }
                    f->angle = func_80049C84(-dx, -dz) + 0x8000;
                    hop = 1;
                }
            } else {
                f->timer -= updateRate;
                if (f->timer < 0) {
                    hop = 1;
                    f->speed = func_80048E2C(40, 72);
                    f->angle = func_80048E2C(-0x4000, 0x4000) + obj->trans.rotation.y_rotation;
                }
            }
            if (hop) {
                i = 0;
                ok = 0;
                do {
                    f->vx = func_80049C34(f->angle) * f->speed;
                    f->vz = func_80049C30(f->angle) * -f->speed;
                    dx = obj->trans.x_position + f->vx - f->homeX;
                    dz = obj->trans.z_position + f->vz - f->homeZ;
                    i++;
                    if (dx * dx + dz * dz < f->rangeSq) {
                        ok = 1;
                    } else {
                        f->angle += 0x4000;
                    }
                } while (i < 4 && !ok);
                if (!ok) {
                    f->angle = func_80049C84(f->homeX - obj->trans.x_position, f->homeZ - obj->trans.z_position);
                }
                f->state = 1;
                f->startX = obj->trans.x_position;
                f->hop = 32;
                f->startZ = obj->trans.z_position;
                obj->trans.rotation.y_rotation = -f->angle;
            }
            break;
        case 1:
            f->hop -= updateRate;
            if (f->cooldown > 0) {
                f->cooldown -= updateRate;
            }
            if (f->hop < 0) {
                f->state = 0;
                f->timer = func_80048E2C(0, 300);
                f->hop = 0;
            }
            func_800116BC(obj, (32.0f - f->hop) * 0.03125f);
            s = (func_80049C30(f->hop << 10) + 1.0f) * 0.5f;
            obj->trans.x_position = f->startX;
            obj->trans.z_position = f->startZ;
            obj->velocity.x = f->vx * s;
            obj->velocity.z = f->vz * s;
            func_800099F4();
            func_80009A04(obj, obj->velocity.x, 0.0f, obj->velocity.z);
            if (func_8001B078(obj->trans.x_position, obj->trans.z_position, 0x800, &floor) != 0) {
                obj->trans.y_position = 0.0f;
                func_800099F4();
                func_80009A04(obj, 0.0f, **floor, 0.0f);
            }
            if (f->cooldown <= 0 && (f->hop < 6 || f->hop >= 27) &&
                func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 40.0f, 0, list) != 0) {
                if (f->collect) {
                    func_800079B8(obj);
                    return;
                }
                f->state = 2;
            }
            break;
        case 2:
            f->squash -= 0.15f * dt;
            if (f->squash < 0.05f) {
                f->squash = 0.05f;
                f->state = 3;
                f->timer = 40;
            }
            break;
        case 3:
            f->timer -= updateRate;
            if (f->timer < 0) {
                f->state = 4;
            }
            break;
        case 4:
            if (f->squash > 1.0f) {
                f->squash = 1.0f;
                f->state = 0;
                f->timer = 0;
                f->cooldown = 60;
            } else {
                f->squash += 0.15f * dt;
            }
            break;
        case 5:
            obj->particleEmittersEnabled = 1;
            func_8005F5AC(obj, updateRate);
            f->burstA += 0.3f * rate;
            f->burstB -= 0.1f * rate;
            if (f->burstB < 0.3f) {
                f->burstB = 0.3f;
                f->fade -= updateRate << 5;
                if (f->fade <= 0) {
                    f->fade = 0;
                    func_800079B8(obj);
                }
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m099", func_m099_0008C);

INCLUDE_ASM("asm/nonmatchings/modules/m099", data);
#endif
