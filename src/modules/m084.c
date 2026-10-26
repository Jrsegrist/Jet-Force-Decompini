/* Runlink module 84: an object that shrinks away and then fades out. */
#include "common.h"

typedef struct M084Setup {
    u8 pad0[0xA];
    s16 unkA;
    f32 unkC;
    f32 unk10;
} M084Setup;

typedef struct M084Object {
    s16 unk0;
    u8 pad2[0x39 - 0x2];
    u8 unk39; /* alpha */
    u8 pad3A[0x50 - 0x3A];
    f32 *unk50;
    u8 pad54[0x68 - 0x54];
    f32 *unk68;
} M084Object;

extern void func_800079B8(M084Object *);

void func_m084_00000(M084Object *obj, M084Setup *setup) {
    f32 *timer = obj->unk68;

    *obj->unk50 *= setup->unk10;
    obj->unk0 = setup->unkA;
    *timer = setup->unkC;
}

void func_m084_00030(M084Object *obj, s32 updateRate) {
    f32 *timer = obj->unk68;
    s32 fade;

    if (*timer > 0.0f) {
        *timer -= updateRate / 60.0f;
    } else {
        fade = updateRate * 2;
        if (obj->unk39 < fade) {
            obj->unk39 = 0;
        } else {
            obj->unk39 -= fade;
        }
        if (obj->unk39 == 0) {
            func_800079B8(obj);
        }
    }
}
