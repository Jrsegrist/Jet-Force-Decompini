#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern u8 gJinglesOff;

/* DKR src/audio_spatial.h (reduced) */
typedef struct AudioPoint {
    struct {
        f32 x, y, z;
    } pos;
} AudioPoint;

extern AudioPoint **gAudioPoints;
extern u16 gNumAudioPoints;
void audspat_point_stop_by_index(s32 index);

INCLUDE_ASM("asm/nonmatchings/3430", func_80002830);

/**
 * Stop any playing jingles, then block audio lines from playing anymore.
 * Does not stop audio lines that are playing sounds of type AUDIO_LINE_TYPE_SOUND.
 * Official Name: amAmbientPause
 */
void audspat_jingle_off(void) {
    music_jingle_stop();
    gJinglesOff = TRUE;
}

/**
 * Allow audio lines to play jingles.
 * Official Name: amAmbientRestart
 */
void audspat_jingle_on(void) {
    gJinglesOff = FALSE;
}

INCLUDE_ASM("asm/nonmatchings/3430", func_80002944);

INCLUDE_ASM("asm/nonmatchings/3430", func_80002AB8);

INCLUDE_ASM("asm/nonmatchings/3430", func_800032B4);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003410);

INCLUDE_ASM("asm/nonmatchings/3430", func_800034C4);

INCLUDE_ASM("asm/nonmatchings/3430", func_800034DC);

INCLUDE_ASM("asm/nonmatchings/3430", func_800034F4);

/**
 * Sets the position of an audio point.
 * JFG: ignores a NULL point.
 * Official Name: amSndSetXYZ
 */
void audspat_point_set_position(AudioPoint *audioPoint, f32 x, f32 y, f32 z) {
    if (audioPoint != NULL) {
        audioPoint->pos.x = x;
        audioPoint->pos.y = y;
        audioPoint->pos.z = z;
    }
}

/**
 * Official Name: amSndStopXYZ
 * Stops the sound associated with the audio point.
 * JFG: ignores a NULL point; the point list has a count.
 */
void audspat_point_stop(AudioPoint *point) {
    s32 i;
    if (point != NULL) {
        for (i = 0; i < gNumAudioPoints; i++) {
            if (point == gAudioPoints[i]) {
                audspat_point_stop_by_index(i);
                return;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/3430", func_80003680);

INCLUDE_ASM("asm/nonmatchings/3430", func_800036C8);

INCLUDE_ASM("asm/nonmatchings/3430", func_800037F8);

INCLUDE_ASM("asm/nonmatchings/3430", func_8000380C);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003994);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003B14);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003B74);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003CA4);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003DB0);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003E40);

INCLUDE_ASM("asm/nonmatchings/3430", func_80004164);

INCLUDE_ASM("asm/nonmatchings/3430", func_8000474C);

INCLUDE_ASM("asm/nonmatchings/3430", func_80004768);
