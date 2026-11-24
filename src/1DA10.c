/*
 * Object shadows: the double-buffered shadow vertex and triangle buffers and building the shadows.
 */
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
void *mempool_alloc_safe(s32 size, u32 colourTag);

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
    D_800A0E80[0] = (s32) mempool_alloc_safe(size0 * 4, 0xFFFF00FF);
    size1 = count1 * 16;
    D_800A0E90[0] = (s32) mempool_alloc_safe(size1 * 4, 0xFFFF00FF);
    size2 = count2 * 8;
    D_800A0EA0[0] = (s32) mempool_alloc_safe(size2 * 4, 0xFFFF00FF);
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

/* JFG: flip between two buffers (0x8001D9B4). */
void shadow_buffers_flip(void) {
    D_800A0EC8 ^= 1;
}

/* The current shadow buffers (0x8001D9D0). */
void shadow_heap_current_ptrs(s32 flags, s32 *a, s32 *b, s32 *c) {
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

/* JFG: a shadow vertex, triangle and mesh entry in the shadow heaps (DKR Vertex, Triangle, ShadowHeapProperties) */
typedef struct ShadowHeapVertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} ShadowHeapVertex;

typedef struct ShadowHeapTriangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vi0;
    /* 0x02 */ u8 vi1;
    /* 0x03 */ u8 vi2;
    /* 0x04 */ s32 uv0;
    /* 0x08 */ s32 uv1;
    /* 0x0C */ s32 uv2;
} ShadowHeapTriangle;

typedef struct ShadowHeapMesh {
    /* 0x00 */ ShadowTexture *texture;
    /* 0x04 */ s16 triCount;
    /* 0x06 */ s16 vtxCount;
} ShadowHeapMesh;

extern ShadowHeapVertex *D_800A0EB0;   // DKR: gCurrShadowVerts
extern ShadowHeapTriangle *D_800A0EB4; // DKR: gCurrShadowTris
extern ShadowHeapMesh *D_800A0EB8;     // DKR: gCurrShadowHeapData
extern s32 D_800A0EBC;                 // DKR: gNewShadowVtxCount
extern s32 D_800A0EC0;                 // DKR: gNewShadowTriCount
extern s32 D_800A0EC4;                 // DKR: gShadowTail
extern f32 D_800F5AF0;                 // DKR: gShadowOpacity

/* JFG: a light direction for a shadow (6 bytes): the level's sun, or one of the object's shading lights */
typedef struct ShadowLight {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ s16 pitch;
    /* 0x04 */ s16 intensity;
} ShadowLight;

/* JFG: one light of an object's shading data (0x20 bytes) */
typedef struct ShadingLight {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 padC[0x15 - 0xC];
    /* 0x15 */ u8 intensity;
    /* 0x16 */ u8 pad16[0x20 - 0x16];
} ShadingLight;

/* JFG: an object's shading data (DKR ShadeProperties, obj 0x54) */
typedef struct ShadowShading {
    /* 0x00 */ f32 brightness;
    /* 0x04 */ u8 pad4[0xE - 0x4];
    /* 0x0E */ s16 numLights; // [0] is not a light
    /* 0x10 */ ShadingLight lights[1];
} ShadowShading;

/* JFG: an object's shadow (DKR ShadowData, obj 0x50; ShadowFlags below) */
typedef struct ShadowCaster {
    /* 0x00 */ f32 scale;
    /* 0x04 */ ShadowTexture *texture;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ u8 flags; // 1, 2: the group; 4: one shadow only; 8, 0x10: a module may pick the linked shadow
    /* 0x0D */ u8 maxShadows;
    /* 0x0E */ u8 stretch; // how far a low light stretches the shadow
    /* 0x0F */ u8 numShadows;
    /* 0x10 */ s16 meshStart[3]; // per shadow built (DKR meshStart)
    /* 0x16 */ s16 meshEnd[3];   // (DKR meshEnd)
    /* 0x1C */ struct ShadowCaster *linked;
} ShadowCaster;

/* JFG: the object header as func_8001DA1C reads it */
typedef struct ShadowGenHeader {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ u16 flags; // bit 0: no shadow
    /* 0x16 */ u8 pad16[0x68 - 0x16];
    /* 0x68 */ s16 shadowFadeMin; // DKR: shadowFadeMin (no shadow at or beyond)
    /* 0x6A */ s16 shadowFadeMax; // DKR: shadowFadeMax (fading from)
    /* 0x6C */ s16 shadowBottom;  // DKR: unk42
    /* 0x6E */ s16 shadowTop;     // DKR: unk44
} ShadowGenHeader;

/* JFG: the object as func_8001DA1C reads it */
typedef struct ShadowGenObj {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 flags; // 0x400: invisible
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x30 - 0x18];
    /* 0x30 */ f32 distanceToCamera;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ ShadowGenHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ u8 pad4C[0x50 - 0x4C];
    /* 0x50 */ ShadowCaster *shadow;
    /* 0x54 */ ShadowShading *shading;
} ShadowGenObj;

/* JFG: the level header's sun */
typedef struct ShadowLevelHeader {
    /* 0x00 */ u8 pad0[0xD4];
    /* 0xD4 */ s16 sunYaw;
    /* 0xD6 */ s16 sunPitch;
    /* 0xD8 */ u8 padD8[0xDE - 0xD8];
    /* 0xDE */ u8 sunIntensity;
} ShadowLevelHeader;

extern ShadowLight D_800A0ECC; // JFG: the level's sun
extern f32 D_800F5B14;
f32 func_80049BEC(f32 rate, s32 updateRate);
s32 func_80049C84(f32 x, f32 z);
f32 sqrtf(f32 x);
ShadowLevelHeader *level_header(void);
ShadowGenObj **objGetObjList(s32 *start, s32 *end);
s32 get_current_viewport(void);
f32 get_distance_to_active_camera(f32 x, f32 y, f32 z);
s32 func_80054530();                                          // a module function (patched in by the runtime linker)
s32 func_80054530_f32(ShadowGenObj *obj, s32 arg1, f32 rate); // the same, with a float argument
void func_8001E17C(ShadowGenObj *obj, ShadowLight *light, ShadowCaster *shadow);
void obj_shading_from_ground(u8 *obj);

/**
 * DKR shadow_update (tracks.c), JFG's form: set up the shadow heaps for the group, then for each object whose shadow
 * is in the group, close enough (fading between the header's two distances) and not hidden, build its shadow: once
 * (a module may pick the linked shadow), or once per light, the level's sun and the object's shading lights above
 * it, the brightest first when there are more than the shadow allows. Then the object's ground shading.
 * The level header goes through header and the sort's swap through rec (each pair is never live together); dist,
 * dead after the fade, holds the light's ground distance (sqrtf's result as a local, not a nested call: a nested
 * call costs IDO one more reserved temporary word).
 */
void func_8001DA1C(s32 group, s32 updateRate) {
    ShadowGenObj *obj;
    ShadowCaster *shadow;
    ShadowGenHeader *header;
    s32 objIndex;
    s32 count;
    s32 numLights;
    ShadowLight buf[6];
    ShadowLight *lights[4];
    s32 i;
    s32 j;
    ShadowGenObj **objects;
    ShadowCaster *linked;
    ShadowShading *shading;
    ShadingLight *rec;
    f32 dist;
    f32 x;
    f32 z;
    f32 y;

    count = (group & 2) | D_800A0EC8; // the heap index, until the object count
    D_800A0EB0 = (ShadowHeapVertex *) D_800A0E80[count];
    D_800A0EBC = 0;
    D_800A0EB4 = (ShadowHeapTriangle *) D_800A0E90[count];
    D_800A0EC0 = 0;
    D_800A0EB8 = (ShadowHeapMesh *) D_800A0EA0[count];
    D_800A0EC4 = 0;
    D_800F5B14 = 1.0f - func_80049BEC(0.9f, updateRate);
    header = (ShadowGenHeader *) level_header(); // header holds the level header until the loop
    D_800A0ECC.yaw = ((ShadowLevelHeader *) header)->sunYaw;
    D_800A0ECC.pitch = ((ShadowLevelHeader *) header)->sunPitch;
    D_800A0ECC.intensity = ((ShadowLevelHeader *) header)->sunIntensity;
    objects = objGetObjList(&objIndex, &count);
    while (objIndex < count) {
        obj = objects[objIndex];
        objIndex++;
        shadow = obj->shadow;
        if (shadow == NULL) {
            continue;
        }
        if ((shadow->flags & group) == 0) {
            continue;
        }
        header = obj->header;
        dist = 0.0f;
        if ((shadow->flags & group) == 1) {
            if (get_current_viewport() == 0) {
                dist = get_distance_to_active_camera(obj->x, obj->y, obj->z);
            } else if (obj->behaviorId != 1 && obj->unk4A != 0x278) {
                dist = 32768.0f;
            }
        }
        if ((shadow->flags & 0x18) && shadow->linked != NULL) {
            linked = shadow->linked;
            linked->numShadows = 0;
        } else {
            linked = NULL;
        }
        shadow->numShadows = 0;
        if (shadow->scale > 0.0f && !(obj->flags & 0x400) && !(header->flags & 1)) {
            if (dist < header->shadowFadeMin) {
                if (header->shadowFadeMax < dist) {
                    D_800F5AF0 = (header->shadowFadeMin - dist) / (header->shadowFadeMin - header->shadowFadeMax);
                } else {
                    D_800F5AF0 = 1.0f;
                }
                if (shadow->flags & 4) {
                    j = 0; // the module's choice
                    if (linked != NULL) {
                        if (linked->flags & 8) {
                            j = func_80054530_f32(obj, 0, updateRate);
                        } else if (linked->flags & 0x10) {
                            j = func_80054530(obj);
                        }
                    }
                    if (j) {
                        func_8001E17C(obj, NULL, linked);
                    } else {
                        func_8001E17C(obj, NULL, shadow);
                    }
                } else if (linked != NULL && (linked->flags & 8)) {
                    if (func_80054530_f32(obj, 1, updateRate)) {
                        func_8001E17C(obj, NULL, linked);
                    } else {
                        func_8001E17C(obj, NULL, shadow);
                    }
                } else {
                    numLights = 0;
                    if (linked != NULL && (linked->flags & 0x10)) {
                        if (func_80054530(obj)) {
                            func_8001E17C(obj, NULL, linked);
                        } else {
                            func_8001E17C(obj, NULL, shadow);
                        }
                    } else if (D_800A0ECC.intensity > 0) {
                        numLights = 1;
                        lights[0] = &D_800A0ECC;
                    }
                    shading = obj->shading;
                    if (shading != NULL) {
                        for (i = 1; i < shading->numLights; i++) {
                            rec = &shading->lights[i];
                            if (rec->y > 0.0f) {
                                x = rec->x;
                                z = rec->z;
                                y = rec->y;
                                lights[numLights] = &buf[numLights];
                                buf[numLights].yaw = func_80049C84(-x, -z);
                                dist = sqrtf(x * x + z * z); // the light's ground distance (the fade's dist is dead)
                                buf[numLights].pitch = func_80049C84(y, dist);
                                buf[numLights].intensity = rec->intensity;
                                numLights++;
                            }
                        }
                    }
                    if (shadow->maxShadows < numLights) {
                        for (i = numLights - 1; i > 0; i--) {
                            for (j = 0; j < i; j++) {
                                if (lights[j]->intensity < lights[j + 1]->intensity) {
                                    rec = (ShadingLight *) lights[j]; // rec as the swap temporary
                                    lights[j] = lights[j + 1];
                                    lights[j + 1] = (ShadowLight *) rec;
                                }
                            }
                        }
                    }
                    for (i = 0; shadow->numShadows < shadow->maxShadows && i < numLights; i++) {
                        func_8001E17C(obj, lights[i], shadow);
                    }
                }
            }
        }
        if (obj->shading != NULL) {
            obj_shading_from_ground((u8 *) obj);
        }
    }
    D_800A0EB8[D_800A0EC4].triCount = D_800A0EC0;
    D_800A0EB8[D_800A0EC4].vtxCount = D_800A0EBC;
}

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

struct ShadowParams;
struct ShadowFlags;
extern s32 D_800F5AF4; // JFG: the planes used so far (func_8001EE1C)
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
void func_80020954(s32 count, ShadowEdge *points, f32 *minX, f32 *minZ, f32 *maxX, f32 *maxZ); // the points' box
void func_80020AF8(s32 count, ShadowCacheVertex *verts, s32 *counts, ShadowClipVertex *clipVerts); // their heights
u32 func_8001D344(void *bb, s32 x1, s32 z1, s32 x2, s32 z2);
s32 get_inside_segment_count_xyz(s32 *arg0, s16 xPos1, s16 yPos1, s16 zPos1, s16 xPos2, s16 yPos2, s16 zPos2);
void func_8001E8B8(ShadowBuild *shadow, ShadowEdge *edges, ShadowSegmentCol *seg, s32 mask);
void func_8001EE1C(ShadowBuild *shadow, ShadowEdge *edges);
s32 func_8001FA24(ShadowBuild *shadow, struct ShadowParams *params, struct ShadowFlags *arg2);

#ifdef NON_MATCHING
/**
 * DKR shadow_generate (tracks.c), JFG's form: one shadow of obj (for a light, or straight down without one): the
 * shadow's size, a lift that grows with the camera distance (non-players), a stretch away from a low light, the
 * four corners turned to the light's yaw, then the level segments and solid object models under it, the heights,
 * and the heap entry (meshStart/meshEnd per shadow; numShadows counts the ones built).
 * NON_MATCHING: a first body written from the ROM: 461/463 words, frame 0x170 (ROM 0x168): the corner rotation
 * spills differently (the ROM keeps -(width * cos) and -(length * cos) in stack temps), FP registers in the lift
 * and the ratio, and scheduling at the start.
 */
void func_8001E17C(ShadowGenObj *obj, ShadowLight *light, ShadowCaster *shadow) {
    s32 yPos;
    f32 dist;
    s32 one;
    s32 ok;
    UNUSED s32 pad0;
    UNUSED s32 pad1;
    UNUSED s32 pad2;
    s32 segs[28];
    s32 count;
    s32 i;
    f32 c;
    f32 ratio;
    UNUSED s32 pad3;
    UNUSED s32 pad4;
    f32 s;
    UNUSED s32 pad5;
    ShadowBuild build;
    ShadowEdge edges[4];

    yPos = obj->y;
    build.obj = (ShadowObj *) obj;
    build.yOffset = 2.0f;
    shadow->meshStart[shadow->numShadows] = D_800A0EC4;
    build.texture = shadow->texture;
    build.y1 = obj->header->shadowBottom + yPos;
    build.y2 = obj->header->shadowTop + yPos;
    one = TRUE;
    if (obj->behaviorId != one) {
        dist = obj->distanceToCamera;
        if (dist < 0.0f) {
            dist = -dist;
        }
        dist -= 250.0f;
        if (dist < 0.0f) {
            dist = 0.0f;
        }
        if (dist > 1024.0f) {
            dist = 1024.0f;
        }
        build.yOffset += dist * 0.01f;
    }
    build.scale = shadow->scale;
    build.width = shadow->scale * 10.0f;
    build.length = shadow->scale * 10.0f;
    build.unk34 = 1.0f;
    if (light != NULL) {
        c = func_80049C30(light->pitch);
        if (c > 0.0f) {
            ratio = c;
            s = func_80049C34(light->pitch);
            if (s != 0.0f) {
                ratio /= s;
                if (ratio > 2.0f) {
                    ratio = 2.0f;
                }
            } else {
                ratio = 2.0f;
            }
            build.unk34 += 0.25f * ratio * shadow->stretch / build.length;
        }
    }
    build.unk34 *= build.length;
    build.area = (build.width + build.width) * (build.length + build.unk34);
    build.unk1C = obj->header->shadowBottom * 0.125f;
    if (build.unk1C < 0.0f) {
        build.unk1C = -build.unk1C;
    }
    build.fadeDistance = 7.0f * build.unk1C;
    build.groundY = -32768.0f;
    build.scale = 144.0f / build.scale;
    for (i = 0; i < 4; i++) {
        edges[i].x = obj->x;
        edges[i].z = obj->z;
    }
    if (light != NULL) {
        s = func_80049C34(light->yaw);
        c = func_80049C30(light->yaw);
        edges[0].x += -(build.width * c) - build.length * s;
        edges[0].z += build.width * s - build.length * c;
        edges[1].x += build.width * c - build.length * s;
        edges[1].z += -(build.length * c) - build.width * s;
        edges[2].x += build.width * c + build.unk34 * s;
        edges[2].z += build.unk34 * c - build.width * s;
        edges[3].x += -(build.width * c) + build.unk34 * s;
        edges[3].z += build.unk34 * c + build.width * s;
    } else {
        edges[0].x += build.width;
        edges[0].z += build.length;
        edges[1].x -= build.width;
        edges[1].z += build.length;
        edges[2].x -= build.width;
        edges[2].z -= build.length;
        edges[3].x += build.width;
        edges[3].z -= build.length;
    }
    func_80020954(4, edges, &build.x1, &build.z1, &build.x2, &build.z2);
    count = get_inside_segment_count_xyz(segs, build.x1, build.y1, build.z1, build.x2, build.y2, build.z2);
    D_800F45E8 = 0;
    D_800F33D0 = 0;
    for (i = 0; i < 4; i++) {
        D_800F35D8[i] = 0;
    }
    D_800F5AF8 = -1;
    D_800F5AFC = -1;
    D_800F5AF4 = 0;
    D_800A0EFC = FALSE;
    if (obj->unk4A == 0x3F || obj->unk4A == 0x40 || obj->unk4A == 0x41 || obj->unk4A == 0x42) {
        D_800A0EFC = one;
    }
    for (i = 0; i < count; i++) {
        if (segs[i] >= 0) {
            func_8001E8B8(&build, edges, (ShadowSegmentCol *) &((ShadowLevel *) D_800F5B0C)->segments[segs[i]],
                          func_8001D344(&((ShadowLevel *) D_800F5B0C)->segmentsBoundingBoxes[segs[i]], build.x1,
                                        build.z1, build.x2, build.z2));
        }
    }
    func_8001EE1C(&build, edges);
    ok = TRUE;
    if (D_800F45E8 > 0) {
        func_80020AF8(D_800F33D0, D_800F33D8, D_800F35D8, D_800F35E8);
        if (!func_8001FA24(&build, (struct ShadowParams *) light, (struct ShadowFlags *) shadow)) {
            ok = FALSE;
        }
    }
    shadow->meshEnd[shadow->numShadows] = D_800A0EC4;
    if (ok) {
        shadow->numShadows++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E17C);
#endif

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

#ifdef NON_MATCHING
/* JFG: an object whose model can take shadows, as func_8001EE1C reads it */
typedef struct ShadowColFlags {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags; // bit 0: solid
} ShadowColFlags;

typedef struct ShadowColData {
    /* 0x000 */ u8 pad0[0x100];
    /* 0x100 */ f32 bboxMin[3];
    /* 0x10C */ f32 bboxMax[3];
} ShadowColData;

typedef struct ShadowColObj {
    /* 0x00 */ u8 pad0[0x4C];
    /* 0x4C */ ShadowColFlags *flags;
    /* 0x50 */ u8 pad50[0x5C - 0x50];
    /* 0x5C */ ShadowColData *data;
} ShadowColObj;

struct HitObj;
ShadowColObj **collision_objects_get(s32 *count);
s32 func_800203C4(struct HitObj *obj, s16 *out, f32 *planes, f32 x1, f32 z1, f32 x2, f32 z2);
extern s16 D_800F48F0[][10]; // JFG: the model faces under the shadow (three corners each)
extern f32 D_800F52F0[][4];  // JFG: their planes
extern s32 D_800F5AF4;       // JFG: the planes used so far

/**
 * JFG: the shadow on solid object models: the (up to 15) solid objects whose box overlaps 20 units around the
 * shadow's object, nearest first (by the box half-size less the position, as the ROM computes it), then each one's
 * up-facing faces under the shadow (func_800203C4) are clipped to the edges and added as shadow polygons, as
 * DKR's func_8002EEEC does for water. As in DKR's, a few variables carry two values each (inside is also the
 * vertex counter, minY the clipped count, maxY the cached-vertex index, dx the swap temporary): that gives the
 * ROM's frame.
 * NON_MATCHING: a first body written from the ROM, at the ROM's length and frame (424 words, 0x1D8); uopt picks
 * other argument/temporary registers for the loop counter, the flag and the object data (a3/v1/a1 in the ROM).
 */
void func_8001EE1C(ShadowBuild *shadow, ShadowEdge *edges) {
    ShadowColObj **list;
    ShadowColObj *obj;
    ShadowObj *self;
    ShadowPoint points[8];
    f32 dists[16];
    ShadowColData *data;
    s32 inside;
    f32 lo[2];
    f32 hi[2];
    f32 dx;
    f32 dz;
    s32 j;
    s32 minY;
    s32 numFound;
    s32 numTris;
    s32 count;
    s32 t;
    s16 *tri;
    s32 maxY;
    s32 i;
    s32 tempIdx;
    s32 ids[16];

    self = shadow->obj;
    lo[0] = self->x - 20.0f;
    lo[1] = self->z - 20.0f;
    hi[0] = self->x + 20.0f;
    hi[1] = self->z + 20.0f;
    list = collision_objects_get(&count);
    numFound = 0;
    for (i = 0; i < count; i++) {
        obj = list[i];
        if ((obj->flags->flags & 1) && (data = obj->data) != NULL) {
            inside = TRUE;
            if (lo[0] < data->bboxMin[0] && hi[0] < data->bboxMin[0]) {
                inside = FALSE;
            } else if (data->bboxMax[0] < lo[0] && data->bboxMax[0] < hi[0]) {
                inside = FALSE;
            }
            if (lo[1] < data->bboxMin[2] && hi[1] < data->bboxMin[2]) {
                inside = FALSE;
            } else if (data->bboxMax[2] < lo[1] && data->bboxMax[2] < hi[1]) {
                inside = FALSE;
            }
            if (inside) {
                dx = (data->bboxMax[0] - data->bboxMin[0]) / 2.0f - self->x;
                dz = (data->bboxMax[2] - data->bboxMin[2]) / 2.0f - self->z;
                ids[numFound] = i;
                dists[numFound] = dx * dx + dz * dz;
                j = numFound;
                while (j > 0 && dists[j] < dists[j - 1]) {
                    dx = dists[j];
                    dists[j] = dists[j - 1];
                    dists[j - 1] = dx;
                    inside = ids[j];
                    ids[j] = ids[j - 1];
                    ids[j - 1] = inside;
                    j--;
                }
                numFound++;
                if (numFound >= 15) {
                    i = count;
                }
            }
        }
    }
    for (i = 0; i < numFound; i++) {
        obj = list[ids[i]];
        if (self != (ShadowObj *) obj) {
            numTris = func_800203C4((struct HitObj *) obj, D_800F48F0[0], D_800F52F0[D_800F5AF4], shadow->x1,
                                    shadow->z1, shadow->x2, shadow->z2);
            for (t = 0; t < numTris; t++) {
                tri = D_800F48F0[t];
                maxY = minY = tri[1];
                if (tri[4] < minY) {
                    minY = tri[4];
                } else if (maxY < tri[4]) {
                    maxY = tri[4];
                }
                if (tri[7] < minY) {
                    minY = tri[7];
                } else if (maxY < tri[7]) {
                    maxY = tri[7];
                }
                if (shadow->y2 >= minY && maxY >= shadow->y1) {
                    points[0].x = tri[0];
                    points[0].z = tri[2];
                    points[0].unkE = -1;
                    points[1].x = tri[3];
                    points[1].z = tri[5];
                    points[1].unkE = -1;
                    points[2].x = tri[6];
                    points[2].z = tri[8];
                    points[2].unkE = -1;
                    shadow->plane = (f32 *) (D_800F52F0 + D_800F5AF4 + t);
                    if ((D_800F52F0 + D_800F5AF4 + t)[0][1] > 0.5f) {
                        if (shadow->unk1C > 0.0f) {
                            func_800209E4(shadow, points);
                        }
                        minY = func_8001F4BC(shadow, 3, points, 4, edges);
                        if (minY >= 3) {
                            tempIdx = D_800F45E8;
                            D_800F45F0[tempIdx].sharedMask = 0;
                            for (inside = 0; inside < minY; inside++) {
                                if (points[inside].unkE < 0) {
                                    maxY = -1;
                                    j = 0;
                                    while (j < D_800F33D0 && maxY == -1) {
                                        if (D_800F33D8[j].x == points[inside].x && D_800F33D8[j].z == points[inside].z) {
                                            maxY = j;
                                        }
                                        j++;
                                    }
                                    if (maxY == -1) {
                                        if (D_800F33D0 >= 32) {
                                            D_800F33D0 = 31;
                                        }
                                        D_800F33D8[D_800F33D0].x = points[inside].x;
                                        D_800F33D8[D_800F33D0].plane = shadow->plane;
                                        D_800F33D8[D_800F33D0].z = points[inside].z;
                                        D_800F45F0[tempIdx].verts[inside] = D_800F33D0++;
                                    } else {
                                        D_800F45F0[tempIdx].verts[inside] = maxY;
                                    }
                                } else {
                                    D_800F45F0[tempIdx].verts[inside] = points[inside].unkE;
                                    D_800F45F0[tempIdx].sharedMask |= 1 << inside;
                                }
                            }
                            D_800F45F0[tempIdx].numVerts = minY;
                            D_800F45F0[tempIdx].shade = 0;
                            D_800F45E8 = tempIdx + 1;
                        }
                    }
                }
            }
            D_800F5AF4 += numTris;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001EE1C);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: an FP register swap only: dz and the edge offset get f24/f20 (target f20/f24). -dx negated in
 * place (`temp_f14 = -temp_f14;`) puts it in the target's f22 (as the old empty `if (temp_f14) {}` did). */
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
        temp_f14 = edges[var_v1].x - edges[var_v0].x;
        temp_f14 = -temp_f14;
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

/* JFG: an optional override of the shadow's angle and opacity */
typedef struct ShadowParams {
    /* 0x00 */ s16 angle;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 opacity;
} ShadowParams;

typedef struct ShadowFlags {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ u8 flags;
} ShadowFlags;

f32 func_80049C30(s32 angle);          // cosine of a 16-bit angle
f32 func_80049C34(s32 angle);          // sine of a 16-bit angle

#ifdef NON_MATCHING
/* NON_MATCHING: same shape, other registers: the target's FPRs are zero f20, scaleU/scaleV f22/f24, width/length f26/f28, sin f14, cos f16 (mine width/length f22/f24, sin/cos swapped) and its loop values in t-registers (poly t4, base t5) */
/* DKR func_8002F440 (tracks.c): turn the clipped shadow polygons into vertices and triangles in the shadow heaps
 * (JFG: an optional angle/opacity override, a flag for an unrotated shadow, packed s32 texture coordinates and
 * the heap limits checked; returns 0 when a heap is full). */
s32 func_8001FA24(ShadowBuild *shadow, ShadowParams *params, ShadowFlags *arg2) {
    s32 spAC;
    s32 var_t0;
    ShadowHeapTriangle *tri;
    ShadowHeapVertex *vert;
    s32 uv[8];
    s32 alpha;
    s32 var_s2;
    f32 alphaF;
    f32 someY;
    f32 scaleU;
    f32 scaleV;
    f32 width;
    f32 length;
    f32 x;
    f32 y;
    f32 z;
    f32 yRotSin;
    f32 yRotCos;
    f32 objX;
    f32 objZ;
    f32 u;
    f32 v;
    s32 vtxCount;
    s32 triCount;
    s32 tail;
    ShadowHeapMesh *mesh;
    ShadowPoly *poly;
    ShadowObj *obj;
    s32 i;

    if (arg2->flags & 0x20) {
        yRotCos = 1;
        yRotSin = 0.0f;
    } else if (params != NULL) {
        yRotSin = func_80049C34(params->angle);
        yRotCos = func_80049C30(params->angle);
    } else {
        yRotSin = func_80049C34(shadow->obj->yRotation);
        yRotCos = func_80049C30(shadow->obj->yRotation);
    }
    width = shadow->width;
    length = shadow->length;
    alphaF = 255;
    scaleU = (f32) (shadow->texture->width << 4) / width;
    var_s2 = 25;
    scaleV = (f32) (shadow->texture->height << 5) / (length + shadow->unk34);
    if (shadow->unk1C > 0.0f) {
        someY = shadow->obj->y - shadow->groundY;
        if (shadow->unk1C < someY) {
            alphaF = 255.0f * (1.0f - ((someY - shadow->unk1C) / shadow->fadeDistance));
            if (alphaF < 0.0f) {
                alphaF = 0.0f;
            }
        }
        if (someY > 0.0f) {
            someY = (someY / shadow->obj->header->shadowScale) + 1.0f;
            scaleU *= someY;
            width /= someY;
            scaleV *= someY;
            length /= someY;
        }
    }
    obj = shadow->obj;
    alpha = alphaF * D_800F5AF0;
    if (params != NULL) {
        alpha = (s32) (params->opacity * (u32) alpha) >> 8;
    }
    vtxCount = D_800A0EBC;
    triCount = D_800A0EC0;
    vert = &D_800A0EB0[vtxCount];
    tri = &D_800A0EB4[triCount];
    tail = D_800A0EC4;
    mesh = &D_800A0EB8[tail];
    objX = obj->x;
    objZ = obj->z;
    for (spAC = 0; spAC < D_800F45E8; spAC++) {
        poly = &D_800F45F0[spAC];
        if ((poly->numVerts + var_s2) >= 24) {
            mesh->vtxCount = vtxCount;
            mesh->triCount = triCount;
            mesh->texture = shadow->texture;
            mesh++;
            tail++;
            var_s2 = 0;
        }
        if (tail >= D_800F5B08) {
            return 0;
        }
        var_t0 = poly->sharedMask;
        for (i = 0; i < poly->numVerts; i++) {
            if (var_t0 & 1) {
                x = D_800F35E8[poly->verts[i]].x;
                y = D_800F35E8[poly->verts[i]].y;
                z = D_800F35E8[poly->verts[i]].z;
            } else {
                x = D_800F33D8[poly->verts[i]].x;
                y = D_800F33D8[poly->verts[i]].y;
                z = D_800F33D8[poly->verts[i]].z;
            }
            var_t0 >>= 1;
            vtxCount++;
            vert->x = x;
            vert->y = shadow->yOffset + y;
            vert->z = z;
            vert->r = 255;
            vert->g = 255;
            vert->b = 255;
            vert->a = alpha;
            vert++;
            if (vtxCount >= D_800F5B00) {
                return 0;
            }
            x = x - objX;
            z = z - objZ;
            u = (x * yRotCos) - (z * yRotSin);
            v = (z * yRotCos) + (x * yRotSin);
            uv[i] = ((s32) ((v + length) * scaleV) & 0xFFFF) | ((s32) (scaleU * (u + width)) << 16);
        }
        for (i = 1; i < poly->numVerts - 1; i++) {
            tri->flags = 0;
            tri->vi0 = var_s2 + i;
            tri->vi1 = var_s2 + i + 1;
            tri->vi2 = var_s2;
            triCount++;
            tri->uv0 = uv[i];
            tri->uv1 = uv[i + 1];
            tri->uv2 = uv[0];
            tri++;
            if (triCount >= D_800F5B04) {
                return 0;
            }
        }
        var_s2 += poly->numVerts;
    }
    D_800A0EBC = vtxCount;
    D_800A0EC0 = triCount;
    D_800A0EC4 = tail;
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001FA24);
#endif


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
/* NON_MATCHING (111 words, from 266, after the empty ifs on always-set values below): the frame matches; the target keeps the point matrix in s7 for the whole function and the batch
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
        if (inst) {}
    } else {
        normalSet = model->normals;
    }
    normals = normalSet->normals;
    planesOut = planes;
    dst = out;
    count = 0;
    mtx = (MtxF *) ((u8 *) obj->data + (obj->data->matrixIndex << 6));
    pointMtx = mtx + 2;
    i = 0;
    for (;;) {
        if (!(i < model->numberOfBatches)) break;
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
        i++;
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
