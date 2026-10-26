#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern u8 gJinglesOff;

/* DKR src/audio_spatial.h (reduced) */
typedef struct AudioPoint {
    /* 0x00 */ struct {
        f32 x, y, z;
    } pos;
    /* 0x0C */ u16 soundBite;
    /* 0x0E */ u8 volume;
    /* 0x0F */ u8 pitch;
    /* 0x10 */ u8 minVolume;
    /* 0x11 */ u8 flags;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ s32 range;
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ struct AudioPoint **userHandlePtr;
    /* 0x20 */ u8 fastFalloff;
    /* 0x21 */ u8 priority;
    /* 0x22 */ u8 triggeredOnce;
    /* 0x23 */ u8 unk23; // JFG
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

extern MtxF D_800F2A08; // JFG: the listener's matrix
s32 func_80048548(void); // the number of cameras
void func_80048B60(MtxF *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz);
s32 func_80049C84(f32 x, f32 z); // arctangent as a 16-bit angle

/**
 * Computes the pan value based on the listener's position and orientation relative to the sound source.
 * JFG: centred with more than one camera; takes a world position; a sound behind the
 * listener has 0xFF000000 set.
 * Official Name: amCalcSfxStereo
 */
s32 audspat_calculate_spatial_pan(f32 x, f32 y, f32 z) {
    s16 angle;
    s32 pan;
    f32 ox, oy, oz;

    if (func_80048548() >= 2) {
        return 64;
    }
    func_80048B60(&D_800F2A08, x, y, z, &ox, &oy, &oz);
    angle = func_80049C84(ox, oz);
    if (angle >= -0x2000 && angle <= 0x2000) {
        pan = (angle * 63) / 8192 + 64;
    } else if (angle < -0x2000 && angle >= -0x5FFF) {
        pan = ((-angle * 63 - 0x17A000) / 16384 + 64) | 0xFF000000;
    } else if (angle > 0x2000 && angle < 0x6000) {
        pan = ((-angle * 63 + 0x17A000) / 16384 + 64) | 0xFF000000;
    } else {
        pan = 0xFF000040;
    }
    return pan;
}

/* DKR src/audio.h (Size: 0x0A bytes) */
typedef struct SoundData {
    u16 soundBite;
    u8 volume;
    u8 minVolume;
    u8 pitch;
    u8 unk5;
    u16 range;
    u8 priority;
    u8 unk9;
} SoundData;

extern SoundData *gSpatialSoundTable;

/* JFG: there is an extra argument before the handle. */
void audspat_point_create(u16 soundBite, f32 x, f32 y, f32 z, u8 flags, u8 minVolume, u8 volume, u16 range,
                          u8 fastFalloff, u8 pitch, u8 priority, s32 unk, AudioPoint **handlePtr);

/**
 * Play Sound at position
 * JFG: does nothing if the handle already holds a point.
 * Official Name: amSndPlayXYZ
 */
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, AudioPoint **handlePtr) {
    if (handlePtr == NULL || *handlePtr == NULL) {
        audspat_point_create(gSpatialSoundTable[soundId].soundBite, x, y, z, flags,
                             gSpatialSoundTable[soundId].minVolume, gSpatialSoundTable[soundId].volume,
                             gSpatialSoundTable[soundId].range, FALSE, gSpatialSoundTable[soundId].pitch,
                             gSpatialSoundTable[soundId].priority, gSpatialSoundTable[soundId].unk5, handlePtr);
    }
}

void func_800034C4(AudioPoint *point, u8 volume) {
    if (point != NULL) {
        point->volume = volume;
    }
}

void func_800034DC(AudioPoint *point, u8 pitch) {
    if (point != NULL) {
        point->pitch = pitch;
    }
}

/**
 * Uncertain of the exact purpose of this function, but it directly takes a sound ID
 * and bypasses the sound table.
 * Official Name: amSndPlayDirectXYZ
 */
void audspat_play_sound_direct(u16 soundBite, f32 x, f32 y, f32 z, u8 flags, u8 volume, f32 pitch, s32 unused,
                               AudioPoint **handlePtr) {
    audspat_point_create(soundBite, x, y, z, flags, /* minVolume */ 100, volume, /* range */ 15000, FALSE, pitch,
                         /* priority */ 63, 0, handlePtr);
}

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

/* JFG: forget the user's handle of an audio point (the point keeps playing). */
void func_80003680(AudioPoint *point) {
    s32 i;

    if (point != NULL) {
        for (i = 0; i < gNumAudioPoints; i++) {
            if (point == gAudioPoints[i]) {
                gAudioPoints[i]->userHandlePtr = NULL;
                return;
            }
        }
    }
}

#define MAX_AUDIO_POINTS 40

extern AudioPoint **gFreeAudioPoints;
extern u8 gLastFreePointIndex;
extern s32 D_800A0814; // JFG: the most audio points in use at once

/**
 * Creates a point sound source and sets its parameters
 * JFG: the pitch is an integer and there is one more byte; keeps a high-water mark.
 * Official Name: amCreateAudioPoint
 */
void audspat_point_create(u16 soundBite, f32 x, f32 y, f32 z, u8 flags, u8 minVolume, u8 volume, u16 range,
                          u8 fastFalloff, u8 pitch, u8 priority, s32 unk, AudioPoint **handlePtr) {
    AudioPoint *audioPoint;

    if (D_800A0814 < gNumAudioPoints) {
        D_800A0814 = gNumAudioPoints;
    }
    if (gNumAudioPoints == MAX_AUDIO_POINTS) {
        if (handlePtr != NULL) {
            *handlePtr = NULL;
        }
        return;
    }
    audioPoint = gFreeAudioPoints[gLastFreePointIndex--];
    audioPoint->pos.x = x;
    audioPoint->pos.y = y;
    audioPoint->pos.z = z;
    audioPoint->soundBite = soundBite;
    audioPoint->flags = flags;
    audioPoint->minVolume = minVolume;
    audioPoint->volume = volume;
    audioPoint->pitch = pitch;
    audioPoint->range = range;
    audioPoint->fastFalloff = fastFalloff;
    audioPoint->priority = priority;
    audioPoint->triggeredOnce = 0;
    audioPoint->unk23 = unk;
    audioPoint->userHandlePtr = handlePtr;
    gAudioPoints[gNumAudioPoints++] = audioPoint;
    if (handlePtr != NULL) {
        *handlePtr = audioPoint;
    }
}

void func_800037F8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
}

INCLUDE_ASM("asm/nonmatchings/3430", func_8000380C);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003994);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003B14);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003B74);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003CA4);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003DB0);

INCLUDE_ASM("asm/nonmatchings/3430", func_80003E40);

INCLUDE_ASM("asm/nonmatchings/3430", func_80004164);

extern void *D_800F2B10;

/* JFG: push a node onto a free list. */
void func_8000474C(void **node) {
    *node = D_800F2B10;
    D_800F2B10 = node;
}

extern f32 D_800AA1C8; // a constant in .rodata (not split out yet)

/* JFG: D_800AA1C8 raised to the power of depth (by squaring). */
f32 func_80004768(u8 depth) {
    f32 base = D_800AA1C8;
    f32 result = 1.0f;

    while (depth) {
        if (depth & 1) {
            result *= base;
        }
        base *= base;
        depth >>= 1;
    }
    return result;
}
