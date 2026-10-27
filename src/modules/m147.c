/*
 * Runlink module 147: an enemy that swoops at the player. It picks a point
 * at a random angle around and above the player, flies there (rolling), then
 * closes in, fires a burst of six shots, picks a second point further out and
 * finally flies back to its home.
 */
#include "module.h"

/* Its home object's data (that object's unk68) */
typedef struct M147Home {
    /* 0x00 */ u8 pad0[0xE];
    /* 0x0E */ u8 attackChance;
} M147Home;

/* The part of module 3's enemy record that this enemy uses */
typedef struct M147Enemy {
    /* 0x00 */ u32 pad0_0 : 25;
    /* 0x00 */ u32 flash : 1;
    /* 0x00 */ u32 rolling : 1;
    /* 0x00 */ u32 pad0_1 : 5;
    /* 0x04 */ u8 pad4[0x15 - 0x4];
    /* 0x15 */ u8 homePoint;
    /* 0x16 */ u8 pad16[0x1A - 0x16];
    /* 0x1A */ u8 mission;
    /* 0x1B */ u8 state;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ Object *home;
    /* 0x28 */ u8 pad28[0x38 - 0x28];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0x50 - 0x44];
    /* 0x50 */ u8 shots;
    /* 0x51 */ u8 timer;
    /* 0x52 */ u8 rollTime;
    /* 0x53 */ s8 rollSpeed;
    /* 0x54 */ u8 pad54[0x64 - 0x54];
    /* 0x64 */ Object *target;
} M147Enemy;

/* What func_m003_0D2C0 returns */
typedef struct M147Gun {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ u32 pad24_0 : 25;
    /* 0x24 */ u32 alternate : 1; /* the shots alternate between the two sides */
    /* 0x24 */ u32 pad24_1 : 6;
} M147Gun;

extern s32 func_m003_0CAA4(f32, f32, s32);
extern s32 func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_04D08(s32, s32, s32, s32);
extern void func_m003_04398(s32, s32);
extern void func_m003_07E88(s32, s32, s32);
extern Object *func_m003_053C4(void);
extern M147Gun *func_m003_0D2C0(Object *obj);
extern void func_m003_06D28(Object *home, s32 point, f32 *x, f32 *y, f32 *z);
extern f32 point_distance(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2); /* distance */
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle); /* cosine */

void func_m147_00000(Object *obj) {
    M147Enemy *e = obj->unk68;
    Object *homeObj = e->home;
    M147Home *home = homeObj->unk68;
    s32 r;

    if (func_m003_0CAA4(0.0f, 0.0f, 0x7FFF) != 0) {
        r = func_80048E2C(0, 50);
        if (func_80048E2C(0, 50) + r < home->attackChance) {
            if (e->mission != 0x12) {
                if (func_m003_04260(5, 0x12, 0, 0, 0) != 0) {
                    func_m003_04D08(6, 0x12, 5, 0);
                }
            }
        }
    }
}

/* Where the enemy flies to (*x, *y, *z) */
void func_m147_000B8(Object *obj, f32 *x, f32 *y, f32 *z) {
    M147Enemy *e = obj->unk68;
    Object *player;
    M147Gun *gun;
    Object *home;
    s16 angle;
    s16 dist;

    player = func_m003_053C4();
    gun = func_m003_0D2C0(obj);
    home = e->home;
    e->target = player;
    if (e->rollTime != 0) {
        e->rollTime--;
        obj->trans.rotation.z_rotation += e->rollSpeed << 11;
    } else {
        e->rolling = 0;
    }
    switch (e->state) {
        case 1:
            angle = func_80048E2C(-0x2000, 0x2000) + player->trans.rotation.y_rotation;
            dist = func_80048E2C(550, 650);
            e->x = player->trans.x_position - func_80049C34(angle) * dist;
            e->y = func_80048E2C(60, 200) + player->trans.y_position;
            e->z = player->trans.z_position - func_80049C30(angle) * dist;
            e->state = 2;
            e->rollTime = 32;
            e->rollSpeed = func_80048E2C(-2, 3);
            if (e->rollSpeed == 0) {
                e->rollSpeed = -3;
            }
            e->rolling = 1;
            break;
        case 2:
            *x = e->x;
            *y = e->y;
            *z = e->z;
            if (point_distance(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, *x, *y, *z) < 200.0f) {
                e->state = 3;
            }
            break;
        case 3:
            *x = player->trans.x_position;
            *y = player->trans.y_position + 80.0f;
            *z = player->trans.z_position;
            if (point_distance(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, *x, *y, *z) < 600.0f) {
                e->state = 4;
                e->shots = 6;
                e->timer = 1;
            }
            break;
        case 4:
            *x = player->trans.x_position;
            *y = player->trans.y_position + 80.0f;
            *z = player->trans.z_position;
            if (--e->timer == 0) {
                func_m003_07E88(e->flash, 0, 0);
                if (gun->alternate) {
                    e->flash ^= 1;
                }
                e->timer = 2;
                if (--e->shots == 0) {
                    angle = func_80048E2C(-0x2000, 0x2000) + player->trans.rotation.y_rotation;
                    dist = 800;
                    e->x = player->trans.x_position - func_80049C34(angle) * dist;
                    e->y = func_80048E2C(600, 650) + player->trans.y_position;
                    e->z = player->trans.z_position - func_80049C30(angle) * dist;
                    e->state = 6;
                }
            }
            break;
        case 5:
            break;
        case 6:
            *x = e->x;
            *y = e->y;
            *z = e->z;
            if (point_distance(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, *x, *y, *z) < 200.0f) {
                e->state = 7;
            }
            break;
        case 7:
            func_m003_06D28(home, e->homePoint, x, y, z);
            if (point_distance(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, *x, *y, *z) < 200.0f) {
                func_m003_04398(1, 200);
            }
            break;
    }
}

/* How the enemy faces: 2 (towards the player?) until it has fired */
s32 func_m147_0056C(Object *obj, s32 arg1, s32 arg2, s32 arg3) {
    M147Enemy *e = obj->unk68;

    if (e->state < 5) {
        return 2;
    }
    return 0;
}
