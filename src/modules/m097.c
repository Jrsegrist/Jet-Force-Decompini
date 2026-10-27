/* Runlink module 97: a fragment that flies off when something is hit, tumbling and bouncing until it fades. */
#include "module.h"

typedef struct M097Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ Vec3s rotation; /* z_rotation is also the lifetime */
    /* 0x10 */ f32 scale;
    /* 0x14 */ f32 speed;
    /* 0x18 */ f32 gravity;
    /* 0x1C */ s8 unk1C;
    /* 0x1D */ u8 pad1D[0x20 - 0x1D];
    /* 0x20 */ s32 unk20;
} M097Setup;

typedef struct M097Fragment {
    /* 0x0 */ f32 gravity;
    /* 0x4 */ s16 timer;
    /* 0x6 */ u8 spin;
    /* 0x7 */ u8 alpha;
    /* 0x8 */ s8 unk8;
    /* 0x9 */ u8 pad9[0xC - 0x9];
    /* 0xC */ s32 unkC;
} M097Fragment;

typedef struct M097Level {
    /* 0x00 */ u8 pad0[0xE2];
    /* 0xE2 */ u8 unkE2;
} M097Level;

extern void runlink_unload_timer_set(s32, s32, s32);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern M097Level *func_8004470C(void); /* DKR levelGetLevel */

s32 D_m097_003C0 = 97; /* the module's number */

void func_m097_00000(Object *obj, M097Setup *setup) {
    M097Fragment *f = obj->unk68;

    obj->velocity.z = -setup->speed;
    func_8004944C(&setup->rotation, &obj->velocity);
    obj->trans.scale = obj->header->scale * setup->scale;
    f->gravity = setup->gravity;
    f->timer = setup->rotation.z_rotation;
    f->spin = func_80048E2C(8, 16);
    f->alpha = 0xFF;
    f->unk8 = setup->unk1C;
    f->unkC = setup->unk20;
    if (obj->interactObj != NULL) {
        func_80079D1C(obj, 0x100, 1, 4, 0, 0, 0, 200.0f, 200.0f, -1.0f, 1.0f, 0x11);
    }
}

void func_m097_00100(Object *obj, s32 updateRate) {
    s32 spin;
    HitResult hit;
    M097Fragment *f;
    Vec3f oldPos;
    f32 rate;
    s32 sound;
    s32 mode;

    runlink_unload_timer_set(D_m097_003C0, 2, 2);
    f = obj->unk68;
    f->timer -= updateRate;
    spin = f->spin * updateRate;
    obj->trans.rotation.y_rotation += spin << 6;
    obj->trans.rotation.x_rotation += spin << 7;
    obj->trans.rotation.z_rotation += spin << 5;
    if (obj->interactObj != NULL && obj->interactObj->hitPoints <= 0) {
        mode = 3;
    } else if (f->timer <= 0) {
        mode = 4;
    } else {
        mode = 0;
    }
    if (mode == 0) {
        rate = updateRate;
        if (f->timer < 16) {
            f->alpha = f->timer << 4;
        }
        oldPos.x = obj->trans.x_position;
        oldPos.y = obj->trans.y_position;
        oldPos.z = obj->trans.z_position;
        func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
        obj->velocity.y -= rate * f->gravity;
        obj->particleEmittersEnabled = 1;
        obj_spawn_particle(obj, updateRate);
        if (func_80018808(obj, &oldPos, (Vec3f *) &obj->trans.x_position, &hit, 0, 0) != 0) {
            func_80009A04(obj, hit.pos.x - obj->trans.x_position, hit.pos.y - obj->trans.y_position,
                          hit.pos.z - obj->trans.z_position);
            obj->trans.rotation.y_rotation = 0;
            obj->trans.rotation.z_rotation = obj->trans.rotation.x_rotation = obj->trans.rotation.y_rotation;
            mode = 1;
            if ((hit.flags & 0x2000) && !(hit.flags & 0x8000)) {
                mode = 2;
            }
        }
    }
    switch (mode) {
        default:
            obj->particleEmittersEnabled = 2;
            sound = -1;
            break;
        case 0:
            return;
        case 1:
            obj->particleEmittersEnabled = 2;
            if (func_8004470C()->unkE2 == 0) {
                sound = 0x16;
            } else {
                sound = 0xF;
            }
            break;
        case 2:
            obj->particleEmittersEnabled = 12;
            sound = 0x9E;
            break;
        case 3:
            obj->particleEmittersEnabled = 2;
            sound = 0xF;
            break;
    }
    if (sound >= 0) {
        func_80003410(sound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
    }
    obj_spawn_particle(obj, updateRate);
    func_800079B8(obj);
}
