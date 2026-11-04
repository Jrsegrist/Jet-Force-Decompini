/*
 * Runlink module 106: a hovering tank enemy (init and control still in
 * assembly; its effects are drawn far to near) and the gems it drops: a gem object that
 * spins, fades out over its last 64 frames and sparkles at random, and the
 * function that spawns one with a given position, direction and speed.
 */
#include "module.h"

/* The tank's data (obj->unk68) */
typedef struct M106Tank {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ Vec3f effectPos;
    /* 0x10 */ u8 pad10[0x1C - 0x10];
    /* 0x1C */ void *effect; /* a module 81 effect */
    /* 0x20 */ f32 unk20;
    /* 0x24 */ Object *child;
    /* 0x28 */ s16 unk28;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ f32 unk30;
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 unk35;
    /* 0x36 */ u8 unk36;
    /* 0x37 */ u8 pad37;
    /* 0x38 */ union {
        s16 word;
        struct {
            u8 unk38;
            u8 flareAlpha;
        } b;
    } u38;
    /* 0x3A */ s8 unk3A; /* random */
    /* 0x3B */ s8 unk3B;
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

/* What the tank's init gets */
typedef struct M106Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 scale; /* in 20ths */
} M106Setup;

extern f32 rl_m106_01D30; /* 0.05 (the module's data, still assembly) */
extern void *effect_mesh_create(s16 x, s16 y, s16 z, s32 arg3, s32 arg4, f32 arg5, f32 arg6, s32 arg7, s32 arg8,
                                s32 arg9); /* a flare (freed with effect_texture_pair_free) */

void func_m106_00000(Object *obj, M106Setup *setup) {
    f32 zero = 0.0f;
    M106Tank *t;
    s16 x;

    obj->velocity.x = zero;
    obj->velocity.y = zero;
    obj->velocity.z = zero;
    t = obj->unk68;
    obj->trans.scale = setup->scale * rl_m106_01D30;
    if (game_flag_get(0x59)) {
        t->unk0 = 5;
    } else {
        t->unk0 = 0;
    }
    t->unk1 = 0;
    t->unk2 = 0;
    t->effect = NULL;
    t->unk20 = zero;
    t->child = NULL;
    t->unk28 = 0;
    t->unk2A = 0;
    t->unk35 = 0;
    t->unk2C = zero;
    t->unk30 = zero;
    t->unk34 = 1;
    t->unk36 = 0;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    t->u38.word = 0;
    t->unk3A = func_80048E2C(-0x80, 0x7F);
    x = zero;
    t->flare1 = effect_mesh_create(x, x, obj->trans.scale * -68.0f, 0, -0x4000, 18.0f, 75.0f, 0, 0x11, 0x12);
    t->unk3B = func_80048E2C(-0x80, 0x7F);
    t->flare2 = effect_mesh_create(x, x, obj->trans.scale * 70.0f, 0, -0x4000, 24.0f, 100.0f, 0, 0x11, 0x12);
    t->sound1 = 0;
    t->sound2 = 0;
    t->sound3 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m106", func_m106_00290);

extern void func_80049DC0(ObjectTransform *trans, MtxF *out); /* the transform's matrix */
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern f32 get_distance_to_active_camera(f32 x, f32 y, f32 z);
extern void mtx_model_push(Gfx **dl, Mtx **mtx, MtxF *transform);
extern void mtx_pop(Gfx **dl);
extern void func_80069BE8(Gfx **dl, void *flare, s32 arg2, s32 alpha);
extern void func_m081_006F8(Gfx **dl, Mtx **mtx, void *effect, s16 *rotation, Vec3f *pos, MtxF *transform);

/* Draws the tank's two flares and its effect, the farthest from the camera first */
void func_m106_01694(Gfx **dl, Mtx **mtx, Object *obj) {
    M106Tank *t = obj->unk68;
    s32 n;
    s32 i;
    s32 j;
    s32 kind[3];
    f32 x;
    f32 y;
    f32 z;
    s32 k;
    MtxF m;
    s16 rot[3];
    f32 dist[3];
    ObjectTransform trans;
    f32 tmp;

    trans.rotation.y_rotation = obj->trans.rotation.y_rotation;
    trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
    trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
    trans.scale = 1.0f;
    trans.x_position = obj->trans.x_position;
    trans.y_position = obj->trans.y_position;
    trans.z_position = obj->trans.z_position;
    func_80049DC0(&trans, &m);
    n = 0;
    if (t->flare1 != NULL) {
        func_80048B60(&m, 0.0f, 0.0f, obj->trans.scale * -68.0f, &x, &y, &z);
        dist[0] = get_distance_to_active_camera(x, y, z);
        kind[0] = 0;
        n = 1;
    }
    if (t->flare2 != NULL) {
        func_80048B60(&m, 0.0f, 0.0f, obj->trans.scale * 70.0f, &x, &y, &z);
        dist[n] = get_distance_to_active_camera(x, y, z);
        kind[n] = 1;
        n++;
    }
    if (t->effect != NULL) {
        dist[n] = get_distance_to_active_camera(t->effectPos.x, t->effectPos.y, t->effectPos.z);
        kind[n] = 2;
        n++;
    }
    for (i = n - 1; i > 0; i--) {
        for (j = 0; j < i; j++) {
            if (dist[j] < dist[j + 1]) {
                tmp = dist[j];
                dist[j] = dist[j + 1];
                dist[j + 1] = tmp;
                k = kind[j];
                kind[j] = kind[j + 1];
                kind[j + 1] = k;
            }
        }
    }
    for (i = 0; i < n;) {
        switch (kind[i++]) {
            case 0:
                mtx_model_push(dl, mtx, &m);
                func_80069BE8(dl, t->flare1, 6, t->u38.b.flareAlpha);
                mtx_pop(dl);
                break;
            case 1:
                mtx_model_push(dl, mtx, &m);
                func_80069BE8(dl, t->flare2, 6, t->u38.b.flareAlpha);
                mtx_pop(dl);
                break;
            case 2:
                rot[0] = 0;
                rot[1] = 0x4000;
                rot[2] = 0;
                func_m081_006F8(dl, mtx, t->effect, rot, &t->effectPos, &m);
                break;
        }
    }
}

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
