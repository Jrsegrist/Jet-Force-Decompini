#include "common.h"
#include "joypad.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern OSMesgQueue gNMIMesgQueue;
extern s32 gGameMode;
extern s32 gNMIMesgBuf;          // Official Name: resetPressed;
extern s32 sControllerStatus;
extern s32 gSaveDataFlags; // Official Name: load_save_flags

/* JFG's save flag bits (DKR has them elsewhere) */
#define SAVE_DATA_FLAG_WRITE_SAVE_FILE_NUMBER_BITS 0x38
#define SAVE_DATA_FLAG_WRITE_SAVE_DATA 0x40000000
#define GAMEMODE_INGAME 0

extern s32 D_800A331C;
extern s32 D_800A3320;
extern s16 D_800A3260;
extern s16 gPlayableMapId;
extern s32 sBootDelayTimer;
s32 leveltable_world(s32 level);
s32 func_800841A0(void);
s32 func_800841CC(void);
void func_80044DFC(void);
void func_8004665C(s32 arg0, s32 level, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void check_reset_and_halt(void);
void func_80044FAC(void);
void thread3_verify_stack(void);
s32 input_update(s32 saveDataFlags, s32 updateRate);

/**
 * Main looping function for the main thread.
 * JFG: hangs on a PAL console, runs two boot checks, and starts at a level
 * instead of checking for a reset in the loop.
 * Official Name: mainThread
 */
void thread3_main(UNUSED void *unused) {
    u32 *mem;
    if (osTvType == OS_TV_PAL) {
        // Clear memory downwards from the end of the first 4 MB until it crashes.
        mem = (u32 *) 0x80400000;
        while (1) {
            *--mem = 0;
        }
    }
    D_800A331C = func_800841A0();
    D_800A3320 = func_800841CC();
    func_80044DFC();
    gSaveDataFlags = input_update(gSaveDataFlags, 0);
    sBootDelayTimer = 0;
    gGameMode = 6;
    func_8004665C(0, D_800A3260, 0, 0, 1, 0);
    while (1) {
        check_reset_and_halt();
        func_80044FAC();
        thread3_verify_stack();
    }
}

extern s32 D_800A3290;
void audioStopThread(void);
void func_80054B44(void);
void rumble_stop_all(s32 arg0);
void func_8004B56C(s32 updateRate);
s32 is_reset_pressed(void);
void __osSpSetStatus(u32 data);

/* JFG: on reset, stop the audio, wait a little, stop the RSP/RDP and the rumble paks, then hang (0x80044C20). */
void check_reset_and_halt(void) {
    if (is_reset_pressed() != 0) {
        audioStopThread();
        if (D_800A3290 != 0) {
            osSetTime(0);
            while (osGetTime() < 0x22ADD7) {
            }
        }
        __osSpSetStatus(0xAAAA82);
        osDpSetStatus(0x1D6);
        func_80054B44();
        rumble_stop_all(1);
        func_8004B56C(2);
        func_8004B56C(2);
        func_8004B56C(2);
        while (1) {
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80044CF0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044DFC);

extern s32 D_800A328C;
extern s16 D_800FD7A4;
extern s16 D_800A3280;
extern u8 gCurrDisplayList;
void func_80054530();

void func_80044F24(s32 step) {
    s32 alpha;

    if (step < D_800A328C) {
        D_800A328C -= step;
        alpha = D_800A328C * 4;
        if (alpha >= 0x100) {
            alpha = 0xFF;
        }
        func_80054530(&gCurrDisplayList, D_800FD7A4, leveltable_world(gPlayableMapId), D_800A3280, alpha);
    } else {
        D_800A328C = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80044FAC);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004581C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045914);

extern s8 D_800FD757[];

s32 func_80045A14(s32 index) {
    if (index < 0 || index >= 8) {
        return 1;
    }
    return D_800FD757[index * 8];
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80045A44);

extern s16 D_800FD794;
extern s16 D_800FD796;
extern s16 D_800FD798;
extern s16 D_800FD79A;
extern s16 D_800FD79C;
extern s16 D_800FD79E;
extern s16 D_800FD7A0;
extern s16 D_800FD7A2;
extern s16 D_800A3230;
extern s16 D_800A3234;
extern s16 D_800A3238;

void func_80045B98(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 time) {
    D_800FD794 = a0;
    D_800FD796 = a1;
    D_800FD798 = a2;
    D_800FD79A = a3;
    D_800FD79C = a4;
    D_800FD79E = a5;
    D_800FD7A0 = a6;
    D_800FD7A2 = a7;
    if (time < 0) {
        D_800A3234 = -time;
        D_800A3238 = 1;
    } else {
        D_800A3234 = time;
        D_800A3238 = 0;
    }
    D_800A3230 = D_800A3234;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80045C38);

s16 func_80045DC0(void) {
    return D_800A3230;
}

extern s16 D_800A3220;
extern s16 D_800A3224;
extern s16 D_800A3228;
extern s16 D_800A322C;

void func_80045DD0(s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    *x1 = D_800A3220;
    *y1 = D_800A3224;
    *x2 = D_800A3228;
    *y2 = D_800A322C;
}

extern s8 D_800A4FB0;
s32 get_language(void);
void set_language(s32 language);
s32 options_screen_mode_get(void);
void func_8005A834(s32 arg0);
void func_8005A8C4(void);
s8 options_screen_offset_get(void);
void options_screen_offset_set(s32 adjust);
u8 func_8005A98C(void);
void func_8005A99C(s32 mode);
u16 func_8005A9E4(void);
void func_8005A9F4(s32 volume);
u16 options_music_volume_get(void);
void options_music_volume_set(s32 volume);
void func_80047608(s32 arg0, s32 level, s32 arg2);

#ifdef NON_MATCHING
/* NON_MATCHING: in the START loop the target puts the andi before i++ and ORs as (start, held) */
/* JFG: the boot screen; START on any controller sets D_800A4FB0, after 10 frames the settings are applied. */
void func_80045E04(UNUSED s32 updateRate) {
    s32 i;
    s32 start;

    for (i = 0, start = 0; i < 4; i++) {
        start = start | (input_held(i) & START_BUTTON);
    }
    if (start) {
        D_800A4FB0 = 1;
    }
    if (++sBootDelayTimer >= 10) {
        set_language(get_language());
        func_8005A834(options_screen_mode_get());
        options_screen_offset_set(options_screen_offset_get());
        func_8005A99C(func_8005A98C());
        func_8005A9F4(func_8005A9E4());
        options_music_volume_set(options_music_volume_get());
        func_8005A8C4();
        func_80047608(2, 0, 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80045E04);
#endif

INCLUDE_ASM("asm/nonmatchings/45750", func_80045F00);

void func_80046334(s32 *level, s32 *a, s32 *b) {
    s32 value = *level;
    if (value == 0xF5 || value == 0xFA || value == 0x11D || value == 0x16B || value == 0x196) {
        *level = 0xF5;
        *a = 0;
        *b = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80046378);

/* JFG: where some levels send the player (level, two zeros and D_800FD7A4); FALSE for the others. */
s32 func_80046550(s16 level, s16 *newLevel, s16 *arg2, s16 *arg3, s16 *arg4) {
    switch (level) {
        case 0xD2:
            *newLevel = 0xF9;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0xB5:
            *newLevel = 0x13;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0xC3:
            *newLevel = 0xC2;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0x182:
            *newLevel = 0x18B;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
        case 0xE9:
            *newLevel = 0x18C;
            *arg2 = 0;
            *arg3 = 0;
            *arg4 = D_800FD7A4;
            return TRUE;
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_8004665C);

extern s16 D_800A3254;

void func_800468A4(s32 arg0) {
    D_800A3254 = arg0;
}

extern s16 D_800A3248;

s16 func_800468B0(void) {
    return D_800A3248;
}

extern s16 D_800A325C;

void func_800468C0(s32 arg0) {
    D_800A325C = arg0;
}

extern s16 D_800A3260;

s16 func_800468CC(void) {
    return D_800A3260;
}

extern s16 D_800A3250;

s16 func_800468DC(void) {
    return D_800A3250;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_800468EC);

extern u8 D_800A326C;

void func_80047570(void) {
    D_800A326C = 0x1;
}

/**
 *  Sets the current game mode.
 *  Official Name: mainSetMode?
 */
void set_game_mode(s32 changeTo) {
    gGameMode = changeTo;
}

extern s32 D_800FD7C8;
extern s8 D_800FD7BC;

/* JFG: a level and the flags needed to start it */
typedef struct LevelUnlock {
    s16 level;
    u16 flags;
} LevelUnlock;
extern LevelUnlock D_800A337C[];
extern u8 D_800FF391;

/* Starts the first level of the unlock list whose required flags are all set (0x8004758C). */
void load_first_unlocked_level(void) {
    LevelUnlock *entry;

    gGameMode = 1;
    D_800FD7C8 = 1;
    entry = D_800A337C;
    while ((D_800FF391 & entry->flags) != entry->flags) {
        entry++;
    }
    func_8004665C(entry->level, 0, 0, 3, 1, 0);
}

void func_80047608(s32 arg0, s32 level, UNUSED s32 arg2) {
    gGameMode = 1;
    D_800FD7C8 = 1;
    if (D_800FD7BC == 0) {
        D_800FD7BC = 1;
    }
    func_8004665C(level, D_800A3260, 0, arg0, 1, 0);
}

/* JFG: one character's data in the game state (0x76 bytes) */
typedef struct CharacterData {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ u16 unk4_hi : 11;
    /* 0x04 */ u16 unk4_lo : 5;
    /* 0x06 */ u8 pad6[0x71 - 6];
    /* 0x71 */ s8 unk71;
    /* 0x72 */ u8 pad72[4];
} CharacterData;

typedef struct GameStats {
    /* 0x000 */ u8 unk0;
    /* 0x001 */ u8 unk1;
    /* 0x002 */ s8 unk2;
    /* 0x003 */ u8 pad3;
    /* 0x004 */ u8 unk4;
    /* 0x005 */ u8 unk5;
    /* 0x006 */ u8 unk6;
    /* 0x007 */ u8 pad7[0x5C - 7];
    /* 0x05C */ u16 unk5C[64];
    /* 0x0DC */ u16 unkDC[64];
    /* 0x15C */ CharacterData chars[4];
    /* 0x334 */ u8 pad334[0x474 - 0x334];
    /* 0x474 */ u32 shots[16];
    /* 0x4B4 */ u32 hits[16];
    /* 0x4F4 */ u16 kills[16];
} GameStats;
extern GameStats *gSettingsPtr;

void func_80047674(void) {
    gSettingsPtr->unk1 = gSettingsPtr->chars[gSettingsPtr->unk0 & 3].unk4_hi;
    gSettingsPtr->unk2 = leveltable_world(gSettingsPtr->unk1);
}

INCLUDE_ASM("asm/nonmatchings/45750", func_800476F0);

/* Counts a kill with a weapon in the game statistics (0x80047868). */
void stats_kill_add(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && gSettingsPtr->kills[weapon] < 0xFFFF) {
        gSettingsPtr->kills[weapon]++;
    }
}

/* Counts a shot with a weapon in the game statistics (0x800478A4). */
void stats_shot_add(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && gSettingsPtr->shots[weapon] < 0xFFFFFFFF) {
        gSettingsPtr->shots[weapon]++;
    }
}

/* Counts a hit with a weapon in the game statistics (0x800478E0). */
void stats_hit_add(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && gSettingsPtr->hits[weapon] < 0xFFFFFFFF) {
        gSettingsPtr->hits[weapon]++;
    }
}

extern u8 D_800FEFD0[4];
extern CharacterData D_800FEFD8[4];
void func_80047AE0(CharacterData *chr, s32 arg1);

void func_8004791C(void) {
    s32 i;
    for (i = 0; i < 4; i++) {
        func_80047AE0(&D_800FEFD8[i], 0);
        D_800FEFD0[i] = 0;
    }
}

/* JFG: one player's data in multiplayer (0xA0 bytes) */
typedef struct MultiPlayer {
    /* 0x00 */ s16 unk0[4];
    /* 0x08 */ s16 unk8[4];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 pad14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ CharacterData chr;
    /* 0x8C */ s8 unk8C;
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ s32 unk90;
    /* 0x94 */ s16 unk94[4];
    /* 0x9C */ s16 unk9C;
    /* 0x9E */ u8 pad9E[2];
} MultiPlayer;
extern MultiPlayer D_800FED50[4];
extern u8 D_800A4FC4;
extern u8 D_800A4FD0;
extern u8 D_800A4FE0;

/* The unk94 and unk9C fields of D_800FED50[0..3], referenced as their own symbols. */
extern s16 D_800FEDE4, D_800FEDE6, D_800FEDE8, D_800FEDEA, D_800FEDEC;
extern s16 D_800FEE84, D_800FEE86, D_800FEE88, D_800FEE8A, D_800FEE8C;
extern s16 D_800FEF24, D_800FEF26, D_800FEF28, D_800FEF2A, D_800FEF2C;
extern s16 D_800FEFC4, D_800FEFC6, D_800FEFC8, D_800FEFCA, D_800FEFCC;

void func_8004797C(void) {
    D_800FEDEC = 0;
    D_800FEDE4 = 0;
    D_800FEDE6 = 0;
    D_800FEDE8 = 0;
    D_800FEDEA = 0;
    D_800FEE8C = 0;
    D_800FEE84 = 0;
    D_800FEE86 = 0;
    D_800FEE88 = 0;
    D_800FEE8A = 0;
    D_800FEF2C = 0;
    D_800FEF24 = 0;
    D_800FEF26 = 0;
    D_800FEF28 = 0;
    D_800FEF2A = 0;
    D_800FEFCC = 0;
    D_800FEFC4 = 0;
    D_800FEFC6 = 0;
    D_800FEFC8 = 0;
    D_800FEFCA = 0;
}

void func_80047A20(void) {
    s32 i;
    s32 j;
    s32 n;
    u8 *src;
    u8 *dst;
    MultiPlayer *mp;
    CharacterData *chr;
    u8 *id;

    for (i = 0; i < 4; i++) {
        mp = &D_800FED50[i];
        chr = &D_800FEFD8[i];
        id = &D_800FEFD0[i];
        for (j = 0; j < 4; j++) {
            D_800FED50[i].unk0[j] = 0;
            D_800FED50[i].unk8[j] = 0;
        }
        mp->unk10 = 0;
        mp->unk13 = D_800A4FE0;
        mp->unk8C = i;
        mp->unk90 = 0;
        mp->unk15 = 0;
        mp->unk12 = *id;
        src = (u8 *) chr;
        dst = (u8 *) &mp->chr;
        n = sizeof(CharacterData);
        while (n--) {
            *dst++ = *src++;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80047AE0);

/* Returns the game settings (the save data being played) (0x80047C0C). */
GameStats *get_settings(void) {
    return gSettingsPtr;
}

#ifdef NON_MATCHING
/* NON_MATCHING: the target schedules `character & 3` into the first delay slot (mine: the lui of gSettingsPtr) */
u8 *func_80047C1C(s32 player, s32 character) {
    if (D_800A4FC4 != 0 && D_800A4FD0 >= 2) {
        if (player < 0 || player >= D_800A4FD0) {
            player = 0;
        }
        return (u8 *) &D_800FED50[player].chr;
    }
    return (u8 *) &gSettingsPtr->chars[character & 3];
}
#else
INCLUDE_ASM("asm/nonmatchings/45750", func_80047C1C);
#endif

u8 *func_80047C1C(s32 player, s32 character);
extern s32 gCharItemsChanged; // JFG: characters whose items changed (one bit each)

/* JFG: give a player's character an item (0x80047C9C). */
void character_item_give(s32 player, s32 character, s32 item) {
    u8 *chr = func_80047C1C(player, character);

    if (chr != NULL) {
        (chr + 0x66)[item >> 3] |= 0x80 >> (item & 7);
        gCharItemsChanged |= 1 << (character & 3);
    }
}

/* JFG: take an item from a player's character (0x80047D0C). */
void character_item_take(s32 player, s32 character, s32 item) {
    u8 *chr = func_80047C1C(player, character);

    if (chr != NULL) {
        (chr + 0x66)[item >> 3] &= ~(0x80 >> (item & 7));
        gCharItemsChanged |= 1 << (character & 3);
    }
}

/* JFG: whether a player's character has an item (a bit array at 0x66, highest bit first) (0x80047D80). */
s32 character_has_item(s32 player, s32 character, s32 item) {
    UNUSED s32 pad;
    u8 *chr;
    s32 has = 0;

    chr = func_80047C1C(player, character);
    if (chr != NULL) {
        if ((chr + 0x66)[item >> 3] & (0x80 >> (item & 7))) {
            has = 1;
        }
    }
    return has;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80047DD8);

/* JFG: whether a player's character has a weapon (a bit mask at 0xA) (0x80047E44). */
s32 character_has_weapon(s32 player, s32 character, s32 weapon) {
    u8 *chr = func_80047C1C(player, character);

    if (chr != NULL) {
        if (*(u16 *) (chr + 0xA) & (1 << weapon)) {
            return TRUE;
        }
    }
    return FALSE;
}

/* JFG: read one of the game's 0x480 flags (a bit array at 0x30, lowest bit first) (0x80047E8C). */
s32 settings_flag_get(GameStats *game, s32 flag) {
    if (flag >= 0 && flag < 0x480) {
        if (((u8 *) game)[(flag >> 3) + 0x30] & (1 << (flag & 7))) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Returns one of the 0x480 game flags of the current settings (0x80047ED0). */
s32 game_flag_get(s32 flag) {
    return settings_flag_get(gSettingsPtr, flag);
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80047EFC);

typedef struct LevelHeader {
    /* 0x000 */ u8 pad0[0x103];
    /* 0x103 */ u8 region;
} LevelHeader;
LevelHeader *level_header(void);

void func_80048130(s32 bit, s32 mode) {
    LevelHeader *header = level_header();
    s32 mask = 1 << (bit & 0xF);

    if (!(gSettingsPtr->unk5C[header->region] & mask) && !(gSettingsPtr->unkDC[header->region] & mask)) {
        switch (mode) {
            case 1:
                gSettingsPtr->unk5C[header->region] |= mask;
                gSettingsPtr->unk4++;
                gSettingsPtr->unk6--;
                break;
            case 2:
                gSettingsPtr->unkDC[header->region] |= mask;
                gSettingsPtr->unk5++;
                gSettingsPtr->unk6--;
                break;
        }
    }
}

s32 func_8004822C(s32 bit) {
    LevelHeader *header = level_header();
    s32 mask = 1 << (bit & 0xF);
    if (gSettingsPtr->unk5C[header->region] & mask) {
        return 1;
    }
    if (gSettingsPtr->unkDC[header->region] & mask) {
        return 2;
    }
    return 0;
}

void func_800482A0(s32 player, s32 arg1, s32 arg2) {
    if (arg1 >= 0) {
        CharacterData *chr = &gSettingsPtr->chars[player & 3];
        gSettingsPtr->unk1 = arg1;
        chr->unk4_hi = arg1;
        chr->unk71 = arg2;
    }
    gSettingsPtr->unk2 = arg2;
}

extern s8 gIsPaused;

/* Returns the pause state (nonzero while paused) (0x800482F8). */
s8 is_game_paused(void) {
    return gIsPaused;
}

s32 runlink_module_loaded(s32 module);
void func_8005342C(s32 module);
void func_80000450(s32 mode);
extern s8 gIsPaused;

/* Sets the pause state and changes the sound volume to match (0x80048308). */
void set_game_paused(s32 mode) {
    gIsPaused = mode;
    if (gIsPaused == 0) {
        if (runlink_module_loaded(0x13) != 0) {
            func_8005342C(0x13);
        }
        func_80000450(0);
        return;
    }
    func_80000450(1);
}

/**
 * Sets and returns (nonzero) the message set when pressing the reset button.
 * Official name: mainResetPressed
 */
s32 is_reset_pressed(void) {
    if (gNMIMesgBuf == 0) {
        gNMIMesgBuf = (s32) ((osRecvMesg(&gNMIMesgQueue, NULL, OS_MESG_NOBLOCK) + 1) != 0);
    }
    return gNMIMesgBuf;
}

/* Returns the current level (map) number (0x800483B8). */
s16 get_ingame_map_id(void) {
    return gPlayableMapId;
}

/* Marks a save file to be read (0x800483C8). */
void mark_read_save_file(s32 saveFileIndex) {
    gSaveDataFlags &= ~7;
    gSaveDataFlags |= (0x80000000 | (saveFileIndex & 7));
}

extern s32 gSavefileData;

/* Marks all save files to be read into the given buffer (0x800483F8). */
void mark_read_all_save_files(s32 games) {
    gSavefileData = games;
    gSaveDataFlags |= 0x8000000;
}

extern s32 gSavefileData;

/* Returns the buffer all save files are read into (0x8004841C). */
s32 get_all_save_files_ptr(void) {
    return gSavefileData;
}

/**
 * Forcefully marks a flag to write a save file.
 * JFG: the file number is three bits at bit 3.
 * Official Name: mainSaveGame
 */
void force_mark_write_save_file(s32 saveFileIndex) {
    gSaveDataFlags &= ~SAVE_DATA_FLAG_WRITE_SAVE_FILE_NUMBER_BITS;
    gSaveDataFlags |= (SAVE_DATA_FLAG_WRITE_SAVE_DATA | ((saveFileIndex & 7) << 3));
}

/**
 * Marks a flag to write a save file, as long as the game is in its main mode.
 * JFG: no tracks mode check.
 * Official Name: mainSaveGame2
 */
void safe_mark_write_save_file(s32 saveFileIndex) {
    if (gGameMode == GAMEMODE_INGAME) {
        gSaveDataFlags &= ~SAVE_DATA_FLAG_WRITE_SAVE_FILE_NUMBER_BITS;
        gSaveDataFlags |= (SAVE_DATA_FLAG_WRITE_SAVE_DATA | ((saveFileIndex & 7) << 3));
    }
}

/* Marks a save file to be erased (0x800484A4). */
void mark_save_file_to_erase(s32 saveFileIndex) {
    gSaveDataFlags = ((saveFileIndex & 7) << 6) | 0x20000000;
}

extern s32 gSaveDataFlags;

/* Marks all save files to be erased (0x800484C0). */
void mark_all_save_files_to_erase(void) {
    gSaveDataFlags = 0x10000000;
}

extern s32 gSaveDataFlags;

/* Marks the whole save flash to be erased (0x800484D0). */
void mark_erase_all_save_data(void) {
    gSaveDataFlags = 0x1000000;
}

/* Marks the settings to be read (0x800484E0). */
void mark_read_eeprom_settings(void) {
    gSaveDataFlags |= 0x4000000;
}

/* Marks the settings to be written (0x800484FC). */
void mark_write_eeprom_settings(void) {
    gSaveDataFlags |= 0x2000000;
}

extern s16 D_800A32BC;

void func_80048518(s32 arg0) {
    D_800A32BC = arg0;
}

/**
 * Returns TRUE if the game doesn't detect any controllers.
 * Official name: mainDemoOnly
 */
s32 is_controller_missing(void) {
    if (sControllerStatus == CONTROLLER_MISSING) {
        return TRUE;
    } else {
        return FALSE;
    }
}

extern u8 D_800A4FCC;

u8 func_80048548(void) {
    return D_800A4FCC;
}

s32 leveltable_type(s32 level);
extern u8 D_800A4FB4;

u8 func_80048558(void) {
    s32 type = leveltable_type(gPlayableMapId);
    if (type == 1 || type == 2) {
        return 0;
    }
    if (D_800A4FC4 == 0 && D_800A4FB4 == 0) {
        return 1;
    }
    return D_800A4FD0;
}

extern s16 D_800A3240;

s32 func_800485C8(s32 player) {
    if (D_800A4FD0 < 2 || (D_800A4FC4 == 0 && D_800A4FB4 == 0)) {
        return D_800A3240;
    }
    return D_800FED50[player & 3].unk12;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80048630);

INCLUDE_ASM("asm/nonmatchings/45750", func_800487B8);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004883C);
