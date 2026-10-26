/* Runlink module 134: an area that sets the weather while the player is inside it. */
#include "common.h"

typedef struct M134Setup {
    u8 pad0[0xA];
    s16 unkA; /* radius */
} M134Setup;

typedef struct M134Weather {
    u8 pad0[0xC];
    s16 unkC;
    s16 unkE;
    s16 unk10;
    u8 unk12;
    u8 unk13;
    s16 unk14;
} M134Weather;

typedef struct M134Object {
    u8 pad0[0xC];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x3C - 0x18];
    M134Weather *unk3C;
    u8 pad40[0x88 - 0x40];
    f32 unk88; /* radius squared */
} M134Object;

extern M134Object *func_8000E4D0(s32, M134Object *);
extern void func_8005B1C0(s32, s32, s32, s32, s32, s32);

void func_m134_00000(M134Object *obj, M134Setup *setup) {
    f32 radius = setup->unkA;

    radius *= radius;
    obj->unk88 = radius;
}

void func_m134_00020(M134Object *obj, s32 updateRate) {
    M134Object *player;
    M134Weather *weather;
    f32 dx;
    f32 dz;
    f32 radiusSq;

    player = func_8000E4D0(0, obj);
    if (player != NULL) {
        dx = obj->unkC - player->unkC;
        dz = obj->unk14 - player->unk14;
        radiusSq = obj->unk88;
        weather = obj->unk3C;
        if (dx * dx + dz * dz <= radiusSq) {
            func_8005B1C0(weather->unkC << 8, weather->unkE << 8, weather->unk10 << 8, weather->unk12 * 0x101,
                          weather->unk13 * 0x101, weather->unk14);
        }
    }
}
