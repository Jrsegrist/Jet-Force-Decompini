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

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FCEC);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FD98);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800706D4);

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

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070910);

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

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070D68);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070DE4);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070EAC);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007128C);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80071388);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800715E8);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800720FC);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007216C);

/* Size: 0x10 bytes */
typedef struct FontData {
    u8 pad0[3];
    u8 unk3;
    u8 pad4[0xC];
} FontData;
extern FontData *gFonts; // Official Name: Font

u8 func_8007230C(s32 font) {
    return gFonts[font].unk3;
}
