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
    /* 0x18 */ u8 pad18[0x30 - 0x18];
    /* 0x30 */ f32 unk30; // JFG: an offset added to the position (the skydome's)
    /* 0x34 */ f32 unk34;
    /* 0x38 */ f32 unk38;
    /* 0x3C */ u8 pad3C[0x3E - 0x3C];
    /* 0x3E */ s16 cameraSegmentID;
    /* 0x40 */ u8 pad40[0x4C - 0x40];
} Camera;

/* DKR include/structs.h, reduced (JFG: size 0x10) */
typedef struct TriangleBatchInfo {
    /* 0x00 */ u8 textureIndex;
    /* 0x01 */ u8 pad1[0x6 - 1];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u16 texOffset;
    /* 0x0C */ u32 flags;
} TriangleBatchInfo;

/* DKR include/structs.h */
typedef struct Vertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} Vertex;

/* DKR include/structs.h (Size 0x10) */
typedef struct Triangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vi0;
    /* 0x02 */ u8 vi1;
    /* 0x03 */ u8 vi2;
    /* 0x04 */ s16 uv[3][2];
} Triangle;

/* JFG: a segment's vertex colours (func_8001400C) */
typedef struct SegmentColours {
    /* 0x00 */ u8 *colours; // three bytes per vertex
    /* 0x04 */ u16 *changed; // a bit per vertex, 16 per halfword
} SegmentColours;

/* JFG: a level segment, reduced. Size: 0x48 bytes */
typedef struct LevelModelSegment {
    /* 0x00 */ Vertex *vertices;
    /* 0x04 */ Triangle *triangles;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ TriangleBatchInfo *batches;
    /* 0x10 */ u32 *triangleMasks; // JFG: per triangle, the bounding box cells it covers (see func_8001D344)
    /* 0x14 */ u8 *triangleHeightMasks; // JFG: per triangle, the height cells it covers (see func_8001ACE0)
    /* 0x18 */ struct TrianglePlaneRef *trianglePlanes;
    /* 0x1C */ struct Plane *planes;
    /* 0x20 */ u8 *edgeFlags; // JFG: per triangle, which of its edges go into the edge list (track_edges_build)
    /* 0x24 */ s16 numberOfVertices;
    /* 0x26 */ u8 pad26[0x28 - 0x26];
    /* 0x28 */ s16 numberOfBatches;
    /* 0x2A */ u8 pad2A[0x30 - 0x2A];
    /* 0x30 */ s16 unk30; // DKR: unk28
    /* 0x32 */ s8 colourFlags;
    /* 0x33 */ u8 pad33;
    /* 0x34 */ SegmentColours *colours;
    /* 0x38 */ u8 pad38[0x46 - 0x38];
    /* 0x46 */ u8 numberOfOpaqueBatches; // DKR: numberofOpaqueBatches
    /* 0x47 */ u8 unk47;                 // JFG: the end of the second batch group (see func_80014EBC)
} LevelModelSegment;

/* DKR include/structs.h, reduced */
typedef struct TextureHeader {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0x6 - 2];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 pad8[0x12 - 8];
    /* 0x12 */ u16 numOfTextures;
    /* 0x14 */ u16 frameAdvanceDelay;
} TextureHeader;

typedef struct LevelModelTexture {
    /* 0x00 */ TextureHeader *texture;
    /* 0x04 */ u8 pad4[4];
} LevelModelTexture;

typedef struct Vec3i {
    s32 x, y, z;
} Vec3i;

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
    /* 0x00 */ LevelModelTexture *textures;
    /* 0x04 */ LevelModelSegment *segments;
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
s32 func_80054530();           // a module function (patched in by the runtime linker)

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

extern Gfx *D_800F2F70; // DKR: gTrackDL
extern s32 D_801008B4;  // JFG: the screen row the effect below starts at (0: all of it)
Camera *cam_get_active_camera(void);
s32 cam_get_active_camera_id(); // also called with two arguments it ignores (below)
u8 func_8003FC4C(s32 index);
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
s32 track_height_at(f32 x, f32 z, f32 *y, s32 flags);
void fb_size(u32 *width, u32 *height);
s32 func_80041E58(f32 x, f32 y, f32 z, f32 *outX, f32 *outY, u8 toView);
void func_8006F084(Gfx **dList, s32 arg1, s32 top, s32 width, s32 height, s32 arg5);

/**
 * JFG: if the ground found (flag 0x2000) 10 units behind the camera is above its height - 16, draw the screen
 * effect (func_8006F084) from the row where a point 10 units ahead, at that height difference, projects to (or
 * all of the screen once the camera is 10 or more units below it); the row goes to D_801008B4.
 */
void ground_screen_effect_update(s32 arg0) {
    Camera *cam;
    u32 width;
    u32 height;
    f32 x;
    f32 y;
    f32 z;
    f32 screenX;
    f32 screenY;
    f32 s;
    f32 c;
    s32 top;

    cam = cam_get_active_camera();
    if (func_8003FC4C(cam_get_active_camera_id())) {
        x = cam->trans.x_position - func_80049C34(cam->trans.rotation[0] + 0x8000) * 10.0f;
        z = cam->trans.z_position - func_80049C30(cam->trans.rotation[0] + 0x8000) * 10.0f;
        if (track_height_at(x, z, &y, 0x2000) & 0x2000) {
            if (cam->trans.y_position - 16.0f < y) {
                fb_size(&width, &height);
                s = func_80049C34(cam->trans.rotation[1]);
                c = func_80049C30(cam->trans.rotation[1]);
                y -= cam->trans.y_position;
                if (y >= 10.0f) {
                    func_8006F084(&D_800F2F70, 0, 0, width, height, arg0);
                    D_801008B4 = 0;
                } else {
                    z = y * s - 10.0f * c;
                    y = y * c + 10.0f * s;
                    if (z > 0.0f) {
                        z = -z;
                    }
                    if (func_80041E58(0.0f, y, z, &screenX, &screenY, 0)) {
                        top = screenY;
                        if (top < (s32) height) {
                            if (top < 0) {
                                top = 0;
                            }
                            func_8006F084(&D_800F2F70, 0, top, width, height, arg0);
                            D_801008B4 = top;
                        }
                    }
                }
            }
        }
    }
}

void tex_animate_texture(TextureHeader *texture, u32 *triangleBatchInfoFlags, s32 *arg2, s32 updateRate);

/**
 * Handle the flipbook effect for level geometry textures.
 * JFG: the offset is passed as it is, with no misc data.
 * DKR: track_tex_anim
 */
void track_tex_anim(s32 updateRate) {
    s32 segmentNumber, batchNumber;
    LevelModelSegment *segment;
    TextureHeader *texture;
    TriangleBatchInfo *batch;
    s32 temp;

    segment = gCurrentLevelModel->segments;
    for (segmentNumber = 0; segmentNumber < gCurrentLevelModel->numberOfSegments; segmentNumber++) {
        batch = segment[segmentNumber].batches;
        for (batchNumber = 0; batchNumber < segment[segmentNumber].numberOfBatches; batchNumber++) {
            if (batch[batchNumber].flags & 0x10000) {
                if (batch[batchNumber].textureIndex != 0xFF) {
                    texture = gCurrentLevelModel->textures[batch[batchNumber].textureIndex].texture;
                    if (texture->numOfTextures != 0x100 && texture->frameAdvanceDelay) {
                        temp = batch[batchNumber].texOffset;
                        tex_animate_texture(texture, &batch[batchNumber].flags, &temp, updateRate);
                        batch[batchNumber].texOffset = temp;
                    }
                }
            }
        }
    }
}

/* DKR include/structs.h Object, reduced (JFG offsets) */
typedef struct Object_Racer {
    /* 0x000 */ s8 playerIndex;
    /* 0x001 */ u8 pad1[0x19C - 1];
    /* 0x19C */ s8 unk19C;
    /* 0x19D */ u8 pad19D[0x564 - 0x19D];
    /* 0x564 */ struct RacerUnk564 *unk564;
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x5C0 - 0x569];
    /* 0x5C0 */ s32 unk5C0;
} Object_Racer;

typedef struct Object {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x39 - 0x18];
    /* 0x39 */ u8 opacity;
    /* 0x3A */ u8 pad3A[0x3C - 0x3A];
    /* 0x3C */ void *level_entry;
    /* 0x40 */ struct ObjectHeader12 *header;
    /* 0x44 */ u8 pad44[0x4A - 0x44];
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

/* JFG: the game's own vertex load (include/modules/gfx.h gSPVertexJFG), given the index of the
   last vertex */
#define gSPVertexLast(pkt, v, last, v0) \
    gDma1p(pkt, G_VTX, v, ((last) << 3) + ((last) << 1) + 8, ((last) << 3) | ((u32) (v) & 6) | (v0))

/* DKR include/f3ddkr.h */
#define gSPPolygon(dl, ptr, numTris, texEnabled)                                                                   \
    {                                                                                                              \
        Gfx *_g = (Gfx *) (dl);                                                                                    \
        _g->words.w0 =                                                                                             \
            _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(5, 24, 8) | _SHIFTL(((numTris) * 16), 0, 16); \
        _g->words.w1 = (unsigned int) (ptr);                                                                       \
    }

/* JFG: the level header, reduced */
typedef struct LevelHeader12 {
    /* 0x00 */ u8 pad0[0x69];
    /* 0x69 */ s8 skydomeFixed; // JFG: the skydome stays at the camera offset, unrotated (DKR: skyDome)
    /* 0x6A */ u8 pad6A[0xCE - 0x6A];
    /* 0xCE */ u8 bgBottomR;
    /* 0xCF */ u8 bgBottomG;
    /* 0xD0 */ u8 bgBottomB;
    /* 0xD1 */ u8 bgTopR;
    /* 0xD2 */ u8 bgTopG;
    /* 0xD3 */ u8 bgTopB;
    /* 0xD4 */ u8 padD4[0xE1 - 0xD4];
    /* 0xE1 */ u8 skydomeSpin; // JFG: the skydome's rotation per frame
    /* 0xE2 */ u8 padE2[0xEC - 0xE2];
    /* 0xEC */ s16 maxEdges; // JFG: the edge list's size (track_edges_build)
} LevelHeader12;

/* JFG: an object's header, reduced */
typedef struct ObjectHeader12 {
    /* 0x00 */ u8 pad0[0xA2];
    /* 0xA2 */ u8 unkA2;
} ObjectHeader12;

extern Gfx *D_800F2F70;    // DKR: gTrackDL
extern Mtx *D_800F2F74;    // DKR: gTrackMtxPtr
extern Vertex *D_800F2F78; // DKR: gTrackVtxPtr
extern LevelHeader12 *D_800A0D64;
extern u8 D_A0DA8[];       // the background's two triangles
void mtx_ortho(Gfx **dList, Mtx **mtx);
void rendermode_reset(Gfx **dList);
void func_80055C00(Gfx **dList, void *tex, s32 flags, s32 texOffset); // DKR: material_set
void func_8004046C(Gfx **dList, Mtx **mtx);
void fb_size(u32 *width, u32 *height);
s32 get_current_viewport(void);
s32 cam_get_active_camera_id();
void copy_framebuffer_size_to_coords(s32 layout, s32 viewPortIndex, u32 *ulx, u32 *uly, u32 *lrx, u32 *lry);

#ifdef NON_MATCHING
/* JFG: draw the level's background as a gradient from its top colour to its bottom colour.
   NON_MATCHING: 23 words: the t-registers of the vertex coordinates are numbered one lower. */
void func_80013454(void) {
    u32 width;
    u32 height;
    u32 x1;
    u32 y1;
    u32 x2;
    u32 y2;
    u8 topR;
    u8 topG;
    u8 topB;
    u8 bottomR;
    u8 bottomG;
    u8 bottomB;
    Vertex *verts;

    verts = D_800F2F78;
    D_800F2FAC = -1;
    mtx_ortho(&D_800F2F70, &D_800F2F74);
    rendermode_reset(&D_800F2F70);
    func_80055C00(&D_800F2F70, NULL, 8, 0);
    gSPVertexLast(D_800F2F70++, OS_K0_TO_PHYSICAL(verts), 4, 0);
    gSPPolygon(D_800F2F70++, D_A0DA8, 2, 0);
    func_8004046C(&D_800F2F70, &D_800F2F74);
    topR = D_800A0D64->bgTopR;
    topG = D_800A0D64->bgTopG;
    topB = D_800A0D64->bgTopB;
    bottomR = D_800A0D64->bgBottomR;
    bottomG = D_800A0D64->bgBottomG;
    bottomB = D_800A0D64->bgBottomB;
    fb_size(&width, &height);
    copy_framebuffer_size_to_coords(get_current_viewport(), cam_get_active_camera_id(), &x1, &y1, &x2, &y2);
    width >>= 1;
    height >>= 1;
    verts->x = x1 - width;
    verts->y = height - y2;
    verts->z = 16;
    verts->r = topR;
    verts->a = 255;
    verts->g = topG;
    verts->b = topB;
    verts++;
    verts->x = x2 - width;
    verts->y = height - y2;
    verts->z = 16;
    verts->r = topR;
    verts->a = 255;
    verts->g = topG;
    verts->b = topB;
    verts++;
    verts->x = x1 - width;
    verts->y = height - y1;
    verts->z = 16;
    verts->r = bottomR;
    verts->a = 255;
    verts->b = bottomB;
    verts->g = bottomG;
    verts++;
    verts->x = x2 - width;
    verts->y = height - y1;
    verts->z = 16;
    verts->a = 255;
    verts->r = bottomR;
    verts->b = bottomB;
    verts->g = bottomG;
    verts++;
    D_800F2F78 = verts;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80013454);
#endif

Camera *cam_get_active_camera(void);
void func_80041168(Gfx **dList, Mtx **matrices); // DKR: mtx_world_origin
void func_8000DF00(Object *obj, s32 updateRate);
void render_object(Gfx **dList, Mtx **matrices, Vertex **verts, Object *obj);

/**
 * Moves the skydome to the camera, spins it, then renders it if set to be visible.
 * JFG: the camera's offset, a spin and a texture scroll unless the level fixes the skydome.
 * DKR: skydome_render
 */
void skydome_render(s32 updateRate) {
    Camera *cam;

    if (gSkydomeSegment == NULL) {
        return;
    }
    cam = cam_get_active_camera();
    if (D_800A0D64->skydomeFixed == 0) {
        gSkydomeSegment->trans.x_position = cam->trans.x_position + cam->unk30;
        gSkydomeSegment->trans.y_position = cam->trans.y_position + cam->unk34;
        gSkydomeSegment->trans.z_position = cam->trans.z_position + cam->unk38;
        gSkydomeSegment->trans.rotation[0] += D_800A0D64->skydomeSpin * updateRate;
        if (gSkydomeSegment->header->unkA2 != 0xFF) {
            func_8000DF00(gSkydomeSegment, updateRate);
        }
    } else {
        gSkydomeSegment->trans.x_position = cam->unk30;
        gSkydomeSegment->trans.y_position = cam->unk34;
        gSkydomeSegment->trans.z_position = cam->unk38;
    }
    func_80041168(&D_800F2F70, &D_800F2F74);
    if (gSceneRenderSkyDome) {
        render_object(&D_800F2F70, &D_800F2F74, &D_800F2F78, gSkydomeSegment);
    }
}

/* Returns the skydome object (0x80013810). */
Object *get_skydome_object(void) {
    return gSkydomeSegment;
}

Camera *cam_get_active_camera(void);
void compute_scene_camera_transform_matrix(void);
void update_envmap_position(f32 x, f32 y, f32 z);
void func_8001448C(s32 arg0, s32 arg1);
extern Vec3i D_800F3360; // DKR: gScenePerspectivePos
extern f32 D_800F31CC;   // DKR: gPrevCameraX
extern f32 D_800F31D0;   // DKR: gPrevCameraY
extern f32 D_800F31D4;   // DKR: gPrevCameraZ

/**
 * Sets up the variables for the player's view.
 * JFG: no waves, viewport or level header; ends with a call to func_8001448C.
 * DKR: initialise_player_viewport_vars
 */
void initialise_player_viewport_vars(s32 arg0, s32 arg1) {
    s32 segmentIndex;

    gSceneActiveCamera = cam_get_active_camera();
    compute_scene_camera_transform_matrix();
    update_envmap_position(D_800F3360.x / 65536.0f, D_800F3360.y / 65536.0f, D_800F3360.z / 65536.0f);
    segmentIndex = gSceneActiveCamera->cameraSegmentID;
    if (segmentIndex > -1 && (segmentIndex < gCurrentLevelModel->numberOfSegments)) {
        gSceneStartSegment = gCurrentLevelModel->segments[segmentIndex].unk30;
    } else {
        gSceneStartSegment = -1;
    }
    D_800F31CC = gSceneActiveCamera->trans.x_position;
    D_800F31D0 = gSceneActiveCamera->trans.y_position;
    D_800F31D4 = gSceneActiveCamera->trans.z_position;
    func_8001448C(arg0, arg1);
}

extern s32 D_800A0D8C;
extern s32 D_800F3370[16];

/* JFG: queue a texture scroll (up to 16 a frame). */
void func_80013924(s32 arg0, s32 arg1, s32 arg2) {
    if (D_800A0D8C < 16) {
        D_800F3370[D_800A0D8C] = (arg0 << 24) | ((arg1 & 0xFFF) << 12) | (arg2 & 0xFFF);
        D_800A0D8C++;
    }
}

/* JFG: an entry per segment of D_800A0D80 */
typedef struct SegmentAlloc {
    /* 0x00 */ Vertex *vertices;
    /* 0x04 */ void *data; // a copy of the vertices
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
extern s32 D_800A0D7C;
void mempool_free(void *data);
void *mempool_alloc_safe(s32 size, u32 colourTag);
void track_segment_data_free(void);

#ifdef NON_MATCHING
/* JFG: scroll the texture coordinates of the triangles using each queued texture (func_80013924).
   NON_MATCHING: the same code with other registers (target: outer values in t-registers, the
   per-triangle values in s3..fp, no $ra use). */
void func_80013970(void) {
    s32 count;
    s32 *entry;
    s32 value;
    s32 texIndex;
    s32 du, dv;
    s32 maskU, maskV;
    TextureHeader *tex;
    LevelModelSegment *segment;
    s32 segCount;
    TriangleBatchInfo *batch;
    s32 batchCount;
    Triangle *tri;
    s32 triCount;
    s32 u, v;
    s32 du1, dv1, du2, dv2;

    if (gCurrentLevelModel != NULL) {
        entry = D_800F3370;
        if (D_800A0D8C != 0) {
            count = D_800A0D8C;
            while (count--) {
                value = *entry++;
                texIndex = (value >> 24) & 0xFF;
                du = (value << 8) >> 20;
                dv = (value << 20) >> 20;
                tex = gCurrentLevelModel->textures[texIndex].texture;
                segment = gCurrentLevelModel->segments;
                segCount = gCurrentLevelModel->numberOfSegments;
                if (tex->width < 0x41 && tex->height < 0x41) {
                    maskU = (tex->width << 8) - 1;
                    maskV = (tex->height << 8) - 1;
                } else {
                    maskU = (tex->width << 6) - 1;
                    maskV = (tex->height << 6) - 1;
                }
                while (segCount--) {
                    batchCount = segment->numberOfBatches;
                    batch = segment->batches;
                    while (batchCount--) {
                        if (texIndex == batch->textureIndex) {
                            triCount = batch[1].facesOffset - batch[0].facesOffset;
                            tri = &segment->triangles[batch->facesOffset];
                            while (triCount--) {
                                u = tri->uv[0][0];
                                v = tri->uv[0][1];
                                du1 = tri->uv[1][0] - u;
                                dv1 = tri->uv[1][1] - v;
                                du2 = tri->uv[2][0] - u;
                                dv2 = tri->uv[2][1] - v;
                                u = (u + du) & maskU;
                                v = (v + dv) & maskV;
                                tri->uv[0][0] = u;
                                tri->uv[0][1] = v;
                                tri->uv[1][0] = u + du1;
                                tri->uv[1][1] = v + dv1;
                                tri->uv[2][0] = u + du2;
                                tri->uv[2][1] = v + dv2;
                                tri++;
                            }
                        }
                        batch++;
                    }
                    segment++;
                }
            }
        }
    }
    D_800A0D8C = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80013970);
#endif

#ifdef NON_MATCHING
/* JFG: allocate count track lights (all free) and, if asked, a copy of every segment's vertices.
   NON_MATCHING: 19 words: v0/v1 swapped for the loop counters' copies, and the src/failed set-up order. */
void func_80013B74(s32 count, s32 copyVertices) {
    s32 failed;
    s32 i;
    SegmentAlloc *alloc;
    LevelModelSegment *segment;
    u8 *src;
    u8 *dst;
    s32 size;

    failed = TRUE;
    gNumTrackLights = 0;
    gMaxTrackLights = count;
    if ((gTrackLights = mempool_alloc_safe(gMaxTrackLights * sizeof(TrackLight), 0xFF0000FF)) != NULL) {
        i = gMaxTrackLights;
        while (i--) {
            gTrackLights[i].outerRadius = 0.0f;
        }
        failed = copyVertices;
        if (copyVertices) {
            alloc = mempool_alloc_safe(gCurrentLevelModel->numberOfSegments * sizeof(SegmentAlloc), 0xFF0000FF);
            if (alloc != NULL) {
                segment = gCurrentLevelModel->segments;
                D_800A0D7C = 1;
                D_800A0D80 = alloc;
                failed = FALSE;
                for (i = 0; i < gCurrentLevelModel->numberOfSegments; i++) {
                    src = (u8 *) segment->vertices;
                    size = segment->numberOfVertices * sizeof(Vertex);
                    alloc->vertices = segment->vertices;
                    alloc->data = mempool_alloc_safe(size, 0xFF0000FF);
                    if (alloc->data != NULL) {
                        dst = alloc->data;
                        while (size--) {
                            *dst++ = *src++;
                        }
                    } else {
                        failed = TRUE;
                    }
                    alloc++;
                    segment++;
                }
            }
        }
    }
    if (failed) {
        track_segment_data_free();
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80013B74);
#endif

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

extern u8 D_800F31E1[]; // JFG: a flag per segment (whether it is drawn)

#ifdef NON_MATCHING
/* JFG: copy the vertex colours of the drawn segments into their vertices (all of them while
   D_800A0D80 is set, else only the changed ones).
   NON_MATCHING: the t-registers are numbered one lower (target loads the drawn flag into a new temp). */
void func_8001400C(void) {
    LevelModelSegment *segment;
    s32 count;
    u8 *visible;
    s32 dir;
    SegmentColours *colours;
    s32 n;
    u8 *src;
    Vertex *vert;
    u16 *changed;
    u32 bits;
    u8 *s;
    Vertex *v;

    visible = D_800F31E1;
    segment = gCurrentLevelModel->segments;
    count = gCurrentLevelModel->numberOfSegments;
    if (D_800A0D80 != NULL) {
        dir = 1;
    } else {
        dir = -1;
    }
    while (count--) {
        if (*visible++ && (segment->colourFlags & dir) && (colours = segment->colours) != NULL) {
            src = colours->colours;
            vert = segment->vertices;
            n = segment->numberOfVertices;
            if (dir > 0) {
                while (n--) {
                    vert->r = src[0];
                    vert->g = src[1];
                    vert->b = src[2];
                    vert++;
                    src += 3;
                }
                segment->colourFlags ^= 1;
            } else {
                n = (n + 15) >> 4;
                changed = colours->changed;
                while (n--) {
                    v = vert;
                    s = src;
                    bits = *changed;
                    *changed = 0;
                    while (bits) {
                        if (bits & 1) {
                            v->r = s[0];
                            v->g = s[1];
                            v->b = s[2];
                        }
                        v++;
                        s += 3;
                        bits >>= 1;
                    }
                    vert += 16;
                    src += 48;
                    changed++;
                }
                segment->colourFlags = 0;
            }
        }
        segment++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_8001400C);
#endif

extern u8 D_800A4FCC;  // JFG: the number of cameras
extern s16 D_80104704; // the vertex colour level (m046)
extern s16 D_801046FC; // its time left (m046)
void func_8001D500(LevelModel *model, TrackLight *light, u8 *visible);
void func_8001400C(void);

/* JFG: flip the segments' double-buffered vertices, update their colours (a module's or func_8001400C)
   and apply each lit track light; one camera at most. */
void track_lighting_update(s32 arg0, s32 updateRate) {
    s32 i;
    LevelModelSegment *segment;
    SegmentAlloc *alloc;
    TrackLight *light;

    if (gCurrentLevelModel == NULL) {
        return;
    }
    if (D_800A4FCC >= 2) {
        return;
    }
    if (arg0 && D_800A0D80 != NULL) {
        return;
    }
    if (!arg0 && D_800A0D80 == NULL) {
        return;
    }
    alloc = D_800A0D80;
    if (alloc != NULL) {
        D_800A0D7C ^= 1;
        i = gCurrentLevelModel->numberOfSegments;
        segment = gCurrentLevelModel->segments;
        while (i--) {
            segment->vertices = (&alloc->vertices)[D_800A0D7C];
            segment->colourFlags = ((segment->colourFlags << 1) & 2) | ((segment->colourFlags >> 1) & 1);
            segment++;
            alloc++;
        }
    }
    if (runlink_module_loaded(0x1B)) {
        func_80054530(D_800F31E1, gCurrentLevelModel, updateRate);
    } else if (D_80104704 != 0xFF || D_801046FC != 0 || runlink_module_loaded(0x17) || runlink_module_loaded(0x1D)) {
        func_80054530(D_800F31E1, gCurrentLevelModel, updateRate);
    } else {
        func_8001400C();
    }
    i = gMaxTrackLights;
    light = gTrackLights;
    while (i--) {
        if (light->outerRadius != 0.0f) {
            func_8001D500(gCurrentLevelModel, light, D_800F31E1);
        }
        light++;
    }
}

/* JFG: what a racer points at from 0x564, reduced */
typedef struct RacerUnk564 {
    /* 0x000 */ u8 pad0[0x620];
    /* 0x620 */ u32 unk620_0 : 6;
    u32 unk620_6 : 6;
    u32 unk620_12 : 20;
} RacerUnk564;

Object **get_racer_objects(s32 *numRacers);

/* JFG: let each racer's block (from the last racer) change the three values through a module function. */
void racers_apply_module_values(s32 *arg0, s32 *arg1, s32 *arg2) {
    Object *obj;
    Object_Racer *racer;
    s32 numRacers;
    Object **racers;
    RacerUnk564 *block;
    s32 a;
    s32 b;
    s32 c;

    a = *arg0;
    b = *arg1;
    c = *arg2;
    racers = get_racer_objects(&numRacers);
    while (numRacers--) {
        obj = racers[numRacers];
        racer = obj->racer;
        block = racer->unk564;
        if (block != NULL && block->unk620_6) {
            func_80054530(obj, block, &a, &b, &c);
        }
    }
    *arg0 = a;
    *arg1 = b;
    *arg2 = c;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001448C);

#ifdef NON_MATCHING
/* NON_MATCHING: register choice only: the target keeps the batch pointer in $s3 and the texture in $s1 (mine
   the other way round); 18 words */
/* JFG's vertex and triangle commands here: length, command and parameter or'ed in that order */
#define gSPVertexSeg(pkt, v, last)                                                                        \
    {                                                                                                     \
        Gfx *_g = (Gfx *) (pkt);                                                                          \
        _g->words.w0 = _SHIFTL(((last) << 1) + ((last) << 3) + 8, 0, 16) | _SHIFTL(G_VTX, 24, 8) |          \
                       _SHIFTL(((last) << 3) | ((u32) (v) & 6), 16, 8);                                   \
        _g->words.w1 = (u32) (v);                                                                         \
    }
#define gSPPolygonSeg(pkt, ptr, n, tex)                                                                   \
    {                                                                                                     \
        Gfx *_g = (Gfx *) (pkt);                                                                          \
        _g->words.w0 = _SHIFTL((n) * 16, 0, 16) | _SHIFTL(5, 24, 8) | _SHIFTL((((n) - 1) << 4) | (tex), 16, 8); \
        _g->words.w1 = (u32) (ptr);                                                                       \
    }
extern s32 D_800F2FA0; // JFG: render flags added to batches with neither of 0x180
void tex_disable_modes(s32 flags);
void tex_enable_modes(s32 flags);
void func_8001C778(TextureHeader *tex, s32 texOffset, s32 flags);

/**
 * Render a group of a level segment's batches: pass 0x10000000 draws the opaque ones (without Z compare), the
 * others the batches whose flags match (4 and 0x20000000 the second group).
 * JFG: environment colour from the texture offset for some textures, flag 0x4000 for a special material.
 * DKR: render_level_segment
 */
void func_80014EBC(s32 segmentId, s32 pass) {
    s32 i;
    s32 end;
    s32 batchFlags;
    s32 mask;
    s32 wanted;
    TextureHeader *texture;
    TriangleBatchInfo *batch;
    UNUSED s32 numberVertices;
    UNUSED s32 numberTriangles;
    s32 vertices;
    s32 triangles;
    s32 hasTexture;
    s32 texOffset;
    s32 special;
    UNUSED s32 pad;
    LevelModelSegment *segment;

    segment = &gCurrentLevelModel->segments[segmentId];
    mask = 0x10000400;
    wanted = 0;
    switch (pass) {
        case 0x10000000:
            tex_disable_modes(2);
            end = segment->numberOfOpaqueBatches;
            i = 0;
            wanted = 0x10000000;
            break;
        case 4:
            mask = 0x30000400;
            i = segment->unk47;
            end = segment->numberOfBatches;
            break;
        case 0x20000000:
            mask = 0x30000400;
            wanted = 0x20000000;
            i = segment->unk47;
            end = segment->numberOfBatches;
            break;
        default:
            i = segment->numberOfOpaqueBatches;
            end = segment->unk47;
            break;
    }
    for (batch = &segment->batches[i]; i < end; i++, batch++) {
        batchFlags = batch->flags;
        if ((batchFlags & mask) != wanted) {
            continue;
        }
        texture = NULL;
        if (batch->textureIndex != 0xFF) {
            hasTexture = TRUE;
            texture = gCurrentLevelModel->textures[batch->textureIndex].texture;
        } else {
            hasTexture = FALSE;
        }
        vertices = (s32) &segment->vertices[batch->verticesOffset];
        triangles = (s32) &segment->triangles[batch->facesOffset];
        texOffset = batch->texOffset << 8;
        if (texture != NULL && (texture->flags & 0x40) && (batchFlags & 0x30) != 0x20) {
            gDPSetEnvColor(D_800F2F70++, (u8) (texOffset >> 8), (u8) (texOffset >> 8), (u8) (texOffset >> 8),
                           (u8) (texOffset >> 8));
        } else {
            gDPSetEnvColor(D_800F2F70++, 255, 255, 255, 0);
        }
        if (!(batchFlags & 0x180)) {
            batchFlags |= D_800F2FA0;
        }
        special = batchFlags & 0x4000;
        if (special && texture != NULL) {
            func_8001C778(texture, texOffset, batchFlags);
        } else {
            func_80055C00(&D_800F2F70, texture, batchFlags | 2, texOffset);
        }
        gSPVertexSeg(D_800F2F70++, OS_K0_TO_PHYSICAL(vertices), (batch + 1)->verticesOffset - batch->verticesOffset);
        gSPPolygonSeg(D_800F2F70++, OS_K0_TO_PHYSICAL(triangles), (batch + 1)->facesOffset - batch->facesOffset,
                      hasTexture);
        if (special) {
            rendermode_reset(&D_800F2F70);
        }
    }
    if (pass == 0x10000000) {
        tex_enable_modes(2);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80014EBC);
#endif

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

typedef struct Vec3f12 {
    f32 x, y, z;
} Vec3f12;

typedef struct Vec4f12 {
    f32 x, y, z, w;
} Vec4f12;

extern Vec3f12 D_800A0CF4[3][3]; // DKR: D_800DC8AC
extern Vec4f12 D_800F2FB8[3];    // DKR: D_8011D0F8
extern s32 D_800F2F8C;           // DKR: gIsNearCurrBBox
extern f32 D_800F31D8;           // DKR: gCurrBBoxDistanceToCamera
MtxF *get_projection_matrix_f32(void);
void func_80048B60(MtxF *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); // DKR: mtxf_transform_point
f32 sqrtf(f32 x);
f32 get_distance_to_active_camera(f32 x, f32 y, f32 z);

/**
 * JFG: the three view planes from the camera matrix (pointer loops instead of DKR's indices).
 * DKR: func_8002A31C
 */
void view_planes_update(void) {
    f32 ox1;
    f32 oy1;
    f32 oz1;
    f32 ox2;
    f32 oy2;
    f32 oz2;
    f32 ox3;
    f32 oy3;
    f32 oz3;
    MtxF *cameraMatrix;
    f32 inverseMagnitude;
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    f32 w;
    Vec3f12 *in;
    Vec4f12 *out;

    in = D_800A0CF4[0];
    out = D_800F2FB8;
    cameraMatrix = get_projection_matrix_f32();
    i = 0;
    do {
        func_80048B60(cameraMatrix, in[0].x, in[0].y, in[0].z, &ox1, &oy1, &oz1);
        func_80048B60(cameraMatrix, in[1].x, in[1].y, in[1].z, &ox2, &oy2, &oz2);
        func_80048B60(cameraMatrix, in[2].x, in[2].y, in[2].z, &ox3, &oy3, &oz3);
        x = ((oz2 - oz3) * oy1) + (oy2 * (oz3 - oz1)) + (oy3 * (oz1 - oz2));
        y = ((ox2 - ox3) * oz1) + (oz2 * (ox3 - ox1)) + (oz3 * (ox1 - ox2));
        z = ((oy2 - oy3) * ox1) + (ox2 * (oy3 - oy1)) + (ox3 * (oy1 - oy2));
        inverseMagnitude = 1.0f / sqrtf((x * x) + (y * y) + (z * z));
        if (inverseMagnitude > 0.0f) {
            x *= inverseMagnitude;
            y *= inverseMagnitude;
            z *= inverseMagnitude;
        }
        i++;
        w = -((ox1 * x) + (oy1 * y) + (oz1 * z));
        out->x = x;
        out->y = y;
        out->z = z;
        out->w = w;
        in += 3;
        out++;
    } while (i != 3);
}

/**
 * Takes a normalised (0-1) face direction of the active camera, then adds together a magnitude
 * to a total figure to determine whether or not a segment should be visible.
 * JFG: three planes, a single-precision distance check.
 * DKR: block_visible
 */
s32 block_visible(LevelModelSegmentBoundingBox *bb) {
    UNUSED u8 unknown[0x28];
    s64 sp48;
    s32 i, j;
    s32 isVisible;
    f32 dirX, dirY, dirZ, dirW;
    f32 x, y, z;

    for (j = 0; j < 3; j++) {
        dirX = D_800F2FB8[j].x;
        dirY = D_800F2FB8[j].y;
        dirZ = D_800F2FB8[j].z;
        dirW = D_800F2FB8[j].w;

        for (i = 0, isVisible = FALSE; i < 8 && !isVisible; i++) {
            if (i & 1) {
                sp48 = bb->x1 * dirX;
            } else {
                sp48 = bb->x2 * dirX;
            }
            if (i & 2) {
                sp48 += bb->y1 * dirY;
            } else {
                sp48 += bb->y2 * dirY;
            }
            if (i & 4) {
                sp48 += bb->z1 * dirZ;
            } else {
                sp48 += bb->z2 * dirZ;
            }
            sp48 += dirW;
            if (sp48 > 0) {
                isVisible = TRUE;
            }
        }
        if (i == 8 && !isVisible) {
            return FALSE;
        }
    }
    x = (bb->x2 + bb->x1) >> 1;
    y = (bb->y2 + bb->y1) >> 1;
    z = (bb->z2 + bb->z1) >> 1;
    D_800F31D8 = get_distance_to_active_camera(x, y, z);
    if (D_800F31D8 < 1000.0f) {
        D_800F2F8C = TRUE;
    } else {
        D_800F2F8C = FALSE;
    }
    return TRUE;
}

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

/* JFG: what the sweep tests fill in (func_80016208) */
typedef struct SweepHit {
    /* 0x00 */ void *obj;
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ f32 normal[3];
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 unk25;
    /* 0x26 */ u8 unk26;
    /* 0x27 */ u8 unk27;
} SweepHit;

#ifdef NON_MATCHING
/* NON_MATCHING: the target copies flags to $s0 (in the delay slot of the first branch) and takes the 0x40 test
   from that copy, which also moves the hit-field stores after the float stores; IDO propagates my copies away */
extern f32 D_800ABA28;
void track_edges_build(void);
void func_800805F4(void);
s32 func_8007F280(void *obj, f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags, f32 radius2);
s32 func_80017094(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags, f32 radius2);
s32 func_8007FC0C(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags);
s32 func_80017794(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags);

/* JFG: sweep a sphere from `from` to `to` against objects and the level (and, with flag 0x10, the extra tests),
 * repeating while something is hit (up to 6 times; then `to` goes back to `from`); flag 0x40: test once. */
s32 func_80016208(void *obj, f32 *from, f32 *to, f32 radius, u16 flags, SweepHit *hit) {
    s32 found;
    s32 result;
    s32 tries;
    s32 once;
    s32 extra;

    extra = flags & 0x10;
    if (extra) {
        track_edges_build();
        func_800805F4();
    }
    hit->obj = NULL;
    hit->unk25 = 0;
    hit->pos[0] = 0.0f;
    hit->pos[1] = 0.0f;
    hit->pos[2] = 0.0f;
    hit->normal[0] = 0.0f;
    hit->normal[1] = 0.0f;
    hit->normal[2] = 0.0f;
    once = flags & 0x40;
    result = 0;
    tries = 0;
    hit->unk24 = 0;
    hit->unk20 = 0;
    hit->unk26 = 0;
    hit->unk27 = 0;
    hit->unk1C = D_800ABA28;
    found = TRUE;
    while (found && !once) {
        found = FALSE;
        if (func_8007F280(obj, from, to, hit, radius, flags, radius)) {
            found = TRUE;
            result = TRUE;
        }
        if (func_80017094(from, to, hit, radius, flags, radius)) {
            found = TRUE;
            result = TRUE;
        }
        if (extra) {
            if (func_8007FC0C(from, to, hit, radius, flags)) {
                found = TRUE;
                result = TRUE;
            }
            if (func_80017794(from, to, hit, radius, flags)) {
                found = TRUE;
                result = TRUE;
            }
        }
        if (found) {
            tries++;
            if (tries >= 6) {
                found = FALSE;
                to[0] = from[0];
                result = FALSE;
                to[1] = from[1];
                to[2] = from[2];
                hit->unk25 = 0;
            }
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80016208);
#endif

INCLUDE_ASM("asm/nonmatchings/12D50", func_800163FC);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80017094);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80017794);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80018024);

f32 sqrtf(f32 x);

#ifdef NON_MATCHING
/**
 * JFG: where a ray (origin, unit direction) meets a sphere; FALSE if it misses.
 * NON_MATCHING: 7 words: b and b*b swap FP registers ($f14/$f18); the frame matches with the three unused words.
 */
s32 func_800183D4(f32 *origin, f32 *dir, f32 *centre, f32 radius, f32 *tNear, f32 *tFar) {
    f32 dx, dy, dz;
    f32 b;
    f32 b2;
    f32 c;
    f32 d;
    UNUSED s32 pad[3];
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

/* JFG: a collision edge of the level geometry (0x30 bytes) */
typedef struct TrackEdge {
    /* 0x00 */ f32 x0, y0, z0;
    /* 0x0C */ f32 x1, y1, z1;
    /* 0x18 */ f32 dirX, dirY, dirZ; // the unit vector from the first end to the second
    /* 0x24 */ f32 length;
    /* 0x28 */ LevelModelSegment *segment;
    /* 0x2C */ s16 batch;
    /* 0x2E */ s16 unk2E; // JFG: set from the triangle's flag bit (edge << 2)
} TrackEdge;

extern TrackEdge *D_800F33B0; // JFG: the edge list
extern s32 D_800F33B4;        // JFG: the number of edges in it
extern s32 *D_800F33B8;       // JFG: per entry, a segment address (> 0) or 0 for a triangle
extern s16 *D_800F33BC;       // JFG: per entry, the batch of the triangle
extern s16 *D_800F33C0;       // JFG: per entry, the triangle
extern s32 D_800F33C4;        // JFG: the number of entries

/**
 * JFG: build the edge list from the entries: a segment entry selects the segment, a triangle entry adds the
 * triangle's edges that its edge flags mark (ends, direction and length; zero-length edges are skipped), up to
 * the level header's limit. The segment is only set by an earlier entry.
 */
void track_edges_build(void) {
    UNUSED s32 pad;
    UNUSED s32 pad2;
    LevelModelSegment *segment;
    s32 i;
    s32 j;
    s32 k;
    s32 bit;
    s32 tri;
    Triangle *triangle;
    s32 batch;
    Vertex *verts;
    Vertex *va;
    Vertex *vb;
    TrackEdge *edge;
    f32 dx;
    f32 dy;
    f32 dz;

    D_800F33B4 = 0;
    for (i = 0; i < D_800F33C4; i++) {
        if (D_800F33B8[i] > 0) {
            segment = (LevelModelSegment *) (D_800F33B8[i] | 0x80000000);
        } else {
            batch = D_800F33BC[i];
            tri = D_800F33C0[i];
            triangle = &segment->triangles[tri];
            verts = &segment->vertices[segment->batches[batch].verticesOffset];
            for (j = 0; j < 3; j++) {
                bit = 1 << j;
                if (segment->edgeFlags[tri] & bit) {
                    edge = &D_800F33B0[D_800F33B4];
                    k = j + 1;
                    if (k >= 3) {
                        k = 0;
                    }
                    va = &verts[(&triangle->vi0)[j]];
                    vb = &verts[(&triangle->vi0)[k]];
                    edge->x0 = va->x;
                    edge->y0 = va->y;
                    edge->z0 = va->z;
                    edge->x1 = vb->x;
                    edge->y1 = vb->y;
                    edge->z1 = vb->z;
                    dx = edge->x1 - edge->x0;
                    dy = edge->y1 - edge->y0;
                    dz = edge->z1 - edge->z0;
                    edge->length = sqrtf(dx * dx + dy * dy + dz * dz);
                    if (edge->length > 0.0f) {
                        edge->segment = segment;
                        edge->dirX = dx / edge->length;
                        edge->dirY = dy / edge->length;
                        edge->dirZ = dz / edge->length;
                        edge->batch = D_800F33BC[i];
                        edge->unk2E = 0;
                        if (segment->triangles[tri].flags & (bit << 2)) {
                            edge->unk2E = 1;
                        }
                        if (++D_800F33B4 >= D_800A0D64->maxEdges) {
                            i = D_800F33C4;
                            j = 3;
                        }
                    }
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80018808);

s32 line_clip_plane(f32 denom, f32 num, f32 *tEnter, f32 *tLeave);

/**
 * JFG: clip the line from (*x0, *y0, *z0) to (*x1, *y1, *z1) against the box min..max: TRUE if part of it is
 * inside; the ends are moved to the box and the entry and exit fractions go to tEnterOut and tLeaveOut (both 0
 * for a point inside).
 */
s32 line_clip_box(f32 *x0, f32 *y0, f32 *z0, f32 *x1, f32 *y1, f32 *z1, f32 *min, f32 *max, f32 *tEnterOut,
                  f32 *tLeaveOut) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 tEnter;
    f32 tLeave;
    s32 hit;

    dx = *x1 - *x0;
    dy = *y1 - *y0;
    dz = *z1 - *z0;
    hit = FALSE;
    if (dx == 0.0f && dy == 0.0f && dz == 0.0f && min[0] <= *x0 && *x0 <= max[0] && min[1] <= *y0 &&
        *y0 <= max[1] && min[2] <= *z0 && *z0 <= max[2]) {
        hit = TRUE;
        *tEnterOut = 0.0f;
        *tLeaveOut = 0.0f;
    } else {
        tEnter = 0.0f;
        tLeave = 1.0f;
        if (line_clip_plane(dx, min[0] - *x0, &tEnter, &tLeave) &&
            line_clip_plane(-dx, *x0 - max[0], &tEnter, &tLeave) &&
            line_clip_plane(dy, min[1] - *y0, &tEnter, &tLeave) &&
            line_clip_plane(-dy, *y0 - max[1], &tEnter, &tLeave) &&
            line_clip_plane(dz, min[2] - *z0, &tEnter, &tLeave) &&
            line_clip_plane(-dz, *z0 - max[2], &tEnter, &tLeave)) {
            hit = TRUE;
            if (tLeave < 1.0f) {
                *x1 = *x0 + tLeave * dx;
                *y1 = *y0 + tLeave * dy;
                *z1 = *z0 + tLeave * dz;
            }
            *tLeaveOut = tLeave;
            if (tEnter > 0.0f) {
                *x0 += tEnter * dx;
                *y0 += tEnter * dy;
                *z0 += tEnter * dz;
            }
            *tEnterOut = tEnter;
        }
    }
    return hit;
}

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

u32 func_8001D344(LevelModelSegmentBoundingBox *bb, s32 x1, s32 z1, s32 x2, s32 z2);
s32 func_800494AC(s32 x, s32 z, Vertex *v0, Vertex *v1, Vertex *v2); // the point is inside the triangle

/* JFG: the height of the first level triangle (of a batch with one of the flags) under the point,
   written to *outY (-32768 if none); returns that batch's flags, or 0. */
s32 track_height_at(f32 xIn, f32 zIn, f32 *outY, s32 flags) {
    LevelModelSegment *segment;
    TriangleBatchInfo *batch;
    s32 segmentCount;
    Triangle *tri;
    s32 batchNum;
    Vertex *verts;
    Vertex *v0;
    s32 batchFlags;
    s32 i;
    s16 segmentsInside[32];
    Vertex *v1;
    Vertex *v2;
    u32 mask;
    u32 bits;
    s32 faceNum;
    s32 x;
    s32 z;

    x = xIn;
    z = zIn;
    segmentCount = get_inside_segment_count_xz(x, z, segmentsInside);
    *outY = -32768.0f;
    i = 0;
    while (i < segmentCount) {
        mask = func_8001D344(&gCurrentLevelModel->segmentsBoundingBoxes[segmentsInside[i]], x, z, x, z);
        segment = &gCurrentLevelModel->segments[segmentsInside[i++]];
        batch = segment->batches;
        for (batchNum = 0; batchNum < segment->numberOfBatches; batchNum++) {
            batchFlags = batch->flags;
            if (batchFlags & flags) {
                tri = &segment->triangles[batch->facesOffset];
                verts = &segment->vertices[batch->verticesOffset];
                faceNum = batch->facesOffset;
                for (; faceNum < batch[1].facesOffset; faceNum++) {
                    bits = segment->triangleMasks[faceNum] & mask;
                    if ((bits >> 16) && (bits & 0xFFFF)) {
                        v0 = &verts[tri->vi0];
                        v1 = &verts[tri->vi1];
                        v2 = &verts[tri->vi2];
                        if (func_800494AC(x, z, v0, v1, v2)) {
                            *outY = v0->y;
                            return batchFlags;
                        }
                    }
                    tri++;
                }
            }
            batch++;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/12D50", func_80019888);

INCLUDE_ASM("asm/nonmatchings/12D50", func_80019E0C);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001A218);

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001AB84);

/* JFG: a triangle's entry in the segment's plane index (8 bytes) */
typedef struct TrianglePlaneRef {
    /* 0x00 */ u16 plane;
    /* 0x02 */ u8 pad2[6];
} TrianglePlaneRef;

/* JFG: a plane a*x + b*y + c*z + d = 0 */
typedef struct Plane {
    /* 0x00 */ f32 a, b, c, d;
} Plane;

#ifdef NON_MATCHING
/* NON_MATCHING: registers only: the plane's b is $f2 and the distance $f14 in the target (mine $f12 and $f2), and
   some temporaries are numbered differently; 19 words */
u8 func_8001D464(LevelModelSegmentBoundingBox *bb, s32 y1, s32 y2);

/**
 * JFG: the level triangle nearest in height to (x, y, z) among those of the point's segment under it (batches
 * without any of flags): its height goes to *outY and its plane to *outPlane (each if not NULL); NULL if none.
 */
Triangle *func_8001ACE0(f32 x, f32 y, f32 z, f32 *outY, Plane **outPlane, s32 flags) {
    s32 segmentIndex;
    s32 ix;
    Triangle *result;
    TriangleBatchInfo *batch;
    LevelModelSegment *segment;
    Triangle *tri;
    Vertex *verts;
    u32 *masks;
    u8 *heightMasks;
    s32 faceNum;
    Plane *plane;
    s32 i;
    f32 d;
    f32 h;
    f32 best;
    u32 bits;
    UNUSED s32 pad;
    u32 mask;
    s32 iz;
    u8 heightBits;
    u8 heightMask;

    segmentIndex = get_level_segment_index_from_position(x, y, z);
    if (segmentIndex < 0) {
        return NULL;
    }
    best = 32768.0f;
    ix = x;
    result = NULL;
    iz = z;
    mask = func_8001D344(&gCurrentLevelModel->segmentsBoundingBoxes[segmentIndex], ix, iz, ix, iz);
    heightMask = func_8001D464(&gCurrentLevelModel->segmentsBoundingBoxes[segmentIndex], y, y);
    segment = &gCurrentLevelModel->segments[segmentIndex];
    batch = segment->batches;
    for (i = 0; i < segment->numberOfBatches; batch++, i++) {
        if (!(batch->flags & flags)) {
            faceNum = batch->facesOffset;
            tri = &segment->triangles[faceNum];
            verts = &segment->vertices[batch->verticesOffset];
            masks = &segment->triangleMasks[faceNum];
            heightMasks = &segment->triangleHeightMasks[faceNum];
            for (; faceNum < batch[1].facesOffset; faceNum++, tri++) {
                bits = *masks++ & mask;
                heightBits = *heightMasks++ & heightMask;
                if ((bits & 0xFFFF) && (bits >> 16) && (heightBits & 0xFF)) {
                    if (func_800494AC(ix, iz, &verts[tri->vi0], &verts[tri->vi1], &verts[tri->vi2])) {
                        plane = &segment->planes[segment->trianglePlanes[faceNum].plane];
                        if (plane->b != 0.0f) {
                            h = -((plane->a * x + plane->c * z + plane->d) / plane->b);
                            d = y - h;
                            if (y < h) {
                                d = -d;
                            }
                            if (d < best) {
                                best = d;
                                if (outY != NULL) {
                                    *outY = h;
                                }
                                if (outPlane != NULL) {
                                    *outPlane = plane;
                                }
                                result = tri;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_8001ACE0);
#endif

INCLUDE_ASM("asm/nonmatchings/12D50", func_8001B078);

/**
 * Returns the current loaded level geometry
 * Official Name: trackGetTrack
 */
LevelModel *get_current_level_model(void) {
    return gCurrentLevelModel;
}

extern void *D_800A0D88;
extern s32 D_800A0D84;
extern void *D_800A0D68;
extern void *D_800F31C8;
extern TrackEdge *D_800F33B0;
extern s32 *D_800F33B8;
extern s16 *D_800F33BC;
extern s16 *D_800F33C0;
extern void *D_800F2FB0;
extern s8 D_800A0CE0;
void track_segment_data_free(void);
void func_80038F04(void);
void tex_free(void *tex);
void func_8001D938(void);
void free_object(Object *obj);
void gParticlePtrList_flush(void);
void func_80077060(void);
void free_all_objects(void);

/**
 * Frees and unloads level data from RAM.
 * JFG: its own buffers, and each segment's block at 0x34.
 * DKR: free_track
 */
void free_track(void) {
    s32 i;

    track_segment_data_free();
    if (D_800A0D88 != NULL) {
        mempool_free(D_800A0D88);
        D_800A0D88 = NULL;
        D_800A0D84 = 0;
    }
    func_80038F04();
    if (D_800A0D68 != NULL) {
        tex_free(D_800A0D68);
        D_800A0D68 = NULL;
    }
    for (i = 0; i < gCurrentLevelModel->numberOfSegments; i++) {
        if (gCurrentLevelModel->segments[i].colours != NULL) {
            mempool_free(gCurrentLevelModel->segments[i].colours);
        }
    }
    for (i = 0; i < gCurrentLevelModel->numberOfTextures; i++) {
        tex_free(gCurrentLevelModel->textures[i].texture);
    }
    mempool_free(D_800F31C8);
    mempool_free(D_800F33B8);
    mempool_free(D_800F33BC);
    mempool_free(D_800F33C0);
    if (func_80054530(osRomBase) != 0) {
        mempool_free(D_800F33B0);
    }
    func_8001D938();
    if (gSkydomeSegment != NULL) {
        free_object(gSkydomeSegment);
        gParticlePtrList_flush();
    }
    func_80077060();
    free_all_objects();
    gCurrentLevelModel = NULL;
    if (D_800F2FB0 != NULL) {
        mempool_free(D_800F2FB0);
        D_800F2FB0 = NULL;
    }
    D_800A0CE0 = 0;
}

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

/* DKR src/tracks.h ShadowHeapProperties (8 bytes) */
typedef struct ShadowHeapProperties {
    /* 0x00 */ void *texture;
    /* 0x04 */ s16 triCount;
    /* 0x06 */ s16 vtxCount;
} ShadowHeapProperties;

/* JFG: an object's shadow, reduced (up to three meshes) */
typedef struct ShadowData {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ u16 texOffset;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ u8 group;
    /* 0x0D */ u8 padD[2];
    /* 0x0F */ u8 numMeshes;
    /* 0x10 */ s16 meshStart[3];
    /* 0x16 */ s16 meshEnd[3];
} ShadowData;

void func_8001D9D0(s32 flags, Vertex **verts, Triangle **tris, ShadowHeapProperties **data);

/**
 * Render the shadow of the object on the ground.
 * JFG: up to three meshes, a texture offset, the alpha in the primitive colour.
 * DKR: shadow_render
 */
void shadow_render(Object *obj, ShadowData *shadow) {
    s32 i;
    s32 numVerts;
    s32 numTris;
    Vertex *vtx;
    Triangle *tri;
    ShadowHeapProperties *mesh;
    ShadowHeapProperties *data;
    Triangle *tris;
    s32 j;
    Vertex *verts;
    s32 alpha;

    if (shadow->group) {
        func_8001D9D0(shadow->group, &verts, &tris, &data);
        for (i = 0; i < shadow->numMeshes; i++) {
            if (shadow->meshStart[i] != -1) {
                j = shadow->meshStart[i];
                mesh = &data[j];
                alpha = verts[mesh->vtxCount].a;
                alpha = (obj->opacity * alpha) >> 8;
                if (alpha > 0) {
                    gDPSetPrimColor(D_800F2F70++, 0, 0, 0, 0, 0, alpha);
                    while (j < shadow->meshEnd[i]) {
                        func_80055C00(&D_800F2F70, mesh->texture, 0xE, shadow->texOffset << 8);
                        numTris = mesh->triCount;
                        numVerts = mesh->vtxCount;
                        tri = &tris[numTris];
                        vtx = &verts[numVerts];
                        numTris = mesh[1].triCount - numTris;
                        numVerts = mesh[1].vtxCount - numVerts;
                        gSPVertexLast(D_800F2F70++, OS_K0_TO_PHYSICAL(vtx), numVerts, 0);
                        gSPPolygon(D_800F2F70++, OS_K0_TO_PHYSICAL(tri), numTris, 1);
                        j++;
                        mesh++;
                    }
                    gDPSetPrimColor(D_800F2F70++, 0, 0, 255, 255, 255, 255);
                }
            }
        }
    }
}

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
s32 func_80054530();

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

/**
 * If the fog override timer is active, apply that and slowly degrade.
 * Otherwise, set the current fog to the intended fog settings.
 * JFG: a fog whose unk3 is set swings its near plane between unk14 and unk18 instead
 * (bit 7 gives the direction).
 * DKR: update_fog
 */
void update_fog(s32 viewportCount, s32 updateRate) {
    s32 i;

    for (i = 0; i < viewportCount; i++) {
        if (gFogData[i].intendedFog.unk3 > 0) {
            gFogData[i].fog.near += (updateRate * (gFogData[i].intendedFog.unk3 & 0x7F)) << 11;
            if (gFogData[i].fog.near > gFogData[i].unk18) {
                gFogData[i].fog.near = gFogData[i].unk18 + (gFogData[i].unk18 - gFogData[i].fog.near);
                gFogData[i].intendedFog.unk3 |= 0x80;
            }
        } else if (gFogData[i].intendedFog.unk3 < 0) {
            gFogData[i].fog.near -= (updateRate * (gFogData[i].intendedFog.unk3 & 0x7F)) << 11;
            if (gFogData[i].fog.near < gFogData[i].unk14) {
                gFogData[i].fog.near = gFogData[i].unk14 + (gFogData[i].unk14 - gFogData[i].fog.near);
                gFogData[i].intendedFog.unk3 ^= 0x80;
            }
        } else if (gFogData[i].switchTimer > 0) {
            if (updateRate < gFogData[i].switchTimer) {
                gFogData[i].fog.r += gFogData[i].addFog.r * updateRate;
                gFogData[i].fog.g += gFogData[i].addFog.g * updateRate;
                gFogData[i].fog.b += gFogData[i].addFog.b * updateRate;
                gFogData[i].fog.near += gFogData[i].addFog.near * updateRate;
                gFogData[i].fog.far += gFogData[i].addFog.far * updateRate;
                gFogData[i].switchTimer -= updateRate;
            } else {
                gFogData[i].fog.r = gFogData[i].intendedFog.r << 16;
                gFogData[i].fog.g = gFogData[i].intendedFog.g << 16;
                gFogData[i].fog.b = gFogData[i].intendedFog.b << 16;
                gFogData[i].fog.near = gFogData[i].intendedFog.near << 16;
                gFogData[i].fog.far = gFogData[i].intendedFog.far << 16;
                gFogData[i].switchTimer = 0;
            }
        }
    }
}

extern Gfx *D_800F2F70; // DKR: gTrackDL

/**
 * Sets the fog settings for the active viewport based on the parameters of the environment data.
 * JFG: through get_fog_settings (which modules can override).
 * DKR: apply_fog
 */
void apply_fog(s32 playerID) {
    s16 near;
    s16 far;
    s16 unk;
    u8 r;
    u8 g;
    u8 b;
    u8 unk2;

    get_fog_settings(playerID, &near, &far, &unk, &r, &g, &b, &unk2);
    gDPSetFogColor(D_800F2F70++, r, g, b, 0xFF);
    gSPFogPosition(D_800F2F70++, near, far);
}

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

void func_80049F08(ObjectTransform *trans, MtxF *mtx); // DKR: mtxf_from_transform (arguments swapped)
void func_80048B60(MtxF *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); // DKR: mtxf_transform_point

/**
 * Take the current camera position and calculate the perspective position, for envmapping.
 * JFG: the scale is left unset.
 * DKR: compute_scene_camera_transform_matrix
 */
void compute_scene_camera_transform_matrix(void) {
    MtxF mtx;
    ObjectTransform trans;

    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = -65536.0f;

    trans.rotation[2] = gSceneActiveCamera->trans.rotation[2];
    trans.rotation[1] = gSceneActiveCamera->trans.rotation[1];
    trans.rotation[0] = gSceneActiveCamera->trans.rotation[0];
    trans.x_position = 0.0f;
    trans.y_position = 0.0f;
    trans.z_position = 0.0f;

    func_80049F08(&trans, &mtx);
    func_80048B60(&mtx, x, y, z, &x, &y, &z);

    D_800F3360.x = x;
    D_800F3360.y = y;
    D_800F3360.z = z;
}

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
