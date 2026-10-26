#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern MtxF gInverseViewMatrixF;
extern MtxF gViewMatrixF;
extern f32 gCurCamFOV;
extern s32 gActiveCameraID;
extern Mtx gPerspectiveMatrix[];
/* Size: 0x4C bytes (layout not worked out yet) */
typedef struct Camera {
    u8 pad0[0x4C];
} Camera;
extern Camera gCameras[];
extern s32 D_800FA634;
extern u8 D_800A316C;

/* DKR src/camera.h */
enum ViewportFlags {
    VIEWPORT_EXTRA_BG = 0x0001,
    VIEWPORT_UNK_02 = 0x0002,
    VIEWPORT_UNK_04 = 0x0004,
    VIEWPORT_X_CUSTOM = 0x0008,
    VIEWPORT_Y_CUSTOM = 0x0010,
    VIEWPORT_WIDTH_CUSTOM = 0x0020,
    VIEWPORT_HEIGHT_CUSTOM = 0x0040
};

/* Size: 0x34 bytes */
typedef struct ScreenViewport {
    /* 0x00 */ s32 x1;
    /* 0x04 */ s32 y1;
    /* 0x08 */ s32 x2;
    /* 0x0C */ s32 y2;
    /* 0x10 */ s32 posX;
    /* 0x14 */ s32 posY;
    /* 0x18 */ s32 width;
    /* 0x1C */ s32 height;
    /* 0x20 */ s32 scissorX1;
    /* 0x24 */ s32 scissorY1;
    /* 0x28 */ s32 scissorX2;
    /* 0x2C */ s32 scissorY2;
    /* 0x30 */ s32 flags;
} ScreenViewport;

extern ScreenViewport gScreenViewports[4];
extern MtxF gOrthoMatrixF;

INCLUDE_ASM("asm/nonmatchings/40240", func_8003F640);

extern s32 D_800FAA58;

void func_8003F7A8(void) {
    D_800FAA58 = 0x1;
}

extern s32 D_800FAA58;

void func_8003F7B8(void) {
    D_800FAA58 = 0;
}

/**
 * Unused function that will return the current camera's FoV.
 * Official Name: camGetFOV
 */
f32 cam_get_fov(void) {
    return gCurCamFOV;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_8003F7D4);

INCLUDE_ASM("asm/nonmatchings/40240", func_8003F7F4);

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FA6C);

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FAEC);

/**
 * Return the index of the active view.
 * 0-3 is players 1-4, and 4-7 is the same, but with 4 added on for cutscenes.
 * Official Name: camGetMode
 */
s32 get_current_viewport(void) {
    return gActiveCameraID;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FBDC);

extern s32 D_800FA634;

s32 func_8003FC1C(void) {
    return D_800FA634;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FC2C);

extern u8 D_800A3178[];

/* JFG: a per-camera byte. */
u8 func_8003FC4C(s32 index) {
    return D_800A3178[index];
}

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FC60);

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FC80);

void camEnableUserView(s32 viewPortIndex, s32 arg1) {
    if (arg1) {
        gScreenViewports[viewPortIndex].flags |= VIEWPORT_EXTRA_BG;
    } else {
        gScreenViewports[viewPortIndex].flags |= VIEWPORT_UNK_02;
    }
    gScreenViewports[viewPortIndex].flags &= ~VIEWPORT_UNK_04;
}

void camDisableUserView(s32 viewPortIndex, s32 arg1) {
    if (arg1) {
        gScreenViewports[viewPortIndex].flags &= ~VIEWPORT_EXTRA_BG;
    } else {
        gScreenViewports[viewPortIndex].flags |= VIEWPORT_UNK_04;
    }
    gScreenViewports[viewPortIndex].flags &= ~VIEWPORT_UNK_02;
}

/**
 * Return's the current viewport's flag status for extended backgrounds.
 * Required to draw some extra things used in menus.
 */
s32 check_viewport_background_flag(s32 viewPortIndex) {
    return gScreenViewports[viewPortIndex].flags & VIEWPORT_EXTRA_BG;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FF64);

INCLUDE_ASM("asm/nonmatchings/40240", func_800400D8);

/**
 * Sets the passed values to be equal to the selected viewports scissor size, before drawing the background elements.
 * Usually, this is the same size as the viewport's size.
 * Official name: camGetVisibleUserView
 */
s32 copy_viewport_background_size_to_coords(s32 viewPortIndex, s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    // gDPFillRectangle values
    *x1 = gScreenViewports[viewPortIndex].scissorX1;
    *x2 = gScreenViewports[viewPortIndex].scissorX2;
    *y1 = gScreenViewports[viewPortIndex].scissorY1;
    *y2 = gScreenViewports[viewPortIndex].scissorY2;
    if ((*x1 | *x2 | *y1 | *y2) == 0) {
        return 0;
    }
    return 1;
}

/**
 * Sets the passed values to the coordinate size of the passed viewport.
 * Official name: camGetUserView
 */
void copy_viewport_frame_size_to_coords(s32 viewPortIndex, s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    *x1 = gScreenViewports[viewPortIndex].x1;
    *y1 = gScreenViewports[viewPortIndex].y1;
    *x2 = gScreenViewports[viewPortIndex].x2;
    *y2 = gScreenViewports[viewPortIndex].y2;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_800402B0);

INCLUDE_ASM("asm/nonmatchings/40240", func_8004046C);

INCLUDE_ASM("asm/nonmatchings/40240", func_800408C0);

INCLUDE_ASM("asm/nonmatchings/40240", func_80040A70);

INCLUDE_ASM("asm/nonmatchings/40240", func_80040AA4);

/**
 * Sets the Y value of the Y axis in the matrix to the passed value.
 * This is used to vertically scale ortho geometry to look identical across NTSC and PAL systems.
 * Official Name: camOrthoYAspect
 */
void set_ortho_matrix_height(f32 value) {
    gOrthoMatrixF[1][1] = value;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_80040D64);

INCLUDE_ASM("asm/nonmatchings/40240", func_80040E64);

INCLUDE_ASM("asm/nonmatchings/40240", func_80040F2C);

INCLUDE_ASM("asm/nonmatchings/40240", func_80040FE4);

void func_80041168(s32 arg0, s32 arg1) {
}

INCLUDE_ASM("asm/nonmatchings/40240", func_80041174);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041450);

INCLUDE_ASM("asm/nonmatchings/40240", func_800416CC);

INCLUDE_ASM("asm/nonmatchings/40240", func_800419AC);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041A90);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041BE0);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041CA0);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041DA0);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041DC0);

/**
 * Returns the active camera.
 * JFG: no cutscene camera offset.
 * Official name: camGetPtr
 */
Camera *cam_get_active_camera(void) {
    return &gCameras[D_800FA634];
}

/**
 * Returns the camera list.
 * JFG: always the start of the list (DKR picks the cutscene camera if active).
 * Official name: camGetListPtr
 */
Camera *cam_get_cameras(void) {
    return &gCameras[0];
}

/**
 * Return the current floating point projection matrix.
 * Official name: camGetInvProjMtx
 */
MtxF *get_projection_matrix_f32(void) {
    return &gInverseViewMatrixF;
}

/**
 * Return the current fixed point projection matrix.
 * JFG: one per D_800A316C (DKR has a single matrix).
 * Official name: camGetProjOrgMtx
 */
Mtx *get_projection_matrix_s16(void) {
    return &gPerspectiveMatrix[D_800A316C];
}

extern u8 D_800FAAF0[];

void *func_80041E40(void) {
    return D_800FAAF0;
}

/**
 * Return the current camera matrix.
 * Official name: camGetRotationMtx
 */
MtxF *get_camera_matrix(void) {
    return &gViewMatrixF;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_80041E58);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041FE0);

INCLUDE_ASM("asm/nonmatchings/40240", func_800420F0);

INCLUDE_ASM("asm/nonmatchings/40240", func_80042134);

INCLUDE_ASM("asm/nonmatchings/40240", func_800421F0);

INCLUDE_ASM("asm/nonmatchings/40240", func_800422C4);

INCLUDE_ASM("asm/nonmatchings/40240", func_80042310);

INCLUDE_ASM("asm/nonmatchings/40240", func_80042438);

INCLUDE_ASM("asm/nonmatchings/40240", func_80042470);

INCLUDE_ASM("asm/nonmatchings/40240", func_800424A0);
