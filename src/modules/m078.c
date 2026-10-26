/* Runlink module 78: an object that switches its triggers on at random. */
#include "common.h"

typedef struct M078Setup {
    u8 pad0[0xA];
    u8 unkA;
    u8 unkB;
    u16 unkC;
} M078Setup;

typedef struct M078Object {
    u8 pad0[0x70];
    s32 unk70;
    u8 pad74[0x84 - 0x74];
    s32 unk84;
    s32 unk88;
} M078Object;

extern void func_8005EDA4(s32, s32, s32, s32, s32, s32);
extern s32 func_80048E2C(s32, s32);
extern void func_8005F5AC(M078Object *, s32);

void func_m078_00000(M078Object *obj, M078Setup *setup) {
    func_8005EDA4(obj->unk70, setup->unkB, setup->unkA, 0, 0, 0);
    obj->unk88 = setup->unkC;
}

void func_m078_00054(M078Object *obj, s32 updateRate) {
    if (obj->unk88 >= func_80048E2C(0, 0x400)) {
        obj->unk84 = 1;
    } else {
        obj->unk84 = 0;
    }
    func_8005F5AC(obj, updateRate);
}
