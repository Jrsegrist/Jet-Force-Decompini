/*
 * Runlink module 65: animates a fire texture. Two heat buffers per texture
 * take turns: the bottom rows are seeded with random heat (centred, fading to
 * the sides), the heat is blurred upwards and cools, and the result is turned
 * into the texture's 16-bit pixels (red/green from the heat), every other row
 * with its pixel pairs swapped.
 */
#include "module.h"

typedef struct M065Texture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0x20 - 0x2];
    /* 0x20 */ u16 pixels[1];
} M065Texture;

extern u8 *D_80102D60[]; /* heat buffers, by texture */
extern u8 *D_80102D78[]; /* the other heat buffers */

/*
 * NON_MATCHING: same instructions in nearly the same order, but register
 * allocation and spill placement differ (the ROM spills the source buffer
 * pointer at its definition and uses fewer induction variables in the pixel
 * loops). The asm refers to the data by its offset name, so the matching
 * build keeps the data in asm too.
 */
#ifdef NON_MATCHING
static u8 sOtherBuffer = 0;
static s32 sSeeded = 0;

void func_m065_00000(s32 index, M065Texture *tex, s32 arg2) {
    s32 width;
    s32 height;
    u8 *src;
    u8 *dst;
    u8 *out;
    u16 *pix;
    u16 *row;
    s32 i;
    s32 j;
    s32 d;
    s32 max;
    s32 v;
    s32 r;
    s32 g;
    s32 off;

    width = tex->width;
    height = tex->height;
    if (sOtherBuffer) {
        src = D_80102D60[index];
    } else {
        src = D_80102D78[index];
    }
    dst = src + (height - 6) * width + 3;
    if (!sSeeded) {
        i = width - 6;
        while (i--) {
            d = (width >> 1) - i - 3;
            v = d;
            if (d <= 0) {
                v = -d;
            }
            *dst++ = func_80048E2C(0, 255 - v * 16);
        }
        sSeeded = 1;
    } else {
        i = width - 6;
        while (i--) {
            d = (width >> 1) - i - 3;
            v = d;
            if (d <= 0) {
                v = -d;
            }
            max = 255 - v * 16;
            v = func_80048E2C(-0x60, 0x60) + *dst;
            if (max < v) {
                v = max;
            }
            if (v < 0) {
                v = 0;
            }
            *dst++ = v;
        }
    }
    if (sOtherBuffer ^= 1) {
        out = D_80102D60[index];
    } else {
        out = D_80102D78[index];
    }
    dst = out;
    i = height - 6;
    while (i--) {
        *dst++ = 0;
        src++;
        j = width - 2;
        while (j--) {
            v = (src[width] * 4 + src[0] * 2 + src[width - 1] + src[width + 1]) >> 3;
            if (v < 170) {
                v--;
            }
            if (v < 0) {
                v = 0;
            }
            *dst++ = v;
            src++;
        }
        *dst++ = 0;
        src++;
    }
    dst += width;
    src += width;
    i = 5;
    while (i--) {
        *dst++ = 0;
        src++;
        j = width - 2;
        while (j--) {
            v = (src[-width] + src[-width - 1] + src[-width + 1]) >> 2;
            if (v < 170) {
                v--;
            }
            if (v < 0) {
                v = 0;
            }
            *dst++ = v;
            src++;
        }
        *dst++ = 0;
        src++;
    }
    dst = out + (height - 6) * width;
    src = dst - width;
    i = width;
    while (i--) {
        *dst++ = *src++;
    }
    src = out;
    pix = tex->pixels;
    i = height;
    while (i--) {
        row = pix;
        if (i & 1) {
            j = width;
            while (j--) {
                v = *src++;
                r = v + 52;
                if (v >> 4) {
                    v = (v >> 4) * v;
                }
                g = v;
                if (r >= 256) {
                    r = 255;
                }
                if (g >= 256) {
                    g = 255;
                }
                *pix++ = (r << 8) | g;
            }
        } else {
            for (j = 0; j < width; j++) {
                if (j & 2) {
                    off = -2;
                } else {
                    off = 2;
                }
                v = *src++;
                r = v + 52;
                if (v >> 4) {
                    v = (v >> 4) * v;
                }
                g = v;
                if (r >= 256) {
                    r = 255;
                }
                if (g >= 256) {
                    g = 255;
                }
                row[off + j] = (r << 8) | g;
            }
            pix = row + width;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m065", func_m065_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m065", data);
#endif
