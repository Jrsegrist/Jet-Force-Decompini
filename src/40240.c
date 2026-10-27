#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern MtxF gInverseViewMatrixF;
extern MtxF gViewMatrixF;
extern f32 gCurCamFOV;
extern s32 gActiveCameraID;
extern Mtx gPerspectiveMatrix[];
/* Size: 0x4C bytes (mostly not worked out yet) */
typedef struct Camera {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
    /* 0x18 */ f32 unk18; // JFG: the position again
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 fov; // JFG
    /* 0x30 */ f32 shakeX;
    /* 0x34 */ f32 shakeY;
    /* 0x38 */ f32 shakeZ;
    /* 0x3C */ s8 unk3C;
    /* 0x3D */ u8 unk3D;
    /* 0x3E */ s16 unk3E;
    /* 0x40 */ f32 unk40;
    /* 0x44 */ u8 pad44[6];
    /* 0x4A */ s16 unk4A;
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

#define CAMERA_DEFAULT_FOV 60.0f
#define CAMERA_ASPECT 1.3333334f
#define CAMERA_NEAR 10.0f
#define CAMERA_SCALE 1.0 /* a double: IDO does not share it with the 1.0f in cam_set_fov */

extern u8 D_800FA650;
extern s32 D_800A2ED0;
extern f32 D_800AC40C; // the far plane (a .rodata constant, not split out yet)
extern MtxF gPerspectiveMatrixF;
extern u16 perspNorm;
void func_8003FAEC(s32 x, s32 y, s32 z, s32 angleZ, s32 angleX, s32 angleY);
void cam_shake_stop_all(void);
void func_80048D84(MtxF *in, Mtx *out);

/**
 * JFG: four cameras, each with a field of view; the perspective matrix is one of
 * 16 kept in turn. No anti-tamper check.
 * Official Name: camInit
 */
void cam_init(s32 arg0) {
    s32 j;

    D_800FA634 = 0;
    gActiveCameraID = 0;
    D_800FA650 = 0;
    for (j = 0; j < 4; j++) {
        D_800FA634 = j;
        func_8003FAEC(200, 200, 200, 0, 0, 180);
        gCameras[D_800FA634].fov = CAMERA_DEFAULT_FOV;
    }
    cam_shake_stop_all();
    if (arg0) {
        D_800A2ED0 = 0;
    }
    gCurCamFOV = CAMERA_DEFAULT_FOV;
    guPerspectiveF(gPerspectiveMatrixF, &perspNorm, gCurCamFOV, CAMERA_ASPECT, CAMERA_NEAR, D_800AC40C, CAMERA_SCALE);
    D_800A316C = (D_800A316C + 1) & 0xF;
    func_80048D84(&gPerspectiveMatrixF, &gPerspectiveMatrix[D_800A316C]);
}

extern s32 gNoCamShake;

/* Disables camera shake (0x8003F7A8). */
void cam_shake_off(void) {
    gNoCamShake = 0x1;
}

extern s32 gNoCamShake;

/* Enables camera shake (0x8003F7B8). */
void cam_shake_on(void) {
    gNoCamShake = 0;
}

/**
 * Unused function that will return the current camera's FoV.
 * Official Name: camGetFOV
 */
f32 cam_get_fov(void) {
    return gCurCamFOV;
}

extern f32 D_800FAA60;
extern f32 D_800FAA64;
extern s32 D_800FAA5C;

/* Sets X and Y scales applied once to the next perspective matrix built by cam_set_fov (0x8003F7D4). */
void cam_fov_scale_set(f32 x, f32 y) {
    D_800FAA60 = x;
    D_800FAA64 = y;
    D_800FAA5C = 1;
}

#define CAMERA_MIN_FOV 0.0f
#define CAMERA_MAX_FOV 90.0f

extern u8 gCamZoomSet[4];  // JFG: per camera, a zoom is set
extern f32 gCamZoom[4]; // JFG: per camera, the zoom
extern f32 D_800AC410;    // .rodata constants, not split out yet
extern f32 D_800AC414;
extern f32 D_800AC418;
extern f32 D_800A3168;
s32 fb_mode(void); // the video mode
s32 level_type(void);
s32 func_8005AA7C(void); // the two-player screen is split vertically

/**
 * Set the FoV of the viewspace, then recalculate the perspective matrix.
 * JFG: per-camera FoV and zoom; the matrix is stretched for some video modes and
 * split screens, and is one of 16 kept in turn.
 * Official Name: camSetFOV
 */
void cam_set_fov(f32 camFieldOfView, s32 force) {
    s32 mode;
    s32 type;
    f32 zoom;

    gCameras[D_800FA634].fov = camFieldOfView;
    if (gCamZoomSet[D_800FA634]) {
        zoom = gCamZoom[D_800FA634] - 1.0f;
        camFieldOfView *= 1.0f / (0.25f * zoom + 1.0f);
    }
    if (CAMERA_MIN_FOV < camFieldOfView && camFieldOfView < CAMERA_MAX_FOV) {
        if (force || camFieldOfView != gCurCamFOV) {
            gCurCamFOV = camFieldOfView;
            guPerspectiveF(gPerspectiveMatrixF, &perspNorm, gCurCamFOV, CAMERA_ASPECT, CAMERA_NEAR, D_800AC410, CAMERA_SCALE);
            D_800A3168 = gPerspectiveMatrixF[0][0] / D_800AC414;
            mode = fb_mode();
            if (D_800FAA5C) {
                gPerspectiveMatrixF[0][0] *= D_800FAA60;
                gPerspectiveMatrixF[1][1] *= D_800FAA64;
                D_800FAA5C = 0;
            } else if (mode & 1) {
                type = level_type();
                if (type == 1 || type == 2) {
                    gPerspectiveMatrixF[1][1] *= D_800AC418;
                } else {
                    gPerspectiveMatrixF[0][0] *= 0.75f;
                }
            } else if (gActiveCameraID == 1 && func_8005AA7C()) {
                gPerspectiveMatrixF[0][0] *= 0.75f;
                gPerspectiveMatrixF[1][1] *= 0.75f;
            }
            D_800A316C = (D_800A316C + 1) & 0xF;
            func_80048D84(&gPerspectiveMatrixF, &gPerspectiveMatrix[D_800A316C]);
        }
    }
}

f32 sqrtf(f32 x);

/* JFG: the distance from the current camera to a point (0x8003FA6C). */
f32 get_distance_to_active_camera(f32 x, f32 y, f32 z) {
    Camera *cam = &gCameras[D_800FA634];
    f32 dx = x - cam->x_position;
    f32 dy = y - cam->y_position;
    f32 dz = z - cam->z_position;

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

INCLUDE_ASM("asm/nonmatchings/40240", func_8003FAEC);

/**
 * Return the index of the active view.
 * 0-3 is players 1-4, and 4-7 is the same, but with 4 added on for cutscenes.
 * Official Name: camGetMode
 */
s32 get_current_viewport(void) {
    return gActiveCameraID;
}

/* Sets the viewport layout and returns the number of cameras for it (0x8003FBDC). */
s32 cam_set_layout(s32 mode) {
    if (mode < 0 || mode >= 4) {
        mode = 0;
    }
    if ((gActiveCameraID = mode) < D_800FA634) {
        D_800FA634 = 0;
    }
    return mode + 1;
}

extern s32 D_800FA634;

/* Returns the index of the active camera (0x8003FC1C). */
s32 cam_get_active_camera_id(void) {
    return D_800FA634;
}

/* Sets the active camera, 0 to 3 (else 0) (0x8003FC2C). */
void set_active_camera(s32 index) {
    if (index < 0 || index >= 4) {
        index = 0;
    }
    D_800FA634 = index;
}

extern u8 D_800A3178[];

/* JFG: a per-camera byte. */
u8 func_8003FC4C(s32 index) {
    return D_800A3178[index];
}

void func_8003FC60(s32 index, s32 value) {
    if (index >= 0 && index < 4) {
        D_800A3178[index] = value;
    }
}

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

void fb_size(u32 *width, u32 *height);

/**
 * Sets the intended viewport to the size passed through by arguments.
 * Official Name: camSetUserView
 */
void viewport_menu_set(s32 viewPortIndex, s32 x1, s32 y1, s32 x2, s32 y2) {
    s32 temp;
    u32 width, height;

    fb_size(&width, &height);

    if (x2 < x1) {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y2 < y1) {
        temp = y1;
        y1 = y2;
        y2 = temp;
    }

    if (x1 >= width || x2 < 0 || y1 >= height || y2 < 0) {
        gScreenViewports[viewPortIndex].scissorX1 = 0;
        gScreenViewports[viewPortIndex].scissorY1 = 0;
        gScreenViewports[viewPortIndex].scissorX2 = 0;
        gScreenViewports[viewPortIndex].scissorY2 = 0;
    } else {
        if (x1 < 0) {
            gScreenViewports[viewPortIndex].scissorX1 = 0;
        } else {
            gScreenViewports[viewPortIndex].scissorX1 = x1;
        }
        if (y1 < 0) {
            gScreenViewports[viewPortIndex].scissorY1 = 0;
        } else {
            gScreenViewports[viewPortIndex].scissorY1 = y1;
        }
        if (x2 >= width) {
            gScreenViewports[viewPortIndex].scissorX2 = width - 1;
        } else {
            gScreenViewports[viewPortIndex].scissorX2 = x2;
        }
        if (y2 >= height) {
            gScreenViewports[viewPortIndex].scissorY2 = height - 1;
        } else {
            gScreenViewports[viewPortIndex].scissorY2 = y2;
        }
    }
    gScreenViewports[viewPortIndex].x1 = x1;
    gScreenViewports[viewPortIndex].x2 = x2;
    gScreenViewports[viewPortIndex].y1 = y1;
    gScreenViewports[viewPortIndex].y2 = y2;
}

#define VIEWPORT_AUTO 0x8000

/* Sets the selected viewport's coordinate offsets and view size (0x800400D8). */
/**
 * Set the selected viewport's coordinate offsets and view size.
 * JFG: the Y offset goes to posY (DKR writes posX).
 * DKR: set_viewport_properties
 */
void set_viewport_properties(s32 viewPortIndex, s32 posX, s32 posY, s32 width, s32 height) {
    if (posX != VIEWPORT_AUTO) {
        gScreenViewports[viewPortIndex].posX = posX;
        gScreenViewports[viewPortIndex].flags |= VIEWPORT_X_CUSTOM;
    } else {
        gScreenViewports[viewPortIndex].flags &= ~VIEWPORT_X_CUSTOM;
    }
    if (posY != VIEWPORT_AUTO) {
        gScreenViewports[viewPortIndex].posY = posY;
        gScreenViewports[viewPortIndex].flags |= VIEWPORT_Y_CUSTOM;
    } else {
        gScreenViewports[viewPortIndex].flags &= ~VIEWPORT_Y_CUSTOM;
    }
    if (width != VIEWPORT_AUTO) {
        gScreenViewports[viewPortIndex].width = width;
        gScreenViewports[viewPortIndex].flags |= VIEWPORT_WIDTH_CUSTOM;
    } else {
        gScreenViewports[viewPortIndex].flags &= ~VIEWPORT_WIDTH_CUSTOM;
    }
    if (height != VIEWPORT_AUTO) {
        gScreenViewports[viewPortIndex].height = height;
        gScreenViewports[viewPortIndex].flags |= VIEWPORT_HEIGHT_CUSTOM;
    } else {
        gScreenViewports[viewPortIndex].flags &= ~VIEWPORT_HEIGHT_CUSTOM;
    }
}

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

/**
 * Writes the window limits of a viewport for the given screen layout.
 * JFG: the viewport's own scissor if it has one; otherwise the framebuffer,
 * split for two to four players.
 * Official name: camGetWindowLimits
 */
void copy_framebuffer_size_to_coords(s32 layout, s32 viewPortIndex, u32 *ulx, u32 *uly, u32 *lrx, u32 *lry) {
    s32 mode;
    u32 halfWidth;
    u32 halfHeight;

    if (gScreenViewports[viewPortIndex].flags & VIEWPORT_EXTRA_BG) {
        *ulx = gScreenViewports[viewPortIndex].scissorX1;
        *uly = gScreenViewports[viewPortIndex].scissorY1;
        *lrx = gScreenViewports[viewPortIndex].scissorX2;
        *lry = gScreenViewports[viewPortIndex].scissorY2;
        return;
    }
    fb_size(lrx, lry);
    mode = fb_mode();
    halfWidth = *lrx >> 1;
    halfHeight = *lry >> 1;
    *ulx = 0;
    *uly = 0;
    switch (layout) {
        case 0:
            break;
        case 1:
            if ((mode & 1) || func_8005AA7C()) {
                if (gScreenViewports == &gScreenViewports[viewPortIndex]) {
                    *lrx = halfWidth;
                } else {
                    *ulx = halfWidth;
                }
            } else {
                if (gScreenViewports == &gScreenViewports[viewPortIndex]) {
                    *lry = halfHeight;
                } else {
                    *uly = halfHeight;
                }
            }
            break;
        case 2:
        case 3:
            if (viewPortIndex & 1) {
                *ulx = halfWidth;
            } else {
                *lrx = halfWidth;
            }
            if (viewPortIndex & 2) {
                *uly = halfHeight;
            } else {
                *lry = halfHeight;
            }
            break;
    }
}

INCLUDE_ASM("asm/nonmatchings/40240", func_8004046C);

void copy_framebuffer_size_to_coords(s32 layout, s32 viewPortIndex, u32 *ulx, u32 *uly, u32 *lrx, u32 *lry);

/**
 * Takes the size of the screen as depicted by the active viewport, then sets the RDP scissor to match it.
 * JFG: the window limits come from copy_framebuffer_size_to_coords.
 * Official Name: camSetScissor
 */
void viewport_scissor(Gfx **dList) {
    u32 ulx, uly, lrx, lry;

    copy_framebuffer_size_to_coords(gActiveCameraID, D_800FA634, &ulx, &uly, &lrx, &lry);
    gDPSetScissor((*dList)++, G_SC_NON_INTERLACE, ulx, uly, lrx, lry);
}

extern MtxF gPerspectiveMatrixF;
void func_80048C7C(MtxF *a, MtxF *b, MtxF *out);

/* Writes the view matrix times the perspective matrix (0x80040A70). */
void mtx_view_persp_get(s32 arg0, MtxF *out) {
    func_80048C7C(&gViewMatrixF, &gPerspectiveMatrixF, out);
}

INCLUDE_ASM("asm/nonmatchings/40240", func_80040AA4);

/**
 * Sets the Y value of the Y axis in the matrix to the passed value.
 * This is used to vertically scale ortho geometry to look identical across NTSC and PAL systems.
 * Official Name: camOrthoYAspect
 */
void set_ortho_matrix_height(f32 value) {
    gOrthoMatrixF[1][1] = value;
}

extern u16 perspNorm;   // the perspective normalisation value
extern s16 D_800A30F8[]; // an ObjectTransform (DKR: D_800DD288)
extern MtxF D_800FAFF8;  // DKR: gCurrentModelMatrixF
extern MtxF gViewProjMatrixF;
extern Vp gViewportStack[];
extern s32 gViewportWithBG;
void func_8004A1A0(s16 *trans, MtxF *out);
void func_80048D84(MtxF *in, Mtx *out);
void func_80042134(MtxF *out);

/**
 * Sets the current matrix to represent an orthogonal view.
 * Used for drawing triangles on screen as HUD.
 * JFG: the viewport is set first, and the ortho matrix is copied by a function.
 * Official Name: camStandardOrtho
 */
void mtx_ortho(Gfx **dList, Mtx **mtx) {
    u32 width, height;

    fb_size(&width, &height);
    gViewportStack[D_800FA634 + 5].vp.vscale[0] = width * 2;
    gViewportStack[D_800FA634 + 5].vp.vscale[1] = width * 2;
    gViewportStack[D_800FA634 + 5].vp.vtrans[0] = width * 2;
    gViewportStack[D_800FA634 + 5].vp.vtrans[1] = height * 2;
    gSPViewport((*dList)++, OS_K0_TO_PHYSICAL(&gViewportStack[D_800FA634 + 5]));
    func_80042134(&gViewProjMatrixF);
    func_80048D84(&gViewProjMatrixF, *mtx);
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

/**
 * Sets the current matrix to represent a perspective view.
 * Necessary for setting up any 3D scene.
 * JFG: sets the perspective normalisation (old GBI form), one matrix less.
 * Official Name: camStandardPersp
 */
void mtx_perspective(Gfx **dList, Mtx **mtx) {
    gImmp1((*dList)++, G_RDPHALF_1, perspNorm);
    func_8004A1A0(D_800A30F8, &D_800FAFF8);
    func_80048C7C(&D_800FAFF8, &gPerspectiveMatrixF, &gViewProjMatrixF);
    func_80048D84(&gViewProjMatrixF, *mtx);
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

/**
 * Sets the RSP viewport onscreen to the given properties.
 * Viewports have a centre position and a scale factor, rather than a standard four corners.
 * JFG: no mirror cheat or anti-tamper flip.
 * Official Name: camSetViewport
 */
void viewport_rsp_set(Gfx **dList, s32 halfWidth, s32 halfHeight, s32 centerX, s32 centerY) {
    Vp *viewport;
    s32 index;
    index = D_800FA634;
    if (!(gScreenViewports[index].flags & VIEWPORT_EXTRA_BG)) {
        viewport = &gViewportStack[index];
        viewport->vp.vtrans[0] = centerX * 4;
        viewport->vp.vtrans[1] = centerY * 4;
        viewport->vp.vscale[0] = halfWidth * 4;
        viewport->vp.vscale[1] = halfHeight * 4;
    } else {
        index += gViewportWithBG * 5 + 10;
        viewport = &gViewportStack[index];
    }
    gSPViewport((*dList)++, OS_K0_TO_PHYSICAL(viewport));
}

void func_800408C0(Gfx **dList);

/**
 * Resets the viewport back to default.
 * JFG: no small-screen view; the scissor and viewport cover the framebuffer.
 * Official Name: camResetView
 */
void viewport_reset(Gfx **dList) {
    u32 height, width;
    D_800FA634 = 4;
    fb_size(&width, &height);
    if (!(gScreenViewports[D_800FA634].flags & VIEWPORT_EXTRA_BG)) {
        gDPSetScissor((*dList)++, G_SC_NON_INTERLACE, 0, 0, width - 1, height - 1);
        viewport_rsp_set(dList, width >> 1, height >> 1, width >> 1, height >> 1);
    } else {
        func_800408C0(dList);
        viewport_rsp_set(dList, 0, 0, 0, 0);
    }
    D_800FA634 = 0;
}

void func_80041168(s32 arg0, s32 arg1) {
}

INCLUDE_ASM("asm/nonmatchings/40240", func_80041174);

INCLUDE_ASM("asm/nonmatchings/40240", func_80041450);

INCLUDE_ASM("asm/nonmatchings/40240", func_800416CC);

extern MtxF D_800FAAB0;
extern Mtx *D_800FAB30;
extern MtxF gCurrentMVPMatrixF;

/* JFG: keep a model matrix and upload it times the view-projection as the MVP matrix (0x800419AC). */
void mtx_model_push(Gfx **dList, Mtx **mtx, MtxF *in) {
    s32 i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            D_800FAAB0[i][j] = (*in)[i][j];
        }
    }
    func_80048C7C(in, &gViewProjMatrixF, &gCurrentMVPMatrixF);
    func_80048D84(&gCurrentMVPMatrixF, *mtx);
    D_800FAB30 = *mtx;
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_PROJECTION);
}

INCLUDE_ASM("asm/nonmatchings/40240", func_80041A90);

extern MtxF D_800FAAB0; // DKR: gCurrentModelMatrixF
extern MtxF gCurrentMVPMatrixF;
extern Mtx *D_800FAB30;
void func_80049874(f32 x, f32 y, f32 z, MtxF *out); // a scale matrix

/* JFG: upload a scale matrix (unless the scale is 1) as the MVP matrix (0x80041BE0). */
void mtx_scale_push(Gfx **dList, Mtx **mtx, f32 scale) {
    if (scale != 1.0f) {
        func_80049874(scale, scale, scale, &D_800FAAB0);
        func_80048C7C(&D_800FAAB0, &gViewProjMatrixF, &gCurrentMVPMatrixF);
        func_80048D84(&gCurrentMVPMatrixF, *mtx);
        D_800FAB30 = *mtx;
        gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_PROJECTION);
    }
}
void func_80049DC0(void *trans, MtxF *out);
void func_80048F4C(MtxF *mtx, f32 offsetY);
void func_80048F20(MtxF *mtx, f32 scaleY);

/**
 * Generates the model's MVP matrix from its transform and uploads it to the RSP.
 * JFG: no model matrix stack or camera-relative position.
 * Official Name: camPushModelMtx
 */
void mtx_cam_push(Gfx **dList, Mtx **mtx, void *trans, f32 scaleY, f32 offsetY) {
    func_80049DC0(trans, &D_800FAAB0);
    if (offsetY != 0.0f) {
        func_80048F4C(&D_800FAAB0, offsetY);
    }
    if (scaleY != 1.0f) {
        func_80048F20(&D_800FAAB0, scaleY);
    }
    func_80048C7C(&D_800FAAB0, &gViewProjMatrixF, &gCurrentMVPMatrixF);
    func_80048D84(&gCurrentMVPMatrixF, *mtx);
    D_800FAB30 = *mtx;
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_PROJECTION);
}

/* JFG: an old-GBI G_MOVEWORD (index 0x0A, value 0) (0x80041DA0). */
void mtx_pop(Gfx **dList) {
    Gfx *_g = (*dList)++;
    _g->words.w0 = 0xBC00000A;
    _g->words.w1 = 0;
}

void func_80041DC0(Gfx **dList) {
    Gfx *_g = (*dList)++;
    _g->words.w0 = 0xBC00000A;
    _g->words.w1 = 0;
}

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

/* Returns the view-projection matrix (0x80041E40). */
MtxF *get_view_proj_matrix_f32(void) {
    return &gViewProjMatrixF;
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

/* JFG: the view-space depth of a point (the third column of the view matrix) (0x800420F0). */
f32 get_distance_to_camera(f32 x, f32 y, f32 z) {
    f32 result;

    result = (x * gViewMatrixF[0][2]) + (y * gViewMatrixF[1][2]) + (z * gViewMatrixF[2][2]) + gViewMatrixF[3][2];
    return result;
}

#ifdef NON_MATCHING
/**
 * Copy the ortho matrix.
 * JFG: (from DKR's mtx_ortho) its last element is half the framebuffer width.
 * NON_MATCHING: one extra lui: the target shares %hi of gOrthoMatrixF across its first loads,
 * which needs the matrix defined in this file (an extern gets a new lui per offset).
 */
void func_80042134(MtxF *out) {
    UNUSED s32 pad;
    u32 width, height;
    s32 i;

    fb_size(&width, &height);
    for (i = 0; i < 15; i++) {
        ((f32 *) out)[i] = ((f32 *) gOrthoMatrixF)[i];
    }
    (*out)[3][3] = width >> 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/40240", func_80042134);
#endif

typedef struct CameraShake {
    /* 0x00 */ s16 unk0; // JFG: the end of each phase, in frames
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 frame; // JFG: the frame count
    /* 0x08 */ s32 unk8;
} CameraShake;
extern CameraShake gCameraShakes[4];

/* JFG: start a camera shake with three phases (lengths in seconds) (0x800421F0). */
void cam_shake_start(s32 index, f32 phase1, f32 phase2, f32 phase3, s32 arg4) {
    CameraShake *shake;

    if (index >= 0 && index < 4) {
        shake = &gCameraShakes[index];
        shake->unk0 = phase1 * 60.0f;
        shake->unk2 = shake->unk0 + (s32) (phase2 * 60.0f);
        shake->unk4 = shake->unk2 + (s32) (phase3 * 60.0f);
        shake->frame = 0;
        shake->unk8 = arg4;
    }
}

/* JFG: stop the shakes of the four cameras (0x800422C4). */
void cam_shake_stop_all(void) {
    s32 i;
    Camera *cam;
    CameraShake *shake;

    gNoCamShake = 0;
    cam = gCameras;
    shake = gCameraShakes;
    i = 4;
    while (i--) {
        cam->shakeX = 0.0f;
        cam->shakeY = 0.0f;
        cam->shakeZ = 0.0f;
        shake->unk8 = 0;
        cam++;
        shake++;
    }
}

INCLUDE_ASM("asm/nonmatchings/40240", func_80042310);

extern u8 gCamZoomSet[4];
extern f32 gCamZoom[4];

/* Sets a camera's FOV zoom (cameras 0 to 3) (0x80042438). */
void cam_fov_zoom_set(s32 index, f32 zoom) {
    if (index >= 0 && index < 4) {
        gCamZoomSet[index] = 1;
        gCamZoom[index] = zoom;
    }
}

typedef struct MatrixListEntry {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s16 unk4;
} MatrixListEntry;
extern MatrixListEntry D_800FA658[];
extern s32 D_800A2ED0;

void func_80042470(s32 arg0, s32 arg1) {
    D_800FA658[D_800A2ED0].unk0 = arg0;
    D_800FA658[D_800A2ED0++].unk4 = arg1;
}

INCLUDE_ASM("asm/nonmatchings/40240", func_800424A0);
