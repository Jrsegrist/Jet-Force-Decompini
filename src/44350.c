/*
 * Level loading and the current level's header. Functions in C are from the
 * Diddy Kong Racing decompilation (CC0 1.0), src/game.c, or marked "JFG".
 */
#include "common.h"

u32 *asset_table_load(u32 assetIndex);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/* DKR include/level.h (reduced to the fields used here) */
typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0x69];
    /* 0x69 */ s8 unk69;
    /* 0x6A */ u8 pad6A[2];
    /* 0x6C */ u8 race_type;
    /* 0x6D */ u8 pad6D[0x72 - 0x6D];
    /* 0x72 */ u8 music;
    /* 0x73 */ u8 pad73;
    /* 0x74 */ u16 instruments;
    /* 0x76 */ u8 pad76[0x92 - 0x76];
    /* 0x92 */ s16 colourCycles[7];
    /* 0xA0 */ s16 weatherCount;
    /* 0xA2 */ u8 padA2[0xB4 - 0xA2];
    /* 0xB4 */ void *texture;
    /* 0xB8 */ u8 padB8[0xE3 - 0xB8];
    /* 0xE3 */ u8 unkE3;
    /* 0xE4 */ u8 padE4[0x102 - 0xE4];
    /* 0x102 */ u8 unk102;
    /* 0x103 */ u8 region;
} LevelHeader;

/* JFG: per-region object flags (17 bytes each) */
typedef struct ObjectFlags {
    u8 unk0;
    u8 bits[16];
} ObjectFlags;
extern ObjectFlags *D_800A31C4;

extern LevelHeader *gCurrentLevelHeader;
extern s32 D_800FB114;

typedef struct ColourCycle {
    u8 pad0[0x10];
} ColourCycle;
extern ColourCycle D_800FB170[7];
void update_colour_cycle(ColourCycle *cycle, s32 updateRate);

void music_dynamic_set(u16 channelMask);
void music_channel_reset_all(void);
void music_tempo_set_relative(f32 tempo);
void music_stop(void);
u8 music_current_sequence(void);
void music_play(u8 seqID);
void func_80000798(void);

INCLUDE_ASM("asm/nonmatchings/44350", func_80043750);

/* Size: 5 bytes */
typedef struct LevelListEntry {
    /* 0x00 */ u8 type;
    /* 0x01 */ s8 world;
    /* 0x02 */ u8 region;
    /* 0x03 */ u8 unk3 : 3;
    /* 0x03 */ u8 blur : 2;
    /* 0x03 */ u8 screenMode : 3;
    /* 0x04 */ u8 tune;
} LevelListEntry;
extern s32 D_800FB124;
extern LevelListEntry *D_800FB12C;

s32 func_80043B78(s32 level) {
    if (level >= 0 && level < D_800FB124) {
        return D_800FB12C[level].type;
    }
    return -1;
}

s32 func_80043BB8(s32 level) {
    if (level >= 0 && level < D_800FB124) {
        return D_800FB12C[level].tune;
    }
    return -1;
}

s32 func_80043BF8(s32 level) {
    if (level >= 0 && level < D_800FB124) {
        return D_800FB12C[level].world;
    }
    return 0;
}

s32 func_80043C38(s32 level) {
    if (level >= 0 && level < D_800FB124) {
        return D_800FB12C[level].region;
    }
    return 0;
}

s32 func_80043C78(s32 level) {
    if (level >= 0 && level < D_800FB124) {
        return D_800FB12C[level].screenMode;
    }
    return 0;
}

s32 func_80043CC0(s32 level) {
    if (level >= 0 && level < D_800FB124) {
        return D_800FB12C[level].blur;
    }
    return 0;
}

u8 func_80048548(void);

s32 func_80043D08(s32 level) {
    s32 count = func_80048548() - 1;
    if (level >= 0 && level < D_800FB124) {
        if (D_800FB12C[level].unk3 != 0) {
            count = D_800FB12C[level].unk3;
        }
    }
    return count;
}

INCLUDE_ASM("asm/nonmatchings/44350", func_80043D70);

INCLUDE_ASM("asm/nonmatchings/44350", func_80043E3C);

/* JFG: start the current level's music (if it isn't already playing). */
void func_8004455C(f32 tempo) {
    if (gCurrentLevelHeader->music != 0) {
        if (gCurrentLevelHeader->music != music_current_sequence()) {
            music_channel_reset_all();
            music_play(gCurrentLevelHeader->music);
            func_80000798();
            music_tempo_set_relative(tempo);
            music_dynamic_set(gCurrentLevelHeader->instruments);
        }
    } else {
        music_stop();
    }
}

/* JFG: update the current level's colour cycles. */
void func_80044610(s32 updateRate) {
    s32 i;
    for (i = 0; i < 7; i++) {
        if (gCurrentLevelHeader->colourCycles[i] != -1) {
            update_colour_cycle(&D_800FB170[i], updateRate);
        }
    }
}

/* JFG: the address of the current level's colour cycle data. */
void *func_800446B0(void) {
    return D_800FB170;
}

/* JFG: the current level number. */
s32 func_800446BC(void) {
    return D_800FB114;
}

/**
 * Return the race type ID of the current level.
 * Official name: levelGetType
 */
u8 level_type(void) {
    return gCurrentLevelHeader->race_type;
}

/* JFG: a byte of the current level header (0 when no level is loaded). */
u8 func_800446E4(void) {
    if (gCurrentLevelHeader != NULL) {
        return gCurrentLevelHeader->unkE3;
    }
    return 0;
}

/**
 * Return the header data of the current level.
 * Official Name: levelGetLevel
 */
LevelHeader *level_header(void) {
    return gCurrentLevelHeader;
}

extern char *D_800A31A0;
extern u32 *D_800FB110;

/* JFG: load a level's name into D_800A31A0 */
char *func_8004471C(s32 level) {
    *D_800A31A0 = 0;
    if (level < D_800FB124) {
        if ((D_800FB110 = asset_table_load(0x1E)) != NULL) {
            asset_load(0x1F, (u32) D_800A31A0, D_800FB110[level], 0x20);
            mempool_free(D_800FB110);
        }
    }
    return D_800A31A0;
}

void func_8006EE28(s32 arg0);
void func_80068874(void);
void bgdraw_primcolour(u8 red, u8 green, u8 blue);
void music_jingle_stop(void);
void lights_free(void);
void func_80023A00(void);
void func_8001B660(void);
void func_80079CC4(void);
void func_80002944(void);
void sound_volume_change(s32 behaviour);
void weather_free(void);
void tex_free(void *tex);
void func_80053934(void);
void func_8006FB18(void);
extern s32 D_800A089C;

/* JFG: free the current level. */
void func_800447B4(void) {
    func_8006EE28(1);
    func_80068874();
    bgdraw_primcolour(0, 0, 0);
    mempool_free(gCurrentLevelHeader);
    music_jingle_stop();
    lights_free();
    func_80023A00();
    func_8001B660();
    func_80079CC4();
    func_80002944();
    D_800A089C = 0;
    sound_volume_change(0);
    if (gCurrentLevelHeader->weatherCount > 0) {
        weather_free();
    }
    if (gCurrentLevelHeader->unk69 == -1) {
        tex_free(gCurrentLevelHeader->texture);
    }
    func_80053934();
    func_8006FB18();
}

/* JFG: the next level (wrapping around) in the given world. */
s32 func_80044898(s32 level, s8 world) {
    s32 next = level + 1;
    if (next >= D_800FB124) {
        next = 0;
    }
    while (next != level && world != D_800FB12C[next].world) {
        next++;
        if (next >= D_800FB124) {
            next = 0;
        }
    }
    return next;
}

/* JFG: the previous level (wrapping around) in the given world. */
s32 func_80044928(s32 level, s8 world) {
    s32 prev = level - 1;
    if (prev < 0) {
        prev = D_800FB124 - 1;
    }
    while (prev != level && world != D_800FB12C[prev].world) {
        prev--;
        if (prev < 0) {
            prev = D_800FB124 - 1;
        }
    }
    return prev;
}

extern ObjectFlags D_800FB1E0[32];

void func_800449C8(void) {
    s32 i;
    s32 j;
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 16; j++) {
            D_800FB1E0[i].bits[j] = 0;
        }
        D_800FB1E0[i].unk0 = 0;
    }
}

void func_80044A1C(void) {
    u8 region = gCurrentLevelHeader->region;
    if (region >= 32) {
        D_800A31C4 = NULL;
        return;
    }
    D_800A31C4 = &D_800FB1E0[region];
    D_800A31C4->unk0 = 0;
}

/* JFG: reserve count object IDs in the current region; returns the first, or -1 if they don't fit in 128. */
s32 func_80044A6C(s32 count) {
    u8 id;
    s32 next;

    if (D_800A31C4 == NULL) {
        return -1;
    }
    id = D_800A31C4->unk0;
    next = id + count;
    if (next > 0x80) {
        return -1;
    }
    D_800A31C4->unk0 = next;
    return id;
}

/* JFG: set bit `flag` of the level's object flags. */
void func_80044AB4(s32 flag) {
    if (D_800A31C4 != NULL) {
        D_800A31C4->bits[flag >> 3] |= (1 << (flag & 7));
    }
}

s32 func_80044AE8(s32 flag) {
    u8 mode = gCurrentLevelHeader->unk102;
    if (mode == 0 || mode == 0xFF || D_800A31C4 == NULL) {
        return 0;
    }
    return (D_800A31C4->bits[flag >> 3] & (1 << (flag & 7))) != 0;
}
