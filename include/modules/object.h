#ifndef MODULES_OBJECT_H
#define MODULES_OBJECT_H

/*
 * The game object as the runlink object-behaviour modules see it.
 *
 * The layout up to 0x60 follows Diddy Kong Racing's Object (DKR decomp,
 * include/structs.h, CC0): the transform at 0, opacity, model index, the
 * level entry and the header pointer, the object ID at 0x4A and the
 * interaction and shadow pointers. From 0x64 on JFG differs. Fields marked
 * (DKR) are named as in DKR; the rest are named by offset until their use is
 * known.
 */

typedef struct Vec3s {
    s16 y_rotation;
    s16 x_rotation;
    s16 z_rotation;
} Vec3s;

/* Size 0x18 (DKR's ObjectTransform) */
typedef struct ObjectTransform {
    /* 0x00 */ Vec3s rotation;
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
} ObjectTransform;

/*
 * The start of every level object entry (the `setup` that a behaviour's
 * init function gets). Behaviour-specific fields follow at 0xA.
 */
typedef struct LevelObjectEntryCommon {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[0xA - 0x2];
} LevelObjectEntryCommon;

/* What object->interactObj points at (JFG's own layout) */
typedef struct ObjectInteraction {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 unk6; /* hit points; 0x100 = not hit (switches, triggers) */
    /* 0x08 */ u8 pad8[0x2];
    /* 0x0A */ u16 flags;
} ObjectInteraction;

/* What object->header points at (JFG's own layout) */
typedef struct ObjectHeader {
    /* 0x00 */ f32 scale;
    /* 0x04 */ u8 pad4[0x1F - 0x4];
    /* 0x1F */ s8 numberOfModelIds;
    /* 0x20 */ u8 pad20[0x22 - 0x20];
    /* 0x22 */ s8 particleCount;
} ObjectHeader;

/* What object->unk5C points at */
typedef struct ObjectUnk5C {
    /* 0x000 */ u8 pad0[0x130];
    /* 0x130 */ s16 damage; /* taken this frame */
} ObjectUnk5C;

typedef struct Object {
    /* 0x00 */ ObjectTransform trans;            /* (DKR) */
    /* 0x18 */ u8 pad18[0x39 - 0x18];
    /* 0x39 */ u8 opacity;                       /* (DKR) */
    /* 0x3A */ s8 modelIndex;                    /* (DKR) */
    /* 0x3B */ s8 animationID;                   /* (DKR) */
    /* 0x3C */ LevelObjectEntryCommon *level_entry; /* (DKR) */
    /* 0x40 */ ObjectHeader *header;             /* (DKR) */
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;                   /* (DKR) */
    /* 0x4A */ s16 objectID;                     /* (DKR) */
    /* 0x4C */ ObjectInteraction *interactObj;   /* (DKR) */
    /* 0x50 */ f32 *shadow;                      /* (DKR: ShadowData, whose first field is its scale) */
    /* 0x54 */ u8 pad54[0x5C - 0x54];
    /* 0x5C */ ObjectUnk5C *unk5C;
    /* 0x60 */ u8 pad60[0x68 - 0x60];
    /* 0x68 */ void *unk68;                      /* behaviour data, for behaviours that allocate it */
    /* 0x6C */ void **unk6C;                     /* model instances? */
    /* 0x70 */ u8 pad70[0x84 - 0x70];
    /* 0x84 */ s32 particleEmittersEnabled;      /* bit mask for obj_spawn_particle (DKR's use) */
    /* 0x88 */ u8 state[0x8];                    /* behaviour state kept in the object (OBJ_STATE) */
} Object;

/* A behaviour's own state, kept in the object at 0x88 */
#define OBJ_STATE(obj, type) ((type *) (obj)->state)

#endif
