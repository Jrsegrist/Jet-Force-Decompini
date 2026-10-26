/*
 * Runlink module 145: a floating enemy that drifts up and down inside a
 * column around its home object, firing when it can.
 */
#include "module.h"

/* Its home object's data (that object's unk68) */
typedef struct M145Home {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ u8 fireChance;
    /* 0x0D */ u8 padD[0x10 - 0xD];
    /* 0x10 */ s8 offsetX;
    /* 0x11 */ s8 offsetY;
    /* 0x12 */ s8 offsetZ;
    /* 0x13 */ u8 radius;
    /* 0x14 */ u8 height;
} M145Home;

/* The part of module 3's enemy record that this enemy uses */
typedef struct M145Enemy {
    /* 0x00 */ u8 pad0[0x1A];
    /* 0x1A */ u8 mission;
    /* 0x1B */ u8 pad1B[0x24 - 0x1B];
    /* 0x24 */ Object *home;
    /* 0x28 */ u8 pad28[0x38 - 0x28];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0xA8 - 0x44];
    /* 0xA8 */ s16 flags; /* 1: target chosen, 2: going down */
} M145Enemy;

extern s32 func_m003_0CAA4(f32, f32, s32);
extern s32 func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_04398(s32, s32);
extern void func_m003_07E88(s32, s32, s32);
extern void func_m003_04D08(s32, s32, s32, s32);
extern void func_m003_04F88(s32, f32);
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle); /* cosine */

void func_m145_00000(Object *obj) {
    M145Enemy *e = obj->unk68;
    Object *homeObj = e->home;
    M145Home *home = homeObj->unk68;
    s32 r;

    if (e->mission != 5) {
        if (func_m003_0CAA4(0.0f, 0.0f, 0x7FFF) != 0) {
            if (func_m003_04260(4, 5, 0, 0, 0) != 0) {
                r = func_80048E2C(0, 50);
                if (home->fireChance >= func_80048E2C(0, 50) + r) {
                    func_m003_07E88(0, 0, 0);
                    func_m003_07E88(1, 0, 0);
                }
                func_m003_04398(1, 0);
            }
        }
    }
}

#ifdef NON_MATCHING
/* Equivalent; only the stack slot of `flags` differs (0x28 here, 0x2C in the original) */
void func_m145_000D0(Object *obj) {
    M145Enemy *e;
    Object *homeObj;
    M145Home *home;
    f32 unused; /* keeps the frame layout */
    f32 top;
    f32 bottom;
    s16 angle;
    f32 dist;
    s16 *flags;

    e = obj->unk68;
    homeObj = e->home;
    home = homeObj->unk68;
    top = homeObj->trans.y_position + (home->offsetY << 4) + (home->height << 4);
    bottom = homeObj->trans.y_position + (home->offsetY << 4) - (home->height << 4);
    flags = &e->flags;
    if (!(*flags & 1)) {
        angle = func_80048E2C(0, 0xFFFF);
        dist = func_80048E2C(-(home->radius << 4), home->radius << 4);
        e->x = func_80049C30(angle) * dist + (homeObj->trans.x_position + (home->offsetX << 4));
        e->z = func_80049C34(angle) * dist + (homeObj->trans.z_position + (home->offsetZ << 4));
        func_m003_04D08(0, 0x11, 0, 0);
        *flags = 1;
    }
    if (top <= obj->trans.y_position) {
        if (!(*flags & 2)) {
            *flags |= 2;
            func_m003_04D08(0, 0x10, 0, 0);
        }
    } else if (obj->trans.y_position <= bottom) {
        if (*flags & 2) {
            *flags &= ~2;
            func_m003_04D08(0, 0x11, 0, 0);
            angle = func_80048E2C(0, 0xFFFF);
            dist = func_80048E2C(-(home->radius << 4), home->radius << 4);
            e->x = func_80049C30(angle) * dist + (homeObj->trans.x_position + (home->offsetX << 4));
            e->z = func_80049C34(angle) * dist + (homeObj->trans.z_position + (home->offsetZ << 4));
        }
    }
    if (*flags & 2) {
        e->y = bottom - 10.0f;
        obj->particleEmittersEnabled = 0;
    } else {
        e->y = top + 10.0f;
        obj->particleEmittersEnabled = 8;
    }
    func_m003_04F88(0, 1.0f);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m145", func_m145_000D0);
#endif
