/* Runlink module 70: sets up an object's state from the screen size and two counters. */
#include "common.h"

typedef struct M070Setup {
    u8 pad0[0x10];
    u8 unk10;
    u8 unk11;
} M070Setup;

typedef struct M070State {
    u8 unk0;
    u8 unk1;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    f32 unk10; /* half the screen width */
    f32 unk14; /* half the screen height */
    f32 unk18;
    f32 unk1C;
} M070State;

typedef struct M070Object {
    u8 pad0[0x68];
    M070State *unk68;
} M070Object;

extern void func_80054EA0(u32 *, u32 *);
extern s8 D_800A5018;
extern s8 D_800A501C;

void func_m070_00000(M070Object *obj, M070Setup *setup) {
    u32 width;
    u32 height;
    M070State *state;

    func_80054EA0(&width, &height);
    state = obj->unk68;
    state->unk0 = setup->unk10;
    state->unk1 = setup->unk11;
    state->unk2 = 0;
    state->unk4 = 0;
    state->unk6 = 0;
    state->unk8 = 0;
    state->unkC = D_800A5018;
    state->unkE = D_800A501C;
    state->unk10 = width >> 1;
    state->unk14 = height >> 1;
    state->unk18 = 0.0f;
    state->unk1C = 0.0f;
}
