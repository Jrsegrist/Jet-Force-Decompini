#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern char *gCurrentTextProperties;
extern char *gSubtitleProperties[1];
extern s16 gCurrentTextID;
extern s16 gShowSubtitles;
extern s16 gSubtitleLineCount;
extern s16 gSubtitleTimer;

extern s8 gMessageTextAllocated;
extern char *gCurrentMessageText[2];
void mempool_free(void *data);
void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 asset_table_size(u32 assetIndex);
extern char *D_80104560;
extern s16 gTextTableEntries;
extern s16 gDialogueAlpha;
extern s16 gTextAlphaVelocity;
extern s16 gDialogueXPos1;
extern s16 gDialogueYPos1;
extern s16 gDialogueXPos2;
extern s16 gDialogueYPos2;
extern s32 D_80104594;

/* Sets up the dialogue defaults and allocates the message text (0x80073670). */
/**
 * Set the default values of dialogue and allocate memory for the active text entry.
 * JFG: also counts the text table entries and sets up a third buffer.
 * DKR: init_dialogue_text
 */
void init_dialogue_text(void) {
    gTextTableEntries = (asset_table_size(5) >> 2) - 2;
    gCurrentMessageText[0] = (char *) mempool_alloc_safe(0x790, 0xFF00FF);
    gCurrentMessageText[1] = gCurrentMessageText[0] + 0x3C0;
    D_80104560 = gCurrentMessageText[1] + 0x3C0;
    D_80104594 = 0;
    gShowSubtitles = FALSE;
    gDialogueAlpha = 0;
    gTextAlphaVelocity = 32;
    gCurrentTextID = 0;
    gDialogueXPos1 = 32;
    gDialogueXPos2 = 288;
    if (osTvType == OS_TV_PAL) {
        gDialogueYPos1 = 224;
        gDialogueYPos2 = 248;
    } else {
        gDialogueYPos1 = 202;
        gDialogueYPos2 = 222;
    }
    gMessageTextAllocated = TRUE;
}
void dialogue_close(s32 dialogueBoxID);
void dialogue_clear(s32 dialogueBoxID);

/* Closes the screen dialogue and frees its text (0x80073758). */
void free_message_box(void) {
    if (gMessageTextAllocated) {
        mempool_free(gCurrentMessageText[0]);
        dialogue_close(6);
        dialogue_clear(6);
        gMessageTextAllocated = 0;
        gShowSubtitles = 0;
    }
}

extern s32 gSubtitleSetting;

/* Sets whether subtitles may show (0x800737AC). */
void set_subtitles(s32 arg0) {
    gSubtitleSetting = arg0;
}

INCLUDE_ASM("asm/nonmatchings/74270", func_800737B8);

/**
 * Get the line count and text timer from the next message of the subtitle.
 * Close the subtitles if none can be found.
 */
void find_next_subtitle(void) {
    u32 new_var3;
    u8 new_var;
    s32 new_var2;
    s32 done;

    gSubtitleLineCount = 0;
    gSubtitleTimer = 0;
    done = FALSE;
    while (gCurrentTextProperties[0] != NULL && done == FALSE) {
        gCurrentTextID = gCurrentTextProperties[0] - 1;
        gSubtitleProperties[gSubtitleLineCount] = gCurrentTextProperties;
        gSubtitleTimer = normalise_time(gCurrentTextProperties[7] * 6);
        gCurrentTextProperties += 8;
        do {
            new_var = gCurrentTextProperties[0];
            if (new_var & 0x80) {
                gCurrentTextProperties += 2;
            } else {
                gCurrentTextProperties++;
            }
        } while (gCurrentTextProperties[0] != NULL);
        gSubtitleLineCount++;
        if (gSubtitleLineCount >= 2) {
            done = TRUE;
        }
        gCurrentTextProperties++;
        new_var2 = gCurrentTextProperties[0];
        if (gCurrentTextProperties[0] == 10) {
            gCurrentTextProperties++;
        } else if (new_var2 == 12) {
            gCurrentTextProperties++;
            done = TRUE;
        }
    }
    if (gSubtitleLineCount > 0) {
        gShowSubtitles = TRUE;
    }
}

INCLUDE_ASM("asm/nonmatchings/74270", func_80073BDC);

INCLUDE_ASM("asm/nonmatchings/74270", func_80073D14);
