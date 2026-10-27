/*
 * Runlink module 35: sets up the text windows and text entries (as DKR's
 * load_fonts does) and the font tables.
 */
#include "module.h"

/* Size 0x28 (DKR's DialogueBoxBackground, font.h) */
typedef struct DialogueBoxBackground {
    /* 0x00 */ s16 xpos;
    /* 0x02 */ s16 ypos;
    /* 0x04 */ s16 x1;
    /* 0x06 */ s16 y1;
    /* 0x08 */ s16 x2;
    /* 0x0A */ s16 y2;
    /* 0x0C */ s16 width;
    /* 0x0E */ s16 height;
    /* 0x10 */ u8 backgroundColourR;
    /* 0x11 */ u8 backgroundColourG;
    /* 0x12 */ u8 backgroundColourB;
    /* 0x13 */ u8 backgroundColourA;
    /* 0x14 */ u8 textColourR;
    /* 0x15 */ u8 textColourG;
    /* 0x16 */ u8 textColourB;
    /* 0x17 */ u8 textColourA;
    /* 0x18 */ u8 textBGColourR;
    /* 0x19 */ u8 textBGColourG;
    /* 0x1A */ u8 textBGColourB;
    /* 0x1B */ u8 textBGColourA;
    /* 0x1C */ u8 opacity;
    /* 0x1D */ u8 font;
    /* 0x1E */ u16 flags;
    /* 0x20 */ s16 textOffsetX;
    /* 0x22 */ s16 textOffsetY;
    /* 0x24 */ void *textBox;
} DialogueBoxBackground;

/* Size 0x20 (DKR's DialogueTextElement, font.h) */
typedef struct DialogueTextElement {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 number;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ char *text;
    /* 0x08 */ s16 posX;
    /* 0x0A */ s16 posY;
    /* 0x0C */ s16 offsetX;
    /* 0x0E */ s16 offsetY;
    /* 0x10 */ u8 textColourR;
    /* 0x11 */ u8 textColourG;
    /* 0x12 */ u8 textColourB;
    /* 0x13 */ u8 textColourA;
    /* 0x14 */ u8 textBGColourR;
    /* 0x15 */ u8 textBGColourG;
    /* 0x16 */ u8 textBGColourB;
    /* 0x17 */ u8 textBGColourA;
    /* 0x18 */ u8 opacity;
    /* 0x19 */ u8 font;
    /* 0x1A */ u16 flags;
    /* 0x1C */ void *nextBox;
} DialogueTextElement;

typedef struct M035Glyph {
    /* 0x0 */ u8 unk0;
    /* 0x1 */ u8 pad1;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ u8 unk4;
    /* 0x5 */ u8 unk5;
    /* 0x6 */ u8 pad6[0x10 - 0x6];
} M035Glyph;

typedef struct M035Font {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s32 offset;
    /* 0x8 */ s16 stride;
    /* 0xA */ u8 padA[0x10 - 0xA];
} M035Font;

typedef struct M035Char {
    /* 0x0 */ u8 pad0[8];
    /* 0x8 */ u8 unk8;
} M035Char;

extern DialogueBoxBackground D_80103FA8[8];
extern DialogueTextElement D_80103BA8[32];
extern s8 D_80103BA0;
extern M035Font *D_80103BA4;
extern u8 *D_801040E8[4];
extern u8 *D_801040F8;
extern M035Glyph *D_801040FC;
extern M035Char *D_80104100;
extern void *D_80104104;
extern void *D_80104108;
extern s8 D_8010410C;
extern s8 D_8010410D;

extern void func_80054EA0(s32 *width, s32 *height);
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern M035Font *func_8004D37C(s32 asset);
extern void func_8004D530(s32 asset, void *dst, s32 offset, s32 size);
extern void func_800715E8(void *);

void func_m035_00000(void) {
    s32 i;
    s32 j;
    s32 width;
    s32 height;
    M035Char *ch;
    M035Font *font;

    func_80054EA0(&width, &height);
    for (i = 0; i < 8; i++) {
        D_80103FA8[i].xpos = 0;
        D_80103FA8[i].ypos = 0;
        D_80103FA8[i].x1 = 0;
        D_80103FA8[i].y1 = 0;
        D_80103FA8[i].x2 = width - 1;
        D_80103FA8[i].y2 = height - 1;
        D_80103FA8[i].width = width;
        D_80103FA8[i].height = height;
        D_80103FA8[i].backgroundColourR = 0;
        D_80103FA8[i].backgroundColourG = 0;
        D_80103FA8[i].backgroundColourB = 0;
        D_80103FA8[i].backgroundColourA = 0;
        D_80103FA8[i].textColourR = 255;
        D_80103FA8[i].textColourG = 255;
        D_80103FA8[i].textColourB = 255;
        D_80103FA8[i].textColourA = 0;
        D_80103FA8[i].textBGColourR = 255;
        D_80103FA8[i].textBGColourG = 255;
        D_80103FA8[i].textBGColourB = 255;
        D_80103FA8[i].textBGColourA = 0;
        D_80103FA8[i].opacity = 255;
        D_80103FA8[i].font = 0;
        D_80103FA8[i].flags = 0;
        D_80103FA8[i].textOffsetX = 0;
        D_80103FA8[i].textOffsetY = 0;
        D_80103FA8[i].textBox = NULL;
    }
    for (i = 0; i < 32; i++) {
        D_80103BA8[i].number = 255;
        D_80103BA8[i].text = NULL;
        D_80103BA8[i].textColourR = 255;
        D_80103BA8[i].textColourG = 255;
        D_80103BA8[i].textColourB = 255;
        D_80103BA8[i].textColourA = 0;
        D_80103BA8[i].textBGColourR = 255;
        D_80103BA8[i].textBGColourG = 255;
        D_80103BA8[i].textBGColourB = 255;
        D_80103BA8[i].textBGColourA = 0;
        D_80103BA8[i].nextBox = NULL;
    }
    D_801040F8 = func_8004A57C(0x8010, 0xFF0000FF);
    D_801040FC = func_8004A57C(0x1000, 0xFF0000FF);
    D_801040F8 += 0x10;
    for (i = 0, j = 0; i < 256; i++) {
        D_801040FC[i].unk0 = 255;
        D_801040FC[i].unk4 = 0;
        D_801040FC[i].unk2 = j;
        j += 0x80;
        D_801040FC[i].unk5 = 0;
    }
    D_80103BA4 = func_8004D37C(0x3A);
    D_80104100 = func_8004A57C(0x200, 0xFF0000FF);
    D_80104104 = func_8004A57C(0x200, 0xFF0000FF);
    D_80104108 = func_8004A57C(0x200, 0xFF0000FF);
    ch = D_80104100;
    font = D_80103BA4;
    for (i = 0; i < 4; i++) {
        D_801040E8[i] = func_8004A57C(0x400, 0xFF0000FF);
        for (j = 0; j < 256; j++) {
            func_8004D530(0x3B, ch, font->offset + j * font->stride, 0x20);
            D_801040E8[i][j] = ch->unk8;
        }
        func_800715E8(font);
        font++;
    }
    D_80103BA0 = 0;
    D_8010410C = 0;
    D_8010410D = 0;
}
