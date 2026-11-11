/*
 * Screen images (DKR screen_asset.c): split from 55130.c (see 560B0.c).
 */
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


void fb_size(s32 *width, s32 *height);
void rendermode_reset(Gfx **dList);

#define ASSET_SCREENS 0x13
#define ASSET_SCREENS_TABLE 0x14

/**
 * Loads a 320x240 RGBA16 image to be displayed on the screen.
 * Returns the address of the screen asset, or 0x80100000 if no screen assets were found.
 * JFG: the screens are compressed, and are decompressed in place at the end of the buffer.
 * Official Name: screenLoad
 * (JFG: one word, header, holds both the header's address and then the compressed data's, which is why it is
 * a u32; the colour tag is 0xFFFF.)
 */
UNUSED u32 screenimage_load(s32 screenIndex) {
    u32 *screenTable;
    s32 start;
    s32 size;
    s32 screenTableCount;
    s32 imageSize;
    u8 *image;
    u32 header;

    screenTable = asset_table_load(ASSET_SCREENS_TABLE);
    screenTableCount = 0;

    while (screenTable[screenTableCount] != 0xFFFFFFFF) {
        screenTableCount++;
    }

    screenTableCount--;

    if (screenTableCount == 0) {
        mempool_free(screenTable);
        return (u32) OS_PHYSICAL_TO_K0(0x100000);
    }
    if (screenIndex < 0 || screenIndex >= screenTableCount) {
        screenIndex = 0;
    }

    start = screenTable[screenIndex];
    size = screenTable[screenIndex + 1] - start;
    image = NULL;
    header = (u32) mempool_alloc_safe(16, 0xFFFF);
    if (header != 0) {
        asset_load(ASSET_SCREENS, header, start, 16);
        imageSize = byteswap32((u8 *) header) + 0x80;
        mempool_free((u8 *) header);
        image = mempool_alloc_safe(imageSize, 0xFFFF);
        if (image != NULL) {
            header = (u32) image + imageSize - size;
            header -= header & 0xF;
            asset_load(ASSET_SCREENS, header, start, size);
            gzip_inflate((u8 *) header, image);
        }
    }
    mempool_free(screenTable);
    return (u32) image;
}

#ifdef NON_MATCHING
/* The texture rectangle of the older GBI (as in printf.c). */
#ifndef G_RDPHALF_CONT
#define G_RDPHALF_CONT 0xB2
#endif
#define gSPTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                          \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12));        \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));            \
        gImmp1(pkt, G_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                          \
        gImmp1(pkt, G_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));                 \
    }
/* DKR include/f3ddkr.h: G_DMADL (7) runs numberOfCommands commands from address. */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(7, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                      \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
extern Gfx D_800A4F20[]; // JFG: render modes for an unscaled screen
extern Gfx D_800A4F48[]; // JFG: render modes for a scaled screen
extern Gfx D_800A4F70[]; // JFG: the texture image command (its address is replaced)
extern Gfx D_800A4F78[]; // JFG: the six commands loading a 320x6 slice

/**
 * Renders a 320x240 RGBA16 screen asset.
 * JFG: centred at its own size on a larger screen, or scaled to fill it when scale is set.
 * Official Name: screenDraw
 * NON_MATCHING: same instructions in outline, but IDO keeps stepY in a sixth saved register where the
 * target has five (and $ra as a temporary); every stack slot and register after that differs.
 */
void screenimage_draw(Gfx **dList, u8 *screenAddress, s32 scale) {
    u32 width;
    u32 height;
    s32 y_pos;
    s32 posY;
    s32 nextY;
    s32 stepY;
    s32 ulx;
    s32 lrx;
    s32 dsdx;
    s32 dtdy;

    screenAddress += 16;
    fb_size((s32 *) &width, (s32 *) &height);
    y_pos = 0;
    if ((width == SCREEN_WIDTH && height == SCREEN_HEIGHT) || !scale) {
        gSPDisplayList((*dList)++, D_800A4F20);
        posY = (height - SCREEN_HEIGHT) << 15;
        ulx = (width - SCREEN_WIDTH) << 1;
        lrx = ulx + (SCREEN_WIDTH << 2);
        dsdx = 1 << 10;
        dtdy = 1 << 10;
        stepY = 6 << 16;
    } else {
        stepY = (height << 16) / (SCREEN_HEIGHT / 6);
        lrx = width << 2;
        posY = 0;
        ulx = 0;
        gSPDisplayList((*dList)++, D_800A4F48);
        dsdx = (SCREEN_WIDTH << 10) / width;
        dtdy = (SCREEN_HEIGHT << 10) / height;
    }
    while (y_pos != SCREEN_HEIGHT) {
        nextY = posY + stepY;
        (*dList)->words.w0 = D_800A4F70[0].words.w0;
        (*dList)->words.w1 = (u32) screenAddress;
        (*dList)++;
        gDkrDmaDisplayList((*dList)++, OS_K0_TO_PHYSICAL(D_800A4F78), 6);
        gSPTextureRectangleOld((*dList)++, ulx, posY >> 14, lrx, nextY >> 14, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);
        posY = nextY;
        screenAddress += SCREEN_WIDTH * 6 * 2;
        y_pos += 6;
    }
    rendermode_reset(dList);
}
#else
INCLUDE_ASM("asm/nonmatchings/58CC0", func_80058204);
#endif
