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

/* JFG: a segment's vertex colours (segment_vertex_colours_copy) */
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
    /* 0x14 */ u8 *triangleHeightMasks; // JFG: per triangle, the height cells it covers (see track_nearest_triangle)
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
    /* 0x47 */ u8 unk47;                 // JFG: the end of the second batch group (see render_level_segment)
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
    /* 0x04 */ u8 pad4[3];
    /* 0x07 */ s8 surfaceType; // JFG: the batch's surface type (func_8001B078)
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
    /* 0x1C */ u8 pad1C[0x1E - 0x1C];
    /* 0x1E */ s16 numberOfAnimatedTextures;
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
void track_module_effects_run(s32 arg0) {
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

/* JFG: the level header, reduced */
typedef struct LevelHeader12 {
    /* 0x00 */ u8 pad0[0x69];
    /* 0x69 */ s8 skydomeFixed; // JFG: the skydome stays at the camera offset, unrotated (DKR: skyDome)
    /* 0x6A */ u8 pad6A[0x6C - 0x6A];
    /* 0x6C */ s8 raceType; // DKR: race_type
    /* 0x6D */ u8 pad6D[0xA0 - 0x6D];
    /* 0xA0 */ s16 weatherEnable;
    /* 0xA2 */ u8 padA2[0xB0 - 0xA2];
    /* 0xB0 */ u8 skyScaleU;           // DKR: unkA0
    /* 0xB1 */ u8 skyScaleV;           // DKR: unkA1
    /* 0xB2 */ s8 skyScrollU;          // DKR: unkA2
    /* 0xB3 */ s8 skyScrollV;          // DKR: unkA3
    /* 0xB4 */ TextureHeader *skyTex;  // DKR: unkA4
    /* 0xB8 */ s16 skyOffsetU;         // DKR: unkA8
    /* 0xBA */ s16 skyOffsetV;         // DKR: unkAA
    /* 0xBC */ struct PulsatingLightData *pulseLightData;
    /* 0xC0 */ u8 padC0[0xCD - 0xC0];
    /* 0xCD */ s8 antiAliasing; // DKR: unkBD
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

/* DKR include/f3ddkr.h */
#define gDkrDisableBillboard(pkt) gMoveWd(pkt, 0x02, 0, 0)

extern Gfx *D_800F2F70;      // DKR: gTrackDL
extern Mtx *D_800F2F74;      // DKR: gTrackMtxPtr
extern Vertex *D_800F2F78;   // DKR: gTrackVtxPtr
extern Triangle *D_800F2F7C; // DKR: gTrackTriPtr
extern s32 D_800F2F84;       // DKR: gSceneCurrentPlayerID
extern s32 D_800F2F8C;       // DKR: gIsNearCurrBBox
extern s32 D_800F2FA0;       // DKR: gAntiAliasing
extern s32 D_800F2FA4;      // DKR: gTrackTexAnimOffset
extern u32 D_800F2FA8;       // DKR: gTrackTexAnimFlags
extern s8 D_800A0CD0;        // DKR: gDrawLevelSegments
extern u8 D_800A0CE0;
extern LevelHeader12 *D_800A0D64; // DKR: gCurrentLevelHeader2
extern TextureHeader *D_800A0D68; // JFG: an animated texture (its frame below)
extern s32 D_800A0D6C;
extern u8 D_800A321C;
extern s32 D_800A74E0;
extern s16 D_801046EE;
u8 camera_count_get(void); // the number of cameras
s8 is_game_paused(void);
void debug_trace_add(Gfx *dList, char *file, s32 line);
void set_active_camera(s32 index);
void func_8001DA1C(s32 arg0, s32 updateRate);
void level_colour_cycles_update(s32 updateRate);
void update_pulsating_light_data(struct PulsatingLightData *data, s32 timeDelta);
void tex_animate_texture(TextureHeader *texture, u32 *triangleBatchInfoFlags, s32 *arg2, s32 updateRate);
void rendermode_reset(Gfx **dList);
void rain_fog(void);
void update_fog(s32 viewportCount, s32 updateRate);
void track_tex_anim(s32 updateRate);
void level_effects_update(s32 updateRate);
void func_8003FE3C(s32 arg0, s32 arg1);
void func_8003FEB8(s32 arg0, s32 arg1);
void copy_viewports_to_stack(void);
void apply_fog(s32 playerID);
void viewport_apply(Gfx **dList, Mtx **mtx);
void view_planes_update(void);
void func_8000C9E8(s32 arg0);
void screen_lines_set_window(s32 arg0);
void trackbg_render_flashy(); // takes no arguments, as in DKR
void skydome_render(s32 updateRate);
void func_80013454(void);
void initialise_player_viewport_vars(s32 arg0, s32 arg1);
void weather_clip_planes(s16 near, s16 far);
void weather_update(Gfx **dList, Mtx **mtx, Vertex **vtx, Triangle **tris, s32 updateRate);
void coronas_update(void);
void func_80024044(void);
void func_80024634(Gfx **dList, Mtx **mtx, Vertex **vtx);
void ground_screen_effect_update(s32 arg0);
void glitch_effect_draw(Gfx **dList);
void viewport_reset(Gfx **dList);
void shadow_buffers_flip(void);

/**
 * The main rendering function for the level: updates the level's animated parts, then draws every
 * viewport's scene.
 * JFG: only when a module call returns non-zero; no mirror mode, a debug trace, more module calls.
 * DKR: render_scene
 */
void func_800121C0(Gfx **dList, Mtx **mtx, Vertex **vtx, Triangle **tris, s32 updateRate) {
    s32 numViewports;
    s32 tempUpdateRate;
    s32 i;
    s32 j;

    if (&updateRate) {} // fakematch: keeps updateRate on the stack
    numViewports = camera_count_get();
    if (func_80054530()) {
        D_800F2F70 = *dList;
        D_800F2F74 = *mtx;
        D_800F2F78 = *vtx;
        D_800F2F7C = *tris;
        debug_trace_add(D_800F2F70, "track/track.c", 0x1F8);
        set_active_camera(0);
        gSceneRenderSkyDome = TRUE;
        D_800F2F8C = FALSE;
        if (is_game_paused()) {
            tempUpdateRate = 0;
        } else {
            tempUpdateRate = updateRate;
        }
        if (D_800A0D68 != NULL) {
            i = D_800A0D6C;
            i += D_800A0D68->frameAdvanceDelay * tempUpdateRate;
            while (i >= D_800A0D68->numOfTextures) {
                i -= D_800A0D68->numOfTextures;
            }
            D_800A0D6C = i;
        }
        func_8001DA1C(1, updateRate);
        level_colour_cycles_update(tempUpdateRate);
        if (D_800A0D64->pulseLightData != (struct PulsatingLightData *) -1) {
            update_pulsating_light_data(D_800A0D64->pulseLightData, tempUpdateRate);
        }
        if (D_800A0D64->raceType == 2) {
            D_800A0CD0 = FALSE;
        } else {
            D_800A0CD0 = TRUE;
        }
        if (D_800A0D64->raceType == 1 || D_800A0D64->raceType == 2 || D_800A0D64->antiAliasing) {
            D_800F2FA0 = TRUE;
        }
        if (D_800A0D64->skydomeFixed == -1) {
            i = (D_800A0D64->skyTex->width << 9) - 1;
            D_800A0D64->skyOffsetU = (D_800A0D64->skyOffsetU + (D_800A0D64->skyScrollU * tempUpdateRate)) & i;
            i = (D_800A0D64->skyTex->height << 9) - 1;
            D_800A0D64->skyOffsetV = (D_800A0D64->skyOffsetV + (D_800A0D64->skyScrollV * tempUpdateRate)) & i;
            tex_animate_texture(D_800A0D64->skyTex, &D_800F2FA8, &D_800F2FA4, tempUpdateRate);
        }
        rendermode_reset(&D_800F2F70);
        gDkrDisableBillboard(D_800F2F70++);
        gSPClearGeometryMode(D_800F2F70++, G_CULL_FRONT);
        gDPSetBlendColor(D_800F2F70++, 0, 0, 0, 100);
        gDPSetPrimColor(D_800F2F70++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor(D_800F2F70++, 255, 255, 255, 0);
        rain_fog();
        update_fog(numViewports, tempUpdateRate);
        if (gCurrentLevelModel->numberOfAnimatedTextures > 0) {
            track_tex_anim(tempUpdateRate);
        }
        if (D_800A0CE0) {
            func_80054530(tempUpdateRate);
        }
        level_effects_update(tempUpdateRate);
        if (D_800A321C && numViewports == 1) {
            func_8003FE3C(0, 1);
            copy_viewports_to_stack();
        }
        for (j = D_800F2F84 = 0; j < numViewports; D_800F2F84++, j = D_800F2F84) {
            apply_fog(j);
            gDPPipeSync(D_800F2F70++);
            set_active_camera(D_800F2F84);
            viewport_apply(&D_800F2F70, &D_800F2F74);
            view_planes_update();
            func_8000C9E8(1);
            screen_lines_set_window(D_800F2F84);
            if (numViewports < 2) {
                if (D_800A0D64->skydomeFixed == -1) {
                    trackbg_render_flashy();
                } else {
                    skydome_render(updateRate);
                }
            } else {
                func_80013454();
            }
            debug_trace_add(D_800F2F70, "track/track.c", 0x27D);
            gDPPipeSync(D_800F2F70++);
            initialise_player_viewport_vars(numViewports, updateRate);
            weather_clip_planes(-1, -512);
            if (D_800A0D64->weatherEnable > 0 && numViewports < 2) {
                weather_update(&D_800F2F70, &D_800F2F74, &D_800F2F78, &D_800F2F7C, tempUpdateRate);
            }
            if (numViewports < 2) {
                coronas_update();
                func_80024044();
                func_80024634(&D_800F2F70, &D_800F2F74, &D_800F2F78);
                ground_screen_effect_update(tempUpdateRate);
                glitch_effect_draw(&D_800F2F70);
            }
        }
        if (D_800A74E0 > 0) {
            func_80054530(&D_800F2F70);
        }
        func_80054530(&D_800F2F70);
        if (D_801046EE != 0) {
            func_80054530(&D_800F2F70);
        }
        func_8003FEB8(0, 1);
        viewport_reset(&D_800F2F70);
        gDPPipeSync(D_800F2F70++);
        gDkrDisableBillboard(D_800F2F70++);
        shadow_buffers_flip();
        *dList = D_800F2F70;
        *mtx = D_800F2F74;
        *vtx = D_800F2F78;
        *tris = D_800F2F7C;
    }
}

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
    /* 0x18 */ u8 pad18[0x34 - 0x18];
    /* 0x34 */ f32 unk34;
    /* 0x38 */ u8 pad38;
    /* 0x39 */ u8 opacity;
    /* 0x3A */ u8 pad3A[0x3C - 0x3A];
    /* 0x3C */ void *level_entry;
    /* 0x40 */ struct ObjectHeader12 *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ s16 objectID;
    /* 0x4C */ u8 pad4C[0x68 - 0x4C];
    /* 0x68 */ Object_Racer *racer;
    /* 0x6C */ u8 pad6C[0x88 - 0x6C];
    /* 0x88 */ f32 radius; // DKR: properties.distance.radius
    /* 0x8C */ s32 unk8C;
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

Camera *cam_get_active_camera(void);
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
void func_80041168(Gfx **dList, Mtx **matrices); // DKR: mtx_world_origin
void func_80055C00(Gfx **dList, void *tex, s32 flags, s32 texOffset); // DKR: material_set
extern u8 D_800A0D90[]; // DKR: D_800DC92C, the 8 triangles' vertex indices

/**
 * DKR trackbg_render_flashy (tracks.c): a flat textured plane around the camera, fading out at the edges.
 * JFG: the plane's texture and scales from the level header's sky fields, white prim/env colours, no reset.
 */
void trackbg_render_flashy() {
    Triangle *tris;
    Vertex *verts;
    s32 vCoordMask;
    s32 uCoordMask;
    f32 scaledXSin;
    f32 scaledXCos;
    f32 var_f16;
    s16 uCoords[9];
    s16 vCoords[9];
    f32 xCos;
    f32 xSin;
    f32 pad_sp108;
    Camera *camera;
    f32 pad_sp100;
    f32 xPositions[9];
    f32 zPositions[9];
    f32 pos[3]; // DKR: Vec3f
    s32 i;
    s32 var_v0;
    s32 var_v1;
    s32 var_a1;
    s32 var_a2;
    u8 *var_v0_3;
    f32 var_f14;
    s16 vertY;
    UNUSED s16 vTempCoord;
    UNUSED s16 uTempCoord;
    TextureHeader *texHeader;
    UNUSED s32 pad[4];

    verts = D_800F2F78;
    tris = D_800F2F7C;

    camera = cam_get_active_camera();
    texHeader = D_800A0D64->skyTex;
    D_800F2FAC = -1;
    uCoordMask = (texHeader->width << 5) - 1;
    vCoordMask = (texHeader->height << 5) - 1;
    xSin = func_80049C34(-camera->trans.rotation[0]);
    xCos = func_80049C30(-camera->trans.rotation[0]);

    scaledXSin = xSin * 1280.0f;
    scaledXCos = xCos * 1280.0f;
    pad_sp100 = 2.0f * scaledXSin;
    xPositions[0] = -scaledXCos - (xSin * 1280.0f);
    zPositions[0] = -scaledXCos + (xSin * 1280.0f);
    xPositions[1] = scaledXCos - (xSin * 1280.0f);
    zPositions[1] = -scaledXCos - (xSin * 1280.0f);
    xPositions[2] = scaledXCos + scaledXSin;
    zPositions[2] = scaledXCos - (xSin * 1280.0f);
    xPositions[3] = -scaledXCos + (xSin * 1280.0f);
    zPositions[3] = scaledXCos + (xSin * 1280.0f);
    xPositions[4] = 0.0f;
    zPositions[4] = 0.0f;

    xPositions[5] = -(xCos * 1280.0f) - (2.0f * scaledXSin);
    zPositions[5] = scaledXSin + -(2.0f * (xCos * 1280.0f));
    xPositions[6] = (xCos * 1280.0f) - (2.0f * scaledXSin);
    zPositions[6] = -(2.0f * (xCos * 1280.0f)) - scaledXSin;
    xPositions[7] = (xCos * 1280.0f) + (2.0f * scaledXSin);
    zPositions[7] = (2.0f * (xCos * 1280.0f)) - scaledXSin;
    xPositions[8] = -(xCos * 1280.0f) + (2.0f * scaledXSin);
    zPositions[8] = (2.0f * (xCos * 1280.0f)) + scaledXSin;

    scaledXCos = 1280.0f;
    var_f14 = scaledXCos * 0.25f;

    var_a1 = texHeader->width * 16 * D_800A0D64->skyScaleU;
    var_a2 = texHeader->height * 16 * D_800A0D64->skyScaleV;

    var_v0 = ((s32) (camera->trans.x_position * ((scaledXCos * 0.25f) / var_a1)) + (D_800A0D64->skyOffsetU >> 4)) &
             uCoordMask;
    var_v1 = ((s32) (camera->trans.z_position * ((scaledXCos * 0.25f) / var_a2)) + (D_800A0D64->skyOffsetV >> 4)) &
             vCoordMask;

    var_f14 = var_a1 * xCos;
    pos[2] = var_a1 * xCos;
    pos[0] = var_a1 * xCos;
    var_f16 = var_a2 * xSin;
    xCos = var_f16;
    pad_sp108 = var_f16;

    // @fake
    var_a2 = texHeader->height * 16 * D_800A0D64->skyScaleV;

    uCoords[0] = (s16) (-var_f14 - pad_sp108) + var_v0;
    vCoords[0] = (s16) (var_f16 - var_f14) + var_v1;
    uCoords[1] = (s16) (var_f14 - pad_sp108) + var_v0;
    vCoords[1] = (s16) (-var_f14 - var_f16) + var_v1;
    uCoords[2] = (s16) (var_f14 + var_f16) + var_v0;
    vCoords[2] = (s16) (var_f14 - var_f16) + var_v1;
    uCoords[3] = (s16) (var_f16 - var_f14) + var_v0;
    vCoords[3] = (s16) (var_f14 + var_f16) + var_v1;

    uCoords[4] = var_v0;
    vCoords[4] = var_v1;

    uCoords[5] = (s16) (-var_f14 - (2.0f * xCos)) + var_v0;
    vCoords[5] = (s16) (var_f16 - (2.0f * var_f14)) + var_v1;
    uCoords[6] = (s16) (var_f14 - (2.0f * xCos)) + var_v0;
    vCoords[6] = (s16) ((-(2.0f * var_f14)) - var_f16) + var_v1;
    uCoords[7] = (s16) (pos[2] + (2.0f * xCos)) + var_v0;
    vCoords[7] = (s16) ((2.0f * pos[0]) - var_f16) + var_v1;
    uCoords[8] = (s16) ((2.0f * xCos) - pos[2]) + var_v0;
    vCoords[8] = (s16) ((2.0f * pos[0]) + var_f16) + var_v1;

    func_80041168(&D_800F2F70, &D_800F2F74);
    func_80055C00(&D_800F2F70, texHeader, 0x10, D_800F2FA4 << 8);
    gDPSetPrimColor(D_800F2F70++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(D_800F2F70++, 255, 255, 255, 255);
    gSPVertexLast(D_800F2F70++, OS_K0_TO_PHYSICAL(D_800F2F78), 9, 0);
    gSPPolygon(D_800F2F70++, OS_K0_TO_PHYSICAL(D_800F2F7C), 8, 1);
    gDPPipeSync(D_800F2F70++);

    vertY = camera->trans.y_position + 192.0f;
    for (i = 0; i < 9; i++) {
        verts->x = xPositions[i] + camera->trans.x_position;
        verts->y = vertY;
        verts->z = zPositions[i] + camera->trans.z_position;
        verts->r = 0xFF;
        verts->g = 0xFF;
        verts->b = 0xFF;
        verts->a = (i <= 4) ? (255) : (0);
        verts++;
    }

    var_v0_3 = D_800A0D90;
    for (i = 0; i < 8; i++) {
        tris->flags = 0x40; // DKR: BACKFACE_DRAW
        tris->vi0 = *var_v0_3;
        tris->uv[0][0] = uCoords[*var_v0_3];
        tris->uv[0][1] = vCoords[*var_v0_3];
        var_v0_3 += 1;
        tris->vi1 = *var_v0_3;
        tris->uv[1][0] = uCoords[*var_v0_3];
        tris->uv[1][1] = vCoords[*var_v0_3];
        var_v0_3 += 1;
        tris->vi2 = *var_v0_3;
        tris->uv[2][0] = uCoords[*var_v0_3];
        tris->uv[2][1] = vCoords[*var_v0_3];
        var_v0_3 += 1;
        tris++;
    }

    D_800F2F78 = verts;
    D_800F2F7C = tris;
}

/* JFG: an object's header, reduced */
typedef struct ObjectHeader12 {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 drawDistance;
    /* 0x18 */ u8 pad18[0xA2 - 0x18];
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
void viewport_apply(Gfx **dList, Mtx **mtx);
void fb_size(u32 *width, u32 *height);
s32 get_current_viewport(void);
s32 cam_get_active_camera_id();
void copy_framebuffer_size_to_coords(s32 layout, s32 viewPortIndex, u32 *ulx, u32 *uly, u32 *lrx, u32 *lry);

/* JFG: draw the level's background as a gradient from its top colour to its bottom colour.
   (width is an s32 halved as unsigned: the s32 type gives the target's register numbering.) */
void func_80013454(void) {
    s32 width;
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
    viewport_apply(&D_800F2F70, &D_800F2F74);
    topR = D_800A0D64->bgTopR;
    topG = D_800A0D64->bgTopG;
    topB = D_800A0D64->bgTopB;
    bottomR = D_800A0D64->bgBottomR;
    bottomG = D_800A0D64->bgBottomG;
    bottomB = D_800A0D64->bgBottomB;
    fb_size((u32 *) &width, &height);
    copy_framebuffer_size_to_coords(get_current_viewport(), cam_get_active_camera_id(), &x1, &y1, &x2, &y2);
    width = (u32) width >> 1;
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

Camera *cam_get_active_camera(void);
void func_80041168(Gfx **dList, Mtx **matrices); // DKR: mtx_world_origin
void object_uv_offsets_update(Object *obj, s32 updateRate);
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
            object_uv_offsets_update(gSkydomeSegment, updateRate);
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
void tex_scroll_queue(s32 arg0, s32 arg1, s32 arg2) {
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
/* JFG: scroll the texture coordinates of the triangles using each queued texture (tex_scroll_queue).
   NON_MATCHING: the same code with other registers (target: outer values in t-registers, the
   per-triangle values in s3..fp, no $ra use). `value` (dead after the entry is decoded) carrying the batch's
   face offset gives the ROM's length and frame (129 words, 0x28; aligned distance 183 -> 65). The second,
   no-code `& 0xFF` on the texture index (a no-op on a byte) takes the code generator's extra temporary
   (65 -> 48). */
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
                texIndex = ((value >> 24) & 0xFF) & 0xFF;
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
                            value = batch->facesOffset;
                            tri = &segment->triangles[value];
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

/* JFG: allocate count track lights (all free) and, if asked, a copy of every segment's vertices. */
void track_segment_data_alloc(s32 count, s32 copyVertices) {
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
                i = 0;
                failed = FALSE;
                for (; i < gCurrentLevelModel->numberOfSegments; i++) {
                    src = (u8 *) segment->vertices;
                    size = segment->numberOfVertices * sizeof(Vertex);
                    alloc->vertices = (void *) src;
                    alloc->data = mempool_alloc_safe(size, 0xFF0000FF);
                    dst = alloc->data;
                    if (dst != NULL) {
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

void track_light_set_colour(u8 *light, s32 red, s32 green, s32 blue, s32 brightness);

/* JFG: add a track light in the first free slot (0x80013E0C). */
TrackLight *track_light_create(f32 x, f32 y, f32 z, f32 outerRadius, f32 innerRadius, s32 red, s32 green, s32 blue) {
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
            light->z = z;
            light->outerRadius = outerRadius;
            light->outerRadiusSquare = outerRadius * outerRadius;
            light->innerRadius = innerRadius;
            light->innerRadiusSquare = innerRadius * innerRadius;
            light->falloff = 31.99f / (outerRadius - innerRadius);
            track_light_set_colour((u8 *) light, red, green, blue, 255);
            gNumTrackLights++;
            return light;
        }
        light++;
    }
    return NULL;
}

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

/* JFG: copy the vertex colours of the drawn segments into their vertices (all of them while
   D_800A0D80 is set, else only the changed ones). */
void segment_vertex_colours_copy(void) {
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
if (*visible && (segment->colourFlags & dir) && (colours = segment->colours) != NULL) {
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
        visible++;
    }
}

extern u8 D_800A4FCC;  // JFG: the number of cameras
extern s16 D_80104704; // the vertex colour level (m046)
extern s16 D_801046FC; // its time left (m046)
void func_8001D500(LevelModel *model, TrackLight *light, u8 *visible);
void segment_vertex_colours_copy(void);

/* JFG: flip the segments' double-buffered vertices, update their colours (a module's or segment_vertex_colours_copy)
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
        segment_vertex_colours_copy();
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
struct WaterTexture;
void track_texture_load(struct WaterTexture *tex, s32 texOffset, s32 flags);

/**
 * Render a group of a level segment's batches: pass 0x10000000 draws the opaque ones (without Z compare), the
 * others the batches whose flags match (4 and 0x20000000 the second group).
 * JFG: environment colour from the texture offset for some textures, flag 0x4000 for a special material.
 * DKR: render_level_segment
 * (The empty `if (!texture) {}` keeps the batch pointer and texture in the ROM's saved registers; permuter.)
 */
void render_level_segment(s32 segmentId, s32 pass) {
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
            if (!texture) {}
            gDPSetEnvColor(D_800F2F70++, 255, 255, 255, 0);
        }
        if (!(batchFlags & 0x180)) {
            batchFlags |= D_800F2FA0;
        }
        special = batchFlags & 0x4000;
        if (special && texture != NULL) {
            track_texture_load((struct WaterTexture *) texture, texOffset, batchFlags);
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
 * JFG: inclusive bounds, a smaller starting distance; x is converted once, before the loop.
 * Official Name: trackGetBlock
 */
s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos) {
    LevelModelSegmentBoundingBox *bb;
    s32 i;
    s32 minVal;
    s32 result;
    s32 heightDiff;
    s32 x1, x2, z1, z2;
    s32 x;
    LevelModel *model;

    result = -1;
    if (gCurrentLevelModel != NULL) {
        model = gCurrentLevelModel;
        minVal = 0x7FFF;
        x = xPos;
        bb = model->segmentsBoundingBoxes;
        for (i = 0; i < model->numberOfSegments; i++, bb++) {
            x1 = bb->x1;
            x2 = bb->x2;
            z1 = bb->z1;
            z2 = bb->z2;
            if (x <= x2 && x >= x1 && (s32) zPos <= z2 && (s32) zPos >= z1) {
                heightDiff = (bb->y1 + bb->y2) >> 1;
                heightDiff -= (s32) yPos;
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
/* The current camera. Defined with no parameters (40240.c), but called below with two it ignores: the ROM's
   code needs the two-argument call (not portable C; a1 is set to the racer for nothing). */
s32 cam_get_active_camera_id();

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

/* JFG: the object's 0x68 block as the draw range check reads it (it differs per object kind) */
typedef struct DrawRangeData {
    /* 0x00 */ u8 pad0[7];
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 pad9[0x24 - 0x9];
    /* 0x24 */ Object *rider;
    /* 0x28 */ u8 pad28[0x55 - 0x28];
    /* 0x55 */ u8 unk55;
    /* 0x56 */ u8 pad56[0x90 - 0x56];
    /* 0x90 */ s16 unk90;
} DrawRangeData;

#ifdef NON_MATCHING
/* NON_MATCHING: same length; the switch cases from 0x2B on use other t-registers, and the plane loop is entered
 * through a second inRange test (the target tests it once, before the loads). The (u16)/(s16) casts on two
 * opacity stores (a u8 field: the stored low byte is the same) took the aligned distance 73 -> 53. */
/**
 * Sets the object's opacity for its kind, fades it out towards the end of its draw distance, and tells whether it is
 * in range and in front of the three view planes.
 * JFG: per-kind opacities (racers through player_get_draw_alpha, others from their data), a 30% fade band, and the
 * plane test stops at the first failure.
 * DKR: check_if_in_draw_range
 */
s32 func_80015ED8(Object *obj) {
    s32 viewDistance;
    s32 inRange;
    DrawRangeData *data;
    s32 alpha;
    s32 value;
    f32 dist;
    f32 fadeDist;
    f32 fade;
    s32 i;
    f32 py;
    f32 pz;
    f32 pw;
    UNUSED s32 pad;
    f32 y;
    f32 z;
    f32 px;
    f32 w;

    inRange = TRUE;
    switch (obj->behaviorId) {
        case 2:
            data = (DrawRangeData *) obj->racer;
            obj->opacity = data->unk90;
            if (data->rider != NULL) {
                alpha = player_get_draw_alpha(data->rider);
                if (alpha != 255) {
                    obj->opacity = (obj->opacity * alpha) >> 8;
                }
            }
            break;
        case 1:
            obj->opacity = player_get_draw_alpha(obj);
            break;
        case 0x26:
            data = (DrawRangeData *) obj->racer;
            obj->opacity = data->unk8;
            break;
        case 0x18:
            obj->opacity = (u16) (255 - ((DrawRangeData *) obj->racer)->unk55);
            break;
        case 0x2B:
        case 0x3D:
        case 0x50:
        case 0x51:
            obj->opacity = obj->unk8C;
            break;
        case 0x42:
            obj->opacity = (s16) (func_80054530(obj, cam_get_active_camera_id()));
            break;
        case 0x55:
        case 0x56:
            obj->opacity = obj->unk8C;
            break;
        case 0x5E:
            obj->opacity = obj->unk8C;
            break;
        case 0x54:
        case 0xA8:
            obj->opacity = ((DrawRangeData *) obj->racer)->unk7;
            break;
        case 0x66:
        case 0x67:
            obj->opacity = obj->unk8C;
            break;
        case 0x6E:
            value = obj->unk8C;
            if (value >= 255) {
                obj->opacity = 255;
            } else {
                obj->opacity = value;
            }
            break;
        case 0x7A:
        case 0x7B:
        case 0x7E:
        case 0x86:
        case 0xA6:
        case 0xA7:
            obj->opacity = obj->unk8C;
            break;
        case 0x24:
        case 0x2E:
        case 0x2F:
        case 0x30:
        case 0x31:
        case 0x33:
        case 0x40:
        case 0x52:
        case 0x71:
        case 0x8A:
        case 0xA0:
        case 0xAB:
            break;
        default:
            obj->opacity = 255;
            break;
    }
    if (obj->opacity == 0) {
        return FALSE;
    }
    viewDistance = obj->header->drawDistance;
    if (viewDistance != 0) {
        dist = get_distance_to_active_camera(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        if (viewDistance < dist) {
            inRange = FALSE;
        } else {
            fadeDist = viewDistance - dist;
            fade = viewDistance * 0.3f;
            if (fadeDist < fade) {
                obj->opacity = (s32) (obj->opacity * (fadeDist / fade));
            }
        }
    }
    if (inRange) {
        y = obj->trans.y_position;
        z = obj->trans.z_position;
        w = obj->unk34;
        for (i = 0; inRange && i < 3; i++) {
            px = D_800F2FB8[i].x;
            py = D_800F2FB8[i].y;
            pz = D_800F2FB8[i].z;
            pw = D_800F2FB8[i].w;
            if ((px * obj->trans.x_position) + (py * y) + (pz * z) + pw + w < 0.0f) {
                inRange = FALSE;
            }
        }
    }
    return inRange;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80015ED8);
#endif

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
/* NON_MATCHING (33 words, from 116, after the empty ifs on always-set values below): the target copies flags to $s0 (in the delay slot of the first branch) and takes the 0x40 test
   from that copy, which also moves the hit-field stores after the float stores; IDO propagates my copies away */
void track_edges_build(void);
void func_800805F4(void);
// the callee reads flags as u16 (lhu), but a u16 parameter here makes this caller mask the argument (126 words)
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
        if (extra) {}
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
    hit->unk1C = 1024000000.0f;
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
    if (obj) {}
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80016208);
#endif

#ifdef NON_MATCHING
void track_edges_build(void);
void func_800805F4(void);
s32 func_8007F280(void *obj, f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags, f32 radius2);
s32 func_80017094(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags, f32 radius2);
s32 func_8007FC0C(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags);
s32 func_80017794(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags);

/*
 * JFG: func_80016208 for `count` points at once: sweep each point from prev to points (its radius and flags; flag
 * 0x40 skips it) against the objects and the level, repeating while something is hit (up to 11 times; then the
 * point goes back to prev), summing the hit normals into push and noting in *nearest the last point whose hit had
 * surface 3. With offsets, the point that touched first (the smallest hit distance) places pos (its point less its
 * offset) and every point is put back at pos plus its offset. All of it repeats while something was touched: after
 * 11 rounds (or when one hit was both a floor/wall and a slope/ceiling kind) every point goes back to prev.
 * Returns the touch masks of every round or-ed, with 0xFFFF0000 (gave up) or 0x11110000 (stuck) added.
 * As in the ROM, best keeps its last value when no touched point's distance is below the start value.
 * NON_MATCHING: first body (written from the ROM; checked equal to it by simulation).
 */
s32 func_800163FC(void *obj, f32 *pos, f32 *prev, f32 *points, f32 *radii, f32 *offsets, u16 *flags, SweepHit *hits,
                  s32 count, s32 *nearest, f32 *push) {
    s32 j;
    s32 a;
    s32 b;
    f32 min;
    s32 bit;
    s32 touched;
    s32 found;
    s32 n;
    s32 i;
    SweepHit *hit;
    f32 *from;
    s32 result;
    f32 *to;
    s32 tries;
    s32 best;
    s32 done;
    f32 radius;
    s32 f;
    UNUSED s32 pad0; // the frame has four more words here
    UNUSED s32 pad1;
    UNUSED s32 pad2;
    UNUSED s32 pad3;

    push[0] = 0.0f;
    push[1] = 0.0f;
    push[2] = 0.0f;
    *nearest = -1;
    for (j = 0; j < count; j++) {
        hits[j].obj = NULL;
        hits[j].unk25 = 0;
        hits[j].pos[0] = 0.0f;
        hits[j].pos[1] = 0.0f;
        hits[j].pos[2] = 0.0f;
        hits[j].normal[0] = 0.0f;
        hits[j].normal[1] = 0.0f;
        hits[j].normal[2] = 0.0f;
        hits[j].unk1C = 1024000000.0f;
        hits[j].unk24 = 0;
        hits[j].unk20 = 0;
        hits[j].unk26 = 0;
        hits[j].unk27 = 0;
    }
    min = 1024000000.0f;
    track_edges_build();
    func_800805F4();
    result = 0;
    tries = 0;
    done = 0;
    do {
        i = 0;
        touched = 0;
        bit = 1;
        do {
            f = flags[i];
            radius = radii[i];
            n = 0;
            if (!(f & 0x40)) {
                hit = &hits[i];
                from = &prev[i * 3];
                to = &points[i * 3];
                do {
                    found = FALSE;
                    if (func_8007F280(obj, from, to, hit, radius, f, 0.0f)) {
                        push[0] += hit->normal[0];
                        push[1] += hit->normal[1];
                        push[2] += hit->normal[2];
                        touched |= bit;
                        found = TRUE;
                    }
                    if (func_80017094(from, to, hit, radius, f, 0.0f)) {
                        push[0] += hit->normal[0];
                        push[1] += hit->normal[1];
                        push[2] += hit->normal[2];
                        touched |= bit;
                        found = TRUE;
                    }
                    if (func_8007FC0C(from, to, hit, radius, f)) {
                        push[0] += hit->normal[0];
                        push[1] += hit->normal[1];
                        push[2] += hit->normal[2];
                        touched |= bit;
                        found = TRUE;
                    }
                    if (func_80017794(from, to, hit, radius, f)) {
                        push[0] += hit->normal[0];
                        push[1] += hit->normal[1];
                        push[2] += hit->normal[2];
                        touched |= bit;
                        found = TRUE;
                    }
                    if (found) {
                        n++;
                        if (hit->unk24 == 3) {
                            *nearest = i;
                        }
                        if (n >= 11) {
                            found = FALSE;
                            to[0] = from[0];
                            to[1] = from[1];
                            to[2] = from[2];
                            hit->unk1C = 0.0f;
                            hit->unk25 |= 0x80;
                        } else if ((hit->unk25 & 0x12) && (hit->unk25 & 0x48)) {
                            found = FALSE;
                            to[0] = from[0];
                            to[1] = from[1];
                            to[2] = from[2];
                            hit->unk1C = 0.0f;
                        }
                    }
                } while (found && !(f & 0x40));
            }
            i++;
            bit <<= 1;
        } while (i < count);
        if (touched && offsets != NULL) {
            bit = 1;
            radius = min;
            for (j = 0; j < count; j++) {
                if ((touched & bit) && hits[j].unk1C < radius) {
                    best = j;
                    radius = hits[j].unk1C;
                }
                bit <<= 1;
            }
            hits[best].unk25 |= 1;
            pos[0] = points[best * 3 + 0] - offsets[best * 3 + 0];
            pos[1] = points[best * 3 + 1] - offsets[best * 3 + 1];
            pos[2] = points[best * 3 + 2] - offsets[best * 3 + 2];
            for (j = 0; j < count; j++) {
                points[j * 3 + 0] = offsets[j * 3 + 0] + pos[0];
                points[j * 3 + 1] = offsets[j * 3 + 1] + pos[1];
                points[j * 3 + 2] = offsets[j * 3 + 2] + pos[2];
            }
        }
        tries++;
        result |= touched;
        if (tries >= 11) {
            touched = 0;
            for (j = 0; j < count; j++) {
                points[j * 3 + 0] = prev[j * 3 + 0];
                points[j * 3 + 1] = prev[j * 3 + 1];
                points[j * 3 + 2] = prev[j * 3 + 2];
            }
            result |= 0xFFFF0000;
        } else {
            a = 0;
            b = 0;
            for (j = 0; j < count; j++) {
                if (hits[j].unk25 & 0x48) {
                    a = 1;
                }
                if (hits[j].unk25 & 0x12) {
                    b = 1;
                }
            }
            if (a && b) {
                done = 1;
                for (j = 0; j < count; j++) {
                    if (hits[j].unk25) {
                        hits[j].unk25 |= 1;
                    }
                    points[j * 3 + 0] = prev[j * 3 + 0];
                    points[j * 3 + 1] = prev[j * 3 + 1];
                    points[j * 3 + 2] = prev[j * 3 + 2];
                }
                result |= 0x11110000;
            }
        }
    } while (touched && !done);
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_800163FC);
#endif

/* JFG: a triangle's entry in the segment's plane index (8 bytes; DKR's CollisionFacetPlanes) */
typedef struct TrianglePlaneRef {
    /* 0x00 */ u16 plane;    // the triangle's own plane
    /* 0x02 */ u16 edges[3]; // its edges' planes; bit 15: the inside is the plane's positive side
} TrianglePlaneRef;

/* JFG: a plane a*x + b*y + c*z + d = 0 */
typedef struct Plane {
    /* 0x00 */ f32 a, b, c, d;
} Plane;

#ifdef NON_MATCHING
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
s32 func_80049C84(f32 x, f32 z);
f32 sqrtf(f32 x);
extern s32 *D_800F33B8;
extern s16 *D_800F33BC;
extern s32 D_800F33C4;

/**
 * JFG: sweep a sphere of the given radius from `from` to `to` against the candidate triangles (D_800F33B8): the
 * first triangle whose plane `to` ends up behind (and `from` in front of, by the radius) with the crossing point
 * inside its edges (within radius2) stops the sweep. `to` is pushed back out of it: along the normal with flag 1
 * or 0x80 (flag 1 by the radius plus 0.01), else up onto a floor (normal y >= 0.707), out along the normal under a
 * ceiling (y <= -0.866), or for a wall horizontally by the depth over the slope's cosine. hit gets the squared
 * length of the move to the crossing, the plane's normal (also in pos with flag 0x81 or a floor), the kind (unk25:
 * 2 floor, 4 wall, 8 ceiling), the batch's surface type and flags. Returns 1 on a hit. As in the ROM, segment is
 * only set by an earlier entry.
 * NON_MATCHING: a first body written from the ROM (443 of 448 words, frame 0xC8 of 0x110); checked equal to the
 * ROM by simulation. The ROM keeps from[] and the plane values in more spill slots (0x68-0x84) and its frame has
 * 18 more words between mz and them; the registers differ from the first block on.
 */
s32 func_80017094(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags, f32 radius2) {
    s32 result;
    LevelModelSegment *segment;
    TrianglePlaneRef *ref;
    f32 a;
    f32 b;
    f32 c;
    f32 d;
    Plane *planes;
    Plane *plane;
    f32 distTo;
    f32 distFrom;
    f32 t;
    f32 px;
    f32 py;
    f32 pz;
    s32 j;
    s32 inside;
    f32 mx;
    f32 my;
    f32 mz;
    s32 edge;
    s32 flip;
    f32 dist;
    f32 side;
    s16 angle;
    f32 cs;
    f32 len;
    s32 i;

    result = 0;
    for (i = 0; i < D_800F33C4; i++) {
        if (D_800F33B8[i] > 0) {
            segment = (LevelModelSegment *) (D_800F33B8[i] | 0x80000000);
        } else {
            ref = (TrianglePlaneRef *) (u32) D_800F33B8[i]; // NOT PORTABLE: the list holds 32-bit addresses
            planes = segment->planes;
            plane = &planes[ref->plane];
            a = plane->a;
            b = plane->b;
            c = plane->c;
            d = plane->d - radius;
            distTo = a * to[0] + b * to[1] + c * to[2] + d;
            if (distTo < 0.0f) {
                distFrom = a * from[0] + b * from[1] + c * from[2] + d;
                if (distFrom >= 0.0f) {
                    t = distFrom / (distFrom - distTo);
                    mx = (to[0] - from[0]) * t;
                    my = (to[1] - from[1]) * t;
                    mz = (to[2] - from[2]) * t;
                    px = from[0] + mx - radius * a;
                    py = from[1] + my - radius * b;
                    pz = from[2] + mz - radius * c;
                    inside = TRUE;
                    for (j = 0; j < 3 && inside; j++) {
                        edge = ref->edges[j];
                        flip = edge & 0x8000;
                        edge ^= flip;
                        plane = &planes[edge];
                        dist = plane->a * px + plane->b * py + plane->c * pz + plane->d;
                        side = dist;
                        if (flip) {
                            side = -dist;
                        }
                        if (radius2 < side) {
                            inside = FALSE;
                        }
                    }
                    if (inside) {
                        result = 1;
                        hit->unk1C = mx * mx + my * my + mz * mz;
                        if ((u16) flags & 0x81) { // the ROM reads the argument's low half
                            if ((u16) flags & 1) {
                                radius += 0.01f;
                                to[0] = radius * a + px;
                                to[1] = radius * b + py;
                                to[2] = radius * c + pz;
                            } else {
                                distTo = 0.01f - distTo;
                                to[0] += distTo * a;
                                to[1] += distTo * b;
                                to[2] += distTo * c;
                            }
                            hit->pos[1] = b;
                            hit->pos[0] = a;
                            hit->pos[2] = c;
                            if (b >= 0.707f) {
                                hit->unk25 |= 2;
                            } else if (b <= -0.866f) {
                                hit->unk25 |= 8;
                            } else {
                                hit->unk25 |= 4;
                            }
                        } else if (b >= 0.707f) {
                            to[1] = -(to[2] * c + a * to[0] + d) / b + 0.01f;
                            hit->pos[1] = b;
                            hit->unk25 |= 2;
                            hit->pos[2] = c;
                            hit->pos[0] = a;
                        } else if (b <= -0.866f) {
                            distTo = 0.01f - distTo;
                            to[0] += distTo * a;
                            to[1] += distTo * b;
                            to[2] += distTo * c;
                            hit->unk25 |= 8;
                        } else {
                            distFrom = -distTo;
                            px = to[0] + distFrom * a;
                            py = to[1] + distFrom * b;
                            pz = to[2] + distFrom * c;
                            mx = to[0] - px;
                            mz = to[2] - pz;
                            my = to[1] - py;
                            angle = func_80049C84(my, sqrtf(mx * mx + mz * mz));
                            cs = func_80049C30(angle);
                            if (cs != 0.0f) {
                                distTo = (distFrom + 0.01f) / cs;
                                len = sqrtf(a * a + c * c);
                                to[0] += distTo * (a / len);
                                to[2] += distTo * (c / len);
                            } else {
                                to[0] = px;
                                to[1] = py;
                                to[2] = pz;
                            }
                            hit->unk25 |= 4;
                        }
                        hit->normal[1] = b;
                        hit->normal[0] = a;
                        hit->normal[2] = c;
                        hit->unk24 = gCurrentLevelModel->textures[segment->batches[D_800F33BC[i]].textureIndex].surfaceType;
                        hit->unk20 = segment->batches[D_800F33BC[i]].flags;
                        i = D_800F33C4;
                    }
                }
            }
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80017094);
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

#ifdef NON_MATCHING
extern TrackEdge *D_800F33B0;
extern s32 D_800F33B4;
s32 func_80018024(f32 *start, f32 *dir, f32 *base, f32 *axis, f32 radius, f32 *in, f32 *out);
s32 func_800183D4(f32 *origin, f32 *dir, f32 *centre, f32 radius, f32 *tNear, f32 *tFar);

/**
 * JFG: func_80017094 for the edge list (D_800F33B0, track_edges_build): sweep a sphere of the given radius from
 * `from` to `to` against each edge, as a cylinder around it (func_80018024; the hit must fall within the edge) or
 * else a sphere at either end (func_800183D4). The first edge hit within the move stops the sweep: the contact
 * normal (from the edge to the sphere's centre at contact, over the radius) is the plane `to` is pushed out of,
 * as in func_80017094 (flag 1 puts `to` at the contact centre instead). hit gets the squared distance to the
 * contact, the normal, the kind (unk25), the edge's batch surface type and flags, and the edge's unk2E (unk26).
 * Returns 1 on a hit.
 * NON_MATCHING: a first body written from the ROM (552 of 548 words, the ROM's frame 0x108); checked equal to the
 * ROM by simulation. The registers differ from the first statement on (the ROM's dx is f2, ours f16).
 */
s32 func_80017794(f32 *from, f32 *to, SweepHit *hit, f32 radius, s32 flags) {
    s32 i;
    f32 in;
    f32 out;
    f32 dir[3];
    f32 len;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 t;
    f32 d;
    f32 wz;
    f32 distTo;
    f32 wx;
    f32 wy;
    f32 px;
    f32 py;
    f32 pz;
    s16 angle;
    f32 cs;
    TrackEdge *edge;
    s32 found;

    dx = to[0] - from[0];
    dy = to[1] - from[1];
    dz = to[2] - from[2];
    len = dx * dx + dy * dy + dz * dz;
    if (len > 0.0f) {
        len = sqrtf(len);
        dir[0] = dx / len;
        dir[1] = dy / len;
        dir[2] = dz / len;
    } else {
        return 0;
    }
    found = 0;
    for (i = 0; i < D_800F33B4; i++) {
        edge = &D_800F33B0[i];
        if (func_80018024(from, dir, &edge->x0, &edge->dirX, radius, &in, &out) && in >= 0.0f && in <= len) {
            px = dir[0] * in + from[0];
            py = dir[1] * in + from[1];
            pz = dir[2] * in + from[2];
            dx = px - edge->x0;
            dy = py - edge->y0;
            dz = pz - edge->z0;
            t = (dx * edge->dirX + dy * edge->dirY + dz * edge->dirZ) /
                (edge->dirX * edge->dirX + edge->dirY * edge->dirY + edge->dirZ * edge->dirZ);
            if (t >= 0.0f && t <= edge->length) {
                found = 1;
                dx = edge->dirX * t + edge->x0;
                dy = edge->dirY * t + edge->y0;
                dz = edge->dirZ * t + edge->z0;
                nx = px - dx;
                nz = pz - dz;
                ny = py - dy;
            }
        }
        if (!found && func_800183D4(from, dir, &edge->x0, radius, &in, &out) && in >= 0.0f && in <= len) {
            found = 1;
            px = dir[0] * in + from[0];
            py = dir[1] * in + from[1];
            pz = dir[2] * in + from[2];
            nx = px - edge->x0;
            ny = py - edge->y0;
            nz = pz - edge->z0;
        }
        if (!found && func_800183D4(from, dir, &edge->x1, radius, &in, &out) && in >= 0.0f && in <= len) {
            found = 1;
            px = dir[0] * in + from[0];
            py = dir[1] * in + from[1];
            pz = dir[2] * in + from[2];
            nx = px - edge->x1;
            ny = py - edge->y1;
            nz = pz - edge->z1;
        }
        if (found) {
            nz /= radius;
            nx /= radius;
            ny /= radius;
            d = -(px * nx + py * ny + pz * nz);
            distTo = nx * to[0] + ny * to[1] + nz * to[2] + d;
            if ((u16) flags & 0x81) { // the ROM reads the argument's low half
                if ((u16) flags & 1) {
                    to[0] = px;
                    to[1] = py;
                    to[2] = pz;
                } else {
                    distTo = 0.01f - distTo;
                    to[0] += distTo * nx;
                    to[1] += distTo * ny;
                    to[2] += distTo * nz;
                }
                hit->pos[1] = ny;
                hit->pos[0] = nx;
                hit->pos[2] = nz;
                if (ny >= 0.707f) {
                    hit->unk25 |= 2;
                } else if (ny <= -0.866f) {
                    hit->unk25 |= 8;
                } else {
                    hit->unk25 |= 4;
                }
            } else if (ny >= 0.707f) {
                to[1] = -(to[2] * nz + nx * to[0] + d) / ny + 0.01f;
                hit->pos[1] = ny;
                hit->pos[0] = nx;
                hit->unk25 |= 2;
                hit->pos[2] = nz;
            } else if (ny <= -0.866f) {
                distTo = 0.01f - distTo;
                to[0] += distTo * nx;
                to[1] += distTo * ny;
                to[2] += distTo * nz;
                hit->unk25 |= 8;
            } else {
                d = -distTo;
                wx = to[0] + d * nx;
                wy = to[1] + d * ny;
                wz = to[2] + d * nz;
                dx = to[0] - wx;
                dz = to[2] - wz;
                dy = to[1] - wy;
                angle = func_80049C84(dy, sqrtf(dx * dx + dz * dz));
                cs = func_80049C30(angle);
                if (cs != 0.0f) {
                    distTo = (d + 0.01f) / cs;
                    t = sqrtf(nx * nx + nz * nz);
                    px = nx / t;
                    pz = nz / t;
                    to[0] += distTo * px;
                    to[2] += distTo * pz;
                } else {
                    to[0] = wx;
                    to[1] = wy;
                    to[2] = wz;
                }
                hit->unk25 |= 4;
            }
            hit->unk1C = in * in;
            hit->normal[1] = ny;
            hit->normal[0] = nx;
            hit->normal[2] = nz;
            hit->unk24 = gCurrentLevelModel->textures[edge->segment->batches[edge->batch].textureIndex].surfaceType;
            hit->unk20 = edge->segment->batches[edge->batch].flags;
            hit->unk26 = edge->unk2E;
            i = D_800F33B4;
        }
    }
    return found;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80017794);
#endif

f32 sqrtf(f32 x);

#ifdef NON_MATCHING
/**
 * NON_MATCHING: ROM length and frame, 152 aligned diffs: f4 and f8 swapped from the first statement on (the
 * target loads base[0] into f8). The cross products are written as Graphics Gems' V3Cross (a.y*b.z - a.z*b.y,
 * which gives the target's operand order) and the denominator is its own local (the target spills it at 0x24
 * around the last sqrtf).
 * JFG: intersect a ray (start, unit direction dir) with an infinite cylinder (a point on its axis, unit axis
 * direction, radius): TRUE if it hits, with the ray distances where it enters and leaves in *in and *out (the
 * classic Graphics Gems IV method: Cychosz and Waggenspack). A ray parallel to the axis never hits.
 */
s32 func_80018024(f32 *start, f32 *dir, f32 *base, f32 *axis, f32 radius, f32 *in, f32 *out) {
    s32 hit;
    f32 rc[3];
    f32 n[3];
    f32 o[3];
    f32 d;
    f32 t;
    f32 s;
    f32 ln;
    f32 den;

    rc[0] = start[0] - base[0];
    rc[1] = start[1] - base[1];
    rc[2] = start[2] - base[2];
    n[0] = dir[1] * axis[2] - dir[2] * axis[1];
    n[1] = dir[2] * axis[0] - dir[0] * axis[2];
    n[2] = dir[0] * axis[1] - dir[1] * axis[0];
    ln = n[0] * n[0] + n[1] * n[1] + n[2] * n[2];
    if (ln == 0.0f) {
        return FALSE;
    }
    ln = sqrtf(ln);
    n[0] /= ln;
    n[1] /= ln;
    n[2] /= ln;
    d = rc[0] * n[0] + rc[1] * n[1] + rc[2] * n[2];
    if (d < 0.0f) {
        d = -d;
    }
    hit = d <= radius;
    if (hit) {
        o[0] = rc[1] * axis[2] - rc[2] * axis[1];
        o[1] = rc[2] * axis[0] - rc[0] * axis[2];
        o[2] = rc[0] * axis[1] - rc[1] * axis[0];
        t = -(o[0] * n[0] + o[1] * n[1] + o[2] * n[2]) / ln;
        o[0] = n[1] * axis[2] - n[2] * axis[1];
        o[1] = n[2] * axis[0] - n[0] * axis[2];
        o[2] = n[0] * axis[1] - n[1] * axis[0];
        ln = sqrtf(o[0] * o[0] + o[1] * o[1] + o[2] * o[2]);
        o[0] /= ln;
        o[1] /= ln;
        o[2] /= ln;
        den = dir[0] * o[0] + dir[1] * o[1] + dir[2] * o[2];
        s = sqrtf(radius * radius - d * d) / den;
        if (s < 0.0f) {
            s = -s;
        }
        *in = t - s;
        *out = t + s;
    }
    return hit;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80018024);
#endif

/**
 * JFG: where a ray (origin, unit direction) meets a sphere; FALSE if it misses.
 * (The empty `if (c) {}` keeps b and b*b in the ROM's registers.)
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
        if (c) {}
        d = sqrtf(b2 - c);
        *tNear = -b - d;
        *tFar = -b + d;
    }
    return hit;
}


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

/* JFG: what func_80018808 fills in (0x2C bytes) */
typedef struct TrackLineHit {
    /* 0x00 */ void *obj;   // NULL for a level hit (func_800809F4 fills it in for an object)
    /* 0x04 */ f32 pos[3];  // where the line meets the triangle
    /* 0x10 */ Plane plane; // the triangle's plane
    /* 0x20 */ f32 dist;    // the distance from `from` to pos (to `to` with no hit)
    /* 0x24 */ u32 flags;   // the triangle's batch flags
    /* 0x28 */ s32 surface; // the batch's surface type
} TrackLineHit;

#ifdef NON_MATCHING
s32 line_clip_box(f32 *x0, f32 *y0, f32 *z0, f32 *x1, f32 *y1, f32 *z1, f32 *min, f32 *max, f32 *tEnterOut,
                  f32 *tLeaveOut);
u32 func_8001D344(LevelModelSegmentBoundingBox *bb, s32 x1, s32 z1, s32 x2, s32 z2);
u8 func_8001D464(LevelModelSegmentBoundingBox *bb, s32 y1, s32 y2);
s32 func_800809F4(void *obj, f32 *from, f32 *to, TrackLineHit *hit, s32 flags);

/**
 * JFG: the first thing the line from `from` to `to` hits: the level segments whose boxes it passes through (up to
 * 20, sorted by where it enters them) are searched in that order for the nearest triangle whose plane it crosses
 * from the front, inside the triangle's edges; batches with any of flags | 0xCE000880 are skipped, and with
 * `require` set so are batches with none of its bits. The search stops after the first segment with a hit. With
 * `require` 0 the objects are tested too (func_800809F4, up to the level hit), and win. hit gets the point, the
 * triangle's plane, the distance, the batch flags and surface (or for no hit only the distance to `to`).
 * Returns 1 on a hit.
 * NON_MATCHING: a first body written from the ROM (621 of 628 words, frame 0x2A0 of 0x2A8: every stack slot is the
 * ROM's but the f30/f31 save, as the ROM keeps distTo and t in f20/f22); checked equal to the ROM by simulation.
 */
s32 func_80018808(void *obj, f32 *from, f32 *to, TrackLineHit *hit, s32 flags, s32 require) {
    LevelModelSegment *segs[20];
    Plane *planes;
    Plane *plane;
    TrianglePlaneRef *ref;
    u32 batchFlags;
    Plane *bestPlane;
    f32 tEnters[20];
    f32 best[3];
    f32 min[3];
    f32 max[3];
    LevelModelSegmentBoundingBox *bb;
    LevelModelSegment *seg;
    s32 minX;
    s32 maxX;
    s32 minY;
    s32 maxY;
    s32 minZ;
    s32 maxZ;
    s32 ix1;
    s32 iy1;
    s32 iz1;
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 tEnter;
    f32 tLeave;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distTo;
    f32 distFrom;
    f32 t;
    f32 dist;
    f32 side;
    s32 edge;
    s32 flip;
    s32 inside;
    s32 i;
    s32 count;
    s32 j;
    s32 k;
    s32 result;
    s32 tri;
    s32 end;
    u32 bits;
    s32 b;
    u32 mask;
    u32 masks[20];
    UNUSED s32 pad[2];
    u32 bestFlags;
    u8 heights[20];
    u8 hmask;
    u8 hb;
    u8 bestSurface;

    count = 0;
    for (i = 0; i < gCurrentLevelModel->numberOfSegments; i++) {
        bb = &gCurrentLevelModel->segmentsBoundingBoxes[i];
        min[0] = bb->x1;
        min[1] = bb->y1;
        min[2] = bb->z1;
        max[0] = bb->x2;
        max[1] = bb->y2;
        max[2] = bb->z2;
        x0 = from[0];
        y0 = from[1];
        z0 = from[2];
        x1 = to[0];
        y1 = to[1];
        z1 = to[2];
        if (line_clip_box(&x0, &y0, &z0, &x1, &y1, &z1, min, max, &tEnter, &tLeave)) {
            minX = x0;
            minY = y0;
            minZ = z0;
            ix1 = x1;
            iy1 = y1;
            iz1 = z1;
            maxX = ix1;
            if (ix1 < minX) {
                maxX = minX;
                minX = ix1;
            }
            maxY = iy1;
            if (iy1 < minY) {
                maxY = minY;
                minY = iy1;
            }
            maxZ = iz1;
            if (iz1 < minZ) {
                maxZ = minZ;
                minZ = iz1;
            }
            masks[count] = func_8001D344(bb, minX, minZ, maxX, maxZ);
            heights[count] = func_8001D464(bb, minY, maxY);
            tEnters[count] = tEnter;
            segs[count] = &gCurrentLevelModel->segments[i];
            for (j = count; j > 0 && tEnters[j] < tEnters[j - 1]; j--) {
                seg = segs[j];
                mask = masks[j];
                hmask = heights[j];
                tEnter = tEnters[j];
                tEnters[j] = tEnters[j - 1];
                tEnters[j - 1] = tEnter;
                segs[j] = segs[j - 1];
                segs[j - 1] = seg;
                masks[j] = masks[j - 1];
                masks[j - 1] = mask;
                heights[j] = heights[j - 1];
                heights[j - 1] = hmask;
            }
            count++;
            if (count >= 20) {
                i = gCurrentLevelModel->numberOfSegments;
            }
        }
    }
    dx = to[0] - from[0];
    dy = to[1] - from[1];
    dz = to[2] - from[2];
    result = 0;
    tLeave = 1.0f;
    best[0] = to[0];
    best[1] = to[1];
    best[2] = to[2];
    flags |= 0xCE000880;
    for (i = 0; i < count && !result; i++) {
        seg = segs[i];
        mask = masks[i];
        hmask = heights[i];
        planes = seg->planes;
        for (b = 0; b < seg->numberOfBatches; b++) {
            batchFlags = seg->batches[b].flags;
            tri = seg->batches[b].facesOffset;
            end = seg->batches[b + 1].facesOffset;
            if ((batchFlags & flags) || (require != 0 && !(batchFlags & require))) {
                tri = end;
            }
            for (; tri < end; tri++) {
                bits = seg->triangleMasks[tri] & mask;
                hb = seg->triangleHeightMasks[tri] & hmask;
                if ((bits & 0xFFFF) && (bits & 0xFFFF0000) && hb) {
                    ref = &seg->trianglePlanes[tri];
                    plane = &planes[ref->plane];
                    distTo = plane->a * to[0] + plane->b * to[1] + to[2] * plane->c + plane->d;
                    if (distTo < 0.0f) {
                        distFrom = plane->a * from[0] + plane->b * from[1] + from[2] * plane->c + plane->d;
                        if (distFrom >= 0.0f) {
                            t = distFrom / (distFrom - distTo);
                            inside = TRUE;
                            x0 = from[0] + dx * t;
                            y0 = from[1] + dy * t;
                            z0 = from[2] + dz * t;
                            for (k = 0; k < 3 && inside; k++) {
                                edge = ref->edges[k];
                                flip = edge & 0x8000;
                                edge ^= flip;
                                plane = &planes[edge];
                                dist = plane->a * x0 + plane->b * y0 + plane->c * z0 + plane->d;
                                side = dist;
                                if (flip) {
                                    side = -dist;
                                }
                                if (side > 0.0f) {
                                    inside = FALSE;
                                }
                            }
                            if (inside && t < tLeave) {
                                tLeave = t;
                                best[0] = x0;
                                best[1] = y0;
                                best[2] = z0;
                                bestSurface = gCurrentLevelModel->textures[seg->batches[b].textureIndex].surfaceType;
                                bestFlags = seg->batches[b].flags;
                                bestPlane = &planes[ref->plane];
                                result = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    if (require == 0 && func_800809F4(obj, from, best, hit, flags)) {
        result = 1;
    } else if (result) {
        hit->obj = NULL;
        hit->pos[0] = best[0];
        hit->pos[1] = best[1];
        hit->pos[2] = best[2];
        hit->plane.a = bestPlane->a;
        hit->plane.b = bestPlane->b;
        hit->plane.c = bestPlane->c;
        hit->plane.d = bestPlane->d;
        dx *= tLeave;
        dy *= tLeave;
        dz *= tLeave;
        hit->dist = sqrtf(dx * dx + dy * dy + dz * dz);
        hit->flags = bestFlags;
        hit->surface = bestSurface;
    } else {
        hit->dist = sqrtf(dx * dx + dy * dy + dz * dz);
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_80018808);
#endif

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

/* JFG: a level edge crossed by a line (func_80019888) */
typedef struct EdgeHit {
    /* 0x00 */ f32 x0, y0, z0; // the edge's ends
    /* 0x0C */ f32 x1, y1, z1;
    /* 0x18 */ f32 along;      // where the line crosses it, 0..1 from the first end
    /* 0x1C */ Plane *plane;   // the plane of the edge's triangle
} EdgeHit;

#ifdef NON_MATCHING
/**
 * JFG: the first level edge (among the entries' triangle edges marked in edgeFlags >> 3, walked as
 * track_edge_find_next does) that the line from..to passes through, taking each edge as a wall 10 units high:
 * its ends, the crossing point along it and its triangle's plane go to out. Returns 1 or -1 for the side the
 * line crosses from, 0 if none. As in the ROM, segment is only set by an earlier entry.
 * NON_MATCHING: a first body written from the ROM, at the ROM's length and frame (353 words, 0xB0); the
 * crossing fraction gets f16 where the ROM uses f12, so the three constants move up one FP register (an extra
 * saved f22/f23) and the FP temporaries follow.
 */
s32 func_80019888(f32 *from, f32 *to, EdgeHit *out) {
    LevelModelSegment *segment;
    s32 i;
    s32 j;
    s32 k;
    s32 result;
    f32 n[3];
    f32 dir[3];
    f32 v[3];
    f32 a[3];
    f32 b[3];
    f32 e[3];
    s32 tri;
    s32 batch;
    Triangle *triangle;
    Vertex *verts;
    Vertex *va;
    Vertex *vb;
    u8 flags;
    f32 d;
    f32 t;
    s32 side;

    dir[0] = to[0] - from[0];
    dir[1] = to[1] - from[1];
    dir[2] = to[2] - from[2];
    result = 0;
    for (i = 0; i < D_800F33C4 && !result; i++) {
        if (D_800F33B8[i] > 0) {
            segment = (LevelModelSegment *) (D_800F33B8[i] | 0x80000000);
        } else {
            batch = D_800F33BC[i];
            tri = D_800F33C0[i];
            triangle = &segment->triangles[tri];
            verts = &segment->vertices[segment->batches[batch].verticesOffset];
            flags = (u32) segment->edgeFlags[tri] >> 3;
            j = 0;
            if (flags & 7) {
                for (; j < 3; j++) {
                    if (flags & (1 << j)) {
                        k = j + 1;
                        if (k >= 3) {
                            k = 0;
                        }
                        va = &verts[(&triangle->vi0)[j]];
                        vb = &verts[(&triangle->vi0)[k]];
                        a[0] = va->x;
                        a[1] = va->y;
                        a[2] = va->z;
                        b[0] = vb->x;
                        b[1] = vb->y;
                        b[2] = vb->z;
                        v[0] = 0.0f;
                        v[1] = 10.0f;
                        v[2] = 0.0f;
                        e[0] = b[0] - a[0];
                        e[1] = b[1] - a[1];
                        e[2] = b[2] - a[2];
                        n[0] = v[1] * e[2] - v[2] * e[1];
                        n[1] = v[2] * e[0] - v[0] * e[2];
                        n[2] = v[0] * e[1] - v[1] * e[0];
                        d = dir[0] * n[0] + dir[1] * n[1] + dir[2] * n[2];
                        if (d != 0.0f) {
                            side = 1;
                            if (d < 0.0f) {
                                side = -1;
                            }
                            v[0] = n[0] * (a[0] - from[0]);
                            v[1] = n[1] * (a[1] - from[1]);
                            v[2] = n[2] * (a[2] - from[2]);
                            t = (v[0] + v[1] + v[2]) / d;
                            if (t >= 0.0f && t <= 1.0f) {
                                v[0] = dir[0] * t + from[0];
                                v[1] = dir[1] * t + from[1];
                                v[2] = dir[2] * t + from[2];
                                d = e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
                                v[0] = e[0] * (v[0] - a[0]);
                                v[1] = e[1] * (v[1] - a[1]);
                                v[2] = e[2] * (v[2] - a[2]);
                                t = v[0] + v[1] + v[2];
                                if (t / d >= 0.0f && t / d <= 1.0f) {
                                    result = side;
                                    out->x0 = a[0];
                                    out->y0 = a[1];
                                    out->z0 = a[2];
                                    out->x1 = b[0];
                                    out->y1 = b[1];
                                    out->z1 = b[2];
                                    out->plane = &segment->planes[segment->trianglePlanes[tri].plane];
                                    out->along = t / d;
                                }
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
INCLUDE_ASM("asm/nonmatchings/12D50", func_80019888);
#endif

/**
 * JFG: find the level edge (among the entries' triangle edges, as track_edges_build walks them) that continues
 * edge: with dirX below 0.5 the one ending where edge starts, else the one starting where it ends (whole-unit
 * positions); its ends go to out. TRUE if found.
 * (The ends are copied to scalars per edge before the comparisons; the first copy came from the permuter.)
 */
s32 track_edge_find_next(TrackEdge *edge, TrackEdge *out) {
    LevelModelSegment *segment;
    s32 i;
    s32 j;
    s32 k;
    s32 found;
    s32 start[3];
    s32 end[3];
    s32 tri;
    s32 batch;
    Triangle *triangle;
    Vertex *verts;
    Vertex *va;
    Vertex *vb;
    u8 flags;
    s32 sx;
    s32 sy;
    s32 sz;
    s32 ex;
    s32 ey;
    s32 ez;

    start[0] = edge->x0;
    start[1] = edge->y0;
    start[2] = edge->z0;
    end[0] = edge->x1;
    end[1] = edge->y1;
    end[2] = edge->z1;
    found = FALSE;
    for (i = 0; i < D_800F33C4 && !found; i++) {
        if (D_800F33B8[i] > 0) {
            segment = (LevelModelSegment *) (D_800F33B8[i] | 0x80000000);
        } else {
            batch = D_800F33BC[i];
            tri = D_800F33C0[i];
            triangle = &segment->triangles[tri];
            verts = &segment->vertices[segment->batches[batch].verticesOffset];
            flags = (u32) segment->edgeFlags[tri] >> 3;
            if (flags & 7) {
                for (j = 0; j < 3; j++) {
                    if (flags & (1 << j)) {
                        k = j + 1;
                        if (k >= 3) {
                            k = 0;
                        }
                        va = &verts[(&triangle->vi0)[j]];
                        sx = start[0];
                        sy = start[1];
                        sz = start[2];
                        ex = end[0];
                        ey = end[1];
                        ez = end[2];
                        vb = &verts[(&triangle->vi0)[k]];
                        if (edge->dirX < 0.5f) {
                            if (sx == vb->x && sy == vb->y && sz == vb->z) {
                                found = TRUE;
                                out->x0 = va->x;
                                out->y0 = va->y;
                                out->z0 = va->z;
                                out->x1 = vb->x;
                                out->y1 = vb->y;
                                out->z1 = vb->z;
                            }
                        } else if (ex == va->x && ey == va->y && ez == va->z) {
                            found = TRUE;
                            out->x0 = va->x;
                            out->y0 = va->y;
                            out->z0 = va->z;
                            out->x1 = vb->x;
                            out->y1 = vb->y;
                            out->z1 = vb->z;
                        }
                    }
                }
            }
        }
    }
    return found;
}

/* JFG: a level height under a point: the plane's height there, its a/b/c, the batch flags and surface type */
typedef struct TrackHeight {
    /* 0x00 */ f32 y;
    /* 0x04 */ f32 a;
    /* 0x08 */ f32 b;
    /* 0x0C */ f32 c;
    /* 0x10 */ union {
        u32 flags;
        struct {
            u8 pad[3];
            s8 surfaceType;
        } s;
    } info;
} TrackHeight;

extern TrackHeight D_800F2FE8[20];
extern TrackHeight *D_800F3178[20];

#ifdef NON_MATCHING
s32 func_8001AB84(f32 px, f32 pz, f32 x1, f32 z1, f32 x2, f32 z2, f32 radiusSq);

/**
 * JFG: func_8001B078 for a cylinder: the level triangles in the segments the box around (x, z) of the given radius
 * and from yMin to yMax touches (no result if 8 or more segments do), facing up (or with `below` set, down), with a
 * corner between yMin and yMax, that the point lies in or within the radius of an edge of, on the x/z plane (batches
 * with any of flags are skipped). With heights NULL: 1 if one of them is between yMin and yMax at (x, z), else 0.
 * Otherwise every one (up to 20) goes into the height list as func_8001B078 builds it, sorted from the highest down,
 * and *heights gets the list; the count is returned.
 * NON_MATCHING: a first body written from the ROM (607 of 603 words, the ROM's frame 0x1C8 and stack slots; the pads
 * fill it); checked equal to the ROM by simulation. The ROM keeps `below` in a0 through the triangle loop.
 */
s32 func_8001A218(f32 x, f32 z, f32 yMin, f32 yMax, f32 radius, s32 flags, s32 below, TrackHeight ***heights) {
    LevelModelSegment *segment;
    s32 batchNum;
    s32 faceNum;
    s32 start;
    s32 nextFace;
    u32 bits;
    s32 planeIndex;
    Vertex *vert0;
    Vertex *vert1;
    Vertex *vert2;
    s32 hit;
    f32 x0;
    f32 z0;
    f32 z2;
    f32 x1;
    f32 z1;
    f32 B;
    f32 x2;
    f32 radiusSq;
    f32 A;
    f32 C;
    f32 D;
    f32 h;
    s32 verticesOffset;
    s32 j;
    s32 sorted;
    TrackHeight *swap;
    s32 segmentCount;
    s32 i;
    s32 ix;
    s32 iz;
    s32 segs[32];
    s32 ir;
    UNUSED s32 pad;
    s32 count;
    UNUSED s32 pad2;
    u32 mask;
    s16 min[3];
    s16 max[3];
    s8 surfaceType;

    if (heights != NULL) {
        *heights = NULL;
    }
    ix = x;
    ir = radius;
    iz = z;
    min[0] = ix - ir;
    min[1] = (s32) yMin;
    min[2] = iz - ir;
    max[0] = ix + ir;
    max[1] = (s32) yMax;
    max[2] = iz + ir;
    segmentCount = get_inside_segment_count_xyz(segs, min[0], min[1], min[2], max[0], max[1], max[2]);
    if (segmentCount == 0 || segmentCount >= 8) {
        return 0;
    }
    count = 0;
    for (i = 0; i < segmentCount; i++) {
        segment = &gCurrentLevelModel->segments[segs[i]];
        mask = func_8001D344(&gCurrentLevelModel->segmentsBoundingBoxes[segs[i]], min[0], min[2], max[0], max[2]);
        for (batchNum = 0; batchNum < segment->numberOfBatches; batchNum++) {
            verticesOffset = segment->batches[batchNum].verticesOffset;
            start = segment->batches[batchNum].facesOffset;
            nextFace = segment->batches[batchNum + 1].facesOffset;
            if (segment->batches[batchNum].flags & 0x2000) {
                surfaceType = 2;
            } else {
                surfaceType = gCurrentLevelModel->textures[segment->batches[batchNum].textureIndex].surfaceType;
            }
            if (segment->batches[batchNum].flags & flags) {
                start = nextFace;
            }
            for (faceNum = start; faceNum < nextFace; faceNum++) {
                bits = segment->triangleMasks[faceNum] & mask;
                planeIndex = segment->trianglePlanes[faceNum].plane << 4;
                B = ((Plane *) ((u8 *) segment->planes + planeIndex))->b;
                if ((bits >> 16) && (bits & 0xFFFF) && ((!below && 0.0f < B) || (below && B < 0.0f))) {
                    vert0 = &segment->vertices[segment->triangles[faceNum].vi0 + verticesOffset];
                    vert1 = &segment->vertices[segment->triangles[faceNum].vi1 + verticesOffset];
                    vert2 = &segment->vertices[segment->triangles[faceNum].vi2 + verticesOffset];
                    if (vert0->y < min[1] && vert1->y < min[1] && vert2->y < min[1]) {
                        continue;
                    }
                    if (max[1] < vert0->y && max[1] < vert1->y && max[1] < vert2->y) {
                        continue;
                    }
                    if (below) {
                        hit = func_800494AC(ix, iz, vert2, vert1, vert0);
                    } else {
                        hit = func_800494AC(ix, iz, vert0, vert1, vert2);
                    }
                    if (!hit) {
                        x0 = vert0->x;
                        z0 = vert0->z;
                        x1 = vert1->x;
                        z1 = vert1->z;
                        x2 = vert2->x;
                        z2 = vert2->z;
                        radiusSq = radius * radius;
                        if (func_8001AB84(x, z, x0, z0, x1, z1, radiusSq)) {
                            hit = TRUE;
                        } else if (func_8001AB84(x, z, x1, z1, x2, z2, radiusSq)) {
                            hit = TRUE;
                        } else if (func_8001AB84(x, z, x2, z2, x0, z0, radiusSq)) {
                            hit = TRUE;
                        }
                    }
                    if (hit) {
                        A = ((Plane *) ((u8 *) segment->planes + planeIndex))->a;
                        C = ((Plane *) ((u8 *) segment->planes + planeIndex))->c;
                        D = ((Plane *) ((u8 *) segment->planes + planeIndex))->d;
                        if (heights != NULL) {
                            D_800F2FE8[count].y = -(((A * x) + (C * z) + D) / B);
                            D_800F2FE8[count].a = A;
                            D_800F2FE8[count].info.flags = segment->batches[batchNum].flags;
                            D_800F2FE8[count].b = B;
                            D_800F2FE8[count].c = C;
                            // NOT PORTABLE: the surface type overlays the flags' low byte (big-endian), as in func_8001B078
                            D_800F2FE8[count].info.s.surfaceType = surfaceType;
                            count++;
                            if (count >= 20) {
                                i = segmentCount;
                                batchNum = segment->numberOfBatches;
                                faceNum = nextFace;
                            }
                        } else {
                            h = -(((A * x) + (C * z) + D) / B);
                            if (yMin <= h && h <= yMax) {
                                count = 1;
                                batchNum = segment->numberOfBatches;
                                faceNum = nextFace;
                                i = segmentCount;
                            }
                        }
                    }
                }
            }
        }
    }
    if (heights == NULL) {
        return count;
    }
    for (j = 0; j < count; j++) {
        D_800F3178[j] = &D_800F2FE8[j];
    }
    do {
        j = 0;
        sorted = TRUE;
        for (; j < count - 1; j++) {
            if (D_800F3178[j]->y < D_800F3178[j + 1]->y) {
                swap = D_800F3178[j];
                D_800F3178[j] = D_800F3178[j + 1];
                D_800F3178[j + 1] = swap;
                sorted = FALSE;
            }
        }
    } while (!sorted);
    *heights = D_800F3178;
    return count;
}
#else
INCLUDE_ASM("asm/nonmatchings/12D50", func_8001A218);
#endif

/**
 * JFG: TRUE if the point (px, pz) is within sqrt(radiusSq) of the segment (x1, z1)-(x2, z2) on the x/z plane: the
 * nearest point when the perpendicular falls on the segment, otherwise only the first end (x1, z1) is tested.
 * (dx/dz are reused for the offset from the nearest point, as the ROM's registers show; the three pads fill the
 * ROM's frame.)
 */
s32 func_8001AB84(f32 px, f32 pz, f32 x1, f32 z1, f32 x2, f32 z2, f32 radiusSq) {
    f32 dx;
    f32 dz;
    f32 lenSq;
    f32 t;
    f32 ex;
    f32 ez;
    s32 hit;
    UNUSED s32 pad0;
    UNUSED s32 pad1;
    UNUSED s32 pad2;

    dx = x2 - x1;
    dz = z2 - z1;
    lenSq = dx * dx + dz * dz;
    hit = FALSE;
    if (lenSq > 0.0f) {
        t = ((px - x1) * dx + (pz - z1) * dz) / lenSq;
        if (t >= 0.0f && t <= 1.0f) {
            dx = px - (t * dx + x1);
            dz = pz - (t * dz + z1);
            if (dx * dx + dz * dz <= radiusSq) {
                hit = TRUE;
            }
        }
    }
    if (!hit) {
        ex = px - x1;
        ez = pz - z1;
        if (ex * ex + ez * ez <= radiusSq) {
            hit = TRUE;
        }
    }
    return hit;
}

u8 func_8001D464(LevelModelSegmentBoundingBox *bb, s32 y1, s32 y2);
s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos);

/**
 * JFG: the level triangle nearest in height to (x, y, z) among those of the point's segment under it (batches
 * without any of flags): its height goes to *outY and its plane to *outPlane (each if not NULL); NULL if none.
 * (`best = d` stored last and the no-op `& 0xFFFF` on the u16 plane index: permuter, two rounds.)
 */
Triangle *track_nearest_triangle(f32 x, f32 y, f32 z, f32 *outY, Plane **outPlane, s32 flags) {
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
                        plane = &segment->planes[segment->trianglePlanes[faceNum].plane & 0xFFFF];
                        if (plane->b != 0.0f) {
                            h = -((plane->a * x + plane->c * z + plane->d) / plane->b);
                            d = y - h;
                            if (y < h) {
                                d = -d;
                            }
                            if (d < best) {
                                if (outY != NULL) {
                                    *outY = h;
                                }
                                if (outPlane != NULL) {
                                    *outPlane = plane;
                                }
                                best = d;
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

/**
 * JFG: every level height under (x, z) (up to 20; batches with any of flags count as having no triangles),
 * sorted from the highest down; *heights gets the sorted list, the count is returned.
 * DKR: get_level_segment_waves (a JFG variant: no wave segment, the planes are kept, sorted the same way)
 * (The plane index goes through `plane` before it is scaled, the entry is written through
 * D_800F2FE8[count] with count++ after it, and the pointer loop is one line, as in DKR.)
 */
s32 func_8001B078(f32 x, f32 z, s32 flags, TrackHeight ***heights) {
    LevelModelSegment *segment;
    s32 batchNum;
    s32 faceNum;
    s32 count;
    UNUSED s32 pad;
    u32 bits;
    s32 plane;
    s32 surfaceType;
    s32 verticesOffset;
    TrackHeight **waves;
    TrackHeight *wave;
    s32 sorted;
    s32 j;
    s32 segmentCount;
    s32 i;
    s32 nextFace;
    s32 planeIndex;
    s32 start;
    f32 A;
    f32 C;
    s16 segmentsInside[32];
    s32 ix;
    s32 iz;
    f32 D;
    Vertex *vert0;
    Vertex *vert1;
    Vertex *vert2;
    u32 mask;
    f32 B;

    ix = x;
    iz = z;
    *heights = NULL;
    segmentCount = get_inside_segment_count_xz(ix, iz, segmentsInside);
    if (segmentCount == 0 || segmentCount >= 32) {
        return 0;
    }
    count = 0;
    for (i = 0; i < segmentCount; i++) {
        segment = &gCurrentLevelModel->segments[segmentsInside[i]];
        mask = func_8001D344(&gCurrentLevelModel->segmentsBoundingBoxes[segmentsInside[i]], ix, iz, ix, iz);
        for (batchNum = 0; batchNum < segment->numberOfBatches; batchNum++) {
            start = segment->batches[batchNum].facesOffset;
            verticesOffset = segment->batches[batchNum].verticesOffset;
            nextFace = segment->batches[batchNum + 1].facesOffset;
            if (segment->batches[batchNum].flags & 0x2000) {
                surfaceType = 2;
            } else {
                surfaceType = gCurrentLevelModel->textures[segment->batches[batchNum].textureIndex].surfaceType;
            }
            if (segment->batches[batchNum].flags & flags) {
                start = nextFace;
            }
            for (faceNum = start; faceNum < nextFace; faceNum++) {
                bits = segment->triangleMasks[faceNum] & mask;
                plane = segment->trianglePlanes[faceNum].plane & 0xFFFF;
                planeIndex = plane << 4;
                B = ((Plane *) ((u8 *) segment->planes + planeIndex))->b;
                if ((bits >> 16) && (bits & 0xFFFF) && (0.0f < B)) {
                    vert0 = &segment->vertices[segment->triangles[faceNum].vi0 + verticesOffset];
                    vert1 = &segment->vertices[segment->triangles[faceNum].vi1 + verticesOffset];
                    vert2 = &segment->vertices[segment->triangles[faceNum].vi2 + verticesOffset];
                    if (func_800494AC(ix, iz, vert0, vert1, vert2)) {
                        A = ((Plane *) ((u8 *) segment->planes + planeIndex))->a;
                        C = ((Plane *) ((u8 *) segment->planes + planeIndex))->c;
                        D = ((Plane *) ((u8 *) segment->planes + planeIndex))->d;
                        D_800F2FE8[count].y = -(((A * x) + (C * z) + D) / B);
                        D_800F2FE8[count].info.flags = segment->batches[batchNum].flags;
                        D_800F2FE8[count].b = B;
                        D_800F2FE8[count].info.s.surfaceType = surfaceType;
                        D_800F2FE8[count].a = A;
                        D_800F2FE8[count].c = C;
                        count++;
                        if (count >= 20) {
                            batchNum = segment->numberOfBatches;
                            faceNum = nextFace;
                            i = segmentCount;
                        }
                    }
                }
            }
        }
    }
    waves = D_800F3178;
    wave = D_800F2FE8;
    // clang-format off
    for (j = 0; j < count; j++) { waves[j] = &wave[j]; }
    // clang-format on
    do {
        j = 0;
        sorted = TRUE;
        for (; j < count - 1; j++) {
            if (waves[j]->y < waves[j + 1]->y) {
                wave = waves[j];
                waves[j] = waves[j + 1];
                waves[j + 1] = wave;
                sorted = FALSE;
            }
        }
    } while (!sorted);
    *heights = D_800F3178;
    return count;
}

/**
 * Returns the current loaded level geometry
 * Official Name: trackGetTrack
 */
LevelModel *get_current_level_model(void) {
    return gCurrentLevelModel;
}

extern void *D_800A0D88;
extern s32 D_800A0D84;
extern void *D_800F31C8;
extern TrackEdge *D_800F33B0;
extern s32 *D_800F33B8;
extern s16 *D_800F33BC;
extern s16 *D_800F33C0;
extern void *D_800F2FB0;
void track_segment_data_free(void);
void weapon_buffers_release_all(void);
void tex_free(void *tex);
void shadow_buffers_free(void);
void free_object(Object *obj);
void gParticlePtrList_flush(void);
void sequence_table_free(void);
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
    weapon_buffers_release_all();
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
    shadow_buffers_free();
    if (gSkydomeSegment != NULL) {
        free_object(gSkydomeSegment);
        gParticlePtrList_flush();
    }
    sequence_table_free();
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

void shadow_heap_current_ptrs(s32 flags, Vertex **verts, Triangle **tris, ShadowHeapProperties **data);

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
        shadow_heap_current_ptrs(shadow->group, &verts, &tris, &data);
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

/* JFG: TextureHeader with the fields the water texture code reads */
typedef struct WaterTexture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0xC - 2];
    /* 0x0C */ Gfx *cmd;
    /* 0x10 */ u8 pad10[2];
    /* 0x12 */ u16 numOfTextures;
    /* 0x14 */ u8 pad14[0x1B - 0x14];
    /* 0x1B */ u8 unk1B; // JFG: 2 and up = the texture loads itself (its own display list)
} WaterTexture;

extern Gfx D_800A0DC8[];
extern Gfx D_800A0DF0[];
extern Gfx D_800A0E18[];
extern Gfx D_800A0E48[];
void *set_animated_texture_header(void *texHead, s32 offset);

/**
 * JFG: load a level texture for drawing: a self-loading texture runs its display list; otherwise the water effect
 * texture (or the texture's next frame) is loaded as a second, IA8 texture at tmem 256 with the texture
 * (RGBA16, or IA8 with the env colour from texOffset) as the first.
 */
void track_texture_load(WaterTexture *tex, s32 texOffset, s32 flags) {
    s32 masks;
    s32 maskt;
    s32 envMaskt;
    s32 envMasks;
    WaterTexture *env;
    s32 envOffset;
    s32 shifts;
    s32 shiftt;
    s32 blend;
    void *frame;
    void *envFrame;

    frame = set_animated_texture_header(tex, texOffset);
    if (tex->unk1B >= 2) {
        D_800F2F70->words.w0 = tex->cmd->words.w0;
        D_800F2F70->words.w1 = (u32) frame;
        D_800F2F70++;
        gSPDisplayList(D_800F2F70++, tex->cmd + 1);
        gSPDisplayList(D_800F2F70++, D_800A0E48);
        return;
    }
    masks = highest_set_bit(tex->width);
    maskt = highest_set_bit(tex->height);
    env = (WaterTexture *) D_800A0D68;
    if (env != NULL) {
        envOffset = D_800A0D6C;
        blend = FALSE;
    } else {
        env = tex;
        envOffset = (texOffset >> 8) + 0x100;
        if (envOffset >= tex->numOfTextures) {
            envOffset -= tex->numOfTextures;
        }
        envOffset <<= 8;
        blend = TRUE;
    }
    envFrame = set_animated_texture_header(env, envOffset);
    envMasks = highest_set_bit(env->width);
    envMaskt = highest_set_bit(env->height);
    shifts = (masks - envMasks) & 0xF;
    shiftt = (maskt - envMaskt) & 0xF;
    gDPLoadMultiBlockS(D_800F2F70++, envFrame, 256, 1, G_IM_FMT_IA, G_IM_SIZ_8b, env->width, env->height, 0, 0, 0,
                       envMasks, envMaskt, shifts, shiftt);
    if (!blend) {
        gDPLoadTextureBlockS(D_800F2F70++, frame, G_IM_FMT_RGBA, G_IM_SIZ_16b, tex->width, tex->height, 0, 0, 0,
                             masks, maskt, 0, 0);
        gSPDisplayList(D_800F2F70++, D_800A0DC8);
        return;
    }
    gDPLoadTextureBlockS(D_800F2F70++, frame, G_IM_FMT_IA, G_IM_SIZ_8b, tex->width, tex->height, 0, 0, 0, masks,
                         maskt, 0, 0);
    if ((flags & 0x70) == 0x10) {
        gSPDisplayList(D_800F2F70++, D_800A0E18);
    } else {
        gSPDisplayList(D_800F2F70++, D_800A0DF0);
    }
    gDPSetEnvColor(D_800F2F70++, (texOffset >> 8) & 0xFF, (texOffset >> 8) & 0xFF, (texOffset >> 8) & 0xFF,
                   (texOffset >> 8) & 0xFF);
}
