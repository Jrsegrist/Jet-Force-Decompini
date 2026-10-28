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

/* JFG: the level entry of object 0x73 (0x22 bytes) */
typedef struct LevelObjectEntry73 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ s16 unk18;
    /* 0x1A */ u8 pad1A[2];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[4];
} LevelObjectEntry73;

extern f32 D_800ABB44;

#ifdef NON_MATCHING
/* NON_MATCHING: the target loads D_800ABB44 before moving `a` to $f12 (the first two instructions swap) */
/* JFG: spawn an object 0x73 at (x, y, z); the four values are scaled by D_800ABB44. */
void func_80028E7C(s16 x, s16 y, s16 z, f32 a, f32 b, f32 c, f32 d, s16 arg7, u8 arg8) {
    LevelObjectEntry73 entry;

    entry.size = sizeof(LevelObjectEntry73);
    entry.objectID = 0x73;
    entry.unkC = a * D_800ABB44;
    entry.unkE = b * D_800ABB44;
    entry.unk10 = c * D_800ABB44;
    entry.unk12 = d * D_800ABB44;
    entry.x = x;
    entry.unk18 = arg7;
    entry.unk1C = arg8;
    entry.y = y;
    entry.z = z;
    spawn_object((SpawnEntry *) &entry, 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/291B0", func_80028E7C);
#endif

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
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ u8 padC[0x28 - 0xC];
    /* 0x28 */ f32 animFrame;
    /* 0x2C */ u8 pad2C[0x39 - 0x2C];
    /* 0x39 */ u8 opacity;
    /* 0x3A */ u8 pad3A[2];
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
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ s16 lifetime;
    /* 0x0E */ s16 spriteID;
    /* 0x10 */ u8 pad10[2];
    /* 0x12 */ u16 scale;
    /* 0x14 */ u8 fadeOut;
    /* 0x15 */ u8 fadeIn;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 colourCycle1;
    /* 0x18 */ u8 colourCycle2;
    /* 0x19 */ u8 pad19;
    /* 0x1A */ u16 spin;
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

void free_object(void *obj);
void update_colour_cycle(void *cycle, s32 updateRate);
void func_800577A8(void *sprite, s32 *flags, s32 arg2, f32 *frame, s32 updateRate); // animate a sprite

/* JFG: the sprite object's loop: fade in, then after its lifetime fade out and free it; spin and animate. */
void obj_loop_chroma_sprite(ChromaSpriteObject *obj, s32 updateRate) {
    ChromaSpriteState *data;
    ChromaSpriteEntry *entry;
    void *sprite;
    s32 flags;
    u16 opacity;
    s32 fade;

    flags = 1;
    data = obj->data;
    sprite = *obj->sprite;
    entry = (ChromaSpriteEntry *) data->entry;
    if (sprite == NULL) {
        free_object(obj);
        return;
    }
    data->unk6 += updateRate;
    if (data->unk6 >= entry->lifetime) {
        fade = updateRate;
        fade *= entry->fadeOut;
        if (fade < data->unk8) {
            data->unk8 -= fade;
        } else {
            free_object(obj);
            return;
        }
    } else if (data->unk8 < 0xFF) {
        opacity = data->unk8 + updateRate * entry->fadeIn;
        if (opacity >= 0x100) {
            opacity = 0xFF;
        }
        data->unk8 = opacity;
    }
    obj->unk4 += updateRate * entry->spin;
    obj->opacity = data->unk8;
    update_colour_cycle(data->colourCycle1, updateRate);
    update_colour_cycle(data->colourCycle2, updateRate);
    func_800577A8(sprite, &flags, data->unk9, &obj->animFrame, updateRate);
}

/* A sphere (centre and radius) in a fixed table of 16. Size: 0x24 bytes */
typedef struct Zone {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 size;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 value;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s16 unk1C; // frames left (0: unused)
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s32 unk20;
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

#ifdef NON_MATCHING
/* NON_MATCHING: the target keeps the `i != 0` guards of the constant-count loops (and other registers) */
/* JFG: add a zone: the first unused one, or (all 16 in use) the one with the smallest value. */
void func_80029BA4(f32 x, f32 y, f32 z, f32 size, f32 value, f32 time, s32 arg6, s32 arg7) {
    Zone *zone = D_800F6B60;
    Zone *other;
    s32 i;

    if (D_800A11B4 < 16) {
        i = 15;
        while (i-- && zone->unk1C != 0) {
            zone++;
        }
        D_800A11B4++;
    } else {
        other = &D_800F6B60[1];
        i = 15;
        while (i--) {
            if (other->value < zone->value) {
                zone = other;
            }
            other++;
        }
    }
    zone->x = x;
    zone->y = y;
    zone->z = z;
    zone->size = size;
    zone->unk10 = 16384.0f / size;
    zone->unk1E = 1;
    zone->value = value;
    zone->unk1C = time * 60.0f;
    zone->unk18 = value / zone->unk1C;
    zone->unk1F = arg6;
    zone->unk20 = arg7;
}
#else
INCLUDE_ASM("asm/nonmatchings/291B0", func_80029BA4);
#endif

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
