/* Runlink module 148: a gun turret enemy (its fire decision and its movement). */
#include "module.h"

/* The part of module 3's enemy record that the turret uses */
typedef struct M148Enemy {
    /* 0x00 */ u32 pad0_0 : 25;
    /* 0x00 */ u32 flash : 1;
    /* 0x00 */ u32 pad0_1 : 6;
    /* 0x04 */ u8 pad4[0x1A - 0x4];
    /* 0x1A */ u8 mission;
    /* 0x1B */ u8 state;
    /* 0x1C */ u8 pad1C[0x38 - 0x1C];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0x50 - 0x44];
    /* 0x50 */ u8 shots;
    /* 0x51 */ u8 timer;
    /* 0x52 */ u8 pad52[0xA8 - 0x52];
    /* 0xA8 */ s16 turn[2]; /* angle, speed */
} M148Enemy;

extern s32 D_800FE9F8;

extern s32 func_m003_0CAA4(f32, f32, s32);
extern void func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_04398(s32, s32);
extern void func_m003_07E88(s32, s32, s32);

void func_m148_00000(Object *obj) {
    M148Enemy *e = obj->unk68;

    if (func_m003_0CAA4(0.0f, 0.0f, 0x2000) != 0) {
        if (e->mission != 0x16) {
            func_m003_04260(4, 0x16, 0, 0, 0);
        }
    }
}

void func_m148_00060(Object *obj, f32 *x, f32 *y, f32 *z) {
    M148Enemy *e = obj->unk68;
    s16 *turn = e->turn;

    *x = e->x;
    *y = e->y;
    *z = e->z;
    switch (e->state) {
        case 1:
            turn[1] += D_800FE9F8 * 123;
            if (turn[1] >= 0x2619) {
                turn[1] = 0x2619;
                e->shots = 24;
                e->timer = 2;
                e->state = 4;
            }
            break;
        case 4:
            if (--e->timer == 0) {
                func_m003_07E88(0, 0, 0);
                e->flash ^= 1;
                e->timer = 2;
                if (--e->shots == 0) {
                    e->state = 5;
                }
            }
            break;
        case 5:
            turn[1] -= D_800FE9F8 * 123;
            if (turn[1] <= 0) {
                func_m003_04398(1, 0);
                turn[1] = 0;
            }
            break;
    }
    turn[0] += turn[1];
}
