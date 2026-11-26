/*
 * guPerspectiveF in the game's own form, and m002's screen distortion (still assembly in the build).
 */
#include "common.h"

f32 func_80049C30(s32 angle);
f32 func_80049C34(s32 angle);

/* JFG: a perspective matrix like libultra's guPerspectiveF, with the 16-bit angle cosine/sine and the scale
   applied per element. */
void guPerspectiveF(f32 mf[4][4], u16 *perspNorm, f32 fovy, f32 aspect, f32 near, f32 far, f32 scale) {
    UNUSED s32 pad;
    s32 angle;
    f32 cot;
    f32 diff;

    angle = fovy * 91.022f;
    cot = func_80049C30(angle) / func_80049C34(angle);
    diff = near - far;
    mf[0][0] = cot / aspect * scale;
    mf[0][1] = 0.0f;
    mf[0][2] = 0.0f;
    mf[0][3] = 0.0f;
    mf[1][0] = 0.0f;
    mf[1][1] = cot * scale;
    mf[1][2] = 0.0f;
    mf[1][3] = 0.0f;
    mf[2][0] = 0.0f;
    mf[2][1] = 0.0f;
    mf[2][2] = (near + far) / diff * scale;
    mf[2][3] = -1.0f;
    mf[3][0] = 0.0f;
    mf[3][1] = 0.0f;
    mf[3][2] = 2.0f * near * far / diff * scale;
    mf[3][3] = 0.0f;
    if (perspNorm != NULL) {
        if (near + far <= 2.0f) {
            *perspNorm = 0xFFFF;
        } else {
            *perspNorm = (s32) (131072.0f / (near + far));
            if (*perspNorm <= 0) {
                *perspNorm = 1;
            }
        }
    }
}

extern u32 *D_800FD820[2]; // JFG: m002's two buffers (one halfword per screen pixel)
extern s32 D_800FD82C;     // JFG: frames to wait first
extern s32 D_800FD830;     // JFG: which buffer is drawn to
extern u8 *D_800FECB0;     // JFG: the previous frame's image
extern s32 D_800FEAD4[];   // JFG: the run lengths, in bytes: zero, non-zero, zero, ...

#ifdef NON_MATCHING
/* JFG: one of m002's distortion halfwords: red - 16 and blue - 16 are a pixel offset (x, y) into the previous
 * frame's 320-pixel-wide image, green (and a 32nd of it) is added to the pixel read there. */
#define DISTORT_PIXEL(image, h) \
    (*(u16 *) ((image) + ((s32) ((h) >> 11) - 16) * 2 + ((s32) (((h) >> 1) & 0x1F) - 16) * 640) + \
     (((h) & 0x7C0) >> 5) + ((h) & 0x7C0))

/* NON_MATCHING: first body, 164 words against the ROM's 246, frame 0x10 against 0x48; verified against the ROM by
 * simulation. The ROM unrolls the zero scan and the copy 8 times with the remainder after, divides through $at
 * and saves ra in a 0x48 frame although it calls nothing: not the shape this file's -O2 gives.
 * JFG: unless still waiting, replace every non-zero halfword of m002's current buffer with the previous frame's
 * pixel it points at (DISTORT_PIXEL), recording the lengths of the zero and non-zero runs, then copy the non-zero
 * runs into the previous frame's image. */
void func_80075C40(void) {
    u32 *buf;
    u32 *p;
    u32 *end;
    u32 *runStart;
    s32 *runs;
    u8 *image;
    u8 *imageRun;
    u32 word;
    u32 half;
    s32 skip;
    s32 n;
    s32 k;

    if (D_800FD82C != 0) {
        D_800FD82C--;
        return;
    }
    buf = D_800FD820[D_800FD830];
    if (buf == NULL) {
        return;
    }
    end = (u32 *) ((u8 *) buf + 0x25800);
    runs = D_800FEAD4;
    image = D_800FECB0;
    p = buf;
    runStart = buf;
    do {
        /* Skip zero words. Like the ROM, test the end only every 8 words (it may look up to 7 words past it). */
        while (TRUE) {
            for (k = 0; k < 8; k++) {
                if (p[k] != 0) {
                    break;
                }
            }
            p += k;
            if (k < 8 || p >= end) {
                break;
            }
        }
        if (p >= end) {
            goto end_reached;
        }
        skip = (u8 *) p - (u8 *) runStart;
        image += skip;
        *runs++ = skip;
        imageRun = image;
        while (TRUE) {
            /* The ROM splits one word load into the two halfwords; reading them as halfwords keeps this C
             * independent of the byte order. */
            half = ((u16 *) p)[0];
            if (half != 0) {
                ((u16 *) p)[0] = DISTORT_PIXEL(image, half);
            }
            half = ((u16 *) p)[1];
            if (half != 0) {
                ((u16 *) p)[1] = DISTORT_PIXEL(image + 2, half);
            }
            p++;
            if (p >= end) {
                *runs++ = (u8 *) p - (u8 *) runStart;
                break;
            }
            word = *p;
            image += 4;
            if (word == 0) {
                break;
            }
        }
        runStart = p;
        p++;
        *runs++ = image - imageRun;
    } while (p < end);
end_reached:
    if (runStart < end) {
        *runs = (u8 *) p - (u8 *) runStart;
    }
    p = (u32 *) ((u8 *) buf + D_800FEAD4[0]);
    image = D_800FECB0 + D_800FEAD4[0];
    if (p < end) {
        runs = &D_800FEAD4[1];
        do {
            n = *runs++ / 4;
            do {
                *(u32 *) image = *p++;
                image += 4;
            } while (--n != 0);
            p = (u32 *) ((u8 *) p + *runs);
            image += *runs++;
        } while (p < end);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/766C0", func_80075C40);
#endif
