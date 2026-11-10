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

/* DKR TexCoords / Triangle (JFG: 0x10 bytes) */
typedef struct LevelTexCoords {
    /* 0x00 */ s16 u;
    /* 0x02 */ s16 v;
} LevelTexCoords;

typedef struct LevelTriangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 pad1[3];
    /* 0x04 */ LevelTexCoords uv0;
    /* 0x08 */ LevelTexCoords uv1;
    /* 0x0C */ LevelTexCoords uv2;
} LevelTriangle;

/* DKR TriangleBatchInfo (JFG: 0x10 bytes) */
typedef struct LevelBatchInfo {
    /* 0x00 */ u8 textureIndex;
    /* 0x01 */ u8 pad1[7];
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[6];
} LevelBatchInfo;

typedef struct LevelTextureHeader {
    /* 0x00 */ u8 width;
} LevelTextureHeader;

typedef struct LevelTextureInfo {
    /* 0x00 */ LevelTextureHeader *texture;
    /* 0x04 */ s32 unk4;
} LevelTextureInfo;

/* Size: 0x48 bytes */
typedef struct LevelModelSegment {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ LevelTriangle *triangles;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ LevelBatchInfo *batches;
    /* 0x10 */ u8 pad10[0x28 - 0x10];
    /* 0x28 */ s16 numberOfBatches;
    /* 0x2A */ u8 pad2A[0x48 - 0x2A];
} LevelModelSegment;

typedef struct LevelModel {
    /* 0x00 */ LevelTextureInfo *textures;
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
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 flags;
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
    /* 0x30 */ u8 pad30[0x3A - 0x30];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ LevelObjectEntry *level_entry;
    /* 0x40 */ ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x68 - 0x44];
    /* 0x68 */ Object_Animator *animator;
    /* 0x6C */ struct ModelInstance26 **models;
    /* 0x70 */ u8 pad70[0x84 - 0x70];
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

void sprite_anim_advance(void *sprite, s32 *arg1, void *arg2, void *arg3, s32 arg4); // JFG: animate a sprite

/* JFG: animate the object's sprite (mode 9) by the update rate. */
void obj_sprite_animate(u8 *obj, s32 updateRate) {
    s32 flags = 9;

    sprite_anim_advance(**(void ***) (obj + 0x6C), &flags, *(void **) (obj + 0x88), obj + 0x28, updateRate);
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

/* JFG: animate the sprite (mode 9); free the object when its animation wrapped. */
void sprite_anim_once_update(u8 *obj, s32 updateRate) {
    s32 flags = 9;
    void *sprite;
    f32 prevFrame;

    sprite = **(void ***) (obj + 0x6C);
    prevFrame = *(f32 *) (obj + 0x28);
    sprite_anim_advance(sprite, &flags, *(void **) (obj + 0x88), obj + 0x28, updateRate);
    if (*(f32 *) (obj + 0x28) < prevFrame) {
        free_object((Object *) obj);
    }
}

/* JFG: a box zone's level object entry */
typedef struct LevelObjectEntry_BoxZone {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u8 width;
    /* 0x0B */ u8 height;
    /* 0x0C */ u8 depth;
    /* 0x0D */ u8 angle;
    /* 0x0E */ u8 mode;
    /* 0x0F */ u8 value;
} LevelObjectEntry_BoxZone;

/* JFG: a racer's data, reduced */
typedef struct RacerZoneData {
    /* 0x000 */ u8 pad0[0x194];
    /* 0x194 */ u8 zoneMode;
    /* 0x195 */ u8 zoneValue;
} RacerZoneData;

Object **get_racer_objects(s32 *numRacers);
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle

#ifdef NON_MATCHING
/* JFG: loop: give each racer inside the (turned) box the zone's mode and value; in mode 1 the value
   fades out over the top half of the box.
   NON_MATCHING: 25 words: dy and dz get $f12/$f2 swapped, and the final multiply's operands swap. */
void func_8002500C(Object *obj, UNUSED s32 updateRate) {
    Object **racers;
    LevelObjectEntry_BoxZone *entry;
    s32 numRacers;
    s32 i;
    Object *racer;
    RacerZoneData *data;
    f32 cosA;
    f32 sinA;
    f32 width;
    f32 height;
    f32 depth;
    f32 dx;
    f32 dz;
    f32 dy;
    f32 lx;

    entry = (LevelObjectEntry_BoxZone *) obj->level_entry;
    racers = get_racer_objects(&numRacers);
    cosA = func_80049C30(-(entry->angle << 8));
    sinA = func_80049C34(-(entry->angle << 8));
    width = entry->width * 3;
    height = entry->height * 3;
    depth = entry->depth * 3;
    for (i = 0; i < numRacers; i++) {
        racer = racers[i];
        dx = racer->x_position - obj->x_position;
        dy = racer->y_position - obj->y_position;
        dz = racer->z_position - obj->z_position;
        if (-height < dy && dy < height) {
            lx = dx * cosA + dz * sinA;
            if (-width < lx && lx < width) {
                dz = -dx * sinA + dz * cosA;
                if (-depth < dz && dz < depth) {
                    dz = height / (f32) 2; // now half the height
                    data = (RacerZoneData *) racer->animator;
                    data->zoneMode = entry->mode;
                    data->zoneValue = entry->value;
                    if (dz < dy && data->zoneMode == 1) {
                        data->zoneValue = (1.0f - (dy - dz) / dz) * data->zoneValue;
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/25A70", func_8002500C);
#endif

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

/**
 * Object animator loop function.
 * Scrolls the texture coordinates of a batch of the level model's triangles.
 * DKR obj_loop_animator (object_functions.c).
 */
void obj_loop_animator(Object *obj, s32 updateRate) {
    s32 pad[2];
    Object_Animator *animator;
    LevelModel *levelModel;
    s32 trisStart;
    s32 trisEnd;
    s32 textureIndex;
    s32 tri;
    s32 shift; // a1
    s32 t0;
    s32 t1;
    LevelTriangle *curTriangle;
    LevelBatchInfo *curBatch;
    LevelModelSegment *curBlock;
    s32 shift2;
    LevelTextureHeader *tex;

    animator = obj->animator;

    t0 = animator->speedFactorX;
    t1 = animator->speedFactorY;

    t0 *= updateRate;
    t1 *= updateRate;

    t0 <<= 4;
    t1 <<= 4;

    animator->xSpeed += t0;
    animator->ySpeed += t1;

    trisStart = animator->xSpeed;
    t0 = trisStart;
    trisEnd = animator->ySpeed;
    t1 = trisEnd;

    animator->xSpeed &= 0xF;
    animator->ySpeed &= 0xF;

    t0 >>= 4;
    t1 >>= 4;

    if (animator->segmentId == -1) {
        return;
    }

    levelModel = get_current_level_model();
    curBlock = &levelModel->segments[animator->segmentId];
    curBatch = &curBlock->batches[animator->batchId];
    textureIndex = curBatch->textureIndex;
    trisStart = curBatch->facesOffset;
    trisEnd = (curBatch + 1)->facesOffset;

    if (textureIndex != 0xFF) {
        if (!curBatch) {} // Fake
        tex = levelModel->textures[textureIndex].texture;
        shift2 = tex->width << 7;
        shift = tex->width << 7;
        for (tri = trisStart; tri < trisEnd; tri++) {
            curTriangle = &curBlock->triangles[tri];
            if (!(curTriangle->flags & 0x80)) {
                if ((shift << 1) < curTriangle->uv0.v) {
                    curTriangle->uv0.v -= shift;
                    curTriangle->uv1.v -= shift;
                    curTriangle->uv2.v -= shift;
                }
                if (curTriangle->uv0.v < 0) {
                    curTriangle->uv0.v += shift;
                    curTriangle->uv1.v += shift;
                    curTriangle->uv2.v += shift;
                }
                if ((shift2 << 1) < curTriangle->uv0.u) {
                    curTriangle->uv0.u -= shift2;
                    curTriangle->uv1.u -= shift2;
                    curTriangle->uv2.u -= shift2;
                }
                if (curTriangle->uv0.u < 0) {
                    curTriangle->uv0.u += shift2;
                    curTriangle->uv1.u += shift2;
                    curTriangle->uv2.u += shift2;
                }
                curTriangle->uv0.v += t1;
                curTriangle->uv1.v += t1;
                curTriangle->uv2.v += t1;
                curTriangle->uv0.u += t0;
                curTriangle->uv1.u += t0;
                curTriangle->uv2.u += t0;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002561C);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80025A08);

u8 **objGetObjList(s32 *start, s32 *end);

/* JFG: for each object 7 of group `id`: count its 0x4A down; when it reaches 0, replace flag 8 of its 0x3E with
 * `flag`. */
void obj_group_countdown(s32 id, u16 flag) {
    u8 *obj;
    u8 *state;
    s32 i;
    s32 start;
    s32 end;
    u8 **list;

    list = objGetObjList(&start, &end);
    for (i = start; i < end; i++) {
        obj = list[i];
        if (*(s16 *) (obj + 0x48) == 7) {
            state = *(u8 **) (obj + 0x68);
            if (state[0x44] == id) {
                if (state[0x4A] > 0) {
                    state[0x4A]--;
                }
                if (state[0x4A] == 0) {
                    *(u16 *) (state + 0x3E) &= ~8;
                    *(u16 *) (state + 0x3E) |= flag;
                }
            }
        }
    }
}

/* JFG: the first object 7 of group `id` gets `flag` set in its 0x3E and its 0x4A counted up; FALSE if none. */
s32 obj_group_flag_first(s32 id, u16 flag) {
    u8 *obj;
    u8 *state;
    s32 i;
    s32 start;
    s32 end;
    u8 **list;

    list = objGetObjList(&start, &end);
    for (i = start; i < end; i++) {
        obj = list[i];
        if (*(s16 *) (obj + 0x48) == 7) {
            state = *(u8 **) (obj + 0x68);
            if (state[0x44] == id) {
                *(u16 *) (state + 0x3E) |= flag;
                state[0x4A]++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* JFG: a model instance, reduced */
typedef struct ModelInstance26 {
    /* 0x00 */ struct ModelHeader26 *header;
    /* 0x04 */ u8 pad4[0x74 - 0x4];
    /* 0x74 */ struct ModelBatch26 *batches;
} ModelInstance26;

typedef struct ModelHeader26 {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numBatches;
} ModelHeader26;

typedef struct ModelBatch26 {
    /* 0x00 */ s16 light;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s32 flags;
} ModelBatch26;

/* JFG: the state of an object lit by a group's flags */
typedef struct GroupLightState {
    /* 0x00 */ f32 level;
    /* 0x04 */ Object *group; // found near the object (obj_find_nearest_of_type)
    /* 0x08 */ u8 lit;
    /* 0x09 */ u8 module;
    /* 0x0A */ u8 timer; // 0xFF: none
    /* 0x0B */ u8 jingle;
    /* 0x0C */ u8 unkC;
} GroupLightState;

/* JFG: a group object's state, reduced (obj_group_flag_first) */
typedef struct GroupState26 {
    /* 0x00 */ u8 pad0[0x3E];
    /* 0x3E */ u16 flags;
} GroupState26;

Object *obj_find_nearest_of_type(f32 x, f32 y, f32 z, s32 type);
s32 normalise_time(s32 timer);
void func_80054530();
void music_jingle_play(s32 jingle);
void func_8001138C(Object *obj, f32 level, f32 updateRate);
extern f32 D_800ABAE8;
extern f32 D_800ABAEC;
extern f32 D_800ABAF0;

#ifdef NON_MATCHING
/* JFG: loop: find the group object nearby, then follow its flags: light up (setting a level, or a module
   effect with a jingle after a delay) and pass the lit state to the model's flagged batches.
   NON_MATCHING: scheduling in three places (the else arm's lit store, the module argument via v0, the
   loop reloads model->header after the batch store). */
void func_80026A50(Object *obj, s32 updateRate) {
    GroupLightState *state;
    GroupState26 *group;
    s32 lit;
    ModelInstance26 *model;
    ModelBatch26 *batch;
    s32 i;

    state = (GroupLightState *) obj->animator;
    lit = state->lit;
    if (state->group == NULL) {
        state->group = obj_find_nearest_of_type(obj->x_position, obj->y_position, obj->z_position, 7);
    } else {
        group = (GroupState26 *) state->group->animator;
        if (!(group->flags & 8)) {
            if (state->unkC) {
                if (group->flags & 5) {
                    lit = 1;
                    state->level = D_800ABAE8;
                } else {
                    state->level = D_800ABAEC;
                    lit = 1;
                }
            } else if (!(group->flags & 0x10)) {
                obj->unk28 = 1.0f;
                lit = 1;
                state->level = 0.0f;
            } else {
                if (state->timer == 0xFF) {
                    if (state->module) {
                        state->timer = normalise_time(30);
                    } else {
                        state->timer = 0;
                    }
                }
                if (state->module) {
                    func_80054530(state->module);
                    state->module = 0;
                    state->jingle = 1;
                }
                if (state->timer != 0xFF) {
                    if (updateRate < state->timer) {
                        state->timer -= updateRate;
                    } else {
                        if (state->jingle) {
                            music_jingle_play(15);
                            state->jingle = 0;
                        }
                        state->timer = 0;
                        lit = 1;
                        state->level = D_800ABAF0;
                    }
                }
            }
        }
    }
    func_8001138C(obj, state->level, updateRate);
    if (lit != state->lit) {
        model = obj->models[obj->modelIndex];
        batch = model->batches;
        if (batch != NULL) {
            for (i = 0; i < model->header->numBatches; i++) {
                if (batch->flags & 0x10000) {
                    batch->light = lit << 8;
                }
                batch++;
            }
        }
        state->lit = lit;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/25A70", func_80026A50);
#endif

void set_skydome_visbility(s32 renderSky);

/* JFG: sky control: when a player is within the radius, set the skydome visibility (0x80026CAC). */
void obj_loop_skycontrol(Object *obj, UNUSED s32 updateRate) {
    Object *objs[8];

    if (obj_dist_racer(obj->x_position, obj->y_position, obj->z_position, obj->properties.skyTrigger.radius, 0,
                       objs)) {
        set_skydome_visbility(obj->properties.skyTrigger.setting);
    }
}

void tex_scroll_queue(s32 segmentId, s32 xSpeed, s32 ySpeed, s32 updateRate);

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
    tex_scroll_queue(animator->segmentId, t0, t1, updateRate);
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

/* JFG: init: keep the entry's id; some ids clear unk4, the others set it to 8; three ids get a list of three
 * (value, 0x20) pairs; the list ends with 0x1000. (No `list` local: p is set from state->list where it starts.) */
void obj_init_model_swap(Object *obj, s16 *entry) {
    ModelSwapState *state = (ModelSwapState *) obj->animator;
    s16 *p;

    state->objectId = entry[0];
    if (state->objectId == 0x204 || state->objectId == 0x1F5 || state->objectId == 0x1F6 ||
        state->objectId == 0x1D2 || state->objectId == 0x2B9 || state->objectId == 0x2BA ||
        state->objectId == 0x320 || state->objectId == 0x31E || state->objectId == 0x31F ||
        state->objectId == 0x2A5 || state->objectId == 0x299 || state->objectId == 0x28A ||
        state->objectId == 0x32C) {
        obj->properties.common.unk4 = 0;
    } else {
        obj->properties.common.unk4 = 8;
    }
    p = state->list;
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

/* JFG: a trigger zone's level object entry */
typedef struct LevelObjectEntry_ZoneTrigger {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 enterRadius;
    /* 0x0C */ u16 leaveRadius;
    /* 0x0E */ u8 is2d;
    /* 0x0F */ u8 id;
    /* 0x10 */ u8 flags;
    /* 0x11 */ u8 undoOnLeave;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 soundID;
} LevelObjectEntry_ZoneTrigger;

s32 zone_sphere_touches(f32 x, f32 y, f32 z, f32 radius, s32 arg4);
void func_80076928(s32 id);
void func_80079BB8(void);
void anim_path_sound_stop(s32 id);
void func_800767EC(s32 id);
void sequence_start(s32 id);
void anim_path_sound_play(s32 id, s32 arg1);
void func_80079BA8(void);
void sound_play(u16 soundID, s32 *handlePtr);

/* JFG: loop: a trigger zone; entering (a racer within enterRadius, or zone_sphere_touches with flag 0x40) sets
   off entry->id one way (flags 0x80/0x20) and a sound, leaving undoes it if asked; flag 1 calls
   func_80079BA8 while inside. */
void obj_loop_zone_trigger(Object *obj, UNUSED s32 updateRate) {
    Object *racer[8]; // the frame holds 8 words here
    LevelObjectEntry_ZoneTrigger *entry;

    entry = (LevelObjectEntry_ZoneTrigger *) obj->level_entry;
    if (entry->flags & 0x40) {
        if (obj->properties.common.unk0 == 0) {
            if (zone_sphere_touches(obj->x_position, obj->y_position, obj->z_position, entry->enterRadius, 1)) {
                obj->properties.common.unk0 = 1;
            }
        }
    } else if (obj->properties.common.unk0 != 0) {
        if (!obj_dist_racer(obj->x_position, obj->y_position, obj->z_position, entry->leaveRadius, entry->is2d, racer)) {
            if (entry->undoOnLeave) {
                if (entry->flags & 0x80) {
                    func_80076928(entry->id);
                } else {
                    func_80079BB8();
                }
                if (entry->unk12) {
                    anim_path_sound_stop(entry->id);
                }
            }
            obj->properties.common.unk0 = 0;
        }
    } else if (obj_dist_racer(obj->x_position, obj->y_position, obj->z_position, entry->enterRadius, entry->is2d, racer)) {
        if (entry->flags & 0x80) {
            func_800767EC(entry->id);
        } else if (!(entry->flags & 0x20)) {
            sequence_start(entry->id);
        }
        if (entry->unk12) {
            anim_path_sound_play(entry->id, entry->unk12);
        }
        if (entry->soundID) {
            sound_play(entry->soundID, NULL);
        }
        obj->properties.common.unk0 = 1;
    }
    if (obj->properties.common.unk0 && (entry->flags & 1)) {
        func_80079BA8();
    }
}

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

void *mempool_alloc(s32 size, u32 colourTag);
s32 func_8003E638(void *out, Object *obj, u8 *target, s32 arg3, s32 arg4, s32 lo, s32 hi);
s32 rand_range(s32 min, s32 max);
s32 obj_id_is_special(s32 objectID);

/* JFG: an object 0x10 bytes batch of a model (see LevelBatchInfo) */
typedef struct ModelBatch275 {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ s8 unk1;
    /* 0x02 */ u8 pad2[6];
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ s32 flags;
} ModelBatch275;

#ifdef NON_MATCHING
/* NON_MATCHING: register allocation: the target copies obj/entry to $s2/$s0 first and keeps entry->0xC in $a1,
   the model in $a3, the batch count in $a0 and the total in $a2 (mine: $t0, $t1, $a2, $a3) */
/* JFG: init: break the target object's model batches lo..hi (entry 0x10/0x11) off as pieces flying off
 * at random; freed when it has no such triangles or the buffers can not be had. */
void func_800275E8(Object *obj, u8 *entry) {
    u8 *state;
    s32 total;

    {
        u8 *target;
        u8 *model;
        s32 n;
        ModelBatch275 *batch;

        target = *(u8 **) (entry + 0xC);
        model = **(u8 ***) (*(u8 ***) (target + 0x6C) + *(s8 *) (target + 0x3A));
        state = *(u8 **) &obj->animator;
        n = *(s16 *) (model + 0x16);
        batch = *(ModelBatch275 **) (model + 0x24);
        **(u8 **) ((u8 *) obj + 0x7C) &= ~2;
        *(s32 *) (state + 4) = 0;
        *(s32 *) (state + 8) = 0;
        *(s32 *) (state + 0xC) = 0;
        *(s32 *) (state + 0x10) = 0;
        *(u8 **) state = *(u8 **) (entry + 0xC);
        state[0x17] = entry[0x12];
        *(s16 *) (state + 0x14) = 0x3B6;
        total = 0;
        while (n--) {
            if (batch->unk1 >= entry[0x10] && batch->unk1 <= entry[0x11] && !(batch->flags & 0x400)) {
                total += (batch + 1)->facesOffset - batch->facesOffset;
            }
            batch++;
        }
    }
    if (total != 0 && (*(void **) (state + 4) = mempool_alloc(total * 26, 0xFF0000FF)) != NULL &&
        (*(void **) (state + 8) = mempool_alloc(total * 16, 0xFF0000FF)) != NULL) {
        func_8003E638(state + 0xC, obj, *(u8 **) state, 2, 0, entry[0x10], entry[0x11]);
        func_8003E638(state + 0x10, obj, *(u8 **) state, 6, 0, entry[0x10], entry[0x11]);
        if (*(s32 *) (state + 0x10) != 0) {
            obj->level_entry = NULL;
            obj->x_velocity = rand_range(-5, 5);
            obj->y_velocity = rand_range(-5, 5);
            obj->z_velocity = rand_range(-5, 5);
            *(s16 *) (state + 0x18) = rand_range(-5000, 5000);
            *(s16 *) (state + 0x1A) = rand_range(-5000, 5000);
            *(s16 *) (state + 0x1C) = rand_range(-5000, 5000);
            if (obj_id_is_special(*(s16 *) (*(u8 **) state + 0x4A))) {
                state[0x1E] = 0;
            } else {
                state[0x1E] = 1;
            }
            return;
        }
    }
    free_object(obj);
}
#else
INCLUDE_ASM("asm/nonmatchings/25A70", func_800275E8);
#endif

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027844);

typedef struct BufferState {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ void *buffers[4];
} BufferState;

void mempool_free(void *data);

/* JFG: free the four buffers of the object's state. */
void obj_state_buffers_free(Object *obj) {
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

Object *cam_get_cameras(void);
f32 func_80049BEC(f32 base, s32 exponent);
extern f32 D_800ABB14;
extern f32 D_800ABB18;

/* JFG: loop: flag the object while the camera is above its floor height; grow in, then drift (easing the
   vertical speed to its target) until it rises past that height, where it emits particles and is freed. */
void obj_loop_drift(Object *obj, s32 updateRate) {
    f32 moveScale;
    f32 diff;
    DriftState *state;

    moveScale = updateRate;
    state = (DriftState *) obj->animator;
    diff = cam_get_cameras()->y_position - state->unkC;
    if (diff > 0.0f) {
        obj->flags |= 8;
    } else {
        obj->flags &= ~8;
    }
    if (state->unk4 < 1.0f) {
        state->unk4 += updateRate * D_800ABB14;
        if (state->unk4 >= 1.0f) {
            state->unk4 = 1.0f;
        }
        obj->scale = state->unk0 * state->unk4;
    } else {
        move_object(obj, obj->x_velocity * moveScale, obj->y_velocity * moveScale, obj->z_velocity * moveScale);
        obj->y_velocity += (state->unk8 - obj->y_velocity) * (1.0f - func_80049BEC(D_800ABB18, updateRate));
        if (state->unkC < obj->y_position) {
            obj->y_position = state->unkC;
            obj->particleEmittersEnabled = 1;
            obj_spawn_particle(obj, 2);
            free_object(obj);
        }
    }
}

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
