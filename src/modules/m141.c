/*
 * Runlink module 141: a flying insect enemy that bobs up and down around
 * its home and sways as it flies.
 */
#include "module.h"

/* The part of module 3's enemy record that this enemy uses */
typedef struct M141Enemy {
    /* 0x00 */ u32 pad0_0 : 5;
    /* 0x00 */ u32 still : 1;
    /* 0x00 */ u32 pad0_1 : 26;
    /* 0x04 */ u8 pad4[0x1A - 0x4];
    /* 0x1A */ u8 mission;
    /* 0x1B */ u8 pad1B[0xA8 - 0x1B];
    /* 0xA8 */ s16 sway[2]; /* angle, phase */
} M141Enemy;

extern s32 D_800FE9F8;

extern s32 func_m003_0CAA4(f32, f32, s32);
extern s32 func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_0512C(s32, f32, s32);
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */

void func_m141_00000(Object *obj) {
    M141Enemy *e = obj->unk68;

    if (e->mission != 0x14) {
        if (func_m003_0CAA4(0.0f, 0.0f, 0x7FFF) != 0) {
            if (e->mission != 5) {
                if (func_m003_04260(4, 5, 0, 0, 0) != 0) {
                    func_m003_0512C(6, 1.0f, 5);
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m141", func_m141_00084);

/* Appends the body tilt commands for the model to *list */
void func_m141_00280(Object *obj, s16 **list) {
    M141Enemy *e = obj->unk68;
    s16 *p = *list;
    s16 *sway = e->sway;

    if (!e->still) {
        *p++ = 40;
        *p++ = func_80048E2C(-700, 700);
        *p++ = 46;
        *p++ = func_80048E2C(-700, 700);
    } else {
        p[0] = 40;
        p[1] = sway[0];
        p[2] = 46;
        p[3] = -sway[0];
        sway[1] += D_800FE9F8 * 17677;
        p += 4;
        sway[0] = func_80049C34(sway[1]) * 3100.0f;
    }
    *list = p;
}
