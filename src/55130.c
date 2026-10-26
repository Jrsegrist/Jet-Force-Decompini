#include "common.h"
#include "memory.h"

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
    /* 0x14 */ u8 pad14[2];
    /* 0x16 */ s16 textureSize;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
} TextureHeader;

/* DKR's Sprite (renamed: libultra's sp.h has a Sprite too) */
typedef struct SpriteAsset {
    /* 0x00 */ s16 numberOfFrames;
    /* 0x02 */ s16 numberOfTextures;
    /* 0x04 */ s16 numberOfInstances;
    /* 0x06 */ s16 drawFlags;
    /* 0x08 */ u8 pad8[8];
    /* 0x10 */ TextureHeader **textures;
} SpriteAsset;

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
    /* 0x08 */ u8 pad8[0x28 - 0x08];
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

INCLUDE_ASM("asm/nonmatchings/55130", func_80054710);

INCLUDE_ASM("asm/nonmatchings/55130", func_8005483C);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054B44);

extern s32 D_800FECA0;
extern s32 D_800A390C;
extern s32 D_800FECBC;

INCLUDE_ASM("asm/nonmatchings/55130", func_80054C4C);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054CA4);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054CF0);

/**
 * Return the current framebuffer dimensions.
 * JFG: writes the width and height (DKR packs them into one s32).
 * Official Name: viGetCurrentSize
 */
void fb_size(s32 *width, s32 *height) {
    *width = D_800A392C[D_800FECA8].width;
    *height = D_800A392C[D_800FECA8].height;
}

INCLUDE_ASM("asm/nonmatchings/55130", func_80054EF0);

extern f32 D_800A391C;
extern f32 D_800A3920;

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

INCLUDE_ASM("asm/nonmatchings/55130", func_80054FBC);

s32 func_80055180(void) {
    return D_800FECA8 & 3;
}

extern s8 D_800FECAA;

s8 func_80055198(void) {
    return D_800FECAA;
}

void func_80054CF0(void);

void func_800551A8(s32 adjust) {
    if (adjust < -30) {
        adjust = -30;
    }
    if (adjust > 30) {
        adjust = 30;
    }
    D_800FECAA = adjust;
    func_80054CF0();
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

INCLUDE_ASM("asm/nonmatchings/55130", func_80055260);

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

INCLUDE_ASM("asm/nonmatchings/55130", func_80055694);

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

extern s32 D_800A4E7C;
TextureHeader *load_texture(s32 assetIndex);

TextureHeader *func_80055B0C(s32 textureID, s32 address) {
    TextureHeader *tex;
    D_800A4E7C = address;
    tex = load_texture(textureID);
    D_800A4E7C = 0;
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

INCLUDE_ASM("asm/nonmatchings/55130", func_80056798);

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

INCLUDE_ASM("asm/nonmatchings/55130", func_80057A10);

INCLUDE_ASM("asm/nonmatchings/55130", func_80057A54);

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

INCLUDE_ASM("asm/nonmatchings/55130", func_80057D58);

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

INCLUDE_ASM("asm/nonmatchings/55130", func_800580C0);

INCLUDE_ASM("asm/nonmatchings/55130", func_80058204);
