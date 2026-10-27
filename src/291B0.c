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

/* JFG: spawn entry index at (x, y, z). */
void func_800285B0(s16 index, s16 x, s16 y, s16 z, s32 arg4, s32 arg5) {
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

/* JFG: set up the object's emitters (in the given buffer); returns the bytes used. */
s32 func_8002863C(EmitterObject *obj, Emitter *emitters, EmitterHeader *header) {
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

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002993C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029A20);

/* Size: 0x24 bytes */
typedef struct Blast {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 size;
    /* 0x10 */ u8 pad10[0xC];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[6];
} Blast;
extern Blast D_800F6B60[16];
extern s32 D_800A11B4;

/* JFG: clear the 16 blast slots. */
void func_80029B78(void) {
    Blast *blast = D_800F6B60;
    s32 i = 16;
    while (i--) {
        blast->unk1C = 0;
        blast++;
    }
    D_800A11B4 = 0;
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029BA4);

INCLUDE_ASM("asm/nonmatchings/291B0", func_80029CFC);

/* JFG: TRUE if the point is inside an active blast (radius plus the blast's size unless ignoreSize). */
s32 func_8002A704(f32 x, f32 y, f32 z, f32 radius, s32 ignoreSize) {
    Blast *blast;
    s32 i;
    s32 hit = FALSE;
    f32 dx, dy, dz;
    f32 r;

    if (D_800A11B4 != 0) {
        blast = D_800F6B60;
        i = 16;
        while (i--) {
            if (blast->unk1C != 0) {
                dx = blast->x - x;
                dy = blast->y - y;
                dz = blast->z - z;
                r = radius;
                if (!ignoreSize) {
                    r += blast->size;
                }
                r *= r;
                if (dx * dx + dy * dy + dz * dz < r) {
                    i = 0;
                    hit = TRUE;
                }
            }
            blast++;
        }
    }
    return hit;
}

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002A7D8);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002A954);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002AA7C);

INCLUDE_ASM("asm/nonmatchings/291B0", func_8002AC3C);
