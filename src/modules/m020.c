/* Runlink module 20: two object behaviours (a swaying object and a glowing object). */
#include "common.h"

typedef struct M020Setup {
    s16 unk0;
    u8 pad2[0xA - 0x2];
    u8 unkA;
    u8 unkB; /* size */
    u8 unkC; /* angle */
    u8 unkD;
} M020Setup;

typedef struct M020Header {
    f32 unk0;
    u8 pad4[0xA - 0x4];
    u16 unkA;
    u8 padC[0x1F - 0xC];
    s8 unk1F;
    u8 pad20[0x22 - 0x20];
    s8 unk22;
    u8 pad23[0x3C - 0x23];
    s32 unk3C;
    u8 pad40[0x54 - 0x40];
    f32 unk54;
} M020Header;

typedef struct M020Sway {
    s32 unk0;
    s16 unk4; /* timer */
    s16 unk6; /* swing */
} M020Sway;

typedef union M020Vars {
    s32 w[2];
    M020Sway sway;
} M020Vars;

typedef struct M020Object {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6; /* flags */
    f32 unk8; /* scale */
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x3A - 0x18];
    s8 unk3A;
    u8 pad3B[0x40 - 0x3B];
    M020Header *unk40;
    u8 pad44[0x4A - 0x44];
    s16 unk4A;
    M020Header *unk4C;
    f32 *unk50;
    u8 pad54[0x68 - 0x54];
    s32 *unk68;
    u8 pad6C[0x84 - 0x6C];
    s32 unk84;
    M020Vars unk88;
} M020Object;

extern void func_80079D1C(M020Object *, s32, s32, s32, s32, s32, s32, f32, f32, f32, f32, s32);
extern void func_80003410(s32, f32, f32, f32, s32, s32 *);
extern s32 func_80048E2C(s32, s32);
extern void func_8005F5AC(M020Object *, s32);
extern s32 func_8007E150(f32, f32, f32, f32, s32, void *);
extern void func_800079B8(M020Object *);

void func_m020_00000(M020Object *obj, M020Setup *setup) {
    f32 size;

    obj->unk6 |= 2;
    size = setup->unkB & 0xFF;
    if (size < 10) {
        size = 10;
    }
    size /= 64;
    obj->unk8 = obj->unk40->unk0 * size;
    if (obj->unk50 != NULL) {
        *obj->unk50 = obj->unk40->unk54 * size;
    }
    obj->unk3A = setup->unkA;
    obj->unk0 = (setup->unkC & 0xFF) << 10;
    if (setup->unkD != 0) {
        func_80079D1C(obj, -1, 2, 1, 0, 0, 0, 20.0f, 10.0f, -1.0f, 1.0f, 0x11);
    }
    if (obj->unk3A >= obj->unk40->unk1F) {
        obj->unk3A = 0;
    }
    obj->unk88.w[0] = 0;
    obj->unk88.w[1] = 0;
}

void func_m020_0012C(M020Object *obj, s32 updateRate) {
    M020Sway *sway;
    s32 shift;

    if (obj->unk4C != NULL) {
        sway = &obj->unk88.sway;
        if (sway->unk4 > 0) {
            sway->unk4 -= updateRate;
        }
        if ((obj->unk4C->unkA & 2) && sway->unk4 <= 0) {
            func_80003410(0, obj->unkC, obj->unk10, obj->unk14, 4, NULL);
            sway->unk0 = obj->unk4C->unk3C;
            sway->unk6 = 0x71C;
            sway->unk4 = 10;
            if (obj->unk40->unk22 < 2) {
                shift = 0;
            } else {
                shift = func_80048E2C(0, obj->unk40->unk22 - 1);
            }
            obj->unk84 = 1 << shift;
            func_8005F5AC(obj, 2);
        } else if (sway->unk4 <= 0) {
            sway->unk0 = 0;
        }
        obj->unk4 = sway->unk6;
        sway->unk6 = (sway->unk6 * -200) >> 8;
    }
}

void func_m020_00284(M020Object *obj, M020Setup *setup) {
    s32 *data = obj->unk68;

    obj->unk8 = *(f32 *) obj->unk40 * setup->unkB * 0.015625f;
    if (obj->unk8 < 0.01f) {
        obj->unk8 = 0.01f;
    }
    obj->unk88.w[0] = 0;
    obj->unk88.w[1] = 0;
    if (obj->unk4A == 0x1B2) {
        obj->unk88.w[0] = 720;
        obj->unk6 |= 0x400;
    }
    if (setup->unk0 == 0x1B2) {
        func_80003410(0x1BD, obj->unkC, obj->unk10, obj->unk14, 1, data);
    } else {
        *data = 0;
    }
}

void func_m020_0036C(M020Object *obj, s32 updateRate) {
    u8 hit[0x20];

    if (obj->unk88.w[0] > 180 && obj->unk88.w[1] == 0) {
        obj->unk88.w[0] = 180;
    }
    if ((obj->unk88.w[0] == 0 || obj->unk88.w[0] > 180) &&
        func_8007E150(obj->unkC, obj->unk10, obj->unk14, 600.0f, 0, hit) != 0) {
        obj->unk84 = 1;
    } else {
        obj->unk84 = 0;
    }
    func_8005F5AC(obj, updateRate);
    if (obj->unk88.w[0] != 0) {
        obj->unk88.w[0] -= updateRate;
        if (obj->unk88.w[0] <= 0) {
            func_800079B8(obj);
        }
    }
}
