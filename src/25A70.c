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

typedef struct ObjProperties_SkyTrigger {
    /* 0x00 */ s32 setting;
    /* 0x04 */ s32 radius;
} ObjProperties_SkyTrigger;

typedef struct ObjProperties_Common {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
} ObjProperties_Common;

typedef union ObjectProperties {
    ObjProperties_Common common;
    ObjProperties_Timer timer;
    ObjProperties_SkyTrigger skyTrigger;
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
    /* 0x2C */ u8 pad2C[2];
    /* 0x2E */ s16 segmentID;
    /* 0x30 */ u8 pad30[0x3C - 0x30];
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

/* DKR obj_init_torch_mist (object_functions.c), plus JFG's float at 0x28 from byte 0xC (0x80024E70). */
void obj_init_torch_mist(Object *obj, LevelObjectEntry_Torch_Mist *entry) {
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

#ifdef NON_MATCHING
/* NON_MATCHING: frame 0x30 vs 0x38 (flags at 0x34, prevFrame at 0x2C in the target) and the load order */
/* JFG: animate the sprite (mode 9); free the object when its animation wrapped. */
void func_80024F94(u8 *obj, s32 updateRate) {
    s32 flags = 9;
    f32 prevFrame;

    prevFrame = *(f32 *) (obj + 0x28);
    func_800577A8(**(void ***) (obj + 0x6C), &flags, *(void **) (obj + 0x88), obj + 0x28, updateRate);
    if (*(f32 *) (obj + 0x28) < prevFrame) {
        free_object((Object *) obj);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/25A70", func_80024F94);
#endif

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

/* JFG: sky control: when a player is within the radius, set the skydome visibility (0x80026CAC). */
void obj_loop_skycontrol(Object *obj, UNUSED s32 updateRate) {
    Object *objs[8];

    if (obj_dist_racer(obj->x_position, obj->y_position, obj->z_position, obj->properties.skyTrigger.radius, 0,
                       objs)) {
        set_skydome_visbility(obj->properties.skyTrigger.setting);
    }
}

void func_80013924(s32 segmentId, s32 xSpeed, s32 ySpeed, s32 updateRate);

/* JFG: a texture animator loop (DKR obj_loop_animator, the update rate passed instead of the batch). */
void obj_loop_texture_animator(Object *obj, s32 updateRate) {
    Object_Animator *animator;
    s32 t0;
    s32 t1;

    animator = obj->animator;
    t0 = animator->speedFactorX * updateRate;
    t0 += animator->xSpeed;
    animator->xSpeed = t0 & 0x3;
    t0 >>= 2;
    t1 = animator->speedFactorY * updateRate;
    t1 += animator->ySpeed;
    animator->ySpeed = t1 & 0x3;
    t1 >>= 2;
    func_80013924(animator->segmentId, t0, t1, updateRate);
}

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

/* JFG: an object in a chain (next at 0xA8) */
typedef struct ChainObject {
    /* 0x00 */ u8 pad0[0x93];
    /* 0x93 */ u8 linked[0xA0 - 0x93]; // per player: linked into that player's slots
    /* 0xA0 */ u8 unkA0;
    /* 0xA1 */ u8 padA1[0xA8 - 0xA1];
    /* 0xA8 */ struct ChainObject *next;
} ChainObject;

/* JFG: six slots in a player's state block (the object's 0x68 block) */
typedef struct ChainSlots {
    /* 0x000 */ u8 pad0[0x1A8];
    /* 0x1A8 */ ChainObject *heads[6];
    /* 0x1C0 */ u8 ids[6];
} ChainSlots;

Object **get_racer_objects(s32 *numRacers);

/* JFG: remove an object from the slots of every player it is linked to; the slot gets the next lower
 * occupied one, which is cleared (id 0xFF). */
void chain_object_unlink_from_players(ChainObject *obj) {
    s32 count;
    Object **racers = get_racer_objects(&count);
    Object *racer;
    ChainSlots *slots;
    s32 i;
    s32 j;

    while (count--) {
        if (obj->linked[count] != 0) {
            racer = racers[count];
            slots = (ChainSlots *) racer->animator;
            i = 6;
            while (i--) {
                if (slots->heads[i] == obj) {
                    if (i == 0) {
                        slots->heads[i] = NULL;
                        slots->ids[i] = 0xFF;
                    } else {
                        j = i - 1;
                        while (j--) {
                            if (slots->heads[j] == NULL) {
                                break;
                            }
                        }
                        slots->heads[i] = slots->heads[j + 1];
                        slots->ids[i] = slots->ids[j + 1];
                        slots->heads[j + 1] = NULL;
                        slots->ids[j + 1] = 0xFF;
                    }
                }
            }
        }
        obj->linked[count] = 0;
    }
}

/* JFG: of the six slots, the free head (unkA0 == 0) with the shortest chain; its id goes to *id. */
ChainObject *chain_slot_find_shortest(Object *obj, u8 *id) {
    ChainSlots *slots = (ChainSlots *) obj->animator;
    s32 best = 0x7FFFFFFF;
    ChainObject *found = NULL;
    s32 i = 5;
    ChainObject *head;
    ChainObject *next;
    s32 count;
    u8 bestId;

    do {
        head = slots->heads[i];
        if (head != NULL && head->unkA0 == 0) {
            count = 0;
            for (next = head->next; next != NULL; next = next->next) {
                count++;
            }
            if (count < best) {
                bestId = slots->ids[i];
                best = count;
                found = head;
            }
        }
    } while (i--);
    *id = bestId;
    return found;
}

void func_80026FD4(s32 arg0, s32 arg1) {
}

typedef struct ModelSwapState {
    /* 0x00 */ u8 pad0[0x4C];
    /* 0x4C */ s16 objectId;
    /* 0x4E */ s16 list[1];
} ModelSwapState;

#ifdef NON_MATCHING
/* NON_MATCHING: registers: the target keeps the id in v1 and the list pointer in a1 */
/* JFG: init: keep the entry's id; some ids clear unk4, the others set it to 8; three ids get a list of three
 * (value, 0x20) pairs; the list ends with 0x1000. */
void func_80026FE0(Object *obj, s16 *entry) {
    ModelSwapState *state = (ModelSwapState *) obj->animator;
    s16 *list;
    s16 *p;

    state->objectId = entry[0];
    list = state->list;
    if (state->objectId == 0x204 || state->objectId == 0x1F5 || state->objectId == 0x1F6 ||
        state->objectId == 0x1D2 || state->objectId == 0x2B9 || state->objectId == 0x2BA ||
        state->objectId == 0x320 || state->objectId == 0x31E || state->objectId == 0x31F ||
        state->objectId == 0x2A5 || state->objectId == 0x299 || state->objectId == 0x28A ||
        state->objectId == 0x32C) {
        obj->properties.common.unk4 = 0;
    } else {
        obj->properties.common.unk4 = 8;
    }
    p = list;
    if (state->objectId == 0xC4 || state->objectId == 0x1B6 || state->objectId == 0x11A) {
        *p++ = 0x405A;
        *p++ = 0x20;
        *p++ = 0x405C;
        *p++ = 0x20;
        *p++ = 0x405E;
        *p++ = 0x20;
    }
    *p = 0x1000;
}
#else
INCLUDE_ASM("asm/nonmatchings/25A70", func_80026FE0);
#endif

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

/* JFG: what func_80018808 writes back (0x2C bytes) */
typedef struct HitResult {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 unk10[5];
    /* 0x24 */ s32 flags;
    /* 0x28 */ f32 unk28;
} HitResult;

s32 func_80018808(s32 arg0, f32 *from, f32 *to, HitResult *result, s32 flags, s32 arg5);
void move_object(Object *obj, f32 xPos, f32 yPos, f32 zPos);
extern f32 D_800ABAF8; // gravity

/* JFG: loop: fall along the velocity; every unkE frames test the path since the last check, and on a hit put
 * the object there, emit particles by surface type and free it. */
void obj_loop_falling_spark(Object *obj, s32 updateRate) {
    HitResult result;
    MoverState *state;
    s32 flags;

    state = (MoverState *) obj->animator;
    move_object(obj, obj->x_velocity * updateRate, obj->y_velocity * updateRate, obj->z_velocity * updateRate);
    obj->y_velocity -= D_800ABAF8 * updateRate;
    state->unkC++;
    if (state->unkC >= state->unkE) {
        if (func_80018808(0, &state->x, &obj->x_position, &result, 0, 0)) {
            flags = result.flags;
            if (flags & 0x8000) {
                obj->particleEmittersEnabled = 1;
            } else if (flags & 0x2000) {
                obj->particleEmittersEnabled = 6;
            } else {
                obj->particleEmittersEnabled = 4;
            }
            obj->x_position = result.pos[0];
            obj->y_position = result.pos[1];
            obj->z_position = result.pos[2];
            obj->segmentID = get_level_segment_index_from_position(obj->x_position, obj->y_position, obj->z_position);
            obj_spawn_particle(obj, 2);
            free_object(obj);
        }
        state->unkC = 0;
        state->x = obj->x_position;
        state->y = obj->y_position;
        state->z = obj->z_position;
    }
}

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

s32 rand_range(s32 min, s32 max);
/* The module trap (0x80054530) under a prototype with float arguments: needs
   `func_80054530_f8 = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_f8(f32 speed, f32 x, f32 y, f32 z, f32 arg4, f32 vx, f32 vy, f32 vz);
extern f32 D_800ABB04;
extern f32 D_800ABB08;

#ifdef NON_MATCHING
/* NON_MATCHING: frame 0x80 vs 0x78 (angle/vel 0x10 higher) and the two constants swap $f22/$f24 */
/* JFG: loop: while unk0 counts down, send three random sparks (through a module) per frame, then emit
 * particles; free the object when unk4 runs out. */
void func_80028034(Object *obj, s32 updateRate) {
    s16 angle[2];
    f32 vel[3];
    s32 i;
    f32 speed;
    f32 kVel;
    f32 kSpeed;

    if (obj->properties.common.unk0 != 0) {
        obj->properties.common.unk0--;
        kVel = D_800ABB04;
        kSpeed = D_800ABB08;
        for (i = 0; i < 3; i++) {
            speed = rand_range(75, 125) * kSpeed;
            angle[0] = rand_range(-0x8000, 0x7FFF);
            angle[1] = rand_range(0x2400, 0x3C00);
            vel[2] = rand_range(25, 50) * kVel;
            func_8004944C(angle, vel);
            func_80054530_f8(speed, obj->x_position, obj->y_position, obj->z_position, obj->y_position, vel[0],
                             vel[1], vel[2]);
        }
        if (obj->properties.common.unk0 == 0) {
            obj_spawn_particle(obj, updateRate);
        }
    }
    obj->properties.common.unk4 -= updateRate;
    if (obj->properties.common.unk4 <= 0) {
        free_object(obj);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/25A70", func_80028034);
#endif

typedef struct LevelObjectEntry_Drift {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 angle;
    /* 0x0C */ f32 speed;
    /* 0x10 */ f32 unk10;
} LevelObjectEntry_Drift;

typedef struct DriftState {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
} DriftState;

f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
s32 rand_range(s32 min, s32 max);
extern f32 D_800ABB0C;
extern f32 D_800ABB10;

#ifdef NON_MATCHING
/* NON_MATCHING: the target loads D_800ABB0C before reloading the state pointer and the float registers differ */
/* JFG: init: horizontal velocity along the entry's angle, a random scale factor. */
void func_800281B4(Object *obj, LevelObjectEntry_Drift *entry) {
    f32 r;
    DriftState *state = (DriftState *) obj->animator;
    f32 k;
    f32 s;

    obj->x_velocity = func_80049C34(entry->angle) * entry->speed;
    obj->y_velocity = 0.0f;
    obj->z_velocity = func_80049C30(entry->angle) * entry->speed;
    r = rand_range(0x4B, 0x7D);
    k = D_800ABB0C;
    s = r * D_800ABB10;
    state->unk4 = k;
    state->unk8 = s;
    state->unk0 = obj->scale * s;
    state->unkC = entry->unk10;
    obj->scale = state->unk0 * k;
}
#else
INCLUDE_ASM("asm/nonmatchings/25A70", func_800281B4);
#endif

INCLUDE_ASM("asm/nonmatchings/25A70", func_80028274);

void func_80028414(Object *obj, s8 *entry) {
    obj->properties.timer.unk0 = entry[0xA] * 60;
    obj->properties.timer.unk2 = entry[0xB] * 60;
    obj->properties.timer.unk4 = 0;
    obj->properties.timer.unk6 = 0xFF;
}

void sound_play(u16 soundID, s32 *handlePtr);
void func_80054530();

/* JFG: a pickup-like trigger: fades in, then the first player within 40 units without one gets its value
 * (0x10E of the player's state) and calls into a module; it then respawns after unk2 frames, or is freed. */
void obj_loop_pickup_trigger(Object *obj, s32 updateRate) {
    s32 count;
    s32 i;
    Object *racers[8];
    ObjProperties_Timer *timer;
    u8 *state;

    timer = &obj->properties.timer;
    count = obj_dist_racer(obj->x_position, obj->y_position, obj->z_position, 40.0f, 0, racers);
    if (timer->unk6 == 0) {
        timer->unk4 -= updateRate;
        if (timer->unk4 <= 0) {
            timer->unk4 = 0;
            if (count == 0) {
                timer->unk6 = updateRate;
            }
        }
    } else {
        timer->unk6 += updateRate * 4;
        if (timer->unk6 >= 256) {
            timer->unk6 = 255;
        }
        if (count != 0) {
            for (i = 0; i < count; i++) {
                state = (u8 *) racers[i]->animator;
                if (*(s16 *) (state + 0x10E) == 0) {
                    *(s16 *) (state + 0x10E) = timer->unk0;
                    timer->unk4 = timer->unk2;
                    if (timer->unk4 != 0) {
                        timer->unk6 = 0;
                    } else {
                        free_object(obj);
                    }
                    sound_play(0x8A, NULL);
                    func_80054530(state, 0x88);
                }
            }
        }
    }
}
