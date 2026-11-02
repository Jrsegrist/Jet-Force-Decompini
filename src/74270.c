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

void set_current_dialogue_box_coords(s32 dialogueBoxID, s32 x1, s32 y1, s32 x2, s32 y2);
void set_current_dialogue_background_colour(s32 dialogueBoxID, s32 red, s32 green, s32 blue, s32 alpha);
void set_current_text_background_colour(s32 dialogueBoxID, s32 red, s32 green, s32 blue, s32 alpha);
void set_dialogue_font(s32 dialogueBoxID, s32 font);
void set_current_text_colour(s32 dialogueBoxID, s32 red, s32 green, s32 blue, s32 alpha, s32 opacity);
void render_dialogue_text(s32 dialogueBoxID, s32 posX, s32 posY, char *text, s32 number, s32 flags);
void open_dialogue_box(s32 dialogueBoxID);

/**
 * Render currently active subtitles on the screen.
 * JFG: a green-blue box, no offset for two lines.
 */
void render_subtitles(void) {
    s32 textX;
    s32 textY;
    s32 i;
    s32 textFlags;
    char **textData;

    dialogue_clear(6);
    set_current_dialogue_box_coords(6, gDialogueXPos1, gDialogueYPos1, gDialogueXPos2, gDialogueYPos2);
    set_current_dialogue_background_colour(6, 0, 96, 0, (gDialogueAlpha * 100) >> 8);
    set_current_text_background_colour(6, 0, 0, 0, 0);
    textY = ((((gDialogueYPos2 - gDialogueYPos1) - (gSubtitleLineCount * 12)) - (gSubtitleLineCount * 2)) + 2) >> 1;
    for (i = 0; i < gSubtitleLineCount; i++) {
        textData = &gSubtitleProperties[0];
        set_dialogue_font(6, (s32) textData[i][5]);
        textFlags = textData[i][6];
        if (textFlags == 4) {
            textX = (gDialogueXPos2 - gDialogueXPos1) >> 1;
        } else {
            if (textFlags == 1) {
                textX = (gDialogueXPos2 - gDialogueXPos1) - 8;
            } else {
                textX = 8;
            }
        }
        set_current_text_colour(6, textData[i][1], textData[i][2], textData[i][3], 255,
                                (textData[i][4] * gDialogueAlpha) >> 8);
        render_dialogue_text(6, textX, textY, textData[i] + 8, 1, textFlags);
        set_current_text_colour(6, 0, 0, 0, 255, (gDialogueAlpha * 255) >> 8);
        render_dialogue_text(6, textX + 1, textY + 1, textData[i] + 8, 1, textFlags);
        textY += 14;
    }
    open_dialogue_box(6);
}

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

void render_subtitles(void);

/**
 * Handle the subtitle system from here.
 * Slowly show the text, tick down the timer, find the next message or close the box, then render.
 * JFG: only while the message text is allocated.
 */
void process_subtitles(u32 updateRate) {
    if (gMessageTextAllocated) {
        if (gSubtitleSetting == FALSE) {
            gShowSubtitles = FALSE;
        }
        if (gShowSubtitles) {
            if (gSubtitleTimer <= 0) {
                gDialogueAlpha -= updateRate * gTextAlphaVelocity;
                if (gDialogueAlpha < 0) {
                    gDialogueAlpha = 0;
                    gShowSubtitles = FALSE;
                    dialogue_close(6);
                    dialogue_clear(6);
                }
            } else {
                gDialogueAlpha += updateRate * gTextAlphaVelocity;
                if (gDialogueAlpha > 256) {
                    gDialogueAlpha = 256;
                }
                gSubtitleTimer -= updateRate;
                if (gSubtitleTimer <= 0) {
                    find_next_subtitle();
                }
            }
        }
        if (gShowSubtitles) {
            render_subtitles();
        }
    }
}

s32 get_language(void);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/**
 * Set the current text index based on the entry, and show it as a subtitle.
 * JFG: a text block per language; every entry is a subtitle.
 */
void set_current_text(s32 textID) {
    s32 *entries;
    s32 temp;
    s32 size;

    if (gMessageTextAllocated && textID >= 0 && textID < gTextTableEntries) {
        switch (get_language()) {
            case 2:
                textID += 48;
                break;
            case 1:
                textID += 47;
                break;
            case 3:
                textID += 49;
                break;
            case 4:
                textID += 50;
                break;
        }
        asset_load(5, (u32) D_80104560, (textID & ~1) << 2, 16);
        entries = (s32 *) D_80104560;
        temp = entries[textID & 1] & 0xFF000000;
        size = (entries[(textID & 1) + 1] & 0xFFFFFF) - (entries[textID & 1] & 0xFFFFFF);
        asset_load(4, (u32) gCurrentMessageText[D_80104594], entries[textID & 1] ^ temp, size);
        gCurrentTextProperties = gCurrentMessageText[D_80104594];
        find_next_subtitle();
        D_80104594 = (D_80104594 + 1) & 1;
    }
}
