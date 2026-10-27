/*
 * Object behaviours. Functions in C are from the Diddy Kong Racing
 * decompilation (CC0 1.0), src/object_functions.c; see CREDITS.md.
 */
#include "common.h"

#define OBJ_EMIT_NONE 0

/* DKR include/structs.h, reduced to the fields used here (JFG offsets) */
typedef struct LevelObjectEntry_RangeTrigger {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 radius;
    /* 0x0C */ u16 particleFlags;
} LevelObjectEntry_RangeTrigger;

typedef union LevelObjectEntry {
    LevelObjectEntry_RangeTrigger rangeTrigger;
} LevelObjectEntry;

typedef struct LevelObjectEntry_Animator {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s8 batchID;
    /* 0x0B */ s8 speedFactorX;
    /* 0x0C */ s8 speedfactorY;
} LevelObjectEntry_Animator;

typedef struct Object_Animator {
    /* 0x00 */ s16 segmentId;
    /* 0x02 */ s16 batchId;
    /* 0x04 */ s16 speedFactorX;
    /* 0x06 */ s16 speedFactorY;
    /* 0x08 */ s16 xSpeed;
    /* 0x0A */ s16 ySpeed;
} Object_Animator;

/* Size: 0x48 bytes */
typedef struct LevelModelSegment {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ s16 numberOfBatches;
    /* 0x2A */ u8 pad2A[0x48 - 0x2A];
} LevelModelSegment;

typedef struct LevelModel {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ LevelModelSegment *segments;
} LevelModel;

typedef struct ObjProperties_Timer {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
} ObjProperties_Timer;

typedef struct ObjProperties_SkyControl {
    /* 0x00 */ s32 setting;
    /* 0x04 */ s32 radius;
} ObjProperties_SkyControl;

typedef struct ObjProperties_Common {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
} ObjProperties_Common;

typedef union ObjectProperties {
    ObjProperties_Common common;
    ObjProperties_Timer timer;
    ObjProperties_SkyControl skyControl;
} ObjectProperties;

typedef struct ObjectHeader {
    /* 0x00 */ f32 scale;
} ObjectHeader;

typedef struct Object {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
    /* 0x18 */ u8 pad18[0x1C - 0x18];
    /* 0x1C */ f32 x_velocity;
    /* 0x20 */ f32 y_velocity;
    /* 0x24 */ f32 z_velocity;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x3C - 0x2C];
    /* 0x3C */ LevelObjectEntry *level_entry;
    /* 0x40 */ ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x68 - 0x44];
    /* 0x68 */ Object_Animator *animator;
    /* 0x6C */ u8 pad6C[0x84 - 0x6C];
    /* 0x84 */ s32 particleEmittersEnabled;
    /* 0x88 */ ObjectProperties properties;
} Object;

LevelModel *get_current_level_model(void);
s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos);
void obj_loop_animator(Object *obj, s32 updateRate);

s32 obj_dist_racer(f32 x, f32 y, f32 z, f32 radius, s32 is2dCheck, Object **sortObj);
void obj_spawn_particle(Object *obj, s32 updateRate);

typedef struct LevelObjectEntry_Torch_Mist {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u8 animSpeed;
    /* 0x0B */ u8 radius;
    /* 0x0C */ u8 unkC;
} LevelObjectEntry_Torch_Mist;

/* DKR obj_init_torch_mist (object_functions.c), plus JFG's float at 0x28 from byte 0xC. */
void func_80024E70(Object *obj, LevelObjectEntry_Torch_Mist *entry) {
    f32 radius = entry->radius & 0xFF;
    if (radius < 10.0f) {
        radius = 10.0f;
    }
    radius /= 64;
    obj->scale = obj->header->scale * radius;
    obj->properties.common.unk0 = entry->animSpeed;
    obj->unk28 = entry->unkC;
}

void func_800577A8(void *sprite, s32 *arg1, void *arg2, void *arg3, s32 arg4); // JFG: animate a sprite

/* JFG: animate the object's sprite (mode 9) by the update rate. */
void func_80024EF4(u8 *obj, s32 updateRate) {
    s32 flags = 9;

    func_800577A8(**(void ***) (obj + 0x6C), &flags, *(void **) (obj + 0x88), obj + 0x28, updateRate);
}

/* JFG: init: scale from byte 0xB (DKR's radius scaling, see obj_init_torch_mist). */
void func_80024F40(Object *obj, LevelObjectEntry_Torch_Mist *entry) {
    f32 radius = entry->radius & 0xFF;
    if (radius < 10.0f) {
        radius = 10.0f;
    }
    radius /= 64;
    obj->scale = obj->header->scale * radius;
}

void free_object(Object *object);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80024F94);

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002500C);

/**
 * Object animator init function.
 * Sets up the initial properties of the animator, then calls the loop function once.
 * Official name: animInit
 */
void obj_init_animator(Object *obj, LevelObjectEntry_Animator *entry, s32 param) {
    Object_Animator *animator;
    LevelModel *levelModel;
    s16 segmentBatchCount;

    animator = obj->animator;
    levelModel = get_current_level_model();
    animator->batchId = entry->batchID;
    animator->speedFactorX = entry->speedFactorX;
    animator->speedFactorY = entry->speedfactorY;
    animator->segmentId = get_level_segment_index_from_position(obj->x_position, obj->y_position, obj->z_position);
    // Always true.
    if (param == 0) {
        animator->xSpeed = 0;
        animator->ySpeed = 0;
    }
    if (animator->segmentId != -1) {
        if (animator->batchId < 0) {
            animator->batchId = 0;
        }
        segmentBatchCount = levelModel->segments[animator->segmentId].numberOfBatches;
        if (animator->batchId >= segmentBatchCount) {
            animator->batchId = segmentBatchCount - 1;
        }
    }
    obj_loop_animator(obj, 0x20000);
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_800253F0);

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002561C);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80025A08);

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002662C);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026824);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026A50);

void set_skydome_visbility(s32 renderSky);

/* JFG: sky control: when a player is within the radius, set the skydome visibility. */
void func_80026CAC(Object *obj, UNUSED s32 updateRate) {
    Object *objs[8];

    if (obj_dist_racer(obj->x_position, obj->y_position, obj->z_position, obj->properties.skyControl.radius, 0,
                       objs)) {
        set_skydome_visbility(obj->properties.skyControl.setting);
    }
}

void func_80013924(s32 arg0, s32 arg1, s32 arg2); // JFG: queue a texture scroll

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026D14);

/**
 * Particle Emitter loop func.
 * Emits particles set by the spawn parameters when any races are in range.
 * Official name: rangetriggerControl
 */
void obj_loop_rangetrigger(Object *obj, s32 updateRate) {
    UNUSED s32 pad;
    LevelObjectEntry_RangeTrigger *entry;
    Object *objs[8];

    entry = &obj->level_entry->rangeTrigger;
    if (obj_dist_racer(obj->x_position, 0, obj->z_position, entry->radius, 1, objs) > 0) {
        obj->particleEmittersEnabled = entry->particleFlags;
    } else {
        obj->particleEmittersEnabled = OBJ_EMIT_NONE;
    }
    obj_spawn_particle(obj, updateRate);
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026E20);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026F38);

void func_80026FD4(s32 arg0, s32 arg1) {
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026FE0);

void func_800270C8(s32 arg0, s32 arg1) {
}

void func_800270D4(Object *obj, s32 arg1) {
    s16 *speed = (s16 *) obj->animator + 2;
    *speed = 0x1000;
}

typedef struct AnimDroidState {
    /* 0x00 */ s32 angle;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
} AnimDroidState;

void func_800270EC(Object *obj, s32 updateRate) {
    AnimDroidState *state = (AnimDroidState *) obj->animator;
    state->angle += updateRate * 0x4800;
    state->unk4 = 0x2008, state->unk6 = state->angle;
    state->unk8 = 0x1000;
}

void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, void **handlePtr);

/* JFG: an animation event handler: events 1 and 2 play sounds 0xDD and 0xDE at the object. */
void func_80027120(f32 *obj, s32 event) {
    switch (event) {
        case 1:
            audspat_play_sound_at_position(0xDD, obj[3], obj[4], obj[5], 4, NULL);
            return;
        case 2:
            audspat_play_sound_at_position(0xDE, obj[3], obj[4], obj[5], 4, NULL);
            return;
    }
}

/* JFG: the same handler for another object type. */
void func_800271A0(f32 *obj, s32 event) {
    switch (event) {
        case 1:
            audspat_play_sound_at_position(0xDD, obj[3], obj[4], obj[5], 4, NULL);
            return;
        case 2:
            audspat_play_sound_at_position(0xDE, obj[3], obj[4], obj[5], 4, NULL);
            return;
    }
}

/* JFG: the same handler with events 3 and 4 as well. */
void func_80027220(f32 *obj, s32 event) {
    switch (event) {
        case 1:
        case 3:
            audspat_play_sound_at_position(0xDD, obj[3], obj[4], obj[5], 4, NULL);
            return;
        case 2:
        case 4:
            audspat_play_sound_at_position(0xDE, obj[3], obj[4], obj[5], 4, NULL);
            return;
    }
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_800272B4);

void func_80079D1C(Object *obj, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, f32 arg7, f32 arg8,
                   f32 arg9, f32 argA, s32 argB);

void func_800274FC(Object *obj, UNUSED s32 arg1) {
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 20.0f, 20.0f, -1.0f, 1.0f, 0x11);
}

void func_80027564(Object *obj, UNUSED s32 arg1) {
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 20.0f, 20.0f, -1.0f, 1.0f, 0x11);
}

void func_800275CC(Object *obj, u8 *entry) {
    u8 *state = (u8 *) obj->animator;
    state[0] = entry[0xA];
    state[1] = entry[0xB];
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_800275E8);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027844);

typedef struct BufferState {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ void *buffers[4];
} BufferState;

void mempool_free(void *data);

/* JFG: free the four buffers of the object's state. */
void func_80027D08(Object *obj) {
    BufferState *state = (BufferState *) obj->animator;

    if (state->buffers[0] != NULL) {
        mempool_free(state->buffers[0]);
    }
    if (state->buffers[1] != NULL) {
        mempool_free(state->buffers[1]);
    }
    if (state->buffers[2] != NULL) {
        mempool_free(state->buffers[2]);
    }
    if (state->buffers[3] != NULL) {
        mempool_free(state->buffers[3]);
    }
}

typedef struct LevelObjectEntry_Mover {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 rotation[2];
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 padF;
    /* 0x10 */ f32 speed;
} LevelObjectEntry_Mover;

typedef struct MoverState {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
} MoverState;

void func_8004944C(s16 *rotation, f32 *vec); // DKR: vec3f_rotate_py

/* JFG: init: remember the start position, then move along the entry's rotation. */
void func_80027DA0(Object *obj, LevelObjectEntry_Mover *entry) {
    MoverState *state = (MoverState *) obj->animator;

    state->x = obj->x_position;
    state->y = obj->y_position;
    state->z = obj->z_position;
    obj->z_velocity = -entry->speed;
    func_8004944C(entry->rotation, &obj->x_velocity);
    state->unkC = 0;
    state->unkE = entry->unkE;
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027E1C);

typedef struct LevelObjectEntry_Spray {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 angle;
    /* 0x0C */ f32 size;
    /* 0x10 */ f32 unk10;
} LevelObjectEntry_Spray;

extern f32 D_800ABAFC;
extern f32 D_800ABB00;

/* JFG: init: emitter settings from the entry's (absolute) size. */
void func_80027F90(Object *obj, LevelObjectEntry_Spray *entry) {
    f32 size = entry->size;

    if (size < 0.0f) {
        size = -size;
    }
    obj->properties.common.unk0 = (s32) (size * D_800ABAFC) + 2;
    obj->properties.common.unk4 = 0x23;
    obj->scale *= (15.0f + size) * D_800ABB00;
    obj->particleEmittersEnabled = 1;
    obj->y_velocity = size * 0.25f + 2.5f;
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_80028034);

typedef struct SprayState {
    /* 0x00 */ f32 scale;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
} SprayState;

f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
s32 rand_range(s32 min, s32 max);
extern f32 D_800ABB0C;
extern f32 D_800ABB10;

INCLUDE_ASM("asm/nonmatchings/25A70", func_800281B4);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80028274);

void func_80028414(Object *obj, s8 *entry) {
    obj->properties.timer.unk0 = entry[0xA] * 60;
    obj->properties.timer.unk2 = entry[0xB] * 60;
    obj->properties.timer.unk4 = 0;
    obj->properties.timer.unk6 = 0xFF;
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002844C);
