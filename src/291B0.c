#include "common.h"

typedef struct SpawnEntry {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0x46];
    /* 0x50 */ u8 unk50;
    /* 0x54 */ s32 unk54;
} SpawnEntry;
extern SpawnEntry **D_800F6B04;
extern s32 D_800F6B08;
void *spawn_object(SpawnEntry *entry, s32 arg1);

/* JFG: spawn entry index at (x, y, z) (0x800285B0). */
void spawn_object_from_list(s16 index, s16 x, s16 y, s16 z, s32 arg4, s32 arg5) {
    SpawnEntry *entry;

    if (index >= D_800F6B08) {
        return;
    }
    entry = D_800F6B04[index];
    entry->x = x;
    entry->y = y;
    entry->z = z;
    entry->unk50 = arg4;
    entry->unk54 = arg5;
    spawn_object(entry, 1);
}

/* Size: 0x24 bytes */
typedef struct Emitter {
    u8 pad0[0x24];
} Emitter;
typedef struct EmitterObject {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ Emitter *emitters;
} EmitterObject;
typedef struct EmitterHeader {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ s16 *emitterIDs;
    /* 0x10 */ u8 pad10[0x20];
    /* 0x30 */ u8 emitterCount;
} EmitterHeader;
void emitter_init(Emitter *emitter, s32 behaviourID, s32 particleID);

/* JFG: set up the object's emitters (in the given buffer); returns the bytes used (0x8002863C). */
s32 obj_init_emitter(EmitterObject *obj, Emitter *emitters, EmitterHeader *header) {
    s32 i;

    obj->emitters = emitters;
    for (i = 0; i < header->emitterCount; i++) {
        emitter_init(&obj->emitters[i], (u8) (header->emitterIDs[i] >> 8), (u8) header->emitterIDs[i]);
    }
    return (header->emitterCount * sizeof(Emitter) + 3) & ~3;
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_800286F4);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80028954);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80028E7C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80028F90);

INCLUDE_ASM("asm/nonmatchings/291B0", func_800291BC);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029688);

/* A sprite object with two colour cycles, set up from its level entry */
typedef struct ChromaSpriteState {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 padA[6];
    /* 0x10 */ u8 colourCycle1[0x10];
    /* 0x20 */ u8 colourCycle2[0x10];
    /* 0x30 */ void *entry;
} ChromaSpriteState;
typedef struct ChromaSpriteObject {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ f32 scale;
    /* 0x0C */ u8 padC[0x30];
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ u8 pad40[0x28];
    /* 0x68 */ ChromaSpriteState *data;
    /* 0x6C */ void **sprite;
} ChromaSpriteObject;
typedef struct ChromaSpriteEntry {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[4];
    /* 0x0E */ s16 spriteID;
    /* 0x10 */ u8 pad10[2];
    /* 0x12 */ u16 scale;
    /* 0x14 */ u8 pad14[2];
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 colourCycle1;
    /* 0x18 */ u8 colourCycle2;
} ChromaSpriteEntry;
void sprite_free(void *sprite);
void *tex_load_sprite(s32 spriteID, s32 arg1);
void colour_cycle_init(void *cycle, s32 assetID);
extern f32 D_800ABB4C;

/* JFG: (re)set a sprite object from its entry. */
void func_8002993C(ChromaSpriteObject *obj, ChromaSpriteEntry *entry) {
    ChromaSpriteState *data = obj->data;

    sprite_free(*obj->sprite);
    *obj->sprite = tex_load_sprite(entry->spriteID, 0);
    obj->scale = entry->scale * D_800ABB4C;
    data->x = entry->x;
    data->y = entry->y;
    data->z = entry->z;
    data->unk8 = 0;
    data->unk6 = 0;
    data->unk9 = entry->unk16;
    data->entry = entry;
    obj->unk3C = 0;
    colour_cycle_init(data->colourCycle1, entry->colourCycle1);
    colour_cycle_init(data->colourCycle2, entry->colourCycle2);
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029A20);

/* A sphere (centre and radius) in a fixed table of 16. Size: 0x24 bytes */
typedef struct Zone {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 size;
    /* 0x10 */ u8 pad10[0xC];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[6];
} Zone;
extern Zone D_800F6B60[16];
extern s32 D_800A11B4;

/* JFG: mark all 16 zones unused and zero D_800A11B4 (the zone test is skipped while it is zero). */
void func_80029B78(void) {
    Zone *zone = D_800F6B60;
    s32 i = 16;
    while (i--) {
        zone->unk1C = 0;
        zone++;
    }
    D_800A11B4 = 0;
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029BA4);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029CFC);

/* JFG: TRUE if a sphere of the given radius at the point touches a zone in use (the zone's own size is left out with ignoreSize). */
s32 func_8002A704(f32 x, f32 y, f32 z, f32 radius, s32 ignoreSize) {
    Zone *zone;
    s32 i;
    s32 hit = FALSE;
    f32 dx, dy, dz;
    f32 r;

    if (D_800A11B4 != 0) {
        zone = D_800F6B60;
        i = 16;
        while (i--) {
            if (zone->unk1C != 0) {
                dx = zone->x - x;
                dy = zone->y - y;
                dz = zone->z - z;
                r = radius;
                if (!ignoreSize) {
                    r += zone->size;
                }
                r *= r;
                if (dx * dx + dy * dy + dz * dz < r) {
                    i = 0;
                    hit = TRUE;
                }
            }
            zone++;
        }
    }
    return hit;
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002A7D8);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002A954);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002AA7C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002AC3C);
