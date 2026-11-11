/*
 * Menus. Functions in C are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/menu.c, or marked "JFG"; see CREDITS.md.
 */
#include "common.h"

extern u8 gLanguage;

extern s32 *D_800A51AC; // JFG: the menu text table (count, then one offset per language)
extern char **gMenuText;
s32 *asset_table_load(u32 assetIndex);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/**
 * Loads the menu text of a language and turns its offsets into pointers (-1 = none).
 * JFG: five languages, and no string tables to fill afterwards.
 * DKR: load_menu_text
 */
void load_menu_text(s32 language) {
    char **temp;
    s32 langIndex;
    s32 size;

    if (D_800A51AC == NULL) {
        D_800A51AC = asset_table_load(7);
    }
    switch (language) {
        case 4:
            langIndex = 5;
            break;
        case 3:
            langIndex = 4;
            break;
        case 2:
            langIndex = 3;
            break;
        case 1:
            langIndex = 2;
            break;
        default:
            langIndex = 1;
            break;
    }
    size = D_800A51AC[langIndex + 1];
    langIndex = D_800A51AC[langIndex];
    size -= langIndex;
    temp = gMenuText;
    if (temp == NULL) {
        return;
    }
    asset_load(6, (u32) temp, langIndex, size);
    for (langIndex = 0; langIndex < D_800A51AC[0]; langIndex++) {
        temp = gMenuText;
        if ((s32) temp[langIndex] == -1) {
            temp[langIndex] = NULL;
        } else {
            temp[langIndex] += (s32) temp;
        }
    }
}

void *mempool_alloc_safe(s32 size, u32 colourTag);
extern u8 *D_800FF3A8[3]; // JFG: three 0x538-byte buffers
extern s32 D_800FF3F8[180]; // JFG: the menu items' loaded assets
extern u8 D_800FF6C8[];   // JFG: whether each menu item is loaded
extern u8 D_800A51A0;
extern s16 *D_800A51D0;   // JFG: the menu item types (ending with -1)
extern s16 D_800A51D4;    // JFG: the number of menu items
extern s16 D_800A51D8;    // JFG: the number of loaded menu items
extern u8 D_800FF1B4[4];

#ifdef NON_MATCHING
/* NON_MATCHING: only the last 4 stores: the target shares one lui for D_800FF1B4[0..3] (data defined in the file) */
/* JFG: allocate the menu buffers and text, load the item table and mark every item unloaded. */
void func_80058590(void) {
    s32 i;

    D_800FF3A8[0] = mempool_alloc_safe(0xFA8, -1);
    for (i = 1; i < 3; i++) {
        D_800FF3A8[i] = D_800FF3A8[i - 1] + 0x538;
    }
    gMenuText = mempool_alloc_safe(0x3000, -1);
    load_menu_text(0);
    i = 0;
    while (i < 180) {
        D_800FF3F8[i++] = 0;
    }
    D_800A51A0 = 0;
    D_800A51D0 = (s16 *) asset_table_load(0x1A);
    for (D_800A51D4 = 0; D_800A51D0[D_800A51D4] != -1; D_800A51D4++) {
    }
    D_800A51D8 = 0;
    for (i = 0; i < D_800A51D4; i++) {
        D_800FF6C8[i] = 0;
    }
    for (i = 0; i < 4; i++) {
        D_800FF1B4[i] = 2;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/59060", func_80058590);
#endif

s32 func_80054530(); // a module function (patched in by the runtime linker)
s32 func_80052CFC();  // JFG: load a runlink module
s32 runlink_module_loaded(s32 module);
void func_8005342C(s32 module); // JFG: unload a runlink module
void dialogue_clear(s32 dialogueBoxID);
extern u8 gMenuMode;  // JFG: which menu set is running
extern u8 D_800A4FC4; // JFG: a multiplayer game
extern u8 D_800A4FD0; // JFG: the number of players
extern u8 D_800A4FD8; // JFG: the multiplayer game type (low 4 bits)
extern u8 D_800A4FB4;

/* JFG: close the current menu set (its module's close function; each func_80054530 call is a different one). */
void menu_set_close(void) {
    if (runlink_module_loaded(0xC)) {
        func_80054530();
    }
    if (D_800A51A0) {
        switch (gMenuMode) {
            case 0:
                dialogue_clear(1);
                break;
            case 1:
            case 7:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 23:
                break;
            case 2:
                func_80054530();
                break;
            case 3:
                func_80054530();
                break;
            case 4:
                func_80054530();
                break;
            case 5:
                func_80054530();
                break;
            case 6:
                func_80054530();
                break;
            case 8:
                func_80054530();
                break;
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
                func_80054530();
                break;
            case 24:
                func_80054530();
                break;
            case 16:
                if ((D_800A4FC4 && D_800A4FD8 == 4) || D_800A4FB4) {
                    func_80054530();
                } else if (D_800A4FD0 == 1) {
                    func_80054530();
                } else {
                    func_80054530();
                }
                break;
            case 17:
                func_80054530();
                func_8005342C(9);
                break;
        }
        D_800A51A0 = 0;
    }
}

/* JFG: open the current menu set (its module's init function). */
void menu_set_open(void) {
    func_80054530();
    if (D_800A51A0 == 0) {
        switch (gMenuMode) {
            case 2:
                func_80054530();
                break;
            case 3:
                func_80054530();
                break;
            case 4:
                func_80054530(0);
                break;
            case 17:
                func_80054530(0);
                break;
            case 5:
                func_80054530();
                break;
            case 6:
                func_80054530();
                break;
            case 8:
                func_80054530();
                break;
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
                func_80054530();
                break;
            case 23:
                func_80054530();
                break;
            case 24:
                func_80054530();
                break;
            case 16:
                if ((D_800A4FC4 && D_800A4FD8 == 4) || D_800A4FB4) {
                    func_80054530();
                } else {
                    func_80052CFC(6);
                    if (D_800A4FD0 == 1) {
                        func_80054530();
                    } else {
                        func_80054530();
                    }
                }
                break;
            case 0:
            case 1:
            case 7:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                break;
        }
        D_800A51A0 = 1;
    }
}

void menu_set_close(void);
void menu_set_open(void);
void func_80059A04(void);
extern u8 gMenuMode; // JFG: which menu set is running
extern u8 D_800A5058;
extern u8 D_800A505C;
extern u8 D_800A4FC4; // JFG: a multiplayer game
extern u8 D_800A4FD0; // JFG: the number of players
extern u8 D_800A4FCC; // JFG: the number of cameras

/* JFG: store a new gMenuMode between calls to three menu functions; mode 0 also resets to a single player and camera (0x800589F0). */
void menu_mode_set(s32 mode) {
    menu_set_close();
    gMenuMode = mode;
    menu_set_open();
    func_80059A04();
    D_800A5058 = 0;
    D_800A505C = 1;
    if (mode == 0) {
        D_800A4FC4 = 0;
        D_800A4FD0 = 1;
        D_800A4FCC = 1;
    }
}

/* Returns gMenuMode (0x80058A5C). */
u8 menu_mode_get(void) {
    return gMenuMode;
}

extern s32 D_800FF398;
extern s32 D_800FF39C;
extern s32 D_800FF3A0;
extern s32 D_800FF3A4;
void debug_trace_add(s32 a, char *file, s32 line);
void menu_input_update(s32 arg0);
void menu_show_results(s32 arg0);

/**
 * JFG: run the current menu set's module through the results stage (between two checks
 * tagged with this file's name and line).
 * Only `case 0` of the empty cases is listed: with all of them, IDO stops keeping arg4 in s0.
 */
s32 menu_set_results(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3, s32 arg4) {
    menu_input_update(arg4);
    D_800FF398 = *arg0;
    D_800FF39C = *arg1;
    D_800FF3A0 = *arg2;
    D_800FF3A4 = *arg3;
    debug_trace_add(D_800FF398, "front/front.c", 0x256);
    if (func_80054530()) {
        switch (gMenuMode) {
            case 2:
                func_80054530(arg4);
                break;
            case 3:
                func_80054530(arg4);
                break;
            case 4:
                func_80054530(arg4);
                break;
            case 17:
                func_80054530(arg4);
                break;
            case 5:
                func_80054530(arg4);
                break;
            case 6:
                func_80054530(arg4);
                break;
            case 8:
                func_80054530(arg4);
                break;
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
                func_80054530(arg4);
                break;
            case 23:
                func_80054530(arg4);
                break;
            case 24:
                func_80054530(arg4);
                break;
            case 16:
                menu_show_results(arg4);
                break;
            case 0:
                break;
        }
    }
    func_80054530();
    func_80054530(arg4);
    debug_trace_add(D_800FF398, "front/front.c", 0x27A);
    *arg0 = D_800FF398;
    *arg1 = D_800FF39C;
    *arg2 = D_800FF3A0;
    *arg3 = D_800FF3A4;
    D_800A505C = 0;
    return 0;
}

extern u8 D_800A5194; // JFG: a blink counter
extern char **gMenuText;
void fb_scale_coords(s32 *x, s32 *y);
s32 get_language(void);
void load_menu_text(s32 language);
void set_text_font(s32 fontID);
void set_text_colour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
void set_text_background_colour(s32 red, s32 green, s32 blue, s32 alpha);
void draw_text(Gfx **displayList, s32 xpos, s32 ypos, char *text, s32 alignmentFlags);

/* JFG: advance the blink counter by delta and, in its "on" half, draw menu text 49 (shadowed) at (160, 208). */
void menu_draw_blinking_prompt(Gfx **dList, s32 delta) {
    s32 x, y;

    D_800A5194 += delta;
    if (D_800A5194 & 0x10) {
        x = 160;
        y = 208;
        fb_scale_coords(&x, &y);
        load_menu_text(get_language());
        set_text_font(2);
        set_text_background_colour(0, 0, 0, 0);
        set_text_colour(0, 0, 0, 255, 255);
        draw_text(dList, x + 1, y + 1, gMenuText[49], 12);
        set_text_colour(255, 255, 255, 0, 255);
        draw_text(dList, x, y, gMenuText[49], 12);
    }
}

typedef struct Object Object;
Object **get_racer_objects(s32 *numRacers);
s32 func_80054530(); // a module function (patched in by the runtime linker)
extern u8 D_800A4FD8; // JFG: the multiplayer game type (low 4 bits)
extern u8 D_800A4FB4;
extern s32 D_800FF398;
extern s32 D_800FF39C;
extern s32 D_800FF3A0;

/* JFG: hand the players (or the multiplayer results) to the module that shows them, by game type. */
void menu_show_results(s32 arg0) {
    s32 count;
    Object **racers = get_racer_objects(&count);

    if (D_800A4FC4) {
        switch (D_800A4FD8 & 0xF) {
            case 0:
                func_80054530(count, racers, arg0);
                break;
            case 6:
                func_80054530(&D_800FF398, &D_800FF39C, &D_800FF3A0, D_800A4FD0);
                break;
            case 4:
                func_80054530(&D_800FF398, &D_800FF39C, &D_800FF3A0, D_800A4FD0);
                break;
        }
    } else if (D_800A4FB4) {
        func_80054530(&D_800FF398, &D_800FF39C, &D_800FF3A0, D_800A4FD0);
    } else if (count > 0) {
        func_80054530(racers[0], arg0);
    }
}

s8 is_game_paused(void); // JFG: the pause mode
extern s32 gMenuTimer;  // JFG: a frame counter for the menus

/* JFG: add delta to gMenuTimer when the game is not paused, stopping at limit; TRUE when limit is reached (0x80058E78). */
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

/* (as in 4E610.c) a texture with its load commands */
typedef struct FontTexture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0xA - 0x2];
    /* 0x0A */ s16 numberOfCommands;
    /* 0x0C */ Gfx *cmd;
} FontTexture;

/* DKR include/f3ddkr.h: G_DMADL (7) runs numberOfCommands commands from address. */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(0x07, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                 \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }

/* The texture rectangle in the older microcode's form (RDPHALF_2, RDPHALF_CONT) (as in 4E610.c) */
#define gSPTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                    \
    {                                                                                         \
        Gfx *_g = (Gfx *) (pkt);                                                              \
                                                                                              \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));     \
        gImmp1(pkt, G_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                   \
        gImmp1(pkt, G_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));          \
    }

#define sMenuGfx (*(Gfx **) &D_800FF398) // the menu display list (also the results' first value)
extern Gfx D_800A58A0[];
extern Gfx D_800A58E8[];
void fb_size(s32 *width, s32 *height);
FontTexture *set_animated_texture_header(FontTexture *texHead, s32 offset);

/*
 * JFG: draws the number n with font (8 or 9: a menu asset holding the digits 1-9, 0 side by side) right
 * aligned at (x, y) from the screen centre, in colour, then pads it with zeros in padColour to digits.
 */
void func_80058EF0(s32 n, s32 x, s32 y, s32 font, s32 digits, s32 colour, s32 padColour) {
    FontTexture *tex;
    s32 xl;
    s32 yl;
    s32 xh;
    s32 yh;
    s32 i;
    s32 t;
    s32 spacing;
    u32 charW;
    s32 count;
    Gfx *cmd;
    u32 width;
    u32 height;
    u32 s;

    fb_size((s32 *) &width, (s32 *) &height);
    x += width >> 1;
    y = (height >> 1) - y;
    tex = (FontTexture *) D_800FF3F8[font];
    if (tex != NULL) {
        switch (font) {
            case 9:
                charW = 8;
                spacing = 8;
                break;
            case 8:
                charW = 11;
                spacing = 12;
                break;
        }
        gSPDisplayList(sMenuGfx++, D_800A58A0);
        gDkrDmaDisplayList(sMenuGfx++, (u8 *) D_800A58E8 + 0x80000000, 2);
        xl = x << 2;
        yl = y << 2;
        t = (tex->height - 1) << 5;
        xh = (charW << 2) + xl;
        yh = (tex->height << 2) + yl;
        cmd = tex->cmd;
        sMenuGfx->words.w0 = cmd->words.w0;
        sMenuGfx->words.w1 = (u32) set_animated_texture_header(tex, 0) + 0x80000000;
        sMenuGfx++;
        cmd++;
        {
            Gfx *_g = sMenuGfx++; // gDkrDmaDisplayList, the count read after the pointer
            count = tex->numberOfCommands - 1;
            // both words on one line: IDO schedules by line (the ROM stores w1 first)
            _g->words.w0 = (_SHIFTL(0x07, 24, 8) | _SHIFTL((count), 16, 8) | _SHIFTL((count * 8), 0, 16)); _g->words.w1 = (u32) cmd + 0x80000000;
        }
        gDPSetColor(sMenuGfx++, G_SETPRIMCOLOR, colour);
        i = 0;
        while (n != 0) {
            s = ((n - 1) % 10) * (charW << 5);
            xh -= spacing << 2;
            xl -= spacing << 2;
            gSPTextureRectangleOld(sMenuGfx++, xl, yl, xh, yh, 0, s, t, 0x400, -0x400);
            n /= 10;
            i++;
        }
        gDPSetColor(sMenuGfx++, G_SETPRIMCOLOR, padColour);
        for (i = digits - i; i != 0; i--) {
            xh -= spacing << 2;
            xl -= spacing << 2;
            gSPTextureRectangleOld(sMenuGfx++, xl, yl, xh, yh, 0, 9 * (charW << 5), t, 0x400, -0x400);
            n /= 10;
        }
    }
}

typedef struct MenuBox {
    s16 x1, y1, x2, y2;
    s32 colour;
} MenuBox;
void fb_size(s32 *width, s32 *height);

extern Gfx D_800A58F8[];
void rendermode_reset(Gfx **dList);
/* JFG: fill count coloured boxes, clipped to the screen (those off it skipped), changing the primitive
   colour only when it changes; arg3 picks one of two render modes. */
void menu_fill_boxes(Gfx **dList, s32 count, MenuBox *boxes, s32 arg3) {
    s32 x1, y1, x2, y2;
    s32 colour;
    u32 width, height;

    fb_size((s32 *) &width, (s32 *) &height);
    gSPDisplayList((*dList)++, D_800A58F8);
    if (arg3) {
        gDPSetOtherMode((*dList)++, 0x82C0F, 0x504340);
    } else {
        gDPSetOtherMode((*dList)++, 0x82C0F, 0x504240);
    }
    colour = boxes->colour;
    gDPSetColor((*dList)++, G_SETPRIMCOLOR, colour);
    while (count--) {
        x2 = boxes->x2;
        y2 = boxes->y2;
        if (x2 >= 0 && y2 >= 0) {
            x1 = boxes->x1;
            y1 = boxes->y1;
            if (x1 < (s32) width && y1 < (s32) height) {
                if (x1 < 0) {
                    x1 = 0;
                }
                if (y1 < 0) {
                    y1 = 0;
                }
                if ((s32) width < x2) {
                    x2 = width;
                }
                if ((s32) height < y2) {
                    y2 = height;
                }
                if (colour != boxes->colour) {
                    colour = boxes->colour;
                    gDPPipeSync((*dList)++);
                    gDPSetColor((*dList)++, G_SETPRIMCOLOR, colour);
                }
                gDPFillRectangle((*dList)++, x1, y1, x2, y2);
            }
        }
        boxes++;
    }
    rendermode_reset(dList);
    gDPSetColor((*dList)++, G_SETPRIMCOLOR, -1);
}

void menu_fill_boxes(Gfx **dList, s32 count, MenuBox *boxes, s32 arg3);

/* JFG: fill one coloured box (corners x1,y1 and x2,y2) through the list function, which emits a
 * primitive colour and a fill command per box (0x80059814). */
void menu_fill_box(Gfx **arg0, s32 x1, s32 y1, s32 x2, s32 y2, s32 colour) {
    MenuBox box;

    box.x1 = x1;
    box.y1 = y1;
    box.x2 = x2;
    box.y2 = y2;
    box.colour = colour;
    menu_fill_boxes(arg0, 1, &box, 1);
}

void menu_layout_box_get(s32 index, s32 *x1, s32 *y1, s32 *x2, s32 *y2);
void fb_size(s32 *width, s32 *height);
extern s32 gVideoLastFramebuffer; // JFG: the framebuffer to show next
void screen_effect_queue(s32 framebuffer, s32 width, s32 height, s32 x1, s32 y1, s32 x2, s32 y2, s32 colour);
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
        screen_effect_queue(gVideoLastFramebuffer, width, height, x1, y1, x2, y2, colour);
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

extern s8 D_800FF3E0[4];  // JFG: each player's stick x repeat timer
extern s8 D_800FF3E4[4];  // JFG: each player's stick y repeat timer
extern s32 D_800FF3E8[4]; // JFG: each player's buttons held last frame

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

u16 input_held(s32 player);
s32 input_clamp_stick_x(s32 player);
s32 input_clamp_stick_y(s32 player);
extern s32 D_800FF3B8[4]; // JFG: each player's buttons held
extern s32 D_800FF3C8[4]; // JFG: each player's buttons pressed this frame
extern s8 D_800FF3D8[4];  // JFG: each player's stick x move (with auto-repeat)
extern s8 D_800FF3DC[4];  // JFG: each player's stick y move (with auto-repeat)

/* JFG: read the four players' buttons for the menus, and turn the sticks into moves that repeat
   (after 20 frames, then every 15) while held. */
void menu_input_update(s32 updateRate) {
    s32 i;
    s32 held;

    for (i = 0; i < 4; i++) {
        held = input_held(i);
        D_800FF3B8[i] = held;
        D_800FF3C8[i] = ~D_800FF3E8[i] & held;
        D_800FF3E8[i] = held;
        D_800FF3D8[i] = input_clamp_stick_x(i);
        if (D_800FF3D8[i] < -35) {
            if (D_800FF3E0[i] < 0) {
                D_800FF3E0[i] += updateRate;
                if (D_800FF3E0[i] >= 0) {
                    D_800FF3E0[i] = -15;
                } else {
                    D_800FF3D8[i] = 0;
                }
            } else {
                D_800FF3E0[i] = -20;
            }
        } else if (D_800FF3D8[i] > 35) {
            if (D_800FF3E0[i] > 0) {
                D_800FF3E0[i] -= updateRate;
                if (D_800FF3E0[i] <= 0) {
                    D_800FF3E0[i] = 15;
                } else {
                    D_800FF3D8[i] = 0;
                }
            } else {
                D_800FF3E0[i] = 20;
            }
        } else {
            D_800FF3D8[i] = 0;
            D_800FF3E0[i] = 0;
        }
        D_800FF3DC[i] = input_clamp_stick_y(i);
        if (D_800FF3DC[i] < -35) {
            if (D_800FF3E4[i] < 0) {
                D_800FF3E4[i] += updateRate;
                if (D_800FF3E4[i] >= 0) {
                    D_800FF3E4[i] = -15;
                } else {
                    D_800FF3DC[i] = 0;
                }
            } else {
                D_800FF3E4[i] = -20;
            }
        } else if (D_800FF3DC[i] > 35) {
            if (D_800FF3E4[i] > 0) {
                D_800FF3E4[i] -= updateRate;
                if (D_800FF3E4[i] <= 0) {
                    D_800FF3E4[i] = 15;
                } else {
                    D_800FF3DC[i] = 0;
                }
            } else {
                D_800FF3E4[i] = 20;
            }
        } else {
            D_800FF3DC[i] = 0;
            D_800FF3E4[i] = 0;
        }
    }
}

void menu_item_free(s32 item);

/**
 * Free each menu item of a list ending with -1.
 * DKR: menu_assetgroup_free
 */
void menu_assetgroup_free(s16 *assetGroup) {
    s32 index = 0;
    while (assetGroup[index] != -1) {
        menu_item_free(assetGroup[index++]);
    }
}

void tex_free(void *tex);
void sprite_free(void *sprite);
void free_object(void *object);
void free_3d_model(void *model);
void gParticlePtrList_flush(void);

/* JFG: free menu item's asset (a texture, sprite, object or model by the top bits of its type) if it is loaded. */
void menu_item_free(s32 item) {
    if (D_800FF6C8[item]) {
        if (D_800FF3F8[item]) {
            if ((D_800A51D0[item] & 0xC000) == 0xC000 && D_800FF3F8[item]) {
                tex_free((void *) D_800FF3F8[item]);
            } else if (D_800A51D0[item] & 0x8000) {
                sprite_free((void *) D_800FF3F8[item]);
            } else if (D_800A51D0[item] & 0x4000) {
                free_object((void *) D_800FF3F8[item]);
            } else {
                free_3d_model((void *) D_800FF3F8[item]);
            }
        }
        D_800FF3F8[item] = 0;
        D_800FF6C8[item] = 0;
        D_800A51D8--;
        gParticlePtrList_flush();
    }
}

void menu_item_load(s32 item);

/**
 * Load each menu item of a list ending with -1.
 * DKR: menu_assetgroup_load
 */
void menu_assetgroup_load(s16 *assetGroup) {
    s32 index = 0;
    while (assetGroup[index] != -1) {
        menu_item_load(assetGroup[index++]);
    }
}

/* JFG: the spawn entry of a menu object (size 10) */
typedef struct MenuObjectSpawn {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 unkC;
} MenuObjectSpawn;

typedef struct MenuModelInstance {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u8 unkA;
} MenuModelInstance;

typedef struct MenuObjectHeader {
    /* 0x00 */ u8 pad0[0x1F];
    /* 0x1F */ s8 numberOfModelIds;
} MenuObjectHeader;

typedef struct MenuObject {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ MenuObjectHeader *header;
    /* 0x44 */ u8 pad44[0x6C - 0x44];
    /* 0x6C */ MenuModelInstance **modelInstances;
} MenuObject;

void *load_texture(s32 assetIndex);
void *tex_load_sprite(s32 spriteID, s32 arg1);
MenuObject *spawn_object(void *entry, s32 arg1);
void *func_8003B9E8(s32 modelID, s32 arg1);

/* JFG: load menu item's asset (a texture, sprite, object or model by the top bits of its type) unless it is loaded. */
void menu_item_load(s32 item) {
    MenuObjectSpawn spawn;
    MenuObject *obj;
    MenuModelInstance *inst;

    stubbed_printf("loadFrontEndItem() - Item no %d out of range 0-%d\n");
    if (!D_800FF6C8[item]) {
        if ((D_800A51D0[item] & 0xC000) == 0xC000) {
            D_800FF3F8[item] = (s32) load_texture(D_800A51D0[item] & 0x3FFF);
        } else if (D_800A51D0[item] & 0x8000) {
            D_800FF3F8[item] = (s32) tex_load_sprite(D_800A51D0[item] & 0x3FFF, 0);
        } else if (D_800A51D0[item] & 0x4000) {
            spawn.objectID = D_800A51D0[item] & 0x3FFF;
            spawn.size = 10;
            spawn.x = 0;
            spawn.y = 0;
            spawn.z = 0;
            spawn.unkC = 0;
            spawn.unkB = 0x40;
            spawn.unkA = 0;
            obj = spawn_object(&spawn, 0);
            if (obj->header->numberOfModelIds > 0) {
                inst = obj->modelInstances[0];
                inst->unkA = 2;
            }
            D_800FF3F8[item] = (s32) obj;
        } else {
            D_800FF3F8[item] = (s32) func_8003B9E8(D_800A51D0[item] & 0x3FFF, 0);
        }
        D_800FF6C8[item] = 1;
        D_800A51D8++;
    }
}

void menu_item_reset(s32 item);

/* JFG: reset each menu item of a list ending with -1 (DKR's menu_assetgroup_free loop). */
void menu_assetgroup_reset(s16 *assetGroup) {
    s32 index = 0;
    while (assetGroup[index] != -1) {
        menu_item_reset(assetGroup[index++]);
    }
}

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

extern u8 D_800A4FF0; // selects one of the three per-level replacement lists
s32 get_ingame_map_id(void);

/* JFG: a byte-aligned 10-byte spawn entry header (copied byte by byte) */
typedef struct SpawnHeader {
    /* 0x00 */ u8 data[10];
} SpawnHeader;

typedef struct SpawnEntry {
    /* 0x00 */ SpawnHeader header;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 pad0C[0x28 - 0x0C];
} SpawnEntry;

typedef struct LevelSpawnTable {
    /* 0x00 */ s32 level; // negative ends the table
    /* 0x04 */ s16 (*lists[3])[4];
} LevelSpawnTable;

/**
 * JFG: spawn the current level's replacement for a spawn entry (looked up in a table that a module
 * returns; single-player only). unused0..unused2 only reproduce the original stack frame.
 */
void spawn_level_replacement(SpawnHeader *src) {
    s32 unused0;
    s32 i;
    SpawnEntry entry;
    s32 level;
    s32 size;
    s16 (*list)[4];
    MenuObject *obj;
    u8 *s;
    u8 *d;
    s32 n;
    s32 unused1;
    s32 unused2;
    LevelSpawnTable *table;

    table = NULL;
    if ((D_800A4FD8 & 0xF) == 0) {
        table = (LevelSpawnTable *) func_80054530();
    }
    if (table != NULL) {
        i = 0;
        level = get_ingame_map_id();
        while (table[i].level >= 0 && table[i].level != level) {
            i++;
        }
        if (level == table[i].level) {
            list = table[i].lists[D_800A4FF0];
            switch (list[(s8) src->data[0xA]][0]) {
                case 0xCF:
                case 0xE7:
                case 0xE8:
                    entry.unkB = list[(s8) src->data[0xA]][1];
                    size = 0x10;
                    entry.unkA = list[(s8) src->data[0xA]][2];
                    break;
                case 0xA9:
                case 0xE9:
                    entry.unkA = list[(s8) src->data[0xA]][1];
                    size = 0xC;
                    entry.unkB = list[(s8) src->data[0xA]][2];
                    break;
                case 0xFA:
                case 0xFB:
                case 0xFC:
                case 0xFD:
                case 0xFE:
                case 0xFF:
                case 0x100:
                case 0x101:
                case 0x102:
                case 0x103:
                case 0x104:
                case 0x105:
                case 0x123:
                case 0x124:
                case 0x125:
                    entry.unkA = 0;
                    size = 0xC;
                    entry.unkB = list[(s8) src->data[0xA]][1];
                    break;
                case 0x12A:
                    entry.unkA = list[(s8) src->data[0xA]][1];
                    size = 0xC;
                    entry.unkB = list[(s8) src->data[0xA]][2];
                    break;
                case 0x33C:
                    entry.unkA = list[(s8) src->data[0xA]][1];
                    size = 0xC;
                    entry.unkB = list[(s8) src->data[0xA]][2];
                    break;
                default:
                    return;
            }
            s = src->data;
            d = (u8 *) &entry;
            n = 10;
            while (n--) {
                *d++ = *s++;
            }
            *(s16 *) &entry = list[(s8) src->data[0xA]][0];
            entry.header.data[2] = size;
            obj = spawn_object(&entry, 1);
            if (obj != NULL) {
                *(s32 *) ((u8 *) obj + 0x3C) = 0;
            }
        }
    }
}

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

void load_menu_text(s32 language);

/* Sets the game's language and loads its text (0x8005A7E0). */
void set_language(s32 language) {
    gLanguage = language;
    load_menu_text(language);
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

/* JFG: set the screen mode byte (0-3) and copy its two bits into the settings flags.
   (The compare into a local and the copy of mode give the ROM's registers; found by the permuter.) */
void func_8005A834(s32 mode) {
    s32 changed;
    u8 m;
    s32 bits;

    changed = (mode & 3) != D_800A51A8;
    bits = mode;
    if (changed) {
        D_800A51A8 = mode & 3;
        m = bits & 3;
        if (m & 1) {
            D_800FF318.unk6C_1 = 1;
        } else {
            D_800FF318.unk6C_1 = 0;
        }
        if (m & 2) {
            D_800FF318.unk6C_2 = 1;
        } else {
            D_800FF318.unk6C_2 = 0;
        }
    }
}

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
