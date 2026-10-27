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
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ u16 numberOfInstances;
    /* 0x0A */ u8 padA[0x12 - 0x0A];
    /* 0x12 */ u16 numOfTextures;
    /* 0x14 */ u16 frameAdvanceDelay;
    /* 0x16 */ s16 textureSize;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
} TextureHeader;

/* DKR's Sprite (renamed: libultra's sp.h has a Sprite too), JFG layout */
typedef struct SpriteAsset {
    /* 0x00 */ u8 numberOfFrames;
    /* 0x01 */ u8 pad1;
    /* 0x02 */ s16 numberOfTextures;
    /* 0x04 */ s16 numberOfInstances;
    /* 0x06 */ s16 drawFlags;
    /* 0x08 */ u8 unk8[6]; // JFG: copied from the asset
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ TextureHeader **textures;
    /* 0x14 */ s16 *unk14; // JFG: one entry per texture
    /* 0x18 */ Gfx *frames[1]; // Actual size varies.
} SpriteAsset;

/* DKR's SpriteAsset (the sprite as loaded from ROM), JFG layout */
typedef struct SpriteAssetData {
    /* 0x00 */ s16 baseTextureId;
    /* 0x02 */ s16 numberOfFrames;
    /* 0x04 */ u8 pad4[4];
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
extern s8 D_800FECA8;
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
extern f32 D_800AD150; // .rodata constants, not split out yet
extern f32 D_800AD154;
extern f32 D_800AD158;
extern OSMesgQueue gVideoMesgQueue;
extern OSMesg gVideoMesgBuf[8];
extern OSScClient gVideoSched;
extern void *D_800A3900[]; // JFG: video buffers ([0] the line buffer, [3] the depth buffer allocation)
extern s8 D_800FECAA;
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
        D_800FECC4 = D_800AD150;
        D_800FECC8 = D_800AD154;
        height = 264;
    } else {
        gVideoRefreshRate = 60;
        D_800FECC4 = D_800AD158;
        D_800FECC8 = 1.0f;
        height = 240;
    }
    osCreateMesgQueue(&gVideoMesgQueue, gVideoMesgBuf, ARRAY_COUNT(gVideoMesgBuf));
    osScAddClient(sc, &gVideoSched, &gVideoMesgQueue, OS_SC_ID_VIDEO);
    D_800A3900[0] = mempool_alloc_safe(height * 1280 + 0x30, -1);
    D_800FECAA = 0;
    D_800FECA5 = 0;
    D_800FECA6 = 0;
    D_800FECA7 = 1;
    D_800FECD0 = 0;
    func_8005483C(0);
    D_800A3928 = 1;
    sBlackScreenTimer = 1;
}

INCLUDE_ASM("asm/nonmatchings/55130", func_8005483C);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054B44);

extern s32 D_800FECA0; // JFG: the size of the depth buffer allocation
extern s32 D_800FECBC; // JFG: the depth buffer, 64-byte aligned

/* JFG: allocate a depth buffer for the given size. */
void func_80054C4C(s32 width, s32 height) {
    D_800FECA0 = width * height * 2 + 0x30;
    D_800A3900[3] = mempool_alloc_safe(D_800FECA0, -1);
    D_800FECBC = ((s32) D_800A3900[3] + 0x3F) & ~0x3F;
}

/* JFG: free the depth buffer (the size arguments are unused). */
void func_80054CA4(UNUSED s32 width, UNUSED s32 height) {
    if (D_800A3900[3] != NULL) {
        mempool_free(D_800A3900[3]);
        D_800A3900[3] = NULL;
        D_800FECBC = 0;
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
void func_80055480(u8 *src, u8 *dst, s32 size);

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

    mode = &D_800A392C[D_800FECA8];
    tvViMode = func_800552DC(mode->viMode);
    func_80055480((u8 *) tvViMode, (u8 *) &gTvViMode, sizeof(OSViMode));
    gTvViMode.comRegs.width = mode->width;
    gTvViMode.comRegs.xScale = (mode->width << 9) / mode->unk8;
    offset = mode->unk10;
    if ((D_800FECA8 & 1) && D_800FF384.unk1) {
        offset += D_800FECAA;
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
    *width = D_800A392C[D_800FECA8].width;
    *height = D_800A392C[D_800FECA8].height;
}

extern f32 D_800A391C;
extern f32 D_800A3920;

/* JFG: scale a position by the screen's scale factors. */
void func_80054EF0(s32 *x, s32 *y) {
    *x = *x * D_800A391C;
    *y = *y * D_800A3920;
}

void func_80054F74(f32 *x, f32 *y) {
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

extern s32 D_800FECB8; // the framebuffer to show next
void func_8005539C(void);

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
        func_8005539C();
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
    osViSwapBuffer((void *) D_800FECB8);
    osRecvMesg(&gVideoMesgQueue, NULL, OS_MESG_BLOCK);
    D_800FECAD = 0;
    return tempUpdateRate;
}

s32 func_80055180(void) {
    return D_800FECA8 & 3;
}

extern s8 D_800FECAA;

s8 func_80055198(void) {
    return D_800FECAA;
}

void func_800551A8(s32 adjust) {
    if (adjust < -30) {
        adjust = -30;
    }
    if (adjust > 30) {
        adjust = 30;
    }
    D_800FECAA = adjust;
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

extern s32 D_800FECB8;
extern s32 D_800A3910;

s32 func_80055234(void) {
    if (D_800A3910 == D_800FECB8) {
        return 1;
    }
    return 0;
}

extern u8 D_800A32A0;

#ifdef NON_MATCHING
/**
 * JFG: fill a buffer with zeros, or with -1 when D_800A32A0 is clear, and write the cache back.
 * NON_MATCHING: register numbering only (target keeps the pointer in a2, the counter in v1).
 */
void func_80055260(void *buffer, s32 size) {
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
        while (count--) {
            *ptr = -1;
            ptr++;
        }
    }
    osWritebackDCacheAll();
}
#else
INCLUDE_ASM("asm/nonmatchings/55130", func_80055260);
#endif

INCLUDE_ASM("asm/nonmatchings/55130", func_800552DC);

INCLUDE_ASM("asm/nonmatchings/55130", func_8005539C);

void func_80055480(u8 *src, u8 *dst, s32 size) {
    while (size--) {
        *dst++ = *src++;
    }
}

INCLUDE_ASM("asm/nonmatchings/55130", func_800554B0);

/**
 * Official Name: texDisableModes
 * Add flags to the block list so they are removed when drawn.
 */
void tex_disable_modes(s32 flags) {
    gBlockedRenderFlags |= flags;
}

/**
 * Official Name: texEnableModes
 * Remove flags to the block list so they are no longer removed when drawn.
 */
void tex_enable_modes(s32 flags) {
    gBlockedRenderFlags &= ~flags;
}

extern u8 D_800A4E6C;

void func_80055684(u8 arg0) {
    D_800A4E6C = arg0;
}

#ifdef NON_MATCHING
/* The texture asset's first 0x28 bytes (DKR: TempTexHeader) */
typedef struct TempTexHeader {
    /* 0x00 */ u8 pad0[0x19];
    /* 0x19 */ u8 isCompressed;
    /* 0x1A */ u8 pad1A[6];
    /* 0x20 */ u8 uncompressedSize[4];
    /* 0x24 */ u8 pad24[4];
} TempTexHeader;

extern s32 gTextureTableSize[2];
extern s32 *gTextureAssetTable[2];
extern TempTexHeader *gTempTextureHeader;
extern s32 D_800A33F4; // JFG: the texture being loaded (-1 when none)
extern u8 *D_800A4E7C; // JFG: load the next texture at this address
extern s32 gCiPalettesSize;
void *mempool_alloc(s32 size, u32 colourTag);
void *mempool_alloc_fixed(s32 size, u8 *address, u32 colourTag);
void func_800570D8(TextureHeader *tex, Gfx *dList);

/**
 * Loads a texture into memory and initializes display lists for texture loading.
 * First checks if the texture is already in the cache.
 * JFG: can load at a given address; room for the larger of the compressed and
 * uncompressed sizes; one call sets up the display lists; no palettes here.
 * Official Name: texLoadTexture
 * NON_MATCHING: identical but for one stack slot: compressedStart is spilled at sp+0x20
 * (target 0x24); the declaration orders tried keep it there or grow the frame.
 */
TextureHeader *load_texture(s32 id) {
    s32 i;
    TextureHeader *tex;
    UNUSED TextureHeader *texTemp;
    s32 assetIndex;
    s32 assetOffset;
    s32 assetSize;
    u32 compressedStart;
    s32 assetSection;
    s32 slotIndex;
    s32 tableType;
    s32 uncompressedSize;
    s32 size;

    // 'id' is a 16-bit value; the highest bit determines whether it's a 2D or 3D texture
    id &= 0xFFFF;
    assetIndex = id;
    assetSection = 2;
    tableType = 0;
    if (id & 0x8000) {
        tableType = 1;
        assetIndex = id & 0x7FFF;
        assetSection = 0;
    }
    if (assetIndex >= gTextureTableSize[tableType] || assetIndex < 0) {
        id = 0;
    }
    for (i = 0; i < gNumberOfLoadedTextures; i++) {
        if (id == gTextureCache[ASSETCACHE_ID(i)]) {
            tex = (TextureHeader *) gTextureCache[ASSETCACHE_PTR(i)];
            tex->numberOfInstances++;
            return tex;
        }
    }
    assetOffset = gTextureAssetTable[tableType][assetIndex];
    assetSize = gTextureAssetTable[tableType][assetIndex + 1] - assetOffset;
    D_800A33F4 = id;
    asset_load(assetSection, (u32) gTempTextureHeader, assetOffset, sizeof(TempTexHeader));
    if (!gTempTextureHeader->isCompressed) {
        if (D_800A4E7C) {
            tex = (TextureHeader *) mempool_alloc_fixed(assetSize + 0x90, D_800A4E7C, gTexColourTag);
        } else {
            tex = (TextureHeader *) mempool_alloc(assetSize + 0x90, gTexColourTag);
        }
        D_800A33F4 = -1;
        if (tex == NULL) {
            return NULL;
        }
        asset_load(assetSection, (u32) tex, assetOffset, assetSize);
    } else {
        uncompressedSize = byteswap32(gTempTextureHeader->uncompressedSize);
        if (uncompressedSize < assetSize) {
            size = assetSize;
        } else {
            size = uncompressedSize;
        }
        size += 0x20;
        if (D_800A4E7C) {
            tex = (TextureHeader *) mempool_alloc_fixed(size + 0x90, D_800A4E7C, gTexColourTag);
        } else {
            tex = (TextureHeader *) mempool_alloc(size + 0x90, gTexColourTag);
        }
        D_800A33F4 = -1;
        if (tex == NULL) {
            return NULL;
        }
        // Load the compressed texture at the end of the allocated block,
        // so it can be decompressed into the entire block
        compressedStart = ((s32) tex + size) - assetSize;
        compressedStart = (s32) compressedStart - (s32) compressedStart % 16;
        asset_load(assetSection, compressedStart, assetOffset, assetSize);
        gzip_inflate((u8 *) (compressedStart + 0x20), (u8 *) tex);
        assetSize = uncompressedSize;
    }
    // Find an available slot in the texture cache and store the newly loaded texture
    slotIndex = -1;
    for (i = 0; i < gNumberOfLoadedTextures; i++) {
        if (gTextureCache[ASSETCACHE_ID(i)] == -1) {
            slotIndex = i;
        }
    }
    if (slotIndex == -1) {
        slotIndex = gNumberOfLoadedTextures;
        gNumberOfLoadedTextures++;
    }
    gTextureCache[ASSETCACHE_ID(slotIndex)] = id;
    gTextureCache[ASSETCACHE_PTR(slotIndex)] = (s32) tex;
    func_800570D8(tex, (Gfx *) align16((u8 *) ((s32) tex + assetSize)));
    if (gCiPalettesSize >= 0x280) {
        return NULL;
    }
    if (gNumberOfLoadedTextures > 700) {
        return NULL;
    }
    return tex;
}
#else
INCLUDE_ASM("asm/nonmatchings/55130", func_80055694);
#endif

/**
 * This function attempts to free the texture from memory.
 * It checks if the refcount is zero, then finds the cache entry, before clearing it.
 * Official Name: texFreeTexture
 */
void tex_free(TextureHeader *tex) {
    s32 i;

    if (tex != NULL) {
        if ((--tex->numberOfInstances) <= 0) {
            for (i = 0; i < gNumberOfLoadedTextures; i++) {
                if ((s32) tex == gTextureCache[ASSETCACHE_PTR(i)]) {
                    mempool_free(tex);

                    gTextureCache[ASSETCACHE_ID(i)] = -1;
                    gTextureCache[ASSETCACHE_PTR(i)] = -1;
                    return;
                }
            }
        }
    }
}

s32 func_80055AB8(s32 texture) {
    s32 i;
    for (i = 0; i < gNumberOfLoadedTextures; i++) {
        if (texture == gTextureCache[ASSETCACHE_PTR(i)]) {
            return gTextureCache[ASSETCACHE_ID(i)];
        }
    }
    return -1;
}

extern u8 *D_800A4E7C; // JFG: load the next texture at this address
TextureHeader *load_texture(s32 assetIndex);

TextureHeader *func_80055B0C(s32 textureID, s32 address) {
    TextureHeader *tex;
    D_800A4E7C = (u8 *) address;
    tex = load_texture(textureID);
    D_800A4E7C = NULL;
    return tex;
}

/**
 * Set the colour tag that determines which memory pool textures will be loaded into.
 * By default, this generally stays as COLOUR_TAG_MAGENTA
 * Official Name: setTexMemColour
 */
void set_texture_colour_tag(s32 tagID) {
    gTexColourTag = tagID;
}

/**
 * Shift the texture header by the offset and return the texture data after it.
 * JFG: returns the data, and an offset past the last frame keeps the first.
 * Official Name: texFrame
 */
TextureHeader *set_animated_texture_header(TextureHeader *texHead, s32 offset) {
    TextureHeader *frame = texHead + 1;
    if (offset > 0) {
        if (offset < texHead->numOfTextures << 8) {
            frame = (TextureHeader *) (((u8 *) texHead) + ((offset >> 16) * texHead->textureSize)) + 1;
        }
    }
    return frame;
}

/**
 * Resets all render settings to the default state.
 * The next draw call will be forced to apply all settings instead of skipping unecessary steps.
 * JFG: only touches the display list when given one, and turns fog on.
 * Official Name: texDPInit
 */
void rendermode_reset(Gfx **dList) {
    gBlockedRenderFlags = RENDER_NONE;
    D_800FED34 = 0;
    D_800FED38 = 0;
    D_800FED30 = 0;
    D_800FED3C = 0;
    D_800FED40 = TRUE;
    D_800FED44 = TRUE;
    if (dList != NULL) {
        gDPPipeSync((*dList)++);
        gSPSetGeometryMode((*dList)++, G_SHADING_SMOOTH | G_SHADE | G_ZBUFFER | G_FOG);
    }
}

INCLUDE_ASM("asm/nonmatchings/55130", func_80055C00);

extern s32 D_800A4E64;

void func_80056000(s32 arg0) {
    D_800A4E64 = arg0;
}

extern u8 D_800FED48;
extern u8 D_800FED49;
extern u8 D_800FED4A;
extern u8 D_800FED4B;
extern u8 D_800FED4C;
extern u8 D_800FED4D;
extern s32 D_800A4E74;

void func_8005600C(u8 a, u8 b, u8 c, u8 d, u8 e, u8 f) {
    D_800FED48 = a;
    D_800FED49 = b;
    D_800FED4A = c;
    D_800FED4B = d;
    D_800FED4C = e;
    D_800FED4D = f;
    D_800A4E74 = 1;
}

extern s32 D_800A4E74;

void func_80056064(void) {
    D_800A4E74 = 0;
}

INCLUDE_ASM("asm/nonmatchings/55130", func_80056070);

/* Element sizes of the sprite's buffers */
typedef struct SpriteTriangle {
    u8 pad[16];
} SpriteTriangle;
typedef struct SpriteVertex {
    s16 x, y, z;
    u8 r, g, b, a;
} SpriteVertex;

extern s32 gSpriteTableSize;
extern s32 *gSpriteOffsetTable;
extern SpriteAssetData *gCurrentSpriteAsset;
extern s32 D_800FED1C; // DKR: D_8012635C
extern SpriteVertex *gSpriteVertices;
extern Gfx *gSpriteDLists;
extern SpriteTriangle *gSpriteTriangles;
extern s32 gFirstTexIsLoaded;
TextureHeader *load_texture(s32 assetIndex);
void sprite_init_frame(SpriteAssetData *asset, SpriteAsset *sprite, s32 frame);

#define ASSET_SPRITES 0x15
#define MAX_NUM_SPRITES 100

/**
 * Loads a sprite from its asset, allocates memory for vertices, triangles, and display lists,
 * and initializes the display list for each texture and frame.
 * If the sprite is already cached, returns it from the cache.
 * JFG: one allocation sized for the larger of the texture and frame counts, with the
 * textures before the vertices and a table of shorts at the end; a draw flag (0x40)
 * stops after the first frame.
 * Official name: texLoadSprite
 */
SpriteAsset *tex_load_sprite(s32 spriteID, s32 arg1) {
    SpriteAsset *refSprite;
    SpriteAsset *sprite;
    s32 cacheNum;
    s32 headerSize;
    s32 dlistsOffset;
    s32 verticesOffset;
    s32 texturesOffset;
    s32 unk14Offset;
    SpriteAssetData *spriteAsset;
    TextureHeader *tex;
    s32 i;
    s32 size;
    s8 allocFailed;
    s8 cacheFull;
    s32 numTextures;

    D_800FED1C = arg1;
    if (spriteID < 0 || spriteID >= gSpriteTableSize) {
        return NULL;
    }

    for (i = 0, cacheFull = 0; i < gSpriteCacheCount; i++) {
        if (spriteID == gSpriteCache[ASSETCACHE_ID(i)]) {
            refSprite = (SpriteAsset *) gSpriteCache[ASSETCACHE_PTR(i)];
            refSprite->numberOfInstances++;
            return refSprite;
        }
    }

    cacheNum = -1;
    for (i = 0; i < gSpriteCacheCount; i++) {
        if (gSpriteCache[ASSETCACHE_ID(i)] == -1) {
            cacheNum = i;
        }
    }

    if (cacheNum == -1) {
        cacheFull = TRUE;
        cacheNum = gSpriteCacheCount;
        gSpriteCacheCount++;
    }

    size = gSpriteOffsetTable[spriteID];
    spriteAsset = gCurrentSpriteAsset;
    asset_load(ASSET_SPRITES, (u32) spriteAsset, size, gSpriteOffsetTable[spriteID + 1] - size);

    numTextures = spriteAsset->frameTexOffsets[spriteAsset->numberOfFrames];
    // i is the larger of the texture and frame counts here
    i = numTextures;
    if (numTextures < spriteAsset->numberOfFrames) {
        i = spriteAsset->numberOfFrames;
    }
    headerSize = (s32) align16((u8 *) (spriteAsset->numberOfFrames * 4 + 0x18));
    dlistsOffset = headerSize + i * 2 * sizeof(SpriteTriangle);
    texturesOffset = dlistsOffset + i * 4 * sizeof(Gfx) + spriteAsset->numberOfFrames * sizeof(Gfx);
    verticesOffset = texturesOffset + i * sizeof(TextureHeader *);
    unk14Offset = verticesOffset + i * 4 * sizeof(SpriteVertex);
    sprite = (SpriteAsset *) mempool_alloc((s32) align16((u8 *) (i * sizeof(s16) + unk14Offset)), COLOUR_TAG_MAGENTA);
    if (sprite == NULL) {
        if (cacheFull) {
            gSpriteCacheCount--;
        }
        return NULL;
    }

    gSpriteTriangles = (SpriteTriangle *) ((u8 *) sprite + headerSize);
    gSpriteDLists = (Gfx *) ((u8 *) sprite + dlistsOffset);
    gSpriteVertices = (SpriteVertex *) ((u8 *) sprite + verticesOffset);
    sprite->textures = (TextureHeader **) ((u8 *) sprite + texturesOffset);
    sprite->unk14 = (s16 *) ((u8 *) sprite + unk14Offset);

    allocFailed = FALSE;
    for (i = 0; i < numTextures; i++) {
        gTexColourTag = 0xFF0163;
        tex = load_texture(spriteAsset->baseTextureId + i);
        sprite->textures[i] = tex;
        if (sprite->textures[i] == NULL) {
            allocFailed = TRUE;
        }
        gTexColourTag = COLOUR_TAG_MAGENTA;
        gFirstTexIsLoaded = TRUE;
    }

    gFirstTexIsLoaded = FALSE;
    if (allocFailed) {
        for (i = 0; i < numTextures; i++) {
            tex = sprite->textures[i];
            if (tex != NULL) {
                tex_free(tex);
            }
        }
        if (cacheFull) {
            gSpriteCacheCount--;
        }
        mempool_free(sprite);
        return NULL;
    }

    sprite->numberOfTextures = numTextures;
    sprite->unk8[0] = spriteAsset->unk8[0];
    sprite->unk8[1] = spriteAsset->unk8[1];
    sprite->unk8[2] = spriteAsset->unk8[2];
    sprite->unk8[3] = spriteAsset->unk8[3];
    sprite->unk8[4] = spriteAsset->unk8[4];
    sprite->unk8[5] = spriteAsset->unk8[5];
    sprite->numberOfFrames = spriteAsset->numberOfFrames;
    for (i = 0; i < spriteAsset->numberOfFrames; i++) {
        sprite->frames[i] = gSpriteDLists;
        sprite_init_frame(spriteAsset, sprite, i);
        if (sprite->drawFlags & 0x40) {
            i = spriteAsset->numberOfFrames;
        }
    }
    if (spriteAsset->flags & 0x200) {
        sprite->drawFlags |= 0x200;
    }

    if (gSpriteCacheCount >= MAX_NUM_SPRITES) {
        return NULL;
    }

    gSpriteCache[ASSETCACHE_ID(cacheNum)] = spriteID;
    gSpriteCache[ASSETCACHE_PTR(cacheNum)] = (s32) sprite;
    sprite->numberOfInstances = 1;
    return sprite;
}

/**
 * This function attempts to free the sprite from memory.
 * It checks if the refcount is zero, then finds the cache entry, before clearing it.
 * Official Name: texFreeSprite
 */
void sprite_free(SpriteAsset *sprite) {
    s32 i;
    s32 frame;

    if (sprite != NULL) {
        sprite->numberOfInstances--;
        if (sprite->numberOfInstances <= 0) {
            for (i = 0; i < gSpriteCacheCount; i++) {
                if (sprite == (SpriteAsset *) gSpriteCache[ASSETCACHE_PTR(i)]) {
                    for (frame = 0; frame < sprite->numberOfTextures; frame++) {
                        tex_free(sprite->textures[frame]);
                    }
                    mempool_free(sprite);
                    gSpriteCache[ASSETCACHE_ID(i)] = -1;
                    gSpriteCache[ASSETCACHE_PTR(i)] = -1;
                    break;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/55130", func_80056D24);

INCLUDE_ASM("asm/nonmatchings/55130", func_800570D8);

INCLUDE_ASM("asm/nonmatchings/55130", func_8005719C);

INCLUDE_ASM("asm/nonmatchings/55130", func_800577A8);

s32 func_800577A8(SpriteAsset *sprite, s32 *flags, s32 updateRate, f32 *frame, s32 arg4);

/* JFG: func_800577A8 for a single texture (as a sprite of its frames). */
s32 func_80057A10(TextureHeader *tex, s32 *flags, s32 updateRate, f32 *frame, s32 arg4) {
    SpriteAsset sprite;
    TextureHeader *textures[1];

    textures[0] = tex;
    sprite.textures = textures;
    sprite.numberOfFrames = tex->numOfTextures >> 8;
    return func_800577A8(&sprite, flags, updateRate, frame, arg4);
}

/* JFG's texture animation flag bits (DKR: bits 23, 25 and 26) */
#define RENDER_ANIM_PINGPONG_ONCE 0x20000
#define RENDER_ANIM_PINGPONG 0x40000
#define RENDER_ANIM_REVERSE 0x80000

extern u8 D_800A4E78; // JFG: once-only animation mode for the next call (0: random start, 2: hold, 3: stop at the end)
s32 rand_range(s32 min, s32 max);

/**
 * Official Name: texAnimateTexture
 * JFG: other flag bits, works on local copies, and a once-only mode byte.
 */
void tex_animate_texture(TextureHeader *texture, u32 *triangleBatchInfoFlags, s32 *arg2, s32 updateRate) {
    UNUSED s32 bit23Set;
    UNUSED s32 bit25Set;
    UNUSED s32 bit26Set;
    s32 frame;
    u32 flags;
    s32 breakVar;
    u8 mode;

    flags = *triangleBatchInfoFlags;
    frame = *arg2;
    if (flags & RENDER_ANIM_PINGPONG_ONCE) {
        mode = D_800A4E78;
        if (!(flags & RENDER_ANIM_PINGPONG)) {
            if (mode == 0) {
                if (rand_range(0, 1000) > 985) {
                    flags &= ~RENDER_ANIM_REVERSE;
                    flags |= RENDER_ANIM_PINGPONG;
                }
            } else if (mode != 2) {
                flags &= ~RENDER_ANIM_REVERSE;
                flags |= RENDER_ANIM_PINGPONG;
            }
        } else if (!(flags & RENDER_ANIM_REVERSE)) {
            frame += texture->frameAdvanceDelay * updateRate;
            if (frame >= texture->numOfTextures) {
                if (mode == 3) {
                    frame = texture->numOfTextures - 1;
                } else {
                    frame = ((texture->numOfTextures * 2) - frame) - 1;
                    if (frame < 0) {
                        frame = 0;
                        flags &= ~(RENDER_ANIM_PINGPONG | RENDER_ANIM_REVERSE);
                    } else {
                        flags |= RENDER_ANIM_REVERSE;
                    }
                }
            }
        } else {
            frame -= texture->frameAdvanceDelay * updateRate;
            if (frame < 0) {
                frame = 0;
                flags &= ~(RENDER_ANIM_PINGPONG | RENDER_ANIM_REVERSE);
            }
        }
        D_800A4E78 = 0;
    } else if (flags & RENDER_ANIM_PINGPONG) {
        if (!(flags & RENDER_ANIM_REVERSE)) {
            frame += texture->frameAdvanceDelay * updateRate;
        } else {
            frame -= texture->frameAdvanceDelay * updateRate;
        }
        do {
            breakVar = FALSE;
            if (frame < 0) {
                frame = -frame;
                flags &= ~RENDER_ANIM_REVERSE;
                breakVar = TRUE;
            }
            if (frame >= texture->numOfTextures) {
                frame = ((texture->numOfTextures * 2) - frame) - 1;
                flags |= RENDER_ANIM_REVERSE;
                breakVar = TRUE;
            }
        } while (breakVar);
    } else if (!(flags & RENDER_ANIM_REVERSE)) {
        frame += texture->frameAdvanceDelay * updateRate;
        while (frame >= texture->numOfTextures) {
            frame -= texture->numOfTextures;
        }
    } else {
        frame -= texture->frameAdvanceDelay * updateRate;
        while (frame < 0) {
            frame += texture->numOfTextures;
        }
    }
    *arg2 = frame;
    *triangleBatchInfoFlags = flags;
}

typedef struct ColourCycle {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4];
    /* 0x0C */ struct ColourCycle *source;
} ColourCycle;
s32 *get_misc_asset(s32 index);
void func_80057D1C(ColourCycle *cycle);

void func_80057CEC(ColourCycle *cycle, s32 assetID) {
    cycle->source = (ColourCycle *) get_misc_asset(assetID);
    func_80057D1C(cycle);
}

void func_80057D1C(ColourCycle *cycle) {
    ColourCycle *source = cycle->source;
    cycle->unk0 = 0;
    cycle->unk4 = 0;
    cycle->colour[0] = source->colour[0];
    cycle->colour[1] = source->colour[1];
    cycle->colour[2] = source->colour[2];
    cycle->colour[3] = source->colour[3];
}

/* JFG: what a cycle's source points at (DKR: LevelHeader_70) */
typedef struct ColourCycleEntry {
    /* 0x00 */ u8 colour[4];
    /* 0x04 */ s32 time;
} ColourCycleEntry;

typedef struct ColourCycleData {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 totalTime;
    /* 0x08 */ ColourCycleEntry entries[1]; // count entries
} ColourCycleData;

/**
 * Advance the colour cycle and blend between its current and next colours.
 * Official name: updateColourCycle
 */
void update_colour_cycle(ColourCycle *arg0, s32 updateRate) {
    s32 temp;
    s32 curIndex;
    s32 nextIndex;
    u32 next_red;
    u32 cur_red;
    u32 next_green;
    u32 next_blue;
    u32 next_alpha;
    u32 cur_green;
    u32 cur_blue;
    u32 cur_alpha;
    ColourCycleEntry *cur;
    ColourCycleEntry *next;
    ColourCycleData *data = (ColourCycleData *) arg0->source;

    if (data->count >= 2) {
        arg0->unk4 += updateRate;
        while (arg0->unk4 >= data->totalTime) {
            if (!temp) {} // Fakematch (DKR)
            arg0->unk4 -= data->totalTime;
        }
        while (arg0->unk4 >= data->entries[arg0->unk0].time) {
            arg0->unk4 -= data->entries[arg0->unk0].time;
            arg0->unk0++;
            if (arg0->unk0 >= data->count) {
                arg0->unk0 = 0;
            }
        }

        curIndex = arg0->unk0;
        nextIndex = curIndex + 1;
        if (nextIndex >= data->count) {
            nextIndex = 0;
        }

        cur = &data->entries[curIndex];
        temp = (arg0->unk4 << 16) / cur->time;
        cur_red = cur->colour[0];
        cur_green = cur->colour[1];
        cur_blue = cur->colour[2];
        cur_alpha = cur->colour[3];

        next = &data->entries[nextIndex];
        next_red = next->colour[0];
        next_green = next->colour[1];
        next_blue = next->colour[2];
        next_alpha = next->colour[3];

        next = (ColourCycleEntry *) arg0; // (DKR) needed to match
        arg0->colour[0] = (((next_red - cur_red) * temp) >> 16) + cur_red;
        arg0->colour[1] = (((next_green - cur_green) * temp) >> 16) + cur_green;
        arg0->colour[2] = (((next_blue - cur_blue) * temp) >> 16) + cur_blue;
        arg0->colour[3] = (((next_alpha - cur_alpha) * temp) >> 16) + cur_alpha;
    }
}

/**
 * Official name: resetMixCycle
 */
void init_pulsating_light_data(PulsatingLightData *data) {
    s32 i;
    data->currentFrame = 0;
    data->time = 0;
    data->totalTime = 0;
    data->outColorValue = data->frames[0].value;
    for (i = 0; i < data->numberFrames; i++) {
        data->totalTime += data->frames[i].time;
    }
}

/**
 * Official Name: updateMixCycle
 */
void update_pulsating_light_data(PulsatingLightData *data, s32 timeDelta) {
    s32 thisFrameIndex, nextFrameIndex;

    if (data->numberFrames > 1) {
        data->time += timeDelta;
        while (data->time >= data->totalTime) {
            data->time -= data->totalTime;
        }
        while (data->time >= data->frames[data->currentFrame].time) {
            data->time -= data->frames[data->currentFrame].time;
            data->currentFrame++;
            if (data->currentFrame >= data->numberFrames) {
                data->currentFrame = 0;
            }
        }
        thisFrameIndex = data->currentFrame;
        nextFrameIndex = thisFrameIndex + 1;
        if (nextFrameIndex >= data->numberFrames) {
            nextFrameIndex = 0;
        }

        data->outColorValue = data->frames[thisFrameIndex].value +
                              ((data->frames[nextFrameIndex].value * data->time) / data->frames[thisFrameIndex].time);
    }
}

#define ASSET_SCREENS 0x13
#define ASSET_SCREENS_TABLE 0x14

#ifdef NON_MATCHING
/**
 * Loads a 320x240 RGBA16 image to be displayed on the screen.
 * Returns the address of the screen asset, or 0x80100000 if no screen assets were found.
 * JFG: the screens are compressed, and are decompressed in place at the end of the buffer.
 * Official Name: screenLoad
 * NON_MATCHING: the code is identical, but the frame is 0x40 (target 0x38): IDO gives
 * these locals nine stack slots where the target has eight.
 */
UNUSED u32 screenimage_load(s32 screenIndex) {
    u32 *screenTable;
    s32 start;
    s32 size;
    s32 screenTableCount;
    s32 imageSize;
    u8 *image;
    u8 *header;
    u32 compressed;

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
    header = mempool_alloc_safe(16, COLOUR_TAG_WHITE);
    if (header != NULL) {
        asset_load(ASSET_SCREENS, (u32) header, start, 16);
        imageSize = byteswap32(header) + 0x80;
        mempool_free(header);
        image = mempool_alloc_safe(imageSize, COLOUR_TAG_WHITE);
        if (image != NULL) {
            compressed = (u32) image + imageSize - size;
            compressed -= compressed & 0xF;
            asset_load(ASSET_SCREENS, compressed, start, size);
            gzip_inflate((u8 *) compressed, image);
        }
    }
    mempool_free(screenTable);
    return (u32) image;
}
#else
INCLUDE_ASM("asm/nonmatchings/55130", func_800580C0);
#endif

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
INCLUDE_ASM("asm/nonmatchings/55130", func_80058204);
#endif
