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

/* JFG: the world position of an attachment (its offset rotated, plus its position) (0x8004272C). */
void transform_offset_world_pos(Attachment *obj, f32 *x, f32 *y, f32 *z) {
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

/* JFG: what the camera follows: a point, the distance and height to keep, and the offset to look at */
typedef struct CameraFollow {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 padC[8];
    /* 0x14 */ f32 distance;
    /* 0x18 */ f32 height;
    /* 0x1C */ u8 pad1C[8];
    /* 0x24 */ f32 offset[3];
} CameraFollow;

f32 func_80049BEC(f32 base, s32 exponent);
s16 func_80049C84(f32 x, f32 z);
s16 angle_lerp(s16 angle, s16 target, f32 factor);
f32 sqrtf(f32);
extern f32 D_800AC430;

/*
 * JFG: moves the camera towards the point behind the followed object (at the follow distance from
 * the follow point, raised by the height), and turns it to face the object; with smoothing, only
 * part of the way for this frame's update rate.
 */
void camera_follow_update(Attachment *target, ObjectTransform *cam, s32 updateRate, s32 smooth) {
    s16 yaw;
    s16 pitch;
    f32 posFactor;
    f32 angleFactor;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 tx;
    f32 ty;
    f32 tz;
    UNUSED s32 pad[2];

    posFactor = 1.0f;
    angleFactor = 1.0f;
    if (smooth) {
        posFactor = 1.0f - func_80049BEC(0.875f, updateRate);
        angleFactor = 1.0f - func_80049BEC(D_800AC430, updateRate);
    }
    target->offset[0] = ((CameraFollow *) D_800FB080)->offset[0];
    target->offset[1] = ((CameraFollow *) D_800FB080)->offset[1];
    target->offset[2] = ((CameraFollow *) D_800FB080)->offset[2];
    transform_offset_world_pos(target, &x, &y, &z);
    dx = x - ((CameraFollow *) D_800FB080)->x;
    dy = y - ((CameraFollow *) D_800FB080)->y;
    dz = z - ((CameraFollow *) D_800FB080)->z;
    scale = (dx * dx) + (dy * dy) + (dz * dz);
    if (scale > 0.0f) {
        scale = ((CameraFollow *) D_800FB080)->distance / sqrtf(scale);
    }
    tx = (dx * scale) + x;
    ty = (dy * scale) + y + ((CameraFollow *) D_800FB080)->height;
    tz = (dz * scale) + z;
    cam->x_position += (tx - cam->x_position) * posFactor;
    cam->y_position += (ty - cam->y_position) * posFactor;
    cam->z_position += (tz - cam->z_position) * posFactor;
    dx = cam->x_position - x;
    dy = cam->y_position - y;
    dz = cam->z_position - z;
    scale = sqrtf((dx * dx) + (dz * dz));
    yaw = 0x8000 - func_80049C84(dx, dz);
    pitch = func_80049C84(dy, scale);
    cam->rotation[0] = angle_lerp(cam->rotation[0], yaw, angleFactor);
    cam->rotation[1] = angle_lerp(cam->rotation[1], pitch, angleFactor);
    cam->rotation[2] = angle_lerp(cam->rotation[2], 0, angleFactor);
}

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
