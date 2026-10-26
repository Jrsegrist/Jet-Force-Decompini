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

typedef struct Camera {
    /* 0x00 */ ObjectTransform trans;
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

INCLUDE_ASM("asm/nonmatchings/12D50", func_80012150);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800121C0);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800127A4);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800129AC);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80012B04);

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

extern s32 D_800F2F88;

s32 func_80013810(void) {
    return D_800F2F88;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013820);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013924);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013970);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013B74);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013D3C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013E0C);

extern s32 D_800A0D74;

/* JFG: switch a track light off (its intensity at 0xC), keeping a count of lit ones. */
void func_80013F08(f32 *light) {
    if (light != NULL) {
        if (light[3] != 0.0f) {
            light[3] = 0.0f;
            D_800A0D74--;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80013F48);

void func_80013FE0(f32 *light, f32 x, f32 y, f32 z) {
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

INCLUDE_ASM("asm/nonmatchings/12D50", func_80015510);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001563C);

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

INCLUDE_ASM("asm/nonmatchings/12D50", func_80015E3C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80015ED8);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80016208);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800163FC);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80017094);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80017794);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80018024);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800183D4);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800184B4);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80018808);

INCLUDE_ASM("asm/nonmatchings/12D50", func_800191D8);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80019518);

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

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001BB30);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001BC2C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001BD90);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001BE04);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001BF9C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001C0D8);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001C3D0);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001C650);

/* JFG: the index of the highest set bit (-1 for zero). */
s32 func_8001C758(u32 value) {
    s32 bit = -1;
    while (value != 0) {
        bit++;
        value >>= 1;
    }
    return bit;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001C778);
