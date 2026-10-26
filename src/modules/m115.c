/*
 * Runlink module 115: a pad that switches night vision on or off when the
 * player presses the button on it, showing a cut-scene camera meanwhile.
 */
#include "module.h"

typedef struct M115Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u16 radius;
    /* 0x0C */ s8 minY;
    /* 0x0D */ s8 maxY;
    /* 0x0E */ u8 cameraOn;
    /* 0x0F */ u8 cameraOff;
} M115Setup;

typedef struct M115Pad {
    /* 0x0 */ f32 radiusSq;
    /* 0x4 */ f32 minY;
    /* 0x8 */ f32 maxY;
    /* 0xC */ u8 cameraOn;
    /* 0xD */ u8 cameraOff;
    /* 0xE */ u8 state;
    /* 0xF */ u8 timer;
} M115Pad;

typedef struct M115PlayerData {
    /* 0x000 */ s8 unk0;
    /* 0x001 */ s8 unk1;
    /* 0x002 */ u8 pad2[0x5C0 - 0x2];
    /* 0x5C0 */ Object *cutCamera;
} M115PlayerData;

extern s16 D_80104702;
extern s16 D_80104704;

extern s32 func_8006EE50(void); /* night vision on */
extern void func_8006EDF0(s32);
extern void func_8006EE28(s32);
extern u16 func_80043510(s32 player); /* buttons pressed */
extern s32 func_80047D80(s32, s32, s32);
extern void *func_80041E0C(void);
extern void func_80001248(s32 sound, s32 arg1); /* DKR amSndPlay */
extern void func_m085_00074(s32 camera);
extern void func_m085_00250(void *view, Object *camera, s32 updateRate);
extern void func_m077_00000(s32, s32);

void func_m115_00000(Object *obj, M115Setup *setup) {
    M115Pad *p = obj->unk68;
    f32 r = setup->radius;

    p->radiusSq = r * r;
    p->minY = obj->trans.y_position + setup->minY;
    p->maxY = obj->trans.y_position + setup->maxY;
    p->cameraOn = setup->cameraOn;
    p->cameraOff = setup->cameraOff;
    p->state = 0;
    p->timer = 0;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
}

void func_m115_000E4(Object *obj, s32 updateRate) {
    s32 on;
    M115Pad *p;
    f32 dx;
    f32 dz;
    f32 y;
    M115PlayerData *pd;
    Object *player;

    player = func_8000E4D0(0);
    if (player == NULL) {
        return;
    }
    pd = player->unk68;
    p = obj->unk68;
    on = func_8006EE50();
    D_80104704 = on * 6 + 0x40;
    if (D_80104704 >= 0x100) {
        D_80104704 = 0xFF;
    }
    D_80104702 = D_80104704;
    switch (p->state) {
        case 0:
            if (func_80043510(0) & 0x8000) {
                if (func_80047D80(pd->unk0, pd->unk1, 0x16) != 0) {
                    dx = player->trans.x_position - obj->trans.x_position;
                    dz = player->trans.z_position - obj->trans.z_position;
                    y = player->trans.y_position;
                    if (dx * dx + dz * dz < p->radiusSq && p->minY < y && y < p->maxY) {
                        if (on) {
                            p->state = 2;
                            p->timer = 60;
                            func_m085_00074(p->cameraOn);
                            func_m077_00000(p->cameraOn, 0);
                        } else {
                            p->state = 1;
                            p->timer = 60;
                            func_m085_00074(p->cameraOff);
                            func_m077_00000(p->cameraOff, 0);
                        }
                    }
                }
            }
            break;
        case 1:
            if (pd->cutCamera == NULL) {
                p->state = 3;
                func_m085_00074(p->cameraOn);
                if (pd->cutCamera != NULL) {
                    func_m085_00250(func_80041E0C(), pd->cutCamera, updateRate);
                }
                func_m077_00000(p->cameraOn, 1);
                func_8006EDF0(0);
                func_80001248(0x1EF, 0);
            }
            break;
        case 2:
            if (pd->cutCamera == NULL) {
                p->state = 3;
                func_m085_00074(p->cameraOff);
                if (pd->cutCamera != NULL) {
                    func_m085_00250(func_80041E0C(), pd->cutCamera, updateRate);
                }
                func_m077_00000(p->cameraOff, 1);
                func_8006EE28(0);
                func_80001248(0x1EF, 0);
            }
            break;
        case 3:
            if (pd->cutCamera == NULL) {
                p->state = 0;
            }
            break;
    }
}
