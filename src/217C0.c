/*
 * Lights. Functions in C here are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/lights.c; see CREDITS.md.
 */
#include "common.h"

/* DKR include/structs.h, reduced to the fields used here (JFG offsets) */
typedef struct ObjectLight {
    /* 0x00 */ u8 pad0[3];
    /* 0x03 */ u8 enabled; // bit 0
} ObjectLight;

typedef struct ObjectHeader {
    /* 0x00 */ u8 pad0[0x25];
    /* 0x25 */ s8 numLightSources;
    /* 0x26 */ u8 pad26[0x48 - 0x26];
    /* 0x48 */ struct ObjectHeaderLight { u8 pad[0x18]; } *lights;
} ObjectHeader;

typedef struct Object {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x74 - 0x44];
    /* 0x74 */ ObjectLight **lightData;
} Object;

void light_update(ObjectLight *light, s32 updateRate);
ObjectLight *light_add_from_object_header(Object *obj, struct ObjectHeaderLight *lightHeader);

extern ObjectLight **gActiveLights;
extern s32 gNumActiveLights;

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020BC0);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020C50);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020D84);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020E3C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800210A4);

/**
 * Disable this objects light data.
 * Official Name: turnLightOff?
 */
void light_disable(ObjectLight *light) {
    light->enabled &= ~TRUE;
}

/**
 * Enable this objects light data.
 * Official Name: turnLightOn?
 */
void light_enable(ObjectLight *light) {
    light->enabled |= TRUE;
}

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

/**
 * Return the active lights, and write how many there are.
 * JFG: DKR's light_count only returns the count.
 * Official Name: lightGetLights?
 */
ObjectLight **lightGetLights(s32 *numLights) {
    *numLights = gNumActiveLights;
    return gActiveLights;
}

INCLUDE_ASM("asm/nonmatchings/217C0", func_800218C4);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021A30);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021B8C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022028);

INCLUDE_ASM("asm/nonmatchings/217C0", func_8002215C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800221F0);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800224D0);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022520);

/**
 * Official Name: lightSetupLightSources
 */
void light_setup_light_sources(Object *obj) {
    s32 i;
    for (i = 0; i < obj->header->numLightSources; i++) {
        obj->lightData[i] = light_add_from_object_header(obj, &obj->header->lights[i]);
    }
}

INCLUDE_ASM("asm/nonmatchings/217C0", func_800226B8);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022744);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800228BC);

INCLUDE_ASM("asm/nonmatchings/217C0", func_800229B8);
