/* Runlink module 74: a switch that turns an animation on and off. */
#include "common.h"

typedef struct M074Setup {
    u8 pad0[0xA];
    u8 unkA;
    u8 unkB;
    u8 unkC; /* mode */
    u8 unkD; /* size */
    s16 unkE;
    s16 unk10;
    u16 unk12; /* time */
} M074Setup;

typedef struct M074Header {
    u8 pad0[0x1F];
    s8 unk1F;
} M074Header;

typedef struct M074Hit {
    u8 pad0[0x6];
    s16 unk6;
} M074Hit;

typedef struct M074Switch {
    u8 on;
    u8 unk1; /* animation */
    u8 mode; /* 1: on for a time */
    s16 unk4; /* time */
    s16 unk6; /* time left */
} M074Switch;

typedef struct M074Object {
    s16 unk0;
    s16 unk2;
    u8 pad4[0x8 - 0x4];
    f32 unk8;
    u8 padC[0x3A - 0xC];
    u8 unk3A;
    u8 pad3B[0x40 - 0x3B];
    M074Header *unk40;
    u8 pad44[0x4C - 0x44];
    M074Hit *unk4C;
    u8 pad50[0x88 - 0x50];
    M074Switch sw;
} M074Object;

extern void func_80079D1C(M074Object *, s32, s32, s32, s32, s32, s32, f32, f32, f32, f32, s32);
/* called with the sequence only: no prototype in scope */
extern void func_800767EC();
extern void func_80076928();

void func_m074_00000(M074Object *obj, M074Setup *setup) {
    obj->sw.on = setup->unkA & 1;
    obj->sw.unk1 = setup->unkB;
    obj->sw.mode = setup->unkC;
    obj->sw.unk4 = setup->unk12 * 6;
    obj->sw.unk6 = obj->sw.unk4;
    obj->unk0 = setup->unkE;
    obj->unk2 = setup->unk10;
    obj->unk8 = setup->unkD * 0.05f;
    if (obj->unk40->unk1F > 0 && obj->sw.on != 0) {
        obj->unk3A = 1;
    }
    func_80079D1C(obj, 0x100, 1, 1, 0, 0, 0, obj->unk8 * 8.0f, obj->unk8 * 8.0f, -1.0f, 1.0f, 1);
}

void func_m074_00110(M074Object *obj, s32 updateRate) {
    M074Switch *sw;
    u8 on;
    s32 one = 1; /* a variable: IDO keeps it in a register for the whole function */

    sw = &obj->sw;
    if (sw->mode == one) {
        on = sw->on;
        if (on) {
            sw->unk6 -= updateRate;
            if (sw->unk6 <= 0) {
                sw->on = 0;
            }
            on = sw->on;
        } else if (obj->unk4C->unk6 != 0x100) {
            on = sw->on = 1;
            sw->unk6 = sw->unk4;
        }
    } else {
        if (obj->unk4C->unk6 != 0x100) {
            sw->on ^= 1;
        }
        on = sw->on;
    }
    if (on) {
        if (obj->unk40->unk1F > 0) {
            obj->unk3A = one;
        }
        func_800767EC(sw->unk1);
    } else {
        obj->unk3A = 0;
        func_80076928(sw->unk1);
    }
    obj->unk4C->unk6 = 0x100;
}
