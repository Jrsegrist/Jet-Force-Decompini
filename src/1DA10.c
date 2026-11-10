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

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001DA1C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E17C);

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
} ShadowObjHeader;

/* JFG: the object casting the shadow (DKR gNewShadowObj) */
typedef struct ShadowObj {
    /* 0x00 */ s16 yRotation;
    /* 0x02 */ u8 pad2[0xC - 0x2];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x40 - 0x18];
    /* 0x40 */ ShadowObjHeader *header;
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
    /* 0x24 */ u8 pad24[0x2C - 0x24];
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

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E8B8);

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

INCLUDE_ASM("asm/nonmatchings/1DA10", func_800203C4);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020890);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020954);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_800209E4);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020AF8);
