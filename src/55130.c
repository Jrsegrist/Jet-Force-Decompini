#include "common.h"
#include "memory.h"
#include "asset_loading.h"
#include "gzip.h"

/* DKR include/sched.h (Rare's scheduler), reduced */
#define OS_SC_ID_VIDEO 2
typedef struct OSSched OSSched;
typedef struct OSScClient {
    struct OSScClient *next;
    OSMesgQueue *msgQ;
} OSScClient;
void osScAddClient(OSSched *sc, OSScClient *c, OSMesgQueue *msgQ, s32 id);

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern s32 gBlockedRenderFlags;
extern u32 gTexColourTag;

#define ASSETCACHE_ID(x) ((x << 1) + 0)
#define ASSETCACHE_PTR(x) ((x << 1) + 1)

/* DKR include/structs.h, with JFG's field offsets */
typedef struct TextureHeader {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 format; // Lower 4 bits determines image format.
    /* 0x03 */ u8 posX;   // JFG: the tile's position in a sprite frame
    /* 0x04 */ u8 posY;
    /* 0x05 */ u8 unk5; // JFG: an effect slot to start (func_80056070)
    /* 0x06 */ s16 flags;
    /* 0x08 */ u16 numberOfInstances;
    /* 0x0A */ s16 numberOfCommands;
    /* 0x0C */ Gfx *cmd;
    /* 0x10 */ u8 pad10[2];
    /* 0x12 */ u16 numOfTextures;
    /* 0x14 */ u16 frameAdvanceDelay;
    /* 0x16 */ s16 textureSize;
    /* 0x18 */ u8 unk18; // JFG: a looping animation runs up to numberOfFrames, not one less (see sprite_anim_advance)
    /* 0x19 */ u8 pad19[2];
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 pad1C[0x20 - 0x1C];
} TextureHeader;

/* DKR's Sprite (renamed: libultra's sp.h has a Sprite too), JFG layout */
typedef struct SpriteAsset {
    /* 0x00 */ u8 numberOfFrames;
    /* 0x01 */ s8 unk1; // JFG: copied from the first tile's texture (unk18)
    /* 0x02 */ s16 numberOfTextures;
    /* 0x04 */ s16 numberOfInstances;
    /* 0x06 */ s16 drawFlags;
    /* 0x08 */ u8 unk8[6]; // JFG: copied from the asset
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ TextureHeader **textures;
    /* 0x14 */ s8 *unk14; // JFG: with draw flag 0x40, the command counts of the frame's parts, 0-terminated
    /* 0x18 */ Gfx *frames[1]; // Actual size varies.
} SpriteAsset;

/* DKR's SpriteAsset (the sprite as loaded from ROM), JFG layout */
typedef struct SpriteAssetData {
    /* 0x00 */ s16 baseTextureId;
    /* 0x02 */ s16 numberOfFrames;
    /* 0x04 */ s16 anchorX;
    /* 0x06 */ s16 anchorY;
    /* 0x08 */ u8 unk8[6];
    /* 0x0E */ s16 flags;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ u8 frameTexOffsets[1]; // actual size is numberOfFrames + 1
} SpriteAssetData;

extern s32 *gTextureCache;
extern s32 gNumberOfLoadedTextures;
extern s32 *gSpriteCache;
extern s32 gSpriteCacheCount;

#define RENDER_NONE 0
extern s32 D_800FED30;
extern s32 D_800FED34;
extern s32 D_800FED38;
extern s32 D_800FED3C;
extern s32 D_800FED40;
extern s32 D_800FED44;

/* video.c (DKR); JFG's video mode table */
#define LOGIC_30FPS 2

typedef struct VideoMode {
    /* 0x00 */ s32 width;
    /* 0x04 */ s32 height;
    /* 0x08 */ s32 unk8;  // the width the horizontal scale is relative to
    /* 0x0C */ s32 unkC;  // the number of lines shown
    /* 0x10 */ s32 unk10; // the vertical offset of the picture
    /* 0x14 */ s32 viMode;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
} VideoMode;

extern VideoMode D_800A392C[];
extern s8 gVideoModeIndex;
extern u8 gVideoDeltaCounter;
extern u8 gVideoDeltaTime;
extern u8 D_800FECAD;
extern s32 D_800FECCC;

// Used to update pulsating lights (DKR include/structs.h)
typedef struct PulsatingLightDataFrame {
    u16 value;
    u16 time;
} PulsatingLightDataFrame;

typedef struct PulsatingLightData {
    u16 numberFrames;
    u16 currentFrame;
    u16 time;
    u16 totalTime;
    s32 outColorValue;
    PulsatingLightDataFrame frames[1]; // Length varies based on numberFrames.
} PulsatingLightData;

INCLUDE_ASM("asm/nonmatchings/55130", func_80054530);

extern s32 gVideoRefreshRate; // Official Name: viFramesPerSecond
extern f32 D_800FECC4;
extern f32 D_800FECC8;
extern OSMesgQueue gVideoMesgQueue;
extern OSMesg gVideoMesgBuf[8];
extern OSScClient gVideoSched;
extern void *D_800A3900[]; // JFG: video buffers ([0] the line buffer, [3] the depth buffer allocation)
extern s8 gVideoVerticalAdjust;
extern s8 D_800FECA5;
extern s8 D_800FECA6;
extern u8 D_800FECA7;
extern s32 D_800FECD0; // JFG: no Z buffer reallocation
extern s32 D_800A3928;
extern s8 sBlackScreenTimer;
void func_8005483C(s32 mode);

/**
 * Set up the VI.
 * JFG: one buffer of 1280 bytes per line (DKR allocates framebuffers here).
 * Official Name: viInit
 */
void video_init(OSSched *sc) {
    s32 height;

    if (osTvType == OS_TV_PAL) {
        gVideoRefreshRate = 50;
        D_800FECC4 = 1.2121212f;
        D_800FECC8 = 1.1f;
        height = 264;
    } else {
        gVideoRefreshRate = 60;
        D_800FECC4 = 1.3333334f;
        D_800FECC8 = 1.0f;
        height = 240;
    }
    osCreateMesgQueue(&gVideoMesgQueue, gVideoMesgBuf, ARRAY_COUNT(gVideoMesgBuf));
    osScAddClient(sc, &gVideoSched, &gVideoMesgQueue, OS_SC_ID_VIDEO);
    D_800A3900[0] = mempool_alloc_safe(height * 1280 + 0x30, -1);
    gVideoVerticalAdjust = 0;
    D_800FECA5 = 0;
    D_800FECA6 = 0;
    D_800FECA7 = 1;
    D_800FECD0 = 0;
    func_8005483C(0);
    D_800A3928 = 1;
    sBlackScreenTimer = 1;
}

extern f32 D_800A391C; // JFG: the screen's x scale (to 320)
extern f32 D_800A3920; // JFG: the screen's y scale (to 240)
extern s32 D_800A3910[3]; // JFG: the framebuffers
extern s8 D_800FECA4; // JFG: the framebuffer index
extern u8 D_800FECA7;
void set_screen_dialogue_box_size(s32 width, s32 height);
void zbuffer_alloc(s32 width, s32 height);
void zbuffer_free(s32 width, s32 height);
void buffer_clear_words(void *buffer, s32 size);
void video_delta_reset(void);
void fb_swap(void);
void fb_init_vi(void);

#ifdef NON_MATCHING
/* JFG: switch to video mode (0-3, offset for MPAL/PAL; mode 0 on the first call): the screen scale,
   the depth buffer, two or three framebuffers (the second allocated in modes 2 and 3, the third when
   asked) cleared unless it is the first call, then the VI.
   NON_MATCHING: register choice only: the target keeps the global addresses in a1/v1/a3/a2 (mine
   v1/a3/a2/v1), which also moves the size + 0x30 into a branch delay slot. */
void func_8005483C(s32 mode) {
    s32 size;
    VideoMode *videoMode;

    gVideoModeIndex = mode & 3 & ~D_800A3928;
    D_800A3928 = 0;
    if (osTvType == OS_TV_MPAL) {
        gVideoModeIndex += 4;
    } else if (osTvType == OS_TV_PAL) {
        gVideoModeIndex += 8;
    }
    videoMode = &D_800A392C[gVideoModeIndex];
    D_800A391C = videoMode->width / 320.0f;
    D_800A3920 = videoMode->height / 240.0f;
    set_screen_dialogue_box_size(videoMode->width, videoMode->height);
    if (!D_800FECD0) {
        zbuffer_free(videoMode->width, videoMode->height);
    }
    if (D_800A3900[1] != NULL) {
        mempool_free(D_800A3900[1]);
        D_800A3900[1] = NULL;
    }
    if (D_800A3900[2] != NULL) {
        mempool_free(D_800A3900[2]);
        D_800A3900[2] = NULL;
    }
    size = videoMode->width * videoMode->height * 2;
    D_800FECA4 = 0;
    D_800A3910[0] = ((s32) D_800A3900[0] + 0x3F) & ~0x3F;
    D_800A3910[1] = 0;
    D_800A3910[2] = 0;
    if ((gVideoModeIndex & 3) != 2 && (gVideoModeIndex & 3) != 3) {
        D_800A3910[1] = D_800A3910[0] + size;
    } else {
        D_800A3900[1] = mempool_alloc_safe(size + 0x30, -1);
        D_800A3910[1] = ((s32) D_800A3900[1] + 0x3F) & ~0x3F;
    }
    if (D_800FECA6) {
        D_800A3900[2] = mempool_alloc_safe(size + 0x30, -1);
        D_800A3910[2] = ((s32) D_800A3900[2] + 0x3F) & ~0x3F;
    }
    D_800FECA5 = D_800FECA6;
    if (D_800FECA7) {
        buffer_clear_words((void *) D_800A3910[0], size);
        buffer_clear_words((void *) D_800A3910[1], size);
        if (D_800FECA5) {
            buffer_clear_words((void *) D_800A3910[2], size);
        }
    }
    D_800FECA7 = 1;
    if (!D_800FECD0) {
        zbuffer_alloc(videoMode->width, videoMode->height);
    }
    video_delta_reset();
    fb_swap();
    fb_init_vi();
    D_800FECD0 = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/55130", func_8005483C);
#endif

extern s32 D_800FECB0; // JFG: the previous frame's image
extern s32 gVideoLastFramebuffer; // the framebuffer to show next
void fb_size(s32 *width, s32 *height);
void fb_init_vi(void);

/* JFG: clear two framebuffers, switch to the black-screen mode for the TV type (unless in mode 0 or 4) and unblank. */
void fb_clear_and_unblank(void) {
    s32 *pixel;
    u32 count;
    s32 width, height;

    fb_size(&width, &height);
    pixel = (s32 *) D_800FECB0;
    count = ((u32) width * height) >> 1;
    while (count--) {
        *pixel++ = 0;
    }
    pixel = (s32 *) gVideoLastFramebuffer;
    count = ((u32) width * height) >> 1;
    while (count--) {
        *pixel++ = 0;
    }
    osWritebackDCacheAll();
    if (gVideoModeIndex != 0 && gVideoModeIndex != 4) {
        if (osTvType == OS_TV_PAL) {
            gVideoModeIndex = 14;
        } else if (osTvType == OS_TV_MPAL) {
            gVideoModeIndex = 13;
        } else {
            gVideoModeIndex = 12;
        }
        fb_init_vi();
    }
    osViBlack(FALSE);
}

extern s32 D_800FECA0; // JFG: the size of the depth buffer allocation
extern s32 gVideoDepthBuffer; // JFG: the depth buffer, 64-byte aligned

/* JFG: allocate a depth buffer for the given size (0x80054C4C). */
void zbuffer_alloc(s32 width, s32 height) {
    D_800FECA0 = width * height * 2 + 0x30;
    D_800A3900[3] = mempool_alloc_safe(D_800FECA0, -1);
    gVideoDepthBuffer = ((s32) D_800A3900[3] + 0x3F) & ~0x3F;
}

/* JFG: free the depth buffer (the size arguments are unused) (0x80054CA4). */
void zbuffer_free(UNUSED s32 width, UNUSED s32 height) {
    if (D_800A3900[3] != NULL) {
        mempool_free(D_800A3900[3]);
        D_800A3900[3] = NULL;
        gVideoDepthBuffer = 0;
        D_800FECA0 = 0;
    }
}

typedef struct VideoFlags {
    u32 unk0 : 1;
    u32 unk1 : 1; // JFG: apply the user's vertical adjustment in the odd modes
} VideoFlags;
extern VideoFlags D_800FF384;
extern OSViMode gTvViMode;
extern s32 D_800A3924; // JFG: set once the first mode was built (the first is not applied here)
OSViMode *func_800552DC(s32 mode);
void fb_memcpy(u8 *src, u8 *dst, s32 size);

/**
 * Initialise the VI settings.
 * JFG: builds the mode from a table entry (width, scales, lines shown and vertical offset)
 * on a copy of a libultra mode, instead of DKR's switch.
 * Official Name: viSetTiming
 */
void fb_init_vi(void) {
    OSViMode *tvViMode;
    s32 offset;
    s32 i;
    VideoMode *mode;

    mode = &D_800A392C[gVideoModeIndex];
    tvViMode = func_800552DC(mode->viMode);
    fb_memcpy((u8 *) tvViMode, (u8 *) &gTvViMode, sizeof(OSViMode));
    gTvViMode.comRegs.width = mode->width;
    gTvViMode.comRegs.xScale = (mode->width << 9) / mode->unk8;
    offset = mode->unk10;
    if ((gVideoModeIndex & 1) && D_800FF384.unk1) {
        offset += gVideoVerticalAdjust;
    }
    for (i = 0; i < 2; i++) {
        gTvViMode.fldRegs[i].origin = mode->width * 2;
        gTvViMode.fldRegs[i].yScale = (mode->height << 10) / mode->unkC;
        gTvViMode.fldRegs[i].vStart += offset << 17;
        gTvViMode.fldRegs[i].vStart -= ((240 - offset) - mode->unkC) * 2;
    }
    if (D_800A3924) {
        osViSetMode(&gTvViMode);
    }
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON | OS_VI_DIVOT_ON | OS_VI_GAMMA_OFF);
    D_800A3924 = TRUE;
}

/**
 * Return the current framebuffer dimensions.
 * JFG: writes the width and height (DKR packs them into one s32).
 * Official Name: viGetCurrentSize
 */
void fb_size(s32 *width, s32 *height) {
    *width = D_800A392C[gVideoModeIndex].width;
    *height = D_800A392C[gVideoModeIndex].height;
}

extern f32 D_800A391C;
extern f32 D_800A3920;

/* JFG: scale a position by the screen's scale factors (0x80054EF0). */
void fb_scale_coords(s32 *x, s32 *y) {
    *x = *x * D_800A391C;
    *y = *y * D_800A3920;
}

/* Writes the screen's scale factors (0x80054F74). */
void fb_get_scale(f32 *x, f32 *y) {
    *x = D_800A391C;
    *y = D_800A3920;
}

/**
 * Sets the video counters to their default values.
 * Official Name: viFrameRateReset
 */
void video_delta_reset(void) {
    D_800FECAD = 1;
    gVideoDeltaCounter = 0;
    gVideoDeltaTime = LOGIC_30FPS;
    D_800FECCC = 0;
}

#define MESG_SKIP_BUFFER_SWAP 8
#define LOGIC_60FPS 1
#define LOGIC_20FPS 3

extern s32 gVideoLastFramebuffer; // the framebuffer to show next
void fb_swap(void);

/**
 * Wait for the finished message from the scheduler while counting up a timer,
 * then show the next framebuffer.
 * JFG: drops to 20 FPS (3) after 30 slow frames and back up when it can, unless
 * the delta is locked; a flag forces 60 FPS.
 * Official Name: viFrameSync
 */
s32 fb_update(s32 mesg) {
    u8 tempUpdateRate;

    tempUpdateRate = LOGIC_60FPS;
    if (sBlackScreenTimer) {
        sBlackScreenTimer--;
        if (sBlackScreenTimer == 0) {
            osViBlack(FALSE);
        }
    }
    if (mesg != MESG_SKIP_BUFFER_SWAP) {
        fb_swap();
    }
    while (osRecvMesg(&gVideoMesgQueue, NULL, OS_MESG_NOBLOCK) != -1) {
        tempUpdateRate++;
    }
    if (gVideoDeltaTime == LOGIC_20FPS) {
        if (tempUpdateRate < LOGIC_20FPS) {
            if (++gVideoDeltaCounter >= 31) {
                gVideoDeltaTime = LOGIC_30FPS;
            }
        } else {
            gVideoDeltaCounter = 0;
        }
    } else if (tempUpdateRate >= LOGIC_20FPS && !D_800FECAD) {
        gVideoDeltaTime = LOGIC_20FPS;
        gVideoDeltaCounter = 0;
    }
    if (D_800FECCC) {
        gVideoDeltaTime = LOGIC_60FPS;
    }
    while (tempUpdateRate < gVideoDeltaTime) {
        osRecvMesg(&gVideoMesgQueue, NULL, OS_MESG_BLOCK);
        tempUpdateRate++;
    }
    osViSwapBuffer((void *) gVideoLastFramebuffer);
    osRecvMesg(&gVideoMesgQueue, NULL, OS_MESG_BLOCK);
    D_800FECAD = 0;
    return tempUpdateRate;
}

/* Returns the current video mode index (0x80055180). */
s32 fb_mode(void) {
    return gVideoModeIndex & 3;
}

extern s8 gVideoVerticalAdjust;

/* Returns the screen's vertical adjustment (0x80055198). */
s8 fb_vertical_adjust_get(void) {
    return gVideoVerticalAdjust;
}

/* Sets the screen's vertical adjustment (-30 to 30) and applies it (0x800551A8). */
void fb_vertical_adjust_set(s32 adjust) {
    if (adjust < -30) {
        adjust = -30;
    }
    if (adjust > 30) {
        adjust = 30;
    }
    gVideoVerticalAdjust = adjust;
    fb_init_vi();
}

extern s8 D_800FECA6;

/* JFG: set a one-bit video flag. */
void func_800551E8(s32 arg0) {
    D_800FECA6 = arg0 & 1;
}

extern s8 D_800FECA5;

s8 func_800551F8(void) {
    return D_800FECA5;
}

s32 func_80055208(void) {
    return D_800FECA5 != D_800FECA6;
}

extern u8 D_800FECA7;

void func_80055228(void) {
    D_800FECA7 = 0;
}

extern s32 gVideoLastFramebuffer;
extern s32 D_800A3910[3]; // JFG: the framebuffers

s32 func_80055234(void) {
    if (D_800A3910[0] == gVideoLastFramebuffer) {
        return 1;
    }
    return 0;
}

extern u8 D_800A32A0;

/**
 * JFG: fill a buffer with zeros, or with -1 when D_800A32A0 is clear, and write the cache back.
 * (The empty `if (size) {}` gives the ROM's registers; found by the permuter.)
 */
void buffer_clear_words(void *buffer, s32 size) {
    s32 *ptr = buffer;
    s32 count;

    if (D_800A32A0) {
        count = size >> 2;
        while (count--) {
            *ptr = 0;
            ptr++;
        }
    } else {
        count = size >> 2;
        if (size) {}
        while (count--) {
            *ptr = -1;
            ptr++;
        }
    }
    osWritebackDCacheAll();
}

/* libultra's VI mode table entries (vimodes*.c), by address */
extern OSViMode D_800A9AB0;
extern OSViMode D_800A9B00;
extern OSViMode D_800A9B50;
extern OSViMode D_800A9BA0;
extern OSViMode D_800A9BF0;
extern OSViMode D_800A9C40;
extern OSViMode D_800A9C90;
extern OSViMode D_800A9CE0;
extern OSViMode D_800A9D30;
extern OSViMode D_800A9D80;
extern OSViMode D_800A9DD0;
extern OSViMode D_800A9E20;

/* JFG: the VI mode for one of the video mode table's twelve modes (NULL for others). */
OSViMode *func_800552DC(s32 mode) {
    switch (mode) {
        case 0:
            return &D_800A9B50;
        case 1:
            return &D_800A9BA0;
        case 2:
            return &D_800A9BF0;
        case 3:
            return &D_800A9C40;
        case 4:
            return &D_800A9B00;
        case 5:
            return &D_800A9C90;
        case 6:
            return &D_800A9CE0;
        case 7:
            return &D_800A9D30;
        case 8:
            return &D_800A9AB0;
        case 9:
            return &D_800A9D80;
        case 10:
            return &D_800A9DD0;
        case 11:
            return &D_800A9E20;
    }
    return NULL;
}

extern s8 D_800FECA4; // JFG: the framebuffer index
extern s32 D_800FECB4; // JFG: the frame buffer to draw into

/* JFG: show the next framebuffer and pick the ones to draw into (three buffers when D_800FECA5 is set, else two). */
void fb_swap(void) {
    gVideoLastFramebuffer = D_800A3910[D_800FECA4];
    D_800FECA4++;
    if (D_800FECA5) {
        if (D_800FECA4 >= 3) {
            D_800FECA4 = 0;
        }
        D_800FECB4 = D_800A3910[D_800FECA4];
        if (D_800FECA4 < 2) {
            D_800FECB0 = D_800A3910[D_800FECA4 + 1];
        } else {
            D_800FECB0 = D_800A3910[0];
        }
    } else {
        if (D_800FECA4 >= 2) {
            D_800FECA4 = 0;
        }
        D_800FECB4 = D_800FECB0 = D_800A3910[D_800FECA4];
    }
}

/* Copies bytes from one address to another (0x80055480). */
void fb_memcpy(u8 *src, u8 *dst, s32 size) {
    while (size--) {
        *dst++ = *src++;
    }
}
