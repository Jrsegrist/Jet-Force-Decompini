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

INCLUDE_ASM("asm/nonmatchings/45750", func_80044B50);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044C20);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044CF0);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044DFC);

INCLUDE_ASM("asm/nonmatchings/45750", func_80044F24);

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

INCLUDE_ASM("asm/nonmatchings/45750", func_80045B98);

INCLUDE_ASM("asm/nonmatchings/45750", func_80045C38);

extern s16 D_800A3230;

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

INCLUDE_ASM("asm/nonmatchings/45750", func_80045E04);

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

INCLUDE_ASM("asm/nonmatchings/45750", func_80046550);

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

INCLUDE_ASM("asm/nonmatchings/45750", func_8004758C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047608);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047674);

INCLUDE_ASM("asm/nonmatchings/45750", func_800476F0);

typedef struct GameStats {
    /* 0x000 */ u8 pad0[0x474];
    /* 0x474 */ u32 shots[16];
    /* 0x4B4 */ u32 hits[16];
    /* 0x4F4 */ u16 kills[16];
} GameStats;
extern GameStats *D_800FD7D4;

void func_80047868(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && D_800FD7D4->kills[weapon] < 0xFFFF) {
        D_800FD7D4->kills[weapon]++;
    }
}

void func_800478A4(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && D_800FD7D4->shots[weapon] < 0xFFFFFFFF) {
        D_800FD7D4->shots[weapon]++;
    }
}

void func_800478E0(s32 weapon) {
    if (weapon >= 0 && weapon < 16 && D_800FD7D4->hits[weapon] < 0xFFFFFFFF) {
        D_800FD7D4->hits[weapon]++;
    }
}

INCLUDE_ASM("asm/nonmatchings/45750", func_8004791C);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004797C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047A20);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047AE0);

GameStats *func_80047C0C(void) {
    return D_800FD7D4;
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80047C1C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047C9C);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047D0C);

u8 *func_80047C1C(s32 player, s32 character);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047D80);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047DD8);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047E44);

INCLUDE_ASM("asm/nonmatchings/45750", func_80047E8C);

s32 func_80047E8C(GameStats *game, s32 flag);

s32 func_80047ED0(s32 flag) {
    return func_80047E8C(D_800FD7D4, flag);
}

INCLUDE_ASM("asm/nonmatchings/45750", func_80047EFC);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048130);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004822C);

INCLUDE_ASM("asm/nonmatchings/45750", func_800482A0);

extern s8 D_800FD7BD;

s8 func_800482F8(void) {
    return D_800FD7BD;
}

s32 func_80053370(s32 module);
void func_8005342C(s32 module);
void func_80000450(s32 mode);
extern s8 D_800FD7BD;

void func_80048308(s32 mode) {
    D_800FD7BD = mode;
    if (D_800FD7BD == 0) {
        if (func_80053370(0x13) != 0) {
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

extern s16 D_800A323C;

s16 func_800483B8(void) {
    return D_800A323C;
}

void func_800483C8(s32 saveFileIndex) {
    gSaveDataFlags &= ~7;
    gSaveDataFlags |= (0x80000000 | (saveFileIndex & 7));
}

extern s32 D_800FD7D8;

void func_800483F8(s32 games) {
    D_800FD7D8 = games;
    gSaveDataFlags |= 0x8000000;
}

extern s32 D_800FD7D8;

s32 func_8004841C(void) {
    return D_800FD7D8;
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

void func_800484A4(s32 saveFileIndex) {
    gSaveDataFlags = ((saveFileIndex & 7) << 6) | 0x20000000;
}

extern s32 gSaveDataFlags;

void func_800484C0(void) {
    gSaveDataFlags = 0x10000000;
}

extern s32 gSaveDataFlags;

void func_800484D0(void) {
    gSaveDataFlags = 0x1000000;
}

void func_800484E0(void) {
    gSaveDataFlags |= 0x4000000;
}

void func_800484FC(void) {
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

INCLUDE_ASM("asm/nonmatchings/45750", func_80048558);

INCLUDE_ASM("asm/nonmatchings/45750", func_800485C8);

INCLUDE_ASM("asm/nonmatchings/45750", func_80048630);

INCLUDE_ASM("asm/nonmatchings/45750", func_800487B8);

INCLUDE_ASM("asm/nonmatchings/45750", func_8004883C);
