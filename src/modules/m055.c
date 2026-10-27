/*
 * Runlink module 55: plays texture "movies". A movie (from a table of
 * definitions) streams its frames from ROM into four frame buffers; a player
 * slot advances its time (looping, ping-pong or one shot) and loads the frames
 * it needs, and the render function blends the current frame with the next one
 * as screen rectangles, in strips that fit the texture memory.
 */
#include "module.h"

/* One movie in the definition table */
typedef struct M055MovieDef {
    /* 0x0 */ s16 width;
    /* 0x2 */ s16 height;
    /* 0x4 */ s16 frameCount;
    /* 0x6 */ u8 format;
    /* 0x7 */ u8 fps;
    /* 0x8 */ s32 romOffset;
    /* 0xC */ s32 frameSize;
} M055MovieDef;

/* A movie player slot */
typedef struct M055Player {
    /* 0x00 */ s8 movie; /* -1: free */
    /* 0x01 */ u8 depthCode;
    /* 0x02 */ u8 flags; /* 1: backwards, 2: ping-pong, 4: loop, 0x80: just started */
    /* 0x03 */ u8 brightness;
    /* 0x04 */ s32 time; /* frame << 8 */
    /* 0x08 */ s16 x;
    /* 0x0A */ s16 y;
    /* 0x0C */ u8 padC[2];
    /* 0x0E */ s8 curBuffer;
    /* 0x0F */ s8 nextBuffer;
    /* 0x10 */ s8 bufferFrame[4];
    /* 0x14 */ u8 *buffers[4];
} M055Player;

extern M055MovieDef *D_80104E50; /* the movie definitions */
extern M055Player D_80104E58[3];
extern void *func_8004A57C(s32 size, u32 colour);                     /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);                                /* DKR mempool_free */
extern s32 func_8004D530(s32 asset, void *dst, s32 offset, s32 size); /* DKR asset_load */
extern void func_80054F90(void);
extern void func_80055B80(Gfx **gfx); /* resets the render settings */

void func_m055_0022C(M055Player *player);

static u32 D_m055_00B80[10] = {
    0xE7000000, 0x00000000, 0xB6000000, 0x00010001, 0xFC22AA03,
    0x1F0C93FF, 0xEF100C0F, 0x03024000, 0xB8000000, 0x00000000,
};
static u32 D_m055_00BA8[10] = {
    0xE7000000, 0x00000000, 0xB6000000, 0x00010001, 0xFC22AA03,
    0x1F0C93FF, 0xEF102C0F, 0x00112048, 0xB8000000, 0x00000000,
};

M055Player *func_m055_00000(s32 movie, s32 x, s32 y, s32 brightness, s32 flags) {
    M055Player *player;
    M055MovieDef *def;
    s32 slot;
    s32 i;
    s32 bits;
    s32 size;
    s32 offset;

    player = D_80104E58;
    slot = 2;
    while (player->movie >= 0 && slot--) {
        player++;
    }
    if (slot < 0) {
        return NULL;
    }
    def = &D_80104E50[movie];
    switch (def->format) {
        case 0:
            bits = 32;
            player->depthCode = 0;
            break;
        case 1:
            bits = 4;
            player->depthCode = 3;
            break;
        case 2:
            bits = 8;
            player->depthCode = 2;
            break;
        case 3:
            bits = 16;
            player->depthCode = 1;
            break;
        default:
            bits = 0;
            player->depthCode = 0;
            break;
    }
    size = def->width * bits * def->height >> 3;
    if (size != 0) {
        player->movie = movie;
        player->flags = flags | 0x80;
        player->brightness = brightness;
        player->time = 0;
        player->x = x;
        player->y = y;
        player->curBuffer = -1;
        player->nextBuffer = -1;
        for (i = 0; i < 4; i++) {
            player->bufferFrame[i] = -1;
            player->buffers[i] = func_8004A57C(size, -1);
        }
        if (player->buffers[0] == NULL || player->buffers[1] == NULL || player->buffers[2] == NULL ||
            player->buffers[3] == NULL) {
            func_m055_0022C(player);
            player = NULL;
        } else {
            offset = def->romOffset;
            player->bufferFrame[0] = 0;
            func_8004D530(0x46, player->buffers[0], offset, def->frameSize);
            offset = def->romOffset + def->frameSize;
            player->bufferFrame[1] = 1;
            func_8004D530(0x46, player->buffers[1], offset, def->frameSize);
            func_80054F90();
        }
    }
    return player;
}

void func_m055_0022C(M055Player *player) {
    s32 i;

    if (player != NULL && player->movie != -1) {
        player->movie = -1;
        for (i = 0; i < 4; i++) {
            if (player->buffers[i] != NULL) {
                func_8004AA80(player->buffers[i]);
                player->buffers[i] = NULL;
            }
        }
    }
}

#ifdef NON_MATCHING
/* Differs only in the t-registers of curBuffer, nextBuffer, cur and next (t3, t0, t1, t2 here;
   t0, t1, t2, t3 in the original). */
void func_m055_002A0(M055Player *player, s32 updateRate) {
    s32 last;
    s32 step;
    s32 offset;
    s32 next;
    s32 curBuffer;
    s32 nextBuffer;
    s32 cur;
    s32 i;
    M055MovieDef *def;

    if (player != NULL && player->movie != -1) {
        def = &D_80104E50[player->movie];
        if (player->flags & 4) {
            last = (def->frameCount << 8) - 1;
        } else {
            last = (def->frameCount - 1) << 8;
        }
        curBuffer = -1;
        if (player->flags & 0x80) {
            player->flags &= ~0x80;
        } else {
            step = ((def->fps << 8) / 60) * updateRate;
            do {
                if (player->flags & 1) {
                    player->time -= step;
                    if (player->time < 0) {
                        if (player->flags & 4) {
                            if (player->flags & 2) {
                                player->time = -player->time;
                                player->flags &= ~1;
                            } else {
                                player->time += last;
                            }
                        } else {
                            player->time = 0;
                        }
                    }
                } else {
                    player->time += step;
                    if (last < player->time) {
                        if (player->flags & 2) {
                            player->time = (last - player->time) + last;
                            player->flags |= 1;
                        } else if (player->flags & 4) {
                            player->time -= last;
                        } else {
                            player->time = last;
                        }
                    }
                }
            } while (player->time < 0 || last < player->time);
        }
        cur = player->time >> 8;
        next = cur + 1;
        nextBuffer = -1;
        if (next >= def->frameCount) {
            next = 0;
        }
        i = 4;
        while (i--) {
            if (cur == player->bufferFrame[i]) {
                curBuffer = i;
            }
            if (next == player->bufferFrame[i]) {
                nextBuffer = i;
            }
        }
        if (curBuffer < 0) {
            i = 4;
            while (i--) {
                if (i != nextBuffer && i != player->curBuffer && i != player->nextBuffer) {
                    offset = def->romOffset + cur * def->frameSize;
                    player->bufferFrame[i] = cur;
                    curBuffer = i;
                    func_8004D530(0x46, player->buffers[i], offset, def->frameSize);
                    break;
                }
            }
        }
        if (nextBuffer < 0) {
            i = 4;
            while (i--) {
                if (i != curBuffer && i != player->curBuffer && i != player->nextBuffer) {
                    offset = def->romOffset + next * def->frameSize;
                    player->bufferFrame[i] = next;
                    nextBuffer = i;
                    func_8004D530(0x46, player->buffers[i], offset, def->frameSize);
                    break;
                }
            }
        }
        player->curBuffer = curBuffer;
        player->nextBuffer = nextBuffer;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m055", func_m055_002A0);
#endif

#ifdef NON_MATCHING
/* Not matched yet: the structure is right, but the order of the set-up computations, the
   registers of the hoisted display-list words and the frame (0x108 here, 0x100) differ. */
void func_m055_005B4(M055Player *player, Gfx **dl, f32 scale) {
    M055MovieDef *def;
    u8 *texA;
    u8 *texB;
    s32 width;
    s32 height;
    s32 blend;
    s32 x;
    s32 xh;
    s32 y;
    s32 prevY;
    s32 lineBytes;
    s32 rows;
    s32 n;
    s32 step;
    f32 yScale;
    Gfx *g;

    if (player != NULL && player->movie != -1) {
        def = &D_80104E50[player->movie];
        texA = player->buffers[player->curBuffer];
        width = def->width;
        height = def->height;
        texB = player->buffers[player->nextBuffer];
        blend = player->time & 0xFF;
        if (scale == 1.0f) {
            gSPDisplayList((*dl)++, D_m055_00B80);
        } else {
            gSPDisplayList((*dl)++, D_m055_00BA8);
        }
        gDPSetPrimColor((*dl)++, 0, 0, player->brightness, player->brightness, player->brightness, 255);
        gDPSetEnvColor((*dl)++, blend, blend, blend, blend);
        x = player->x << 2;
        xh = (s32) (width * scale * 4.0f) + x;
        y = player->y << 16;
        prevY = y;
        lineBytes = width * 2;
        step = 1024.0f / scale;
        yScale = scale * 65536.0f;
        if (height != 0) {
            rows = (0x800 / lineBytes) & ~1;
          do {
            if (rows < height) {
                n = rows;
                height -= rows;
            } else {
                n = height;
                height = 0;
            }
            gDPSetTextureImage((*dl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texB);
            gDPSetTile((*dl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x100, G_TX_LOADTILE, 0, G_TX_CLAMP, 0, 0,
                       G_TX_CLAMP, 0, 0);
            y += (s32) (n * yScale);
            gDPLoadSync((*dl)++);
            gDPLoadBlock((*dl)++, G_TX_LOADTILE, 0, 0, width * n - 1, 0);
            gDPPipeSync((*dl)++);
            gDPSetTile((*dl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, (lineBytes + 7) >> 3, 0x100, 1, 0, G_TX_CLAMP, 0, 0,
                       G_TX_CLAMP, 0, 0);
            gDPSetTileSize((*dl)++, 1, 0, 0, (width - 1) << 2, (n - 1) << 2);
            gDPSetTextureImage((*dl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, texA);
            gDPSetTile((*dl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_CLAMP, 0, 0, G_TX_CLAMP,
                       0, 0);
            gDPLoadSync((*dl)++);
            gDPLoadBlock((*dl)++, G_TX_LOADTILE, 0, 0, width * n - 1, 0);
            gDPPipeSync((*dl)++);
            gDPSetTile((*dl)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, (lineBytes + 7) >> 3, 0, 0, 0, G_TX_CLAMP, 0, 0,
                       G_TX_CLAMP, 0, 0);
            gDPSetTileSize((*dl)++, 0, 0, 0, (width - 1) << 2, (n - 1) << 2);
            /* a texture rectangle (F3D: G_RDPHALF_2 then G_RDPHALF_CONT) */
            g = (*dl)++;
            g->words.w0 = _SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(y >> 14, 0, 12);
            g->words.w1 = _SHIFTL(0, 24, 3) | _SHIFTL(x, 12, 12) | _SHIFTL(prevY >> 14, 0, 12);
            gImmp1((*dl)++, 0xB3, 0);
            gImmp1((*dl)++, 0xB2, _SHIFTL(step, 16, 16) | _SHIFTL(step, 0, 16));
            prevY = y;
            texA += n * lineBytes;
            texB += n * lineBytes;
          } while (height != 0);
        }
        func_80055B80(dl);
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m055", func_m055_005B4);
#endif
