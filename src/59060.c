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
extern u8 gMenuMode; // JFG: the front end mode
extern u8 D_800A5058;
extern u8 D_800A505C;
extern u8 D_800A4FC4; // JFG: a multiplayer game
extern u8 D_800A4FD0; // JFG: the number of players
extern u8 D_800A4FCC; // JFG: the number of cameras

/* JFG: change the front end mode; mode 0 goes back to one player (0x800589F0). */
void menu_mode_set(s32 mode) {
    func_800586E0();
    gMenuMode = mode;
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

/* Returns the front end mode (0x80058A5C). */
u8 menu_mode_get(void) {
    return gMenuMode;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_80058A6C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058C54);

INCLUDE_ASM("asm/nonmatchings/59060", func_80058D5C);

s8 is_game_paused(void); // JFG: the pause mode
extern s32 gMenuTimer;  // JFG: the front end timer

/* JFG: advance the front end timer (unless paused) up to limit; TRUE once it got there (0x80058E78). */
s32 menu_timer_advance(UNUSED s32 arg0, s32 limit, s32 delta) {
    s32 done = FALSE;

    if (!is_game_paused()) {
        gMenuTimer += delta;
    }
    if (gMenuTimer >= limit) {
        gMenuTimer = limit;
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

/* JFG: draw one rectangle through the list function (0x80059814). */
void menu_draw_rect(s32 arg0, s32 x1, s32 y1, s32 x2, s32 y2, s32 colour) {
    FrontRect rect;

    rect.x1 = x1;
    rect.y1 = y1;
    rect.x2 = x2;
    rect.y2 = y2;
    rect.colour = colour;
    func_800595F4(arg0, 1, &rect, 1);
}

void menu_layout_box_get(s32 index, s32 *x1, s32 *y1, s32 *x2, s32 *y2);
void fb_size(s32 *width, s32 *height);
extern s32 gVideoLastFramebuffer; // JFG: the framebuffer to show next
void func_8006F424(s32 framebuffer, s32 width, s32 height, s32 x1, s32 y1, s32 x2, s32 y2, s32 colour);
void func_8006F550(s32 arg0, s32 framebuffer, s32 width, s32 height, s32 x1, s32 y1, s32 x2, s32 y2, s32 colour);

/* JFG: draw box index of the current screen layout into the framebuffer (0x80059864). */
void menu_draw_layout_box(s32 arg0, s32 index, s32 colour) {
    s32 x1, y1, x2, y2;
    s32 width, height;

    menu_layout_box_get(index, &x1, &y1, &x2, &y2);
    fb_size(&width, &height);
    if (arg0 != 0) {
        func_8006F550(arg0, gVideoLastFramebuffer, width, height, x1, y1, x2, y2, colour);
    } else {
        func_8006F424(gVideoLastFramebuffer, width, height, x1, y1, x2, y2, colour);
    }
}

s32 fb_mode(void); // JFG: the video mode
s32 func_8005AA7C(void);
void fb_scale_coords(s32 *x, s32 *y);
extern s16 D_800A508C[];

/* JFG: the corners of box index for the current number of cameras (scaled to the screen) (0x80059938). */
void menu_layout_box_get(s32 index, s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    s32 mode = fb_mode();
    s32 offset = (((D_800A4FCC - 1) << 2) + index) << 2;
    s16 *box;

    if ((mode & 1) || (D_800A4FCC == 2 && func_8005AA7C())) {
        offset += 0x40;
    }
    box = &D_800A508C[offset];
    *x1 = box[0];
    *y1 = box[1];
    fb_scale_coords(x1, y1);
    *x2 = box[2];
    *y2 = box[3];
    fb_scale_coords(x2, y2);
}

extern u8 D_800FF3E0[4];
extern u8 D_800FF3E4[4];
extern s32 D_800FF3E8[4];

/* JFG: reset the four players' entries. */
void func_80059A04(void) {
    s32 i = 0;

    while (i < 4) {
        D_800FF3E8[i] = -1;
        D_800FF3E0[i] = 20;
        D_800FF3E4[i] = 15;
        i++;
    }
}

INCLUDE_ASM("asm/nonmatchings/59060", func_80059A98);

void func_80059D2C(s16 item);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059CCC);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059D2C);

void func_80059E7C(s16 item);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059E1C);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059E7C);

void menu_item_reset(s32 item);

INCLUDE_ASM("asm/nonmatchings/59060", func_80059FE8);

/* Size: 0x20 bytes */
typedef struct MenuItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s8 unk1C;
    /* 0x1D */ s8 unk1D;
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 unk1F;
} MenuItem;
extern MenuItem D_800A51DC[];
extern MenuItem D_800FF780[];

/* JFG: reset an item to its default (0x8005A048). */
void menu_item_reset(s32 item) {
    MenuItem *dst = &D_800FF780[item];
    MenuItem *src = &D_800A51DC[item];

    dst->unk0 = src->unk0;
    dst->unk2 = src->unk2;
    dst->unk4 = src->unk4;
    dst->unk6 = src->unk6;
    dst->unkC = src->unkC;
    dst->unk10 = src->unk10;
    dst->unk14 = src->unk14;
    dst->unk8 = src->unk8;
    dst->unk18 = src->unk18;
    dst->unk1C = src->unk1C;
    dst->unk1D = src->unk1D;
    dst->unk1E = src->unk1E;
    dst->unk1F = src->unk1F;
}

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A0D0);

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A51C);

s32 func_8005A794(void) {
    return 0;
}

s32 func_8005A79C(void) {
    return 0;
}

extern char **gMenuText;
extern u16 D_800A5918[];

/* Returns the name text of a world (0x8005A7A4). */
char *get_world_name(s32 world) {
    return gMenuText[D_800A5918[world]];
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

/* Sets the game's language and loads its text (0x8005A7E0). */
void set_language(s32 language) {
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

/* JFG: the screen mode (0-3) from two settings bits (0x8005A804). */
s32 options_screen_mode_get(void) {
    s32 mode = 0;

    if (D_800FF318.unk6C_1) {
        mode = 1;
    }
    if (D_800FF318.unk6C_2) {
        mode |= 2;
    }
    return mode;
}

extern u8 D_800A51A4;
extern u8 D_800A51A8;

INCLUDE_ASM("asm/nonmatchings/59060", func_8005A834);

/* JFG: remember the current screen mode byte (func_8005A8D8 returns it). */
void func_8005A8C4(void) {
    D_800A51A4 = D_800A51A8;
}

u8 func_8005A8D8(void) {
    return D_800A51A4;
}

s32 leveltable_screen_mode();

/* JFG: the screen mode for the current mode byte. */
s32 func_8005A8E8(void) {
    switch (D_800A51A8) {
        case 3:
            return 3;
        case 2:
            return leveltable_screen_mode() | 2;
        case 1:
            return 1;
        default:
            return leveltable_screen_mode();
    }
}

extern s8 gOptionScreenOffset;

/* Returns the stored vertical screen offset (0x8005A950). */
s8 options_screen_offset_get(void) {
    return gOptionScreenOffset;
}

s8 fb_vertical_adjust_get(void);
void fb_vertical_adjust_set(s32 adjust);
extern s8 gOptionScreenOffset;

/* Sets the vertical screen offset and stores it (0x8005A960). */
void options_screen_offset_set(s32 adjust) {
    fb_vertical_adjust_set(adjust);
    gOptionScreenOffset = fb_vertical_adjust_get();
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

extern u16 gOptionMusicVolume;

/* Returns the stored music volume (0-256) (0x8005AA30). */
u16 options_music_volume_get(void) {
    return gOptionMusicVolume;
}

extern u16 gOptionMusicVolume;
void func_80001050(s32 volume);

/* Sets and applies the music volume (0-256) (0x8005AA40). */
void options_music_volume_set(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 256) {
        volume = 256;
    }
    gOptionMusicVolume = volume;
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
