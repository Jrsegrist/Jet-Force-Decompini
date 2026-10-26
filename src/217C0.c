/*
 * Lights. Functions in C here are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/lights.c; see CREDITS.md.
 */
#include "common.h"

typedef struct ObjectLight ObjectLight;

void light_update(ObjectLight *light, s32 updateRate);

extern ObjectLight **gActiveLights;
extern s32 gNumActiveLights;

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020BC0);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020C50);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020D84);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020E3C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800210A4);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800212D0);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800212E4);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800212F8);

INCLUDE_ASM("asm/nonmatchings/217C0", func_8002130C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021338);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021358);

/**
 * Loops through all active lights and updates their properties.
 * Official Name: lightUpdateLights
 */
void light_update_all(s32 updateRate) {
    s32 i;
    for (i = 0; i < gNumActiveLights; i++) {
        light_update(gActiveLights[i], updateRate);
    }
}

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021434);

INCLUDE_ASM("asm/nonmatchings/217C0", func_8002179C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800218A8);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800218C4);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021A30);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021B8C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022028);

INCLUDE_ASM("asm/nonmatchings/217C0", func_8002215C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800221F0);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800224D0);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022520);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022624);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800226B8);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022744);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800228BC);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800229B8);
