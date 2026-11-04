/*
 * Runlink module 87: a breakable object whose damage settings are looked up
 * by object ID in the module's data (init 001DC, control 00510), and the
 * small tumbling pieces it throws (init 01060, control 010DC), which fly
 * under gravity until their timer runs out. 011B0 changes one setting of a
 * type's entry, 011DC resets an object's state from its entry.
 */
#include "module.h"

/* An entry of the settings table (the entries have different sizes) */
typedef struct M087Entry {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
} M087Entry;

/* What the pieces' init gets */
typedef struct M087PieceSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ u8 lifetime;
    /* 0x0F */ u8 emitters;
    /* 0x10 */ f32 speed;
    /* 0x14 */ f32 gravity;
} M087PieceSetup;

typedef struct M087PieceState {
    /* 0x0 */ f32 gravity;
    /* 0x4 */ s32 timer;
} M087PieceState;

extern s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos); /* DKR */
extern void func_8005F5AC(Object *obj, s32 updateRate);

/* The settings entries in the module's .data (still assembly), by module offset */
extern M087Entry rl_m087_01224;
extern M087Entry rl_m087_01264;
extern M087Entry rl_m087_012A4;
extern M087Entry rl_m087_01324;
extern M087Entry rl_m087_01364; /* the default */
extern M087Entry rl_m087_01390;
extern M087Entry rl_m087_013F0;
extern M087Entry rl_m087_01450;
extern M087Entry rl_m087_01470;

/* The settings entry for an object ID */
static M087Entry *func_m087_00000(s32 id) {
    switch (id) {
        case 0x135:
            return &rl_m087_01224;
        case 0x168:
            return &rl_m087_01264;
        case 0x171:
            return &rl_m087_012A4;
        case 0x1A8:
            return &rl_m087_01324;
        case 0x1C3:
            return &rl_m087_01390;
        case 0x2DE:
        case 0x2DF:
            return &rl_m087_013F0;
        case 0x2CC:
            return &rl_m087_01450;
        case 0x58:
            return &rl_m087_01470;
        default:
            return &rl_m087_01364;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m087", func_m087_000D4);

INCLUDE_ASM("asm/nonmatchings/modules/m087", func_m087_001DC);

INCLUDE_ASM("asm/nonmatchings/modules/m087", func_m087_00510);

void func_m087_01060(Object *obj, M087PieceSetup *setup) {
    obj->trans.rotation.y_rotation = setup->yaw;
    obj->trans.rotation.x_rotation = setup->pitch;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->particleEmittersEnabled = setup->emitters;
    OBJ_STATE(obj, M087PieceState)->gravity = setup->gravity;
    OBJ_STATE(obj, M087PieceState)->timer = setup->lifetime * 8;
}

void func_m087_010DC(Object *obj, s32 updateRate) {
    s32 i;

    OBJ_STATE(obj, M087PieceState)->timer -= updateRate * 8;
    if (OBJ_STATE(obj, M087PieceState)->timer <= 0) {
        func_800079B8(obj);
        return;
    }
    i = updateRate;
    while (i--) {
        obj->trans.x_position += obj->velocity.x;
        obj->trans.y_position += obj->velocity.y;
        obj->trans.z_position += obj->velocity.z;
        obj->velocity.y += OBJ_STATE(obj, M087PieceState)->gravity;
    }
    obj->segmentID = get_level_segment_index_from_position(obj->trans.x_position, obj->trans.y_position,
                                                           obj->trans.z_position);
    obj->trans.rotation.x_rotation += updateRate << 11;
    obj->trans.rotation.z_rotation += updateRate << 11;
    func_8005F5AC(obj, updateRate);
}

void func_m087_011B0(s32 id, s16 value) {
    func_m087_00000(id)->unk1E = value;
}

void func_m087_011DC(Object *obj) {
    s16 *state = obj->unk68;

    *state = func_m087_00000(*(s16 *) obj->pad2C)->unk1C;
}

INCLUDE_ASM("asm/nonmatchings/modules/m087", data);

INCLUDE_ASM("asm/nonmatchings/modules/m087", rodata);
