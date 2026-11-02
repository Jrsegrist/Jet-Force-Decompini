/*
 * Runlink module 91: fish. A fish wanders round its home point in a box,
 * each axis on its own sine. It swims to bait (an object of behaviour 1)
 * within 120 units, chases and bites prey (behaviour 0x4D, while it has
 * hit points and is in the water) and follows behaviour 0x9B/0xA6 objects for
 * a while; out of the water (or when set to stay still) it flops about and
 * sinks or floats to the water level.
 */
#include "module.h"

/* The fish's level entry */
typedef struct M091Setup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 flags; /* 1: stays still */
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 rangeX;
    /* 0x0C */ u8 rangeY;
    /* 0x0D */ u8 rangeZ;
    /* 0x0E */ u8 scale; /* in hundredths */
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 phaseSpeed[3];
    /* 0x13 */ u8 swimming;
    /* 0x14 */ u8 damage;   /* in 256ths */
    /* 0x15 */ u8 biteDelay; /* in seconds */
} M091Setup;

/* obj->unk68 */
typedef struct M091Fish {
    /* 0x00 */ s16 side; /* which corners of the box to swim to near bait (bits 0-2: x, y, z) */
    /* 0x02 */ s16 swimming;
    /* 0x04 */ f32 reach;
    /* 0x08 */ Object *bait;
    /* 0x0C */ Object *prey;
    /* 0x10 */ Object *leader;
    /* 0x14 */ f32 homeX;
    /* 0x18 */ f32 homeY;
    /* 0x1C */ f32 homeZ;
    /* 0x20 */ f32 rangeX;
    /* 0x24 */ f32 rangeY;
    /* 0x28 */ f32 rangeZ;
    /* 0x2C */ f32 speed;
    /* 0x30 */ s16 unk30;
    /* 0x32 */ s16 phase[3];
    /* 0x38 */ s16 phaseSpeed[3];
    /* 0x3E */ u8 still;
    /* 0x3F */ u8 inWater;
    /* 0x40 */ f32 waterY;
    /* 0x44 */ s16 damage;
    /* 0x46 */ s16 biteDelay;
    /* 0x48 */ s16 timer;
    /* 0x4A */ s16 leaderTimer;
} M091Fish;

/* What the fish reads of its prey */
typedef struct M091Prey {
    /* 0x00 */ u8 pad0[0x88];
    /* 0x88 */ s32 health;
    /* 0x8C */ s32 unk8C;
} M091Prey;

typedef struct M091PreyData {
    /* 0x00 */ u8 pad0[0xE];
    /* 0x0E */ u8 inWater;
} M091PreyData;

extern s32 func_80048558(void);
extern s32 track_height_at(f32 x, f32 z, f32 *y, s32 mask); /* the water level at x, z */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern s16 angle_lerp(s16 from, s16 to, f32 t);              /* steps an angle towards another */
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern f32 sqrtf(f32);
extern s32 func_80049C84(f32 x, f32 z); /* arctangent as a 16-bit angle */

#define SQ(x) ((x) * (x))
s32 D_m091_00F20 = 91;

/*
 * NON_MATCHING, the whole module: find_targets is two instructions longer
 * (the prey's data pointer is loaded after its health test, and one constant
 * lands in a1), the init function differs in the scheduling of its last
 * stores, and the control function's frame is two words bigger with the
 * bait branch's float registers numbered differently. All three use the
 * .rodata, so the default build takes them all from assembly.
 */
#ifdef NON_MATCHING
#define FISH_SCALE_UNIT 0.01f
/* Looks for bait, prey and a leader near the fish */
static void find_targets(Object *obj, M091Fish *fish) {
    s32 first;
    s32 count;
    Object **objects;
    Object *o;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 nearestBait;
    f32 nearestPrey;
    f32 nearestLeader;

    if (fish->still == 0) {
        nearestBait = nearestPrey = nearestLeader = 1000000.0f;
        objects = func_80005E98(&first, &count);
        while (first < count) {
            o = objects[first++];
            if ((o->behaviorId == 1 && fish->bait == NULL) || (o->behaviorId == 0x4D && fish->prey == NULL) ||
                ((o->behaviorId == 0x9B || o->behaviorId == 0xA6) && fish->leader == NULL)) {
                dx = o->trans.x_position - obj->trans.x_position;
                dy = o->trans.y_position - obj->trans.y_position;
                dz = o->trans.z_position - obj->trans.z_position;
                if (o->behaviorId == 1) {
                    d = dx * dx + dy * dy + dz * dz;
                    if (d < 14400.0f && d < nearestBait) {
                        fish->bait = o;
                        nearestBait = d;
                    }
                }
                if (o->behaviorId == 0x4D && ((M091Prey *) o)->health > 0 &&
                    ((M091PreyData *) o->unk68)->inWater != 0) {
                    d = dx * dx + dy * dy + dz * dz;
                    if (d < 40000.0f && d < nearestPrey) {
                        fish->prey = o;
                        nearestPrey = d;
                    }
                }
                if (o->behaviorId == 0x9B || o->behaviorId == 0xA6) {
                    d = dx * dx + dy * dy + dz * dz;
                    if (d < 40000.0f) {
                        if (d < nearestLeader) {
                            fish->leader = o;
                            fish->leaderTimer = 600;
                            nearestLeader = d;
                        }
                    }
                }
            }
        }
        if ((o = fish->bait) != NULL) {
            fish->prey = NULL;
            fish->leader = NULL;
            fish->side = 0;
            if (o->trans.x_position < fish->homeX) {
                fish->side |= 1;
            }
            if (o->trans.y_position < fish->homeY) {
                fish->side |= 2;
            }
            if (o->trans.z_position < fish->homeZ) {
                fish->side |= 4;
            }
        }
        if (fish->prey != NULL) {
            fish->leader = NULL;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m091", func_m091_00000);
#endif

#ifdef NON_MATCHING

void func_m091_0031C(Object *obj, M091Setup *setup, s32 arg2) {
    M091Fish *fish = obj->unk68;
    void *model;
    s16 *v;
    s16 max;
    s16 a;
    s16 i;

    obj->trans.scale = setup->scale * FISH_SCALE_UNIT;
    fish->side = func_80048E2C(0, 7);
    max = 0;
    fish->swimming = setup->swimming << 8;
    i = 6;
    model = *obj->modelInstances;
    v = (s16 *) ((u8 *) model + 0x8C);
    while (i--) {
        a = *v++;
        if (a < 0) {
            a = -a;
        }
        if (max < a) {
            max = a;
        }
    }
    fish->reach = obj->trans.scale * max;
    fish->bait = NULL;
    fish->prey = NULL;
    fish->leader = NULL;
    fish->homeX = setup->x;
    fish->homeY = setup->y;
    fish->homeZ = setup->z;
    fish->rangeX = setup->rangeX;
    fish->rangeY = setup->rangeY;
    fish->rangeZ = setup->rangeZ;
    fish->speed = 0.0f;
    fish->unk30 = 0;
    fish->phase[0] = func_80048E2C(0, 0x10000);
    fish->phaseSpeed[0] = setup->phaseSpeed[0] << 4;
    fish->phase[1] = func_80048E2C(0, 0x10000);
    fish->phaseSpeed[1] = setup->phaseSpeed[1] << 4;
    fish->phase[2] = func_80048E2C(0, 0x10000);
    fish->phaseSpeed[2] = setup->phaseSpeed[2] << 4;
    fish->still = setup->flags & 1;
    if ((track_height_at(obj->trans.x_position, obj->trans.z_position, &fish->waterY, 0x2000) & 0x2000) &&
        obj->trans.y_position < fish->waterY) {
        fish->inWater = 1;
    } else {
        fish->waterY = obj->trans.y_position;
        fish->inWater = 1;
    }
    fish->damage = setup->damage << 8;
    fish->timer = 0;
    fish->biteDelay = setup->biteDelay * 60;
    fish->leaderTimer = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m091", func_m091_0031C);
#endif

#ifdef NON_MATCHING
void func_m091_005C4(Object *obj, s32 updateRate) {
    s16 yaw;
    s16 dyaw;
    s16 pitch;
    s16 dpitch;
    s16 step;
    s16 maxTurn;
    f32 tx;
    f32 ty;
    f32 tz;
    f32 speed;
    f32 ease;
    f32 dx;
    f32 dyTarget;
    f32 dz;
    f32 horiz;
    f32 rate2;
    f32 t;
    Object *target;
    s32 shift;
    s32 i;
    f32 rate;
    M091Fish *fish;

    runlink_unload_timer_set(D_m091_00F20, 2, 2);
    if (func_80048558() >= 2) {
        func_800079B8(obj);
        return;
    }
    fish = obj->unk68;
    rate = updateRate;
    rate2 = rate;
    if (fish->inWater == 0) {
        return;
    }
    if (fish->swimming == 0 && fish->still == 0) {
        t = func_80049BEC(0.95f, updateRate);
        obj->velocity.x *= t;
        obj->velocity.z *= t;
        ease = 1.0f - t;
        fish->timer += updateRate << 8;
        obj->trans.rotation.z_rotation =
            angle_lerp((s32) (func_80049C34(fish->timer) * 2048.0f) + 0x4000, obj->trans.rotation.z_rotation, ease);
        obj->trans.rotation.x_rotation = angle_lerp(0, obj->trans.rotation.x_rotation, ease);
        fish->speed *= func_80049BEC(0.99f, updateRate);
        if (fish->inWater) {
            speed = (fish->waterY - obj->trans.y_position) * 0.020833334f;
            if (speed > 1.0f) {
                speed = 1.0f;
            }
            obj->velocity.y += (speed - obj->velocity.y) * (1.0f - func_80049BEC(0.9f, updateRate));
        }
    } else {
        if ((target = fish->bait) != NULL) {
            if (SQ(target->trans.x_position - obj->trans.x_position) +
                    SQ(target->trans.y_position - obj->trans.y_position) +
                    SQ(target->trans.z_position - obj->trans.z_position) > 32400.0f) {
                fish->bait = NULL;
                target = NULL;
            }
            tx = fish->homeX - fish->rangeX;
            tz = fish->homeZ - fish->rangeZ;
            ty = fish->homeY - fish->rangeY;
            if (fish->side & 1) {
                tx += fish->rangeX + fish->rangeX;
            }
            if (fish->side & 2) {
                ty += fish->rangeY + fish->rangeY;
            }
            if (fish->side & 4) {
                tz += fish->rangeZ + fish->rangeZ;
            }
            if (target != NULL) {
                if (SQ(tx - obj->trans.x_position) + SQ(ty - obj->trans.y_position) + SQ(tz - obj->trans.z_position) <
                    400.0f) {
                    fish->side = func_80048E2C(0, 7);
                }
            }
            speed = 3.5f - obj->trans.scale * 0.35f;
            dyTarget = ty - obj->trans.y_position;
        } else if ((target = fish->prey) != NULL) {
            find_targets(obj, fish);
            if (fish->timer > 0) {
                fish->timer -= updateRate;
            } else {
                if (SQ(target->trans.x_position - obj->trans.x_position) +
                        SQ(target->trans.y_position - obj->trans.y_position) +
                        SQ(target->trans.z_position - obj->trans.z_position) < 400.0f) {
                    ((M091Prey *) target)->health -= fish->damage;
                    fish->timer = fish->biteDelay;
                    if (((M091Prey *) target)->health <= 0) {
                        fish->prey = NULL;
                    } else if (((M091Prey *) target)->unk8C == 0) {
                        target->velocity.z = -1.5f;
                        func_8004944C(&obj->trans.rotation, &target->velocity);
                    }
                }
            }
            tx = target->trans.x_position;
            tz = target->trans.z_position;
            speed = 1.75f - obj->trans.scale * 0.3f;
            dyTarget = target->trans.y_position - obj->trans.y_position;
        } else if ((target = fish->leader) != NULL) {
            find_targets(obj, fish);
            fish->leaderTimer -= updateRate;
            if (fish->leaderTimer < 0) {
                fish->leader = NULL;
            }
            tx = target->trans.x_position;
            tz = target->trans.z_position;
            speed = 1.0f - obj->trans.scale * 0.15f;
            dyTarget = target->trans.y_position - obj->trans.y_position;
        } else {
            find_targets(obj, fish);
            tx = func_80049C34(fish->phase[0]) * fish->rangeX + fish->homeX;
            ty = func_80049C34(fish->phase[1]) * fish->rangeY + fish->homeY;
            tz = func_80049C34(fish->phase[2]) * fish->rangeZ + fish->homeZ;
            speed = 1.25f - obj->trans.scale * 0.2f;
            dyTarget = ty - obj->trans.y_position;
        }
        dx = obj->trans.x_position - tx;
        dz = obj->trans.z_position - tz;
        horiz = sqrtf(dx * dx + dz * dz);
        yaw = func_80049C84(dx, dz);
        dyaw = yaw - obj->trans.rotation.y_rotation;
        pitch = func_80049C84(dyTarget, horiz);
        dpitch = pitch - obj->trans.rotation.x_rotation;
        if (fish->bait != NULL) {
            t = 0.875f;
            maxTurn = updateRate * 0x5B0;
            shift = 2;
        } else {
            t = 0.95f;
            maxTurn = updateRate * 0x16C;
            shift = 4;
        }
        step = (dyaw * updateRate) >> shift;
        if (step == 0) {
            step = dyaw;
        }
        if (step < -maxTurn) {
            obj->trans.rotation.y_rotation -= maxTurn;
        } else if (step > maxTurn) {
            obj->trans.rotation.y_rotation += maxTurn;
        } else {
            obj->trans.rotation.y_rotation += step;
        }
        step = (dpitch * updateRate) >> shift;
        if (step == 0) {
            step = dpitch;
        }
        if (step < -maxTurn) {
            obj->trans.rotation.x_rotation -= maxTurn;
        } else if (step > maxTurn) {
            obj->trans.rotation.x_rotation += maxTurn;
        } else {
            obj->trans.rotation.x_rotation += step;
        }
        dyaw = yaw - obj->trans.rotation.y_rotation;
        if (dyaw < 0) {
            dyaw = -dyaw;
        }
        dpitch = pitch - obj->trans.rotation.x_rotation;
        if (dpitch < 0) {
            dpitch = -dpitch;
        }
        if (dyaw > 0x400 || dpitch > 0x400) {
            speed = 0.75f;
        }
        fish->speed += (speed - fish->speed) * (1.0f - func_80049BEC(t, updateRate));
        for (i = 0; i < 3; i++) {
            fish->phase[i] += fish->phaseSpeed[i] * updateRate;
        }
        obj->velocity.z = -fish->speed;
        func_8004944C(&obj->trans.rotation, &obj->velocity);
    }
    func_80009A04(obj, obj->velocity.x * rate2, obj->velocity.y * rate2, obj->velocity.z * rate2);
    func_8001138C(obj, fish->speed * 0.03125f, rate);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m091", func_m091_005C4);

INCLUDE_ASM("asm/nonmatchings/modules/m091", rodata);
#endif
