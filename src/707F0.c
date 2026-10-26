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
    u8 pad0;
    u8 y;
    u8 charWidth; // the width of a character without its own entry
    u8 unk3;
    u8 pad4[0xC];
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

/* JFG: set the size of dialogue box 0 (the whole screen). */
void func_8006FBF0(s32 width, s32 height) {
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

void func_8006FD98(Gfx **displayList, DialogueBoxBackground *box, char *text, s32 alignmentFlags);

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
        func_8006FD98(displayList, temp, text, alignmentFlags);
    }
}

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FD98);

extern u8 *D_801040E8[]; // JFG: per font, the width of each two-byte character
extern u8 *D_80104104;   // JFG: the buffer for converted strings
void func_800720FC(u8 *in, u8 *out);

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
        func_800720FC(text, D_80104104);
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
 * Sets the active front of the current dialogue box (as DKR's Japanese version).
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

/* DKR include/f3ddkr.h: G_DMADL (7) runs numberOfCommands commands from address. */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(7, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                      \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }
extern Gfx dDialogueBoxBegin[];
extern Gfx D_800A6E98[]; // DKR: &dDialogueBoxDrawModes[1]
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
        func_8006FD98(dList, dialogueBox, D_80104100, 0);
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

INCLUDE_ASM("asm/nonmatchings/707F0", func_80071388);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800715E8);

extern u8 D_800A6F04[]; // JFG: the font code of each ASCII character from ' '

/* JFG: convert a string to font codes: ASCII from ' ' becomes 0x80 and its code, two-byte characters
 * (bit 7 set) are copied, control characters are kept. */
void func_800720FC(u8 *in, u8 *out) {
    u8 ch;
    u8 *table = D_800A6F04;

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

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007216C);

u8 func_8007230C(s32 font) {
    return gFonts[font].unk3;
}
