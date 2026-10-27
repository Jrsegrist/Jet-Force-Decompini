#include "common.h"

/* A sprite at a point that fades in while nothing is in the way and out otherwise (50 slots). Size: 0x50 bytes */
typedef struct Corona {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ u8 active;
    /* 0x07 */ s8 opacity;
    /* 0x08 */ u8 pad8[4];
    /* 0x0C */ f32 pos[3];
    /* 0x18 */ u8 pad18[0x3C - 0x18];
    /* 0x3C */ void *unk3C; // JFG: a sprite
    /* 0x40 */ u8 pad40[0x10];
} Corona;
extern Corona D_800F5B60[50];
extern s32 D_800A110C;

/* JFG: mark the 50 corona slots free (no sprite). */
void func_800239D0(void) {
    Corona *corona = D_800F5B60;
    s32 i = 50;
    while (i--) {
        corona->active = 0;
        corona->unk3C = 0;
        corona++;
    }
    D_800A110C = 0;
}

void func_80023CB4(Corona *light);

/* JFG: free every one of the 50 coronas. */
void func_80023A00(void) {
    Corona *corona = D_800F5B60;
    s32 i = 50;
    while (i--) {
        func_80023CB4(corona);
        corona++;
    }
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023A48);

extern s32 D_800A1108; // JFG: how many coronas are active
extern void *D_800A1110; // JFG: one sprite all coronas use
void sprite_free(void *sprite);

/* JFG: free an active corona and its sprite; freeing the last one also frees the common sprite. */
void func_80023CB4(Corona *corona) {
    if (corona != NULL && corona->active) {
        corona->active = 0;
        if (corona->unk3C != NULL) {
            sprite_free(corona->unk3C);
            corona->unk3C = NULL;
        }
        if (--D_800A1108 == 0) {
            if (D_800A1110 != NULL) {
                sprite_free(D_800A1110);
                D_800A1110 = NULL;
            }
        }
    }
}

void func_80023DAC(Corona *corona);

/* JFG: run func_80023DAC on each active corona. */
void func_80023D48(void) {
    Corona *corona;
    s32 i;

    if (D_800A1108 > 0) {
        corona = D_800F5B60;
        i = 49;
        do {
            if (corona->active) {
                func_80023DAC(corona);
            }
            corona++;
        } while (i--);
    }
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80023DAC);

/* JFG: what func_80018808 writes back */
typedef struct HitResult {
    /* 0x00 */ f32 unk0[11];
} HitResult;

s32 func_80018808(s32 arg0, f32 *from, f32 *to, HitResult *result, s32 flags, s32 arg5);

/* JFG: raise the corona's opacity by 5 (up to 16) when func_80018808 finds nothing between pos and it, else lower it (down to 0). */
void func_80023FA4(f32 *pos, Corona *corona) {
    HitResult result;

    if (func_80018808(0, pos + 3, corona->pos, &result, 0x2404, 0) == 0) {
        corona->opacity += 5;
        if (corona->opacity > 16) {
            corona->opacity = 16;
        }
    } else {
        corona->opacity -= 5;
        if (corona->opacity < 0) {
            corona->opacity = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024044);

#ifdef NON_MATCHING
/**
 * JFG: of count RGBA colours, each channel of the one brightest in it (times alpha), and the top alpha.
 * NON_MATCHING: 22 of 64 words: the loop is one instruction short (the target copies the count in
 * the loop) and its registers differ.
 */
void func_80024330(s32 count, u8 *colours, u8 *outRed, u8 *outGreen, u8 *outBlue, u8 *outAlpha) {
    s32 blue, green, red;
    s32 maxBlue, maxGreen, maxRed;
    s32 maxAlpha;
    s32 value;
    s32 alpha;
    s32 product;

    blue = 0;
    green = 0;
    red = 0;
    maxBlue = 0;
    maxGreen = 0;
    maxRed = 0;
    maxAlpha = 0;
    while (count--) {
        value = colours[0];
        alpha = colours[3];
        product = value * alpha;
        if (maxRed < product) {
            maxRed = product;
            red = value;
        }
        value = colours[1];
        product = value * alpha;
        if (maxGreen < product) {
            maxGreen = product;
            green = value;
        }
        value = colours[2];
        colours += 4;
        product = value * alpha;
        if (maxBlue < product) {
            maxBlue = product;
            blue = value;
        }
        if (maxAlpha < alpha) {
            maxAlpha = alpha;
        }
    }
    *outRed = red;
    *outGreen = green;
    *outBlue = blue;
    *outAlpha = maxAlpha;
}
#else
INCLUDE_ASM("asm/nonmatchings/245D0", func_80024330);
#endif

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024430);

INCLUDE_ASM("asm/nonmatchings/245D0", func_80024634);
