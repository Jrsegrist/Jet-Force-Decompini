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

typedef struct Object {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
    /* 0x18 */ u8 pad18[0x3C - 0x18];
    /* 0x3C */ LevelObjectEntry *level_entry;
    /* 0x40 */ u8 pad40[0x84 - 0x40];
    /* 0x84 */ s32 particleEmittersEnabled;
} Object;

s32 obj_dist_racer(f32 x, f32 y, f32 z, f32 radius, s32 is2dCheck, Object **sortObj);
void obj_spawn_particle(Object *obj, s32 updateRate);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80024E70);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80024EF4);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80024F40);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80024F94);

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002500C);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800252EC);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800253F0);

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002561C);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80025A08);

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002662C);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026824);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026A50);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026CAC);

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

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026FD4);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80026FE0);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800270C8);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800270D4);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800270EC);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027120);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800271A0);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027220);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800272B4);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800274FC);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027564);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800275CC);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800275E8);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027844);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027D08);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027DA0);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027E1C);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80027F90);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80028034);

INCLUDE_ASM("asm/nonmatchings/25A70", func_800281B4);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80028274);

INCLUDE_ASM("asm/nonmatchings/25A70", func_80028414);

INCLUDE_ASM("asm/nonmatchings/25A70", func_8002844C);
