/*
 * Menus. Functions in C are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/menu.c, or marked "JFG"; see CREDITS.md.
 */
#include "common.h"

extern u8 gLanguage;

INCLUDE_ASM("asm/nonmatchings/59060", func_80058460);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058590);

INCLUDE_ASM("asm/nonmatchings/59060", func_800586E0);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005886C);

void func_800586E0(void);
void func_8005886C(void);
void func_80059A04(void);
extern u8 D_800A51B0; // JFG: the front end mode
extern u8 D_800A5058;
extern u8 D_800A505C;
extern u8 D_800A4FC4; // JFG: a multiplayer game
extern u8 D_800A4FD0; // JFG: the number of players
extern u8 D_800A4FCC; // JFG: the number of cameras

/* JFG: change the front end mode; mode 0 goes back to one player. */
void func_800589F0(s32 mode) {
    func_800586E0();
    D_800A51B0 = mode;
    func_8005886C();
    func_80059A04();
    D_800A5058 = 0;
    D_800A505C = 1;
    if (mode == 0) {
        D_800A4FC4 = 0;
        D_800A4FD0 = 1;
        D_800A4FCC = 1;
    }
}

u8 func_80058A5C(void) {
    return D_800A51B0;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_80058A6C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058C54);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058D5C);

s8 func_800482F8(void); // JFG: the pause mode
extern s32 D_800A5054;  // JFG: the front end timer

/* JFG: advance the front end timer (unless paused) up to limit; TRUE once it got there. */
s32 func_80058E78(UNUSED s32 arg0, s32 limit, s32 delta) {
    s32 done = FALSE;

    if (!func_800482F8()) {
        D_800A5054 += delta;
    }
    if (D_800A5054 >= limit) {
        D_800A5054 = limit;
        done = TRUE;
    }
    return done;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_80058EF0);

INCLUDE_ASM("asm/nonmatchings/59060", func_800595F4);

typedef struct FrontRect {
    s16 x1, y1, x2, y2;
    s32 colour;
} FrontRect;
void func_800595F4(s32 arg0, s32 count, FrontRect *rects, s32 arg3);

/* JFG: draw one rectangle through the list function. */
void func_80059814(s32 arg0, s32 x1, s32 y1, s32 x2, s32 y2, s32 colour) {
    FrontRect rect;

    rect.x1 = x1;
    rect.y1 = y1;
    rect.x2 = x2;
    rect.y2 = y2;
    rect.colour = colour;
    func_800595F4(arg0, 1, &rect, 1);
}

INCLUDE_ASM("asm/nonmatchings/59060", func_80059864);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059938);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059A04);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059A98);

void func_80059D2C(s16 item);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059CCC);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059D2C);

void func_80059E7C(s16 item);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059E1C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059E7C);

void func_8005A048(s16 item);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059FE8);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A048);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A0D0);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A51C);

s32 func_8005A794(void) {
    return 0;
}

s32 func_8005A79C(void) {
    return 0;
}

extern char **D_800A51C8;
extern u16 D_800A5918[];

char *func_8005A7A4(s32 world) {
    return D_800A51C8[D_800A5918[world]];
}

/**
 * Returns the current language being used by the game.
 * JFG: a stored setting (DKR decodes it from the EEPROM settings).
 * Official Name: frontGetLanguage
 */
s32 get_language(void) {
    return gLanguage;
}

void func_80058460(s32 language);

void func_8005A7E0(s32 language) {
    gLanguage = language;
    func_80058460(language);
}

/* JFG: settings flags (the language and volume bytes below sit in the same block). */
typedef struct SettingsFlags {
    /* 0x00 */ u8 pad0[0x6C];
    /* 0x6C */ u32 unk6C_0 : 1;
    /* 0x6C */ u32 unk6C_1 : 1; // JFG: screen mode bit 0 (also read by fb_init_vi)
    /* 0x6C */ u32 unk6C_2 : 1; // JFG: screen mode bit 1
    /* 0x6C */ u32 unk6C_3 : 1;
    /* 0x6C */ u32 unk6C_4 : 28;
} SettingsFlags;
extern SettingsFlags D_800FF318;

/* JFG: the screen mode (0-3) from two settings bits. */
s32 func_8005A804(void) {
    s32 mode = 0;

    if (D_800FF318.unk6C_1) {
        mode = 1;
    }
    if (D_800FF318.unk6C_2) {
        mode |= 2;
    }
    return mode;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A834);

extern u8 D_800A51A4;
extern u8 D_800A51A8;

/* JFG: remember the current screen mode byte (func_8005A8D8 returns it). */
void func_8005A8C4(void) {
    D_800A51A4 = D_800A51A8;
}

u8 func_8005A8D8(void) {
    return D_800A51A4;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A8E8);

extern s8 D_800FF387;

s8 func_8005A950(void) {
    return D_800FF387;
}

s8 func_80055198(void);
void func_800551A8(s32 adjust);
extern s8 D_800FF387;

void func_8005A960(s32 adjust) {
    func_800551A8(adjust);
    D_800FF387 = func_80055198();
}

extern u8 D_800FF38C;

u8 func_8005A98C(void) {
    return D_800FF38C;
}

extern u8 D_800A5938[4];
extern u8 D_800FF38C;
void func_8008B2A0(u8 type, s32 mode);

void func_8005A99C(s32 mode) {
    if (mode < 0) {
        mode = 0;
    }
    if (mode >= 4) {
        mode = 3;
    }
    D_800FF38C = mode;
    func_8008B2A0(D_800A5938[mode], mode);
}

extern u16 D_800FF388;

u16 func_8005A9E4(void) {
    return D_800FF388;
}

extern u16 D_800FF388;
void func_80086510(s32 volume);

void func_8005A9F4(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 256) {
        volume = 256;
    }
    D_800FF388 = volume;
    func_80086510(volume);
}

extern u16 D_800FF38A;

u16 func_8005AA30(void) {
    return D_800FF38A;
}

extern u16 D_800FF38A;
void func_80001050(s32 volume);

void func_8005AA40(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 256) {
        volume = 256;
    }
    D_800FF38A = volume;
    func_80001050(volume);
}

s32 func_8005AA7C(void) {
    s32 value = D_800FF318.unk6C_3;
    return value;
}

void func_8005AA94(s32 value) {
    D_800FF318.unk6C_3 = value & 1;
}

extern u8 D_800FF38D[4];

u8 func_8005AAB8(s32 index) {
    return D_800FF38D[index & 3];
}

void func_8005AAD0(s32 index, s32 value) {
    D_800FF38D[index & 3] = value & 1;
}

extern u8 D_800FF311;

void func_8005AAE8(s32 arg0) {
    D_800FF311 = arg0;
}
