#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern char *gCurrentTextProperties;
extern char *gSubtitleProperties[1];
extern s16 gCurrentTextID;
extern s16 gShowSubtitles;
extern s16 gSubtitleLineCount;
extern s16 gSubtitleTimer;

INCLUDE_ASM("asm/nonmatchings/74270", func_80073670);

INCLUDE_ASM("asm/nonmatchings/74270", func_80073758);

INCLUDE_ASM("asm/nonmatchings/74270", func_800737AC);

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
