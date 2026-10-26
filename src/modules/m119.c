/* Runlink module 119: a pickup that is collected when the player comes close. */
#include "common.h"

typedef struct M119Counter {
    u8 pad0[0x12];
    s16 unk12;
} M119Counter;

typedef struct M119PlayerData {
    s8 unk0;
    s8 unk1;
} M119PlayerData;

typedef struct M119Object {
    s16 unk0;
    u8 pad2[0xC - 0x2];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x68 - 0x18];
    M119PlayerData *unk68;
} M119Object;

extern void func_800541B0(s32, s32, s32);
extern M119Object *func_8000E4D0(s32);
extern M119Counter *func_80047C1C(s32, s32);
extern void func_80001248(s32, s32);
extern void func_800079B8(M119Object *);

s32 D_m119_00100 = 119; /* the module's number */

void func_m119_00000(M119Object *obj, s32 updateRate) {
    M119Object *player;
    M119PlayerData *data;
    M119Counter *counter;
    f32 dx;
    f32 dy;
    f32 dz;

    func_800541B0(D_m119_00100, 2, 2);
    obj->unk0 += updateRate << 8;
    player = func_8000E4D0(0);
    if (player != NULL) {
        data = player->unk68;
        if (data != NULL) {
            dx = obj->unkC - player->unkC;
            dy = obj->unk10 - player->unk10;
            dz = obj->unk14 - player->unk14;
            if (dx * dx + dy * dy + dz * dz < 1600.0f) {
                counter = func_80047C1C(data->unk0, data->unk1);
                if (counter->unk12 < 32000) {
                    counter->unk12++;
                }
                func_80001248(0x1EB, 0);
                func_800079B8(obj);
            }
        }
    }
}
