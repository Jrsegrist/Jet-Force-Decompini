/*
 * Level effects and the screen transitions (fades). Functions in C here are
 * from the Diddy Kong Racing decompilation (CC0 1.0), src/fade_transition.c;
 * see CREDITS.md.
 */
#include "common.h"

/* DKR src/fade_transition.h and include/structs.h */
#define FADE_FLAG_OUT (1 << 7) // Transition fading out

enum TransitionStatus {
    TRANSITION_NONE,
    TRANSITION_ACTIVE
};

typedef struct FadeTransition {
    /* 0x00 */ u8 type; // top 2 bits also act as flags
    /* 0x01 */ u8 red;
    /* 0x02 */ u8 green;
    /* 0x03 */ u8 blue;
    /* 0x04 */ u16 duration;
    /* 0x06 */ u16 endTimer;
} FadeTransition;

typedef struct Vertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} Vertex;

extern Vertex *sTransitionVtx[2];
extern f32 *gTransitionNextVtx;
extern f32 *gTransitionVertexTarget;
extern f32 *gTransitionVtxStep;
extern f32 gTransitionOpacityVel;
extern f32 sTransitionOpacity;
extern s32 gLastFadeBlue;
extern s32 gLastFadeBlueStep;
extern s32 gLastFadeGreen;
extern s32 gLastFadeGreenStep;
extern s32 gLastFadeRed;
extern s32 gLastFadeRedStep;
extern s32 gTransitionInvert;
extern s32 gTransitionVertexCount;
extern s32 sTransitionStatus;
extern s32 sTransitionTaskNum;
extern u16 gTransitionEndTimer;
extern u16 sTransitionFadeDuration;
extern u16 sTransitionFadeTimer;
extern u8 gCurFadeAlpha;
extern u8 gCurFadeBlue;
extern u8 gCurFadeGreen;
extern u8 gCurFadeRed;

INCLUDE_ASM("asm/nonmatchings/69050", func_80068450);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068494);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068874);

INCLUDE_ASM("asm/nonmatchings/69050", func_800688D8);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068938);

void func_800689B4(s32 arg0) {
}

INCLUDE_ASM("asm/nonmatchings/69050", func_800689BC);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068A14);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068A64);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068B54);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068DF8);

INCLUDE_ASM("asm/nonmatchings/69050", func_80068E58);

INCLUDE_ASM("asm/nonmatchings/69050", func_80069000);

INCLUDE_ASM("asm/nonmatchings/69050", func_800691F0);

INCLUDE_ASM("asm/nonmatchings/69050", func_800694CC);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006999C);

INCLUDE_ASM("asm/nonmatchings/69050", func_80069BE8);

INCLUDE_ASM("asm/nonmatchings/69050", func_80069F04);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006A0C0);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006A6B0);

void tex_free(void *tex);
void mempool_free(void *data);

void func_8006A894(u8 *obj) {
    void *tex = *(void **) (obj + 0x30);
    if (tex != NULL) {
        tex_free(tex);
    }
    mempool_free(obj);
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006A8D4);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006A924);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B090);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B3CC);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B69C);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B8C4);

extern s32 D_800A6780;

void func_8006B95C(void) {
    D_800A6780 = 0x1;
}

extern s32 D_800A6780;

void func_8006B96C(void) {
    D_800A6780 = 0;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B978);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006B9CC);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006BC34);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006BD54);

/* JFG: clear the transition state. */
void func_8006BE88(void) {
    sTransitionStatus = 0;
    sTransitionTaskNum = 0;
}

/**
 * Sets the transition timer and opacity to positive or negative depending on whether to fade in or out.
 */
void transition_fullscreen_start(FadeTransition *transition) {
    if (transition->type & FADE_FLAG_OUT) {
        sTransitionOpacity = 255.0f;
        gTransitionOpacityVel = -255.0f / (f32) sTransitionFadeTimer;
    } else {
        sTransitionOpacity = 0.0f;
        gTransitionOpacityVel = 255.0f / (f32) sTransitionFadeTimer;
    }
    sTransitionStatus = TRANSITION_ACTIVE;
}

/**
 * Slowly dade in or out by increasing or decreasing opacity.
 */
void transition_update_fullscreen(s32 updateRate) {
    s32 var_v0;
    do {
        var_v0 = TRUE;
        if ((sTransitionFadeTimer) > 0) {
            if (updateRate < sTransitionFadeTimer) {
                sTransitionFadeTimer -= updateRate;
                sTransitionOpacity += gTransitionOpacityVel * updateRate;
            } else {
                updateRate -= sTransitionFadeTimer;
                var_v0 = FALSE;
                if (gTransitionOpacityVel < 0.0f) {
                    sTransitionOpacity = 0.0f;
                } else {
                    sTransitionOpacity = 255.0f;
                }
                sTransitionFadeTimer = 0 * updateRate; // Fakematch
            }
            if (sTransitionOpacity < 0.0f) {
                sTransitionOpacity = 0.0f;
            } else if (sTransitionOpacity > 255.0f) {
                sTransitionOpacity = 255.0f;
            }
            gCurFadeAlpha = sTransitionOpacity;
        } else if (gTransitionEndTimer != 0xFFFF) {
            if (updateRate < gTransitionEndTimer) {
                gTransitionEndTimer -= updateRate;
            } else {
                updateRate -= gTransitionEndTimer;
                gTransitionEndTimer = 0;
                if (gTransitionInvert != 0) {
                    gTransitionInvert = 0;
                    sTransitionFadeTimer = sTransitionFadeDuration;
                    gTransitionOpacityVel = -gTransitionOpacityVel;
                    var_v0 = FALSE;
                }
            }
        }
    } while (var_v0 == FALSE && updateRate > 0);
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006C124);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006C230);

/**
 * Slowly update the vertices of the transition based on what the next frames target is.
 * That target is set based on a velocity.
 */
void transition_update_shape(s32 updateRate) {
    s32 i;
    f32 updateRateF;

    updateRateF = updateRate;
    if (sTransitionFadeTimer > 0) {
        if (updateRate < sTransitionFadeTimer) {
            for (i = 0; i < gTransitionVertexCount; i++) {
                gTransitionNextVtx[i * 3] += updateRateF * gTransitionVtxStep[i * 3];
                gTransitionNextVtx[i * 3 + 1] += updateRateF * gTransitionVtxStep[i * 3 + 1];
                gTransitionNextVtx[i * 3 + 2] += updateRateF * gTransitionVtxStep[i * 3 + 2];
            }
            sTransitionFadeTimer -= updateRate;
        } else {
            for (i = 0; i < gTransitionVertexCount; i++) {
                gTransitionNextVtx[i * 3] = gTransitionVertexTarget[i * 3];
                gTransitionNextVtx[i * 3 + 1] = gTransitionVertexTarget[i * 3 + 1];
                gTransitionNextVtx[i * 3 + 2] = gTransitionVertexTarget[i * 3 + 2];
            }
            sTransitionFadeTimer = 0;
        }
        for (i = 0; i < gTransitionVertexCount; i++) {
            sTransitionVtx[sTransitionTaskNum][i].x = gTransitionNextVtx[i * 3];
            sTransitionVtx[sTransitionTaskNum][i].y = gTransitionNextVtx[i * 3 + 1];
            sTransitionVtx[sTransitionTaskNum][i].a = (s32) gTransitionNextVtx[i * 3 + 2];
        }
    } else if (gTransitionEndTimer != 0xFFFF) {
        if (updateRate < gTransitionEndTimer) {
            gTransitionEndTimer -= updateRate;
        } else {
            gTransitionEndTimer = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006CA30);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006CB28);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006CC20);

/**
 * Set the transition colour target, then set the velocity for the colour to fade to.
 */
void transition_init_blank(UNUSED FadeTransition *transition) {
    gLastFadeRed <<= 16;
    gLastFadeGreen <<= 16;
    gLastFadeBlue <<= 16;
    gCurFadeAlpha = 255;
    sTransitionOpacity = 255.0f;
    gTransitionOpacityVel = 0.0f;
    gLastFadeRedStep = ((gCurFadeRed << 16) - gLastFadeRed) / sTransitionFadeTimer;
    gLastFadeGreenStep = ((gCurFadeGreen << 16) - gLastFadeGreen) / sTransitionFadeTimer;
    gLastFadeBlueStep = ((gCurFadeBlue << 16) - gLastFadeBlue) / sTransitionFadeTimer;
    sTransitionStatus = TRANSITION_ACTIVE;
}

/**
 * Shifts the colours of the transition over time while the timer ticks down.
 */
void transition_update_blank(s32 updateRate) {
    s32 breakout = FALSE;
    while (!breakout) {
        breakout = TRUE;
        if (sTransitionFadeTimer > 0) {
            gLastFadeRed += gLastFadeRedStep * updateRate;
            gLastFadeGreen += gLastFadeGreenStep * updateRate;
            gLastFadeBlue += gLastFadeBlueStep * updateRate;
            if (updateRate >= sTransitionFadeTimer) {
                gLastFadeRed = gCurFadeRed << 16;
                gLastFadeGreen = gCurFadeGreen << 16;
                gLastFadeBlue = gCurFadeBlue << 16;
                updateRate -= sTransitionFadeTimer;
                sTransitionFadeTimer = 0;
            } else {
                sTransitionFadeTimer -= updateRate;
            }
        } else {
            if (gTransitionEndTimer != 0xFFFF) {
                gTransitionEndTimer -= updateRate;
                if (gTransitionEndTimer <= 0) {
                    gTransitionEndTimer = 0;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006CFC0);

extern s32 D_80103B80[2];
extern s32 D_80103B88[2];
extern s32 D_80103B90;
extern s32 D_80103B94;

void func_8006D0D8(void) {
    D_80103B80[0] = 0;
    D_80103B80[1] = 0;
    D_80103B88[0] = 0;
    D_80103B88[1] = 0;
    D_80103B90 = 0;
    D_80103B94 = -1;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006D110);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006D390);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006D58C);

extern s32 D_80103B94;

void func_8006D600(s32 arg0) {
    D_80103B94 = arg0;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006D60C);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006D70C);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006D7F0);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006DA1C);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006DAE8);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006DB6C);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006DD20);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006DF90);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006E0C4);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006E188);

extern s32 D_800A6D00;
extern u8 D_800A6D08;
extern u8 D_800A6D0C;

void func_8006EDF0(s32 arg0) {
    if (arg0) {
        D_800A6D00 = 0x20;
    } else {
        D_800A6D00 = 1;
    }
    D_800A6D08 = 1;
    D_800A6D0C = 1;
}

void func_8006EE28(s32 arg0) {
    if (arg0) {
        D_800A6D00 = 0;
    } else {
        D_800A6D00 = 0x20;
    }
    D_800A6D08 = 0;
}

extern s32 D_800A6D00;

s32 func_8006EE50(void) {
    return D_800A6D00;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006EE60);

extern u8 D_800A6D0C;

void func_8006F078(void) {
    D_800A6D0C = 0;
}

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F084);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F424);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F4A0);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F550);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F874);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F8E4);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006F998);

INCLUDE_ASM("asm/nonmatchings/69050", func_8006FB18);
