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
extern ObjectFlags *gCurrentObjectFlags;

extern LevelHeader *gCurrentLevelHeader;
extern s32 gMapId;

typedef struct ColourCycle {
    u8 pad0[0x10];
} ColourCycle;
extern ColourCycle gLevelColourCycles[7];
void update_colour_cycle(ColourCycle *cycle, s32 updateRate);

void music_dynamic_set(u16 channelMask);
void music_channel_reset_all(void);
void music_tempo_set_relative(f32 tempo);
void music_stop(void);
u8 music_current_sequence(void);
void music_play(u8 seqID);
void music_volume_fade_stop(void);

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
extern s32 gNumberOfLevelHeaders;
extern LevelListEntry *gGlobalLevelTable;

/* Returns a level's type, or -1 (0x80043B78). */
s32 leveltable_type(s32 level) {
    if (level >= 0 && level < gNumberOfLevelHeaders) {
        return gGlobalLevelTable[level].type;
    }
    return -1;
}

/* Returns a level's tune, or -1 (0x80043BB8). */
s32 leveltable_tune(s32 level) {
    if (level >= 0 && level < gNumberOfLevelHeaders) {
        return gGlobalLevelTable[level].tune;
    }
    return -1;
}

/* Returns a level's world, or 0 (0x80043BF8). */
s32 leveltable_world(s32 level) {
    if (level >= 0 && level < gNumberOfLevelHeaders) {
        return gGlobalLevelTable[level].world;
    }
    return 0;
}

/* Returns a level's region, or 0 (0x80043C38). */
s32 leveltable_region(s32 level) {
    if (level >= 0 && level < gNumberOfLevelHeaders) {
        return gGlobalLevelTable[level].region;
    }
    return 0;
}

/* Returns a level's screen mode, or 0 (0x80043C78). */
s32 leveltable_screen_mode(s32 level) {
    if (level >= 0 && level < gNumberOfLevelHeaders) {
        return gGlobalLevelTable[level].screenMode;
    }
    return 0;
}

/* Returns a level's blur setting, or 0 (0x80043CC0). */
s32 leveltable_blur(s32 level) {
    if (level >= 0 && level < gNumberOfLevelHeaders) {
        return gGlobalLevelTable[level].blur;
    }
    return 0;
}

u8 func_80048548(void);

s32 func_80043D08(s32 level) {
    s32 count = func_80048548() - 1;
    if (level >= 0 && level < gNumberOfLevelHeaders) {
        if (gGlobalLevelTable[level].unk3 != 0) {
            count = gGlobalLevelTable[level].unk3;
        }
    }
    return count;
}

/* JFG: list the different regions of a world's levels; returns how many (0x80043D70). */
s32 leveltable_world_regions(s32 world, u8 *regions) {
    s32 count = 0;
    s32 i;
    s32 j;
    s32 found;
    u8 region;

    for (i = 0; i < gNumberOfLevelHeaders; i++) {
        if (world == gGlobalLevelTable[i].world) {
            if (gGlobalLevelTable[i].region != 0 && gGlobalLevelTable[i].region != 0xFF) {
                found = FALSE;
                for (j = 0; j < count; j++) {
                    if (regions[j] == gGlobalLevelTable[i].region) {
                        found = TRUE;
                        j = count;
                    }
                }
                if (!found) {
                    regions[count++] = gGlobalLevelTable[i].region;
                }
            }
        }
    }
    return count;
}

INCLUDE_ASM("asm/nonmatchings/44350", func_80043E3C);

/* JFG: start the current level's music (if it isn't already playing) (0x8004455C). */
void level_music_start(f32 tempo) {
    if (gCurrentLevelHeader->music != 0) {
        if (gCurrentLevelHeader->music != music_current_sequence()) {
            music_channel_reset_all();
            music_play(gCurrentLevelHeader->music);
            music_volume_fade_stop();
            music_tempo_set_relative(tempo);
            music_dynamic_set(gCurrentLevelHeader->instruments);
        }
    } else {
        music_stop();
    }
}

/* JFG: update the current level's colour cycles (0x80044610). */
void level_colour_cycles_update(s32 updateRate) {
    s32 i;
    for (i = 0; i < 7; i++) {
        if (gCurrentLevelHeader->colourCycles[i] != -1) {
            update_colour_cycle(&gLevelColourCycles[i], updateRate);
        }
    }
}

/* JFG: the address of the current level's colour cycle data (0x800446B0). */
void *level_colour_cycles_get(void) {
    return gLevelColourCycles;
}

/* JFG: the current level number (0x800446BC). */
s32 level_id(void) {
    return gMapId;
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

extern char *gLevelName;
extern u32 *D_800FB110;

/* JFG: load a level's name into gLevelName (0x8004471C). */
char *level_name(s32 level) {
    *gLevelName = 0;
    if (level < gNumberOfLevelHeaders) {
        if ((D_800FB110 = asset_table_load(0x1E)) != NULL) {
            asset_load(0x1F, (u32) gLevelName, D_800FB110[level], 0x20);
            mempool_free(D_800FB110);
        }
    }
    return gLevelName;
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

/* JFG: free the current level (0x800447B4). */
void level_free(void) {
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

/* JFG: the next level (wrapping around) in the given world (0x80044898). */
s32 leveltable_world_next(s32 level, s8 world) {
    s32 next = level + 1;
    if (next >= gNumberOfLevelHeaders) {
        next = 0;
    }
    while (next != level && world != gGlobalLevelTable[next].world) {
        next++;
        if (next >= gNumberOfLevelHeaders) {
            next = 0;
        }
    }
    return next;
}

/* JFG: the previous level (wrapping around) in the given world (0x80044928). */
s32 leveltable_world_prev(s32 level, s8 world) {
    s32 prev = level - 1;
    if (prev < 0) {
        prev = gNumberOfLevelHeaders - 1;
    }
    while (prev != level && world != gGlobalLevelTable[prev].world) {
        prev--;
        if (prev < 0) {
            prev = gNumberOfLevelHeaders - 1;
        }
    }
    return prev;
}

extern ObjectFlags gObjectFlags[32];

/* Clears the object flags of all 32 regions (0x800449C8). */
void objflags_clear_all(void) {
    s32 i;
    s32 j;
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 16; j++) {
            gObjectFlags[i].bits[j] = 0;
        }
        gObjectFlags[i].unk0 = 0;
    }
}

/* Selects the current level region's object flags (0x80044A1C). */
void objflags_select_region(void) {
    u8 region = gCurrentLevelHeader->region;
    if (region >= 32) {
        gCurrentObjectFlags = NULL;
        return;
    }
    gCurrentObjectFlags = &gObjectFlags[region];
    gCurrentObjectFlags->unk0 = 0;
}

/* JFG: reserve count object IDs in the current region; returns the first, or -1 if they don't fit in 128 (0x80044A6C). */
s32 objflags_reserve_ids(s32 count) {
    u8 id;
    s32 next;

    if (gCurrentObjectFlags == NULL) {
        return -1;
    }
    id = gCurrentObjectFlags->unk0;
    next = id + count;
    if (next > 0x80) {
        return -1;
    }
    gCurrentObjectFlags->unk0 = next;
    return id;
}

/* JFG: set bit `flag` of the level's object flags (0x80044AB4). */
void objflag_set(s32 flag) {
    if (gCurrentObjectFlags != NULL) {
        gCurrentObjectFlags->bits[flag >> 3] |= (1 << (flag & 7));
    }
}

/* Returns an object flag of the current region (0x80044AE8). */
s32 objflag_get(s32 flag) {
    u8 mode = gCurrentLevelHeader->unk102;
    if (mode == 0 || mode == 0xFF || gCurrentObjectFlags == NULL) {
        return 0;
    }
    return (gCurrentObjectFlags->bits[flag >> 3] & (1 << (flag & 7))) != 0;
}
