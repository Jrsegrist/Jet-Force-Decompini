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
    /* 0x38 */ f32 minY; // JFG: posY - unk28 (light_update)
    /* 0x3C */ f32 maxY; // JFG: posY + unk28
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
    /* 0x60 */ f32 dir[3]; // JFG: the direction from the angles unk58/unk5A (and the owner's rotation)
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

/* JFG: a model's header, reduced */
typedef struct ModelHeader217 {
    /* 0x00 */ u8 pad0[0x2D];
    /* 0x2D */ u8 numAttachPoints;
} ModelHeader217;

/* JFG: an object model, reduced */
typedef struct Model217 {
    /* 0x00 */ ModelHeader217 *header;
    /* 0x04 */ u8 pad4[0x68 - 4];
    /* 0x68 */ f32 (*attachPoints)[3];
} Model217;

typedef struct Object {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ u8 pad6[6];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x3A - 0x18];
    /* 0x3A */ s8 modelIndex;
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ CoronaEntry *entry;
    /* 0x40 */ ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x54 - 0x44];
    /* 0x54 */ struct ShadeProperties *shading;
    /* 0x58 */ u8 pad58[0x68 - 0x58];
    /* 0x68 */ void *unk68;
    /* 0x6C */ Model217 **models;
    /* 0x70 */ u8 pad70[0x74 - 0x70];
    /* 0x74 */ ObjectLight **lightData;
    /* 0x78 */ void **coronaData; // JFG
    /* 0x7C */ u8 pad7C[0xA0 - 0x7C];
    /* 0xA0 */ u8 freed;
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

s32 func_80013E0C(f32 x, f32 y, f32 z, f32 outerRadius, f32 innerRadius, s32 red, s32 green, s32 blue);
/* Gives a light its track light (unless flag 0x40) (0x80020D84). */
void light_track_light_add(ObjectLight *light) {
    if (!(light->enabled & 0x40)) {
        light->trackLight = func_80013E0C(light->posX, light->posY, light->posZ, light->radius * 1.25f,
                                     light->radius * 0.3f, (light->colourR * light->intensity) >> 8,
                                     (light->colourG * light->intensity) >> 8, (light->colourB * light->intensity) >> 8);
    } else {
        light->trackLight = 0;
    }
}

typedef struct LevelHeader217 {
    /* 0x00 */ u8 pad0[0x6B];
    /* 0x6B */ s8 unk6B;
    /* 0x6C */ u8 pad6C[0x92 - 0x6C];
    /* 0x92 */ s16 colourCycles[7]; // JFG: colour cycle assets lights can use (-1: none)
    /* 0xA0 */ u8 padA0[0xD4 - 0xA0];
    /* 0xD4 */ s16 shadeAngleY;
    /* 0xD6 */ s16 shadeAngleX;
    /* 0xD8 */ u8 padD8[0xDB - 0xD8];
    /* 0xDB */ u8 shadeDiffuse;
    /* 0xDC */ u8 shadeAmbient;
    /* 0xDD */ u8 padDD[0xDF - 0xDD];
    /* 0xDF */ u8 shadeOverride; // JFG: use these instead of the object header's
} LevelHeader217;

LevelHeader217 *level_header(void);
void colour_cycle_init(void *cycle, s32 assetID); // sets up a colour cycle from an asset

/* JFG: a light's level object entry (DKR: LevelObjectEntry_RgbaLight) */
typedef struct LevelObjectEntry_Light {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA; // two nibbles
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 colourR;
    /* 0x0D */ u8 colourG;
    /* 0x0E */ u8 colourB;
    /* 0x0F */ u8 intensity;
    /* 0x10 */ s16 radius;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 pad14[2];
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 colourCycle; // an index into the level header's, if under 7
    /* 0x1F */ u8 unk1F;
} LevelObjectEntry_Light;

/**
 * Adds a light source from a level object entry.
 * JFG: the object isn't used; the light's own layout (a colour cycle from the level header).
 * DKR: light_add_from_level_object_entry
 */
ObjectLight *light_add_from_level_object_entry(Object *obj, LevelObjectEntry_Light *lightEntry) {
    ObjectLight *light;
    LevelHeader217 *levelHeader;

    light = NULL;
    if (gNumActiveLights < gMaxLights) {
        light = gActiveLights[gNumActiveLights++];
        light->unk0 = (lightEntry->unkA >> 4) & 0xF;
        light->unk1 = lightEntry->unkA & 0xF;
        if ((lightEntry->unkB & 0xE0) == 0x40) {
            light->unk0 = 3;
        }
        light->flags = 7;
        light->enabled = ~lightEntry->unk1F & 0x61;
        light->unk4 = 0;
        light->unk6 = -1;
        light->homeX = 0.0f;
        light->homeY = 0.0f;
        light->homeZ = 0.0f;
        light->owner = NULL;
        light->posX = lightEntry->x;
        light->posY = lightEntry->y;
        light->posZ = lightEntry->z;
        light->radius = lightEntry->radius;
        light->unk28 = light->radius;
        light->unk2C = light->radius;
        if (light->unk0 == 2) {
            light->unk28 = lightEntry->unk12;
        }
        light->radiusSquare = light->radius * light->radius;
        light->radiusMag = 1.0f / light->radius;
        light->colourR = lightEntry->colourR;
        light->colourG = lightEntry->colourG;
        light->colourB = lightEntry->colourB;
        light->intensity = lightEntry->intensity;
        light->intensityF = lightEntry->intensity;
        light->unk54 = 0;
        if (lightEntry->colourCycle < 7) {
            levelHeader = level_header();
            if (levelHeader->colourCycles[lightEntry->colourCycle] != -1) {
                colour_cycle_init(light->colourCycle, levelHeader->colourCycles[lightEntry->colourCycle]);
            }
        }
        light->unk58 = lightEntry->unk16;
        light->unk5C = lightEntry->unk1A;
        light->unk5A = lightEntry->unk18;
        light->unk5E = lightEntry->unk1C;
        light_track_light_add(light);
        light_update(light, 0);
    }
    return light;
}

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

void func_800496B0(s32 arg0, s16 *rotation, f32 *in, f32 *out); // DKR: vec3f_rotate
void func_8004944C(s16 *rotation, f32 *vec);                     // DKR: vec3f_rotate_py
void track_light_set_position(s32 trackLight, f32 x, f32 y, f32 z);
void track_light_set_colour(s32 trackLight, s32 red, s32 green, s32 blue, s32 intensity);
void update_colour_cycle(void *cycle, s32 updateRate);

/**
 * Updates a light: its position (from the owner's model attachment point unk6, or its home turned by the owner),
 * its colour cycle and its angles, then the colour table and the track light.
 * JFG: flags bit 0 position, bit 1 colour, bit 2 direction changed; cleared at the end.
 * DKR: light_update
 */
void light_update(ObjectLight *light, s32 updateRate) {
    s16 rot[3];
    Model217 *model;
    ModelHeader217 *header;
    Object *owner;

    owner = light->owner;
    if (owner != NULL && !owner->freed) {
if (light->unk6 >= 0) {
            model = owner->models[owner->modelIndex];
            if (model != NULL && light->unk6 < (header = model->header)->numAttachPoints) {
                light->posX = model->attachPoints[light->unk6][0];
                light->posY = model->attachPoints[light->unk6][1];
                light->posZ = model->attachPoints[light->unk6][2];
                light->flags |= 1;
            }
        } else {
            func_800496B0(1, owner->rotation, &light->homeX, &light->posX);
            light->posX += owner->x;
            light->posY += owner->y;
            light->posZ += owner->z;
            light->flags |= 1;
        }
        light->flags |= 4;
        if (light->trackLight != 0) {
            track_light_set_position(light->trackLight, light->posX, light->posY, light->posZ);
        }
    }
    if (light->unk54 != 0) {
        update_colour_cycle(light->colourCycle, updateRate);
        light->colourR = light->colourCycle[8];
        light->colourG = light->colourCycle[9];
        light->colourB = light->colourCycle[10];
        if (!(light->enabled & 0x10)) {
            light->intensity = light->colourCycle[11];
            light->intensityF = light->intensity;
        }
        light->flags |= 2;
    }
    if (light->unk5C != 0) {
        light->unk58 += light->unk5C * updateRate;
        light->flags |= 4;
    }
    if (light->unk5E != 0) {
        light->unk5A += light->unk5E * updateRate;
        light->flags |= 4;
    }
    if ((light->flags & 1) || (owner != NULL && !owner->freed)) {
        light->minY = light->posY - light->unk28;
        light->maxY = light->posY + light->unk28;
    }
    if (light->flags & 2) {
        func_80023170(light->colourR, light->colourG, light->colourB, light->unk70);
        if (light->trackLight != 0) {
            track_light_set_colour(light->trackLight, light->colourR, light->colourG, light->colourB, light->intensity);
        }
    }
    if ((light->flags & 4) && (light->unk0 == 3 || light->unk0 == 0)) {
        light->dir[2] = -1.0f;
        rot[0] = light->unk58;
        rot[1] = light->unk5A;
        rot[2] = 0;
        func_8004944C(rot, light->dir);
        if (owner != NULL && !owner->freed) {
            func_800496B0(1, owner->rotation, light->dir, light->dir);
        }
        light->dir[0] = -light->dir[0];
        light->dir[1] = -light->dir[1];
        light->dir[2] = -light->dir[2];
    }
    light->flags = 0;
}

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

f32 sqrtf(f32 x);

/* JFG: the active light that shines strongest at a point (falloff by distance times intensity), or NULL. */
ObjectLight *light_find_strongest_at(f32 x, f32 y, f32 z) {
    s32 i;
    ObjectLight *light;
    ObjectLight *best;
    f32 dx, dy, dz;
    f32 distSq;
    f32 strength;
    f32 bestStrength;

    bestStrength = 0.0f;
    best = NULL;
    i = gNumActiveLights;
    while (i--) {
        light = gActiveLights[i];
        if (light != NULL) {
            dx = x - light->posX;
            dy = y - light->posY;
            dz = z - light->posZ;
            distSq = dx * dx + dy * dy + dz * dz;
            if (distSq < light->radiusSquare) {
                strength = (1.0f - sqrtf(distSq) * light->radiusMag) * light->intensityF;
                if (bestStrength < strength) {
                    bestStrength = strength;
                    best = light;
                }
            }
        }
    }
    return best;
}

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
    /* 0x0B */ u8 padB;
    /* 0x0C */ u8 diffuse; // JFG: the object's own (see init_object_shading)
    /* 0x0D */ u8 ambient;
    /* 0x0E */ s16 unkE;   // JFG: the number of lights in use (light first)
    /* 0x10 */ DirLight lights[4]; // JFG: the directional light, then up to three point lights
} ShadeProperties;

Object **objGetObjList(s32 *arg0, s32 *cnt);
void light_shading_points_update(Object *obj, ShadeProperties *shade);
void func_80054530(); // a module function (patched in by the runtime linker)

/**
 * JFG: shade every 3D-model object that has shading (and unkA1 clear): light_shading_points_update, or a module
 * function when the level header says so. (The shading read into a local per loop, as found by the
 * permuter, gives the ROM's frame.)
 */
void objects_shade_all(void) {
    Object **objs;
    struct ShadeProperties *shading2;
    struct ShadeProperties *shading;
    Object *obj;
    s32 i;
    s32 count;

    objs = objGetObjList(&i, &count);
    if (level_header()->unk6B) {
        while (i < count) {
            obj = objs[i++];
            shading2 = obj->shading;
            if (obj->shading != NULL && obj->header->unk1E == 0 && obj->unkA1 == 0) {
                func_80054530(obj, shading2);
            }
        }
    } else {
        while (i < count) {
            obj = objs[i++];
            shading = obj->shading;
            if (obj->shading != NULL && obj->header->unk1E == 0 && obj->unkA1 == 0) {
                light_shading_points_update(obj, shading);
            }
        }
    }
}

f32 light_falloff_scale(f32 scale, f32 arg1, f32 arg2, s32 type);
f32 light_direction_calc(f32 dirX, f32 dirY, f32 dirZ, f32 diffX, f32 diffY, f32 diffZ, f32 distance);
extern s32 gNumActiveLights;
extern ObjectLight **gActiveLights;

/*
 * JFG: pick up to three point lights for an object's shading (the strongest replace the weakest), and dim
 * its ambient and diffuse by the strongest light flagged 0x20.
 */
void light_shading_points_update(Object *obj, ShadeProperties *shade) {
    s32 i;
    ObjectLight *light;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distSq;
    s32 strength;
    s32 maxStrength;
    DirLight *slot;
    s32 minMax;
    f32 dir;
    s32 j;
    s32 scale;
    LevelHeader217 *levelHeader;
    s32 ambient;

    shade->unkE = 1;
    maxStrength = 0;
    levelHeader = level_header();
    for (i = 0; i < gNumActiveLights; i++) {
        light = gActiveLights[i];
        if ((light->enabled & 1) && light->intensity != 0) {
            if (light->unk0 != 0) {
                dx = light->posX - obj->x;
                dy = light->posY - obj->y;
                dz = light->posZ - obj->z;
                strength = 0;
                distSq = dx * dx + dz * dz;
                switch (light->unk0) {
                    case 2:
                        if (distSq < light->radiusSquare && light->minY < obj->y && obj->y <= light->maxY) {
                            strength = light_falloff_scale(light->intensityF, sqrtf(distSq), light->radiusMag,
                                                           light->unk1);
                        }
                        break;
                    case 3:
                        distSq += dy * dy;
                        if (distSq < light->radiusSquare) {
                            distSq = sqrtf(distSq);
                            dir = light_direction_calc(light->dir[0], light->dir[1], light->dir[2], dx, dy, dz,
                                                       distSq);
                            if (dir > 0.0f) {
                                strength = light_falloff_scale(light->intensityF, distSq, light->radiusMag,
                                                               light->unk1) *
                                           dir;
                            }
                        }
                        break;
                    default:
                        distSq += dy * dy;
                        if (distSq < light->radiusSquare) {
                            strength = light_falloff_scale(light->intensityF, sqrtf(distSq), light->radiusMag,
                                                           light->unk1);
                        }
                        break;
                }
            } else {
                dx = light->dir[0];
                dy = light->dir[1];
                dz = light->dir[2];
                strength = light->intensity;
            }
            if (strength != 0) {
                if ((light->enabled & 0x20) && maxStrength < strength) {
                    maxStrength = strength;
                }
                if (shade->unkE < 4) {
                    slot = &shade->lights[shade->unkE];
                    shade->unkE++;
                } else {
                    slot = &shade->lights[1];
                    minMax = shade->lights[1].max;
                    for (j = 2; j < 4; j++) {
                        if (shade->lights[j].max < minMax) {
                            slot = &shade->lights[j];
                            minMax = shade->lights[j].max;
                        }
                    }
                    if (slot->max >= strength) {
                        slot = NULL;
                    }
                }
                if (slot != NULL) {
                    slot->dir[0] = dx;
                    slot->dir[1] = dy;
                    slot->dir[2] = dz;
                    slot->shift = 0;
                    slot->unkC = (1 << (8 - shade->lights[0].shift)) << 6;
                    slot->max = strength;
                    slot->min = 0;
                    slot->range = strength;
                    slot->unk10 = strength << shade->lights[0].shift;
                    slot->table = light->unk70;
                }
            }
        }
    }
    // strength now holds the diffuse range
    scale = 0x13F - maxStrength;
    if (levelHeader->shadeOverride) {
        ambient = levelHeader->shadeAmbient;
        strength = levelHeader->shadeDiffuse - ambient;
    } else {
        ambient = shade->ambient;
        strength = shade->diffuse - ambient;
    }
    if (maxStrength >= 0x40) {
        ambient = (s32) ((u32) scale * ambient) >> 8;
        strength = (s32) ((u32) scale * strength) >> 8;
    }
    shade->lights[0].max = ambient + strength;
    shade->lights[0].range = strength;
    shade->lights[0].min = ambient;
    shade->lights[0].unk10 = strength << shade->lights[0].shift;
}

f32 sqrtf(f32 x);
f32 func_80049C30(s32 angle);

/* JFG: scale times a falloff of x = a*b (0: x, 1: 1-x, 2: 1-sqrt x, 3: cos, 4: cos squared, 5: (1-x) squared). */
f32 light_falloff_scale(f32 scale, f32 arg1, f32 arg2, s32 type) {
    f32 x = arg1 * arg2;
    switch (type) {
        case 1:
            x = 1.0f - x;
            break;
        case 2:
            x = 1.0f - sqrtf(x);
            break;
        case 3:
            x = func_80049C30(x * 16384.0f);
            break;
        case 4:
            x = func_80049C30(x * 16384.0f);
            x = x * x;
            break;
        case 5:
            x = 1.0f - x;
            x = x * x;
            break;
    }
    return scale * x;
}

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

/* JFG: a model, reduced */
typedef struct ShadeModel {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ void *unk20;
    /* 0x24 */ u8 pad24[0x4F - 0x24];
    /* 0x4F */ s8 unk4F;
    /* 0x50 */ u8 pad50[0x68 - 0x50];
    /* 0x68 */ void *unk68;
    /* 0x6C */ void *unk6C;
    /* 0x70 */ void *unk70;
} ShadeModel;

typedef struct ShadeModelInstance {
    /* 0x00 */ ShadeModel *model;
    /* 0x04 */ void *unk4;
} ShadeModelInstance;

/* JFG: the camera, reduced */
typedef struct ShadeCamera {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} ShadeCamera;

/* JFG: an object's position */
typedef struct ShadeObject {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} ShadeObject;

ShadeCamera *cam_get_active_camera(void);
void func_80049DC0(ShadeObject *obj, MtxF *out);
void func_800231C8(s32 count, void *lights, s32 n, MtxF *matrix, void *buf);
void func_800229E0(void *arg0, void *arg1, DirLight *light, void *buf, void *arg4);
void func_80022BC4(void *arg0, void *arg1, void *buf, s32 n, DirLight *light, void *arg5, void *arg6);
void func_80022E98(void *arg0, void *arg1, DirLight *light, void *arg3, void *buf, s32 n, s32 count);
void func_8002333C(void *arg0, void *arg1, void *arg2, void *buf);

#ifdef NON_MATCHING
/* JFG: shade a model instance: its lights (scaled by shade->unk0 for this call) and an environment
   direction from the camera when the model has one.
   NON_MATCHING: the frame is 8 bytes smaller (the target has two more words below localMtx) and the
   scaled brightness values land in v0/v1 instead of t2/t3. */
void func_800221F0(ShadeObject *obj, ShadeModelInstance *inst, ShadeProperties *shade, MtxF *matrix) {
    s32 n;
    s32 max;
    s32 min;
    s32 range;
    s32 shifted;
    MtxF localMtx;
    ShadeModel *model;
    DirLight *light;
    ShadeCamera *cam;
    s32 newMax;
    s32 newMin;
    f32 dir[3];

    model = inst->model;
    if (shade == NULL && model->unk70 == NULL) {
        return;
    }
    n = model->unk4F;
    if (n <= 0) {
        if (matrix == NULL) {
            func_80049DC0(obj, &localMtx);
            matrix = &localMtx;
        }
        n = 1;
    }
    if (shade != NULL) {
        light = &shade->lights[0];
        if (shade->unk0 != 1.0f) {
            max = light->max;
            min = light->min;
            range = light->range;
            shifted = light->unk10;
            newMax = max * shade->unk0;
            light->max = newMax;
            newMin = min * shade->unk0;
            light->range = newMax - newMin;
            light->unk10 = light->range << light->shift;
            light->min = newMin;
        }
        light = &shade->lights[0];
        if (shade->unkE > 0) {
            func_800231C8(shade->unkE, light, n, matrix, D_800A1010);
            if (shade->unkE == 1) {
                func_800229E0(inst->unk4, model->unk6C, light, D_800A1010, model->unk68);
            } else if (shade->unkE == 2) {
                func_80022BC4(inst->unk4, model->unk6C, D_800A1010, n, light, shade->pad30, model->unk68);
            } else {
                func_80022E98(inst->unk4, model->unk6C, light, model->unk68, D_800A1010, n, shade->unkE);
            }
        }
        light = &shade->lights[0];
        if (shade->unk0 != 1.0f) {
            light->max = max;
            light->min = min;
            light->range = range;
            light->unk10 = shifted;
        }
    }
    if (model->unk70 != NULL) {
        cam = cam_get_active_camera();
        dir[0] = cam->x - obj->x;
        dir[1] = cam->y - obj->y;
        dir[2] = cam->z - obj->z;
        func_800231C8(1, dir, n, matrix, D_800A1010);
        func_8002333C(model->unk20, model->unk70, model->unk68, D_800A1010);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/217C0", func_800221F0);
#endif

extern DirLight D_800F5B28; // JFG: the default object light
void dir_light_setup(DirLight *light, s32 a, s32 b, s16 angle0, s16 angle1, s32 shift);

/* JFG: set up the default object light. */
void func_800224D0(s32 a, s32 b, s16 c, s16 d, s32 e) {
    dir_light_setup(&D_800F5B28, a, b, c, d, e);
}

void func_800492DC(s16 *rotation, f32 *vec); // rotates a vector by three angles

/* JFG: set up a directional light: brightness range min(a, b)..b, a shift, and the direction from two angles. */
void dir_light_setup(DirLight *light, s32 a, s32 b, s16 angle0, s16 angle1, s32 shift) {
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
    light->shift = shift;
    light->range = light->max - light->min;
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

/**
 * Official Name: lightSetupLightSources
 */
void light_setup_light_sources(Object *obj) {
    s32 i;
    for (i = 0; i < obj->header->numLightSources; i++) {
        obj->lightData[i] = light_add_from_object_header(obj, &obj->header->lights[i]);
    }
}

void *corona_add(Object *obj, ObjectHeaderCorona *header);

/* JFG: the same loop as light_setup_light_sources, over the header's corona descriptions (0x800226B8). */
void light_setup_coronas(Object *obj) {
    s32 i;
    for (i = 0; i < obj->header->numCoronas; i++) {
        obj->coronaData[i] = corona_add(obj, &obj->header->coronas[i]);
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
            dir_light_setup(&shadeData->lights[0], levelHeader->shadeAmbient, levelHeader->shadeDiffuse,
                          levelHeader->shadeAngleX, levelHeader->shadeAngleY + 0x8000, 0);
        } else {
            dir_light_setup(&shadeData->lights[0], objHeader->shadeAmbient * 255.0f, objHeader->shadeDiffuse * 255.0f,
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
    obj->unk68 = corona_add(NULL, &header);
}

void corona_free(void *light);

s32 func_800229B8(void *light) {
    corona_free(light);
    return 1;
}
