/* Runlink module 108: a key that the player picks up. */
#include "common.h"

typedef struct M108Setup {
    u8 pad0[0xB];
    u8 unkB; /* size */
} M108Setup;

typedef struct M108PlayerData {
    s8 unk0;
    s8 unk1;
} M108PlayerData;

typedef struct M108Object {
    s16 unk0;
    u8 pad2[0x8 - 0x2];
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x40 - 0x18];
    f32 *unk40;
    u8 pad44[0x4A - 0x44];
    s16 unk4A;
    u8 pad4C[0x68 - 0x4C];
    void *unk68;
} M108Object;

extern void func_80079D1C(M108Object *, s32, s32, s32, s32, s32, s32, f32, f32, f32, f32, s32);
extern void runlink_unload_timer_set(s32, s32, s32);
extern M108Object *func_8000E4D0(s32);
extern s32 character_has_item(s32, s32, s32);
extern void character_item_give(s32, s32, s32);
extern void func_80001248(s32, s32);
extern void func_800079B8(M108Object *);
extern void func_m006_011E0(M108PlayerData *, s32);

s32 D_m108_00220 = 108; /* the module's number */

void func_m108_00000(M108Object *obj, M108Setup *setup) {
    u8 *key = obj->unk68;
    f32 scale;

    *key = obj->unk4A - 600;
    scale = setup->unkB * 0.015625f;
    if (scale < 0.1f) {
        scale = 0.1f;
    }
    obj->unk8 = *obj->unk40 * scale;
    func_80079D1C(obj, -1, 1, 1, 0, 0, 0, 30.0f, 30.0f, -1.0f, 1.0f, 0x11);
}

void func_m108_000CC(M108Object *obj, s32 updateRate) {
    s32 pad0[2]; /* the unused stack slots come from locals the original had */
    M108PlayerData *data;
    M108Object *player;
    s32 pad1;
    u8 *key;
    s32 pad2;
    f32 dx;
    f32 dz;

    runlink_unload_timer_set(D_m108_00220, 2, 2);
    key = obj->unk68;
    obj->unk0 += updateRate << 7;
    player = func_8000E4D0(0);
    if (player != NULL) {
        data = player->unk68;
        if (character_has_item(data->unk0, data->unk1, *key) != 0) {
            func_800079B8(obj);
        } else {
            dx = player->unkC - obj->unkC;
            dz = player->unk14 - obj->unk14;
            if (dx * dx + dz * dz < 1024.0f) {
                if (obj->unk10 + -45.0f < player->unk10 && player->unk10 < obj->unk10 + 45.0f) {
                    character_item_give(data->unk0, data->unk1, *key);
                    func_80001248(0x1F4, 0);
                    func_m006_011E0(data, *key + 0x170);
                }
            }
        }
    }
}
