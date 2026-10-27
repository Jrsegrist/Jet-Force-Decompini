/* Runlink module 88: a looping sound source whose pitch follows the camera's approach (a Doppler effect). */
#include "module.h"

typedef struct M088Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 sound;
} M088Setup;

typedef struct M088Sound {
    /* 0x0 */ u16 sound;
    /* 0x4 */ f32 lastDist;
    /* 0x8 */ f32 pitch;
    /* 0xC */ s32 handle;
} M088Sound;

extern Object *func_80041DE0(void); /* the camera */
extern f32 sqrtf(f32);
extern void func_80003614(s32 handle);              /* stops a positioned sound */
extern void audspat_point_set_pitch(s32 handle, u8 volume);
extern f32 func_8008B200(s32);

void func_m088_00000(Object *obj, M088Setup *setup) {
    M088Sound *s = obj->unk68;

    s->sound = setup->sound;
    s->pitch = 1.0f;
    s->lastDist = 0.0f;
}

void func_m088_00024(Object *obj, s32 updateRate) {
    M088Sound *s = obj->unk68;
    Object *cam = func_80041DE0();
    f32 dx;
    f32 dy;
    f32 dz;
    f32 speed;
    f32 dist;
    f32 unused1; /* the frame has room for two more locals */
    f32 vol;
    f32 distSq;
    f32 unused2;

    dx = obj->trans.x_position - cam->trans.x_position;
    dy = obj->trans.y_position - cam->trans.y_position;
    dz = obj->trans.z_position - cam->trans.z_position;
    distSq = dx * dx + dy * dy + dz * dz;
    if (distSq > 2250000.0f) {
        if (s->handle != 0) {
            func_80003614(s->handle);
        }
    } else {
        if (s->handle == 0) {
            func_80003410(s->sound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &s->handle);
        }
        dist = sqrtf(distSq);
        speed = (s->lastDist - dist) / updateRate;
        speed *= 3.0f;
        if (speed > 15.0f) {
            speed = 15.0f;
        } else if (speed < -15.0f) {
            speed = -15.0f;
        }
        s->pitch += (func_8008B200((70.0f + speed) / (70.0f - speed) * 315.0f) - 1.2f - s->pitch) * 0.5f;
        if (s->pitch > 0.8f) {
            s->pitch = 0.8f;
        } else if (s->pitch < -0.8f) {
            s->pitch = -0.8f;
        }
        s->lastDist = dist;
        vol = (s->pitch + 1.0f) * 100.0f;
        audspat_point_set_pitch(s->handle, vol);
    }
}
