/*
 * Runlink module 57: a front-end screen of company logos and credits. It fades
 * in a full-screen image (with a sound and music cue), the text "presented
 * in", two textured logos one after the other, a second full-screen image,
 * then plays module 55's animated logo with the copyright line, and returns
 * to the front end.
 */
#include "module.h"

/* A texture's header (DKR's TextureHeader, structs.h) */
typedef struct M057Texture {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 numberOfCommands; /* (DKR) */
    /* 0x0C */ Gfx *cmd;             /* (DKR) */
} M057Texture;

/* Module 55's animated logo */
typedef struct M055Logo {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ u8 alpha;
    /* 0x4 */ s32 time;
} M055Logo;

/* DKR's gDkrDmaDisplayList (f3ddkr.h): runs a texture's own display list */
#define G_DMADL 0x07
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                         \
    {                                                                              \
        Gfx *_g = (Gfx *) (pkt);                                                   \
                                                                                   \
        _g->words.w0 = (_SHIFTL(G_DMADL, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) | \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                   \
        _g->words.w1 = (unsigned int) (address);                                   \
    }

/* The texture rectangle in the older microcode's form (RDPHALF_2, RDPHALF_CONT) */
#define gSPTextureRectangleOld(pkt, xl, yl, xh, yh, tile, s, t, dsdx, dtdy)                    \
    {                                                                                         \
        Gfx *_g = (Gfx *) (pkt);                                                              \
                                                                                              \
        _g->words.w0 = (_SHIFTL(G_TEXRECT, 24, 8) | _SHIFTL(xh, 12, 12) | _SHIFTL(yh, 0, 12)); \
        _g->words.w1 = (_SHIFTL(tile, 24, 3) | _SHIFTL(xl, 12, 12) | _SHIFTL(yl, 0, 12));     \
        gImmp1(pkt, G_RDPHALF_2, (_SHIFTL(s, 16, 16) | _SHIFTL(t, 0, 16)));                   \
        gImmp1(pkt, G_RDPHALF_CONT, (_SHIFTL(dsdx, 16, 16) | _SHIFTL(dtdy, 0, 16)));          \
    }

extern Gfx *D_800FF398; /* the display list being built */
extern s32 func_800580C0(s32 image);              /* loads a full-screen image */
extern void func_80058204(Gfx **dl, s32 image, s32 arg2); /* draws it */
extern M057Texture *func_80055694(s32 id);         /* loads a texture */
extern void func_80055A00(M057Texture *tex);       /* frees it */
extern void func_8004AA80(s32 data);               /* frees */
extern void func_80055B80(Gfx **dl);
extern void func_80000510();
extern void func_80000FE4(s32 arg0);
extern void func_8004758C();
extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern s32 func_8005A8D8(void);
extern void func_8005A834(s32 arg0);
extern M055Logo *func_m055_00000(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
extern void func_m055_0022C(M055Logo *logo);
extern void func_m055_002A0(M055Logo *logo, s32 updateRate);
extern void func_m055_005B4(M055Logo *logo, Gfx **dl, f32 scale);

static s32 sFirstImage = 0;
static s32 sSecondImage = 0;
static M057Texture *sLogoTop = NULL;
static M057Texture *sLogoBottomLeft = NULL;
static M057Texture *sLogoBottomRight = NULL;
static M055Logo *sAnimatedLogo = NULL;
static s32 sStage = 0;
static s32 sFirstImageFade = 0;
static s32 sTextFade = 0;
static s32 sTopLogoFade = 0;
static s32 sBottomLogosFade = 0;
static s32 sSecondImageFade = 0;
static s32 sAnimatedLogoFade = 0;
static s32 sTimer = 0;
static Gfx sLogoDrawMode[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_FOG),
    gsDPSetCombineLERP(TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, 1, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, 1),
    gsDPSetOtherMode(0x000C0F, 0x0F0A4000),
    gsSPEndDisplayList(),
};

void func_m057_00000(void) {
    sFirstImage = func_800580C0(0);
    sSecondImage = func_800580C0(1);
    sLogoTop = func_80055694(0x28E);
    sLogoBottomLeft = func_80055694(0x28F);
    sLogoBottomRight = func_80055694(0x290);
    sAnimatedLogo = func_m055_00000(0, 0x7C, 0x41, 0, 0);
    sStage = 1;
    sFirstImageFade = 0;
    sTextFade = 0;
    sTopLogoFade = 0;
    sBottomLogosFade = 0;
    sSecondImageFade = 0;
    sAnimatedLogoFade = 0;
}

void func_m057_000C0(s32 updateRate) {
    switch (sStage) {
        case 1:
            sFirstImageFade += updateRate * 4;
            if (sFirstImageFade >= 0xFF) {
                func_80000510(0x55);
                func_80000FE4(0x7F);
                sFirstImageFade = 0xFF;
                sStage = 2;
            }
            break;
        case 2:
            sTextFade += updateRate * 4;
            if (sTextFade >= 0xFF) {
                sTextFade = 0xFF;
                sTimer = 60;
                sStage = 3;
            }
            break;
        case 3:
            if (sTimer <= 0) {
                sTopLogoFade -= updateRate * 8;
                if (sTopLogoFade <= 0) {
                    sTopLogoFade = 0;
                    sTimer = 60;
                    sStage = 4;
                }
            } else if (sTopLogoFade < 0xFF) {
                sTopLogoFade += updateRate * 8;
                if (sTopLogoFade >= 0x100) {
                    sTopLogoFade = 0xFF;
                }
            } else {
                sTimer -= updateRate;
            }
            break;
        case 4:
            if (sTimer <= 0) {
                sFirstImageFade -= updateRate * 8;
                sTextFade -= updateRate * 8;
                sBottomLogosFade -= updateRate * 8;
                if (sBottomLogosFade <= 0) {
                    sFirstImageFade = 0;
                    sTextFade = 0;
                    sBottomLogosFade = 0;
                    sTimer = 120;
                    sStage = 5;
                }
            } else if (sBottomLogosFade < 0xFF) {
                sBottomLogosFade += updateRate * 8;
                if (sBottomLogosFade >= 0x100) {
                    sBottomLogosFade = 0xFF;
                }
            } else {
                sTimer -= updateRate;
            }
            break;
        case 5:
            if (sTimer <= 0) {
                sSecondImageFade -= updateRate * 8;
                if (sSecondImageFade <= 0) {
                    sSecondImageFade = 0;
                    sStage = 6;
                }
            } else if (sSecondImageFade < 0xFF) {
                sSecondImageFade += updateRate * 8;
                if (sSecondImageFade >= 0x100) {
                    sSecondImageFade = 0xFF;
                }
            } else {
                sTimer -= updateRate;
            }
            break;
        case 6:
            func_m055_002A0(sAnimatedLogo, updateRate);
            if (sAnimatedLogo->time < 0x200) {
                sAnimatedLogoFade = sAnimatedLogo->time >> 1;
            } else if (sAnimatedLogo->time > 0x2300) {
                sAnimatedLogoFade = (0x2500 - sAnimatedLogo->time) >> 1;
            } else {
                sAnimatedLogoFade = 0xFF;
            }
            if (sAnimatedLogo->time >= 0x2500) {
                sAnimatedLogoFade = 0;
                sStage = 7;
                sTimer = 180;
            }
            break;
        case 7:
            sTimer -= updateRate;
            if (sTimer <= 0) {
                sStage = 0;
                func_8004758C();
            }
            break;
    }
    if (sFirstImageFade != 0) {
        gDPSetPrimColor(D_800FF398++, 0, 0, sFirstImageFade, sFirstImageFade, sFirstImageFade, 255);
        func_80058204(&D_800FF398, sFirstImage, 0);
    }
    if (sTopLogoFade != 0) {
        gDPSetPrimColor(D_800FF398++, 0, 0, sTopLogoFade, sTopLogoFade, sTopLogoFade, 255);
        gSPDisplayList(D_800FF398++, sLogoDrawMode);
        gDkrDmaDisplayList(D_800FF398++, OS_K0_TO_PHYSICAL(sLogoTop->cmd), sLogoTop->numberOfCommands);
        gSPTextureRectangleOld(D_800FF398++, 88 << 2, 164 << 2, 232 << 2, 220 << 2, 0, 0, 0x700, 0x400, 0xFC00);
        func_80055B80(&D_800FF398);
    }
    if (sBottomLogosFade != 0) {
        gDPSetPrimColor(D_800FF398++, 0, 0, sBottomLogosFade, sBottomLogosFade, sBottomLogosFade, 255);
        gSPDisplayList(D_800FF398++, sLogoDrawMode);
        gDkrDmaDisplayList(D_800FF398++, OS_K0_TO_PHYSICAL(sLogoBottomLeft->cmd),
                           sLogoBottomLeft->numberOfCommands);
        gSPTextureRectangleOld(D_800FF398++, 64 << 2, 160 << 2, 160 << 2, 224 << 2, 0, 0, 0x800, 0x400, 0xFC00);
        gDkrDmaDisplayList(D_800FF398++, OS_K0_TO_PHYSICAL(sLogoBottomRight->cmd),
                           sLogoBottomRight->numberOfCommands);
        gSPTextureRectangleOld(D_800FF398++, 160 << 2, 160 << 2, 256 << 2, 224 << 2, 0, 0, 0x800, 0x400, 0xFC00);
        func_80055B80(&D_800FF398);
    }
    if (sTextFade != 0) {
        func_8006FC54(2);
        func_8006FC8C(0, 0, 0, 0);
        func_8006FC68(255, 255, 255, 255, sTextFade);
        func_8006FCA8(&D_800FF398, 160, 136, "presented in", 12);
    }
    if (sSecondImageFade != 0) {
        gDPSetPrimColor(D_800FF398++, 0, 0, sSecondImageFade, sSecondImageFade, sSecondImageFade, 255);
        func_80058204(&D_800FF398, sSecondImage, 0);
    }
    if (sAnimatedLogoFade != 0) {
        sAnimatedLogo->alpha = sAnimatedLogoFade;
        func_m055_005B4(sAnimatedLogo, &D_800FF398, 1.0f);
        func_8006FC54(2);
        func_8006FC8C(0, 0, 0, 0);
        func_8006FC68(255, 255, 255, 255, sAnimatedLogoFade);
        func_8006FCA8(&D_800FF398, 160, 192, "TM and < 1999 rare", 12);
    }
}

void func_m057_0090C(void) {
    func_m055_0022C(sAnimatedLogo);
    func_80055A00(sLogoBottomRight);
    func_80055A00(sLogoBottomLeft);
    func_80055A00(sLogoTop);
    func_8004AA80(sSecondImage);
    func_8004AA80(sFirstImage);
    func_8005A834(func_8005A8D8());
}
