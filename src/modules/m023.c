/*
 * Runlink module 23: a day-night cycle. Twelve two-hour steps each give the
 * track's colour fade, the fog colour and distances, an ambient light's
 * colour, intensity and direction, and the level's lighting levels; the
 * update advances the time and blends from the current step to the next.
 * Getters hand the fog and the fade to the renderer, and the cycle can be
 * told to stop when it reaches step 6 (midday).
 */
#include "module.h"

typedef struct M023Step {
    /* 0x00 */ u8 trackRed;
    /* 0x01 */ u8 trackGreen;
    /* 0x02 */ u8 trackBlue;
    /* 0x03 */ u8 fogR;
    /* 0x04 */ u8 fogG;
    /* 0x05 */ u8 fogB;
    /* 0x06 */ u16 fogNear;
    /* 0x08 */ u16 fogFar;
    /* 0x0A */ u8 lightR;
    /* 0x0B */ u8 lightG;
    /* 0x0C */ u8 lightB;
    /* 0x0D */ u8 lightIntensity;
    /* 0x0E */ s16 lightDirA;
    /* 0x10 */ s16 lightDirB;
    /* 0x12 */ u8 ambient;
    /* 0x13 */ u8 levelUnkDE;
} M023Step;

/* DKR's ObjectLight (src/217C0.c), the fields used here */
typedef struct M023Light {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 flags;
    /* 0x03 */ u8 pad3[0x55];
    /* 0x58 */ s16 unk58;
    /* 0x5A */ s16 unk5A;
} M023Light;

typedef struct M023Level {
    /* 0x00 */ u8 pad0[0xDB];
    /* 0xDB */ u8 unkDB;
    /* 0xDC */ u8 unkDC;
    /* 0xDD */ u8 padDD;
    /* 0xDE */ u8 unkDE;
    /* 0xDF */ u8 unkDF;
} M023Level;

typedef struct M023ObjLight {
    u8 pad0[0x10];
    u8 unk10;
} M023ObjLight;

typedef struct M023ObjInfo {
    u8 pad0[0x1E];
    s8 unk1E;
} M023ObjInfo;

typedef struct M023Object {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ M023ObjInfo *unk40;
    /* 0x44 */ u8 pad44[0x10];
    /* 0x54 */ M023ObjLight *unk54;
    /* 0x58 */ u8 pad58[0x49];
    /* 0xA1 */ u8 unkA1;
} M023Object;

extern M023Light *func_800210A4(Object *obj, LightParams *params); /* DKR light_add_from_object_header */
extern void func_8002179C(M023Light *light);                            /* DKR light_remove */
extern void light_colour_set(M023Light *light, u8 red, u8 green, u8 blue); /* sets a light's colour */
extern void light_intensity_set(M023Light *light, u8 intensity);             /* sets a light's intensity */
extern void func_80022520(void *light, s32 a, s32 b, s32 c, s32 d, s32 e);
extern M023Level *func_8004470C(void);                                   /* DKR levelGetLevel */

void func_m023_0015C(s32 updateRate);

static M023Step sSteps[12] = {
    { 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x3D4, 0x3F2, 0xFF, 0xFF, 0xFF, 0xFF, -8206, -29136, 0x20, 0x00 },
    { 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x3D4, 0x3F2, 0xFF, 0xFF, 0xFF, 0xFF, -8206, -29136, 0x20, 0x00 },
    { 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x3D4, 0x3F2, 0xFF, 0xFF, 0xFF, 0xFF, -8206, -29136, 0x20, 0x00 },
    { 0xFF, 0xFF, 0xE0, 0xFF, 0xFF, 0xE0, 0x3CA, 0x3FC, 0xFF, 0xFF, 0xE0, 0x80, -5476, -16380, 0x55, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3D7, 0x3FC, 0xFF, 0xFF, 0xFF, 0xE0, -7296, -10920, 0x64, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3D9, 0x3FC, 0xFF, 0xFF, 0xFF, 0xF0, -10936, -5460, 0x78, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3DC, 0x3FF, 0xFF, 0xFF, 0xFF, 0xFF, -12756, 0, 0x80, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3D9, 0x3FC, 0xFF, 0xFF, 0xFF, 0xFF, -10936, 5460, 0x78, 0x00 },
    { 0xFF, 0xE0, 0xC0, 0xFF, 0xE0, 0xC0, 0x3D9, 0x3FC, 0xFF, 0xF0, 0xC0, 0xF0, -7296, 10920, 0x64, 0x00 },
    { 0xFF, 0xC0, 0x80, 0xC8, 0x80, 0x08, 0x3D4, 0x3FC, 0xFF, 0xC0, 0x80, 0xE0, -5476, 16380, 0x55, 0x00 },
    { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3D4, 0x3FC, 0xFF, 0xFF, 0xF0, 0x80, -4566, -21856, 0x30, 0x00 },
    { 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x3D4, 0x3F2, 0xFF, 0xFF, 0xFF, 0xFF, -8206, -29136, 0x20, 0x00 },
};
static M023Light *sLight = NULL;
static s32 sStopAtMidday = 0;

static s32 sStep;
static s32 sTime;
static s32 sStepLength;
static s32 sFadeR;
static s32 sFadeG;
static s32 sFadeB;
static s32 sFogR;
static s32 sFogG;
static s32 sFogB;
static s32 sFogNear;
static s32 sFogFar;
static s32 sAmbient;
static s32 sLevelUnkDE;

#define BLEND(field) (cur->field + (((next->field - cur->field) * frac) >> 14))

void func_m023_00000(f32 start, s32 stepLength) {
    LightParams lp;
    M023Level *level;

    sStepLength = stepLength * 2;
    sStep = (s32) start >> 1;
    sTime = (start - (sStep * 2)) * stepLength;
    lp.type = 0;
    lp.unk1 = 3;
    lp.flags = 0x41;
    lp.unk4 = 0;
    lp.unk3 = -1;
    lp.unk6 = 0;
    lp.unk8 = 0;
    lp.unkA = 0;
    lp.a = 0xFF;
    lp.radius = 100;
    lp.radius2 = 100;
    lp.unk14 = -1;
    lp.unk16 = 0;
    lp.unk17 = 0;
    sLight = func_800210A4(NULL, &lp);
    level = func_8004470C();
    sAmbient = level->unkDC;
    sLevelUnkDE = level->unkDE;
    sStopAtMidday = 0;
    func_m023_0015C(0);
}

/* The module's shutdown function (runlink fini) */
void func_m023_00128(void) {
    if (sLight != NULL) {
        func_8002179C(sLight);
        sLight = NULL;
    }
}
#pragma weak __rl_fini = func_m023_00128

void func_m023_0015C(s32 updateRate) {
    s32 i;
    s32 count;
    s32 frac;
    M023Level *level;
    s32 prevStep;
    M023Step *cur;
    M023Step *next;
    M023Object **objs;
    M023Object *obj;
    s32 ambient;

    if (sStopAtMidday && sStep == 6 && sTime == 0) {
        return;
    }
    if (sStopAtMidday) {
        prevStep = sStep;
    }
    sTime += updateRate;
    while (sTime >= sStepLength) {
        sStep++;
        if (sStep >= 12) {
            sStep = 0;
        }
        sTime -= sStepLength;
    }
    if (sStopAtMidday && prevStep < 6 && sStep >= 6) {
        sStep = 6;
        sTime = 0;
    }
    cur = &sSteps[sStep];
    next = &sSteps[0];
    if (sStep < 11) {
        next += sStep + 1;
    }
    frac = (sTime << 14) / sStepLength;
    sFadeR = BLEND(trackRed);
    sFadeG = BLEND(trackGreen);
    sFadeB = BLEND(trackBlue);
    sFogR = BLEND(fogR);
    sFogG = BLEND(fogG);
    sFogB = BLEND(fogB);
    sFogNear = BLEND(fogNear);
    sFogFar = BLEND(fogFar);
    if (sLight != NULL) {
        sLight->unk5A = BLEND(lightDirA);
        sLight->unk58 = BLEND(lightDirB);
        sLight->flags |= 4;
        light_colour_set(sLight, BLEND(lightR), BLEND(lightG), BLEND(lightB));
        light_intensity_set(sLight, BLEND(lightIntensity));
    }
    level = func_8004470C();
    ambient = BLEND(ambient);
    sLevelUnkDE = BLEND(levelUnkDE);
    level->unkDE = sLevelUnkDE;
    level->unkDF = 1;
    if (ambient != sAmbient) {
        sAmbient = ambient;
        level->unkDC = ambient;
        level->unkDB = ambient + 1;
        objs = (M023Object **) func_80005E98(&i, &count);
        while (i < count) {
            obj = objs[i++];
            if (obj->unk54 != NULL && obj->unk40->unk1E == 0 && obj->unkA1 == 0) {
                func_80022520(&obj->unk54->unk10, ambient, ambient + 1, 0, 0, 0);
            }
        }
    }
}

/* Stops the cycle once it reaches step 6 */
void func_m023_0065C(void) {
    sStopAtMidday = 1;
}

void func_m023_0066C(s16 *near, s16 *far, u8 *r, u8 *g, u8 *b) {
    *near = sFogNear;
    *far = sFogFar;
    *r = sFogR;
    *g = sFogG;
    *b = sFogB;
}

void func_m023_006B0(s32 *r, s32 *g, s32 *b) {
    *r = sFadeR;
    *g = sFadeG;
    *b = sFadeB;
}
