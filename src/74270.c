#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern char *gCurrentTextProperties;
extern char *gSubtitleProperties[1];
extern s16 gCurrentTextID;
extern s16 gShowSubtitles;
extern s16 gSubtitleLineCount;
extern s16 gSubtitleTimer;

extern s8 D_800A6FB0;
extern char *D_80104588[2]; // DKR: gCurrentMessageText
void mempool_free(void *data);
void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 asset_table_size(u32 assetIndex);
extern char *D_80104560;
extern s16 D_80104564; // DKR: gTextTableEntries
extern s16 D_80104566;
extern s16 D_80104568;
extern s16 D_8010456C;
extern s16 D_8010456E;
extern s16 D_80104570;
extern s16 D_80104572;
extern s32 D_80104594;

/**
 * Set the default values of dialogue and allocate memory for the active text entry.
 * JFG: also counts the text table entries and sets up a third buffer.
 * DKR: init_dialogue_text
 */
void func_80073670(void) {
    D_80104564 = (asset_table_size(5) >> 2) - 2;
    D_80104588[0] = (char *) mempool_alloc_safe(0x790, 0xFF00FF);
    D_80104588[1] = D_80104588[0] + 0x3C0;
    D_80104560 = D_80104588[1] + 0x3C0;
    D_80104594 = 0;
    gShowSubtitles = FALSE;
    D_80104566 = 0;
    D_80104568 = 32;
    gCurrentTextID = 0;
    D_8010456C = 32;
    D_80104570 = 288;
    if (osTvType == OS_TV_PAL) {
        D_8010456E = 224;
        D_80104572 = 248;
    } else {
        D_8010456E = 202;
        D_80104572 = 222;
    }
    D_800A6FB0 = TRUE;
}
void dialogue_close(s32 dialogueBoxID);
void dialogue_clear(s32 dialogueBoxID);

void func_80073758(void) {
    if (D_800A6FB0) {
        mempool_free(D_80104588[0]);
        dialogue_close(6);
        dialogue_clear(6);
        D_800A6FB0 = 0;
        gShowSubtitles = 0;
    }
}

extern s32 D_800A6FB8;

void func_800737AC(s32 arg0) {
    D_800A6FB8 = arg0;
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
