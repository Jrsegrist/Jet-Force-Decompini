/* Runlink module 38: draws the dark full-screen backdrop behind the pause menu. */
#include "module.h"
#include "gfx.h"

extern Gfx *D_800FF398; /* the display list being built */
extern void *D_800FF39C;

extern void func_80040D64(Gfx **gfx, void **mtx);     /* sets an orthographic view */
extern void func_80055B80(Gfx **gfx);                 /* resets the render settings */
extern void func_80055C00(Gfx **gfx, void *tex, s32 flags, s32 offset); /* sets a texture (none here) */

static Vertex D_m038_000F0[4] = {
    { -160, -120, 0, 0, 0, 0x00, 0xFF },
    { 160, -120, 0, 0, 0, 0x00, 0xFF },
    { -160, 120, 0, 0, 0, 0x50, 0xFF },
    { 160, 120, 0, 0, 0, 0x50, 0xFF },
};

static Triangle D_m038_00118[2] = {
    { 0x40, 0, 1, 2, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 2, 3, 1, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};

void func_m038_00000(void) {
    func_80040D64(&D_800FF398, &D_800FF39C);
    func_80055B80(&D_800FF398);
    func_80055C00(&D_800FF398, NULL, 0, 0);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(D_800FF398++, 0, 0, 0, 255);
    gSPVertexJFG(D_800FF398++, OS_K0_TO_PHYSICAL(D_m038_000F0), 5, 0);
    gSPPolygon(D_800FF398++, OS_K0_TO_PHYSICAL(D_m038_00118), 2, 1);
}
