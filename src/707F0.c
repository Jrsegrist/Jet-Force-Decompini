/*
 * Text and dialogue boxes. Functions in C are from the Diddy Kong Racing
 * decompilation (CC0 1.0), src/font.c; see CREDITS.md.
 */
#include "common.h"

/* DKR src/font.h (JFG layout) */
#define DIALOGUE_NUM_NULL 0xFF
#define DIALOGUEBOXBACKGROUND_COUNT 8
#define DIALOGUE_BOX_CLOSED 0x7FFF
#define DIALOGUE_BOX_OPEN 0x8000

typedef struct DialogueBox {
    /* 0x00 */ u8 unk00;
    /* 0x01 */ u8 textNum;
    /* 0x02 */ u8 pad2[0x1C - 0x02];
    /* 0x1C */ struct DialogueBox *nextBox;
} DialogueBox;

/* Size: 0x28 bytes */
typedef struct DialogueBoxBackground {
    s16 xpos;
    s16 ypos;
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
    s16 width;
    s16 height;
    u8 backgroundColourR;
    u8 backgroundColourG;
    u8 backgroundColourB;
    u8 backgroundColourA;
    u8 textColourR;
    u8 textColourG;
    u8 textColourB;
    u8 textColourA;
    u8 textBGColourR;
    u8 textBGColourG;
    u8 textBGColourB;
    u8 textBGColourA;
    u8 opacity;
    u8 font;
    u16 flags;
    s16 textOffsetX;
    s16 textOffsetY;
    DialogueBox *textBox;
} DialogueBoxBackground;

/* JFG's font header (Size: 0x10 bytes) */
typedef struct FontData {
    u8 width;
    u8 y;
    u8 charWidth; // the width of a character without its own entry
    u8 lineHeight;
    s32 dataOffset; // JFG: of the glyph images in asset 0x3B
    s16 glyphSize;
    u16 format;     // JFG: 4: the glyphs take two texture loads
    u32 *loadCmds;  // JFG: the texture load commands of the glyphs
} FontData;
extern FontData *gFonts; // Official Name: Font

extern DialogueBoxBackground gDialogueBoxBackground[]; // Official Name: Window
extern u8 fontInUse;                                    // Official Name: fontInUse
extern s32 gDescPowsOf10[]; // JFG: zero-terminated
extern u8 D_8010410D;
extern s32 D_800A6EA8;
extern s32 D_800A6EAC;
extern s32 D_800A6EB0;

void draw_dialogue_text_pos_unused(Gfx **displayList, s32 dialogueBoxID, s32 xpos, s32 ypos, char *text,
                                   s32 alignmentFlags);

/* JFG: set the size of dialogue box 0 (the whole screen) (0x8006FBF0). */
void set_screen_dialogue_box_size(s32 width, s32 height) {
    DialogueBoxBackground *box = &gDialogueBoxBackground[0];
    box->x2 = width - 1;
    box->y2 = height - 1;
    box->width = width;
    box->height = height;
}

/* JFG: set a text mode byte. */
void func_8006FC14(s32 mode) {
    D_8010410D = mode;
}

/* JFG: set three colour components (stored as words). */
void func_8006FC20(u8 red, u8 green, u8 blue) {
    D_800A6EA8 = red;
    D_800A6EAC = green;
    D_800A6EB0 = blue;
}

/**
 * Set the font of the current dialogue box's text (as DKR's Japanese version).
 * Official Name: fontUseFont
 */
void set_text_font(s32 fontID) {
    fontInUse = fontID;
    gDialogueBoxBackground[0].font = fontID;
}

/**
 * Sets the colour of the current dialogue box's text.
 * Official Name: fontColour
 */
void set_text_colour(s32 red, s32 green, s32 blue, s32 alpha, s32 opacity) {
    gDialogueBoxBackground[0].textColourR = red;
    gDialogueBoxBackground[0].textColourG = green;
    gDialogueBoxBackground[0].textColourB = blue;
    gDialogueBoxBackground[0].textColourA = alpha;
    gDialogueBoxBackground[0].opacity = opacity;
}

/**
 * Sets the colour of the current dialogue box's text background.
 * Official Name: fontBackground
 */
void set_text_background_colour(s32 red, s32 green, s32 blue, s32 alpha) {
    gDialogueBoxBackground[0].textBGColourR = red;
    gDialogueBoxBackground[0].textBGColourG = green;
    gDialogueBoxBackground[0].textBGColourB = blue;
    gDialogueBoxBackground[0].textBGColourA = alpha;
}

/**
 * Renders the given text at a given position in dialogue box 0.
 * JFG: a wrapper of draw_dialogue_text_pos_unused.
 */
void draw_text(Gfx **displayList, s32 xpos, s32 ypos, char *text, s32 alignmentFlags) {
    draw_dialogue_text_pos_unused(displayList, 0, xpos, ypos, text, alignmentFlags);
}

#define POS_CENTRED -0x8000

void render_text_string(Gfx **displayList, DialogueBoxBackground *box, char *text, s32 alignmentFlags);

/**
 * Text draw function that draws text in a dialogue box while overriding its position.
 * JFG: no scale argument for the text renderer.
 * Official Name: fontPrintWindowXY
 */
void draw_dialogue_text_pos_unused(Gfx **displayList, s32 dialogueBoxID, s32 xpos, s32 ypos, char *text,
                                   s32 alignmentFlags) {
    if (dialogueBoxID >= 0 && dialogueBoxID < DIALOGUEBOXBACKGROUND_COUNT) {
        DialogueBoxBackground *temp = &gDialogueBoxBackground[dialogueBoxID];
        temp->xpos = (xpos == POS_CENTRED) ? temp->width >> 1 : xpos;
        temp->ypos = (ypos == POS_CENTRED) ? temp->height >> 1 : ypos;
        render_text_string(displayList, temp, text, alignmentFlags);
    }
}

extern u8 *D_80104108; // JFG: the buffer render_text_string converts its text into
extern u8 D_8010410C;  // JFG: compact kerning (one pixel less per two-byte character)
extern u8 D_8010410D;  // JFG: glyph colours on (the text mode byte func_8006FC14 sets)
extern u8 *D_801040F8; // JFG: the glyph cache's image data
extern u8 *D_801040E8[]; // JFG: per font, the width of each two-byte character
void fontConvertString(u8 *in, u8 *out);
extern Gfx dDialogueBoxBegin[];
extern Gfx D_800A6E98[]; // DKR: &dDialogueBoxDrawModes[1]
s32 get_text_width(u8 *text, s32 font, s32 convert);
void viewport_scissor(Gfx **dList);
void rendermode_reset(Gfx **dList);

/* JFG: an entry of the glyph cache (256 blocks of 0x80 bytes). Size: 0x10 bytes */
typedef struct GlyphCacheEntry {
    /* 0x00 */ u8 font; // 0xFF: free
    /* 0x01 */ u8 ch;
    /* 0x02 */ u16 offset; // of the block in D_801040F8
    /* 0x04 */ u8 state;
    /* 0x05 */ u8 next; // how many blocks of the glyph follow
    /* 0x06 */ u16 image;  // offset of the glyph's image in D_801040F8
    /* 0x08 */ u16 image2; // and of its second image (fonts of format 4)
    /* 0x0A */ u8 s;  // the glyph's texture rectangle
    /* 0x0B */ u8 t;
    /* 0x0C */ u8 s2;
    /* 0x0D */ u8 t2;
    /* 0x0E */ u8 spacing;
    /* 0x0F */ u8 padF;
} GlyphCacheEntry;

GlyphCacheEntry *glyph_cache_get(u8 ch);

/* DKR include/f3ddkr.h: G_DMADL (7) runs numberOfCommands commands from address. */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(7, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                      \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }

/* The texture rectangle in the older microcode's form (RDPHALF_2, RDPHALF_CONT), as in 4E610.c */
#define gSPTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                    \
    {                                                                                         \
        Gfx *_g = (Gfx *) (pkt);                                                              \
                                                                                              \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));     \
        gImmp1(pkt, G_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                   \
        gImmp1(pkt, G_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));          \
    }

/**
 * Loops through a string, then draws each character onscreen.
 * Will also draw a fillrect if text backgrounds are enabled.
 * JFG: DKR's Japanese version of render_text_string (two-byte characters, glyphs from a cache), without
 * the scissor scale; the box is clamped to the screen, and one off the screen returns at once (leaving
 * the font switched); glyphs 2, 0xE and 0x41-0x44 recolour the text while D_8010410D is set. The commands after the
 * first go through a copy of *dList taken before it, so they overwrite that first gSPDisplayList.
 */
void render_text_string(Gfx **dList, DialogueBoxBackground *box, char *text, s32 alignmentFlags) {
    s32 prevFont;
    s32 xpos;
    s32 ypos;
    s32 ulx;
    s32 uly;
    s32 lrx;
    s32 lry;
    s32 charSpace;
    s32 textureS;
    s32 textureT;
    s32 xAlignmentDiff;
    u8 ch;
    s32 coloured;
    GlyphCacheEntry *glyph;
    u32 *cmds;
    FontData *fontData;
    Gfx *gfx;
    u8 *str;

    if (text != NULL) {
        gfx = *dList;
        fontConvertString((u8 *) text, D_80104108);
        prevFont = fontInUse;
        fontInUse = box->font;
        fontData = &gFonts[fontInUse];
        str = D_80104108;
        gSPDisplayList((*dList)++, dDialogueBoxBegin);
        if (box != gDialogueBoxBackground) {
            ulx = box->x1;
            uly = box->y1;
            lrx = box->x2;
            lry = box->y2;
            if (ulx > gDialogueBoxBackground[0].x2 || uly > gDialogueBoxBackground[0].y2 || lrx < 0 || lry < 0) {
                return;
            }
            if (ulx < 0) {
                ulx = 0;
            }
            if (uly < 0) {
                uly = 0;
            }
            if (lrx > gDialogueBoxBackground[0].x2) {
                lrx = gDialogueBoxBackground[0].x2;
            }
            if (lry > gDialogueBoxBackground[0].y2) {
                lry = gDialogueBoxBackground[0].y2;
            }
            gDPSetScissor(gfx++, G_SC_NON_INTERLACE, ulx, uly, lrx, lry);
        }
        xpos = box->xpos;
        ypos = box->ypos;
        xAlignmentDiff = -1;
        if (alignmentFlags & (1 | 4)) { // right or centred
            xAlignmentDiff = get_text_width(str, box->font, 0);
            if (alignmentFlags & 1) {
                xpos = (xpos - xAlignmentDiff) + 1;
            } else {
                xpos -= xAlignmentDiff >> 1;
            }
        }
        if (alignmentFlags & 2) { // bottom
            ypos = (ypos - fontData->y) + 1;
        }
        if (alignmentFlags & 8) { // middle
            ypos -= fontData->y >> 1;
        }
        if (box->textBGColourA != 0) {
            gDPSetEnvColor(gfx++, box->textBGColourR, box->textBGColourG, box->textBGColourB, box->textBGColourA);
            if (xAlignmentDiff == -1) {
                xAlignmentDiff = get_text_width(str, box->font, 0);
            }
            lrx = xpos + xAlignmentDiff;
            lry = fontData->y + ypos;
            gDPSetPrimColor(gfx++, 0, 0, D_800A6EA8, D_800A6EAC, D_800A6EB0, 0);
            gDkrDmaDisplayList(gfx++, OS_K0_TO_PHYSICAL(D_800A6E98), 2);
            gDPFillRectangle(gfx++, xpos + box->x1, ypos + box->y1, lrx + box->x1, lry + box->y1);
            gDPPipeSync(gfx++);
        }
        gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, box->opacity);
        gDPSetEnvColor(gfx++, box->textColourR, box->textColourG, box->textColourB, box->textColourA);
        coloured = FALSE;
        while (*str != '\0' && box->y2 >= ypos) {
            ch = *str++;
            if (ch & 0x80) {
                ch = *str++;
                if (ch == 0 || ch == 0xF) { // a space
                    charSpace = fontData->charWidth;
                } else {
                    if (D_8010410D) {
                        if (ch == 2) {
                            gDPPipeSync(gfx++);
                            gDPSetEnvColor(gfx++, 0, 0, 255, 255);
                            coloured = TRUE;
                        } else if (ch == 0xE) {
                            gDPPipeSync(gfx++);
                            gDPSetEnvColor(gfx++, 0, 255, 0, 255);
                            coloured = TRUE;
                        } else if (ch >= 0x41 && ch < 0x45) {
                            gDPPipeSync(gfx++);
                            gDPSetEnvColor(gfx++, 255, 255, 0, 255);
                            coloured = TRUE;
                        } else if (coloured) {
                            gDPPipeSync(gfx++);
                            gDPSetEnvColor(gfx++, box->textColourR, box->textColourG, box->textColourB,
                                           box->textColourA);
                            coloured = FALSE;
                        }
                    }
                    ulx = box->x1 + xpos;
                    uly = box->y1 + ypos;
                    lrx = fontData->width + ulx;
                    lry = fontData->y + uly;
                    charSpace = D_801040E8[fontInUse][ch];
                    if (lrx > 0 && lry > 0 && ulx < box->x2 && uly < box->y2) {
                        glyph = glyph_cache_get(ch);
                        if (glyph != NULL) {
                            ulx <<= 2;
                            uly = (glyph->t + uly) << 2;
                            charSpace = glyph->spacing;
                            lrx = (glyph->s2 - glyph->s) * 4 + ulx;
                            lry = (glyph->t2 - glyph->t) * 4 + uly;
                            if (lrx > 0 && lry > 0) {
                                textureS = glyph->s << 5;
                                textureT = glyph->t << 5;
                                if (ulx < 0) {
                                    textureS -= ulx << 3;
                                    ulx = 0;
                                }
                                if (uly < 0) {
                                    textureT -= uly << 3;
                                    uly = 0;
                                }
                                cmds = fontData->loadCmds;
                                gfx->words.w0 = cmds[0];
                                gfx->words.w1 = (u32) D_801040F8 + glyph->image;
                                gfx++;
                                cmds += 2;
                                if (fontData->format == 4) {
                                    gDkrDmaDisplayList(gfx++, OS_K0_TO_PHYSICAL(cmds), 6);
                                    gfx->words.w0 = cmds[12];
                                    gfx->words.w1 = (u32) D_801040F8 + glyph->image2;
                                    gfx++;
                                    cmds += 14;
                                }
                                gDkrDmaDisplayList(gfx++, OS_K0_TO_PHYSICAL(cmds), 8);
                                gSPTextureRectangleOld(gfx++, ulx, uly, lrx, lry, 0, textureS, textureT, 1 << 10,
                                                       1 << 10);
                            }
                        }
                    }
                }
                if (D_8010410C && charSpace) {
                    charSpace--;
                }
                xpos += charSpace;
            } else {
                switch (ch) {
                    case '\n': // newline
                        xpos = box->textOffsetX;
                        ypos += fontData->lineHeight;
                        break;
                    case '\t': // Tab
                        xpos = ((s32) xpos + (fontData->charWidth * 4)) -
                               ((xpos - box->textOffsetX) % (fontData->charWidth * 4));
                        break;
                    case '\v': // VT - Vertical Tab
                        ypos += fontData->lineHeight;
                        break;
                    case '\r': // Carriage Return
                        xpos = box->textOffsetX;
                        break;
                    default:
                        xpos += fontData->charWidth;
                        break;
                }
            }
        }
        box->xpos = xpos - box->textOffsetX;
        box->ypos = ypos - box->textOffsetY;
        fontInUse = prevFont; // Put previous font index back.
        *dList = gfx;
        rendermode_reset(dList);
        if (box != gDialogueBoxBackground) {
            viewport_scissor(dList);
        }
    }
}

extern u8 *D_801040E8[]; // JFG: per font, the width of each two-byte character
extern u8 *D_80104104;   // JFG: the buffer for converted strings
void fontConvertString(u8 *in, u8 *out);

/**
 * Returns the width of the text in the given font.
 * JFG: a byte with bit 7 set starts a two-byte character with a width of its
 * own; everything else is the font's fixed width. The string can be converted
 * to font codes first.
 * Official Name: fontStringWidth
 */
s32 get_text_width(u8 *text, s32 font, s32 convert) {
    s32 width;
    UNUSED s32 thisDiffX;
    s32 index;
    u8 *widths;
    u8 ch;
    FontData *fontData;
    u8 charWidth;
    s32 defaultWidth;

    fontData = &gFonts[font];
    widths = D_801040E8[font];
    if (convert) {
        fontConvertString(text, D_80104104);
        text = D_80104104;
    }
    width = 0;
    defaultWidth = fontData->charWidth;
    while (*text != '\0') {
        ch = *text++;
        charWidth = defaultWidth;
        if (ch & 0x80) {
            index = *text++;
            if (index != 0 && index != 0xF) {
                charWidth = widths[index];
            }
        }
        width += charWidth;
    }
    return width;
}

/**
 * Sets the position and size of the current dialogue box.
 * Official Name: fontWindowSize
 */
void set_current_dialogue_box_coords(s32 dialogueBoxID, s32 x1, s32 y1, s32 x2, s32 y2) {
    if (dialogueBoxID > 0 && dialogueBoxID < DIALOGUEBOXBACKGROUND_COUNT) {
        DialogueBoxBackground *temp = &gDialogueBoxBackground[dialogueBoxID];
        temp->xpos = 0;
        temp->ypos = 0;
        if (x1 < x2) {
            temp->x1 = (s16) x1;
            temp->x2 = (s16) x2;
        } else {
            temp->x2 = (s16) x1;
            temp->x1 = (s16) x2;
        }
        if (y1 < y2) {
            temp->y1 = (s16) y1;
            temp->y2 = (s16) y2;
        } else {
            temp->y2 = (s16) y1;
            temp->y1 = (s16) y2;
        }
        temp->width = (temp->x2 - temp->x1) + 1;
        temp->height = (temp->y2 - temp->y1) + 1;
    }
}

/**
 * Sets the active font of the current dialogue box (as DKR's Japanese version).
 * Official Name: fontWindowUseFont
 */
void set_dialogue_font(s32 dialogueBoxID, s32 font) {
    gDialogueBoxBackground[dialogueBoxID].font = font;
}

/**
 * Sets the background colour of the current dialogue box.
 * Official Name: fontWindowColour
 */
void set_current_dialogue_background_colour(s32 dialogueBoxID, s32 red, s32 green, s32 blue, s32 alpha) {
    if (dialogueBoxID > 0 && dialogueBoxID < DIALOGUEBOXBACKGROUND_COUNT) {
        DialogueBoxBackground *temp = &gDialogueBoxBackground[dialogueBoxID];
        temp->backgroundColourR = red;
        temp->backgroundColourG = green;
        temp->backgroundColourB = blue;
        temp->backgroundColourA = alpha;
    }
}

/**
 * Sets the text colour of the current dialogue box.
 * Official Name: fontWindowFontColour
 */
void set_current_text_colour(s32 dialogueBoxID, s32 red, s32 green, s32 blue, s32 alpha, s32 opacity) {
    DialogueBoxBackground *temp;
    if (dialogueBoxID <= 0 || dialogueBoxID >= DIALOGUEBOXBACKGROUND_COUNT) {
        return;
    }
    temp = &gDialogueBoxBackground[dialogueBoxID];
    temp->textColourR = red;
    temp->textColourG = green;
    temp->textColourB = blue;
    temp->textColourA = alpha;
    temp->opacity = opacity;
}

/**
 * Sets text background colour of the current dialogue box.
 * Official Name: fontWindowFontBackground
 */
void set_current_text_background_colour(s32 dialogueBoxID, s32 red, s32 green, s32 blue, s32 alpha) {
    DialogueBoxBackground *dialogueBox;
    if (dialogueBoxID <= 0 || dialogueBoxID >= DIALOGUEBOXBACKGROUND_COUNT) {
        return;
    }
    dialogueBox = &gDialogueBoxBackground[dialogueBoxID];
    dialogueBox->textBGColourR = red;
    dialogueBox->textBGColourG = green;
    dialogueBox->textBGColourB = blue;
    dialogueBox->textBGColourA = alpha;
}

/* Size: 0x20 bytes */
typedef struct DialogueTextElement {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 number;
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
    /* 0x1C */ DialogueBox *nextBox;
} DialogueTextElement;

extern DialogueTextElement gDialogueText[32]; // Official Name: String (JFG: 32, DKR: 64)
extern char *D_80104100;                      // JFG: the buffer for strings with a number filled in
void parse_string_with_number(char *input, char *output, s32 number);

#define HORZ_ALIGN_RIGHT 1
#define VERT_ALIGN_BOTTOM 2
#define HORZ_ALIGN_CENTER 4
#define VERT_ALIGN_MIDDLE 8

/**
 * Draws the text portion of a dialogue box that's passed through.
 * Binds the text to the box, then returns it.
 * JFG: a fixed array of 32 texts, and the number goes into a global buffer.
 * Official Name: fontWindowAddStringXY
 */
void *render_dialogue_text(s32 dialogueBoxID, s32 posX, s32 posY, char *text, s32 number, s32 flags) {
    s32 width;
    DialogueBox *textBox;
    DialogueTextElement *ret;
    s32 i;
    DialogueBoxBackground *bg;
    DialogueBox **textBoxPtr;
    FontData *fontData;

    if (text == NULL) {
        return NULL;
    }

    for (i = 0, ret = NULL; i < 32 && ret == NULL; i++) {
        if (gDialogueText[i].number == 255) {
            ret = &gDialogueText[i];
        }
    }

    if (ret != NULL) {
        bg = &gDialogueBoxBackground[dialogueBoxID];
        if (posX == POS_CENTRED) {
            posX = bg->width >> 1;
        }
        if (posY == POS_CENTRED) {
            posY = bg->height >> 1;
        }
        if (bg->font != 0xFF) {
            fontData = &gFonts[bg->font];
            if (flags & (HORZ_ALIGN_CENTER | HORZ_ALIGN_RIGHT)) {
                parse_string_with_number(text, D_80104100, number);
                width = get_text_width((u8 *) D_80104100, bg->font, TRUE);
                if (flags & HORZ_ALIGN_RIGHT) {
                    posX = (posX - width) + 1;
                } else {
                    posX = posX - (width >> 1);
                }
            }
            if (flags & VERT_ALIGN_BOTTOM) {
                posY = (posY - fontData->y) + 1;
            }
            if (flags & VERT_ALIGN_MIDDLE) {
                posY -= fontData->y >> 1;
            }
        }
        if (bg->textBox == NULL) {
            bg->textBox = (DialogueBox *) ret;
            ret->nextBox = NULL;
        } else {
            textBoxPtr = &bg->textBox;
            textBox = *textBoxPtr;
            while (textBox != NULL && number < textBox->textNum) {
                textBoxPtr = &textBox->nextBox;
                textBox = textBox->nextBox;
            }
            *textBoxPtr = (DialogueBox *) ret;
            ret->nextBox = textBox;
        }
        ret->number = number;
        ret->text = text;
        ret->posX = posX;
        ret->posY = posY;
        ret->offsetX = 0;
        ret->offsetY = 0;
        ret->textColourR = bg->textColourR;
        ret->textColourG = bg->textColourG;
        ret->textColourB = bg->textColourB;
        ret->textColourA = bg->textColourA;
        ret->textBGColourR = bg->textBGColourR;
        ret->textBGColourG = bg->textBGColourG;
        ret->textBGColourB = bg->textBGColourB;
        ret->textBGColourA = bg->textBGColourA;
        ret->opacity = bg->opacity;
        ret->font = bg->font;
        ret->flags = bg->flags;
    }

    return ret;
}

/**
 * Clear all strings from this dialogue box ID.
 * Official Name: fontWindowFlushStrings
 */
void dialogue_clear(s32 dialogueBoxID) {
    DialogueBoxBackground *dialogueBox;
    DialogueBox *dialogueTextBox, *dialogueTextBoxTemp;

    dialogueBox = &gDialogueBoxBackground[dialogueBoxID];
    dialogueTextBox = dialogueBox->textBox;
    if (dialogueTextBox != NULL) {
        dialogueTextBoxTemp = dialogueTextBox; // This seems redundant.
        while (dialogueTextBoxTemp != NULL) {
            dialogueTextBoxTemp->textNum = DIALOGUE_NUM_NULL;
            dialogueTextBoxTemp = dialogueTextBoxTemp->nextBox;
        }
        dialogueBox->textBox = NULL;
    }
}

/**
 * Mark the selected dialogue box as open
 */
void open_dialogue_box(s32 dialogueBoxID) {
    gDialogueBoxBackground[dialogueBoxID].flags |= DIALOGUE_BOX_OPEN;
}

/**
 * Mark the selected dialogue box as closed
 * Official Name: fontWindowDisable
 */
void dialogue_close(s32 dialogueBoxID) {
    gDialogueBoxBackground[dialogueBoxID].flags &= DIALOGUE_BOX_CLOSED;
}

/**
 * Convert an integer to a string.
 * JFG: the table of powers of ten ends with a zero instead of a count.
 * Official Name: fontStringAddNumber
 */
void s32_to_string(char **outString, s32 number) {
    u8 digit;
    s32 i;
    s32 hasDigit; // boolean
    s32 div;
    char *ret = *outString;

    // Check for a negative number
    if (number < 0) {
        *ret = '-';
        ret++;
        number = -number;
    }

    // Loop through digit places.
    hasDigit = FALSE;
    for (i = 0; gDescPowsOf10[i] != 0; i++) {
        digit = '0';
        if (number >= gDescPowsOf10[i]) {
            div = number / gDescPowsOf10[i];
            hasDigit = TRUE;
            number -= div * gDescPowsOf10[i];
            digit = div + '0';
        }
        if (hasDigit) {
            *ret = digit;
            ret++;
        }
    }

    // One's digit place
    *ret = '0' + number;
    ret++;
    *outString = ret;
}

typedef struct Vertex Vertex;

void render_dialogue_box(Gfx **dList, Mtx **mat, Vertex **verts, s32 dialogueBoxID);

/**
 * Main function to render each dialogue box entry.
 * JFG: no close timer, and never passes the matrix and vertex lists.
 * Official Name: fontWindowsDraw
 */
void render_dialogue_boxes(Gfx **dList, Mtx **mat, Vertex **verts) {
    s32 i;

    for (i = 1; i < DIALOGUEBOXBACKGROUND_COUNT; i++) {
        if (gDialogueBoxBackground[i].flags & DIALOGUE_BOX_OPEN) {
            render_dialogue_box(dList, NULL, NULL, i);
        }
    }
}

void fb_size(u32 *width, u32 *height);

/**
 * Draws a solid color rectangle at the coordinates specified.
 * ulx, uly = upper-left position
 * lrx, lry = lower-right position
 * JFG: gets the size through pointers.
 */
void render_fill_rectangle(Gfx **dList, s32 ulx, s32 uly, s32 lrx, s32 lry) {
    u32 width;
    u32 height;

    fb_size(&width, &height);
    if (lrx >= 0 && (u32) ulx < width && lry >= 0 && (u32) uly < height) {
        if (ulx < 0) {
            ulx = 0;
        }
        if (uly < 0) {
            uly = 0;
        }
        gDPFillRectangle((*dList)++, ulx, uly, lrx, lry);
    }
}

extern s8 sDialogueBoxDimensions[];
void render_fill_rectangle(Gfx **dList, s32 ulx, s32 uly, s32 lrx, s32 lry);

/**
 * Render the selected dialogue box. Background first, then text.
 * JFG: a primitive colour from D_800A6EA8..B0 first, no matrix/vertex set-up (the
 * arguments stay), and the text goes through the shared number buffer.
 * Official Name: fontWindowDraw
 */
void render_dialogue_box(Gfx **dList, UNUSED Mtx **mat, UNUSED Vertex **verts, s32 dialogueBoxID) {
    DialogueBoxBackground *dialogueBox;
    DialogueTextElement *dialogueTextBox;
    s32 i;
    s32 x1, x2;
    s32 y1, y2;

    dialogueBox = &gDialogueBoxBackground[dialogueBoxID];

    // Render dialogue box background.
    if (dialogueBox->backgroundColourA != 0) {
        gSPDisplayList((*dList)++, dDialogueBoxBegin);
        gDPSetPrimColor((*dList)++, 0, 0, D_800A6EA8, D_800A6EAC, D_800A6EB0, 0);
        gDkrDmaDisplayList((*dList)++, OS_K0_TO_PHYSICAL(D_800A6E98), 2);
        gDPSetEnvColor((*dList)++, 0, 0, 0, 0);
        if (dialogueBox->x2 - dialogueBox->x1 < 10 || dialogueBox->y2 - dialogueBox->y1 < 10) {
            render_fill_rectangle(dList, dialogueBox->x1 - 2, dialogueBox->y1 - 2, dialogueBox->x2 + 2,
                                  dialogueBox->y2 + 2);
        } else {
            render_fill_rectangle(dList, dialogueBox->x1 - 2, dialogueBox->y1 + 2, dialogueBox->x1 + 2,
                                  dialogueBox->y2 - 2);
            render_fill_rectangle(dList, dialogueBox->x1 - 2, dialogueBox->y1 - 2, dialogueBox->x2 + 2,
                                  dialogueBox->y1 + 2);
            render_fill_rectangle(dList, dialogueBox->x2 - 2, dialogueBox->y1 + 2, dialogueBox->x2 + 2,
                                  dialogueBox->y2 - 2);
            render_fill_rectangle(dList, dialogueBox->x1 - 2, dialogueBox->y2 - 2, dialogueBox->x2 + 2,
                                  dialogueBox->y2 + 2);
        }
        gDPPipeSync((*dList)++);
        gDPSetEnvColor((*dList)++, dialogueBox->backgroundColourR, dialogueBox->backgroundColourG,
                       dialogueBox->backgroundColourB, dialogueBox->backgroundColourA);
        // Loops through sDialogueBoxDimensions, rendering fillrects until it forms a rounded rectangle.
        for (i = 0; sDialogueBoxDimensions[i] >= 0; i += 5) {
            x1 = sDialogueBoxDimensions[i] + dialogueBox->x1;
            y1 = (sDialogueBoxDimensions[i + 1]) ? sDialogueBoxDimensions[i + 2] + dialogueBox->y2
                                                 : sDialogueBoxDimensions[i + 2] + dialogueBox->y1;
            x2 = dialogueBox->x2 - sDialogueBoxDimensions[i];
            y2 = (sDialogueBoxDimensions[i + 3]) ? sDialogueBoxDimensions[i + 4] + dialogueBox->y2
                                                 : sDialogueBoxDimensions[i + 4] + dialogueBox->y1;
            render_fill_rectangle(dList, x1, y1, x2, y2);
        }
        gDPPipeSync((*dList)++);
    }
    // Set and render the text portions of the dialogue box.
    dialogueTextBox = (DialogueTextElement *) dialogueBox->textBox;
    while (dialogueTextBox != NULL) {
        dialogueBox->xpos = dialogueTextBox->posX + dialogueTextBox->offsetX;
        dialogueBox->ypos = dialogueTextBox->posY + dialogueTextBox->offsetY;
        dialogueBox->textColourR = dialogueTextBox->textColourR;
        dialogueBox->textColourG = dialogueTextBox->textColourG;
        dialogueBox->textColourB = dialogueTextBox->textColourB;
        dialogueBox->textColourA = dialogueTextBox->textColourA;
        dialogueBox->textBGColourR = dialogueTextBox->textBGColourR;
        dialogueBox->textBGColourG = dialogueTextBox->textBGColourG;
        dialogueBox->textBGColourB = dialogueTextBox->textBGColourB;
        dialogueBox->textBGColourA = dialogueTextBox->textBGColourA;
        dialogueBox->opacity = dialogueTextBox->opacity;
        dialogueBox->font = dialogueTextBox->font;
        parse_string_with_number(dialogueTextBox->text, D_80104100, dialogueTextBox->number);
        render_text_string(dList, dialogueBox, D_80104100, 0);
        dialogueTextBox = (DialogueTextElement *) dialogueTextBox->nextBox;
    }
}

/**
 * Takes in a string and a number, and replaces each instance of the
 * character '~' with the number.
 * JFG: two-byte characters (first byte with bit 7 set) are copied whole, and
 * the two-byte code 0xE also stands for the number; the terminator is copied
 * by the loop itself.
 */
void parse_string_with_number(char *input, char *output, s32 number) {
    u8 ch;
    u8 ch2;

    do {
        ch = *input++;
        if (ch & 0x80) {
            ch2 = *input++;
            if (ch2 == 0xE) {
                s32_to_string(&output, number);
            } else {
                *output++ = ch;
                *output++ = ch2;
            }
        } else if (ch == '~') { // ~ is equivalent to a %d.
            s32_to_string(&output, number);
        } else {
            *output++ = ch;
        }
    } while (ch);
}

/* JFG: the header of a glyph image (asset 0x3B). Size: 0x10 bytes */
typedef struct GlyphHeader {
    /* 0x00 */ u16 width;
    /* 0x02 */ u16 height;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 pad9[7];
} GlyphHeader;

extern GlyphCacheEntry *D_801040FC;
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

#ifdef NON_MATCHING
/* JFG: find the cached glyph of the current font, or load it into free cache blocks (keeping the 16
   bytes the glyph's header overwrites in front of its block).
   NON_MATCHING: the target keeps the 4-word save as a loop (IDO unrolls this one), ch out of $s0,
   and other argument registers for the counters (frame 0x70). */
GlyphCacheEntry *glyph_cache_get(u8 ch) {
    GlyphCacheEntry *entry;
    FontData *font;
    s32 i;
    s32 blocks;
    s32 run;
    u32 *src;
    u32 *dst;
    u32 saved[4];
    GlyphHeader *glyph;
    u32 j;
    s32 fontID;
    GlyphCacheEntry *found;

    fontID = fontInUse;
    found = NULL;
    i = 0;
    font = &gFonts[fontID];
    entry = D_801040FC;
    do {
        i++;
        if (fontID == entry->font && ch == entry->ch) {
            found = entry;
        }
        entry++;
    } while (i < 256 && found == NULL);
    entry = D_801040FC;
    if (found != NULL) {
        entry = found;
        do {
            run = entry->next;
            entry->state = 2;
            entry++;
        } while (run);
        return found;
    }
    i = 0;
    blocks = (u32) (font->glyphSize + 0x6F) >> 7;
    run = 0;
    if (blocks) {
        do {
            i++;
            if (entry->state == 0) {
                if (run == 0) {
                    found = entry;
                }
                run++;
            } else {
                run = 0;
            }
            entry++;
        } while (i < 256 && run != blocks);
    }
    if (run == blocks) {
        entry = found;
        for (i = blocks - 1; i >= 0; i--) {
            blocks = entry->next;
            entry->next = i;
            entry->font = fontID;
            entry->ch = ch;
            entry->state = 2;
            entry++;
        }
        while (blocks) {
            blocks = entry->next;
            entry->font = 0xFF;
            entry->next = 0;
            entry++;
        }
        glyph = (GlyphHeader *) (D_801040F8 + found->offset - 16);
        src = (u32 *) glyph;
        dst = saved;
        j = 0;
        do {
            *dst++ = *src++;
        } while (++j < 4);
        asset_load(0x3B, (u32) glyph, font->dataOffset + ch * font->glyphSize, font->glyphSize);
        found->image = found->offset + glyph->width - 16;
        found->image2 = found->offset + glyph->height - 16;
        found->s = glyph->unk4;
        found->t = glyph->unk5;
        found->s2 = glyph->unk6;
        found->t2 = glyph->unk7;
        found->spacing = glyph->unk8;
        ((u32 *) glyph)[0] = saved[0];
        ((u32 *) glyph)[1] = saved[1];
        ((u32 *) glyph)[2] = saved[2];
        ((u32 *) glyph)[3] = saved[3];
    } else {
        found = NULL;
    }
    return found;
}
#else
INCLUDE_ASM("asm/nonmatchings/707F0", func_80071388);
#endif

void *mempool_alloc_safe(s32 size, u32 colourTag);
extern u32 D_800A6E68[]; // JFG: the draw modes (two commands) that end a font's load commands
extern u32 D_800A6E78[]; // JFG: the same for fonts of format 4
extern u32 D_800A6E88[]; // JFG: the same for fonts of format 7

/**
 * JFG: build a font's texture load commands (0x80 bytes, kept at font->loadCmds) for its format, loading
 * from the glyph cache (render_text_string patches each glyph's image address into the first command), and
 * end them with the format's draw modes.
 * DKR: fontCreateDisplayList (font.c), one load for the whole font.
 * Official Name: fontCreateDisplayList
 */
void fontCreateDisplayList(FontData *font) {
    Gfx *dl;
    u32 *mode;

    dl = (Gfx *) (font->loadCmds = mempool_alloc_safe(0x80, 0xFF0000FF));
    if (dl != NULL) {
        mode = D_800A6E68;
        switch (font->format) {
            case 4: // RGBA16 and I4
                gDPLoadTextureBlockS(dl++, D_801040F8, G_IM_FMT_RGBA, G_IM_SIZ_16b, font->width, font->y, 0,
                                     G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                     G_TX_NOLOD);
                gDPLoadMultiBlock_4bS(dl++, D_801040F8, 0x100, 1, G_IM_FMT_I, font->width, font->y, 0,
                                      G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                      G_TX_NOLOD);
                mode = D_800A6E78;
                break;
            case 2: // RGBA32
                gDPLoadTextureBlockS(dl++, D_801040F8, G_IM_FMT_RGBA, G_IM_SIZ_32b, font->width, font->y, 0,
                                     G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                     G_TX_NOLOD);
                break;
            case 3: // RGBA16
                gDPLoadTextureBlockS(dl++, D_801040F8, G_IM_FMT_RGBA, G_IM_SIZ_16b, font->width, font->y, 0,
                                     G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                     G_TX_NOLOD);
                break;
            case 1: // IA8
                gDPLoadTextureBlockS(dl++, D_801040F8, G_IM_FMT_IA, G_IM_SIZ_8b, font->width, font->y, 0,
                                     G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                     G_TX_NOLOD);
                break;
            case 0: // IA4
                gDPLoadTextureBlock_4bS(dl++, D_801040F8, G_IM_FMT_IA, font->width, font->y, 0,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                        G_TX_NOLOD);
                break;
            case 5: // I4
                gDPLoadTextureBlock_4bS(dl++, D_801040F8, G_IM_FMT_I, font->width, font->y, 0,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                        G_TX_NOLOD);
                break;
            case 6: // I8
                gDPLoadTextureBlockS(dl++, D_801040F8, G_IM_FMT_I, G_IM_SIZ_8b, font->width, font->y, 0,
                                     G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                     G_TX_NOLOD);
                break;
            case 7: // I4, other draw modes
                gDPLoadTextureBlock_4bS(dl++, D_801040F8, G_IM_FMT_I, font->width, font->y, 0,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 8, 8, G_TX_NOLOD,
                                        G_TX_NOLOD);
                mode = D_800A6E88;
                break;
        }
        dl->words.w0 = mode[0];
        dl->words.w1 = mode[1];
        dl++;
        dl->words.w0 = mode[2];
        dl->words.w1 = mode[3];
    }
}

extern u8 gFontConversionTable[]; // JFG: the font code of each ASCII character from ' '

/* Converts a string to font codes (0x800720FC). */
/* JFG: convert a string to font codes: ASCII from ' ' becomes 0x80 and its code, two-byte characters
 * (bit 7 set) are copied, control characters are kept. */
void fontConvertString(u8 *in, u8 *out) {
    u8 ch;
    u8 *table = gFontConversionTable;

    do {
        ch = *in++;
        if (ch & 0x80) {
            *out++ = ch;
            *out++ = *in++;
        } else if (ch < 0x20) {
            *out++ = ch;
        } else {
            *out++ = 0x80;
            *out++ = table[ch - 0x20];
        }
    } while (ch);
}

#ifdef NON_MATCHING
/* NON_MATCHING: register allocation only: the target keeps 0x80/0xF in t3/t4 and the D_801040E8 address unhoisted (4 saved registers, mine 7) */
/* JFG: word wrap: from text, find where the line that fits maxWidth ends (spaces are 0x80 0x0F); the line's
 * first word goes to *lineStart. Returns NULL at the end of the text or if not even one word fits. */
u8 *func_8007216C(s32 font, u8 *text, s32 maxWidth, u8 **lineStart) {
    s32 width = 0;
    s32 first = TRUE;
    s32 overflow = FALSE;
    s32 end = FALSE;
    s32 brk = FALSE;
    s32 wordWidth;
    u8 *wordStart;
    u8 ch;
    u8 *widths;

    do {
        wordWidth = 0;
        wordStart = text;
        do {
            ch = *text;
            if (ch == 0x80 && text[1] == 0xF) {
                text += 2;
                if (!first) {
                    wordWidth += gFonts[font].charWidth;
                }
            } else if (ch == 0) {
                end = TRUE;
            } else if (ch != 0x80) {
                text++;
            } else {
                brk = TRUE;
            }
        } while (!brk && !end);
        if (first) {
            first = FALSE;
            if (end) {
                return NULL;
            }
            *lineStart = text;
        }
        ch = *text;
        if (ch == 0x80 && text[1] != 0xF) {
            widths = D_801040E8[font];
            do {
                wordWidth += widths[text[1]];
                ch = text[2];
                text += 2;
            } while (ch == 0x80 && text[1] != 0xF);
        }
        width += wordWidth;
        if (maxWidth < width) {
            ch = *wordStart;
            overflow = TRUE;
            width -= wordWidth;
            text = wordStart;
        }
        if (ch == 0) {
            end = TRUE;
        }
        brk = FALSE;
    } while (!overflow && !end);
    if (text == *lineStart) {
        return NULL;
    }
    return text;
}
#else
INCLUDE_ASM("asm/nonmatchings/707F0", func_8007216C);
#endif

u8 func_8007230C(s32 font) {
    return gFonts[font].lineHeight;
}
