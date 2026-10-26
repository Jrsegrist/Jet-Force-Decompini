/* Runlink module 103: a small creature that shrinks while it is hurt and dies at zero health. */
#include "common.h"

typedef struct M103Setup {
    u8 pad0[0xA];
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
} M103Setup;

typedef struct M103Health {
    u8 pad0[0x6];
    s16 unk6;
} M103Health;

typedef struct M103State {
    f32 unk0;
    s16 unk4; /* last health */
    s8 unk6;  /* hurt timer */
    u8 unk7;
} M103State;

typedef struct M103Object {
    u8 pad0[0x8];
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x4C - 0x18];
    M103Health *unk4C;
    u8 pad50[0x84 - 0x50];
    s32 unk84;
    M103State unk88;
} M103Object;

extern void func_80079D1C(M103Object *, s32, s32, s32, s32, s32, s32, f32, f32, f32, f32, s32);
extern void func_800541B0(s32, s32, s32);
extern void func_8001138C(M103Object *, f32, f32);
extern void func_80011708(f32, f32, f32, s32);
extern void func_8005F5AC(M103Object *, s32);
extern void func_800079B8(M103Object *);

s32 D_m103_00240 = 103; /* the module's number */

void func_m103_00000(M103Object *obj, M103Setup *setup) {
    M103State *state = &obj->unk88;

    obj->unk8 = setup->unkA * 0.01f;
    func_80079D1C(obj, setup->unkB << 8, 1, 1, 0, 0, 0, obj->unk8 * 20.0f, obj->unk8 * 20.0f, -1.0f, 1.0f, 0x11);
    state->unk0 = setup->unkD * 0.001f;
    state->unk4 = obj->unk4C->unk6;
    state->unk6 = 0;
    state->unk7 = setup->unkC;
}

void func_m103_00110(M103Object *obj, s32 updateRate) {
    M103State *state;

    func_800541B0(D_m103_00240, 2, 2);
    state = &obj->unk88;
    if (obj->unk4C->unk6 > 0) {
        if (obj->unk4C->unk6 < state->unk4) {
            state->unk6 = 15;
        }
        state->unk4 = obj->unk4C->unk6;
        if (state->unk6 > 0) {
            func_8001138C(obj, 0.0667f, updateRate);
            state->unk6 -= updateRate;
        } else {
            func_8001138C(obj, 0.01f, updateRate);
        }
    } else {
        func_80011708(obj->unkC, obj->unk10, obj->unk14, obj->unk88.unk7);
        obj->unk84 = 1;
        func_8005F5AC(obj, 2);
        func_800079B8(obj);
    }
}
