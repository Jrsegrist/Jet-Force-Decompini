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

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E8B8);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001EE1C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001F4BC);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001FA24);

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
void func_8001FFE0(u8 *obj) {
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
