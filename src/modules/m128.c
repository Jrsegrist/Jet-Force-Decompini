/* Runlink module 128: a pole that the player sets off by walking up to it. */
#include "module.h"

typedef struct M128Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u16 flag;
    /* 0x0C */ u8 scale;
    /* 0x0D */ u8 angle;
    /* 0x0E */ u8 padE;
    /* 0x0F */ u8 radius;
    /* 0x10 */ s8 minY;
    /* 0x11 */ s8 maxY;
} M128Setup;

typedef struct M128Pole {
    /* 0x0 */ u16 flag;
    /* 0x2 */ s8 state; /* 1: set off */
    /* 0x4 */ f32 radius;
    /* 0x8 */ f32 minY;
    /* 0xC */ f32 maxY;
} M128Pole;

extern void func_80001170(s32);
extern void func_8003F1C8(void *, s32, s32, Object *);

void func_m128_00000(Object *obj, M128Setup *setup) {
    M128Pole *pole = obj->unk68;

    obj->trans.scale = obj->header->scale * (setup->scale * 0.05f);
    obj->trans.rotation.y_rotation = setup->angle << 8;
    pole->flag = setup->flag;
    if (func_80047ED0(setup->flag) != 0) {
        func_8001149C(obj, 0, 1.0f);
        pole->state = 1;
    } else {
        func_8001149C(obj, 0, 0.0f);
        pole->state = 0;
    }
    pole->radius = setup->radius;
    pole->minY = obj->trans.y_position + setup->minY;
    pole->maxY = obj->trans.y_position + setup->maxY;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
}

void func_m128_00188(Object *obj, s32 updateRate) {
    Object *player;
    void *playerData;
    M128Pole *pole = obj->unk68;
    f32 dx;
    f32 dz;
    f32 y;

    switch (pole->state) {
        case 0:
            player = func_8000E4D0(0);
            playerData = player->unk68;
            dx = player->trans.x_position - obj->trans.x_position;
            dz = player->trans.z_position - obj->trans.z_position;
            y = player->trans.y_position;
            if (dx * dx + dz * dz < pole->radius * pole->radius && y < pole->maxY && pole->minY < y) {
                pole->state = 1;
                func_80047EFC(pole->flag, 1);
                func_80001170(0x1D);
                func_m006_011E0(playerData, 0x1BC);
            }
            break;
        case 1:
            func_8003F1C8(*obj->unk6C, 0, 0x100, obj);
            func_8001138C(obj, 0.017f, updateRate);
            break;
    }
}
