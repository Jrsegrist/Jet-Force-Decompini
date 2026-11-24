/*
 * Runlink module 119: a pickup that is collected when the player comes close.
 * Family: other object behaviours (docs/runlink.md, "Module families").
 */
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

extern void runlink_unload_timer_set(s32 module, u16 timer, u8 value);
extern M119Object *func_8000E4D0(s32);
extern M119Counter *char_record_lookup(s32, s32);
extern void func_80001248(u16 sound, s32 arg1); /* DKR amSndPlay */
extern void func_800079B8(M119Object *);

s32 D_m119_00100 = 119; /* the module's number */

/* Control (every frame): spins; within 40 units of the player, adds one to its counter, plays a sound, frees */
void func_m119_00000(M119Object *obj, s32 updateRate) {
    M119Object *player;
    M119PlayerData *data;
    M119Counter *counter;
    f32 dx;
    f32 dy;
    f32 dz;

    runlink_unload_timer_set(D_m119_00100, 2, 2);
    obj->unk0 += updateRate << 8;
    player = func_8000E4D0(0);
    if (player != NULL) {
        data = player->unk68;
        if (data != NULL) {
            dx = obj->unkC - player->unkC;
            dy = obj->unk10 - player->unk10;
            dz = obj->unk14 - player->unk14;
            if (dx * dx + dy * dy + dz * dz < 1600.0f) {
                counter = char_record_lookup(data->unk0, data->unk1);
                if (counter->unk12 < 32000) {
                    counter->unk12++;
                }
                func_80001248(0x1EB, 0);
                func_800079B8(obj);
            }
        }
    }
}
