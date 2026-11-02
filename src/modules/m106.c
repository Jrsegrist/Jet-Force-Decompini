/*
 * Runlink module 106: a hovering tank enemy (init, control, effects drawing
 * and free still in assembly) and the gems it drops: a gem object that
 * spins, fades out over its last 64 frames and sparkles at random, and the
 * function that spawns one with a given position, direction and speed.
 */
#include "module.h"

/* The tank's data (obj->unk68) */
typedef struct M106Tank {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ void *effect; /* a module 81 effect */
    /* 0x20 */ u8 pad20[0x24 - 0x20];
    /* 0x24 */ Object *child;
    /* 0x28 */ u8 pad28[0x3C - 0x28];
    /* 0x3C */ void *flare1;
    /* 0x40 */ void *flare2;
    /* 0x44 */ s32 sound1;
    /* 0x48 */ s32 sound2;
    /* 0x4C */ s32 sound3;
} M106Tank;

/* A gem's state, kept in the object */
typedef struct M106Gem {
    /* 0x00 */ s32 timer;
    /* 0x04 */ s32 alpha;
} M106Gem;

/* The level entry a gem is spawned from */
typedef struct M106GemEntry {
    LevelObjectEntryCommon common;
    /* 0x0A */ u16 size;
} M106GemEntry;

extern void effect_texture_pair_free(void *flare);
extern void func_m081_0034C(void *effect);
extern void func_80003614(s32 sound); /* stops a sound */
extern void sprite_anim_advance(void *model, s32 *flags, s32 arg2, f32 *frame, s32 updateRate);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern Object *func_80005F14(void *entry, s32 arg1); /* spawns an object from a level entry */

INCLUDE_ASM("asm/nonmatchings/modules/m106", func_m106_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m106", func_m106_00290);

INCLUDE_ASM("asm/nonmatchings/modules/m106", func_m106_01694);

void func_m106_01A30(Object *obj) {
    M106Tank *tank = obj->unk68;

    if (tank->flare1 != NULL) {
        effect_texture_pair_free(tank->flare1);
    }
    if (tank->flare2 != NULL) {
        effect_texture_pair_free(tank->flare2);
    }
    if (tank->effect != NULL) {
        func_m081_0034C(tank->effect);
    }
    if (tank->child != NULL) {
        func_800079B8(tank->child);
    }
    if (tank->sound1 != 0) {
        func_80003614(tank->sound1);
    }
    if (tank->sound2 != 0) {
        func_80003614(tank->sound2);
    }
    if (tank->sound3 != 0) {
        func_80003614(tank->sound3);
    }
}

void func_m106_01AFC(Object *obj, M106GemEntry *setup) {
    OBJ_STATE(obj, M106Gem)->timer = setup->size * 6;
    OBJ_STATE(obj, M106Gem)->alpha = 0xFF;
}

void func_m106_01B1C(Object *obj, s32 updateRate) {
    s32 flags;

    obj->interactObj->hitPoints = 0x1000;
    if (updateRate >= OBJ_STATE(obj, M106Gem)->timer) {
        func_800079B8(obj);
        return;
    }
    OBJ_STATE(obj, M106Gem)->timer -= updateRate;
    if (OBJ_STATE(obj, M106Gem)->timer < 64) {
        OBJ_STATE(obj, M106Gem)->alpha = OBJ_STATE(obj, M106Gem)->timer * 4;
    }
    flags = 9;
    sprite_anim_advance(obj->modelInstances[obj->modelIndex], &flags, 12, &obj->unk28, updateRate);
    if (func_80048E2C(0, 100) > 90) {
        obj->particleEmittersEnabled |= 1;
    } else {
        obj->particleEmittersEnabled &= ~1;
    }
    func_8005F5AC(obj, updateRate);
}

void func_m106_01C04(f32 x, f32 y, f32 z, s16 yRotation, s16 xRotation, f32 speed, f32 size) {
    Vec3s rot;
    M106GemEntry entry;
    Object *obj;

    entry.common.objectID = 0x311;
    entry.common.x = x;
    entry.common.y = y;
    entry.common.z = z;
    entry.size = (s32) (size * 10.0f);
    obj = func_80005F14(&entry, 1);
    if (obj != NULL) {
        obj->level_entry = NULL;
        rot.z_rotation = 0;
        rot.y_rotation = yRotation;
        rot.x_rotation = xRotation;
        obj->velocity.z = -speed;
        func_8004944C(&rot, &obj->velocity);
        if (obj->unk7C != NULL) {
            *obj->unk7C &= ~2;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m106", data);
