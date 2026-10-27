/*
 * Track (level geometry) rendering and queries. Functions in C here are from
 * the Diddy Kong Racing decompilation (CC0 1.0), src/tracks.c; see CREDITS.md.
 */
#include "common.h"

/* DKR include/structs.h, reduced to what these functions use. */
typedef struct ObjectTransform {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
} ObjectTransform;

/* Size: 0x4C bytes */
typedef struct Camera {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x4C - 0x18];
} Camera;

typedef struct BspTreeNode {
    /* 0x00 */ s16 leftNode;  // less than split value
    /* 0x02 */ s16 rightNode; // greater or equal to split value?
    /* 0x04 */ u8 splitType;  // 0 = Camera X, 1 = Camera Y, 2 = Camera Z
    /* 0x05 */ u8 segmentIndex;
    /* 0x06 */ s16 splitValue; // Decides left or right
} BspTreeNode;

typedef struct LevelModelSegmentBoundingBox {
    /* 0x00 */ s16 x1;
    /* 0x02 */ s16 y1;
    /* 0x04 */ s16 z1;
    /* 0x06 */ s16 x2;
    /* 0x08 */ s16 y2;
    /* 0x0A */ s16 z2;
} LevelModelSegmentBoundingBox;

typedef struct LevelModel {
    /* 0x00 */ void *textures;
    /* 0x04 */ void *segments;
    /* 0x08 */ LevelModelSegmentBoundingBox *segmentsBoundingBoxes;
    /* 0x0C */ u8 *unkC;
    /* 0x10 */ u8 *segmentsBitfields;
    /* 0x14 */ BspTreeNode *segmentsBspTree;
    /* 0x18 */ s16 numberOfTextures;
    /* 0x1A */ s16 numberOfSegments;
} LevelModel;

typedef struct unk8002D30C_a0 {
    /* 0x00 */ s32 unk00;
    /* 0x04 */ struct unk8002D30C_a0 *unk04;
    /* 0x08 */ struct unk8002D30C_a0 *unk08;
} unk8002D30C_a0;

s32 block_visible(LevelModelSegmentBoundingBox *bb);
void add_segment_to_order(s32 segmentIndex, s32 *segmentsOrderIndex, u8 *segmentsOrder);

extern Camera *gSceneActiveCamera;
extern LevelModel *gCurrentLevelModel;
extern s32 gSceneRenderSkyDome;
extern s32 gSceneStartSegment;

s32 runlink_module_loaded(s32 module); // JFG: whether a runlink module is loaded
void func_80054530();          // a module function (patched in by the runtime linker)

/* JFG: run the track effects of the modules 1, 8 and 10 that are loaded. */
void func_80012150(s32 arg0) {
    if (runlink_module_loaded(1)) {
        func_80054530(arg0);
    }
    if (runlink_module_loaded(8)) {
        func_80054530(arg0);
    }
    if (runlink_module_loaded(0xA)) {
        func_80054530(arg0);
    }
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_800121C0);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800127A4);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800129AC);

/* DKR include/structs.h Object, reduced (JFG offsets) */
typedef struct Object_Racer {
    /* 0x000 */ s8 playerIndex;
    /* 0x001 */ u8 pad1[0x19C - 1];
    /* 0x19C */ s8 unk19C;
    /* 0x19D */ u8 pad19D[0x568 - 0x19D];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x5C0 - 0x569];
    /* 0x5C0 */ s32 unk5C0;
} Object_Racer;

typedef struct Object {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x3C - 0x18];
    /* 0x3C */ void *level_entry;
    /* 0x40 */ u8 pad40[0x4A - 0x40];
    /* 0x4A */ s16 objectID;
    /* 0x4C */ u8 pad4C[0x68 - 0x4C];
    /* 0x68 */ Object_Racer *racer;
    /* 0x6C */ u8 pad6C[0x88 - 0x6C];
    /* 0x88 */ f32 radius; // DKR: properties.distance.radius
} Object;

/* JFG: the object ID is an s16 and the size byte follows it. */
typedef struct LevelObjectEntryCommon {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
} LevelObjectEntryCommon;

#define OBJECT_SPAWN_UNK02 2

Object *spawn_object(LevelObjectEntryCommon *entry, s32 arg1);

extern Object *gSkydomeSegment;
extern s16 D_800F2FAC; // JFG: the skydome's object ID

/**
 * Spawns and initialises the skydome object seen ingame.
 * Skipped if the object ID doesn't exist.
 * JFG: no anti-tamper check; remembers the object ID.
 * Official name: initSky
 */
void skydome_spawn(s32 objectID) {
    LevelObjectEntryCommon spawnObject;

    if (objectID == -1) {
        gSkydomeSegment = NULL;
        D_800F2FAC = objectID;
        return;
    }
    spawnObject.x = 0;
    spawnObject.y = 0;
    spawnObject.z = 0;
    spawnObject.size = 10;
    spawnObject.objectID = objectID;
    gSkydomeSegment = spawn_object(&spawnObject, OBJECT_SPAWN_UNK02);
    D_800F2FAC = objectID;
    if (gSkydomeSegment != NULL) {
        gSkydomeSegment->level_entry = NULL;
        gSkydomeSegment->objectID = -1;
    }
}

/**
 * Sets the status to render the sky.
 * If set to false, will skip the background and skydome rendering.
 * Official name: trackSkySet
 */
void set_skydome_visbility(s32 renderSky) {
    gSceneRenderSkyDome = renderSky;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80012BAC);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013454);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800136B8);

/* Returns the skydome object (0x80013810). */
Object *get_skydome_object(void) {
    return gSkydomeSegment;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013820);

extern s32 D_800A0D8C;
extern s32 D_800F3370[16];

/* JFG: queue a texture scroll (up to 16 a frame). */
void func_80013924(s32 arg0, s32 arg1, s32 arg2) {
    if (D_800A0D8C < 16) {
        D_800F3370[D_800A0D8C] = (arg0 << 24) | ((arg1 & 0xFFF) << 12) | (arg2 & 0xFFF);
        D_800A0D8C++;
    }
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013970);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013B74);

/* JFG: an entry per segment of D_800A0D80 */
typedef struct SegmentAlloc {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ void *data;
} SegmentAlloc;

/* JFG: a track light (Size: 0x80 bytes) */
typedef struct TrackLight {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 outerRadius; // 0 when the slot is free
    /* 0x10 */ f32 innerRadius;
    /* 0x14 */ f32 outerRadiusSquare;
    /* 0x18 */ f32 innerRadiusSquare;
    /* 0x1C */ f32 falloff;
    /* 0x20 */ u8 ramp[0x60]; // filled by track_light_set_colour
} TrackLight;

extern SegmentAlloc *D_800A0D80;
extern TrackLight *gTrackLights; // JFG: gMaxTrackLights track lights
extern s32 gNumTrackLights;
extern s32 gMaxTrackLights;
void mempool_free(void *data);

/* JFG: free the per-segment buffers and the track lights (0x80013D3C). */
void track_segment_data_free(void) {
    s32 i;

    if (D_800A0D80 != NULL) {
        i = gCurrentLevelModel->numberOfSegments;
        while (i--) {
            if (D_800A0D80[i].data != NULL) {
                mempool_free(D_800A0D80[i].data);
            }
        }
        mempool_free(D_800A0D80);
        D_800A0D80 = NULL;
    }
    if (gTrackLights != NULL) {
        mempool_free(gTrackLights);
        gTrackLights = NULL;
    }
    gNumTrackLights = 0;
    gMaxTrackLights = 0;
}

extern f32 D_800AB648; // a .rodata constant, not split out yet
void track_light_set_colour(u8 *light, s32 red, s32 green, s32 blue, s32 brightness);

#ifdef NON_MATCHING
/**
 * JFG: add a track light in the first free slot.
 * NON_MATCHING: 11 words: the stores in the found-slot block are scheduled differently and the
 * light's spill slot is off by one word.
 */
TrackLight *func_80013E0C(f32 x, f32 y, f32 z, f32 outerRadius, f32 innerRadius, s32 red, s32 green, s32 blue) {
    s32 i;
    TrackLight *light;

    if (outerRadius <= 0.0f) {
        return NULL;
    }
    light = gTrackLights;
    i = gMaxTrackLights;
    while (i--) {
        if (light->outerRadius == 0.0f) {
            light->x = x;
            light->y = y;
            light->outerRadius = outerRadius;
            light->outerRadiusSquare = outerRadius * outerRadius;
            light->innerRadius = innerRadius;
            light->innerRadiusSquare = innerRadius * innerRadius;
            light->z = z;
            light->falloff = D_800AB648 / (outerRadius - innerRadius);
            track_light_set_colour((u8 *) light, red, green, blue, 255);
            gNumTrackLights++;
            return light;
        }
        light++;
    }
    return NULL;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80013E0C);
#endif

extern s32 gNumTrackLights;

/* JFG: switch a track light off (its intensity at 0xC), keeping a count of lit ones (0x80013F08). */
void track_light_off(f32 *light) {
    if (light != NULL) {
        if (light[3] != 0.0f) {
            light[3] = 0.0f;
            gNumTrackLights--;
        }
    }
}

/* JFG: fill a track light's ramp of 32 colours (from 0x20) up to the colour, scaled by brightness (0x80013F48). */
void track_light_set_colour(u8 *light, s32 red, s32 green, s32 blue, s32 brightness) {
    s32 i;
    u8 *ramp;
    s32 stepRed, stepGreen, stepBlue;

    if (light != NULL) {
        if (brightness < 255) {
            red = (red * brightness) >> 8;
            green = (green * brightness) >> 8;
            blue = (blue * brightness) >> 8;
        }
        ramp = light + 0x20;
        stepRed = red;
        stepGreen = green;
        stepBlue = blue;
        i = 31;
        do {
            ramp[0] = red >> 5;
            ramp[1] = green >> 5;
            ramp[2] = blue >> 5;
            red += stepRed;
            green += stepGreen;
            blue += stepBlue;
            ramp += 3;
        } while (i--);
    }
}

/* Moves a track light (0x80013FE0). */
void track_light_set_position(f32 *light, f32 x, f32 y, f32 z) {
    if (light != NULL) {
        light[0] = x;
        light[1] = y;
        light[2] = z;
    }
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001400C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001417C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001436C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001448C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80014EBC);

/**
 * Parse through applicable segments in the level.
 * Uses function recursion to ensure adjacent segments remain next to each other in the list.
 */
void traverse_segments_bsp_tree(s32 nodeIndex, s32 segmentIndex, s32 segmentIndex2, u8 *segmentsOrder,
                                s32 *segmentsOrderIndex) {
    BspTreeNode *curNode;
    s32 camValue;

    curNode = &gCurrentLevelModel->segmentsBspTree[nodeIndex];
    if (curNode->splitType == 0) {
        camValue = gSceneActiveCamera->trans.x_position; // Camera X
    } else if (curNode->splitType == 1) {
        camValue = gSceneActiveCamera->trans.y_position; // Camera Y
    } else {
        camValue = gSceneActiveCamera->trans.z_position; // Camera Z
    }

    if (camValue < curNode->splitValue) {
        if (curNode->leftNode != -1) {
            traverse_segments_bsp_tree(curNode->leftNode, segmentIndex, curNode->segmentIndex - 1, segmentsOrder,
                                       segmentsOrderIndex);
        } else {
            add_segment_to_order(segmentIndex, segmentsOrderIndex, segmentsOrder);
        }

        if (curNode->rightNode != -1) {
            traverse_segments_bsp_tree(curNode->rightNode, curNode->segmentIndex, segmentIndex2, segmentsOrder,
                                       segmentsOrderIndex);
        } else {
            add_segment_to_order(segmentIndex2, segmentsOrderIndex, segmentsOrder);
        }
    } else {
        if (curNode->rightNode != -1) {
            traverse_segments_bsp_tree(curNode->rightNode, curNode->segmentIndex, segmentIndex2, segmentsOrder,
                                       segmentsOrderIndex);
        } else {
            add_segment_to_order(segmentIndex2, segmentsOrderIndex, segmentsOrder);
        }

        if (curNode->leftNode != -1) {
            traverse_segments_bsp_tree(curNode->leftNode, segmentIndex, curNode->segmentIndex - 1, segmentsOrder,
                                       segmentsOrderIndex);
        } else {
            add_segment_to_order(segmentIndex, segmentsOrderIndex, segmentsOrder);
        }
    }
}

/**
 * Add this segment index to the specified segment ordering table if the segment in question is in view of the camera.
 */
void add_segment_to_order(s32 segmentIndex, s32 *segmentsOrderIndex, u8 *segmentsOrder) {
    u32 temp;
    if (segmentIndex < gCurrentLevelModel->numberOfSegments) {
        if (gSceneStartSegment != -1) {
            temp = gCurrentLevelModel->segmentsBitfields[gSceneStartSegment + (segmentIndex >> 3)];
            temp >>= segmentIndex & 7;
            temp &= 0xFF;
        } else {
            temp = 1;
        }
        if (temp & 1 && block_visible(&gCurrentLevelModel->segmentsBoundingBoxes[segmentIndex])) {
            segmentsOrder[(*segmentsOrderIndex)++] = segmentIndex;
        }
    }
}

/**
 * Iterates through every existing segment to see which one the position is inside.
 * Uses mainly a two dimensional axis check here, instead of the function above.
 * Returns the segment currently inside.
 * JFG: inclusive bounds, a smaller starting distance.
 * Official Name: trackGetBlock
 * NON_MATCHING: same instructions, other registers (target: i in a3, the count in t0,
 * minVal in a1, x in v0).
 */
#ifdef NON_MATCHING
s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos) {
    LevelModelSegmentBoundingBox *bb;
    s32 i;
    s32 minVal;
    s32 result;
    s32 heightDiff;
    s32 x1, x2, z1, z2;

    result = -1;
    if (gCurrentLevelModel != NULL) {
        minVal = 0x7FFF;
        for (i = 0, bb = gCurrentLevelModel->segmentsBoundingBoxes; i < gCurrentLevelModel->numberOfSegments;
             i++, bb++) {
            x1 = bb->x1;
            z1 = bb->z1;
            x2 = bb->x2;
            z2 = bb->z2;
            if ((s32) xPos <= x2 && (s32) xPos >= x1 && (s32) zPos <= z2 && (s32) zPos >= z1) {
                heightDiff = ((bb->y1 + bb->y2) >> 1) - (s32) yPos;
                if (heightDiff < 0) {
                    heightDiff = -heightDiff;
                }
                if (heightDiff < minVal) {
                    minVal = heightDiff;
                    result = i;
                }
            }
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80015510);
#endif

/* Writes the segments whose bounding box holds the point on the XZ plane; returns the count (0x8001563C). */
/**
 * Write the segments whose bounding box (grown by 4) holds the point on the XZ plane.
 * JFG: the list is of shorts.
 * DKR: get_inside_segment_count_xz
 */
s32 get_inside_segment_count_xz(s32 x, s32 z, s16 *arg2) {
    s32 i;
    s32 cnt = 0;
    LevelModelSegmentBoundingBox *bb;
    for (i = 0; i < gCurrentLevelModel->numberOfSegments; i++) {
        bb = gCurrentLevelModel->segmentsBoundingBoxes + i;
        if (x < bb->x2 + 4 && bb->x1 - 4 < x && z < bb->z2 + 4 && bb->z1 - 4 < z) {
            *arg2 = i;
            cnt++;
            arg2++;
        }
    }
    return cnt;
}

/**
 * Carbon copy of the above function, but takes into account the Y axis, too.
 * Official name: trackGetCubeBlockList
 */
s32 get_inside_segment_count_xyz(s32 *arg0, s16 xPos1, s16 yPos1, s16 zPos1, s16 xPos2, s16 yPos2, s16 zPos2) {
    s32 cnt;
    s32 i;
    LevelModelSegmentBoundingBox *bb;

    xPos1 -= 4;
    yPos1 -= 4;
    zPos1 -= 4;
    xPos2 += 4;
    yPos2 += 4;
    zPos2 += 4;

    i = 0;
    cnt = 0;

    while (i < gCurrentLevelModel->numberOfSegments) {
        bb = &gCurrentLevelModel->segmentsBoundingBoxes[i];
        if ((bb->x2 >= xPos1) && (xPos2 >= bb->x1) && (bb->z2 >= zPos1) && (zPos2 >= bb->z1) && (bb->y2 >= yPos1) &&
            (yPos2 >= bb->y1)) {
            cnt++;
            *arg0++ = i;
        }
        i++;
    }
    return cnt;
}

/**
 * Returns the bounding box data of this block.
 * Official name: trackBlockDim
 */
LevelModelSegmentBoundingBox *block_boundbox(s32 segmentID) {
    if (segmentID < 0 || gCurrentLevelModel->numberOfSegments < segmentID) {
        return NULL;
    }

    return &gCurrentLevelModel->segmentsBoundingBoxes[segmentID];
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_800158BC);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80015B3C);

extern u8 D_800A4FC4;
s32 cam_get_active_camera_id(); // the current camera (called with two arguments it ignores)

/* JFG: the opacity to draw a racer with (255 = opaque) (0x80015E3C). */
s32 player_get_draw_alpha(Object *obj) {
    s32 alpha;
    Object_Racer *racer;

    racer = obj->racer;
    alpha = 255;
    if (racer->unk5C0 == 0) {
        if (racer->unk568 == 10) {
            alpha = 255 - racer->unk19C * 2;
        } else if (cam_get_active_camera_id(obj, racer) == racer->playerIndex) {
            alpha = 255 - racer->unk19C * 4;
            if (D_800A4FC4) {
                alpha -= racer->unk19C;
            }
            if (alpha < 0) {
                alpha = 0;
            }
        }
    }
    return alpha;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80015ED8);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80016208);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800163FC);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80017094);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80017794);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80018024);

f32 sqrtf(f32 x);

#ifdef NON_MATCHING
/**
 * JFG: where a ray (origin, unit direction) meets a sphere; FALSE if it misses.
 * NON_MATCHING: frame 0x38 (target 0x48, so more locals), and b / b*b swap FP registers.
 */
s32 func_800183D4(f32 *origin, f32 *dir, f32 *centre, f32 radius, f32 *tNear, f32 *tFar) {
    f32 dx, dy, dz;
    f32 b;
    f32 b2;
    f32 c;
    f32 d;
    s32 hit;

    dx = origin[0] - centre[0];
    dy = origin[1] - centre[1];
    dz = origin[2] - centre[2];
    hit = FALSE;
    b = dx * dir[0] + dy * dir[1] + dz * dir[2];
    b2 = b * b;
    c = (dx * dx + dy * dy + dz * dz) - radius * radius;
    if (c <= b2) {
        hit = TRUE;
    }
    if (hit) {
        d = sqrtf(b2 - c);
        *tNear = -b - d;
        *tFar = -b + d;
    }
    return hit;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_800183D4);
#endif

INCLUDE_ASM("asm/nonmatchings/12D50", func_800184B4);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80018808);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800191D8);

/* JFG: clip a line's parameter range [*tEnter, *tLeave] against one plane; FALSE if nothing is left (0x80019518). */
s32 line_clip_plane(f32 denom, f32 num, f32 *tEnter, f32 *tLeave) {
    f32 t;

    if (denom > 0.0f) {
        t = num / denom;
        if (t > *tLeave) {
            return FALSE;
        }
        if (t > *tEnter) {
            *tEnter = t;
        }
    } else if (denom < 0.0f) {
        t = num / denom;
        if (t < *tEnter) {
            return FALSE;
        }
        if (t < *tLeave) {
            *tLeave = t;
        }
    } else if (num > 0.0f) {
        return FALSE;
    }
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_800195E8);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80019888);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80019E0C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001A218);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001AB84);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001ACE0);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001B078);

/**
 * Returns the current loaded level geometry
 * Official Name: trackGetTrack
 */
LevelModel *get_current_level_model(void) {
    return gCurrentLevelModel;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001B660);

void trackMakeAbsolute(unk8002D30C_a0 *arg0, s32 arg1) {
    while (1) {
        if (!arg0) {
            return;
        }
        if (arg0->unk04) {
            arg0->unk04 = (unk8002D30C_a0 *) ((s32) arg0->unk04 + arg1);
        }
        if (arg0->unk08) {
            arg0->unk08 = (unk8002D30C_a0 *) ((s32) arg0->unk08 + arg1);
        }

        trackMakeAbsolute(arg0->unk04, arg1);
        arg0 = arg0->unk08;
    }
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001B8E0);

/* DKR src/tracks.h (JFG: 8 more bytes after the current fog, size 0x40) */
typedef struct EnvironmentFog {
    /* 0x00 */ s32 r, g, b;
    /* 0x0C */ s32 near, far;
} EnvironmentFog;

typedef struct EnvironmentFogCompact {
    /* 0x00 */ u8 r, g, b;
    /* 0x03 */ s8 unk3; // JFG
    /* 0x04 */ s16 near, far;
} EnvironmentFogCompact;

typedef struct FogData {
    /* 0x00 */ EnvironmentFog fog;
    /* 0x14 */ s32 unk14; // JFG: near << 16 again
    /* 0x18 */ s32 unk18; // JFG
    /* 0x1C */ EnvironmentFog addFog;
    /* 0x30 */ EnvironmentFogCompact intendedFog;
    /* 0x38 */ s32 switchTimer;
    /* 0x3C */ Object *fogChanger;
} FogData;

extern FogData gFogData[4];

/**
 * Instantly update current fog properties.
 * JFG: two more arguments, kept in fields DKR doesn't have.
 * Official Name: trackSetFog
 */
void set_fog(s32 fogIdx, s16 near, s16 far, s16 unk, u8 red, u8 green, u8 blue, s8 unk2) {
    s32 tempNear;
    FogData *fogData;

    fogData = &gFogData[fogIdx];

    if (far < near) {
        tempNear = near;
        near = far;
        far = tempNear;
    }

    if (far > 1023) {
        far = 1023;
    }
    if (near >= far - 5) {
        near = far - 5;
    }

    fogData->addFog.near = 0;
    fogData->addFog.far = 0;
    fogData->addFog.r = 0;
    fogData->addFog.g = 0;
    fogData->addFog.b = 0;
    fogData->fog.r = red << 16;
    fogData->fog.g = green << 16;
    fogData->fog.b = blue << 16;
    fogData->fog.near = near << 16;
    fogData->fog.far = far << 16;
    fogData->unk14 = near << 16;
    fogData->unk18 = unk << 16;
    fogData->intendedFog.unk3 = unk2;
    fogData->intendedFog.r = red;
    fogData->intendedFog.g = green;
    fogData->intendedFog.b = blue;
    fogData->intendedFog.near = near;
    fogData->intendedFog.far = far;
    fogData->switchTimer = 0;
    fogData->fogChanger = NULL;
}

s32 runlink_module_loaded(s32 module);
/* Calls into runtime-linked modules: the jal goes to the dangling-jump trap until
   the module is loaded and the linker patches it. Different modules' functions,
   so no prototype. */
void func_80054530();

/**
 * Writes the current fog settings to the arguments.
 * Pre-shifts the data, so the raw values are correct.
 * JFG: two more outputs; two modules can supply the fog instead, and a third
 * one can change it afterwards.
 * Official Name: trackGetFog
 */
void get_fog_settings(s32 playerID, s16 *near, s16 *far, s16 *unk, u8 *r, u8 *g, u8 *b, u8 *unk2) {
    if (runlink_module_loaded(0x27)) {
        func_80054530(near, far, r, g, b);
        *unk = 0;
        *unk2 = 0;
    } else if (runlink_module_loaded(0x17)) {
        func_80054530(near, far, r, g, b);
        *unk = 0;
        *unk2 = 0;
    } else {
        *near = gFogData[playerID].fog.near >> 16;
        *far = gFogData[playerID].fog.far >> 16;
        *unk = gFogData[playerID].unk18 >> 16;
        *r = gFogData[playerID].fog.r >> 16;
        *g = gFogData[playerID].fog.g >> 16;
        *b = gFogData[playerID].fog.b >> 16;
        *unk2 = gFogData[playerID].intendedFog.unk3 & 0x7F;
    }
    if (runlink_module_loaded(0x1D)) {
        func_80054530(r, g, b, near, far, unk2);
    }
}

/**
 * Sets the fog of the player ID to the default values.
 * Current fog attributes are rightshifted 16 bytes.
 * Official Name: trackSetFogOff
 */
void reset_fog(s32 playerID) {
    gFogData[playerID].addFog.near = 0;
    gFogData[playerID].addFog.far = 0;
    gFogData[playerID].addFog.r = 0;
    gFogData[playerID].addFog.g = 0;
    gFogData[playerID].addFog.b = 0;
    gFogData[playerID].fog.near = 1018 << 16;
    gFogData[playerID].fog.far = 1023 << 16;
    gFogData[playerID].intendedFog.r = gFogData[playerID].fog.r >> 16;
    gFogData[playerID].intendedFog.g = gFogData[playerID].fog.g >> 16;
    gFogData[playerID].intendedFog.b = gFogData[playerID].fog.b >> 16;
    gFogData[playerID].intendedFog.near = 1018;
    gFogData[playerID].intendedFog.far = 1023;
    gFogData[playerID].switchTimer = 0;
    gFogData[playerID].fogChanger = NULL;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001BE04);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001BF9C);

#define PLAYER_COMPUTER -1
#define PLAYER_ONE 0
#define PLAYER_FOUR 3

/* JFG's fog changer level entry */
typedef struct LevelObjectEntry_FogChanger {
    /* 0x00 */ u8 pad0[0xB];
    /* 0x0B */ u8 r;
    /* 0x0C */ u8 g;
    /* 0x0D */ u8 b;
    /* 0x0E */ s16 near;
    /* 0x10 */ s16 far;
    /* 0x12 */ s16 switchTimer;
} LevelObjectEntry_FogChanger;

Object **get_racer_objects(s32 *numRacers);

/**
 * Sets the active viewport's fog target when passed through.
 * Used in courses to make less, or more dense.
 * JFG: always uses the player list (the camera list path is left with a NULL camera).
 * Official Name: trackChangeFog
 */
void obj_loop_fogchanger(Object *obj) {
    s32 nearTemp;
    s32 fogNear;
    s32 views;
    s32 playerIndex;
    s32 index;
    UNUSED s32 pad;
    s32 fogFar;
    s32 i;
    s32 fogR;
    s32 fogG;
    s32 fogB;
    f32 x;
    f32 z;
    s32 switchTimer;
    LevelObjectEntry_FogChanger *fogChanger;
    Object **racers;
    Object_Racer *racer;
    UNUSED s32 pad2;
    FogData *fog;
    Camera *camera;

    fogChanger = (LevelObjectEntry_FogChanger *) obj->level_entry;
    camera = NULL;
    racers = get_racer_objects(&views);

    for (i = 0; i < views; i++) {
        index = PLAYER_COMPUTER;
        if (racers != NULL) {
            racer = racers[i]->racer;
            playerIndex = racer->playerIndex;
            if (playerIndex >= PLAYER_ONE && playerIndex <= PLAYER_FOUR && obj != gFogData[playerIndex].fogChanger) {
                index = playerIndex;
                x = racers[i]->trans.x_position;
                z = racers[i]->trans.z_position;
            }
        } else if (i <= PLAYER_FOUR && obj != gFogData[i].fogChanger) {
            index = i;
            x = camera[i].trans.x_position;
            z = camera[i].trans.z_position;
        }
        if (index != PLAYER_COMPUTER) {
            x -= obj->trans.x_position;
            z -= obj->trans.z_position;
            if (1) {} // Fakematch (DKR)
            if ((x * x) + (z * z) < obj->radius) {
                fogNear = fogChanger->near;
                fogFar = fogChanger->far;
                fogR = fogChanger->r;
                fogG = fogChanger->g;
                fogB = fogChanger->b;
                switchTimer = fogChanger->switchTimer;
                // Swap near and far if they're the wrong way around.
                if (fogFar < fogNear) {
                    nearTemp = fogNear;
                    fogNear = fogFar;
                    fogFar = nearTemp;
                }
                if (fogFar > 1023) {
                    fogFar = 1023;
                }
                if (fogNear >= fogFar - 5) {
                    fogNear = fogFar - 5;
                }
                fog = &gFogData[index];
                fog->intendedFog.r = fogR;
                fog->intendedFog.g = fogG;
                fog->intendedFog.b = fogB;
                fog->intendedFog.near = fogNear;
                fog->intendedFog.far = fogFar;
                fog->addFog.r = ((fogR << 16) - fog->fog.r) / switchTimer;
                fog->addFog.g = ((fogG << 16) - fog->fog.g) / switchTimer;
                fog->addFog.b = ((fogB << 16) - fog->fog.b) / switchTimer;
                fog->addFog.near = ((fogNear << 16) - fog->fog.near) / switchTimer;
                fog->addFog.far = ((fogFar << 16) - fog->fog.far) / switchTimer;
                fog->switchTimer = switchTimer;
                fog->fogChanger = obj;
            }
        }
    }
}

/**
 * Set the fog properties from the current values to the target, over a time in seconds.
 * JFG: the time is converted with the TV's refresh rate; a time of zero sets the fog at once.
 * Official Name: trackFadeFog
 */
void slowly_change_fog(s32 fogIdx, s32 red, s32 green, s32 blue, s32 near, s32 far, f32 seconds) {
    s32 temp;
    FogData *fogData;
    s32 switchTimer;

    fogData = &gFogData[fogIdx];
    if (osTvType == OS_TV_PAL) {
        switchTimer = seconds * 50.0f;
    } else {
        switchTimer = seconds * 60.0f;
    }

    if (far < near) {
        temp = near;
        near = far;
        far = temp;
    }

    if (far > 1023) {
        far = 1023;
    }
    if (near >= far - 5) {
        near = far - 5;
    }

    fogData->intendedFog.r = red;
    fogData->intendedFog.g = green;
    fogData->intendedFog.b = blue;
    fogData->intendedFog.near = near;
    fogData->intendedFog.far = far;
    if (switchTimer > 0) {
        fogData->switchTimer = switchTimer;
        fogData->addFog.r = ((red << 16) - fogData->fog.r) / switchTimer;
        fogData->addFog.g = ((green << 16) - fogData->fog.g) / switchTimer;
        fogData->addFog.b = ((blue << 16) - fogData->fog.b) / switchTimer;
        fogData->addFog.near = ((near << 16) - fogData->fog.near) / switchTimer;
        fogData->addFog.far = ((far << 16) - fogData->fog.far) / switchTimer;
    } else {
        fogData->switchTimer = 0;
        fogData->fog.r = red << 16;
        fogData->fog.g = green << 16;
        fogData->fog.b = blue << 16;
        fogData->fog.near = near << 16;
        fogData->fog.far = far << 16;
    }
    fogData->fogChanger = NULL;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001C650);

/* JFG: the index of the highest set bit (-1 for zero) (0x8001C758). */
s32 highest_set_bit(u32 value) {
    s32 bit = -1;
    while (value != 0) {
        bit++;
        value >>= 1;
    }
    return bit;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001C778);
