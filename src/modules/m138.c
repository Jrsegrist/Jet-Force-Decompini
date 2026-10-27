/*
 * Runlink module 138: the movement of a flying enemy that circles the
 * player at a distance, closes in, and fires when it is near.
 */
#include "module.h"

typedef struct M138Ship {
    /* 0x0 */ u16 timer;
    /* 0x2 */ u16 passes;
    /* 0x4 */ u16 attacking;
    /* 0x6 */ s16 angle;
} M138Ship;

/* The part of module 3's enemy record that this enemy uses */
typedef struct M138Enemy {
    /* 0x00 */ u32 pad0_0 : 25;
    /* 0x00 */ u32 flash : 1;
    /* 0x00 */ u32 pad0_1 : 6;
    /* 0x04 */ u8 pad4[0x38 - 0x4];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0x51 - 0x44];
    /* 0x51 */ u8 timer;
    /* 0x52 */ u8 pad52[0xA8 - 0x52];
    /* 0xA8 */ M138Ship ship;
} M138Enemy;

extern s32 D_800FE9F8;

extern Object *func_m003_053C4(void); /* the closest player */
extern void func_m003_07E88(s32, s32, s32);
extern void func_800541B0(s32, s32, s32);
extern s32 func_80005064(s32);
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle); /* cosine */

s32 D_m138_002F0 = 138; /* the module's number */

void func_m138_00000(Object *obj) {
    M138Enemy *e = obj->unk68;
    Object *player;
    M138Ship *s;
    u32 dist;

    s = &e->ship;
    player = func_m003_053C4();
    dist = (obj->trans.x_position - player->trans.x_position) * (obj->trans.x_position - player->trans.x_position) +
           (obj->trans.z_position - player->trans.z_position) * (obj->trans.z_position - player->trans.z_position);
    func_800541B0(D_m138_002F0, 2, 2);
    if (s->attacking != 0) {
        e->x = player->trans.x_position;
        e->y = player->trans.y_position + 400.0f;
        e->z = player->trans.z_position;
        if (dist < 360000) {
            s->attacking = 0;
            s->passes = func_80048E2C(7, 12);
        } else {
            if (dist < 5760000) {
                if (D_800FE9F8 < e->timer) {
                    e->timer -= D_800FE9F8;
                } else {
                    func_m003_07E88(e->flash, 0, 0);
                    e->flash ^= 1;
                    e->timer = func_80005064(12);
                }
                e->y = player->trans.y_position + 200.0f;
            }
            return;
        }
    } else if (D_800FE9F8 < s->timer) {
        s->timer -= D_800FE9F8;
        goto move;
    }
    if (s->passes == 0) {
        s->attacking = 1;
        e->timer = 0;
        return;
    }
    s->passes--;
    s->angle = func_80048E2C(-7500, 7500) - 0x8000;
    s->timer = func_80005064(func_80048E2C(25, 40));
move:
    e->x = func_80049C34(obj->trans.rotation.y_rotation + s->angle) * 1000.0f + obj->trans.x_position;
    e->y = player->trans.y_position + 400.0f;
    e->z = func_80049C30(obj->trans.rotation.y_rotation + s->angle) * 1000.0f + obj->trans.z_position;
    obj->trans.rotation.x_rotation = 0;
}
