#include "common.h"
#include "memory.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern s32 sBackgroundFillColour;
extern u8 sBGPrimColourB;
extern u8 sBGPrimColourG;
extern u8 sBGPrimColourR;
extern Gfx dRspInit[];

/* DKR include/sched.h */
#define OS_SC_NEEDS_RSP 0x0001
#define OS_SC_NEEDS_RDP 0x0002
#define OS_SC_LAST_TASK 0x0020

/* DKR src/rcp_dkr.h */
typedef struct DKR_OSTask {
    /* 0x00 */ struct DKR_OSTask *next;
    /* 0x04 */ u32 state;
    /* 0x08 */ u32 flags;
    /* 0x0C */ void *frameBuffer;
    /* 0x10 */ OSTask_t task;
    /* 0x50 */ OSMesgQueue *mesgQueue;
    /* 0x54 */ OSMesg mesg;
    /* 0x58 */ s32 unused58;
    /* 0x5C */ s32 unused5C;
    /* 0x60 */ s32 unused60;
    /* 0x64 */ s32 unused64;
    /* 0x68 */ u32 unk68;
    /* 0x6C */ s32 unused6C;
} DKR_OSTask;

extern s32 gGfxTaskIsRunning;
extern OSMesgQueue gGfxTaskMesgQueue;
extern DKR_OSTask gGfxTaskBuf[2];
extern s32 gGfxBufCounter;
// JFG: three more task types, each with its own pair of tasks, counter, queue and message
extern DKR_OSTask D_800FE600[2];
extern DKR_OSTask D_800FE6E0[2];
extern DKR_OSTask D_800FE7C0[2];
extern s32 D_800A35D8;
extern s32 D_800A35DC;
extern s32 D_800A35E0;
extern OSMesgQueue D_800FE8A8;
extern OSMesgQueue D_800FE8E0;
extern OSMesgQueue D_800FE918;
extern s16 D_800A35EC[];
extern s16 D_800A3610[];
extern s16 D_800A3634[];
extern u8 gDramStack[];
extern u8 gGfxTaskYieldData[];
extern u8 rspF3DDKRXbusStart[];
extern u8 rspF3DDKRDataXbusStart[];
extern OSMesgQueue *osScInterruptQ;

/**
 * Prepare the gfx task for the F3DDKR XBus microcode.
 * Sends a message to the scheduler to start processing an RSP task once set up.
 * JFG: four task types (0 is the main one; 3-5 have their own queues), and the
 * framebuffer is an argument.
 * Official Name: rcpFast3d
 */
s32 gfxtask_run_xbus(Gfx *dlBegin, Gfx *dlEnd, s32 type, void *frameBuffer) {
    DKR_OSTask *dkrtask;
    s32 flags;

    gGfxTaskIsRunning = TRUE;
    flags = OS_SC_NEEDS_RDP | OS_SC_NEEDS_RSP;
    switch (type) {
        case 0:
            dkrtask = &gGfxTaskBuf[gGfxBufCounter];
            gGfxBufCounter ^= 1;
            flags |= OS_SC_LAST_TASK;
            dkrtask->mesgQueue = &gGfxTaskMesgQueue;
            dkrtask->unused58 = COLOUR_TAG_RED;
            dkrtask->unused5C = COLOUR_TAG_RED;
            dkrtask->unused6C = 2;
            break;
        case 3:
            dkrtask = &D_800FE600[D_800A35D8];
            D_800A35D8 ^= 1;
            dkrtask->mesgQueue = &D_800FE8A8;
            dkrtask->mesg = D_800A35EC;
            dkrtask->unused58 = COLOUR_TAG_MAGENTA;
            dkrtask->unused5C = COLOUR_TAG_MAGENTA;
            dkrtask->unused6C = 5;
            break;
        case 4:
            dkrtask = &D_800FE6E0[D_800A35DC];
            D_800A35DC ^= 1;
            dkrtask->mesgQueue = &D_800FE8E0;
            dkrtask->mesg = D_800A3610;
            dkrtask->unused58 = COLOUR_TAG_YELLOW;
            dkrtask->unused5C = COLOUR_TAG_YELLOW;
            dkrtask->unused6C = 6;
            break;
        case 5:
            dkrtask = &D_800FE7C0[D_800A35E0];
            D_800A35E0 ^= 1;
            dkrtask->mesgQueue = &D_800FE918;
            dkrtask->mesg = D_800A3634;
            dkrtask->unused58 = COLOUR_TAG_GREEN;
            dkrtask->unused5C = COLOUR_TAG_GREEN;
            dkrtask->unused6C = 7;
            break;
    }
    dkrtask->unk68 = FALSE;
    dkrtask->task.dram_stack = (u64 *) gDramStack;
    dkrtask->task.dram_stack_size = 0x400;
    dkrtask->task.yield_data_ptr = (u64 *) gGfxTaskYieldData;
    dkrtask->task.yield_data_size = 0xA00;
    dkrtask->flags = flags;
    dkrtask->task.data_ptr = (u64 *) dlBegin;
    dkrtask->task.data_size = dlEnd - dlBegin;
    dkrtask->task.type = M_GFXTASK;
    dkrtask->task.flags = OS_TASK_DP_WAIT;
    dkrtask->task.ucode_boot = (u64 *) rspbootTextStart;
    dkrtask->task.ucode_boot_size = (s32) ((s32) rspbootTextEnd - (s32) rspbootTextStart);
    dkrtask->task.ucode = (u64 *) rspF3DDKRXbusStart;
    dkrtask->task.ucode_data = (u64 *) rspF3DDKRDataXbusStart;
    dkrtask->task.ucode_data_size = 0x800;
    dkrtask->task.output_buff = NULL;
    dkrtask->task.output_buff_size = NULL;
    dkrtask->next = NULL;
    dkrtask->frameBuffer = frameBuffer;
    dkrtask->unused60 = COLOUR_TAG_BLACK;
    dkrtask->unused64 = COLOUR_TAG_BLACK;
    osWritebackDCacheAll();
    osSendMesg(osScInterruptQ, dkrtask, OS_MESG_BLOCK);
    return 0;
}

extern s32 D_800A3630; // JFG: a second RSP task is running
extern s32 D_800A360C; // JFG: a third RSP task is running
extern s32 D_800A35E8;
void func_80054530(void); // a module function (patched in by the runtime linker)

/**
 * Called from the main game loop, will halt until a message comes through saying the graphics task
 * has finished.
 * Alternatively, if no task is active, then it will just skip.
 * JFG: also waits for up to three other tasks.
 * Official Name: rcpWaitDP
 */
s32 gfxtask_wait(void) {
    OSMesg *mesg = NULL;
    OSMesg mesg2 = NULL;
    OSMesg mesg3 = NULL;
    if (gGfxTaskIsRunning == FALSE) {
        return 0;
    }
    osRecvMesg(&gGfxTaskMesgQueue, (OSMesg) &mesg, OS_MESG_BLOCK);
    if (D_800A3630) {
        osRecvMesg(&D_800FE918, &mesg3, OS_MESG_BLOCK);
        D_800A3630 = FALSE;
    }
    if (D_800A360C) {
        osRecvMesg(&D_800FE8E0, &mesg2, OS_MESG_BLOCK);
        D_800A360C = FALSE;
    }
    if (D_800A35E8) {
        func_80054530();
        D_800A35E8 = FALSE;
    }
    gGfxTaskIsRunning = FALSE;
    return (s32) mesg[1];
}

/**
 * Sets the primitive colour for the cyclemode fillrect background.
 * Official name: rcpSetScreenColour
 */
void bgdraw_primcolour(u8 red, u8 green, u8 blue) {
    sBGPrimColourR = red;
    sBGPrimColourG = green;
    sBGPrimColourB = blue;
}

/**
 * Sets the fill colour for the fillmode fillrect background.
 * Uses RGBA5551
 * Official name: rcpSetBorderColour
 */
void bgdraw_fillcolour(s32 red, s32 green, s32 blue) {
    sBackgroundFillColour = GPACK_RGBA5551(red, green, blue, 1);
    sBackgroundFillColour |= (sBackgroundFillColour << 16);
}

extern s32 gVideoDepthBuffer;

/*
 * JFG: clears a rectangle of the depth buffer: fill mode, the depth buffer as the colour image, the
 * farthest depth as the fill colour, then the colour image back (needs a depth buffer).
 */
void zbuffer_clear_rect(Gfx **dList, u32 width, u32 height, s32 ulx, s32 uly, s32 lrx, s32 lry) {
    Gfx *gfx;

    if (gVideoDepthBuffer && ulx < lrx && uly < lry) {
        gfx = *dList;
        ulx &= ~3;
        lrx = (lrx + 3) & ~3;
        gDPPipeSync(gfx++);
        gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, width - 1, height - 1);
        gDPSetOtherMode(gfx++, G_CYC_FILL | 0xF, 0);
        gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, 0x02000000);
        gDPSetFillColor(gfx++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16) | GPACK_ZDZ(G_MAXFBZ, 0));
        gDPFillRectangle(gfx++, ulx, uly, lrx, lry);
        gDPPipeSync(gfx++);
        gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, 0x01000000);
        *dList = gfx;
    }
}

void fb_size(u32 *width, u32 *height);
extern s32 gChequerBGEnabled;
extern struct RectTexture *gTexBGTex1;
extern void (*gBGDrawFunc)(Gfx **, Mtx **);
s32 check_viewport_background_flag(s32 viewPortIndex);
s32 copy_viewport_background_size_to_coords(s32 viewPortIndex, s32 *x1, s32 *y1, s32 *x2, s32 *y2);
void viewport_scissor(Gfx **dList);
void bgdraw_chequer(Gfx **dList);
void bgdraw_texture(Gfx **dList);

/**
 * Clears the ZBuffer first, then decides how to draw the background which goes directly
 * over the colour buffer. DrawBG if set to 0 (which never happens) will completely skip
 * over clearing the colour buffer.
 * JFG: the depth buffer clear is its own function (zbuffer_clear_rect).
 * Official Name: rcpClearScreen
 */
void bgdraw_render(Gfx **dList, Mtx **mtx, s32 drawBG) {
    u32 w;
    u32 h;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    fb_size(&w, &h);
    zbuffer_clear_rect(dList, w, h, 0, 0, w, h);
    if (drawBG) {
        if (check_viewport_background_flag(0)) {
            if (gChequerBGEnabled) {
                bgdraw_chequer(dList); // Unused
            } else if (gTexBGTex1) {
                bgdraw_texture(dList);
            } else if (gBGDrawFunc != NULL) {
                gBGDrawFunc(dList, mtx);
            } else {
                gDPSetFillColor((*dList)++, sBackgroundFillColour);
                gDPFillRectangle((*dList)++, 0, 0, w - 1, h - 1);
            }
            // Used for secondary viewport backgrounds.
            if (copy_viewport_background_size_to_coords(0, &x1, &y1, &x2, &y2)) {
                gDPSetCycleType((*dList)++, G_CYC_1CYCLE);
                gDPSetPrimColor((*dList)++, 0, 0, sBGPrimColourR, sBGPrimColourG, sBGPrimColourB, 255);
                gDPSetCombineMode((*dList)++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
                gDPSetRenderMode((*dList)++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
                gDPFillRectangle((*dList)++, x1, y1, x2, y2);
            }
        } else {
            if (gChequerBGEnabled) {
                bgdraw_chequer(dList); // Unused
            } else if (gTexBGTex1) {
                bgdraw_texture(dList);
            } else if (gBGDrawFunc != NULL) {
                gBGDrawFunc(dList, mtx);
            } else {
                gDPSetFillColor((*dList)++, (GPACK_RGBA5551(sBGPrimColourR, sBGPrimColourG, sBGPrimColourB, 1) << 16) |
                                                GPACK_RGBA5551(sBGPrimColourR, sBGPrimColourG, sBGPrimColourB, 1));
                gDPFillRectangle((*dList)++, 0, 0, w - 1, h - 1);
            }
        }
    }
    gDPPipeSync((*dList)++);
    viewport_scissor(dList);
}

extern Gfx dRdpInit[];

/**
 * Sets the colour and depth image segments, then the RDP's default modes.
 * JFG: a pipe sync first, and the size through pointers.
 */
void rdp_init(Gfx **dList) {
    u32 width;
    u32 height;

    fb_size(&width, &height);
    gDPPipeSync((*dList)++);
    gDPSetColorImage((*dList)++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, 0x01000000);
    gDPSetDepthImage((*dList)++, 0x02000000);
    gSPDisplayList((*dList)++, dRdpInit);
}

/* Calls the display list that sets the RDP's default modes (0x8004E3E8). */
void rdp_set_default_modes(Gfx **dList) {
    gSPDisplayList((*dList)++, dRdpInit);
}

/**
 * Calls the draw command that sets all the OtherModes, ready for use.
 * Official name: rcpInitSp or rcpInitDpNoSize
 */
void rsp_init(Gfx **dList) {
    gSPDisplayList((*dList)++, dRspInit);
}

typedef struct OSSched OSSched;
OSMesgQueue *osScGetInterruptQ(OSSched *sc);
extern OSMesgQueue gRCPMesgQueue;
extern OSMesg gRCPMesgBuf;
extern OSMesgQueue gUnusedMesgQueue;
extern OSMesg gUnusedMesgBuf[8];
extern OSMesg gGfxTaskMesgBuf[8];
extern OSMesg D_800FE8C0[8];
extern OSMesg D_800FE8F8[8];
extern OSMesg D_800FE930[8];

/**
 * Initialise the mesg queues for the gfx tasks, that interface with the scheduler.
 * JFG: and one for each of the three other task types.
 */
void gfxtask_init(OSSched *sc) {
    osScInterruptQ = osScGetInterruptQ(sc);
    osCreateMesgQueue(&gRCPMesgQueue, &gRCPMesgBuf, 1);
    osCreateMesgQueue(&gUnusedMesgQueue, gUnusedMesgBuf, ARRAY_COUNT(gUnusedMesgBuf));
    osCreateMesgQueue(&gGfxTaskMesgQueue, gGfxTaskMesgBuf, ARRAY_COUNT(gGfxTaskMesgBuf));
    osCreateMesgQueue(&D_800FE8A8, D_800FE8C0, ARRAY_COUNT(D_800FE8C0));
    osCreateMesgQueue(&D_800FE8E0, D_800FE8F8, ARRAY_COUNT(D_800FE8F8));
    osCreateMesgQueue(&D_800FE918, D_800FE930, ARRAY_COUNT(D_800FE930));
}

/* A texture's header (DKR's TextureHeader, structs.h), the fields used here */
typedef struct RectTexture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0xA - 0x2];
    /* 0x0A */ s16 numberOfCommands;
    /* 0x0C */ Gfx *cmd;
} RectTexture;

extern RectTexture *gTexBGTex2;
extern s32 gTexBGShiftX;
extern Gfx dRaceFinishBackgroundSettings[];

/* DKR include/f3ddkr.h: G_DMADL (7) runs numberOfCommands commands from address. */
#define gDkrDmaDisplayList(pkt, address, numberOfCommands)                                           \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(0x07, 24, 8) | _SHIFTL((numberOfCommands), 16, 8) |                 \
                        _SHIFTL((numberOfCommands * 8), 0, 16));                                     \
        _g->words.w1 = (unsigned int) (address);                                                     \
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

/**
 * Seems to render the background screen after a race finishes while you're at the menu deciding what to do next.
 * JFG: the size through pointers (so the shifted width and height live on the stack), unsigned compares.
 * Official Name: rcpMosaicClear
 */
void bgdraw_texture(Gfx **dList) {
    // The following variables use the (30.2) fixed-point format
    u32 sumTextureHeights;
    u32 texWidth;
    u32 texHeight;
    s32 xOffset;
    u32 uly;
    s32 ulx;
    u32 videoWidth;
    u32 videoHeight;

    fb_size(&videoWidth, &videoHeight);
    gSPDisplayList((*dList)++, dRaceFinishBackgroundSettings);

    if (gTexBGTex2 == NULL) {
        // Fill the background with a single texture
        gDkrDmaDisplayList((*dList)++, OS_K0_TO_PHYSICAL(gTexBGTex1->cmd), gTexBGTex1->numberOfCommands);
        videoWidth <<= 2;
        videoHeight <<= 2;
        texWidth = gTexBGTex1->width << 2;
        texHeight = gTexBGTex1->height << 2;
        for (xOffset = 0, uly = 0; uly < videoHeight;
             uly += texHeight, xOffset = (xOffset + gTexBGShiftX) & (texWidth - 1)) {
            for (ulx = -xOffset; ulx < videoWidth; ulx += texWidth) {
                if (ulx < 0) {
                    gSPTextureRectangleOld((*dList)++, 0, uly, ulx + texWidth, uly + texHeight, G_TX_RENDERTILE,
                                           -(ulx << 3), 0, 1024, 1024);
                } else {
                    gSPTextureRectangleOld((*dList)++, ulx, uly, ulx + texWidth, uly + texHeight, G_TX_RENDERTILE,
                                           0, 0, 1024, 1024);
                }
            }
        }
    } else {
        // The screen is filled with alternating horizontal stripes of two textures
        gDkrDmaDisplayList((*dList)++, OS_K0_TO_PHYSICAL(gTexBGTex1->cmd), gTexBGTex1->numberOfCommands);
        videoWidth <<= 2;
        videoHeight <<= 2;
        uly = 0;
        texWidth = gTexBGTex1->width << 2;
        texHeight = gTexBGTex1->height << 2;
        sumTextureHeights = (gTexBGTex2->height << 2) + texHeight;
        for (xOffset = 0; uly < videoHeight;
             uly += sumTextureHeights, xOffset = (xOffset + gTexBGShiftX) & (texWidth - 1)) {
            for (ulx = -xOffset; ulx < videoWidth; ulx += texWidth) {
                if (ulx < 0) {
                    gSPTextureRectangleOld((*dList)++, 0, uly, ulx + texWidth, uly + texHeight, G_TX_RENDERTILE,
                                           -(ulx << 3), 0, 1024, 1024);
                } else {
                    gSPTextureRectangleOld((*dList)++, ulx, uly, ulx + texWidth, uly + texHeight, G_TX_RENDERTILE,
                                           0, 0, 1024, 1024);
                }
            }
        }

        gDkrDmaDisplayList((*dList)++, OS_K0_TO_PHYSICAL(gTexBGTex2->cmd), gTexBGTex2->numberOfCommands);
        // Width and height are mistakenly multiplied by 4 again, but it doesn't cause any issues
        videoWidth <<= 2;
        videoHeight <<= 2;

        uly = texHeight;
        texHeight = gTexBGTex2->height << 2;
        for (xOffset = 0; uly < videoHeight;
             uly += sumTextureHeights, xOffset = (xOffset + gTexBGShiftX) & (texWidth - 1)) {
            for (ulx = -xOffset; ulx < videoWidth; ulx += texWidth) {
                if (ulx < 0) {
                    gSPTextureRectangleOld((*dList)++, 0, uly, ulx + texWidth, uly + texHeight, G_TX_RENDERTILE,
                                           -(ulx << 3), 0, 1024, 1024);
                } else {
                    gSPTextureRectangleOld((*dList)++, ulx, uly, ulx + texWidth, uly + texHeight, G_TX_RENDERTILE,
                                           0, 0, 1024, 1024);
                }
            }
        }
    }
    gDPPipeSync((*dList)++);
}

extern Gfx dChequerBGSettings[];
extern u8 gChequerBGColourR1;
extern u8 gChequerBGColourG1;
extern u8 gChequerBGColourB1;
extern u8 gChequerBGColourA1;
extern u8 gChequerBGColourR2;
extern u8 gChequerBGColourG2;
extern u8 gChequerBGColourB2;
extern u8 gChequerBGColourA2;
extern u32 gChequerBGWidth;
extern u32 gChequerBGHeight;

/**
 * Uses global chequerboard settings to render a background using two different alternating colours.
 * JFG: gets the size through pointers, and works in unsigned values.
 * Official Name: rcpCheckClear
 */
void bgdraw_chequer(Gfx **dList) {
    s32 flip; // Flips between 0 and 1
    u32 y;
    u32 x;
    u32 width;
    u32 height;

    fb_size(&width, &height);
    gSPDisplayList((*dList)++, dChequerBGSettings);
    gDPSetPrimColor((*dList)++, 0, 0, gChequerBGColourR1, gChequerBGColourG1, gChequerBGColourB1, gChequerBGColourA1);

    for (y = 0, flip = 0; y < height; y += gChequerBGHeight, flip ^= 1) {
        for (x = flip * gChequerBGWidth; x < width; x += gChequerBGWidth * 2) {
            gDPFillRectangle((*dList)++, x, y, x + gChequerBGWidth, y + gChequerBGHeight);
        }
    }

    gDPSetPrimColor((*dList)++, 0, 0, gChequerBGColourR2, gChequerBGColourG2, gChequerBGColourB2, gChequerBGColourA2);

    for (y = 0, flip = 1; y < height; y += gChequerBGHeight, flip ^= 1) {
        for (x = flip * gChequerBGWidth; x < width; x += gChequerBGWidth * 2) {
            gDPFillRectangle((*dList)++, x, y, x + gChequerBGWidth, y + gChequerBGHeight);
        }
    }

    gDPPipeSync((*dList)++);
}

/* One textured rectangle of a list (ended by a NULL texture) */
typedef struct TexturedRect {
    /* 0x00 */ RectTexture *tex;
    /* 0x04 */ s32 frame;
    /* 0x08 */ s16 x;
    /* 0x0A */ s16 y;
} TexturedRect;

RectTexture *set_animated_texture_header(RectTexture *texHead, s32 offset);
extern Gfx D_800A37A8[];

#ifdef NON_MATCHING
/* NON_MATCHING: the texture pointer comes through v0 (the ROM loads it into s1 for the loop test) */
/*
 * JFG: draws a list of textured rectangles at (x, y) plus each one's offset, in a primitive colour,
 * clipped at the screen's top and left edges.
 */
void func_8004EDBC(Gfx **dList, TexturedRect *rects, s32 x, s32 y, u8 red, u8 green, u8 blue, u8 alpha) {
    Gfx *gfx;
    RectTexture *tex;
    s32 xl;
    s32 yl;
    s32 xh;
    s32 yh;
    s32 s;
    s32 t;
    s32 count;

    gfx = *dList;
    gSPDisplayList(gfx++, D_800A37A8);
    gDPSetPrimColor(gfx++, 0, 0, red, green, blue, alpha);
    x <<= 2;
    y <<= 2;
    for (; rects->tex != NULL; rects++) {
        tex = rects->tex;
        xl = (rects->x << 2) + x;
        yl = (rects->y << 2) + y;
        xh = (tex->width << 2) + xl;
        yh = (tex->height << 2) + yl;
        if (xh > 0 && yh > 0) {
            s = 0;
            t = 0;
            if (xl < 0) {
                s = -(xl << 3);
                xl = 0;
            }
            if (yl < 0) {
                t = -(yl << 3);
                yl = 0;
            }
            gfx->words.w0 = tex->cmd->words.w0;
            gfx->words.w1 = (u32) set_animated_texture_header(tex, rects->frame) + 0x80000000;
            gfx++;
            count = tex->numberOfCommands - 1;
            gDkrDmaDisplayList(gfx++, (u32) (tex->cmd + 1) + 0x80000000, count);
            gSPTextureRectangleOld(gfx++, xl, yl, xh, yh, 0, s, t, 0x400, 0x400);
        }
    }
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, 255);
    *dList = gfx;
}
#else
INCLUDE_ASM("asm/nonmatchings/4E610", func_8004EDBC);
#endif

/* DKR include/macros.h */
#define gDPSetPrimColorRGBA(pkt, rgba)                                                                \
    {                                                                                                \
        Gfx *_g = (Gfx *) (pkt);                                                                     \
        _g->words.w0 = (_SHIFTL(G_SETPRIMCOLOR, 24, 8));                                             \
        _g->words.w1 = (rgba);                                                                       \
    }

extern Gfx D_800A37F0[];    // DKR: dScaledRectangleBaseModes
extern Gfx D_800A3828[][2]; // DKR: dTextureRectangleScaledOpa
extern Gfx D_800A3868[][2]; // DKR: dTextureRectangleScaledXlu

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words short: the ROM keeps a dead `move a2, zero` before the loop (and so one register fewer,
 * where mine keeps 0x80000000 in $ra); otherwise the same code. */
/*
 * Renders one or more textures directly on screen, scaled, in a primitive colour (flags 0x1000 and 0x2000 flip
 * them in x and y).
 * JFG: the size through pointers; each texture's first command is its own frame's.
 * DKR: texrect_draw_scaled
 */
void func_8004F02C(Gfx **dList, TexturedRect *element, f32 xPos, f32 yPos, f32 xScale, f32 yScale, u32 colour,
                   s32 flags) {
    RectTexture *tex;
    Gfx *dmaDlist;
    s32 i;
    s32 bFlipX;
    s32 bFlipY;
    s32 s;
    s32 width = 0;
    s32 height = 0;
    s32 t;
    s32 dsdx;
    s32 dtdy;
    s32 lrx; // the ROM reads lrx and lry before they are set (unused there): see the loop
    s32 lry;
    s32 ulx;
    s32 uly;
    s32 xPos4x;
    s32 yPos4x;
    Gfx *gfx;
    s32 count;

    gfx = *dList;
    fb_size(&width, &height);
    height *= 4;
    width *= 4;
    if ((colour & 0xFF) == 0xFF) {
        dmaDlist = D_800A3828[flags & 0xFF];
    } else {
        dmaDlist = D_800A3868[flags & 0xFF];
    }
    gSPDisplayList(gfx++, D_800A37F0);
    gDkrDmaDisplayList(gfx++, OS_K0_TO_PHYSICAL(dmaDlist), 2);
    gDPSetPrimColorRGBA(gfx++, colour);
    bFlipX = flags & 0x1000;
    bFlipY = flags & 0x2000;
    xScale *= 4;
    yScale *= 4;
    xPos4x = xPos * 4;
    yPos4x = yPos * 4;
    for (i = 0; (tex = element[i].tex); i++) {
        if (!bFlipX) {
            ulx = (s32) (element[i].x * xScale) + xPos4x;
        } else {
            lrx = xPos4x - (s32) (element[i].x * xScale);
            ulx = lrx - (s32) (tex->width * xScale);
        }
        if (!bFlipY) {
            uly = (s32) (element[i].y * yScale) + yPos4x;
        } else {
            lry = yPos4x - (s32) (element[i].y * yScale);
            uly = lry - (s32) (tex->height * yScale);
        }
        if (ulx < width && uly < height) {
            if (!bFlipX) {
                lrx = (s32) (tex->width * xScale) + ulx;
            }
            if (!bFlipY) {
                lry = (s32) (tex->height * yScale) + uly;
            }
            if (lrx > 0 && lry > 0 && ulx < lrx && uly < lry) {
                dsdx = ((tex->width - 1) << 12) / (lrx - ulx);
                if (bFlipX) {
                    s = (tex->width - 1) << 5;
                    dsdx = -dsdx;
                } else {
                    s = 0;
                }
                dtdy = ((tex->height - 1) << 12) / (lry - uly);
                if (bFlipY) {
                    t = (tex->height - 1) << 5;
                    dtdy = -dtdy;
                } else {
                    t = 0;
                }
                if (ulx < 0) {
                    s += (-ulx * dsdx) >> 7;
                    ulx = 0;
                }
                if (uly < 0) {
                    t += (-uly * dtdy) >> 7;
                    uly = 0;
                }
                dmaDlist = tex->cmd;
                gfx->words.w0 = dmaDlist->words.w0;
                gfx->words.w1 = (u32) set_animated_texture_header(tex, element[i].frame) + 0x80000000;
                gfx++;
                count = tex->numberOfCommands - 1;
                dmaDlist++;
                gDkrDmaDisplayList(gfx++, (u32) dmaDlist + 0x80000000, count);
                gSPTextureRectangleOld(gfx++, ulx, uly, lrx, lry, 0, s, t, dsdx, dtdy);
            }
        }
    }
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, 255);
    *dList = gfx;
}
#else
INCLUDE_ASM("asm/nonmatchings/4E610", func_8004F02C);
#endif
