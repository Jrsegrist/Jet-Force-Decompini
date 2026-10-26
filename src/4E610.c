#include "common.h"

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern s32 sBackgroundFillColour;
extern u8 sBGPrimColourB;
extern u8 sBGPrimColourG;
extern u8 sBGPrimColourR;
extern Gfx dRspInit[];

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004DA10);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004DCB0);

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

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004DDEC);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004DFC4);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004E338);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004E3E8);

/**
 * Calls the draw command that sets all the OtherModes, ready for use.
 * Official name: rcpInitSp or rcpInitDpNoSize
 */
void rsp_init(Gfx **dList) {
    gSPDisplayList((*dList)++, dRspInit);
}

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004E430);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004E4E8);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004EB0C);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004EDBC);

INCLUDE_ASM("asm/nonmatchings/4E610", func_8004F02C);
