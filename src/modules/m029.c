/*
 * Runlink module 29: a player special attack with its own camera. When the
 * player's state byte asks for it (and the player stands still), it fades the
 * fog in, spawns an effect object above the player and frames the player with
 * a camera that circles them. Then it looks for a target (an object of
 * behaviour 0x18 that still has hit points, within 1000 units and not under
 * a floor), cuts the camera to a fixed shot of it, flies the effect down onto
 * it over a second, and takes 0x5000 hit points off it with an explosion and
 * a sound. With no target left it fades the fog out and frees the effect.
 * The fog getters let the renderer darken the colours while it runs.
 */
#include "module.h"

/* The player state this module reads and steps through */
typedef struct M029Player {
    /* 0x000 */ u8 pad0[4];
    /* 0x004 */ f32 speed; /* only starts at 0 */
    /* 0x008 */ u8 pad8[0x19D - 0x8];
    /* 0x19D */ u8 attackState; /* 1 start, 2 wait, 3 find a target, 4 strike, 5 end; 0 off */
    /* 0x19E */ u8 pad19E[0x568 - 0x19E];
    /* 0x568 */ u8 unk568; /* cancels the start */
} M029Player;

/* The level entry of the effect object it spawns */
typedef struct M029Entry {
    /* 0x0 */ s16 objectID;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s16 x;
    /* 0x6 */ s16 y;
    /* 0x8 */ s16 z;
    /* 0xA */ u8 unkA;
    /* 0xB */ u8 unkB;
    /* 0xC */ u8 unkC;
    /* 0xD */ u8 unkD;
} M029Entry;

/* What a target's interactObj holds here */
typedef struct M029Interaction {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0x3C - 0x8];
    /* 0x3C */ s32 unk3C;
} M029Interaction;


extern Object *func_80041E0C(void); /* the camera object */
extern void *D_800F6DC0;            /* the camera's transform */
extern Object **func_80005E98(s32 *first, s32 *count);
extern Object *func_80005F14(M029Entry *entry, s32 count); /* spawns an object from a level entry */
extern s32 func_80050ED0(s32 id);
extern s32 func_8001B078(f32 x, f32 z, s32 arg2, f32 ***heights); /* floor heights at x, z */
extern s32 func_80082D80(f32 x, f32 z, s32 arg2, f32 ***heights);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern void func_800285B0(s32 index, s16 x, s16 y, s16 z, s32 arg4, Object *obj); /* spawn entry index at x, y, z */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern f32 sqrtf(f32);
extern s32 func_80049C84(f32 x, f32 z); /* arctangent as a 16-bit angle */
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s16 func_80033FA4(s16 from, s16 to, f32 t); /* steps an angle towards another */

s32 D_m029_00F90 = 29;
static s32 sCutTimer = 0;
static f32 sFog = 0.0f;
static f32 sFogFrom = 0.0f;
static f32 sFogTo = 0.0f;
static s32 sFogDuration = 0;
static s32 sFogTimer = 0;

static s32 sStrikeTimer;
static s32 sTimer;
static Object *sTarget;
static Object *sEffect;
static f32 sEffectStartX; /* where the effect starts its fall */
static f32 sEffectStartY;
static f32 sEffectStartZ;
static f32 sCentreX; /* of the circling camera */
static f32 sCentreY;
static f32 sCentreZ;
static f32 sRadius;
static f32 sHeight;
static s16 sAngle;
static s16 sAngleSpeed;
static s16 sRestoreYaw; /* what the camera turns back to after the cut (never set) */
static s16 sRestorePitch;
static f32 sCutX;
static f32 sCutY;
static f32 sCutZ;
static s16 sCutYaw;
static s16 sCutPitch;
static s16 sSavedYaw; /* the camera's before the cut */
static s16 sSavedPitch;
static s32 sWaitTimer;

void func_m029_0070C(Object *obj);
void func_m029_00AD0(f32 x, f32 y, f32 z, f32 dist, f32 height, s16 angle, f32 seconds);
void func_m029_00BD8(f32 to, f32 seconds);
void func_m029_00C34(s32 updateRate);

/*
 * NON_MATCHING: IDO keeps the target's x for the floor check in the saved f20
 * (and so saves f20/f21), where the original uses f12. The jump table and the
 * 1000000.0f are this function's, so the .rodata stays assembly with it.
 */
#ifdef NON_MATCHING
void func_m029_00000(Object *obj, M029Player *player, s32 updateRate) {
    s32 first;
    s32 count;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 t;
    f32 tx;
    M029Entry entry;
    f32 **heights;
    f32 **heights2;
    Object **objects;
    f32 tz;
    f32 ty;

    runlink_unload_timer_set(D_m029_00F90, 2, 2);
    switch (player->attackState) {
        case 1:
            if (player->unk568 != 0) {
                player->attackState = 0;
            } else if (player->speed == 0.0f) {
                func_m029_00BD8(1.0f, 2.75f);
                func_m029_0070C(obj);
                player->attackState = 2;
                sWaitTimer = 180;
                entry.objectID = 0x56;
                entry.x = 0;
                entry.y = 0;
                entry.z = 0;
                entry.unkA = 0;
                entry.unkB = 0x40;
                entry.unkC = 0;
                entry.unkD = 0;
                sEffect = func_80005F14(&entry, 1);
                if (sEffect != NULL) {
                    sEffect->level_entry = NULL;
                    sEffect->trans.flags |= 0x400;
                }
            }
            break;
        case 2:
            sWaitTimer -= updateRate;
            if (sWaitTimer <= 0) {
                player->attackState = 3;
            }
            break;
        case 3:
            objects = func_80005E98(&first, &count);
            sTarget = NULL;
            while (first < count) {
                sTarget = objects[first++];
                if (sTarget->behaviorId == 0x18 && sTarget->interactObj != NULL &&
                    sTarget->interactObj->hitPoints > 0 && func_80050ED0(sTarget->objectID) == 0) {
                    dx = obj->trans.x_position - sTarget->trans.x_position;
                    dy = obj->trans.y_position - sTarget->trans.y_position;
                    dz = obj->trans.z_position - sTarget->trans.z_position;
                    if (dx * dx + dy * dy + dz * dz < 1000000.0f) {
                        if (func_8001B078(sTarget->trans.x_position, sTarget->trans.z_position, 0x800,
                                          &heights) != 0 &&
                            sTarget->trans.y_position < **heights) {
                            sTarget = NULL;
                        } else if (func_80082D80(sTarget->trans.x_position, sTarget->trans.z_position, 0x800,
                                                 &heights2) != 0 &&
                                   sTarget->trans.y_position < **heights2) {
                            sTarget = NULL;
                        } else {
                            break;
                        }
                        continue;
                    }
                }
                sTarget = NULL;
            }
            if (sTarget == NULL) {
                player->attackState = 5;
                sTimer = 180;
                func_m029_00BD8(0.0f, 2.75f);
            } else {
                sEffectStartX = sTarget->trans.x_position;
                sEffectStartY = sTarget->trans.y_position + 2000.0f;
                sEffectStartZ = sTarget->trans.z_position;
                if (sEffect != NULL) {
                    func_80009A04(sEffect, sEffectStartX - sEffect->trans.x_position,
                                  sEffectStartY - sEffect->trans.y_position,
                                  sEffectStartZ - sEffect->trans.z_position);
                    sEffect->trans.flags &= ~0x400;
                }
                player->attackState = 4;
                sStrikeTimer = 60;
                func_m029_00AD0(sTarget->trans.x_position, sTarget->trans.y_position,
                                sTarget->trans.z_position, 75.0f, 400.0f, func_80048E2C(0, 0x10000), 4.0f);
                func_80001248(0xF1, 0);
            }
            break;
        case 4:
            if (sStrikeTimer != 0) {
                sStrikeTimer -= updateRate;
                if (sStrikeTimer < 0) {
                    sStrikeTimer = 0;
                }
                if (sEffect != NULL) {
                    t = sStrikeTimer / 60.0f;
                    tx = sTarget->trans.x_position;
                    ty = sTarget->trans.y_position;
                    tz = sTarget->trans.z_position;
                    func_80009A04(sEffect, (tx + (sEffectStartX - tx) * t) - sEffect->trans.x_position,
                                  (ty + (sEffectStartY - ty) * t) - sEffect->trans.y_position,
                                  (tz + (sEffectStartZ - tz) * t) - sEffect->trans.z_position);
                    sEffect->particleEmittersEnabled = 1;
                    func_8005F5AC(sEffect, updateRate);
                }
                if (sStrikeTimer == 0) {
                    sTarget->interactObj->hitPoints -= 0x5000;
                    ((M029Interaction *) sTarget->interactObj)->unk3C = 0;
                    func_800285B0(12, sTarget->trans.x_position, sTarget->trans.y_position + 8.0f,
                                  sTarget->trans.z_position, 15, obj);
                    func_80003410(0x22, sTarget->trans.x_position, sTarget->trans.y_position,
                                  sTarget->trans.z_position, 4, NULL);
                    sTimer = 120;
                    if (sEffect != NULL) {
                        sEffect->trans.flags |= 0x400;
                    }
                }
            } else {
                sTimer -= updateRate;
                if (sTimer <= 0) {
                    player->attackState = 3;
                }
            }
            break;
        case 5:
            sTimer -= updateRate;
            if (sTimer <= 0) {
                player->attackState = 0;
                if (sEffect != NULL) {
                    func_800079B8(sEffect);
                }
            }
            break;
    }
    func_m029_00C34(updateRate);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m029", func_m029_00000);
#endif

#ifdef NON_MATCHING
#define CAMERA_EASE 0.975f
#else
/* the .rodata is assembly: the constant through the section's start, as IDO refers to it */
extern f32 rl_m029_00FB0[];
#define CAMERA_EASE rl_m029_00FB0[6]
#endif

/* Starts the camera circling the object */
void func_m029_0070C(Object *obj) {
    f32 dx;
    f32 dz;
    Object *camera;

    camera = func_80041E0C();
    sCentreX = obj->trans.x_position;
    sCentreY = obj->trans.y_position + 45.0f;
    sCentreZ = obj->trans.z_position;
    dx = camera->trans.x_position - sCentreX;
    dz = camera->trans.z_position - sCentreZ;
    sRadius = sqrtf(dx * dx + dz * dz);
    sHeight = 0.0f;
    sAngle = 0x8000 - func_80049C84(dx, dz);
    sAngleSpeed = 0;
    sCutTimer = 0;
}

/* The camera: the cut to the target while it lasts, else circling the player */
void func_m029_007D4(Object *camera, s32 arg1, s32 updateRate) {
    s16 yaw;
    s16 pitch;
    f32 x;
    f32 z;
    f32 dy;
    f32 dz;
    f32 t;

    if (sCutTimer > 0) {
        sCutTimer -= updateRate;
        if (sCutTimer <= 0) {
            camera->trans.rotation.y_rotation = sRestoreYaw;
            camera->trans.rotation.x_rotation = sRestorePitch;
            sCutTimer = 0;
        } else {
            camera->trans.x_position = sCutX;
            camera->trans.y_position = sCutY;
            camera->trans.z_position = sCutZ;
            if (sEffect != NULL) {
                x = ((Object *) D_800F6DC0)->trans.x_position - sEffect->trans.x_position;
                dy = ((Object *) D_800F6DC0)->trans.y_position - sEffect->trans.y_position;
                dz = ((Object *) D_800F6DC0)->trans.z_position - sEffect->trans.z_position;
                camera->trans.rotation.y_rotation = 0x8000 - func_80049C84(x, dz);
                camera->trans.rotation.x_rotation = func_80049C84(dy, sqrtf(x * x + dz * dz));
            } else {
                camera->trans.rotation.y_rotation = sCutYaw;
                camera->trans.rotation.x_rotation = sCutPitch;
            }
        }
    }
    if (sCutTimer <= 0) {
        t = 1.0f - func_80049BEC(CAMERA_EASE, updateRate);
        sRadius += (100.0f - sRadius) * t;
        sHeight += (180.0f - sHeight) * t;
        sAngleSpeed += updateRate * 31;
        if (sAngleSpeed >= 0x110) {
            sAngleSpeed = 0x10F;
        }
        sAngle += sAngleSpeed * updateRate;
        x = func_80049C34(sAngle) * sRadius;
        z = -func_80049C30(sAngle) * sRadius;
        camera->trans.x_position = sCentreX + x;
        camera->trans.y_position = sCentreY + sHeight;
        camera->trans.z_position = sCentreZ + z;
        yaw = 0x8000 - func_80049C84(x, z);
        pitch = func_80049C84(sHeight, sRadius);
        t = 1.0f - func_80049BEC(0.875f, updateRate);
        camera->trans.rotation.y_rotation = func_80033FA4(camera->trans.rotation.y_rotation, yaw, t);
        camera->trans.rotation.x_rotation = func_80033FA4(camera->trans.rotation.x_rotation, pitch, t);
    }
}

/* Sets up the cut: a fixed shot of (x, y, z) from dist away at the angle, for the given seconds */
void func_m029_00AD0(f32 x, f32 y, f32 z, f32 dist, f32 height, s16 angle, f32 seconds) {
    f32 dx;
    f32 dz;
    Object *camera;

    camera = func_80041E0C();
    dx =func_80049C34(angle) * dist;
    dz = -func_80049C30(angle) * dist;
    sCutX = x + dx;
    sCutY = y + height;
    sCutZ = z + dz;
    sCutYaw = 0x8000 - func_80049C84(dx, dz);
    sCutPitch = func_80049C84(height, dist);
    sSavedYaw = camera->trans.rotation.y_rotation;
    sSavedPitch = camera->trans.rotation.x_rotation;
    sCutTimer = seconds * 60.0f;
}

/* Fades the fog to `to` over the given seconds */
void func_m029_00BD8(f32 to, f32 seconds) {
    sFogFrom = sFog;
    sFogTo = to;
    sFogTimer = sFogDuration = seconds * 60.0f;
}

void func_m029_00C34(s32 updateRate) {
    sFog = sFogTo;
    if (updateRate < sFogTimer) {
        sFogTimer -= updateRate;
        sFog += (sFogFrom - sFogTo) * ((f32) sFogTimer / (f32) sFogDuration);
    }
}

/* Darkens the fog colour and pulls the fog range in by the fog amount */
void func_m029_00CA8(u8 *r, u8 *g, u8 *b, s16 *near, s16 *far, s8 *a) {
    f32 f;

    if (sFog != 0.0f) {
        f = 1.0f - sFog;
        *r = (s32) (*r * f);
        *g = (s32) (*g * f);
        *b = (s32) (*b * f);
        *near += (s32) ((985 - *near) * sFog);
        *far += (s32) ((1020 - *far) * sFog);
        *a = (s32) (*a * f);
    }
}

void func_m029_00E98(s32 *a, s32 *b, s32 *c) {
    if (sFog != 0.0f) {
        *a += (s32) ((64 - *a) * sFog);
        *b += (s32) ((64 - *b) * sFog);
        *c += (s32) ((64 - *c) * sFog);
    }
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m029", rodata);
#endif
