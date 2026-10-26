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

INCLUDE_ASM("asm/nonmatchings/55130", func_80054C4C);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054CA4);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054CF0);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054EA0);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054EF0);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054F74);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054F90);

INCLUDE_ASM("asm/nonmatchings/55130", func_80054FBC);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055180);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055198);

INCLUDE_ASM("asm/nonmatchings/55130", func_800551A8);

INCLUDE_ASM("asm/nonmatchings/55130", func_800551E8);

INCLUDE_ASM("asm/nonmatchings/55130", func_800551F8);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055208);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055228);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055234);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055260);

INCLUDE_ASM("asm/nonmatchings/55130", func_800552DC);

INCLUDE_ASM("asm/nonmatchings/55130", func_8005539C);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055480);

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

INCLUDE_ASM("asm/nonmatchings/55130", func_80055684);

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

INCLUDE_ASM("asm/nonmatchings/55130", func_80055AB8);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055B0C);

/**
 * Set the colour tag that determines which memory pool textures will be loaded into.
 * By default, this generally stays as COLOUR_TAG_MAGENTA
 * Official Name: setTexMemColour
 */
void set_texture_colour_tag(s32 tagID) {
    gTexColourTag = tagID;
}

INCLUDE_ASM("asm/nonmatchings/55130", func_80055B40);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055B80);

INCLUDE_ASM("asm/nonmatchings/55130", func_80055C00);

INCLUDE_ASM("asm/nonmatchings/55130", func_80056000);

INCLUDE_ASM("asm/nonmatchings/55130", func_8005600C);

INCLUDE_ASM("asm/nonmatchings/55130", func_80056064);

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

INCLUDE_ASM("asm/nonmatchings/55130", func_80057CEC);

INCLUDE_ASM("asm/nonmatchings/55130", func_80057D1C);

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
