/*
 * Level loading and the current level's header. Functions in C are from the
 * Diddy Kong Racing decompilation (CC0 1.0), src/game.c, or marked "JFG".
 */
#include "common.h"

/* DKR include/level.h (reduced to the fields used here) */
typedef struct LevelHeader {
    /* 0x00 */ u8 pad0[0x6C];
    /* 0x6C */ u8 race_type;
    /* 0x6D */ u8 pad6D[0xE3 - 0x6D];
    /* 0xE3 */ u8 unkE3;
} LevelHeader;

extern LevelHeader *gCurrentLevelHeader;
extern s32 D_800FB114;
extern u8 D_800FB170;

INCLUDE_ASM("asm/nonmatchings/44350", func_80043750);

/* Size: 5 bytes */
typedef struct LevelListEntry {
    /* 0x00 */ u8 type;
    /* 0x01 */ s8 world;
    /* 0x02 */ u8 region;
    /* 0x03 */ u8 unk3;
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

INCLUDE_ASM("asm/nonmatchings/44350", func_80043C78);

INCLUDE_ASM("asm/nonmatchings/44350", func_80043CC0);

INCLUDE_ASM("asm/nonmatchings/44350", func_80043D08);

INCLUDE_ASM("asm/nonmatchings/44350", func_80043D70);

INCLUDE_ASM("asm/nonmatchings/44350", func_80043E3C);

INCLUDE_ASM("asm/nonmatchings/44350", func_8004455C);

INCLUDE_ASM("asm/nonmatchings/44350", func_80044610);

/* JFG: the address of the current level's colour cycle data. */
void *func_800446B0(void) {
    return &D_800FB170;
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

INCLUDE_ASM("asm/nonmatchings/44350", func_8004471C);

INCLUDE_ASM("asm/nonmatchings/44350", func_800447B4);

INCLUDE_ASM("asm/nonmatchings/44350", func_80044898);

INCLUDE_ASM("asm/nonmatchings/44350", func_80044928);

INCLUDE_ASM("asm/nonmatchings/44350", func_800449C8);

INCLUDE_ASM("asm/nonmatchings/44350", func_80044A1C);

INCLUDE_ASM("asm/nonmatchings/44350", func_80044A6C);

typedef struct ObjectFlags {
    u8 unk0;
    u8 bits[1]; // variable length
} ObjectFlags;
extern ObjectFlags *D_800A31C4;

/* JFG: set bit `flag` of the level's object flags. */
void func_80044AB4(s32 flag) {
    if (D_800A31C4 != NULL) {
        D_800A31C4->bits[flag >> 3] |= (1 << (flag & 7));
    }
}

INCLUDE_ASM("asm/nonmatchings/44350", func_80044AE8);
