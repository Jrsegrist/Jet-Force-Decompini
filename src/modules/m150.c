/* Runlink module 150: an enemy with a hand that follows one of its points, and its laser beam. */
#include "module.h"
#include "gfx.h"

/* The part of module 3's enemy record that this enemy uses */
typedef struct M150Enemy {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ Object *hand;
} M150Enemy;

typedef struct M150Hand {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ Vec3f target;
    /* 0x10 */ Vec3f pos;
} M150Hand;

typedef struct M150BeamSetup {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ f32 startX;
    /* 0x10 */ f32 startY;
    /* 0x14 */ f32 startZ;
    /* 0x18 */ u8 size;
    /* 0x19 */ u8 pad19[0x1C - 0x19];
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
} M150BeamSetup;

typedef struct M150Beam {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ Vec3f end;
    /* 0x10 */ Vec3f start;
    /* 0x1C */ u8 size;
    /* 0x1D */ u8 pad1D[0x20 - 0x1D];
    /* 0x20 */ Triangle tris[2];
    /* 0x40 */ u8 pad40[0x80 - 0x40];
    /* 0x80 */ u8 unk80;
    /* 0x81 */ u8 pad81[0x88 - 0x81];
    /* 0x88 */ s32 unk88;
    /* 0x8C */ s32 unk8C;
    /* 0x90 */ s32 sound;
} M150Beam;

extern void func_m003_0609C(s32, f32 *, f32 *, f32 *);
extern void func_m003_0CD34(Object *);
extern void func_800035E8(s32 handle, f32 x, f32 y, f32 z); /* moves a positioned sound */

void func_m150_00000(Object *obj) {
    M150Enemy *e = obj->unk68;
    Object *hand;
    M150Hand *h;
    f32 x;
    f32 y;
    f32 z;

    hand = e->hand;
    if (hand != NULL) {
        h = hand->unk68;
        func_m003_0609C(0, &x, &y, &z);
        h->target.x = x;
        h->target.y = y;
        h->target.z = z;
        hand->trans.x_position = h->pos.x;
        hand->trans.y_position = h->pos.y;
        hand->trans.z_position = h->pos.z;
    }
    func_m003_0CD34(obj);
}

void func_m150_00098(Object *obj, M150BeamSetup *setup) {
    M150Beam *b = obj->unk68;

    b->tris[0].flags = 0x40;
    b->tris[0].vi0 = 0;
    b->tris[0].uv[0][0] = 0;
    b->tris[0].uv[0][1] = 0;
    b->tris[0].vi1 = 1;
    b->tris[0].uv[1][0] = 0;
    b->tris[0].uv[1][1] = 0;
    b->tris[0].vi2 = 2;
    b->tris[0].uv[2][0] = 0;
    b->tris[0].uv[2][1] = 0;
    b->tris[1].flags = 0x40;
    b->tris[1].vi0 = 1;
    b->tris[1].uv[0][0] = 0;
    b->tris[1].uv[0][1] = 0;
    b->tris[1].vi1 = 2;
    b->tris[1].uv[1][0] = 0;
    b->tris[1].uv[1][1] = 0;
    b->tris[1].vi2 = 3;
    b->tris[1].uv[2][0] = 0;
    b->tris[1].uv[2][1] = 0;
    b->unk0 = setup->unk1C;
    b->end.x = setup->x;
    b->end.y = setup->y;
    b->end.z = setup->z;
    obj->trans.x_position = b->start.x = setup->startX;
    obj->trans.y_position = b->start.y = setup->startY;
    obj->trans.z_position = b->start.z = setup->startZ;
    b->size = setup->size;
    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    obj->velocity.z = 0.0f;
    b->unk80 = 0;
    b->unk88 = setup->unk20;
    b->unk8C = setup->unk24;
    func_80079D1C(obj, setup->size << 8, 1, 4, 0, 0, 0, 4.0f, 4.0f, -1.0f, 1.0f, 0x11);
}

void func_m150_00210(Object *obj, s32 updateRate) {
    M150Beam *b = obj->unk68;
    u8 hit[0x28];

    if (b->sound == 0) {
        func_80003410(0x125, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &b->sound);
    } else {
        func_800035E8(b->sound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    }
    func_8001CE10(1, &b->end, (Vec3f *) &obj->trans.x_position, &obj->header->unk50->radius, 0xCE000000, 0, 0);
    func_80016208(obj, &b->end, &b->start, obj->header->unk50->radius, obj->header->unk50->unk10, hit);
    obj->particleEmittersEnabled = 1;
    obj_spawn_particle(obj, updateRate);
}
