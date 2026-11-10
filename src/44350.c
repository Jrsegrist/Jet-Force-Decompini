/*
 * Level loading and the current level's header. Functions in C are from the
 * Diddy Kong Racing decompilation (CC0 1.0), src/game.c, or marked "JFG".
 */
#include "common.h"

u32 *asset_table_load(u32 assetIndex);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/* DKR include/level.h (reduced to the fields used here) */
typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0x54];
    /* 0x54 */ s16 unk54;
    /* 0x56 */ s16 unk56;
    /* 0x58 */ s16 unk58;
    /* 0x5A */ s16 fogNear;
    /* 0x5C */ s16 fogFar;
    /* 0x5E */ s16 fogUnk5E;
    /* 0x60 */ u8 fogR;
    /* 0x61 */ u8 fogG;
    /* 0x62 */ u8 fogB;
    /* 0x63 */ u8 fogUnk63;
    /* 0x64 */ u8 pad64[0x69 - 0x64];
    /* 0x69 */ s8 unk69;
    /* 0x6A */ u8 pad6A[2];
    /* 0x6C */ u8 race_type;
    /* 0x6D */ u8 pad6D[0x72 - 0x6D];
    /* 0x72 */ u8 music;
    /* 0x73 */ u8 pad73;
    /* 0x74 */ u16 instruments;
    /* 0x76 */ u8 pad76[0x92 - 0x76];
    /* 0x92 */ s16 colourCycles[7];
    /* 0xA0 */ s16 weatherCount; // DKR: weatherEnable
    /* 0xA2 */ u8 padA2;
    /* 0xA3 */ u8 weatherType;
    /* 0xA4 */ u8 weatherIntensity;
    /* 0xA5 */ u8 weatherOpacity;
    /* 0xA6 */ s16 weatherVelX;
    /* 0xA8 */ s16 weatherVelY;
    /* 0xAA */ s16 weatherVelZ;
    /* 0xAC */ s8 cameraFOV;
    /* 0xAD */ u8 bgColorRed;
    /* 0xAE */ u8 bgColorGreen;
    /* 0xAF */ u8 bgColorBlue;
    /* 0xB0 */ u8 padB0[4];
    /* 0xB4 */ void *texture;
    /* 0xB8 */ s16 unkB8;
    /* 0xBA */ s16 unkBA;
    /* 0xBC */ s32 pulseLightData; // a misc asset ID, then the asset
    /* 0xC0 */ u8 padC0[2];
    /* 0xC2 */ s8 unkC2;
    /* 0xC3 */ u8 voiceLimit;
    /* 0xC4 */ u8 padC4[4];
    /* 0xC8 */ s8 unkC8;
    /* 0xC9 */ u8 padC9;
    /* 0xCA */ s16 unkCA;
    /* 0xCC */ u8 padCC[0xE3 - 0xCC];
    /* 0xE3 */ u8 unkE3;
    /* 0xE4 */ s32 unkE4; // a misc asset ID, then the asset (or 0)
    /* 0xE8 */ s16 unkE8;
    /* 0xEA */ u8 padEA[0xF7 - 0xEA];
    /* 0xF7 */ u8 unkF7;
    /* 0xF8 */ u8 padF8[0x101 - 0xF8];
    /* 0x101 */ u8 unk101;
    /* 0x102 */ u8 unk102;
    /* 0x103 */ u8 region;
    /* 0x104 */ u8 lightCount;
    /* 0x105 */ u8 pad105[2];
    /* 0x107 */ u8 unk107;
    /* 0x108 */ s16 sounds[3]; // looping level sounds
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

/* The level header fields copied into the level table */
typedef struct LevelHeaderListFields {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ s8 world;
    /* 0x21 */ u8 pad21[2];
    /* 0x23 */ s8 unk23;
    /* 0x24 */ u8 pad24[0x6C - 0x24];
    /* 0x6C */ s8 race_type;
    /* 0x6D */ u8 pad6D[0x72 - 0x6D];
    /* 0x72 */ u8 music;
    /* 0x73 */ u8 pad73[0xC8 - 0x73];
    /* 0xC8 */ s8 blur;
    /* 0xC9 */ u8 screenMode;
    /* 0xCA */ u8 padCA[0x102 - 0xCA];
    /* 0x102 */ u8 region;
} LevelHeaderListFields;

#define COLOUR_TAG_YELLOW 0xFFFF00FF
void *mempool_alloc_safe(s32 size, u32 colourTag);
void mempool_free(void *data);
s32 align4(s32 value);
extern u32 *gTempAssetTable;     // DKR: gTempAssetTable
extern s32 gNumberOfWorlds;      // DKR: gNumberOfWorlds
extern s32 gRaceTypeCountTable[16];  // DKR: gRaceTypeCountTable
extern char **gLevelNames;   // DKR: gLevelNames
extern char *gLevelName;

/**
 * Allocates memory for gGlobalLevelTable, then populates it with relevant data from every level header.
 * The level headers are streamed from ROM. Then loads the level names.
 * JFG: 5-byte entries (type, world, region, then three small settings and the tune), a level name
 * buffer, and the names in one allocation after their pointers.
 */
void level_global_init(void) {
    s32 i;
    s32 size;
    u8 *header;
    char *names;

    header = mempool_alloc_safe(0x114, COLOUR_TAG_YELLOW);
    gTempAssetTable = asset_table_load(0x1E);
    i = 0;
    while (i < 16) {
        gRaceTypeCountTable[i++] = 0;
    }
    gNumberOfLevelHeaders = 0;
    while ((s32) gTempAssetTable[gNumberOfLevelHeaders] != -1) {
        gNumberOfLevelHeaders++;
    }
    gNumberOfLevelHeaders--;
    gGlobalLevelTable = mempool_alloc_safe(gNumberOfLevelHeaders * sizeof(LevelListEntry), COLOUR_TAG_YELLOW);
    gNumberOfWorlds = -1;
    gCurrentLevelHeader = (LevelHeader *) header;
    for (i = 0; i < gNumberOfLevelHeaders; i++) {
        asset_load(0x1F, (u32) gCurrentLevelHeader, gTempAssetTable[i], 0x114);
        if (gNumberOfWorlds < ((LevelHeaderListFields *) gCurrentLevelHeader)->world) {
            gNumberOfWorlds = ((LevelHeaderListFields *) gCurrentLevelHeader)->world;
        }
        if (((LevelHeaderListFields *) gCurrentLevelHeader)->race_type >= 0 &&
            ((LevelHeaderListFields *) gCurrentLevelHeader)->race_type < 16) {
            gRaceTypeCountTable[((LevelHeaderListFields *) gCurrentLevelHeader)->race_type]++;
        }
        gGlobalLevelTable[i].type = ((LevelHeaderListFields *) gCurrentLevelHeader)->race_type;
        gGlobalLevelTable[i].world = ((LevelHeaderListFields *) gCurrentLevelHeader)->world;
        gGlobalLevelTable[i].region = ((LevelHeaderListFields *) gCurrentLevelHeader)->region;
        gGlobalLevelTable[i].unk3 = ((LevelHeaderListFields *) gCurrentLevelHeader)->unk23;
        gGlobalLevelTable[i].blur = ((LevelHeaderListFields *) gCurrentLevelHeader)->blur;
        gGlobalLevelTable[i].screenMode = ((LevelHeaderListFields *) gCurrentLevelHeader)->screenMode;
        gGlobalLevelTable[i].tune = ((LevelHeaderListFields *) gCurrentLevelHeader)->music;
    }
    gNumberOfWorlds++;
    mempool_free(gTempAssetTable);
    mempool_free(header);
    gLevelName = mempool_alloc_safe(0x20, COLOUR_TAG_YELLOW);
    gTempAssetTable = asset_table_load(0x22);
    for (i = 0; (s32) gTempAssetTable[i] != -1; i++) {}
    i--;
    size = gTempAssetTable[i] - gTempAssetTable[0];
    names = mempool_alloc_safe(align4(size) + i * sizeof(char *), COLOUR_TAG_YELLOW);
    gLevelNames = (char **) (align4(size) + names);
    asset_load(0x23, (u32) names, 0, size);
    for (size = 0; size < i; size++) {
        gLevelNames[size] = &names[gTempAssetTable[size]];
    }
    mempool_free(gTempAssetTable);
}

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

void rumble_stop_all(s32 arg0);
u8 func_80048558(void); // the number of players
void sndp_set_active_sound_limit(s32 limit);
void check_reset_and_halt(void);
void objflags_select_region(void);
void colour_cycle_init(ColourCycle *cycle, s32 assetID);
void music_voicelimit_set(u8 voiceLimit);
void lights_init(s32 count, s32 arg1, s32 arg2);
void troop_lists_init(void);
void collision_level_buffers_alloc(void);
void func_8000E674(s32 arg0);
s32 func_80054530(); // a module function (patched in by the runtime linker)
/* The module trap called with a float first; needs `func_80054530_fi = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_fi(f32 arg0, s32 arg1);
void sequence_start(s32 id);
void func_80051EA4(void);
void reset_fog(s32 playerID);
void set_fog(s32 fogIdx, s16 near, s16 far, s16 unk, u8 red, u8 green, u8 blue, u8 unk2);
void func_8005AD90(s32 weatherType, s32 density, s32 velX, s32 velY, s32 velZ, s32 intensity, s32 opacity);
void weather_clip_planes(s16 near, s16 far);
void *load_texture(s32 assetIndex);
s32 *get_misc_asset(s32 index);
void init_pulsating_light_data(void *data);
void bgdraw_primcolour(u8 red, u8 green, u8 blue);
void video_delta_reset(void);
void level_music_start(f32 tempo);
void set_active_camera(s32 index);
void cam_set_fov(f32 camFieldOfView, s32 force);
void glitch_effect_start(s32 arg0);
void func_8005342C(s32 module);
void sound_stop(s32 soundHandle);
void sound_play(u16 soundID, s32 *handlePtr);
extern s32 D_800A31B0[3]; // JFG: the handles of the looping level sounds
extern s16 D_800A31BC[3]; // JFG: the IDs of the looping level sounds (-1: none)

/**
 * Load a level: its header, then everything it sets up (colour cycles, lights, the objects and the
 * track through the modules, fog for the four cameras, weather, the sky, the camera field of view,
 * the music), then swap the looping level sounds over to the new level's.
 * DKR: level_load (a different order and set of steps)
 */
#ifdef NON_MATCHING
/* NON_MATCHING: register allocation (loop tests, the sound loops' pointers and slot) */
void level_load(s32 level, s32 arg1, UNUSED s32 arg2, s32 arg3) {
    s32 count;
    s32 offset;
    s32 size;
    s32 i;
    s32 j;
    s32 k;
    s32 missing;
    s16 soundID;
    s32 slot;
    UNUSED s32 pad1;
    UNUSED s32 pad2;

    rumble_stop_all(1);
    gTempAssetTable = asset_table_load(0x1E);
    if (arg3 < 0) {
        arg3 = 0;
    }
    switch (func_80048558()) {
        case 4:
            sndp_set_active_sound_limit(16);
            break;
        case 3:
            sndp_set_active_sound_limit(16);
            break;
        case 2:
            sndp_set_active_sound_limit(12);
            break;
        default:
            sndp_set_active_sound_limit(8);
            break;
    }
    for (count = 0; (s32) gTempAssetTable[count] != -1; count++) {}
    count--;
    if (level >= count) {
        level = 0;
    }
    offset = gTempAssetTable[level];
    size = gTempAssetTable[level + 1] - offset;
    gCurrentLevelHeader = mempool_alloc_safe(size, COLOUR_TAG_YELLOW);
    asset_load(0x1F, (u32) gCurrentLevelHeader, offset, size);
    check_reset_and_halt();
    mempool_free(gTempAssetTable);
    gMapId = level;
    objflags_select_region();
    for (i = 0; i < 7; i++) {
        if (gCurrentLevelHeader->colourCycles[i] != -1) {
            colour_cycle_init(&gLevelColourCycles[i], gCurrentLevelHeader->colourCycles[i]);
        }
    }
    music_voicelimit_set(gCurrentLevelHeader->voiceLimit);
    music_volume_fade_stop();
    check_reset_and_halt();
    lights_init(gCurrentLevelHeader->lightCount, 8, 16);
    check_reset_and_halt();
    troop_lists_init();
    check_reset_and_halt();
    collision_level_buffers_alloc();
    func_8000E674(arg3);
    check_reset_and_halt();
    func_80054530(gCurrentLevelHeader->unk54, gCurrentLevelHeader->unk58, arg1, gCurrentLevelHeader->unk56,
                  gCurrentLevelHeader->unkCA, gCurrentLevelHeader->unkE8);
    check_reset_and_halt();
    sequence_start(arg3);
    check_reset_and_halt();
    func_80051EA4();
    if (gCurrentLevelHeader->fogNear == 0 && gCurrentLevelHeader->fogFar == 0 && gCurrentLevelHeader->fogR == 0 &&
        gCurrentLevelHeader->fogG == 0 && gCurrentLevelHeader->fogB == 0) {
        for (i = 0; i < 4; i++) {
            reset_fog(i);
        }
    } else {
        for (i = 0; i < 4; i++) {
            set_fog(i, gCurrentLevelHeader->fogNear, gCurrentLevelHeader->fogFar, gCurrentLevelHeader->fogUnk5E,
                    gCurrentLevelHeader->fogR, gCurrentLevelHeader->fogG, gCurrentLevelHeader->fogB,
                    gCurrentLevelHeader->fogUnk63);
        }
    }
    if (gCurrentLevelHeader->weatherCount > 0) {
        func_8005AD90(gCurrentLevelHeader->weatherType, gCurrentLevelHeader->weatherCount,
                      gCurrentLevelHeader->weatherVelX << 8, gCurrentLevelHeader->weatherVelY << 8,
                      gCurrentLevelHeader->weatherVelZ << 8, gCurrentLevelHeader->weatherIntensity * 257,
                      gCurrentLevelHeader->weatherOpacity * 257);
        weather_clip_planes(-1, -512);
    }
    if (gCurrentLevelHeader->unk69 == -1) {
        gCurrentLevelHeader->texture = load_texture((s32) gCurrentLevelHeader->texture);
        gCurrentLevelHeader->unkB8 = 0;
        gCurrentLevelHeader->unkBA = 0;
    }
    if (gCurrentLevelHeader->pulseLightData != -1) {
        gCurrentLevelHeader->pulseLightData = (s32) get_misc_asset(gCurrentLevelHeader->pulseLightData);
        init_pulsating_light_data((void *) gCurrentLevelHeader->pulseLightData);
    }
    bgdraw_primcolour(gCurrentLevelHeader->bgColorRed, gCurrentLevelHeader->bgColorGreen,
                      gCurrentLevelHeader->bgColorBlue);
    video_delta_reset();
    level_music_start(1.0f);
    i = 0;
    do {
        set_active_camera(i);
        cam_set_fov(gCurrentLevelHeader->cameraFOV, 1);
        i++;
    } while (i < 4);
    set_active_camera(0);
    if (gCurrentLevelHeader->unkE4 == -1) {
        gCurrentLevelHeader->unkE4 = 0;
    } else {
        gCurrentLevelHeader->unkE4 = (s32) get_misc_asset(gCurrentLevelHeader->unkE4);
    }
    check_reset_and_halt();
    if (gCurrentLevelHeader->unk107) {
        glitch_effect_start(1);
    }
    if (gCurrentLevelHeader->unkC2) {
        func_80054530();
    }
    if (gCurrentLevelHeader->unkC8) {
        func_80054530();
    }
    if (gCurrentLevelHeader->unkF7) {
        func_80054530(gCurrentLevelHeader);
    }
    if (gCurrentLevelHeader->unk101) {
        func_80054530_fi(12.0f, gCurrentLevelHeader->unk101 * 60);
    }
    check_reset_and_halt();
    func_8005342C(0x18);
    func_8005342C(0x1E);
    for (i = 0; i < 3; i++) {
        missing = TRUE;
        for (j = 0; j < 3; j++) {
            if (gCurrentLevelHeader->sounds[j] == D_800A31BC[i]) {
                missing = FALSE;
            }
        }
        if (missing && D_800A31B0[i] != 0) {
            D_800A31BC[i] = -1;
            sound_stop(D_800A31B0[i]);
        }
    }
    if (func_80054530() == 0) {
        gCurrentLevelHeader->fogNear = 900;
        gCurrentLevelHeader->fogFar = 920;
    }
    for (j = 0; j < 3; j++) {
        soundID = gCurrentLevelHeader->sounds[j];
        missing = TRUE;
        if (soundID != -1) {
            for (k = 0; k < 3; k++) {
                if (soundID == D_800A31BC[k]) {
                    missing = FALSE;
                } else if (D_800A31BC[k] == -1) {
                    slot = k;
                }
            }
            if (missing) {
                sound_play(soundID, &D_800A31B0[slot]);
                D_800A31BC[slot] = gCurrentLevelHeader->sounds[j];
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/44350", func_80043E3C);
#endif

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
extern u32 *gTempAssetTable;

/* JFG: load a level's name into gLevelName (0x8004471C). */
char *level_name(s32 level) {
    *gLevelName = 0;
    if (level < gNumberOfLevelHeaders) {
        if ((gTempAssetTable = asset_table_load(0x1E)) != NULL) {
            asset_load(0x1F, (u32) gLevelName, gTempAssetTable[level], 0x20);
            mempool_free(gTempAssetTable);
        }
    }
    return gLevelName;
}

void glitch_effect_stop(s32 arg0);
void level_effects_reset(void);
void bgdraw_primcolour(u8 red, u8 green, u8 blue);
void music_jingle_stop(void);
void lights_free(void);
void coronas_free_all(void);
void free_track(void);
void collision_level_buffers_free(void);
void audspat_reset(void);
void sound_volume_change(s32 behaviour);
void weather_free(void);
void tex_free(void *tex);
void runlink_unload_all(void);
void effect_slots_free(void);
extern s32 D_800A089C;

/* JFG: free the current level (0x800447B4). */
void level_free(void) {
    glitch_effect_stop(1);
    level_effects_reset();
    bgdraw_primcolour(0, 0, 0);
    mempool_free(gCurrentLevelHeader);
    music_jingle_stop();
    lights_free();
    coronas_free_all();
    free_track();
    collision_level_buffers_free();
    audspat_reset();
    D_800A089C = 0;
    sound_volume_change(0);
    if (gCurrentLevelHeader->weatherCount > 0) {
        weather_free();
    }
    if (gCurrentLevelHeader->unk69 == -1) {
        tex_free(gCurrentLevelHeader->texture);
    }
    runlink_unload_all();
    effect_slots_free();
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
