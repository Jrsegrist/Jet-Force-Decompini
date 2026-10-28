/*
 * Lights. Functions in C here are from the Diddy Kong Racing decompilation
 * (CC0 1.0), src/lights.c; see CREDITS.md.
 */
#include "common.h"

/* DKR include/structs.h, reduced to the fields used here (JFG offsets) */
typedef struct ObjectLight {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 flags;
    /* 0x03 */ u8 enabled; // bit 0
    /* 0x04 */ u16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 homeX;
    /* 0x0C */ f32 homeY;
    /* 0x10 */ f32 homeZ;
    /* 0x14 */ struct Object *owner;
    /* 0x18 */ f32 posX;
    /* 0x1C */ f32 posY;
    /* 0x20 */ f32 posZ;
    /* 0x24 */ f32 radius;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ f32 radiusSquare;
    /* 0x34 */ f32 radiusMag;
    /* 0x38 */ u8 pad38[0x40 - 0x38];
    /* 0x40 */ u8 colourR;
    /* 0x41 */ u8 colourG;
    /* 0x42 */ u8 colourB;
    /* 0x43 */ u8 intensity;
    /* 0x44 */ f32 intensityF;
    /* 0x48 */ u8 colourCycle[0xC]; // JFG: a ColourCycle (55130.c), its source at 0x54
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s16 unk58;
    /* 0x5A */ s16 unk5A;
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 unk5E;
    /* 0x60 */ u8 pad60[0x6C - 0x60];
    /* 0x6C */ s32 trackLight;
    /* 0x70 */ u8 *unk70; // JFG: 0x200 bytes of the light buffer
} ObjectLight;

/* Size: 0x18 bytes (DKR: ObjectHeader24) */
typedef struct ObjectHeaderLight {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ s16 homeX;
    /* 0x08 */ s16 homeY;
    /* 0x0A */ s16 homeZ;
    /* 0x0C */ u16 radius;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u8 colourR;
    /* 0x11 */ u8 colourG;
    /* 0x12 */ u8 colourB;
    /* 0x13 */ u8 intensity;
    /* 0x14 */ s16 colourCycleID;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
} ObjectHeaderLight;

typedef struct ObjectHeader {
    /* 0x00 */ u8 pad0[0x1E];
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ u8 pad1F[0x25 - 0x1F];
    /* 0x25 */ s8 numLightSources;
    /* 0x26 */ u8 numCoronas; // JFG
    /* 0x27 */ u8 pad27[0x48 - 0x27];
    /* 0x48 */ struct ObjectHeaderLight *lights;
    /* 0x4C */ struct ObjectHeaderCorona *coronas; // JFG
    /* 0x50 */ u8 pad50[0x90 - 0x50];
    /* 0x90 */ f32 shadeDiffuse;
    /* 0x94 */ f32 shadeAmbient;
    /* 0x98 */ u8 pad98[0x9C - 0x98];
    /* 0x9C */ s16 shadeAngleX;
    /* 0x9E */ s16 shadeAngleY;
    /* 0xA0 */ s16 shadeShift;
} ObjectHeader;

/* JFG: the description of one of an object's coronas (245D0.c). Size: 0x20 bytes */
typedef struct ObjectHeaderCorona {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 pad2[6];
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ u16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 unk1F;
} ObjectHeaderCorona;

/* JFG: the level entry of an object with a corona (corona_from_level_entry) */
typedef struct CoronaEntry {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 pad11[2];
    /* 0x13 */ u8 unk13;
} CoronaEntry;

typedef struct Object {
    /* 0x00 */ u8 pad0[0x3C];
    /* 0x3C */ CoronaEntry *entry;
    /* 0x40 */ ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x54 - 0x44];
    /* 0x54 */ struct ShadeProperties *shading;
    /* 0x58 */ u8 pad58[0x68 - 0x58];
    /* 0x68 */ void *unk68;
    /* 0x6C */ u8 pad6C[0x74 - 0x6C];
    /* 0x74 */ ObjectLight **lightData;
    /* 0x78 */ void **coronaData; // JFG
    /* 0x7C */ u8 pad7C[0xA1 - 0x7C];
    /* 0xA1 */ u8 unkA1;
} Object;

void light_update(ObjectLight *light, s32 updateRate);
ObjectLight *light_add_from_object_header(Object *obj, ObjectHeaderLight *lightHeader);

void mempool_free(void *data);

extern s32 gMaxLights;
extern s32 gNumActiveLights;
extern ObjectLight **gActiveLights;
extern ObjectLight *gActiveLightAttrs;
extern void *D_800A1010;
extern u8 *D_800F5B20; // JFG: 0x200 bytes per light, plus a table at the start

/**
 * Free all lights from RAM.
 * JFG: two more buffers of their own.
 * Official Name: freeLights
 */
void lights_free(void) {
    if (gActiveLights != NULL) {
        mempool_free(gActiveLights);
        gActiveLights = NULL;
        gActiveLightAttrs = NULL;
    }
    if (D_800F5B20 != NULL) {
        mempool_free(D_800F5B20);
        D_800F5B20 = NULL;
    }
    if (D_800A1010 != NULL) {
        mempool_free(D_800A1010);
        D_800A1010 = NULL;
    }
    gNumActiveLights = 0;
    gMaxLights = 0;
}

#define COLOUR_TAG_MAGENTA 0xFF00FFFF
void *mempool_alloc_safe(s32 size, u32 colourTag);
void func_80023170(s32 red, s32 green, s32 blue, u8 *table);

/**
 * Allocates RAM for all buffers containing light source properties.
 * JFG: each light also gets 0x200 bytes of a second buffer, which starts with a
 * table built here; no shade or direction buffers.
 * Official Name: setupLights
 */
void lights_init(s32 count, s32 arg1, s32 arg2) {
    s32 i;
    u8 *buffer;

    lights_free();
    gMaxLights = count;
    buffer = (u8 *) mempool_alloc_safe(gMaxLights * (sizeof(ObjectLight *) + sizeof(ObjectLight)), COLOUR_TAG_MAGENTA);
    D_800F5B20 = mempool_alloc_safe(gMaxLights * 0x200 + 0x200, COLOUR_TAG_MAGENTA);
    D_800A1010 = mempool_alloc_safe(0x240, COLOUR_TAG_MAGENTA);
    gActiveLightAttrs = (ObjectLight *) ((u32) buffer + gMaxLights * sizeof(ObjectLight *));
    gActiveLights = (ObjectLight **) buffer;
    for (i = 0; i < gMaxLights; i++) {
        gActiveLights[i] = &gActiveLightAttrs[i];
        gActiveLightAttrs[i].unk70 = D_800F5B20 + 0x200 + i * 0x200;
    }
    func_80023170(0xFF, 0xFF, 0xFF, D_800F5B20);
}

extern f32 D_800ABA90;
s32 func_80013E0C(f32 x, f32 y, f32 z, f32 outerRadius, f32 innerRadius, s32 red, s32 green, s32 blue);
/* Gives a light its track light (unless flag 0x40) (0x80020D84). */
void light_track_light_add(ObjectLight *light) {
    if (!(light->enabled & 0x40)) {
        light->trackLight = func_80013E0C(light->posX, light->posY, light->posZ, light->radius * 1.25f,
                                     light->radius * D_800ABA90, (light->colourR * light->intensity) >> 8,
                                     (light->colourG * light->intensity) >> 8, (light->colourB * light->intensity) >> 8);
    } else {
        light->trackLight = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/217C0", func_80020E3C);

void colour_cycle_init(void *cycle, s32 assetID); // sets up a colour cycle from an asset
void light_track_light_add(ObjectLight *light);

/**
 * Official Name: addObjectLight
 * JFG: its own light layout (a colour cycle, two radii).
 */
ObjectLight *light_add_from_object_header(Object *obj, ObjectHeaderLight *arg1) {
    ObjectLight *light;

    light = NULL;
    if (gNumActiveLights < gMaxLights) {
        light = gActiveLights[gNumActiveLights++];
        light->unk0 = arg1->unk0;
        light->unk1 = arg1->unk1;
        light->flags = 7;
        light->enabled = arg1->unk2;
        light->unk4 = arg1->unk4;
        light->unk6 = arg1->unk3;
        light->homeX = arg1->homeX;
        light->homeY = arg1->homeY;
        light->homeZ = arg1->homeZ;
        light->owner = obj;
        light->posX = light->homeX;
        light->posY = light->homeY;
        light->posZ = light->homeZ;
        light->radius = arg1->radius;
        light->unk28 = light->radius;
        light->unk2C = light->radius;
        if (light->unk0 == 2) {
            light->unk28 = arg1->unkE;
        }
        light->radiusSquare = light->radius * light->radius;
        light->radiusMag = 1.0f / light->radius;
        light->colourR = arg1->colourR;
        light->colourG = arg1->colourG;
        light->colourB = arg1->colourB;
        light->intensity = arg1->intensity;
        light->intensityF = arg1->intensity;
        if (arg1->colourCycleID != -1) {
            colour_cycle_init(light->colourCycle, arg1->colourCycleID);
        } else {
            light->unk54 = 0;
        }
        light->unk58 = arg1->unk16 << 8;
        light->unk5C = 0;
        light->unk5A = arg1->unk17 << 8;
        light->unk5E = 0;
        light_track_light_add(light);
        light_update(light, 0);
    }
    return light;
}

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

/* JFG: toggle this light (0x800212F8). */
void light_toggle(ObjectLight *light) {
    light->enabled ^= TRUE;
}

/* JFG: set a light's colour and flag it as changed (0x8002130C). */
void light_colour_set(ObjectLight *light, u8 red, u8 green, u8 blue) {
    light->colourR = red;
    light->colourG = green;
    light->colourB = blue;
    light->flags |= 2;
    light->unk54 = 0;
}

/* JFG: give the light a new colour cycle (0x80021338). */
void light_colour_cycle_set(ObjectLight *light, s32 assetID) {
    colour_cycle_init(light->colourCycle, assetID);
}

/* JFG: set a light's intensity, flagging it as changed when it is linked (0x80021358). */
void light_intensity_set(ObjectLight *light, u8 intensity) {
    light->intensity = intensity;
    light->intensityF = intensity;
    if (light->trackLight != 0) {
        light->flags |= 2;
    }
}

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

void track_light_off(s32 trackLight);

/**
 * JFG: also switches off the light's track light.
 * Official Name: killLight
 */
void light_remove(ObjectLight *light) {
    s32 i;
    ObjectLight *entry = NULL;
    for (i = 0; (i < gNumActiveLights) && (entry == NULL); i++) {
        if (light == gActiveLights[i]) {
            entry = gActiveLights[i];
        }
    }
    if (entry != NULL) {
        if (light->trackLight != 0) {
            track_light_off(light->trackLight);
        }
        gNumActiveLights--;
        for (i--; i < gNumActiveLights; i++) {
            gActiveLights[i] = gActiveLights[i + 1];
        }
        gActiveLights[gNumActiveLights] = entry;
    }
}

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

typedef struct LevelHeader217 {
    /* 0x00 */ u8 pad0[0x6B];
    /* 0x6B */ s8 unk6B;
    /* 0x6C */ u8 pad6C[0xD4 - 0x6C];
    /* 0xD4 */ s16 shadeAngleY;
    /* 0xD6 */ s16 shadeAngleX;
    /* 0xD8 */ u8 padD8[0xDB - 0xD8];
    /* 0xDB */ u8 shadeDiffuse;
    /* 0xDC */ u8 shadeAmbient;
    /* 0xDD */ u8 padDD[0xDF - 0xDD];
    /* 0xDF */ u8 shadeOverride; // JFG: use these instead of the object header's
} LevelHeader217;

Object **objGetObjList(s32 *arg0, s32 *cnt);
LevelHeader217 *level_header(void);
void func_80021B8C(Object *obj, void *arg1);
void func_80054530(); // a module function (patched in by the runtime linker)

#ifdef NON_MATCHING
/**
 * JFG: shade every 3D-model object that has shading (and unkA1 clear): func_80021B8C, or a module
 * function when the level header says so.
 * NON_MATCHING: i and count sit two words higher (target 0x24/0x20, with no temporaries below).
 */
void func_80021A30(void) {
    Object **objs;
    Object *obj;
    s32 i;
    s32 count;

    objs = objGetObjList(&i, &count);
    if (level_header()->unk6B) {
        while (i < count) {
            obj = objs[i++];
            if (obj->shading != NULL && obj->header->unk1E == 0 && obj->unkA1 == 0) {
                func_80054530(obj, obj->shading);
            }
        }
    } else {
        while (i < count) {
            obj = objs[i++];
            if (obj->shading != NULL && obj->header->unk1E == 0 && obj->unkA1 == 0) {
                func_80021B8C(obj, obj->shading);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/217C0", func_80021A30);
#endif

INCLUDE_ASM("asm/nonmatchings/217C0", func_80021B8C);

INCLUDE_ASM("asm/nonmatchings/217C0", func_80022028);

/**
 * JFG: the light's direction, the difference vector and the squared distance are
 * arguments, and the distance is not square rooted.
 * Official Name: lightDirectionCalc
 */
f32 light_direction_calc(f32 dirX, f32 dirY, f32 dirZ, f32 diffX, f32 diffY, f32 diffZ, f32 distance) {
    f32 invDist;
    f32 result;

    if (distance > 0.0f) {
        invDist = 1.0f / distance;
        result = (diffX * invDist * dirX) + (diffY * invDist * dirY) + (diffZ * invDist * dirZ);
        if (result < 0.0f) {
            result = 0.0f;
        }
    } else {
        result = 1.0f;
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/217C0", func_800221F0);

/* JFG: a directional light (D_800F5B28 is the default object light) */
typedef struct DirLight {
    /* 0x00 */ f32 dir[3];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ u8 shift;
    /* 0x15 */ u8 max;
    /* 0x16 */ u8 range;
    /* 0x17 */ u8 min;
    /* 0x18 */ u8 *table;
    /* 0x1C */ s16 angle1;
    /* 0x1E */ s16 angle0;
} DirLight;

/* JFG: an object's shading (Size: 0x90 bytes, 8 for sprites) */
typedef struct ShadeProperties {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 padB[0xE - 0xB];
    /* 0x0E */ s16 unkE;
    /* 0x10 */ DirLight light;
    /* 0x30 */ u8 pad30[0x90 - 0x30];
} ShadeProperties;

extern DirLight D_800F5B28; // JFG: the default object light
void func_80022520(DirLight *light, s32 a, s32 b, s16 angle0, s16 angle1, s32 shift);

/* JFG: set up the default object light. */
void func_800224D0(s32 a, s32 b, s16 c, s16 d, s32 e) {
    func_80022520(&D_800F5B28, a, b, c, d, e);
}

void func_800492DC(s16 *rotation, f32 *vec); // rotates a vector by three angles

#ifdef NON_MATCHING
/* JFG: set up a directional light: brightness range min(a, b)..b, a shift, and the direction from two angles.
   NON_MATCHING: 20 words: the range/shift computation is scheduled differently (store order 17, 15, 16, 10, 14). */
void func_80022520(DirLight *light, s32 a, s32 b, s16 angle0, s16 angle1, s32 shift) {
    s16 rotation[3];
    f32 dir[3];

    a &= 0xFF;
    b &= 0xFF;
    shift &= 7;
    if (b < a) {
        a = b;
    }
    light->min = a;
    light->max = b;
    light->range = b - a;
    light->shift = shift;
    light->unk10 = light->range << light->shift;
    light->unkC = (1 << (8 - shift)) << 6;
    light->angle0 = angle0;
    light->angle1 = angle1;
    rotation[1] = angle0;
    rotation[2] = 0;
    rotation[0] = angle1;
    dir[0] = 0.0f;
    dir[1] = 0.0f;
    dir[2] = 1.0f;
    func_800492DC(rotation, dir);
    light->dir[0] = dir[0];
    light->dir[1] = dir[1];
    light->dir[2] = dir[2];
    light->table = D_800F5B20;
}
#else
INCLUDE_ASM("asm/nonmatchings/217C0", func_80022520);
#endif

/**
 * Official Name: lightSetupLightSources
 */
void light_setup_light_sources(Object *obj) {
    s32 i;
    for (i = 0; i < obj->header->numLightSources; i++) {
        obj->lightData[i] = light_add_from_object_header(obj, &obj->header->lights[i]);
    }
}

void *func_80023A48(Object *obj, ObjectHeaderCorona *header);

/* JFG: the same loop as light_setup_light_sources, over the header's corona descriptions (0x800226B8). */
void light_setup_coronas(Object *obj) {
    s32 i;
    for (i = 0; i < obj->header->numCoronas; i++) {
        obj->coronaData[i] = func_80023A48(obj, &obj->header->coronas[i]);
    }
}

/**
 * Sets the shading properties of the object.
 * JFG: a directional light from the level header (when it overrides) or the object header.
 * DKR: init_object_shading
 */
s32 init_object_shading(Object *obj, ShadeProperties *shadeData) {
    s32 returnSize;
    LevelHeader217 *levelHeader;
    ObjectHeader *objHeader;

    levelHeader = level_header();
    returnSize = 0;
    objHeader = obj->header;
    obj->shading = shadeData;
    if (objHeader->unk1E == 0) {
        shadeData->unkE = 1;
        returnSize = sizeof(ShadeProperties);
        if (levelHeader->shadeOverride) {
            func_80022520(&shadeData->light, levelHeader->shadeAmbient, levelHeader->shadeDiffuse,
                          levelHeader->shadeAngleX, levelHeader->shadeAngleY + 0x8000, 0);
        } else {
            func_80022520(&shadeData->light, objHeader->shadeAmbient * 255.0f, objHeader->shadeDiffuse * 255.0f,
                          objHeader->shadeAngleX, objHeader->shadeAngleY, objHeader->shadeShift);
        }
        shadeData->unk4 = 255;
        shadeData->unk5 = 255;
        shadeData->unk6 = 255;
        shadeData->unk7 = 255;
        shadeData->unk8 = 255;
        shadeData->unk9 = 255;
        shadeData->unkA = 255;
        shadeData->unk0 = 1.0f;
    } else if (objHeader->unk1E == 1) {
        shadeData->unk4 = 255;
        shadeData->unk5 = 255;
        shadeData->unk6 = 255;
        shadeData->unk7 = 0;
        returnSize = 8;
        shadeData->unk0 = 1.0f;
    }
    return (returnSize & ~3) + 4;
}

/* JFG: give the object a corona described by its level entry. */
void corona_from_level_entry(Object *obj) {
    ObjectHeaderCorona header;
    CoronaEntry *entry;

    entry = obj->entry;
    header.unk0 = 0x41;
    header.unk1 = entry->unk13;
    header.x = entry->x;
    header.y = entry->y;
    header.z = entry->z;
    header.unk14 = entry->unkA;
    header.unk15 = entry->unkB;
    header.unk16 = entry->unkC;
    header.unk17 = entry->unkD;
    header.unk18 = 0x61;
    header.unk1A = entry->unkE;
    header.unk1C = (entry->unkE * entry->unk10) / 100;
    header.unk1E = -1;
    header.unk1F = 0;
    obj->unk68 = func_80023A48(NULL, &header);
}

void func_80023CB4(void *light);

s32 func_800229B8(void *light) {
    func_80023CB4(light);
    return 1;
}
