/*
 * Runlink module 120: three flame effects.
 * - A burst (00000/00060): plays its model's animation once; for a few frames
 *   it throws sparks (kind 2) and spawns flame particles in a cone or all
 *   around (kinds 0-2), then leaves dark puffs while it fades the linked
 *   object out.
 * - A lingering flame (0080C/008CC) with a light that dims and shrinks at the
 *   end of its life.
 * - A halo (009D0/00A24) that grows and fades out.
 */
#include "module.h"

/* An object field that object.h does not name yet */
#define OBJ_FIELD(obj, type, offset) (*(type *) ((u8 *) (obj) + (offset)))

extern u8 D_800A4FC4; /* a multiplayer game */

extern s32 func_800577A8(void *model, s32 *anim, s32 arg2, f32 *arg3, s32 updateRate); /* animates a model */
extern Object *func_80005F14(void *entry, s32 arg1);                                    /* DKR spawn_object */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern f32 func_80049BEC(f32 base, s32 exponent);

/* The burst's level entry */
typedef struct M120BurstSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ s8 kind;
    /* 0x11 */ s8 particleArg;
    /* 0x12 */ s8 variant;
    /* 0x13 */ s8 aimed;
    /* 0x14 */ s32 owner;
} M120BurstSetup;

/* obj->unk68 of the burst */
typedef struct M120Burst {
    /* 0x00 */ s8 framesLeft; /* of spawning */
    /* 0x01 */ u8 kind;
    /* 0x02 */ s8 puffTimer; /* -1: done */
    /* 0x03 */ u8 variant;
    /* 0x04 */ s16 yaw;
    /* 0x06 */ s16 pitch;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s8 particleArg;
    /* 0x0D */ s8 aimed;
    /* 0x10 */ s32 owner;
} M120Burst;

/* The object whose fade the burst drives (through obj+0x78) */
typedef struct M120Linked {
    /* 0x00 */ u8 pad0[0x3B];
    /* 0x3B */ u8 alpha;
} M120Linked;

/* The level entry of a flame particle */
typedef struct M120Particle {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ s16 life;
    /* 0x10 */ f32 speed;
    /* 0x14 */ f32 lifeScale;
    /* 0x18 */ f32 size;
    /* 0x1C */ s8 particleArg;
    /* 0x1D */ u8 pad1D[3];
    /* 0x20 */ s32 owner;
} M120Particle;

/* The level entry of the scorch decal of an aimed burst */
typedef struct M120Decal {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s8 size;
    /* 0x0B */ s8 angle;
} M120Decal;

/* The level entry of a dark puff */
typedef struct M120Puff {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ s16 life;
    /* 0x10 */ f32 scale;
} M120Puff;

extern s32 func_800210A4(Object *obj, LightParams *params); /* DKR light_add_from_object_header */
extern void func_80021358(s32 light, u8 intensity);              /* sets a light's intensity */
extern void func_8002179C(s32 light);                            /* DKR light_remove */

typedef struct M120LingerSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s8 scale; /* tenths */
    /* 0x0B */ s8 life;  /* tenths of a second */
} M120LingerSetup;

/* OBJ_STATE of the lingering flame and of the halo */
typedef struct M120Linger {
    /* 0x0 */ s32 light;
    /* 0x4 */ s16 timer;
    /* 0x6 */ s16 alpha;
} M120Linger;

typedef struct M120Halo {
    /* 0x0 */ f32 growth; /* scale per frame */
    /* 0x4 */ s16 timer;
    /* 0x6 */ s16 alpha;
} M120Halo;

typedef struct M120HaloSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 y_rotation;
    /* 0x0C */ s16 x_rotation;
    /* 0x0E */ s16 life;
    /* 0x10 */ f32 size;
} M120HaloSetup;

s32 D_m120_00AD0 = 120; /* the module's number */

void func_m120_00000(Object *obj, M120BurstSetup *setup) {
    M120Burst *b = obj->unk68;

    b->framesLeft = 4;
    b->kind = setup->kind;
    b->puffTimer = 0;
    b->variant = setup->variant;
    b->unk8 = 0;
    b->particleArg = setup->particleArg;
    b->yaw = setup->yaw;
    b->pitch = setup->pitch;
    b->aimed = setup->aimed;
    b->owner = setup->owner;
}

void func_m120_00060(Object *obj, s32 updateRate) {
    M120Burst *b;
    s32 pitchMin;
    s32 pitchMax;
    s32 speedMin;
    s32 speedMax;
    s32 lifeMin;
    s32 lifeMax;
    s32 yawMin;
    s32 yawMax;
    s32 anim;
    M120Particle particle;
    M120Linked *linked;
    Object *spawned;
    s32 i;
    SparkParams spark;
    M120Decal decal;
    s32 unused2;
    M120Puff puff;
    s32 unused3;

    func_800541B0(D_m120_00AD0, 2, 2);
    b = obj->unk68;
    anim = 1;
    if (func_800577A8(obj->unk6C[0], &anim, 15, &obj->unk28, updateRate) != 0) {
        func_800079B8(obj);
        return;
    }
    if (OBJ_FIELD(obj, M120Linked **, 0x78) != NULL &&
        (linked = *OBJ_FIELD(obj, M120Linked **, 0x78)) != NULL) {
        i = linked->alpha - updateRate * 4;
        if (i < 0) {
            i = 0;
        }
        linked->alpha = i;
    }
    if (b->framesLeft != 0) {
        b->framesLeft--;
        if (b->kind == 2) {
            spark.x_position = obj->trans.x_position;
            spark.y_position = obj->trans.y_position;
            spark.z_position = obj->trans.z_position;
            spark.unk18 = 20;
            spark.unk1A = 10;
            spark.unk1B = 0xFF;
            spark.unk1C = 0xFFFF80FF;
            spark.unk20 = 0xFF0000FF;
            i = 4;
            if (D_800A4FC4) {
                i = 2;
            }
            while (i--) {
                spark.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
                spark.x_rotation = func_80048E2C(0x1000, 0x3000);
                spark.speed = func_80048E2C(70, 90) * 0.1f;
                spark.unk14 = func_80048E2C(25, 50) * 0.1f;
                func_m010_0000C(&spark);
            }
        }
        if (b->kind == 2) {
            particle.objectID = 0x14A;
        } else if (b->variant == 0) {
            particle.objectID = 0x139;
        } else {
            particle.objectID = 0x138;
        }
        particle.x = obj->trans.x_position;
        particle.y = obj->trans.y_position;
        particle.z = obj->trans.z_position;
        particle.particleArg = b->particleArg;
        particle.owner = b->owner;
        switch (b->kind) {
            case 0:
                yawMin = -0x8000;
                yawMax = 0x7FFF;
                pitchMin = 0x3000;
                pitchMax = 0x3C00;
                speedMin = 50;
                speedMax = 75;
                lifeMin = 150;
                lifeMax = 180;
                particle.life = 180;
                particle.size = 0.45f;
                break;
            case 1:
                yawMin = b->yaw - 0x3000;
                yawMax = b->yaw + 0x3000;
                pitchMin = b->pitch - 0x2000;
                pitchMax = b->pitch + 0x2000;
                speedMin = 50;
                speedMax = 75;
                lifeMin = 150;
                lifeMax = 180;
                particle.life = 180;
                particle.size = 0.45f;
                break;
            case 2:
                if (b->aimed) {
                    yawMin = b->yaw - 0x3000;
                    yawMax = b->yaw + 0x3000;
                    pitchMin = b->pitch - 0x2000;
                    pitchMax = b->pitch + 0x2000;
                } else {
                    yawMin = -0x8000;
                    yawMax = 0x7FFF;
                    pitchMin = 0x1000;
                    pitchMax = 0x2000;
                }
                speedMin = 15;
                speedMax = 25;
                lifeMin = 50;
                lifeMax = 80;
                particle.life = 120;
                particle.size = 0.15f;
                break;
        }
        if (b->kind != 0 && !D_800A4FC4) {
            i = 3;
            if (updateRate >= 4) {
                i = 2;
            }
            if (updateRate >= 5) {
                i = 1;
            }
            while (i--) {
                particle.yaw = func_80048E2C(yawMin, yawMax);
                particle.pitch = func_80048E2C(pitchMin, pitchMax);
                particle.speed = func_80048E2C(speedMin, speedMax) * 0.01f;
                particle.lifeScale = func_80048E2C(lifeMin, lifeMax) * 0.1f;
                spawned = func_80005F14(&particle, 1);
                if (spawned != NULL) {
                    spawned->level_entry = NULL;
                }
            }
        }
        if (!D_800A4FC4 && b->kind == 1 && b->aimed && b->framesLeft == 0) {
            if (b->pitch > 0x3000 && b->pitch < 0x5000 && updateRate < 5) {
                decal.objectID = 0x13D;
                decal.x = obj->trans.x_position;
                decal.y = obj->trans.y_position;
                decal.z = obj->trans.z_position;
                decal.size = particle.speed * 85.0f;
                decal.angle = particle.pitch >> 7;
                spawned = func_80005F14(&decal, 1);
                if (spawned != NULL) {
                    spawned->level_entry = NULL;
                }
            }
        }
    }
    if (b->puffTimer == 0) {
        if (OBJ_FIELD(obj, M120Linked **, 0x78) != NULL &&
            (linked = *OBJ_FIELD(obj, M120Linked **, 0x78)) != NULL) {
            linked->alpha = 0x80;
        }
    }
    if (b->puffTimer >= 0 && b->kind < 2) {
        if (b->puffTimer == 0 || b->puffTimer >= 20) {
            puff.objectID = 0x13B;
            puff.x = obj->trans.x_position;
            puff.y = obj->trans.y_position + 3.0f;
            puff.z = obj->trans.z_position;
            puff.unkA = 0;
            puff.pitch = -0x4000;
            if (b->puffTimer == 0) {
                puff.life = 70;
                puff.scale = 0.5f;
            } else {
                puff.life = 45;
                puff.scale = 0.7f;
            }
            spawned = func_80005F14(&puff, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
        }
        if (b->puffTimer >= 20) {
            b->puffTimer = -1;
        } else {
            b->puffTimer += updateRate;
        }
    }
}

void func_m120_0080C(Object *obj, M120LingerSetup *setup) {
    LightParams lp;
    s32 unusedA;
    s32 unusedB;

    obj->trans.scale = setup->scale * 0.1f;
    OBJ_STATE(obj, M120Linger)->timer = setup->life * 6;
    OBJ_STATE(obj, M120Linger)->alpha = 0xFF;
    lp.type = 1;
    lp.unk1 = 3;
    lp.flags = 0x31;
    lp.unk3 = -1;
    lp.unk4 = 0;
    lp.unk6 = 0;
    lp.unk8 = 0;
    lp.unkA = 0;
    lp.radius = 0x100;
    lp.radius2 = 0x100;
    lp.a = 0xFF;
    lp.unk14 = 0x12;
    lp.unk16 = 0;
    lp.unk17 = 0;
    OBJ_STATE(obj, M120Linger)->light = func_800210A4(obj, &lp);
}

void func_m120_008CC(Object *obj, s32 updateRate) {
    s32 anim;
    M120Linger *st;

    func_800541B0(D_m120_00AD0, 2, 2);
    st = OBJ_STATE(obj, M120Linger);
    st->timer -= updateRate;
    if (st->timer > 0) {
        obj->particleEmittersEnabled = 1;
        func_8005F5AC(obj, updateRate);
        anim = 9;
        func_800577A8(obj->unk6C[0], &anim, 10, &obj->unk28, updateRate);
        if (st->timer < 0x40) {
            obj->trans.scale *= func_80049BEC(0.99f, updateRate);
            st->alpha = st->timer * 4;
            if (st->light != 0) {
                func_80021358(st->light, st->timer * 4);
            }
        }
    } else {
        if (st->light != 0) {
            func_8002179C(st->light);
        }
        func_800079B8(obj);
    }
}

void func_m120_009D0(Object *obj, M120HaloSetup *setup) {
    obj->trans.rotation.y_rotation = setup->y_rotation;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->trans.scale = 0.01f;
    OBJ_STATE(obj, M120Halo)->growth = setup->size / setup->life;
    OBJ_STATE(obj, M120Halo)->timer = setup->life;
    OBJ_STATE(obj, M120Halo)->alpha = 0xFF;
}

void func_m120_00A24(Object *obj, s32 updateRate) {
    M120Halo *st;

    func_800541B0(D_m120_00AD0, 2, 2);
    st = OBJ_STATE(obj, M120Halo);
    st->timer -= updateRate;
    if (st->timer > 0) {
        obj->trans.scale += st->growth * updateRate;
        if (st->timer < 0x20) {
            st->alpha = st->timer * 8;
        }
    } else {
        func_800079B8(obj);
    }
}
