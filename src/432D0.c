#include "common.h"

/* DKR include/structs.h */
typedef struct ObjectTransform {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
} ObjectTransform;

extern ObjectTransform *D_800FB084;

void func_800426D0(void) {
    D_800FB084 = NULL;
}

void func_800426DC(ObjectTransform *trans) {
    D_800FB084 = trans;
}

void func_800426E8(ObjectTransform *trans) {
    if (trans == D_800FB084) {
        D_800FB084 = NULL;
    }
}

extern s32 D_800FB080;

s32 func_8004270C(void) {
    return D_800FB080;
}

ObjectTransform *func_8004271C(void) {
    return D_800FB084;
}

/* JFG: something with a position and an offset rotated by its angles */
typedef struct Attachment {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
    /* 0x14 */ f32 offset[3];
} Attachment;
void func_80049394(Attachment *obj, f32 *vec);
void func_800492DC(Attachment *obj, f32 *vec);

/* JFG: the world position of an attachment (its offset rotated, plus its position). */
void func_8004272C(Attachment *obj, f32 *x, f32 *y, f32 *z) {
    f32 vec[3];

    vec[0] = obj->offset[0];
    vec[1] = obj->offset[1];
    vec[2] = obj->offset[2];
    if (obj->unk6 != 0) {
        func_80049394(obj, vec);
    } else {
        func_800492DC(obj, vec);
    }
    *x = vec[0] + obj->x;
    *y = vec[1] + obj->y;
    *z = vec[2] + obj->z;
}

INCLUDE_ASM("asm/nonmatchings/432D0", func_800427E8);

INCLUDE_ASM("asm/nonmatchings/432D0", func_80042A74);

/* JFG: copy the transform of D_800FB084, turned to face the other way. */
void func_80042F48(ObjectTransform *trans) {
    if (D_800FB084 != NULL) {
        trans->x_position = D_800FB084->x_position;
        trans->y_position = D_800FB084->y_position;
        trans->z_position = D_800FB084->z_position;
        trans->rotation[2] = D_800FB084->rotation[2];
        trans->rotation[1] = -D_800FB084->rotation[1];
        trans->rotation[0] = 0x8000 - D_800FB084->rotation[0];
    }
}
