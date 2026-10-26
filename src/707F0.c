/*
 * Text and dialogue boxes. Functions in C are from the Diddy Kong Racing
 * decompilation (CC0 1.0), src/font.c; see CREDITS.md.
 */
#include "common.h"

/* DKR src/font.h (JFG layout) */
#define DIALOGUE_NUM_NULL 0xFF

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

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FBF0);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FC14);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FC20);

/**
 * Set the font of the current dialogue box's text (as DKR's Japanese version).
 * Official Name: fontUseFont
 */
void set_text_font(s32 fontID) {
    fontInUse = fontID;
    gDialogueBoxBackground[0].font = fontID;
}

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FC68);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FC8C);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FCA8);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FCEC);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8006FD98);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800706D4);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007078C);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007082C);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070848);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070888);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800708D0);

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

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070C4C);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070C78);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070CA4);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070D68);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070DE4);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80070EAC);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007128C);

INCLUDE_ASM("asm/nonmatchings/707F0", func_80071388);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800715E8);

INCLUDE_ASM("asm/nonmatchings/707F0", func_800720FC);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007216C);

INCLUDE_ASM("asm/nonmatchings/707F0", func_8007230C);
