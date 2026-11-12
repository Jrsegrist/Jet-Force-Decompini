#include "common.h"

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001CE10);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D344);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D464);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D500);

extern s32 D_800A0E80[4];
extern s32 D_800A0E90[4];
extern s32 D_800A0EA0[4];
extern s32 D_800A0EC8;
extern s32 D_800F5B00;
extern s32 D_800F5B04;
extern s32 D_800F5B08;
extern s32 D_800F5B0C;
extern s32 D_800F5B10;
void mempool_free(void *data);
s32 mempool_alloc_safe(s32 size, u32 colourTag);

/* JFG: allocate the three shadow buffers (each split in four) for the given counts. */
void shadow_buffers_alloc(s32 arg0, s32 arg1, s32 count0, s32 count1, s32 count2) {
    s32 size0;
    s32 size1;
    s32 size2;
    s32 i;

    D_800F5B0C = arg0;
    D_800F5B10 = arg1;
    D_800F5B00 = count0;
    D_800F5B04 = count1;
    D_800F5B08 = count2;
    size0 = count0 * 10;
    D_800A0E80[0] = mempool_alloc_safe(size0 * 4, 0xFFFF00FF);
    size1 = count1 * 16;
    D_800A0E90[0] = mempool_alloc_safe(size1 * 4, 0xFFFF00FF);
    size2 = count2 * 8;
    D_800A0EA0[0] = mempool_alloc_safe(size2 * 4, 0xFFFF00FF);
    for (i = 1; i < 4; i++) {
        D_800A0E80[i] = D_800A0E80[i - 1] + size0;
        D_800A0E90[i] = D_800A0E90[i - 1] + size1;
        D_800A0EA0[i] = D_800A0EA0[i - 1] + size2;
    }
    D_800A0EC8 = 0;
}

/* JFG: free the three shadow buffers. */
void shadow_buffers_free(void) {
    if (((void **) D_800A0E80)[0] != NULL) {
        mempool_free(((void **) D_800A0E80)[0]);
        ((void **) D_800A0E80)[0] = NULL;
    }
    if (((void **) D_800A0E90)[0] != NULL) {
        mempool_free(((void **) D_800A0E90)[0]);
        ((void **) D_800A0E90)[0] = NULL;
    }
    if (((void **) D_800A0EA0)[0] != NULL) {
        mempool_free(((void **) D_800A0EA0)[0]);
        ((void **) D_800A0EA0)[0] = NULL;
    }
}

/* JFG: flip between two buffers. */
void func_8001D9B4(void) {
    D_800A0EC8 ^= 1;
}

void func_8001D9D0(s32 flags, s32 *a, s32 *b, s32 *c) {
    s32 index = D_800A0EC8;
    if (flags & 2) {
        index += 2;
    }
    *a = D_800A0E80[index];
    *b = D_800A0E90[index];
    *c = D_800A0EA0[index];
}

/* DKR include/structs.h, reduced (as in 12D50.c) */
typedef struct ShadowBatch {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u16 texOffset;
    /* 0x0C */ u32 flags;
} ShadowBatch;

typedef struct ShadowVertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 pad6[4];
} ShadowVertex;

typedef struct ShadowTriangle {
    /* 0x00 */ u8 verticesArray[4]; // [0]: flags
    /* 0x04 */ s16 uv[3][2];
} ShadowTriangle;

/* DKR unk8011C8B8 (a clipped shadow vertex) */
typedef struct ShadowPoint {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE; // the index of the vertex it was clipped to (-1 for a new one)
} ShadowPoint;

/* DKR's shadow vertex cache entry (D_8011B120) */
typedef struct ShadowCacheVertex {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 *plane;
} ShadowCacheVertex;

/* DKR's shadow polygon (D_8011C238) */
typedef struct ShadowPoly {
    /* 0x00 */ u8 numVerts;
    /* 0x01 */ u8 sharedMask;
    /* 0x02 */ u8 verts[8];
    /* 0x0A */ s16 shade;
} ShadowPoly;

/* JFG: the shadow being built (DKR's gNewShadow* globals) */
typedef struct ShadowObjHeader {
    /* 0x00 */ u8 pad0[0x58];
    /* 0x58 */ f32 shadowScale;
    /* 0x5C */ u8 pad5C[0x6C - 0x5C];
    /* 0x6C */ s16 shadowBottom; // DKR: unk42
    /* 0x6E */ s16 shadowTop;    // DKR: unk44
} ShadowObjHeader;

/* JFG: the object casting the shadow (DKR gNewShadowObj) */
typedef struct ShadowObj {
    /* 0x00 */ s16 yRotation;
    /* 0x02 */ u8 pad2[0xC - 0x2];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x30 - 0x18];
    /* 0x30 */ f32 distanceToCamera;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ ShadowObjHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ s16 unk4A;
} ShadowObj;

/* JFG: the shadow texture (DKR gNewShadowTexture) */
typedef struct ShadowTexture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
} ShadowTexture;

typedef struct ShadowBuild {
    /* 0x00 */ ShadowObj *obj;
    /* 0x04 */ ShadowTexture *texture;
    /* 0x08 */ f32 *plane;
    /* 0x0C */ s16 y1;
    /* 0x0E */ s16 y2;
    /* 0x10 */ u8 pad10[0x14 - 0x10];
    /* 0x14 */ f32 yOffset;      // DKR D_8011D0C8
    /* 0x18 */ f32 groundY;      // DKR D_8011D0D0
    /* 0x1C */ f32 unk1C;        // DKR D_8011D0F0
    /* 0x20 */ f32 fadeDistance; // DKR D_8011D0F4
    /* 0x24 */ f32 scale;          // DKR gNewShadowScale
    /* 0x28 */ f32 area;           // DKR D_8011D0E4
    /* 0x2C */ f32 width;        // DKR gNewShadowWidth
    /* 0x30 */ f32 length;       // DKR gNewShadowLength
    /* 0x34 */ f32 unk34;
    /* 0x38 */ f32 x1;
    /* 0x3C */ f32 z1;
    /* 0x40 */ f32 x2;
    /* 0x44 */ f32 z2;
} ShadowBuild;

/* JFG: ShadowSegment with the collision data DKR's func_8002E904 reads */
typedef struct ShadowSegmentCol {
    /* 0x00 */ ShadowVertex *vertices;
    /* 0x04 */ struct ShadowTriangle *triangles;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ struct ShadowBatch *batches;
    /* 0x10 */ u32 *triangleMasks;
    /* 0x14 */ u8 pad14[0x18 - 0x14];
    /* 0x18 */ struct { u16 basePlaneIndex; u8 pad2[6]; } *collisionFacets;
    /* 0x1C */ f32 *collisionPlanes;
    /* 0x20 */ u8 pad20[0x28 - 0x20];
    /* 0x28 */ s16 numberOfBatches;
} ShadowSegmentCol;

/* DKR Vec2f: a clip edge point (x, z) */
typedef struct ShadowEdge {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 z;
} ShadowEdge;

/* DKR's clip intersection cache entry (D_8011B330) */
typedef struct ShadowClipVertex {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 *plane;
    /* 0x10 */ f32 x1;
    /* 0x14 */ f32 z1;
    /* 0x18 */ f32 x2;
    /* 0x1C */ f32 z2;
} ShadowClipVertex;

extern s32 D_800F35D8[4]; // DKR: D_8011B320
extern ShadowClipVertex D_800F35E8[4 * 32]; // DKR: D_8011B330

extern s32 D_800A0EFC; // JFG: build water shadows
extern s32 D_800F33D0; // DKR: D_8011B118
extern ShadowCacheVertex D_800F33D8[32]; // DKR: D_8011B120
extern s32 D_800F45E8; // DKR: D_8011C230
extern ShadowPoly D_800F45F0[]; // DKR: D_8011C238
extern s32 D_800F5AF8; // DKR: D_8011D0E8
extern s32 D_800F5AFC; // DKR: D_8011D0EC
s32 func_80020890(f32 x1, f32 z1, f32 x2, f32 z2, s32 count, ShadowPoint *points); // DKR: func_8002FD74
void func_800209E4(ShadowBuild *shadow, ShadowPoint *points); // DKR: func_800304C8
s32 func_8001F4BC(ShadowBuild *shadow, s32 count, ShadowPoint *points, s32 numEdges, ShadowEdge *edges); // DKR: func_8002FF6C

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001DA1C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E17C);

#ifdef NON_MATCHING
/* NON_MATCHING: 6 words longer: the target keeps the face loop's strength-reduced offsets (face * 8, face * 4) in v1/a0 and spills them once per face, mine keeps them on the stack (and the next-face bound in v1, target a2) */
/* DKR func_8002E904 (tracks.c): clip the shadow against each upward-facing triangle of the segment that the mask
 * selects and add the clipped polygons (JFG: the shadow struct and the clip edges passed in, the water flag a
 * global, faces need a plane normal y above 0.5, at most 32 cached vertices). */
void func_8001E8B8(ShadowBuild *shadow, ShadowEdge *edges, ShadowSegmentCol *seg, s32 mask) {
    ShadowPoint sp100[8];
    s32 spAC;
    s32 curFacesOffset;
    s32 nextFacesOffset;
    ShadowTriangle *triangles;
    ShadowVertex *vertices;
    s32 yPos;
    s32 minY;
    s32 foundIndex;
    s32 maxY;
    s32 temp_t6;
    s32 sp88;
    s32 someCount;
    s32 i2;
    s32 i;
    s32 tempIdx;
    u32 batchFlags;

    for (spAC = 0; spAC < seg->numberOfBatches; spAC++) {
        batchFlags = seg->batches[spAC].flags;
        if ((!D_800A0EFC && !(batchFlags & 0x3480)) ||
            (D_800A0EFC && (batchFlags & 0x400))) {
            curFacesOffset = seg->batches[spAC].facesOffset;
            nextFacesOffset = seg->batches[spAC + 1].facesOffset;
            vertices = &seg->vertices[seg->batches[spAC].verticesOffset];
            sp88 = (seg->batches[spAC].flags >> 22) & 7;
            for (; curFacesOffset < nextFacesOffset; curFacesOffset++) {
                temp_t6 = seg->collisionFacets[curFacesOffset].basePlaneIndex * 4;
                if (((seg->triangleMasks[curFacesOffset] & mask) & 0xFFFF) &&
                    ((seg->triangleMasks[curFacesOffset] & mask) >> 16) &&
                    seg->collisionPlanes[temp_t6 + 1] > 0.5f) {
                    triangles = &seg->triangles[curFacesOffset];
                    maxY = minY = vertices[triangles->verticesArray[1]].y;
                    for (i = 1; i < 3; i++) {
                        yPos = vertices[triangles->verticesArray[i + 1]].y;
                        if (yPos < minY) {
                            minY = yPos;
                        } else if (maxY < yPos) {
                            maxY = yPos;
                        }
                    }
                    if (shadow->y2 >= minY) {
                        if (maxY >= shadow->y1) {
                            for (i = 0; i < 3; i++) {
                                sp100[i].x = vertices[triangles->verticesArray[i + 1]].x;
                                sp100[i].z = vertices[triangles->verticesArray[i + 1]].z;
                                sp100[i].unkE = -1;
                            }
                            if (func_80020890(shadow->x1, shadow->z1, shadow->x2, shadow->z2, 3, sp100) != 0) {
                                shadow->plane = &seg->collisionPlanes[temp_t6];
                                if (shadow->unk1C > 0.0f) {
                                    func_800209E4(shadow, sp100);
                                }
                                someCount = func_8001F4BC(shadow, 3, sp100, 4, edges);
                                if (someCount >= 3) {
                                    tempIdx = D_800F45E8;
                                    D_800F45F0[tempIdx].sharedMask = 0;
                                    for (i2 = 0; i2 < someCount; i2++) {
                                        if (sp100[i2].unkE < 0) {
                                            foundIndex = -1;
                                            i = 0;
                                            while ((i < D_800F33D0) && (foundIndex == -1)) {
                                                if ((D_800F33D8[i].x == sp100[i2].x) &&
                                                    (D_800F33D8[i].z == sp100[i2].z)) {
                                                    foundIndex = i;
                                                }
                                                i++;
                                            }
                                            if (foundIndex == -1) {
                                                if (D_800F33D0 >= 32) {
                                                    D_800F33D0 = 31;
                                                }
                                                D_800F33D8[D_800F33D0].x = sp100[i2].x;
                                                D_800F33D8[D_800F33D0].plane = shadow->plane;
                                                D_800F33D8[D_800F33D0].z = sp100[i2].z;
                                                D_800F45F0[tempIdx].verts[i2] = D_800F33D0++;
                                            } else {
                                                D_800F45F0[tempIdx].verts[i2] = foundIndex;
                                            }
                                        } else {
                                            D_800F45F0[tempIdx].verts[i2] = sp100[i2].unkE;
                                            D_800F45F0[tempIdx].sharedMask |= 1 << i2;
                                        }
                                    }
                                    D_800F45F0[tempIdx].numVerts = someCount;
                                    D_800F45F0[tempIdx].shade = sp88;
                                    D_800F45E8 = tempIdx + 1;
                                    if ((D_800F5AF8 >= 0) && (sp88 != D_800F5AF8)) {
                                        D_800F5AFC = 0;
                                    }
                                    D_800F5AF8 = sp88;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E8B8);
#endif

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001EE1C);

#ifdef NON_MATCHING
/* NON_MATCHING: an FP register rotation only (dz, -dx and the edge offset get f22/f24/f20 instead of
 * f20/f22/f24). */
/* DKR func_8002FF6C (tracks.c): clip the shadow polygon against each edge, caching the edge intersections (JFG: the
 * shadow struct passed in, the counts as parameters, stops at 8 vertices). */
s32 func_8001F4BC(ShadowBuild *shadow, s32 count, ShadowPoint *points, s32 numEdges, ShadowEdge *edges) {
    ShadowPoint spD8[8];
    f32 temp_f12;
    f32 temp_f14;
    f32 temp_f16;
    f32 temp_f22;
    f32 temp_f24;
    f32 var_f2;
    s32 var_a1;
    s32 var_v1_3;
    s32 var_a0;
    s32 var_t2;
    s32 var_v0;
    s32 var_v1;
    s32 var_t1;
    s32 var_s2;
    ShadowPoint *var_s0;
    ShadowPoint *var_s3;
    ShadowPoint *swap; // swap var for var_s0 and var_s3
    ShadowClipVertex *entry;

    var_s3 = points;
    var_s0 = spD8;
    var_s2 = count;
    var_v0 = 0;

    while (var_v0 < numEdges && var_s2 >= 3) {
        var_v1 = var_v0 + 1;
        if (var_v1 >= numEdges) {
            var_v1 = 0;
        }

        temp_f12 = edges[var_v1].z - edges[var_v0].z;
        temp_f14 = -(edges[var_v1].x - edges[var_v0].x);
        if (edges[var_v0].x < edges[var_v1].x) {
            var_f2 = (temp_f12 * edges[var_v0].x) + (edges[var_v0].z * temp_f14);
            var_f2 = -var_f2;
        } else {
            var_f2 = (temp_f12 * edges[var_v1].x) + (edges[var_v1].z * temp_f14);
            var_f2 = -var_f2;
        }

        for (var_t1 = 0, var_t2 = 0; var_t1 < var_s2; var_t1++) {
            var_v1 = var_t1 + 1;
            if (var_v1 >= var_s2) {
                var_v1 = 0;
            }
            temp_f16 = (temp_f12 * var_s3[var_t1].x) + (var_s3[var_t1].z * temp_f14) + var_f2;
            temp_f22 = (temp_f12 * var_s3[var_v1].x) + (var_s3[var_v1].z * temp_f14) + var_f2;
            if ((temp_f16 >= 0.0f && temp_f22 < 0.0f) || (temp_f16 < 0.0f && temp_f22 >= 0.0f)) {
                var_a0 = D_800F35D8[var_v0];
                var_a1 = -1;
                var_v1_3 = var_v0 << 5;
                entry = &D_800F35E8[var_v1_3];
                while (var_a0 > 0) {
                    if (((entry->x1 == var_s3[var_t1].x) && (entry->z1 == var_s3[var_t1].z) &&
                         (entry->x2 == var_s3[var_v1].x) && (entry->z2 == var_s3[var_v1].z)) ||
                        (var_a0 -= 1, (entry->x1 == var_s3[var_v1].x) && (entry->z1 == var_s3[var_v1].z) &&
                                          (entry->x2 == var_s3[var_t1].x) && (entry->z2 == var_s3[var_t1].z))) {
                        var_a1 = var_v1_3;
                        break;
                    }
                    entry++;
                    var_v1_3++;
                }
                if (var_a1 >= 0) {
                    var_s0[var_t2].unkE = var_a1;
                    var_s0[var_t2].x = entry->x;
                    var_s0[var_t2].z = entry->z;
                    var_t2++;
                    if (var_t2 >= 8) {
                        return 0;
                    }
                } else {
                    temp_f24 = temp_f16 / (temp_f16 - temp_f22);
                    var_s0[var_t2].x = var_s3[var_t1].x + ((var_s3[var_v1].x - var_s3[var_t1].x) * temp_f24);
                    var_s0[var_t2].z = var_s3[var_t1].z + ((var_s3[var_v1].z - var_s3[var_t1].z) * temp_f24);
                    entry->x1 = var_s3[var_t1].x;
                    entry->z1 = var_s3[var_t1].z;
                    entry->x2 = var_s3[var_v1].x;
                    entry->z2 = var_s3[var_v1].z;
                    entry->x = var_s0[var_t2].x;
                    entry->z = var_s0[var_t2].z;
                    entry->plane = shadow->plane;
                    var_s0[var_t2].unkE = var_v1_3;
                    var_t2++;
                    if (var_t2 >= 8) {
                        return 0;
                    }
                    D_800F35D8[var_v0]++;
                }
            }
            if (temp_f22 <= 0) {
                var_s0[var_t2].unkE = var_s3[var_v1].unkE;
                var_s0[var_t2].x = var_s3[var_v1].x;
                var_s0[var_t2].z = var_s3[var_v1].z;
                var_t2++;
                if (var_t2 >= 8) {
                    return 0;
                }
            }
        }
        var_s2 = var_t2;
        var_v0++;

        swap = var_s3;
        var_s3 = var_s0;
        var_s0 = swap;
    }

    if (var_s2 >= 3) {
        if (var_s3 != points) {
            for (var_t1 = 0; var_t1 < var_s2; var_t1++) {
                points[var_t1].x = var_s3[var_t1].x;
                points[var_t1].z = var_s3[var_t1].z;
                points[var_t1].unkE = var_s3[var_t1].unkE;
            }
        }
    } else {
        var_s2 = 0;
    }
    return var_s2;
}
#else
INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001F4BC);
#endif

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001FA24);


typedef struct ShadowSegment {
    /* 0x00 */ ShadowVertex *vertices;
    /* 0x04 */ ShadowTriangle *triangles;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ ShadowBatch *batches;
    /* 0x10 */ u32 *triangleMasks;
    /* 0x14 */ u8 pad14[0x28 - 0x14];
    /* 0x28 */ s16 numberOfBatches;
    /* 0x2A */ u8 pad2A[0x48 - 0x2A];
} ShadowSegment;

typedef struct ShadowLevel {
    /* 0x00 */ void *textures;
    /* 0x04 */ ShadowSegment *segments;
    /* 0x08 */ struct ShadowBox { s16 v[6]; } *segmentsBoundingBoxes;
} ShadowLevel;

extern f32 D_800A0ED4[];
extern f32 D_800F5B14;
u32 func_8001D344(void *bb, s32 x1, s32 z1, s32 x2, s32 z2);
s32 func_800494AC(s32 x, s32 z, ShadowVertex *v0, ShadowVertex *v1, ShadowVertex *v2);

/* DKR func_8002DE30 (tracks.c): ease the object's shading (0x54) towards the brightness of the first triangle
 * under it (within its height range, header 0x6C/0x6E); JFG: 16-bit cell masks, a variable rate. */
void obj_shading_from_ground(u8 *obj) {
    s32 sp94;
    s32 sp90;
    s32 blockId;
    s32 k;
    u32 batchFlags;
    s32 foundResult;
    s32 i;
    ShadowSegment *block;
    ShadowTriangle *triangle;
    ShadowVertex *vertices;
    s32 var_t3;
    s32 minYPos;
    s32 maxYPos;
    s32 j;

    sp94 = *(s16 *) (*(u8 **) (obj + 0x40) + 0x6E) + (s32) *(f32 *) (obj + 0x10);
    sp90 = *(s16 *) (*(u8 **) (obj + 0x40) + 0x6C) + (s32) *(f32 *) (obj + 0x10);
    blockId = *(s16 *) (obj + 0x2E);
    foundResult = FALSE;
    if (blockId != -1) {
        var_t3 = func_8001D344(&((ShadowLevel *) D_800F5B0C)->segmentsBoundingBoxes[blockId],
                               *(f32 *) (obj + 0xC) - 16.0f, *(f32 *) (obj + 0x14) - 16.0f,
                               *(f32 *) (obj + 0xC) + 16.0f, *(f32 *) (obj + 0x14) + 16.0f);
        block = &((ShadowLevel *) D_800F5B0C)->segments[blockId];
        for (i = 0; i < block->numberOfBatches && !foundResult; i++) {
            if (!(block->batches[i].flags & 0x3480)) {
                batchFlags = (block->batches[i].flags >> 22) & 7;
                vertices = &block->vertices[block->batches[i].verticesOffset];
                for (j = block->batches[i].facesOffset; j < block->batches[i + 1].facesOffset && !foundResult; j++) {
                    blockId = block->triangleMasks[j] & var_t3;
                    if (blockId) {} // DKR's fakematch
                    if (((block->triangleMasks[j] & var_t3) & 0xFFFF) && ((block->triangleMasks[j] & var_t3) >> 16)) {
                        triangle = &block->triangles[j];
                        minYPos = vertices[triangle->verticesArray[1]].y;
                        maxYPos = minYPos;
                        for (k = 1; k < 3; k++) {
                            if (vertices[triangle->verticesArray[k + 1]].y < minYPos) {
                                minYPos = vertices[triangle->verticesArray[k + 1]].y;
                            } else if (maxYPos < vertices[triangle->verticesArray[k + 1]].y) {
                                maxYPos = vertices[triangle->verticesArray[k + 1]].y;
                            }
                        }
                        if (maxYPos >= sp90 && sp94 >= minYPos) {
                            if (func_800494AC(*(f32 *) (obj + 0xC), *(f32 *) (obj + 0x14),
                                              &vertices[triangle->verticesArray[1]],
                                              &vertices[triangle->verticesArray[2]],
                                              &vertices[triangle->verticesArray[3]])) {
                                foundResult = TRUE;
                                **(f32 **) (obj + 0x54) +=
                                    ((1.0f - D_800A0ED4[batchFlags]) - **(f32 **) (obj + 0x54)) * D_800F5B14;
                            }
                        }
                    }
                }
            }
        }
    }
}

/* JFG: a model's face normals (the first halfword of each 14-byte face entry indexes the normal table) */
typedef struct HitNormalSet {
    /* 0x00 */ u8 *faces;
    /* 0x04 */ u8 pad4[0xC - 0x4];
    /* 0x0C */ f32 (*normals)[4];
} HitNormalSet;

/* JFG: an object model's triangle batch (0x10 bytes) */
typedef struct HitBatch {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ u32 flags;
} HitBatch;

typedef struct HitModel {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
    /* 0x20 */ ShadowTriangle *triangles;
    /* 0x24 */ HitBatch *batches;
    /* 0x28 */ u8 pad28[0x60 - 0x28];
    /* 0x60 */ HitNormalSet *normals;
} HitModel;

typedef struct HitInstance {
    /* 0x00 */ HitModel *model;
    /* 0x04 */ ShadowVertex *vertices;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ HitNormalSet *normals; // NULL: the model's own
} HitInstance;

typedef struct HitObjData {
    /* 0x000 */ u8 pad0[0x12B];
    /* 0x12B */ u8 matrixIndex;
    /* 0x12C */ u8 pad12C[0x133 - 0x12C];
    /* 0x133 */ u8 instanceIndex;
} HitObjData;

typedef struct HitObj {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ f32 scale;
    /* 0x0C */ u8 padC[0x5C - 0xC];
    /* 0x5C */ HitObjData *data;
    /* 0x60 */ u8 pad60[0x6C - 0x60];
    /* 0x6C */ HitInstance **instances;
} HitObj;

void func_8004A2FC(MtxF *mtx, f32 *in, f32 *out);                                // a direction by the matrix
void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); // a point by the matrix

#ifdef NON_MATCHING
/* NON_MATCHING: the frame matches; the target keeps the point matrix in s7 for the whole function and the batch
 * index in memory with its own *16 counter (mine recomputes mtx + 2 and i << 4). */
/*
 * JFG: collect the faces of the object's model that face up (normal y times the scale above 0.5) and overlap the
 * box (x1, z1)-(x2, z2): their three corners go to `out` as shorts (10 per face) and their planes, scaled, to
 * `planes`. Returns the number of faces.
 */
s32 func_800203C4(HitObj *obj, s16 *out, f32 *planes, f32 x1, f32 z1, f32 x2, f32 z2) {
    ShadowPoint points[8];
    HitInstance *inst;
    HitModel *model;
    HitNormalSet *normalSet;
    ShadowVertex *v;
    HitBatch *batch;
    f32 *planesOut;
    MtxF *pointMtx;
    f32 (*normals)[4];
    MtxF *mtx;
    s32 j;
    s32 k;
    s32 vertOffset;
    s32 numBatches;
    f32 plane[4];
    s16 *dst;
    s32 i;
    UNUSED s32 pad;
    s32 faceEnd;
    UNUSED s32 pad2[2];
    s32 count;
    UNUSED s32 pad3;

    inst = obj->instances[obj->data->instanceIndex];
    model = inst->model;
    if (inst->normals != NULL) {
        normalSet = inst->normals;
    } else {
        normalSet = model->normals;
    }
    normals = normalSet->normals;
    planesOut = planes;
    dst = out;
    count = 0;
    mtx = (MtxF *) ((u8 *) obj->data + (obj->data->matrixIndex << 6));
    pointMtx = mtx + 2;
    for (i = 0; i < model->numberOfBatches; i++) {
        batch = &model->batches[i];
        if (!(batch->flags & 0x1C80)) {
            j = batch->facesOffset;
            faceEnd = batch[1].facesOffset;
            vertOffset = batch->verticesOffset;
            for (; j < faceEnd; j++) {
                func_8004A2FC(mtx, normals[*(u16 *) (normalSet->faces + j * 14)], plane);
                if (plane[1] * obj->scale > 0.5f) {
                    for (k = 0; k < 3; k++) {
                        v = &inst->vertices[model->triangles[j].verticesArray[k + 1] + vertOffset];
                        func_80048B60(pointMtx, v->x, v->y, v->z, &points[k].x, &points[k].y, &points[k].z);
                    }
                    if (func_80020890(x1, z1, x2, z2, 3, points)) {
                        dst[0] = points[0].x;
                        dst[1] = points[0].y;
                        dst[2] = points[0].z;
                        dst[3] = points[1].x;
                        dst[4] = points[1].y;
                        dst[5] = points[1].z;
                        dst[6] = points[2].x;
                        dst[7] = points[2].y;
                        dst[8] = points[2].z;
                        dst += 10;
                        planesOut[0] = plane[0] * obj->scale;
                        planesOut[1] = plane[1] * obj->scale;
                        planesOut[2] = plane[2] * obj->scale;
                        planesOut[3] = plane[3] * obj->scale;
                        planesOut += 4;
                        count++;
                    }
                }
            }
        }
    }
    return count;
}
#else
INCLUDE_ASM("asm/nonmatchings/1DA10", func_800203C4);
#endif

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020890);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020954);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_800209E4);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020AF8);
