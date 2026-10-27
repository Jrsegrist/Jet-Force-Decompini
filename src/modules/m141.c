/*
 * Runlink module 141: a flying insect enemy that bobs up and down around
 * its home and sways as it flies.
 */
#include "module.h"

/* Its home object's data (that object's unk68) */
typedef struct M141Home {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ s8 offsetX;
    /* 0x11 */ s8 offsetY;
    /* 0x12 */ s8 offsetZ;
    /* 0x13 */ u8 radius;
    /* 0x14 */ u8 height;
} M141Home;

/* The part of module 3's enemy record that this enemy uses */
typedef struct M141Enemy {
    /* 0x00 */ u32 pad0_0 : 5;
    /* 0x00 */ u32 still : 1;
    /* 0x00 */ u32 pad0_1 : 26;
    /* 0x04 */ u8 pad4[0x1A - 0x4];
    /* 0x1A */ u8 mission;
    /* 0x1B */ u8 pad1B[0x24 - 0x1B];
    /* 0x24 */ Object *home;
    /* 0x28 */ u8 pad28[0x38 - 0x28];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0xA8 - 0x44];
    /* 0xA8 */ s16 sway[2]; /* angle, phase; sway[0] bit 0 also: going down */
} M141Enemy;

extern s32 D_800FE9F8;

extern s32 func_m003_0CAA4(f32, f32, s32);
extern s32 func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_0512C(s32, f32, s32);
extern void func_m003_04D08(s32, s32, s32, s32);
extern void func_m003_04F88(s32, f32);
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle); /* cosine */

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

/* Moves up to the top of its column, then down to the bottom and to a new random spot around home */
void func_m141_00084(Object *obj) {
    M141Enemy *e;
    Object *homeObj;
    M141Home *home;
    s16 *flags;
    f32 top;
    f32 bottom;
    s16 angle;
    f32 dist;

    e = obj->unk68;
    homeObj = e->home;
    home = homeObj->unk68;
    top = homeObj->trans.y_position + (home->offsetY << 4) + (home->height << 4);
    bottom = homeObj->trans.y_position + (home->offsetY << 4) - (home->height << 4);
    flags = &e->sway[0];
    if (top <= obj->trans.y_position) {
        if (!(*flags & 1)) {
            *flags |= 1;
            func_m003_04D08(0, 0x10, 0, 0);
        }
    } else if (obj->trans.y_position <= bottom) {
        if (*flags & 1) {
            *flags &= ~1;
            func_m003_04D08(0, 0x11, 0, 0);
            angle = func_80048E2C(0, 0xFFFF);
            dist = func_80048E2C(-(home->radius << 4), home->radius << 4);
            e->x = func_80049C30(angle) * dist + (homeObj->trans.x_position + (home->offsetX << 4));
            e->z = func_80049C34(angle) * dist + (homeObj->trans.z_position + (home->offsetZ << 4));
        }
    }
    if (*flags & 1) {
        e->y = bottom - 10.0f;
        obj->particleEmittersEnabled = 0;
    } else {
        e->y = top + 10.0f;
        obj->particleEmittersEnabled = 8;
    }
    func_m003_04F88(0, 1.0f);
}

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
