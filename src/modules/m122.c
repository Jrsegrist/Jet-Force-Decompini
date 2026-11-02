/*
 * Runlink module 122: a thrown spinning blade. Init (at most ten of them
 * exist: an eleventh frees the oldest) places it at the object, loads its
 * model and sets its hit box; control (still assembly) flies it at its
 * target; the matrix function puts it in its target's hand or at its own
 * rotation, and the bounce function knocks it back with a random spin.
 */
#include "module.h"

/*
 * The holder's obj->unk5C as the matrix function sees it: joint matrices
 * from 0x80, and at 0x12B the index of the one the blade is held by.
 */
typedef struct M122Joints {
    /* 0x00 */ u8 pad0[0x80];
    /* 0x80 */ MtxF mtx[4];
} M122Joints;

/* Kept in obj->unk68 */
typedef struct M122State {
    /* 0x00 */ s32 modelId;
    /* 0x04 */ void *model;
    /* 0x08 */ Vec3f pos;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ u8 pad20[0x24 - 0x20];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x30 - 0x2C];
    /* 0x30 */ Object *holder;
    /* 0x34 */ MtxF mtx; /* relative to the holder's hand */
    /* 0x74 */ s16 unk74;
    /* 0x76 */ u8 pad76[0x78 - 0x76];
    /* 0x78 */ s16 angle;
    /* 0x7A */ s16 spin[3];
    /* 0x80 */ s16 timer;
    /* 0x82 */ u8 flags;
    /* 0x83 */ u8 pad83;
    /* 0x84 */ u32 age;
} M122State;

/* What init gets */
typedef struct M122Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ s32 modelId;
} M122Setup;

extern void *func_80026F38(s32 id, u8 *arg1); /* loads a model */
extern void obj_list_insert_after(Object *obj, void *model);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out); /* out = a * b */
extern void func_80049DC0(Object *obj, MtxF *out);
extern void func_80049AE8(s32 angle, MtxF *mtx);
extern void mtx_model_push(Gfx **dl, Mtx **mtx, MtxF *m);

void func_m122_00000(Object *obj, M122Setup *setup) {
    M122State *s;
    s32 first;
    s32 last;
    Object **objects;
    s32 count;
    u32 maxAge;
    u8 unused;
    s32 i;
    Object *other;
    Object *oldest;

    s = obj->unk68;
    count = 0;
    maxAge = 0;
    objects = func_80005E98(&first, &last);
    for (i = first; i < last; i++) {
        other = objects[i];
        if (other->behaviorId == 0x99 && !other->freed) {
            count++;
            if (maxAge < ((M122State *) other->unk68)->age) {
                oldest = other;
                maxAge = ((M122State *) other->unk68)->age;
            }
        }
    }
    if (count >= 11) {
        func_800079B8(oldest);
    }
    s->flags = 2;
    s->pos.x = obj->trans.x_position;
    s->pos.y = obj->trans.y_position;
    s->pos.z = obj->trans.z_position;
    s->unk74 = 0;
    s->unk14 = 0.0f;
    s->unk18 = 0.0f;
    s->unk1C = 0.0f;
    s->unk24 = 0.0f;
    s->unk28 = 0.0f;
    s->modelId = setup->modelId;
    s->timer = func_80005064(600);
    s->age = 0;
    func_80079D1C(obj, 0xA00, 1, 2, 0, 0, 0, 16.0f, 16.0f, 1.0f, 0.75f, 0x11);
    obj->unk7C[0] |= 2;
    if ((s->model = func_80026F38(s->modelId, &unused)) != NULL) {
        obj_list_insert_after(obj, s->model);
    } else {
        s->flags |= 8;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m122", func_m122_00374);

INCLUDE_ASM("asm/nonmatchings/modules/m122", func_m122_004EC);

void func_m122_00F38(Gfx **dl, Mtx **mtx, Object *obj) {
    M122State *s;
    Object *holder;
    MtxF m;

    s = obj->unk68;
    holder = s->holder;
    if (holder != NULL && holder->behaviorId != 0x37) {
        func_80048C7C(&s->mtx, &((M122Joints *) holder->unk5C)->mtx[((u8 *) holder->unk5C)[0x12B]], &m);
        mtx_model_push(dl, mtx, &m);
    } else {
        func_80049DC0(obj, &m);
        func_80049AE8(s->angle, &m);
        mtx_model_push(dl, mtx, &m);
    }
}

void func_m122_00FE4(Object *obj, M122State *s) {
    s->model = (void *) s->modelId;
    s->flags |= 0x20;
    obj->unk7C[0] &= ~2;
    obj->velocity.x *= -0.5f;
    obj->velocity.y = func_80048E2C(0, 5);
    obj->velocity.z *= -0.5f;
    s->spin[0] = func_80048E2C(-5000, 5000);
    s->spin[1] = func_80048E2C(-5000, 5000);
    s->spin[2] = func_80048E2C(-5000, 5000);
}

INCLUDE_ASM("asm/nonmatchings/modules/m122", data);

INCLUDE_ASM("asm/nonmatchings/modules/m122", rodata);
