/*
 * Runlink module 24: setting up a level's track, JFG's form of Diddy Kong
 * Racing's init_track and generate_track (DKR decomp, src/tracks.c, CC0):
 * allocates the track buffers, loads and relocates the level model, builds each
 * segment's collision data and vertex colour block, and spawns the level.
 */
#include "module.h"

#include "gfx.h"

/* The level header (D_800A0D64, DKR's gCurrentLevelHeader2), reduced */
typedef struct M024LevelHeader {
    /* 0x000 */ u8 pad0[0x68];
    /* 0x068 */ u8 unk68;
    /* 0x069 */ u8 pad69[0x6C - 0x69];
    /* 0x06C */ s8 race_type; /* (DKR) */
    /* 0x06D */ u8 pad6D[0x88 - 0x6D];
    /* 0x088 */ s16 unk88; /* a texture, or -1 */
    /* 0x08A */ u8 pad8A[0xEA - 0x8A];
    /* 0x0EA */ s16 unkEA;
    /* 0x0EC */ s16 maxEdges; /* the edge list's size */
    /* 0x0EE */ u8 padEE[0xF2 - 0xEE];
    /* 0x0F2 */ s8 unkF2;
    /* 0x0F3 */ u8 padF3[0x105 - 0xF3];
    /* 0x105 */ u8 unk105;
    /* 0x106 */ u8 unk106;
} M024LevelHeader;

/* DKR include/structs.h, reduced (JFG's layouts, as in 12D50.c) */
typedef struct TriangleBatchInfo {
    /* 0x00 */ u8 textureIndex;
    /* 0x01 */ u8 pad1[0x6 - 1];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u16 texOffset;
    /* 0x0C */ u32 flags;
} TriangleBatchInfo;

/* DKR include/structs.h: a triangle's collision plane and its edge planes */
typedef struct CollisionFacetPlanes {
    /* 0x00 */ u16 basePlaneIndex;
    /* 0x02 */ u16 edgeBisectorPlane[3];
} CollisionFacetPlanes;

/* A segment's vertex colours: three bytes per vertex, and a bit per vertex */
typedef struct SegmentColours {
    /* 0x00 */ u8 *colours;
    /* 0x04 */ u16 *changed;
} SegmentColours;

/* Size: 0x48 bytes */
typedef struct LevelModelSegment {
    /* 0x00 */ Vertex *vertices;
    /* 0x04 */ Triangle *triangles;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ TriangleBatchInfo *batches;
    /* 0x10 */ u32 *triangleMasks;
    /* 0x14 */ u8 *triangleHeightMasks;
    /* 0x18 */ CollisionFacetPlanes *trianglePlanes;
    /* 0x1C */ f32 *planes;
    /* 0x20 */ u8 *edgeFlags;
    /* 0x24 */ s16 numberOfVertices;
    /* 0x26 */ s16 numberOfTriangles;
    /* 0x28 */ s16 numberOfBatches;
    /* 0x2A */ u8 pad2A[0x32 - 0x2A];
    /* 0x32 */ s8 colourFlags;
    /* 0x33 */ u8 pad33;
    /* 0x34 */ SegmentColours *colours;
    /* 0x38 */ u8 pad38[0x3C - 0x38];
    /* 0x3C */ s16 unk3C;
    /* 0x3E */ s16 unk3E; /* DKR unk32 */
    /* 0x40 */ s16 *unk40; /* DKR unk34 */
    /* 0x44 */ s16 unk44; /* DKR unk38 */
    /* 0x46 */ u8 pad46[2];
} LevelModelSegment;

typedef struct LevelModelSegmentBoundingBox {
    /* 0x00 */ s16 x1;
    /* 0x02 */ s16 y1;
    /* 0x04 */ s16 z1;
    /* 0x06 */ s16 x2;
    /* 0x08 */ s16 y2;
    /* 0x0A */ s16 z2;
} LevelModelSegmentBoundingBox;

typedef struct LevelModelTexture {
    /* 0x00 */ void *texture;
    /* 0x04 */ u8 pad4[4];
} LevelModelTexture;

typedef struct BspTreeNode BspTreeNode;

typedef struct LevelModel {
    /* 0x00 */ LevelModelTexture *textures;
    /* 0x04 */ LevelModelSegment *segments;
    /* 0x08 */ LevelModelSegmentBoundingBox *segmentsBoundingBoxes;
    /* 0x0C */ u8 *unkC;
    /* 0x10 */ u8 *segmentsBitfields;
    /* 0x14 */ BspTreeNode *segmentsBspTree;
    /* 0x18 */ s16 numberOfTextures;
    /* 0x1A */ s16 numberOfSegments;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ s32 modelSize;
} LevelModel;

/* JFG's level model buffer */
#define LEVEL_MODEL_MAX_SIZE 0x9F000

/* DKR tracks.c */
#define LOCAL_OFFSET_TO_RAM_ADDRESS(ptr_type, ptr) ptr = (ptr_type) ((s32) ((u8 *) ptr) + mdl)

extern void *D_800F2FB0;
extern M024LevelHeader *D_800A0D64;
extern s32 D_800F2F9C;
extern s32 D_800F31DC; /* DKR gWaveBlockCount */
extern LevelModel *D_800A0D60; /* DKR gCurrentLevelModel */
extern s32 *D_800F33B8;
extern s16 *D_800F33BC;
extern s16 *D_800F33C0;
extern void *D_800F33B0;
extern s32 D_800F33C4;
extern LevelModel *D_800F31C8; /* DKR gTrackModelHeap */
extern s32 D_800A0D8C;
extern s8 D_80102550;
extern s32 *rl_m024_01990; /* the level models table */

#define gCurrentLevelModel D_800A0D60
#define sLevelModelTable rl_m024_01990

extern void func_80055B34(s32 tag);  /* DKR set_texture_colour_tag */
extern void func_800496AC(void);
extern void check_reset_and_halt(void);
extern s32 *func_8004D37C(s32 id); /* DKR asset_table_load */
extern void func_8004D530(s32 id, s32 dst, s32 offset, s32 size); /* DKR asset_load */
extern void func_80072548(u8 *src, u8 *dst); /* DKR gzip_inflate */
extern void func_8004AA80(void *data); /* DKR mempool_free */
extern s32 func_8004AFE0(s32 address);
extern s32 func_8004AFC4(s32 address);
extern s32 func_m071_00000(void);
extern s32 mempool_free_timer_get(void);
extern void func_8004AA08(s32 timer); /* DKR mempool_free_timer */
extern void func_8004A828(s32 size, void *address, s32 colour); /* DKR mempool_alloc_fixed */
extern void track_segment_data_alloc(s32 arg0, s32 arg1);
static void func_m024_00908(LevelModelSegment *segment, LevelModelSegmentBoundingBox *bbox, s32 arg2);
#ifdef NON_MATCHING
static s32 func_m024_00CE4(LevelModelSegment *block);
static s32 func_m024_016E0(LevelModelSegment *seg, s32 tri, s16 *a, s16 *b);
#define rl_m024_00CE4 func_m024_00CE4
#else
extern s32 rl_m024_00CE4(LevelModelSegment *block); /* still assembly */
#endif
extern f32 func_80097140(f32 x); /* square root */
static void func_m024_018B4(LevelModelSegment *segment);

static void func_m024_001F8(s32 modelId);
extern s32 D_800F2FA0;
extern s32 D_800F2FA4; /* DKR gTrackTexAnimOffset */
extern s32 D_800F2FA8; /* DKR gTrackTexAnimFlags */
extern void *D_800A0D68;
extern s32 D_800A0D6C;
extern s8 D_800A0CE0;

extern void *func_8004A57C(u32 size, s32 colour); /* DKR mempool_alloc_safe */
extern M024LevelHeader *func_8004470C(void);      /* DKR level_header */
extern u8 func_80048548(void);                    /* the number of cameras */
extern void cam_set_layout(s32 layout);
extern void track_spawn_objects(s32 mapID, s32 index); /* DKR track_spawn_objects */
extern void sequence_table_load(s32 arg0);
extern void func_8000561C(s32 arg0);
extern void shadow_buffers_alloc(void *model, s32 waveBlocks, s32 arg2, s32 arg3, s32 arg4);
extern void func_8001DA1C(s32 arg0, s32 arg1); /* DKR shadow_update */
extern void func_8001D9B4(void);
extern void func_8003DDBC(s32 arg0);
extern void func_80012B04(s32 skybox); /* DKR skydome_spawn */
extern void *func_80055694(s32 id);    /* loads a texture */
extern void func_m137_00000(void *model);


/* DKR init_track */
void func_m024_00000(s32 geometry, s32 skybox, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    D_800F2FB0 = func_8004A57C(0x800, 0xFFFF);
    D_800A0D64 = func_8004470C();
    D_800F2F9C = FALSE;
    if (D_800A0D64->race_type == 1 || D_800A0D64->race_type == 2) {
        D_800F2F9C = TRUE;
    }
    func_m024_001F8(geometry);
    D_800F31DC = 0;
    cam_set_layout(func_80048548() - 1);
    track_spawn_objects(arg4, 0);
    track_spawn_objects(arg3, 1);
    sequence_table_load(arg5);
    func_8000561C(arg2);
    D_800F2FA0 = 0;
    shadow_buffers_alloc(D_800A0D60, D_800F31DC, 500, 200, 100);
    func_8001DA1C(2, 0);
    func_8001DA1C(1, 0);
    func_8001D9B4();
    func_8001DA1C(2, 0);
    func_8001DA1C(1, 0);
    func_8001D9B4();
    if (!D_800A0D64->unk68) {
        func_8003DDBC(0);
    }
    func_80012B04(skybox);
    if (!D_800A0D64->unk68) {
        func_8003DDBC(8);
    }
    D_800F2FA4 = 0;
    D_800F2FA8 = 0x10000;
    if (D_800A0D64->unk88 != -1) {
        D_800A0D68 = func_80055694(D_800A0D64->unk88);
        D_800A0D6C = 0;
    }
    D_800A0CE0 = 0;
    if (D_800A0D64->unkF2) {
        func_m137_00000(D_800A0D60);
    }
}

/* DKR generate_track: loads level model modelId and builds its segments' data */
static void func_m024_001F8(s32 modelId) {
    s32 i;
    s32 k;
    s32 temp_s4;
    s32 temp;
    s32 mdl;
    s32 n;
    s32 bitsSize;
    SegmentColours *block;
    u16 *bits;
    u8 *colours;
    Vertex *v;
    LevelModelSegment *seg;

    func_80055B34(0xFF00FF);
    D_800F33B8 = func_8004A57C(D_800A0D64->unkEA * 4, 0xFFFF00FF);
    D_800F33BC = func_8004A57C(D_800A0D64->unkEA * 2, 0xFFFF00FF);
    D_800F33C0 = func_8004A57C(D_800A0D64->unkEA * 2, 0xFFFF00FF);
    D_800F33B0 = func_8004A57C(D_800A0D64->maxEdges * 48, 0xFFFF00FF);
    D_800F33C4 = 0;
    D_800F31C8 = func_8004A57C(LEVEL_MODEL_MAX_SIZE, 0xFFFF00FF);
    gCurrentLevelModel = D_800F31C8;
    if (gCurrentLevelModel == NULL) {
        func_800496AC();
    }
    check_reset_and_halt();
    sLevelModelTable = func_8004D37C(0x24);
    for (i = 0; sLevelModelTable[i] != -1; i++) {}
    i--;
    if (modelId >= i) {
        modelId = 0;
    }
    mdl = sLevelModelTable[modelId];
    temp_s4 = sLevelModelTable[modelId + 1] - mdl;
    temp = (s32) gCurrentLevelModel;
    temp -= temp_s4;
    temp += LEVEL_MODEL_MAX_SIZE;
    temp -= ((s32) temp % 16);
    check_reset_and_halt();
    func_8004D530(0x25, temp, mdl, temp_s4);
    func_80072548((u8 *) temp, (u8 *) gCurrentLevelModel);
    func_8004AA80(sLevelModelTable);
    check_reset_and_halt();

    mdl = (s32) gCurrentLevelModel;
    LOCAL_OFFSET_TO_RAM_ADDRESS(LevelModelTexture *, gCurrentLevelModel->textures);
    LOCAL_OFFSET_TO_RAM_ADDRESS(LevelModelSegment *, gCurrentLevelModel->segments);
    LOCAL_OFFSET_TO_RAM_ADDRESS(LevelModelSegmentBoundingBox *, gCurrentLevelModel->segmentsBoundingBoxes);
    LOCAL_OFFSET_TO_RAM_ADDRESS(u8 *, gCurrentLevelModel->unkC);
    LOCAL_OFFSET_TO_RAM_ADDRESS(u8 *, gCurrentLevelModel->segmentsBitfields);
    LOCAL_OFFSET_TO_RAM_ADDRESS(BspTreeNode *, gCurrentLevelModel->segmentsBspTree);
    for (k = 0; k < gCurrentLevelModel->numberOfSegments; k++) {
        LOCAL_OFFSET_TO_RAM_ADDRESS(Vertex *, gCurrentLevelModel->segments[k].vertices);
        LOCAL_OFFSET_TO_RAM_ADDRESS(Triangle *, gCurrentLevelModel->segments[k].triangles);
        LOCAL_OFFSET_TO_RAM_ADDRESS(TriangleBatchInfo *, gCurrentLevelModel->segments[k].batches);
        LOCAL_OFFSET_TO_RAM_ADDRESS(void *, gCurrentLevelModel->segments[k].trianglePlanes);
    }
    mdl = (s32) gCurrentLevelModel + gCurrentLevelModel->modelSize;
    for (k = 0; k < gCurrentLevelModel->numberOfSegments; k++) {
        mdl = func_8004AFE0(mdl);
        gCurrentLevelModel->segments[k].triangleMasks = (u32 *) mdl;
        mdl += gCurrentLevelModel->segments[k].numberOfTriangles * 4;
        gCurrentLevelModel->segments[k].triangleHeightMasks = (u8 *) mdl;
        mdl += gCurrentLevelModel->segments[k].numberOfTriangles;
        func_m024_00908(&gCurrentLevelModel->segments[k], &gCurrentLevelModel->segmentsBoundingBoxes[k], k);
        gCurrentLevelModel->segments[k].edgeFlags = (u8 *) mdl;
        mdl += gCurrentLevelModel->segments[k].numberOfTriangles;
        mdl = func_8004AFC4(mdl);
        gCurrentLevelModel->segments[k].planes = (void *) mdl;
        mdl += rl_m024_00CE4(&gCurrentLevelModel->segments[k]) * 16;
        mdl = func_8004AFE0(mdl);
        gCurrentLevelModel->segments[k].unk3C = 0;
        gCurrentLevelModel->segments[k].unk40 = (s16 *) mdl;
        func_m024_018B4(&gCurrentLevelModel->segments[k]);
        mdl = gCurrentLevelModel->segments[k].unk3E * 2 + mdl;
    }
    if (!func_m071_00000()) {
        /* the copy protection check failed: scramble the texture IDs */
        for (k = 0; k < gCurrentLevelModel->numberOfTextures; k++) {
            gCurrentLevelModel->textures[k].texture = (void *) k;
        }
    }
    temp_s4 = mdl - (s32) gCurrentLevelModel;
    check_reset_and_halt();
    temp = mempool_free_timer_get();
    func_8004AA08(0);
    func_8004AA80(D_800F31C8);
    func_8004A828(temp_s4, D_800F31C8, 0xFFFF00FF);
    func_8004AA08(temp);
    check_reset_and_halt();
    for (k = 0; k < gCurrentLevelModel->numberOfTextures; k++) {
        gCurrentLevelModel->textures[k].texture =
            func_80055694(((s32) gCurrentLevelModel->textures[k].texture) | 0x8000);
    }
    check_reset_and_halt();
    seg = gCurrentLevelModel->segments;
    for (k = 0; k < gCurrentLevelModel->numberOfSegments; k++, seg++) {
        bitsSize = ((seg->numberOfVertices + 15) >> 4) * 2;
        block = func_8004A57C(sizeof(SegmentColours) + bitsSize + seg->numberOfVertices * 3, 0x7F7F7FFF);
        if (block != NULL) {
            seg->colours = block;
            bits = (u16 *) ((u8 *) block + 8);
            colours = (u8 *) block + bitsSize + 8;
            block->changed = bits;
            block->colours = colours;
            n = seg->numberOfVertices;
            v = seg->vertices;
            while (n--) {
                *colours++ = v->r;
                *colours++ = v->g;
                *colours++ = v->b;
                v++;
            }
            n = bitsSize >> 1;
            while (n--) {
                *bits++ = 0;
            }
            seg->colourFlags = 0;
        }
    }
    check_reset_and_halt();
    func_80055B34(0xFF00FFFF);
    D_800A0D8C = 0;
    track_segment_data_alloc(D_800A0D64->unk105, D_800A0D64->unk106);
    D_80102550 = 0;
}

/*
 * DKR func_8002C954, JFG's form: for each triangle, the cells of a 16x16 grid over
 * the segment's bounding box (x, then z) that it covers, and of 8 height bands.
 */
static void func_m024_00908(LevelModelSegment *segment, LevelModelSegmentBoundingBox *bbox, UNUSED s32 arg2) {
    Vertex *vert;
    s32 vertZ;
    s32 vertX;
    s32 endTri;
    s32 bit;
    s32 i;
    s32 j;
    s32 val;
    s32 l;
    u8 *tri;
    s16 boxDelta;
    s16 boxMin;
    s16 maxX;
    s16 minX;
    s16 maxY;
    s16 minY;
    s16 maxZ;
    s16 minZ;
    s16 k;
    s16 boxMax;
    s16 vertsOffset;
    s16 startTri;
    s16 vertY;
    u8 heightVal;

    for (i = 0; i < segment->numberOfBatches; i++) {
        startTri = segment->batches[i].facesOffset;
        vertsOffset = segment->batches[i].verticesOffset;
        endTri = segment->batches[i + 1].facesOffset;
        for (j = startTri; j < endTri; j++) {
            if (segment->triangles[j].flags & 0x80) {
                segment->triangleMasks[j] = 0;
                segment->triangleHeightMasks[j] = 0;
            } else {
                maxX = -32700;
                maxY = -32700;
                maxZ = -32700;
                minX = 32700;
                minY = 32700;
                minZ = 32700;
                for (l = 0, tri = (u8 *) &segment->triangles[j]; l < 3; l++, tri++) {
                    vert = &segment->vertices[tri[1] + vertsOffset];
                    k = vert->x;
                    vertX = k;
                    vertY = vert->y;
                    vertZ = vert->z;
                    if (maxX < vertX) {
                        maxX = vertX;
                    }
                    if (vertX < minX) {
                        minX = vertX;
                    }
                    if (maxY < vertY) {
                        maxY = vertY;
                    }
                    if (vertY < minY) {
                        minY = vertY;
                    }
                    if (maxZ < vertZ) {
                        maxZ = vertZ;
                    }
                    if (vertZ < minZ) {
                        minZ = vertZ;
                    }
                }
                val = 0;
                boxDelta = ((bbox->x2 - bbox->x1) >> 4) + 1;
                bit = 1;
                boxMax = boxDelta + bbox->x1;
                boxMin = bbox->x1;
                for (k = 0; k < 16; k++) {
                    if (boxMax >= minX && maxX >= boxMin) {
                        val |= bit;
                    }
                    boxMax += boxDelta;
                    boxMin += boxDelta;
                    bit <<= 1;
                }
                boxDelta = ((bbox->z2 - bbox->z1) >> 4) + 1;
                boxMax = boxDelta + bbox->z1;
                boxMin = bbox->z1;
                for (k = 0; k < 16; k++) {
                    if (boxMax >= minZ && maxZ >= boxMin) {
                        val |= bit;
                    }
                    boxMax += boxDelta;
                    boxMin += boxDelta;
                    bit <<= 1;
                }
                segment->triangleMasks[j] = val;
                heightVal = 0;
                boxDelta = ((bbox->y2 - bbox->y1) >> 3) + 1;
                bit = 1;
                boxMax = boxDelta + bbox->y1;
                boxMin = bbox->y1;
                for (k = 0; k < 8; k++) {
                    if (boxMax >= minY && maxY >= boxMin) {
                        heightVal |= bit;
                    }
                    boxMax += boxDelta;
                    boxMin += boxDelta;
                    bit <<= 1;
                }
                segment->triangleHeightMasks[j] = heightVal;
            }
        }
    }
}

/*
 * DKR track_init_collision, JFG's form: each triangle's plane, then a plane along each
 * edge shared with another triangle, the edge flags and the triangles' edge bits.
 * Returns the number of planes.
 */
#ifdef NON_MATCHING
/* register allocation (FPRs and the loop registers) and the frame */
static s32 func_m024_00CE4(LevelModelSegment *block) {
    s32 facesOffset;
    f32 *plane;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 x3;
    f32 y3;
    f32 z3;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 mag;
    f32 h;
    f32 x5;
    f32 y5;
    f32 z5;
    s32 triIndex;
    s32 nextFacesOffset;
    s32 verticesOffset;
    s32 batchIndex;
    s32 i;
    s32 next;
    s32 next2;
    s32 vertIndex;
    s32 thirdVertIndex;
    s32 nextVertIndex;
    s32 edge;
    s32 numColPlanes;
    s32 counter;
    s32 colPlaneIndex;
    s32 j;
    s32 flags;
    Vertex *v;
    f32 *edgePlane;

    counter = 0;
    for (batchIndex = 0; batchIndex < block->numberOfBatches; batchIndex++) {
        facesOffset = block->batches[batchIndex].facesOffset;
        nextFacesOffset = block->batches[batchIndex + 1].facesOffset;
        verticesOffset = block->batches[batchIndex].verticesOffset;
        for (triIndex = facesOffset; triIndex < nextFacesOffset; triIndex++) {
            if (block->triangles[triIndex].flags & 0x80) {
                continue;
            }
            v = &block->vertices[block->triangles[triIndex].vi0 + verticesOffset];
            x1 = v->x;
            y1 = v->y;
            z1 = v->z;
            v = &block->vertices[block->triangles[triIndex].vi1 + verticesOffset];
            x2 = v->x;
            y2 = v->y;
            z2 = v->z;
            v = &block->vertices[block->triangles[triIndex].vi2 + verticesOffset];
            x3 = v->x;
            y3 = v->y;
            z3 = v->z;
            nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
            ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
            nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
            mag = func_80097140(nx * nx + ny * ny + nz * nz);
            if (mag > 0.0f) {
                nx /= mag;
                ny /= mag;
                nz /= mag;
            }
            block->edgeFlags[triIndex] = 0;
            {
                s32 temp = counter * 4;
                counter++;
                block->planes[temp + 0] = nx;
                block->planes[temp + 1] = ny;
                block->planes[temp + 2] = nz;
                block->planes[temp + 3] = -(x1 * nx + y1 * ny + z1 * nz);
            }
        }
    }

    if (D_800F2F9C) {
        return counter;
    }
    numColPlanes = counter;

    for (batchIndex = 0; batchIndex < block->numberOfBatches; batchIndex++) {
        verticesOffset = block->batches[batchIndex].verticesOffset;
        facesOffset = block->batches[batchIndex].facesOffset;
        nextFacesOffset = block->batches[batchIndex + 1].facesOffset;
        if (block->batches[batchIndex].flags & 0x880) {
            facesOffset = nextFacesOffset;
        }
        for (triIndex = facesOffset; triIndex < nextFacesOffset; triIndex++) {
            if (block->triangles[triIndex].flags & 0x80) {
                continue;
            }
            colPlaneIndex = block->trianglePlanes[triIndex].basePlaneIndex;
            nx = block->planes[4 * colPlaneIndex + 0];
            ny = block->planes[4 * colPlaneIndex + 1];
            nz = block->planes[4 * colPlaneIndex + 2];
            plane = &block->planes[4 * colPlaneIndex];
            for (i = 0; i < 3; i++) {
                next = i + 1;
                if (next >= 3) {
                    next = 0;
                }
                next2 = next + 1;
                if (next2 >= 3) {
                    next2 = 0;
                }
                vertIndex = (&block->triangles[triIndex].vi0)[i] + verticesOffset;
                nextVertIndex = (&block->triangles[triIndex].vi0)[next] + verticesOffset;
                thirdVertIndex = (&block->triangles[triIndex].vi0)[next2] + verticesOffset;
                edge = block->trianglePlanes[triIndex].edgeBisectorPlane[i];
                if (edge < numColPlanes) {
                    flags = block->triangles[triIndex].flags;
                    if ((flags & 1) && (block->triangles[edge].flags & 2)) {
                        j = func_m024_016E0(block, edge, &block->vertices[vertIndex].x,
                                            &block->vertices[nextVertIndex].x);
                        block->triangles[triIndex].flags &= ~1;
                        block->edgeFlags[edge] |= 1 << (j + 3);
                    } else if ((flags & 2) && (block->triangles[edge].flags & 1)) {
                        block->triangles[edge].flags &= ~1;
                        block->edgeFlags[triIndex] |= 1 << (i + 3);
                    }
                    edgePlane = &block->planes[4 * edge];
                    x5 = block->planes[4 * edge + 0] + nx;
                    y5 = block->planes[4 * edge + 1] + ny;
                    z5 = block->planes[4 * edge + 2] + nz;
                    v = &block->vertices[vertIndex];
                    x1 = v->x;
                    y1 = v->y;
                    z1 = v->z;
                    v = &block->vertices[nextVertIndex];
                    x2 = v->x;
                    y2 = v->y;
                    z2 = v->z;
                    x3 = x5 * 5.0f + x1;
                    y3 = y5 * 5.0f + y1;
                    z3 = z5 * 5.0f + z1;
                    x5 = (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
                    y5 = (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1);
                    z5 = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
                    mag = func_80097140(x5 * x5 + y5 * y5 + z5 * z5);
                    if (mag > 0.0f) {
                        x5 /= mag;
                        y5 /= mag;
                        z5 /= mag;
                    }
                    if (edge != colPlaneIndex) {
                        for (j = 0; j < 3; j++) {
                            if (colPlaneIndex == block->trianglePlanes[edge].edgeBisectorPlane[j]) {
                                block->trianglePlanes[edge].edgeBisectorPlane[j] = counter | 0x8000;
                            }
                        }
                        v = &block->vertices[thirdVertIndex];
                        if (v->x * edgePlane[0] + v->y * edgePlane[1] + v->z * edgePlane[2] + edgePlane[3] < 0.0f) {
                            block->edgeFlags[triIndex] |= 1 << i;
                        }
                        if (plane[0] * edgePlane[0] + plane[1] * edgePlane[1] + edgePlane[2] * plane[2] < 0.0f) {
                            h = y2 - y1;
                            if (y2 < y1) {
                                h = -h;
                            }
                            if (h <= func_80097140((x2 - x1) * (x2 - x1) + (z2 - z1) * (z2 - z1))) {
                                block->triangles[triIndex].flags |= (1 << i) << 2;
                            }
                        }
                    } else {
                        block->edgeFlags[triIndex] |= 1 << i;
                    }
                    block->trianglePlanes[triIndex].edgeBisectorPlane[i] = counter;
                    block->planes[4 * counter + 0] = x5;
                    block->planes[4 * counter + 1] = y5;
                    block->planes[4 * counter + 2] = z5;
                    block->planes[4 * counter + 3] = -(x1 * x5 + y1 * y5 + z1 * z5);
                    counter++;
                }
            }
        }
    }
    return counter;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m024", func_m024_00CE4);
#endif

/* Which edge (0-2) of triangle `tri` of the segment runs between points a and b (either way), or -1 */
#ifdef NON_MATCHING
/* two of the compiler's stack temporaries sit two words lower than the ROM's */
static s32 func_m024_016E0(LevelModelSegment *seg, s32 tri, s16 *a, s16 *b) {
    s32 i;
    s32 j;
    s32 k;
    s32 edge;
    s16 ax;
    s16 ay;
    s16 az;
    s16 bx;
    s16 by;
    s16 bz;
    s16 x0;
    s16 y0;
    s16 z0;
    s16 x1;
    s16 y1;
    s16 z1;
    s32 vOffset;
    s32 start;
    s32 end;
    Vertex *verts;
    Triangle *t;
    Vertex *v;

    ax = a[0];
    ay = a[1];
    az = a[2];
    bx = b[0];
    by = b[1];
    bz = b[2];
    edge = -1;
    for (i = 0; i < seg->numberOfBatches; i++) {
        start = seg->batches[i].facesOffset;
        end = seg->batches[i + 1].facesOffset;
        if (tri >= start && tri < end) {
            j = 0;
            vOffset = seg->batches[i].verticesOffset;
            t = &seg->triangles[tri];
            verts = seg->vertices;
            for (; j < 3; j++) {
                k = j + 1;
                if (k >= 3) {
                    k = 0;
                }
                v = &verts[(&t->vi0)[j] + vOffset];
                x0 = v->x;
                y0 = v->y;
                z0 = v->z;
                v = &verts[(&t->vi0)[k] + vOffset];
                x1 = v->x;
                y1 = v->y;
                z1 = v->z;
                if ((ax == x0 && ay == y0 && az == z0 && bx == x1 && by == y1 && bz == z1) ||
                    (ax == x1 && ay == y1 && az == z1 && bx == x0 && by == y0 && bz == z0)) {
                    edge = j;
                    j = 3;
                }
            }
        }
    }
    return edge;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m024", func_m024_016E0);
#endif

/* DKR func_8002C71C: lists the segment's batches with flag 0x2000 and finds their highest first vertex */
static void func_m024_018B4(LevelModelSegment *segment) {
    s32 curVertY;
    s32 numVerts;
    s32 i;

    segment->unk44 = -10000;
    numVerts = 0;
    for (i = 0; i < segment->numberOfBatches; i++) {
        if (segment->batches[i].flags & 0x2000) {
            segment->unk40[numVerts++] = i;
            curVertY = segment->vertices[segment->batches[i].verticesOffset].y;
            if (segment->unk44 == -10000 || segment->unk44 < curVertY) {
                segment->unk44 = curVertY;
            }
        }
    }
    segment->unk3E = numVerts;
}

INCLUDE_ASM("asm/nonmatchings/modules/m024", data);

INCLUDE_ASM("asm/nonmatchings/modules/m024", bss);
