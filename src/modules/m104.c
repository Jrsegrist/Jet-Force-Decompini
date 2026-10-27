/*
 * Runlink module 104: health pickups. They sit on the floor below where
 * they are placed, give the player health (or, kind 0xE9, a health upgrade,
 * once: a game flag remembers it), then respawn after a delay or are freed.
 * Pickups dropped with a lifetime fade out and are freed when it runs out.
 */
#include "module.h"

typedef struct M104Setup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 dropped; /* 1: has a lifetime, and is not put on the floor */
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s8 size;
    /* 0x0B */ s8 respawn; /* in seconds; 0: no respawn */
    /* 0x0C */ u8 flag;
} M104Setup;

/* The pickup's data (its object's unk68) */
typedef struct M104Pickup {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 unkC; /* the object's 0x2E */
    /* 0x0D */ u8 respawn;
    /* 0x0E */ s16 timer;    /* until it comes back */
    /* 0x10 */ s16 lifetime; /* 0: none */
    /* 0x12 */ s16 objectID;
    /* 0x14 */ u8 flag;
} M104Pickup;

/* Kept in the object at 0x88 */
typedef struct M104State {
    /* 0x0 */ s32 size; /* 0: small, 1: medium, 2: full */
    /* 0x4 */ s32 fade; /* 0: hidden; counts up to 0xFF */
} M104State;

typedef struct M104HeaderUnk50 {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ f32 height;
    /* 0x8 */ u8 pad8[4];
    /* 0xC */ f32 radius;
} M104HeaderUnk50;

/* The player's object data (its unk68) */
typedef struct M104PlayerData {
    /* 0x0 */ s8 player;
    /* 0x1 */ s8 unk1;
} M104PlayerData;

/* What func_80047C1C returns */
typedef struct M104PlayerStats {
    /* 0x0 */ u8 pad0[2];
    /* 0x2 */ s16 healthUpgrades;
} M104PlayerStats;

#define ST(obj) OBJ_STATE(obj, M104State)

/* The object's s16 at 0x2E */

extern void func_800541B0(s32, s32, s32);
extern s32 func_8007E150(f32, f32, f32, f32, s32, Object **);
extern M104PlayerStats *func_80047C1C(s32 player, s32 arg1);
extern void func_80001248(s32 sound, s32 arg1); /* DKR amSndPlay */
extern void func_800577A8(void *model, s32 *anim, s32 arg2, f32 *arg3, s32 updateRate);
extern void func_8005F5AC(Object *obj, s32 updateRate);

s32 D_m104_006A0 = 104; /* the module's number */

void func_m104_00000(Object *obj, M104Setup *setup, s32 arg2) {
    M104Pickup *p = obj->unk68;
    u8 hit[0x28];
    Vec3f from;
    Vec3f to;
    f32 radius;
    f32 height;

    if (arg2 == 0 && setup->dropped != 1) {
        height = ((M104HeaderUnk50 *) obj->header->unk50)->height;
        radius = ((M104HeaderUnk50 *) obj->header->unk50)->radius;
        from.x = obj->trans.x_position;
        from.y = obj->trans.y_position + height + radius;
        from.z = obj->trans.z_position;
        to.x = obj->trans.x_position;
        to.y = obj->trans.y_position + height - radius;
        to.z = obj->trans.z_position;
        func_8001CE10(1, &from, &to, &radius, 0x2000, 0, 0);
        if (func_80016208(NULL, &from, &to, radius, 1, hit) != 0) {
            obj->trans.x_position = to.x;
            obj->trans.y_position = to.y - height;
            obj->trans.z_position = to.z;
        }
    }
    p->x = obj->trans.x_position;
    p->y = obj->trans.y_position;
    p->z = obj->trans.z_position;
    p->unkC = obj->segmentID;
    p->respawn = setup->respawn;
    p->timer = 0;
    p->lifetime = 0;
    p->objectID = setup->objectID;
    p->flag = setup->flag;
    if (setup->dropped == 1) {
        p->lifetime = 480;
    }
    ST(obj)->size = setup->size;
    ST(obj)->fade = 0xFF;
    obj->trans.scale = obj->header->scale;
    if (ST(obj)->size == 1) {
        obj->trans.scale *= 1.3f;
    }
    if (ST(obj)->size == 2) {
        obj->trans.scale *= 1.6f;
    }
}

void func_m104_001F0(Object *obj, s32 updateRate) {
    s32 near;
    M104Pickup *p = obj->unk68;
    s32 max;
    s32 anim;
    f32 dx;
    f32 dy;
    f32 dz;
    Object *list[8];
    M104PlayerData *pd;
    M104PlayerStats *stats;
    ObjectInteraction *hit;
    s32 n;

    func_800541B0(D_m104_006A0, 2, 2);
    if (p->lifetime != 0) {
        p->lifetime -= updateRate;
        if (p->lifetime <= 0) {
            func_800079B8(obj);
            return;
        }
        if (p->lifetime < 0x80) {
            ST(obj)->fade = p->lifetime * 2;
        }
    }
    if (ST(obj)->fade == 0) {
        obj->trans.x_position = p->x;
        obj->trans.y_position = p->y;
        obj->trans.z_position = p->z;
        obj->segmentID = p->unkC;
        if (obj->unk7C != NULL) {
            *obj->unk7C |= 2;
        }
    }
    if (p->objectID == 0xE9) {
        if (func_80047ED0(p->flag + 0x2F)) {
            func_800079B8(obj);
            return;
        }
        func_8001138C(obj, 0.01f, updateRate);
    } else {
        anim = 9;
        func_800577A8(obj->unk6C[obj->modelIndex], &anim, 12, &obj->unk28, updateRate);
    }
    n = func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 250.0f, 0, list);
    near = 0;
    if (n != 0) {
        dx = obj->trans.x_position - list[0]->trans.x_position;
        dy = obj->trans.y_position - list[0]->trans.y_position;
        dz = obj->trans.z_position - list[0]->trans.z_position;
        if (dx * dx + dy * dy + dz * dz < 1024.0f) {
            near = 1;
        }
    }
    if (ST(obj)->fade == 0) {
        p->timer -= updateRate;
        if (p->timer <= 0) {
            p->timer = 0;
            if (!near) {
                ST(obj)->fade = updateRate;
            }
        }
    } else {
        ST(obj)->fade += updateRate * 4;
        if (ST(obj)->fade >= 0x100) {
            ST(obj)->fade = 0xFF;
        }
        if (n != 0) {
            if (func_80048E2C(0, 1000) > 800) {
                obj->particleEmittersEnabled |= 1;
            } else {
                obj->particleEmittersEnabled &= ~1;
            }
            func_8005F5AC(obj, updateRate);
            if (near) {
                hit = list[0]->interactObj;
                pd = list[0]->unk68;
                if (hit->unk6 >= 0x100) {
                    stats = func_80047C1C(pd->player, pd->unk1);
                    if (p->objectID == 0xE9) {
                        if (stats->healthUpgrades < 12) {
                            stats->healthUpgrades++;
                            hit->unk6 = (stats->healthUpgrades * 5 + 4) << 8;
                            func_m006_011E0(pd, 0x87);
                            func_80001248(0x1E7, 0);
                            max = stats->healthUpgrades * 5 + 4;
                            func_80047EFC(p->flag + 0x2F, 1);
                        } else {
                            func_m006_011E0(pd, 0x13F);
                            return;
                        }
                    } else {
                        max = stats->healthUpgrades * 5 + 4;
                        switch (ST(obj)->size) {
                            case 1:
                                hit->unk6 += 0x500;
                                func_80001248(0x229, 0);
                                break;
                            case 2:
                                hit->unk6 = max << 8;
                                func_80001248(0x22A, 0);
                                break;
                            default:
                                hit->unk6 += 0x100;
                                func_80001248(0x228, 0);
                                break;
                        }
                        func_m006_011E0(pd, ST(obj)->size + 0x84);
                    }
                    if (max << 8 < hit->unk6) {
                        hit->unk6 = max << 8;
                    }
                    if (p->respawn != 0) {
                        p->timer = p->respawn * 60;
                        ST(obj)->fade = 0;
                    } else {
                        func_800079B8(obj);
                    }
                }
            }
        }
    }
}
