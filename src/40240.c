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

extern s8 D_800FA650; // DKR: gAdjustViewportHeight
extern s32 D_800A2ED0;
extern MtxF gPerspectiveMatrixF;
extern u16 perspNorm;
void cam_reset(s32 x, s32 y, s32 z, s32 angleZ, s32 angleX, s32 angleY);
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
        cam_reset(200, 200, 200, 0, 0, 180);
        gCameras[D_800FA634].fov = CAMERA_DEFAULT_FOV;
    }
    cam_shake_stop_all();
    if (arg0) {
        D_800A2ED0 = 0;
    }
    gCurCamFOV = CAMERA_DEFAULT_FOV;
    guPerspectiveF(gPerspectiveMatrixF, &perspNorm, gCurCamFOV, CAMERA_ASPECT, CAMERA_NEAR, 15000.0f, CAMERA_SCALE);
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
            guPerspectiveF(gPerspectiveMatrixF, &perspNorm, gCurCamFOV, CAMERA_ASPECT, CAMERA_NEAR, 15000.0f, CAMERA_SCALE);
            D_800A3168 = gPerspectiveMatrixF[0][0] / 1.817394f;
            mode = fb_mode();
            if (D_800FAA5C) {
                gPerspectiveMatrixF[0][0] *= D_800FAA60;
                gPerspectiveMatrixF[1][1] *= D_800FAA64;
                D_800FAA5C = 0;
            } else if (mode & 1) {
                type = level_type();
                if (type == 1 || type == 2) {
                    gPerspectiveMatrixF[1][1] *= 1.33f;
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

extern u8 D_800A3170[];

/* JFG: reset the current camera: position (twice), angles in degrees, no shake (DKR camera_reset). */
void cam_reset(s32 x, s32 y, s32 z, s32 angleZ, s32 angleX, s32 angleY) {
    Camera *cam = &gCameras[D_800FA634];
    f32 fx, fy, fz;

    cam->z_rotation = angleZ * 0xB6;
    cam->x_rotation = angleX * 0xB6;
    cam->y_rotation = angleY * 0xB6;
    cam->unk4A = 0;
    fx = x;
    fy = y;
    fz = z;
    cam->x_position = fx;
    cam->y_position = fy;
    cam->z_position = fz;
    cam->unk18 = fx;
    cam->unk1C = fy;
    cam->unk20 = fz;
    cam->unk3C = 0;
    cam->shakeX = 0.0f;
    cam->shakeY = 0.0f;
    cam->shakeZ = 0.0f;
    cam->unk24 = 128.0f;
    cam->unk28 = 32.0f;
    cam->unk3D = D_800A3170[D_800FA634];
    cam->unk3E = -1;
    cam->unk40 = 0.0f;
}

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
        stubbed_printf("Camera Error: Illegal mode!\n");
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
        stubbed_printf("Illegal CAMERA number %d\n", index);
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

extern s32 gViewportWithBG;
extern Vp gViewportStack[];

/**
 * Takes the size of each view frame and writes them to the viewport stack, using values compatable with the RSP.
 * Only does this if extended backgrounds are enabled.
 * JFG: no mirror cheat.
 * DKR: copy_viewports_to_stack
 */
void copy_viewports_to_stack(void) {
    s32 width;
    s32 height;
    s32 port;
    s32 yPos;
    s32 xPos;
    s32 i;

    gViewportWithBG = 1 - gViewportWithBG;
    for (i = 0; i < 4; i++) {
        if (gScreenViewports[i].flags & VIEWPORT_UNK_04) {
            gScreenViewports[i].flags &= ~VIEWPORT_EXTRA_BG;
        } else if (gScreenViewports[i].flags & VIEWPORT_UNK_02) {
            gScreenViewports[i].flags |= VIEWPORT_EXTRA_BG;
        }
        gScreenViewports[i].flags &= ~(VIEWPORT_UNK_02 | VIEWPORT_UNK_04);

        if (gScreenViewports[i].flags & VIEWPORT_EXTRA_BG) {
            if (!(gScreenViewports[i].flags & VIEWPORT_X_CUSTOM)) {
                xPos = (((gScreenViewports[i].x2 - gScreenViewports[i].x1) + 1) << 1) + (gScreenViewports[i].x1 * 4);
            } else {
                xPos = gScreenViewports[i].posX;
                xPos *= 4;
            }
            if (!(gScreenViewports[i].flags & VIEWPORT_Y_CUSTOM)) {
                yPos = (((gScreenViewports[i].y2 - gScreenViewports[i].y1 + 1)) << 1) + (gScreenViewports[i].y1 * 4);
            } else {
                yPos = gScreenViewports[i].posY;
                yPos *= 4;
            }
            if (!(gScreenViewports[i].flags & VIEWPORT_WIDTH_CUSTOM)) {
                width = gScreenViewports[i].x2 - gScreenViewports[i].x1;
                width += 1;
                width *= 2;
            } else {
                width = gScreenViewports[i].width;
                width *= 2;
            }
            if (!(gScreenViewports[i].flags & VIEWPORT_HEIGHT_CUSTOM)) {
                height = (gScreenViewports[i].y2 - gScreenViewports[i].y1) + 1;
                height *= 2;
            } else {
                height = gScreenViewports[i].height;
                height *= 2;
            }
            port = i;
            port += gViewportWithBG * 5 + 10;
            gViewportStack[port].vp.vtrans[0] = xPos;
            gViewportStack[port].vp.vtrans[1] = yPos;
            gViewportStack[port].vp.vscale[0] = width;
            gViewportStack[port].vp.vscale[1] = height;
        }
    }
}

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

void copy_framebuffer_size_to_coords(s32 layout, s32 viewPortIndex, u32 *ulx, u32 *uly, u32 *lrx, u32 *lry);
void viewport_rsp_set(Gfx **dList, s32 width, s32 height, s32 posX, s32 posY);
void func_80040AA4(Gfx **dList, Mtx **mtx);

/* JFG: set up the active viewport: the scissor from the window limits, the RSP viewport (half size 160x120,
 * 224x168 in the high-res modes, halved again for three or four players; scaled up by the camera's zoom, with
 * the clip ratio reset), then the projection matrix if a matrix list is given */
void viewport_apply(Gfx **dList, Mtx **mtx) {
    s32 mode;
    f32 extra;
    u32 ulx;
    u32 uly;
    u32 lrx;
    u32 lry;
    u32 width;
    u32 height;
    f32 zoom;

    copy_framebuffer_size_to_coords(gActiveCameraID, D_800FA634, &ulx, &uly, &lrx, &lry);
    mode = fb_mode();
    if (mode == 2 || mode == 3) {
        width = 224;
        height = 168;
    } else {
        width = 160;
        height = 120;
    }
    if (gActiveCameraID >= 2) {
        width >>= 1;
        height >>= 1;
    }
    if (gCamZoomSet[D_800FA634]) {
        extra = gCamZoom[D_800FA634] - 1.0f;
        zoom = 0.25f * extra + 1.0f;
        width = width * zoom;
        height = height * zoom;
        gSPClipRatio((*dList)++, FRUSTRATIO_1);
    }
    gDPSetScissor((*dList)++, G_SC_NON_INTERLACE, ulx, uly, lrx, lry);
    viewport_rsp_set(dList, width, height, (lrx + ulx) >> 1, (lry + uly) >> 1);
    if (mtx != NULL) {
        func_80040AA4(dList, mtx);
    }
}

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

/* JFG: what a billboard sprite is drawn from (an object's transform and more) */
typedef struct BillboardTrans {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ s32 frame;
} BillboardTrans;

extern BillboardTrans D_800FA638; // DKR: gCameraTransform
extern u8 gCamZoomSet[];           // JFG: per camera, set to force the FoV to be applied again
extern s32 gNoCamShake;            // DKR: gNoCamShake (JFG: the shake moves all three axes)
extern MtxF gViewProjMatrixF;
extern u16 perspNorm;
extern Mtx D_800FAF38; // DKR: gInverseViewMatrix
void func_8004A1A0(s16 *trans, MtxF *out); // DKR: mtxf_from_inverse_transform
void func_80049F08(s16 *trans, MtxF *out); // DKR: mtxf_from_transform
void func_80048D84(MtxF *in, Mtx *out);

/**
 * Sets the view and projection matrices for the active camera.
 * JFG: applies the camera's FoV first if it changed, the view matrix is always built, the display list part is
 * skipped without a list, and the shake offsets all three axes.
 * DKR: func_80067D3C (Official Name: camGetPlayerProjMtx / camSetProjMtx - ??)
 */
void func_80040AA4(Gfx **dList, Mtx **mtx) {
    Camera *cam = &gCameras[D_800FA634];

    if (gCurCamFOV != cam->fov || gCamZoomSet[D_800FA634]) {
        cam_set_fov(cam->fov, 0);
    }
    if (dList != NULL) {
        gImmp1((*dList)++, G_RDPHALF_1, perspNorm);
    }
    D_800FA638.rotation[0] = cam->y_rotation + 0x8000;
    D_800FA638.rotation[1] = cam->x_rotation + cam->unk4A;
    D_800FA638.rotation[2] = cam->z_rotation;
    D_800FA638.x = -cam->x_position;
    D_800FA638.y = -cam->y_position;
    D_800FA638.z = -cam->z_position;
    if (gNoCamShake) {
        D_800FA638.x -= cam->shakeX;
        D_800FA638.y -= cam->shakeY;
        D_800FA638.z -= cam->shakeZ;
    }
    func_8004A1A0(D_800FA638.rotation, &gViewMatrixF);
    func_80048C7C(&gViewMatrixF, &gPerspectiveMatrixF, &gViewProjMatrixF);
    if (dList != NULL) {
        func_80048D84(&gViewProjMatrixF, *mtx);
        gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    }
    D_800FA638.rotation[0] = -0x8000 - cam->y_rotation;
    D_800FA638.rotation[1] = -(cam->x_rotation + cam->unk4A);
    D_800FA638.rotation[2] = -cam->z_rotation;
    D_800FA638.x = cam->x_position;
    D_800FA638.y = cam->y_position;
    D_800FA638.z = cam->z_position;
    if (gNoCamShake) {
        D_800FA638.x += cam->shakeX;
        D_800FA638.y += cam->shakeY;
        D_800FA638.z += cam->shakeZ;
    }
    func_80049F08(D_800FA638.rotation, &gInverseViewMatrixF);
    func_80048D84(&gInverseViewMatrixF, &D_800FAF38);
}

/**
 * Sets the Y value of the Y axis in the matrix to the passed value.
 * This is used to vertically scale ortho geometry to look identical across NTSC and PAL systems.
 * Official Name: camOrthoYAspect
 */
void set_ortho_matrix_height(f32 value) {
    stubbed_printf("%f\n", value); // a debug string with no reference; its function is a guess (the order is the ROM's)
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

/* JFG: the game's vertex (DKR's Vertex) */
typedef struct Vertex40 {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} Vertex40;

extern MtxF D_800FAAB0; // DKR: gCurrentModelMatrixF
extern Mtx *D_800FAB30;
extern f32 D_800FECC4; // JFG: the screen's aspect ratio
s32 fb_mode(void);
void func_8004924C(s32 angle, f32 scaleX, f32 scaleY, MtxF *out); // DKR: mtxf_billboard
void func_80049BA0(MtxF *mtx);                                     // an identity matrix
void func_80056070(Gfx **dList, void *sprite, s32 flags, s32 frame, s32 alpha);

/* JFG: draw a sprite as a billboard at a point: one vertex, a matrix from the camera's roll, the
   scale (and the aspect ratio) and a vertical flip if asked (flag 0x20000000), then the frame. */
void render_sprite_billboard(Gfx **dList, Mtx **mtx, Vertex40 **verts, BillboardTrans *trans, void *sprite, s32 flags,
                   u8 alpha) {
    s32 angle;
    f32 scale;
    f32 aspect;
    Vertex40 *v;

    v = *verts;
    v->x = trans->x;
    v->y = trans->y;
    v->z = trans->z;
    v->r = 255;
    v->g = 255;
    v->b = 255;
    v->a = 255;
    gDma1p((*dList)++, G_VTX, OS_K0_TO_PHYSICAL(*verts), 18, (OS_K0_TO_PHYSICAL(*verts) & 6) | 8);
    (*verts)++;
    angle = trans->rotation[2] + gCameras[D_800FA634].z_rotation;
    scale = trans->scale * D_800A3168;
    aspect = D_800FECC4;
    if (fb_mode() & 1) {
        scale *= 0.75f;
        aspect *= 1.3334f;
    }
    func_8004924C(angle, scale, aspect, &D_800FAAB0);
    if (flags & 0x20000000) {
        func_80049BA0(&D_800FAFF8);
        D_800FAFF8[1][1] = -1.0f;
        func_80048C7C(&D_800FAAB0, &D_800FAFF8, &D_800FAAB0);
    }
    func_80048D84(&D_800FAAB0, *mtx);
    D_800FAB30 = *mtx;
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gMoveWd((*dList)++, 2, 0, 1);
    flags &= ~1;
    if (flags & 4) {
        flags |= 1;
    }
    func_80056070(dList, sprite, flags & 0xF, trans->frame, alpha);
    gMoveWd((*dList)++, 0xA, 0, 0);
    gMoveWd((*dList)++, 2, 0, 0);
}


/* JFG: render_sprite_billboard for a sprite at a given point, roll and scale instead of a transform. */
void render_sprite_billboard_at(Gfx **dList, Mtx **mtx, Vertex40 **verts, void *sprite, s16 x, s16 y, s16 z, s16 roll,
                   f32 scale, s32 frame, s32 flags, u8 alpha) {
    s32 angle;
    f32 aspect;
    Vertex40 *v;

    v = *verts;
    v->x = x;
    v->y = y;
    v->z = z;
    v->r = 255;
    v->g = 255;
    v->b = 255;
    v->a = 255;
    gDma1p((*dList)++, G_VTX, OS_K0_TO_PHYSICAL(*verts), 18, (OS_K0_TO_PHYSICAL(*verts) & 6) | 8);
    (*verts)++;
    angle = gCameras[D_800FA634].z_rotation + roll;
    scale *= D_800A3168;
    aspect = D_800FECC4;
    if (fb_mode() & 1) {
        scale *= 0.75f;
        aspect *= 1.3334f;
    }
    func_8004924C(angle, scale, aspect, &D_800FAAB0);
    if (flags & 0x20000000) {
        func_80049BA0(&D_800FAFF8);
        D_800FAFF8[1][1] = -1.0f;
        func_80048C7C(&D_800FAAB0, &D_800FAFF8, &D_800FAAB0);
    }
    func_80048D84(&D_800FAAB0, *mtx);
    D_800FAB30 = *mtx;
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gMoveWd((*dList)++, 2, 0, 1);
    flags &= ~1;
    if (flags & 4) {
        flags |= 1;
    }
    func_80056070(dList, sprite, flags & 0xF, frame, alpha);
    gMoveWd((*dList)++, 0xA, 0, 0);
    gMoveWd((*dList)++, 2, 0, 0);
}

extern BillboardTrans D_800FA638; // DKR: gCameraTransform
void func_800495D8(MtxF *out, f32 x, f32 y, f32 z); // DKR: mtxf_from_scale

/**
 * Draws a sprite as an orthogonal image at a point, turned and scaled by its transform.
 * JFG: one matrix (no model matrix stack), the frame from the transform and an extra argument.
 * DKR: render_ortho_triangle_image
 */
void render_ortho_triangle_image(Gfx **dList, Mtx **mtx, Vertex40 **verts, BillboardTrans *trans, void *sprite,
                                 s32 flags, u8 alpha) {
    UNUSED s32 pad;
    f32 scale;
    MtxF aspectMtxF;
    MtxF scaleMtxF;
    Vertex40 *v;

    if (sprite == NULL) {
        stubbed_printf("\nCam do 2D sprite called with NULL pointer!");
        return;
    }
    v = *verts;
    v->x = trans->x;
    v->y = trans->y;
    v->z = trans->z;
    v->r = 255;
    v->g = 255;
    v->b = 255;
    v->a = 255;
    gDma1p((*dList)++, G_VTX, OS_K0_TO_PHYSICAL(*verts), 18, (OS_K0_TO_PHYSICAL(*verts) & 6) | 8);
    (*verts)++;
    D_800FA638.rotation[0] = -trans->rotation[0];
    D_800FA638.rotation[1] = -trans->rotation[1];
    D_800FA638.rotation[2] = trans->rotation[2] + gCameras[D_800FA634].z_rotation;
    D_800FA638.x = 0.0f;
    D_800FA638.y = 0.0f;
    D_800FA638.z = 0.0f;
    if (D_800FA650) {
        scale = trans->scale;
        func_800495D8(&scaleMtxF, scale, scale, 1.0f);
        func_8004924C(0, 1.0f, D_800FECC4, &aspectMtxF);
        func_80048C7C(&aspectMtxF, &scaleMtxF, &D_800FAFF8);
    } else {
        scale = trans->scale;
        func_800495D8(&D_800FAFF8, scale, scale, 1.0f);
    }
    func_8004A1A0(D_800FA638.rotation, &aspectMtxF);
    func_80048C7C(&D_800FAFF8, &aspectMtxF, &D_800FAAB0);
    func_80048D84(&D_800FAAB0, *mtx);
    D_800FAB30 = *mtx;
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gMoveWd((*dList)++, 2, 0, 1);
    func_80056070(dList, sprite, flags, trans->frame, alpha);
    gMoveWd((*dList)++, 0xA, 0, 0);
    gMoveWd((*dList)++, 2, 0, 0);
}

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

void func_80048B60(MtxF *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); // transform a point

/* JFG: the model matrix = rot (its translation dropped) times the current one, moved to pos; upload the MVP. */
void mtx_model_rotate_translate_push(Gfx **dList, Mtx **mtx, f32 *pos, MtxF *rot) {
    f32 out[3];
    MtxF tmp;
    s32 i;

    D_800FAAB0[3][0] = 0.0f;
    D_800FAAB0[3][1] = 0.0f;
    D_800FAAB0[3][2] = 0.0f;
    func_80048C7C(rot, &D_800FAAB0, &tmp);
    i = 0;
    while (i < 16) {
        ((f32 *) D_800FAAB0)[i] = ((f32 *) tmp)[i];
        i++;
    }
    func_80048B60(&D_800FAAB0, pos[0], pos[1], pos[2], &out[0], &out[1], &out[2]);
    D_800FAAB0[3][0] = pos[0];
    D_800FAAB0[3][1] = pos[1];
    D_800FAAB0[3][2] = pos[2];
    func_80048C7C(&D_800FAAB0, &gViewProjMatrixF, &gCurrentMVPMatrixF);
    func_80048D84(&gCurrentMVPMatrixF, *mtx);
    D_800FAB30 = *mtx;
    gSPMatrix((*dList)++, OS_K0_TO_PHYSICAL((*mtx)++), G_MTX_PROJECTION);
}

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
    stubbed_printf("CAM Error!! Convlist overflow.\n"); // unreferenced debug string; its function is a guess
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

void func_80048B60(MtxF *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); // transform a point

#ifdef NON_MATCHING
/* NON_MATCHING: float registers of px/py and the order of the viewport loads differ */
/* JFG: project a view-space point (world space with toView) to the current viewport; FALSE when it is not at
 * least 2 units in front. */
s32 func_80041E58(f32 x, f32 y, f32 z, f32 *outX, f32 *outY, u8 toView) {
    s32 visible = FALSE;
    f32 px;
    f32 py;
    f32 w;
    Vp *viewport;
    f32 scaleX, centreX, scaleY;

    if (toView) {
        func_80048B60(&gViewMatrixF, x, y, z, &x, &y, &z);
    }
    px = gPerspectiveMatrixF[0][0] * x;
    py = gPerspectiveMatrixF[1][1] * y;
    w = -(gPerspectiveMatrixF[2][3] * z);
    if (w < -2.0f) {
        viewport = &gViewportStack[D_800FA634];
        if (gScreenViewports[D_800FA634].flags & VIEWPORT_EXTRA_BG) {
            viewport += gViewportWithBG * 5 + 10;
        }
        scaleX = viewport->vp.vscale[0] >> 2;
        centreX = viewport->vp.vtrans[0] >> 2;
        scaleY = viewport->vp.vscale[1] >> 2;
        *outX = centreX - px * scaleX / w;
        visible = TRUE;
        *outY = (viewport->vp.vtrans[1] >> 2) + py * scaleY / w;
    }
    return visible;
}
#else
INCLUDE_ASM("asm/nonmatchings/40240", func_80041E58);
#endif

void func_80048B60(MtxF *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); // transform a point

/* JFG: screen coordinates at depth *outZ back to view space (and to world space with toWorld). */
void screen_to_view(f32 x, f32 y, f32 *outX, f32 *outY, f32 *outZ, u8 toWorld) {
    f32 scaleX;
    f32 scaleY;
    f32 centreX;
    f32 centreY;
    f32 w;
    Vp *viewport;

    w = (*outZ * gPerspectiveMatrixF[2][2]) * gPerspectiveMatrixF[2][3];
    viewport = &gViewportStack[D_800FA634];
    scaleX = viewport->vp.vscale[0] >> 2;
    scaleY = viewport->vp.vscale[1] >> 2;
    centreX = viewport->vp.vtrans[0] >> 2;
    centreY = viewport->vp.vtrans[1] >> 2;
    *outX = (centreX - x) * w / (gPerspectiveMatrixF[0][0] * scaleX);
    *outY = (y - centreY) * w / (gPerspectiveMatrixF[1][1] * scaleY);
    if (toWorld) {
        func_80048B60(&gInverseViewMatrixF, *outX, *outY, *outZ, outX, outY, outZ);
    }
}

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

/* JFG: shake the cameras near a point: shakeY up to magnitude, fading out over range. */
void cam_shake_near_point(f32 x, f32 y, f32 z, f32 range, f32 magnitude) {
    s32 i;
    Camera *cam;
    f32 dx, dy, dz;
    f32 distance;
    f32 shake;

    for (i = 0; i <= gActiveCameraID; i++) {
        cam = &gCameras[i];
        dx = x - cam->x_position;
        dy = y - cam->y_position;
        dz = z - cam->z_position;
        distance = sqrtf(dx * dx + dy * dy + dz * dz);
        if (distance < range) {
            shake = (range - distance) * magnitude / range;
            if (cam->shakeY < shake) {
                cam->shakeY = shake;
            }
        }
    }
}

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

extern u8 D_800A3178[4];
extern MtxF gViewProjMatrixF;
void func_80074E50(MtxF *mtx);
s32 rand_range(s32 min, s32 max);

#ifdef NON_MATCHING
/* NON_MATCHING: the target forms &D_800FA658 in $a1 before indexing (mine folds %lo into the store), which shifts
   the temporaries of the rest by one */
/* JFG: per frame: end the matrix list, reset the zoom flags and run the camera shakes (fade in over unk0 frames,
 * hold to unk2, fade out to unk4) into each camera's shakeY. */
void func_800424A0(s32 updateRate) {
    s32 i;
    Camera *cam;
    CameraShake *shake;
    s32 magnitude;

    D_800FA658[D_800A2ED0].unk4 = -1;
    func_80074E50(&gViewProjMatrixF);
    D_800A2ED0 = 0;
    gNoCamShake = 0;
    cam = gCameras;
    shake = gCameraShakes;
    i = 4;
    while (i--) {
        gCamZoomSet[i] = 0;
        D_800A3178[i] = 1;
        cam->shakeX = 0.0f;
        cam->shakeY = 0.0f;
        cam->shakeZ = 0.0f;
        if (shake->unk8 != 0) {
            shake->frame += updateRate;
            if (shake->frame >= shake->unk4) {
                shake->unk8 = 0;
            } else {
                gNoCamShake = 1;
                magnitude = shake->unk8;
                if (shake->unk2 < shake->frame) {
                    magnitude = (shake->unk4 - shake->frame) * magnitude / (shake->unk4 - shake->unk2);
                } else if (shake->frame < shake->unk0) {
                    magnitude = shake->frame * magnitude / shake->unk0;
                }
                cam->shakeY = rand_range(0, magnitude);
            }
        }
        cam++;
        shake++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/40240", func_800424A0);
#endif
