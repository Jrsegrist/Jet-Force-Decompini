/*
 * Runlink module 44: audio lines, sounds that come from a polyline in the
 * level rather than from a point. Each frame the nearest point of each line
 * to the cameras sets the sound's volume and pan.
 *
 * This is Diddy Kong Racing's audio line code (DKR decomp, src/audio_spatial.c
 * and audio_spatial.h, CC0): DKR's AudioLine, audspat_reset (the audio line
 * part), audspat_update_all (the audio line part), audspat_line_add_vertex,
 * audspat_distance_to_segment and audspat_line_validate, adapted to JFG's
 * four lines, its line layout and its sound calls.
 */
#include "module.h"

#define MAX_AUDIO_LINES 4
#define MAX_LINE_VERTICES 30
#define MIN_VOLUME_THRESHOLD 10

#define AUDIO_LINE_TYPE_SOUND 0
#define AUDIO_LINE_TYPE_JINGLE 1

/* Size 0x17C (DKR's AudioLine, in a different order) */
typedef struct AudioLine {
    /* 0x000 */ f32 coords[MAX_LINE_VERTICES * 3];
    /* 0x168 */ s32 soundHandle;
    /* 0x16C */ s32 soundBite;
    /* 0x170 */ s32 range;
    /* 0x174 */ u8 type; /* 0: sound, 1: jingle */
    /* 0x175 */ u8 volume;
    /* 0x176 */ u8 maxVolume;
    /* 0x177 */ u8 pitch;
    /* 0x178 */ s8 numSegments;
    /* 0x179 */ u8 fastFalloff;
    /* 0x17A */ u8 priority;
} AudioLine;

/* A camera as the line update reads it (0x4C bytes) */
typedef struct M044Camera {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
    /* 0x18 */ u8 pad18[0x4C - 0x18];
} M044Camera;

extern void func_80000F60();                                                   /* stops the jingle */
extern void sound_stop(s32 soundHandle);                                    /* DKR sndp_stop */
extern void func_8000134C(u16 soundBite, u8 volume, u8 pan, f32 pitch, s32 flags, s32 *handlePtr);
extern s32 sound_volume_half();                                                    /* the volume parameter of a volume */
extern void func_80086388(s32 soundHandle, s32 event, s32 value);             /* DKR sndp_set_param */
extern void func_80085E94(s32 soundHandle, u8 priority);                       /* DKR sndp_set_priority */
extern f32 func_80097140(f32 x);                                               /* sqrtf */

static AudioLine gAudioLines[MAX_AUDIO_LINES];

void func_m044_0003C(void);

void func_m044_00000(void) {
    s32 i;

    for (i = 0; i < MAX_AUDIO_LINES; i++) {
        gAudioLines[i].soundHandle = 0;
    }
    func_m044_0003C();
}
#pragma weak __rl_init = func_m044_00000

/* DKR audspat_reset (the audio line part) */
void func_m044_0003C(void) {
    s32 i;
    s32 j;
    f32 *coords;

    for (i = 0; i < MAX_AUDIO_LINES; i++) {
        gAudioLines[i].soundBite = 0;
        if (gAudioLines[i].soundHandle != 0) {
            if (gAudioLines[i].type == AUDIO_LINE_TYPE_SOUND) {
                sound_stop(gAudioLines[i].soundHandle);
            } else if (gAudioLines[i].type == AUDIO_LINE_TYPE_JINGLE) {
                func_80000F60();
            }
            gAudioLines[i].soundHandle = 0;
        }
        gAudioLines[i].numSegments = -1;

        coords = gAudioLines[i].coords;
        for (j = 0; j < MAX_LINE_VERTICES; j++) {
            *coords++ = -100000.0f;
            *coords++ = -100000.0f;
            *coords++ = -100000.0f;
        }
    }
}

s32 func_m044_00A7C(u8 lineID);
static s32 audspat_distance_to_segment(f32 inX, f32 inY, f32 inZ, f32 coords[6], f32 *outX, f32 *outY, f32 *outZ);

/* DKR audspat_update_all (the audio line part) */
void func_m044_00170(M044Camera *cameras, s32 numCameras) {
    s32 i;
    s32 j;
    s32 volume;
    s32 pan;
    s32 jingleVolume;
    s32 k;
    s32 minDist;
    s32 adjustedVolume;
    s32 flags;
    f32 pitch;
    f32 outX;
    f32 outY;
    f32 outZ;
    s32 sumOfDistances;
    s32 lineDistances[MAX_LINE_VERTICES];
    s32 inverseSum;
    s32 linePans[MAX_LINE_VERTICES];
    s32 inverseDistances[MAX_LINE_VERTICES];
    f32 *coords;
    f32 temp;
    AudioLine *line;

    jingleVolume = 0;
    for (i = 0; i < MAX_AUDIO_LINES; i++) {
        line = &gAudioLines[i];
        if (line->soundBite != 0 && func_m044_00A7C(i)) {
            volume = 0;
            for (j = 0; j < numCameras; j++) {
                coords = line->coords;
                sumOfDistances = 0;
                minDist = line->range;
                for (k = 0; k < line->numSegments; k++) {
                    lineDistances[k] = audspat_distance_to_segment(cameras[j].x_position, cameras[j].y_position,
                                                                   cameras[j].z_position, coords, &outX, &outY, &outZ);
                    linePans[k] = 64;
                    if (minDist > lineDistances[k]) {
                        minDist = lineDistances[k];
                    }
                    coords += 3;
                    sumOfDistances += lineDistances[k];
                }

                if (!line->fastFalloff) {
                    adjustedVolume = (1.0f - (f32) minDist / (f32) line->range) * line->volume;
                } else {
                    temp = (f32) (line->range - minDist) / (f32) line->range;
                    adjustedVolume = temp * temp * line->volume;
                }

                if (volume <= adjustedVolume) {
                    volume = adjustedVolume;

                    if (line->numSegments == 1) {
                        pan = linePans[0];
                    } else {
                        inverseSum = 0;
                        for (k = 0; k < line->numSegments; k++) {
                            inverseDistances[k] = sumOfDistances - lineDistances[k];
                            inverseSum += inverseDistances[k];
                        }

                        pan = 0;
                        for (k = 0; k < line->numSegments; k++) {
                            pan += (f32) inverseDistances[k] / (f32) inverseSum * (f32) linePans[k];
                        }
                    }

                    if (minDist < 400) {
                        pan = (pan - 64) * (minDist / 400.0f) + 64;
                    }
                }
            }

            if (line->type == AUDIO_LINE_TYPE_SOUND) {
                if (volume < line->maxVolume) {
                    volume = line->maxVolume;
                }

                if (volume > MIN_VOLUME_THRESHOLD) {
                    pitch = line->pitch / 100.0f;
                    if ((u32) pan >> 24) {
                        pan &= 0xFF;
                        flags = 0x80;
                    } else {
                        flags = 0;
                    }
                    if (line->soundHandle == 0) {
                        func_8000134C(line->soundBite, volume, pan, pitch, flags, &line->soundHandle);
                    } else if (line->soundHandle != 0) {
                        func_80086388(line->soundHandle, 8, sound_volume_half(volume * 256));
                        func_80086388(line->soundHandle, 16, *(s32 *) &pitch);
                        if (numCameras != 1) {
                            pan = 64;
                        }
                        func_80086388(line->soundHandle, 4, pan);
                        func_80085E94(line->soundHandle, line->priority);
                    }
                } else {
                    if (line->soundHandle != 0) {
                        sound_stop(line->soundHandle);
                    }
                }
            } else if (line->type == AUDIO_LINE_TYPE_JINGLE && jingleVolume < volume) {
                jingleVolume = volume;
            }
        }
    }
}

/* DKR audspat_line_add_vertex (official name amAudioLineAddVertex) */
void func_m044_0070C(u8 type, u16 soundBite, f32 x, f32 y, f32 z, u8 maxVolume, u8 volume, u8 pitch, u8 priority,
                     u16 range, u8 fastFalloff, u8 lineID, u8 vertexIndex) {
    AudioLine *line;
    f32 *coords;

    if (lineID >= MAX_AUDIO_LINES) {
        stubbed_printf("amAudioLineAddVertex: Exceeded maximum number of lines (%d)\n", MAX_AUDIO_LINES);
    } else if (vertexIndex >= MAX_LINE_VERTICES) {
        stubbed_printf("amAudioLineAddVertex: Exceeded maximum number of line vertices (%d)\n", MAX_LINE_VERTICES);
    } else {
        line = &gAudioLines[lineID];
        coords = &line->coords[vertexIndex * 3];
        coords[0] = x;
        coords[1] = y;
        coords[2] = z;
        if (vertexIndex == 0) {
            line->soundBite = soundBite;
            line->type = type;
            line->range = range;
            line->fastFalloff = fastFalloff;
            line->volume = volume;
            line->maxVolume = maxVolume;
            line->pitch = pitch;
            line->priority = priority;
        }
        if (line->numSegments < vertexIndex) {
            line->numSegments = vertexIndex;
        }
    }
}

/* DKR audspat_distance_to_segment */
static s32 audspat_distance_to_segment(f32 inX, f32 inY, f32 inZ, f32 coords[6], f32 *outX, f32 *outY, f32 *outZ) {
    f32 dx, dy, dz;
    f32 x1, y1, z1;
    f32 x2, y2, z2;

    f32 projection;
    f32 distance;

    x1 = coords[0];
    y1 = coords[1];
    z1 = coords[2];
    x2 = coords[3];
    y2 = coords[4];
    z2 = coords[5];
    projection = 0.0f;

    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;

    if (dx == 0.0f && dy == 0.0f && dz == 0.0f) {
        projection = 0.0f;
    } else {
        projection = ((inX - x1) * dx + (inY - y1) * dy + (inZ - z1) * dz) / (dx * dx + dy * dy + dz * dz);
    }

    if (projection < 0.0f) {
        *outX = x1;
        *outY = y1;
        *outZ = z1;
        dx = x1 - inX;
        dy = y1 - inY;
        dz = z1 - inZ;
        distance = func_80097140(dx * dx + dy * dy + dz * dz);
    } else if (projection > 1.0f) {
        *outX = x2;
        *outY = y2;
        *outZ = z2;
        dx = x2 - inX;
        dy = y2 - inY;
        dz = z2 - inZ;
        distance = func_80097140(dx * dx + dy * dy + dz * dz);
    } else {
        *outX = projection * dx + x1, *outY = projection * dy + y1, *outZ = projection * dz + z1;
        distance = func_80097140((*outX - inX) * (*outX - inX) + (*outY - inY) * (*outY - inY) +
                                 (*outZ - inZ) * (*outZ - inZ));
    }

    return distance;
}

/* DKR audspat_line_validate: checks that all vertex coordinates are defined */
s32 func_m044_00A7C(u8 lineID) {
    s32 ret;
    s32 i;
    AudioLine *line;
    f32 *coords;

    ret = TRUE;
    line = &gAudioLines[lineID];
    coords = line->coords;

    if (line->numSegments <= 0) {
        stubbed_printf("Audio line definition error (less than 2 vertices on line %d)\n", line->numSegments);
        return FALSE;
    }

    for (i = 0; i < line->numSegments; i++) {
        /* DKR's bug: should be *(coords + 0), *(coords + 1), *(coords + 2) */
        if (*coords + 0 == -100000.0f || *coords + 1 == -100000.0f || *coords + 2 == -100000.0f) {
            stubbed_printf("Audio line definition error (line=%d, vertex=%d)\n", i, 1);
            ret = FALSE;
        }
        coords += 3;
    }

    return ret;
}
