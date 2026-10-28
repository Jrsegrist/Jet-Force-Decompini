#include "common.h"

s32 func_80054530(); // a module function (patched in by the runtime linker)
u8 func_80048548(void); // the number of cameras
s32 fb_mode(void);
void fb_size(u32 *width, u32 *height);
s32 func_8005AA7C(void); // the two-player screen is split vertically
void rendermode_reset(Gfx **dList);
extern Gfx D_800A3540[];

/*
 * JFG: draws the black bars between the split-screen views: a horizontal one across the middle for
 * three or four views, or two views one above the other, and a vertical one for three or four views,
 * or two side by side. Each bar is a transparent 4-pixel band with a solid 2-pixel line in it.
 */
void splitscreen_dividers_draw(Gfx **dList) {
    s32 numViews;
    s32 mode;
    u32 width;
    u32 height;
    u32 halfWidth;
    u32 halfHeight;
    Gfx *gfx;

    if (func_80054530()) {
        numViews = func_80048548();
        mode = fb_mode();
        if (numViews >= 2) {
            gfx = *dList;
            fb_size(&width, &height);
            halfWidth = width >> 1;
            halfHeight = height >> 1;
            gSPDisplayList(gfx++, D_800A3540);
            if (numViews >= 3 || (numViews == 2 && !(mode & 1) && !func_8005AA7C())) {
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, 0);
                gDPFillRectangle(gfx++, 0, halfHeight - 2, width, halfHeight + 2);
                gDPPipeSync(gfx++);
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, 255);
                gDPFillRectangle(gfx++, 0, halfHeight - 1, width, halfHeight + 1);
                gDPPipeSync(gfx++);
            }
            if (numViews >= 3 || (numViews == 2 && ((mode & 1) || func_8005AA7C()))) {
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, 0);
                gDPFillRectangle(gfx++, halfWidth - 2, 0, halfWidth + 2, height);
                gDPPipeSync(gfx++);
                gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, 255);
                gDPFillRectangle(gfx++, halfWidth - 1, 0, halfWidth + 1, height);
                gDPPipeSync(gfx++);
            }
            *dList = gfx;
            rendermode_reset(dList);
        }
    }
}
