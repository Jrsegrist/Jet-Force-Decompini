/*
 * Runlink module 48: a full-screen afterimage effect. Each frame the update
 * builds a display list (alternating between two buffers) that draws the
 * previous frame's image back over the screen in strips of four lines,
 * textured and faded by the environment alpha; a second function sends it
 * to the RSP as its own task.
 */
#include "module.h"

/*
 * The texture rectangle of the game's microcode: the texture coordinates
 * and steps follow in two half commands (0xB3 and 0xB2), as in the original
 * Fast3D's gbi.h, not the 0xB4/0xB3 pair that this gbi.h's gSPTextureRectangle uses.
 */
#define gM048TextureRectangle(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                    \
    {                                                                                         \
        Gfx *_g = (Gfx *) (pkt);                                                              \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));     \
        gImmp1(pkt, 0xB3, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                         \
        gImmp1(pkt, 0xB2, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));                   \
    }

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);            /* DKR mempool_free */
extern s32 func_800551F8(void);
extern void func_80054EA0(u32 *width, u32 *height);
extern void func_80050E90(Gfx **dl, s32 segment, s32 address);
extern void func_8004E40C(Gfx **dl);
extern void rdp_set_default_modes(Gfx **dl);
extern void set_active_camera(s32 arg0);
extern void func_8004046C(Gfx **dl, s32 arg1);
extern void func_8004DA10(Gfx *start, Gfx *end, s32 arg2, s32 arg3);

extern s32 D_800A3338;
extern s32 D_800A3630;
extern u8 *D_800FECB0; /* the previous frame's image */
extern s32 D_800FECB4; /* the frame buffer */
extern s32 gVideoDepthBuffer;

static Gfx *sDisplayLists[2] = { NULL, NULL };
static s32 sCurrentList = 0;
static u8 sEnvAlpha = 100;

static Gfx *sListStart;
static Gfx *sListEnd;
static s32 sFrameBuffer;

void func_m048_00058(void);

void func_m048_00000(void) {
    func_m048_00058();
    sDisplayLists[0] = func_8004A57C(0x2000, -1);
    sDisplayLists[1] = func_8004A57C(0x2000, -1);
    sCurrentList = 0;
    D_800A3338 = 1;
}

void func_m048_00058(void) {
    if (sDisplayLists[0] != NULL) {
        func_8004AA80(sDisplayLists[0]);
    }
    if (sDisplayLists[1] != NULL) {
        func_8004AA80(sDisplayLists[1]);
    }
    sDisplayLists[0] = NULL;
    sDisplayLists[1] = NULL;
    sListStart = NULL;
    sListEnd = NULL;
    D_800A3338 = 0;
    D_800A3630 = 0;
}

/* The module's shutdown function (runlink fini) */
void func_m048_000D4(void) {
    func_m048_00058();
}
#pragma weak __rl_fini = func_m048_000D4

void func_m048_000F4(s32 arg0) {
    u32 y;
    u8 *image;
    u32 width;
    u32 height;
    s32 rows;
    Gfx *dl;

    sListStart = NULL;
    if (func_800551F8()) {
        dl = sDisplayLists[sCurrentList];
        func_80054EA0(&width, &height);
        sListStart = dl;
        sFrameBuffer = D_800FECB4;
        func_80050E90(&dl, 0, 0);
        func_80050E90(&dl, 1, D_800FECB4);
        func_80050E90(&dl, 2, gVideoDepthBuffer);
        func_8004E40C(&dl);
        rdp_set_default_modes(&dl);
        gSPClearGeometryMode(dl++, G_ZBUFFER | G_FOG);
        gDPSetColorImage(dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, D_800FECB4 + 0x80000000);
        gDPSetScissor(dl++, G_SC_NON_INTERLACE, 0, 0, width, height);
        gMoveWd(dl++, G_MW_NUMLIGHT, 0, 0);
        gDPPipeSync(dl++);
        gSPClearGeometryMode(dl++, G_CULL_FRONT);
        gDPSetBlendColor(dl++, 0, 0, 0, 100);
        gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor(dl++, 255, 255, 255, sEnvAlpha);
        set_active_camera(0);
        func_8004046C(&dl, 0);
        gDPPipeSync(dl++);
        gSPClearGeometryMode(dl++, G_ZBUFFER | G_FOG);
        gSPSetGeometryMode(dl++, G_SHADE | G_SHADING_SMOOTH);
        gDPSetCombineLERP(dl++, 0, 0, 0, TEXEL0, 0, 0, 0, PRIMITIVE, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT);
        gDPSetOtherMode(dl++, 0xCCF, 0x504240);
        image = D_800FECB0;
        for (y = 0; y < height; y += rows) {
            rows = height - y;
            if (rows > 4) {
                rows = 4;
            }
            gDPLoadTextureBlock(dl++, image, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, rows, 0, G_TX_CLAMP, G_TX_CLAMP,
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gM048TextureRectangle(dl++, 0, y << 2, width << 2, (y + rows) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            image += width * rows * 2;
        }
        gDPFullSync(dl++);
        gSPEndDisplayList(dl++);
        sListEnd = dl;
    }
}

void func_m048_006F0(void) {
    if (sListStart != NULL) {
        func_8004DA10(sListStart, sListEnd, 5, sFrameBuffer);
        D_800A3630 = 1;
        sCurrentList ^= 1;
    }
}
