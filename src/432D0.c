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

void view_override_clear(void) {
    D_800FB084 = NULL;
}

void view_override_set(ObjectTransform *trans) {
    D_800FB084 = trans;
}

void view_override_remove(ObjectTransform *trans) {
    if (trans == D_800FB084) {
        D_800FB084 = NULL;
    }
}

extern s32 D_800FB080;

s32 camera_follow_get(void) {
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
    /* 0x1C */ f32 sideLimit;  /* how far the camera may move sideways (0: no limit) */
    /* 0x20 */ s16 angle;      /* the facing of the follow area */
    /* 0x22 */ s16 angleRange; /* how far the camera may turn from it */
    /* 0x24 */ f32 offset[3];
} CameraFollow;

f32 func_80049BEC(f32 base, s32 exponent);
s16 func_80049C84(f32 x, f32 z);
s16 angle_lerp(s16 angle, s16 target, f32 factor);
f32 sqrtf(f32);

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
        angleFactor = 1.0f - func_80049BEC(0.9f, updateRate);
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

#ifdef NON_MATCHING
/*
 * JFG: like camera_follow_update, but the camera stays within sideLimit to the side of the follow
 * point and within angleRange of its angle. Logic is believed right; the frame is 8 bytes too big
 * and sinA/cosA land in f22 instead of being spilled.
 */
f32 func_80049C34(s16 angle);
f32 func_80049C30(s16 angle);
void func_80042A74(Attachment *target, ObjectTransform *cam, s32 updateRate, s32 smooth) {
    s32 yaw;
    f32 posFactor;
    f32 angleFactor;
    f32 tx;
    f32 tz;
    f32 x;
    f32 y;
    f32 z;
    f32 dy;
    f32 dz;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 sinA;
    f32 cosA;
    f32 dist;
    f32 side;
    f32 limit;
    f32 dx;

    if (((CameraFollow *) D_800FB080)->angleRange < 0x100) {
        camera_follow_update(target, cam, updateRate, smooth);
        return;
    }
    posFactor = 1.0f;
    angleFactor = 1.0f;
    if (smooth) {
        posFactor = 1.0f - func_80049BEC(0.9f, updateRate);
        angleFactor = 1.0f - func_80049BEC(0.9375f, updateRate);
    }
    target->offset[0] = ((CameraFollow *) D_800FB080)->offset[0];
    target->offset[1] = ((CameraFollow *) D_800FB080)->offset[1];
    target->offset[2] = ((CameraFollow *) D_800FB080)->offset[2];
    transform_offset_world_pos(target, &x, &y, &z);
    tx = ((CameraFollow *) D_800FB080)->x;
    tz = ((CameraFollow *) D_800FB080)->z;
    if (((CameraFollow *) D_800FB080)->sideLimit != 0.0f) {
        sinA = func_80049C34(((CameraFollow *) D_800FB080)->angle);
        cosA = func_80049C30(((CameraFollow *) D_800FB080)->angle);
        dx = x - ((CameraFollow *) D_800FB080)->x;
        dz = z - ((CameraFollow *) D_800FB080)->z;
        side = (dx * sinA) + (dz * cosA);
        dx = (dx * cosA) - (dz * sinA);
        limit = -((CameraFollow *) D_800FB080)->sideLimit;
        if (dx < limit) {
            dx = limit;
        }
        if (((CameraFollow *) D_800FB080)->sideLimit < dx) {
            dx = ((CameraFollow *) D_800FB080)->sideLimit;
        }
        dz = side;
        if (side < 0.0f) {
            dz = -side;
        }
        if (dz < ((CameraFollow *) D_800FB080)->distance) {
            if (((CameraFollow *) D_800FB080)->distance < sqrtf((dx * dx) + (side * side))) {
                if (dx < 0.0f) {
                    dx += sqrtf((((CameraFollow *) D_800FB080)->distance * ((CameraFollow *) D_800FB080)->distance) - (dz * dz));
                } else {
                    dx -= sqrtf((((CameraFollow *) D_800FB080)->distance * ((CameraFollow *) D_800FB080)->distance) - (dz * dz));
                }
            } else {
                dx = 0.0f;
            }
        }
        tz -= dx * sinA;
        tx += dx * cosA;
    }
    dx = tx - x;
    dz = tz - z;
    dist = (dx * dx) + (dz * dz);
    if (dist > 0.0f) {
        dist = sqrtf(dist);
        dx *= ((CameraFollow *) D_800FB080)->distance / dist;
        dz *= ((CameraFollow *) D_800FB080)->distance / dist;
    }
    ny = ((CameraFollow *) D_800FB080)->height + y;
    nx = x + dx;
    nz = z + dz;
    yaw = (s16) (func_80049C84(((CameraFollow *) D_800FB080)->x - nx, ((CameraFollow *) D_800FB080)->z - nz) - ((CameraFollow *) D_800FB080)->angle);
    if (yaw != 0x7FFF && (yaw < -((CameraFollow *) D_800FB080)->angleRange || ((CameraFollow *) D_800FB080)->angleRange < yaw)) {
        nx = tx;
        nz = tz;
    } else if (dist < ((CameraFollow *) D_800FB080)->distance) {
        nx = tx;
        nz = tz;
    }
    cam->x_position += (nx - cam->x_position) * posFactor;
    cam->y_position += (ny - cam->y_position) * posFactor;
    cam->z_position += (nz - cam->z_position) * posFactor;
    dx = cam->x_position - x;
    dy = cam->y_position - y;
    dz = cam->z_position - z;
    dist = sqrtf((dx * dx) + (dz * dz));
    yaw = 0x8000 - func_80049C84(dx, dz);
    func_80049C84(dy, dist);
    cam->rotation[0] = angle_lerp(cam->rotation[0], yaw, angleFactor);
    cam->rotation[1] = angle_lerp(cam->rotation[1], 0, angleFactor);
    cam->rotation[2] = angle_lerp(cam->rotation[2], 0, angleFactor);
}
#else
INCLUDE_ASM("asm/nonmatchings/432D0", func_80042A74);
#endif

/* JFG: copy the transform of D_800FB084, turned to face the other way. */
void view_override_copy_reversed(ObjectTransform *trans) {
    if (D_800FB084 != NULL) {
        trans->x_position = D_800FB084->x_position;
        trans->y_position = D_800FB084->y_position;
        trans->z_position = D_800FB084->z_position;
        trans->rotation[2] = D_800FB084->rotation[2];
        trans->rotation[1] = -D_800FB084->rotation[1];
        trans->rotation[0] = 0x8000 - D_800FB084->rotation[0];
    }
}
