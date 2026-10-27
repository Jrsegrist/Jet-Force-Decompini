/*
 * Runlink module 39: a lighting cycle. A list of keyframes, each held for a
 * number of seconds, gives the track's colour, the fog colour and distances
 * and an ambient light's colour and intensity; the update advances through
 * the list (a zero duration ends it and wraps to the start) and blends
 * towards the next keyframe. Getters hand the fog and the track colour to
 * the renderer (as module 23 does for its day-night cycle).
 */
#include "module.h"

typedef struct M039Key {
    /* 0x00 */ f32 trackRed;
    /* 0x04 */ f32 trackGreen;
    /* 0x08 */ f32 trackBlue;
    /* 0x0C */ f32 fogR;
    /* 0x10 */ f32 fogG;
    /* 0x14 */ f32 fogB;
    /* 0x18 */ f32 fogNear;
    /* 0x1C */ f32 fogFar;
    /* 0x20 */ f32 lightR;
    /* 0x24 */ f32 lightG;
    /* 0x28 */ f32 lightB;
    /* 0x2C */ f32 lightIntensity;
    /* 0x30 */ f32 duration; /* seconds; 0 ends the list */
} M039Key;

/* the description of a light (as module 123's) */
typedef struct M039LightParams {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u16 radius;
    /* 0x0E */ s16 radius2;
    /* 0x10 */ u8 r;
    /* 0x11 */ u8 g;
    /* 0x12 */ u8 b;
    /* 0x13 */ u8 a;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
} M039LightParams;

typedef struct M039Light M039Light;

extern M039Light *func_800210A4(Object *obj, M039LightParams *params); /* DKR add_object_light */
extern void func_8002179C(M039Light *light);                            /* DKR light_remove */
extern void func_8002130C(M039Light *light, u8 red, u8 green, u8 blue); /* sets a light's colour */
extern void func_80021358(M039Light *light, u8 intensity);             /* sets a light's intensity */

void func_m039_000D0(s32 updateRate);

#define KEY_DIM { 192.0f, 64.0f, 64.0f, 35.0f, 17.0f, 83.0f, 985.0f, 1009.0f, 128.0f, 240.0f, 255.0f, 0.0f, 0.1f }
#define KEY_LIT { 255.0f, 255.0f, 255.0f, 35.0f, 17.0f, 83.0f, 1018.0f, 1023.0f, 128.0f, 240.0f, 255.0f, 255.0f, 0.1f }

static M039Key sKeys[11] = {
    { 192.0f, 64.0f, 64.0f, 35.0f, 17.0f, 83.0f, 985.0f, 1009.0f, 128.0f, 240.0f, 255.0f, 0.0f, 2.0f },
    KEY_DIM,
    KEY_LIT,
    KEY_DIM,
    KEY_LIT,
    { 192.0f, 64.0f, 64.0f, 35.0f, 17.0f, 83.0f, 985.0f, 1009.0f, 128.0f, 240.0f, 255.0f, 0.0f, 3.0f },
    KEY_DIM,
    KEY_LIT,
    KEY_DIM,
    KEY_LIT,
    { 192.0f, 128.0f, 128.0f, 35.0f, 17.0f, 83.0f, 985.0f, 1009.0f, 128.0f, 240.0f, 255.0f, 0.0f, 0.0f },
};
static M039Light *sLight = NULL;

static f32 sTime;
static s32 sKey;
static s32 sTrackRed;
static s32 sTrackGreen;
static s32 sTrackBlue;
static s32 sFogR;
static s32 sFogG;
static s32 sFogB;
static s32 sFogNear;
static s32 sFogFar;

void func_m039_00000(void) {
    M039LightParams lp;

    lp.type = 0;
    lp.unk1 = 3;
    lp.unk2 = 0x41;
    lp.unk4 = 0;
    lp.unk3 = -1;
    lp.unk6 = 0;
    lp.unk8 = 0;
    lp.unkA = 0;
    lp.a = 0;
    lp.radius = 100;
    lp.radius2 = 100;
    lp.unk14 = -1;
    lp.unk16 = 0;
    lp.unk17 = 0xC6;
    sLight = func_800210A4(NULL, &lp);
    sKey = 0;
    sTime = 0.0f;
    func_m039_000D0(0);
}

void func_m039_0009C(void) {
    if (sLight != NULL) {
        func_8002179C(sLight);
        sLight = NULL;
    }
}

#ifdef NON_MATCHING
/* registers: the original keeps the keyframe values in f14/f16/f18, spills two to the stack (frame 0x40) */
#define BLEND(field) (cur->field + (next->field - cur->field) * frac)

void func_m039_000D0(s32 updateRate) {
    M039Key *cur;
    M039Key *next;
    f32 frac;
    s32 r;
    s32 g;
    s32 b;
    s32 i;

    sTime += updateRate / 60.0f;
    cur = &sKeys[sKey];
    while (cur->duration <= sTime) {
        sTime -= cur->duration;
        sKey++;
        cur++;
        if (cur->duration == 0.0f) {
            sKey = 0;
            cur = sKeys;
        }
    }
    next = cur + 1;
    frac = sTime / cur->duration;
    if (next->duration == 0.0f) {
        next = sKeys;
    }
    sTrackRed = BLEND(trackRed);
    sTrackGreen = BLEND(trackGreen);
    sTrackBlue = BLEND(trackBlue);
    sFogR = BLEND(fogR);
    sFogG = BLEND(fogG);
    sFogB = BLEND(fogB);
    sFogNear = BLEND(fogNear);
    sFogFar = BLEND(fogFar);
    if (sLight != NULL) {
        r = BLEND(lightR);
        g = BLEND(lightG);
        b = BLEND(lightB);
        i = BLEND(lightIntensity);
        func_8002130C(sLight, r, g, b);
        func_80021358(sLight, i);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m039", func_m039_000D0);
#endif

void func_m039_004FC(s16 *near, s16 *far, u8 *r, u8 *g, u8 *b) {
    *near = sFogNear;
    *far = sFogFar;
    *r = sFogR;
    *g = sFogG;
    *b = sFogB;
}

void func_m039_00540(s32 *r, s32 *g, s32 *b) {
    *r = sTrackRed;
    *g = sTrackGreen;
    *b = sTrackBlue;
}
