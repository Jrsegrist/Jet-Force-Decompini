/*
 * Textures and sprites (DKR textures_sprites.c): split from 55130.c, which is three original files
 * (video, textures, screens: each has its own .rodata).
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


/*
 * The file's debug messages (DKR's stubbed_printf strings in load_texture, tex_free, tex_load_sprite and
 * sprite_free; JFG adds the two Restore lines). Nothing references them, and some of their functions are still
 * asm, so they are kept here in their .rodata order.
 */
const char D_800AD190[] = "Error: Texture no %x out of range on load. !!\n";
const char D_800AD1C0[] = "Restore to %x\n";
const char D_800AD1D0[] = "Restore1 to %x\n";
const char D_800AD1E0[] = "TEX Error: Palette memory overflow!!\n";
const char D_800AD208[] = "TEX Error: TexTab overflow!!\n";
const char D_800AD228[] = "texFreeTexture: NULL tex!!\n";
const char D_800AD244[] = "TEX Error: Tryed to deallocate non-existent texture!!\n";
const char D_800AD27C[] = "SRPBUF overflow!!\n";
const char D_800AD290[] = "Error: Sprite table overflow!!\n";
const char D_800AD2B0[] = "texFreeSprite: NULL sprite!!\n";
const char D_800AD2D0[] = "TEXSPR Error: Tryed to deallocate non-existent sprite!!\n";

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
extern s32 gCiPalettesSize;
extern u8 *gCiPalettes;
extern SpriteAssetData *gCurrentSpriteAsset;
extern s32 *gSpriteOffsetTable;
extern s32 gSpriteTableSize;
extern s32 gFirstTexIsLoaded;

/**
 * Load the texture table from ROM and allocate space for all the texture asset management.
 * JFG: 700 textures, 100 sprites.
 * DKR: tex_init_textures
 */
void tex_init_textures(void) {
    s32 i;

    gTextureCache = mempool_alloc_safe(8 * 700, 0xFF00FFFF);
    gCiPalettes = mempool_alloc_safe(0x280, 0xFF00FFFF);
    gNumberOfLoadedTextures = 0;
    gCiPalettesSize = 0;
    gTextureAssetTable[0] = (s32 *) asset_table_load(3);
    gTextureAssetTable[1] = (s32 *) asset_table_load(1);

    for (i = 0; gTextureAssetTable[0][i] != -1; i++) {}
    gTextureTableSize[0] = --i;

    for (i = 0; gTextureAssetTable[1][i] != -1; i++) {}
    gTextureTableSize[1] = --i;

    gSpriteCache = mempool_alloc_safe(8 * 100, 0xFF00FFFF);
    gCurrentSpriteAsset = mempool_alloc_safe(0x200, 0xFF00FFFF);
    gSpriteCacheCount = 0;
    gSpriteOffsetTable = (s32 *) asset_table_load(0x16);
    gSpriteTableSize = 0;
    while (gSpriteOffsetTable[gSpriteTableSize] != -1) {
        gSpriteTableSize++;
    }
    gSpriteTableSize--;

    gTempTextureHeader = mempool_alloc_safe(sizeof(TempTexHeader), 0xFF00FFFF);
    gFirstTexIsLoaded = FALSE;
}

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
extern s32 gForcedColourTag; // JFG: the texture being loaded (-1 when none)
extern u8 *D_800A4E7C; // JFG: load the next texture at this address
void *mempool_alloc(s32 size, u32 colourTag);
void *mempool_alloc_fixed(s32 size, u8 *address, u32 colourTag);
void tex_dlist_build(TextureHeader *tex, Gfx *dList);

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
    gForcedColourTag = id;
    asset_load(assetSection, (u32) gTempTextureHeader, assetOffset, sizeof(TempTexHeader));
    if (!gTempTextureHeader->isCompressed) {
        if (D_800A4E7C) {
            tex = (TextureHeader *) mempool_alloc_fixed(assetSize + 0x90, D_800A4E7C, gTexColourTag);
        } else {
            tex = (TextureHeader *) mempool_alloc(assetSize + 0x90, gTexColourTag);
        }
        gForcedColourTag = -1;
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
        gForcedColourTag = -1;
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
    tex_dlist_build(tex, (Gfx *) align16((u8 *) ((s32) tex + assetSize)));
    if (gCiPalettesSize >= 0x280) {
        return NULL;
    }
    if (gNumberOfLoadedTextures > 700) {
        return NULL;
    }
    return tex;
}
#else
INCLUDE_ASM("asm/nonmatchings/560B0", func_80055694);
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

/* Returns the cache id of a loaded texture (-1 if not found) (0x80055AB8). */
s32 tex_find_id(s32 texture) {
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

/* Loads a texture at a given address (0x80055B0C). */
TextureHeader *load_texture_at(s32 textureID, s32 address) {
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

/* JFG: a render mode: the display lists to run (picked by the mode bits) and the bits it keeps/adds */
typedef struct MaterialMode {
    /* 0x00 */ Gfx *combine;  // indexed by bits >> 3
    /* 0x04 */ Gfx *otherMode; // indexed by bits
    /* 0x08 */ u32 mask;
    /* 0x0C */ u32 bits;
} MaterialMode;

#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(7, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                      \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }

extern MaterialMode D_800A48E0[]; // textured
extern MaterialMode D_800A4BE0[]; // untextured
extern u8 D_800A4E70;             // JFG: semi-transparent draws become cutouts

#ifdef NON_MATCHING
/* NON_MATCHING: 5 words: the final copy keeps bits >> 3 in $a1 and bits << 3 in $a2 (the ROM has them swapped) */
/**
 * Load a texture (JFG: up to two frames for blending) and set the render mode for the flags.
 * Tracks which modes are active, to skip setting them again.
 * DKR material_set (textures_sprites.c), heavily changed in JFG.
 */
void func_80055C00(Gfx **dList, TextureHeader *texhead, s32 flags, s32 texOffset) {
    Gfx *gfx;
    s32 textured;
    TextureHeader *tex1;
    TextureHeader *tex2;
    MaterialMode *modes;
    MaterialMode *mode;
    Gfx *cmd;
    s32 frame;
    s32 numFrames;
    s32 next;
    s32 index;
    s32 bits;
    s32 key;

    gfx = *dList;
    textured = FALSE;
    if (texhead != NULL) {
        frame = texOffset >> 16;
        numFrames = texhead->numOfTextures >> 8;
        if (numFrames >= 2 && frame < numFrames && D_800A4E6C == 0) {
            tex1 = (TextureHeader *) ((u8 *) texhead + frame * texhead->textureSize + 0x20);
            if ((texhead->flags & 0x40) && texhead->unk1B < 2) {
                next = frame + 1;
                if (next >= numFrames) {
                    if (texhead->unk18) {
                        next = 0;
                    } else {
                        next = numFrames - 1;
                    }
                }
                tex2 = (TextureHeader *) ((u8 *) texhead + next * texhead->textureSize + 0x20);
            } else {
                tex2 = tex1;
            }
        } else {
            tex1 = texhead + 1;
            tex2 = tex1;
        }
        flags |= texhead->flags;
        textured = TRUE;
        modes = D_800A48E0;
        if ((s32) tex1 != D_800FED34 || (s32) tex2 != D_800FED38) {
            D_800FED34 = (s32) tex1;
            D_800FED38 = (s32) tex2;
            cmd = texhead->cmd;
            gfx->words.w0 = cmd->words.w0;
            gfx->words.w1 = (u32) tex1;
            gfx++;
            cmd++;
            if (texhead->unk1B >= 2) {
                gSPDisplayList(gfx++, cmd);
            } else {
                gDkrDmaDisplayList(gfx++, OS_K0_TO_PHYSICAL(cmd), 6);
                if ((texhead->flags & 0x40) && texhead->unk1B < 2) {
                    gfx->words.w0 = cmd[6].words.w0;
                    gfx->words.w1 = (u32) tex2;
                    gfx++;
                    cmd += 7;
                    gDkrDmaDisplayList(gfx++, OS_K0_TO_PHYSICAL(cmd), 6);
                }
            }
        }
    } else {
        modes = D_800A4BE0;
    }
    if ((flags & 0x80) && D_800A4E70) {
        flags = (flags & ~0x80) | 4;
    }
    flags &= ~gBlockedRenderFlags;
    index = (flags & 0x70) >> 4;
    if (textured) {
        if (texhead->unk1B >= 2) {
            index += 0x20;
            if (flags & 0x80) {
                index += 8;
            }
        } else if (flags & 0x80) {
            index += 8;
        } else if (flags & 0x100) {
            index += 0x10;
        } else if (flags & 0x200) {
            index += 0x18;
        }
    } else if (flags & 0x400) {
        index += 8;
    }
    mode = (MaterialMode *) ((u8 *) modes + (index << 4));
    bits = mode->bits | (flags & mode->mask);
    key = (index << 8) | bits;
    if (key != D_800FED3C || (s32) modes != D_800FED30) {
        D_800FED3C = key;
        D_800FED30 = (s32) modes;
        gDPPipeSync(gfx++);
        if (bits & 2) {
            if (!D_800FED40) {
                gSPSetGeometryMode(gfx++, G_ZBUFFER);
            }
            D_800FED40 = TRUE;
        } else {
            if (D_800FED40) {
                gSPClearGeometryMode(gfx++, G_ZBUFFER);
            }
            D_800FED40 = FALSE;
        }
        if (bits & 8) {
            if (!D_800FED44) {
                gSPSetGeometryMode(gfx++, G_FOG);
            }
            D_800FED44 = TRUE;
        } else {
            if (D_800FED44) {
                gSPClearGeometryMode(gfx++, G_FOG);
            }
            D_800FED44 = FALSE;
        }
        gfx[0].words.w0 = mode->combine[bits >> 3].words.w0;
        gfx[0].words.w1 = mode->combine[bits >> 3].words.w1;
        gfx[1].words.w0 = mode->otherMode[bits].words.w0;
        gfx[1].words.w1 = mode->otherMode[bits].words.w1;
        gfx += 2;
    }
    *dList = gfx;
}
#else
INCLUDE_ASM("asm/nonmatchings/560B0", func_80055C00);
#endif

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

extern MaterialMode D_800A4CE0[]; // JFG: the sprite render modes
void effect_slot_start(s32 slot, u8 *dims);

INCLUDE_ASM("asm/nonmatchings/560B0", func_80056070);

/* Element sizes of the sprite's buffers */
typedef struct SpriteTriangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vi0;
    /* 0x02 */ u8 vi1;
    /* 0x03 */ u8 vi2;
    /* 0x04 */ s16 uv[3][2];
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
    sprite->unk14 = (s8 *) ((u8 *) sprite + unk14Offset);

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

#ifdef NON_MATCHING
/* NON_MATCHING: 4 words: the target stores the spilled tileEnd before the loop's entry test (in its delay slot),
   mine in the loop preheader */
/* JFG's DMA display list, vertex and two-triangle commands here: parameter, command and length or'ed in that
   order; the vertices are appended (0x200) and counted from 0 */
#define gDkrDmaDisplayListSpr(pkt, address, numberOfCommands)                                        \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL((numberOfCommands), 16, 8) | _SHIFTL(7, 24, 8) |                      \
                        _SHIFTL((numberOfCommands) * 8, 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
    }
#define gSPVertexSpr(pkt, v, n)                                                                      \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(((n) << 3) | ((u32) (v) & 6), 16, 8) | _SHIFTL(G_VTX, 24, 8) |        \
                        _SHIFTL((((n) << 3) + ((n) << 1) + 8) | 0x200, 0, 16));                     \
        _g->words.w1 = (unsigned int) (v);                                                           \
    }
#define gSPPolygonSpr(pkt, ptr)                                                                      \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = 0x05110020; /* two textured triangles */                                       \
        _g->words.w1 = (unsigned int) (ptr);                                                         \
    }

/**
 * Creates a display list that renders the specified sprite frame.
 * A frame may consist of multiple tiles.
 * JFG: the first frame always sets the draw flags; with draw flag 0x40 the tiles' own display lists are left out
 * and the command counts of the parts are recorded in unk14 instead.
 * DKR: sprite_init_frame
 */
void sprite_init_frame(SpriteAssetData *spriteAsset, SpriteAsset *sprite, s32 frameId) {
    UNUSED s32 pad[2];
    s32 anchorX;
    s32 anchorY;
    s32 tileEnd;
    s32 tileOffsetX;
    s32 tileOffsetY;
    s32 left;
    s32 numQuads;
    s32 curVertIndex;
    s32 texWidth;
    s32 texHeight;
    s32 tileIndex;
    s32 numMarks;
    SpriteVertex *vertex;
    SpriteVertex *curVerts;
    SpriteTriangle *triangle;
    Gfx *dlptr;
    Gfx *mark;
    TextureHeader *tex;

    anchorX = spriteAsset->anchorX;
    anchorY = spriteAsset->anchorY;
    dlptr = gSpriteDLists;
    vertex = gSpriteVertices;
    triangle = gSpriteTriangles;
    tileIndex = spriteAsset->frameTexOffsets[frameId];
    tileEnd = spriteAsset->frameTexOffsets[frameId + 1];
    if (frameId == 0 || tileIndex < tileEnd) {
        tex = sprite->textures[tileIndex];
        sprite->unk1 = tex->unk18;
        sprite->drawFlags = (tex->flags & 0xFFFF) & 0xC07B;
    }
    curVertIndex = 0;
    numQuads = 0;
    mark = dlptr;
    numMarks = 0;
    while (tileIndex < tileEnd) {
        tex = sprite->textures[tileIndex];
        texWidth = tex->width;
        texHeight = tex->height;
        tileOffsetX = tex->posX - anchorX;
        tileOffsetY = anchorY - tex->posY;
        curVerts = vertex;

        vertex->x = tileOffsetX;
        vertex->y = tileOffsetY - 1;
        vertex->z = 0;
        vertex->r = 255;
        vertex->g = 255;
        vertex->b = 255;
        vertex->a = 255;
        vertex++;

        vertex->x = tileOffsetX + texWidth - 1;
        vertex->y = tileOffsetY - 1;
        vertex->z = 0;
        vertex->r = 255;
        vertex->g = 255;
        vertex->b = 255;
        vertex->a = 255;
        vertex++;

        vertex->x = tileOffsetX + texWidth - 1;
        vertex->y = tileOffsetY - texHeight;
        vertex->z = 0;
        vertex->r = 255;
        vertex->g = 255;
        vertex->b = 255;
        vertex->a = 255;
        vertex++;

        vertex->x = tileOffsetX;
        vertex->y = tileOffsetY - texHeight;
        vertex->z = 0;
        vertex->r = 255;
        vertex->g = 255;
        vertex->b = 255;
        vertex->a = 255;
        vertex++;

        if (sprite->drawFlags & 0x40) {
            if (mark != dlptr) {
                sprite->unk14[numMarks++] = dlptr - mark;
                mark = dlptr;
            }
        } else {
            gDkrDmaDisplayListSpr(dlptr++, OS_K0_TO_PHYSICAL(tex->cmd), tex->numberOfCommands);
        }

        if (numQuads == 0) {
            left = tileEnd - tileIndex;
            if (left > 5) {
                left = 5;
            }
            gSPVertexSpr(dlptr++, OS_K0_TO_PHYSICAL(curVerts), left * 4);
        }

        gSPPolygonSpr(dlptr++, OS_K0_TO_PHYSICAL(triangle));
        triangle->flags = 0x40;
        triangle->vi0 = curVertIndex + 3;
        triangle->vi1 = curVertIndex + 2;
        triangle->vi2 = curVertIndex + 1;
        triangle->uv[0][0] = (texWidth - 1) << 5;
        triangle->uv[0][1] = (texHeight - 1) << 5;
        triangle->uv[1][0] = (texWidth - 1) << 5;
        triangle->uv[1][1] = 0;
        triangle->uv[2][0] = 1;
        triangle->uv[2][1] = 0;
        triangle++;

        triangle->flags = 0x40;
        triangle->vi0 = curVertIndex + 4;
        triangle->vi1 = curVertIndex + 3;
        triangle->vi2 = curVertIndex + 1;
        triangle->uv[0][0] = 1;
        triangle->uv[0][1] = (texHeight - 1) << 5;
        triangle->uv[1][0] = (texWidth - 1) << 5;
        triangle->uv[1][1] = (texHeight - 1) << 5;
        triangle->uv[2][0] = 1;
        triangle->uv[2][1] = 0;
        triangle++;

        curVertIndex += 4;
        numQuads++;
        tileIndex++;
        if (numQuads >= 5) {
            numQuads = 0;
            curVertIndex = 0;
        }
    }
    if (sprite->drawFlags & 0x40) {
        sprite->unk14[numMarks++] = dlptr - mark;
        sprite->unk14[numMarks] = 0;
    }
    gDPPipeSync(dlptr++);
    gSPEndDisplayList(dlptr++);
    gSpriteDLists = dlptr;
    gSpriteVertices = vertex;
    gSpriteTriangles = triangle;
}
#else
INCLUDE_ASM("asm/nonmatchings/560B0", func_80056D24);
#endif

void func_8005719C(Gfx **dList, TextureHeader *tex, s32 arg2, s32 arg3);

/* JFG: build a texture's display list at dList (a second load for some textures) and count its commands. */
void tex_dlist_build(TextureHeader *tex, Gfx *dList) {
    Gfx *dl = dList;

    tex->cmd = dList;
    func_8005719C(&dl, tex, 0, 0);
    if ((tex->unk1B & 0xFF) < 2 && (tex->flags & 0x40)) { // the no-op & 0xFF is needed to match
        if (!(tex->format & 0xF)) {
            func_8005719C(&dl, tex, 1, (0x1000 - tex->textureSize) >> 3);
        } else {
            func_8005719C(&dl, tex, 1, 0x100);
        }
    }
    tex->numberOfCommands = dl - tex->cmd;
}

INCLUDE_ASM("asm/nonmatchings/560B0", func_8005719C);


/**
 * JFG: advance a sprite animation's frame position by updateRate * speed / 60 frames in the direction of flag 2
 * (flag 1: running, 4: ping-pong, 8: loop); returns 1 when it reached an end (and bounced, wrapped or stopped).
 * The empty address tests keep sprite, updateRate and frame in their stack homes, as the target does.
 */
s32 sprite_anim_advance(SpriteAsset *sprite, s32 *flags, u32 updateRate, f32 *frame, u32 speed) {
    s32 reverse;
    s32 loop;
    s32 pingpong;
    s32 done;
    f32 pos;
    f32 lo;
    f32 hi;

    if (&sprite) {}
    if (&updateRate) {}
    if (&frame) {}
    if (!(*flags & 1)) {
        return 0;
    }
    reverse = *flags & 2;
    pos = *frame;
    loop = *flags & 8;
    pingpong = *flags & 4;
    done = 0;
    if (!(sprite->textures[0]->flags & 0x40)) {
        if (pingpong) {
            hi = sprite->numberOfFrames - 0.5f;
            if (loop) {
                lo = 0.5f;
            } else {
                lo = 0.0f;
            }
        } else {
            lo = 0.0f;
            hi = sprite->numberOfFrames - 0.01f;
        }
    } else {
        lo = 0.0f;
        if (pingpong || !loop || !sprite->textures[0]->unk18) {
            hi = sprite->numberOfFrames - 1.0f;
        } else {
            hi = sprite->numberOfFrames;
        }
    }
    if (reverse) {
        pos -= (s32) (updateRate * speed) / 60.0f;
        if (pos < lo) {
            if (pingpong) {
                if (loop) {
                    pos = (lo - pos) + lo;
                    *flags &= ~2;
                    done = 1;
                } else {
                    pos = lo;
                    done = 1;
                }
            } else {
                if (loop) {
                    pos += hi;
                } else {
                    pos = lo;
                }
                done = 1;
            }
        }
    } else {
        pos += (s32) (updateRate * speed) / 60.0f;
        if (hi < pos) {
            done = 1;
            if (pingpong) {
                pos = hi - (pos - hi);
                *flags |= 2;
            } else if (loop) {
                pos -= hi;
            } else {
                pos = hi;
            }
        }
    }
    *frame = pos;
    return done;
}

/* JFG: sprite_anim_advance for a single texture (as a sprite of its frames). */
s32 func_80057A10(TextureHeader *tex, s32 *flags, s32 updateRate, f32 *frame, s32 arg4) {
    SpriteAsset sprite;
    TextureHeader *textures[1];

    textures[0] = tex;
    sprite.textures = textures;
    sprite.numberOfFrames = tex->numOfTextures >> 8;
    return sprite_anim_advance(&sprite, flags, updateRate, frame, arg4);
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
void colour_cycle_reset(ColourCycle *cycle);

/* Sets up a colour cycle from a misc asset (0x80057CEC). */
void colour_cycle_init(ColourCycle *cycle, s32 assetID) {
    cycle->source = (ColourCycle *) get_misc_asset(assetID);
    colour_cycle_reset(cycle);
}

/* Resets a colour cycle to its first colour (0x80057D1C). */
void colour_cycle_reset(ColourCycle *cycle) {
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
