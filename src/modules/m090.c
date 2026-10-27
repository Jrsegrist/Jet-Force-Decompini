/*
 * Runlink module 90: a firework. After a delay it rises (with a dim light),
 * slowing under gravity; at the top of its climb it bursts into coloured
 * sparks (module 10) over two frames, then fades its light out and either
 * goes away or relaunches from where it started after its interval.
 */
#include "module.h"

typedef struct M090Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 y_rotation;
    /* 0x0C */ s16 x_rotation;
    /* 0x0E */ s16 speed; /* hundredths */
    /* 0x10 */ u8 r;
    /* 0x11 */ u8 g;
    /* 0x12 */ u8 b;
    /* 0x13 */ u8 size; /* 0..3 */
    /* 0x14 */ s16 delay;    /* hundredths of a second */
    /* 0x16 */ s16 interval; /* hundredths of a second; 0: only once */
} M090Setup;

typedef struct M090Firework {
    /* 0x00 */ u8 phase; /* 0: rising, 1-2: bursting, then counts up while fading */
    /* 0x01 */ u8 r;
    /* 0x02 */ u8 g;
    /* 0x03 */ u8 b;
    /* 0x04 */ u8 pad4;
    /* 0x05 */ u8 sparks;
    /* 0x06 */ u8 sparkSizeMin;
    /* 0x07 */ u8 sparkSizeMax;
    /* 0x08 */ s16 sparkSpeedMin; /* hundredths */
    /* 0x0A */ s16 sparkSpeedMax;
    /* 0x0C */ f32 growth; /* scale factor while bursting */
    /* 0x10 */ f32 x;      /* where it starts */
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
    /* 0x1C */ s16 speed;
    /* 0x1E */ s16 interval;
} M090Firework;

typedef struct M090State {
    /* 0x0 */ s32 delay;
    /* 0x4 */ s32 timer;
} M090State;

/* The object's level segment, at 0x2E (DKR's Object.segmentID) */
#define OBJ_SEGMENT(obj) (*(s16 *) &(obj)->pad2C[0x2E - 0x2C])
/* What the object has at 0x74: its light (a pointer to the object light) */
#define OBJ_LIGHT(obj) (((s32 **) (obj)->pad70)[(0x74 - 0x70) / 4])

extern s16 func_80015510(f32 x, f32 y, f32 z); /* the level segment at a position */
extern void func_8002130C(s32 light, u8 r, u8 g, u8 b); /* sets a light's colour */
extern void func_80021358(s32 light, u8 intensity);    /* sets a light's intensity */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_m010_00000(f32 arg0);

/* By size */
static s32 sSparkCounts[4] = { 16, 24, 32, 48 };
static s32 sSparkSizesMin[4] = { 8, 9, 10, 10 };
static s32 sSparkSizesMax[4] = { 10, 11, 12, 12 };
static s32 sSparkSpeedsMin[4] = { 450, 500, 550, 600 };
static s32 sSparkSpeedsMax[4] = { 475, 550, 625, 700 };
static f32 sGrowths[4] = { 8.0f, 9.5f, 11.0f, 12.5f };

/* Sends the firework up again from where it started */
static void relaunch(Object *obj, M090Firework *fw) {
    obj->velocity.z = -fw->speed * 0.01f;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->trans.x_position = fw->x;
    obj->trans.y_position = fw->y;
    obj->trans.z_position = fw->z;
    OBJ_SEGMENT(obj) = func_80015510(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    fw->phase = 0;
    OBJ_STATE(obj, M090State)->delay = OBJ_STATE(obj, M090State)->timer;
}

void func_m090_000A0(Object *obj, M090Setup *setup) {
    s32 pad;
    M090Firework *fw;

    obj->trans.rotation.y_rotation = setup->y_rotation;
    fw = obj->unk68;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->velocity.z = -setup->speed * 0.01f;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    fw->phase = 0;
    fw->r = setup->r;
    fw->g = setup->g;
    fw->b = setup->b;
    fw->sparks = sSparkCounts[setup->size & 3];
    fw->sparkSizeMin = sSparkSizesMin[setup->size & 3];
    fw->sparkSizeMax = sSparkSizesMax[setup->size & 3];
    fw->sparkSpeedMin = sSparkSpeedsMin[setup->size & 3];
    fw->sparkSpeedMax = sSparkSpeedsMax[setup->size & 3];
    fw->growth = sGrowths[setup->size & 3];
    fw->x = obj->trans.x_position;
    fw->y = obj->trans.y_position;
    fw->z = obj->trans.z_position;
    fw->speed = setup->speed;
    fw->interval = setup->interval * 60 / 100;
    if (OBJ_LIGHT(obj) != NULL && *OBJ_LIGHT(obj) != 0) {
        func_8002130C(*OBJ_LIGHT(obj), fw->r, fw->g, fw->b);
    }
    OBJ_STATE(obj, M090State)->delay = setup->delay * 60 / 100;
    OBJ_STATE(obj, M090State)->timer = fw->interval;
}

void func_m090_002B4(Object *obj, s32 updateRate) {
    s32 pad;
    SparkParams spark;
    f32 fade;
    s32 light;
    M090Firework *fw;
    s32 i;

    fw = obj->unk68;
    if (OBJ_LIGHT(obj) != NULL) {
        light = *OBJ_LIGHT(obj);
    } else {
        light = 0;
    }
    obj->trans.scale = obj->header->scale;
    obj->particleEmittersEnabled = 0;
    OBJ_STATE(obj, M090State)->timer -= updateRate;
    if (OBJ_STATE(obj, M090State)->delay > 0) {
        obj->trans.flags |= 0x400;
        OBJ_STATE(obj, M090State)->delay -= updateRate;
        if (OBJ_STATE(obj, M090State)->delay <= 0) {
            OBJ_STATE(obj, M090State)->timer = fw->interval + OBJ_STATE(obj, M090State)->delay;
        }
    } else if (fw->phase == 0) {
        obj->trans.flags &= ~0x400;
        i = updateRate;
        while (i--) {
            obj->trans.x_position += obj->velocity.x;
            obj->trans.y_position += obj->velocity.y;
            obj->trans.z_position += obj->velocity.z;
            obj->velocity.x *= 0.99f;
            obj->velocity.y -= 0.2f;
            obj->velocity.z *= 0.99f;
        }
        OBJ_SEGMENT(obj) = func_80015510(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        if (obj->velocity.y < 0.0f) {
            fw->phase = 1;
        }
        if (light != 0) {
            func_80021358(light, 0x40);
        }
        obj->particleEmittersEnabled = 1;
    } else if (fw->phase < 3) {
        obj->trans.scale *= fw->growth;
        i = fw->sparks >> 1;
        spark.x_position = obj->trans.x_position;
        spark.y_position = obj->trans.y_position;
        spark.z_position = obj->trans.z_position;
        spark.unk14 = 10.0f;
        spark.unk18 = 20;
        spark.unk1B = 0xFF;
        spark.unk1C = -1;
        spark.unk20 = (fw->r << 24) | (fw->g << 16) | (fw->b << 8) | 0xFF;
        func_m010_00000(0.075f);
        while (i--) {
            spark.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
            spark.x_rotation = func_80048E2C(-0x4000, 0x4000);
            spark.speed = func_80048E2C(fw->sparkSpeedMin, fw->sparkSpeedMax) * 0.01f;
            spark.unk1A = func_80048E2C(fw->sparkSizeMin, fw->sparkSizeMax);
            func_m010_0000C(&spark);
        }
        fw->phase++;
        if (light != 0) {
            func_80021358(light, 0xFF);
        }
    } else {
        obj->trans.flags |= 0x400;
        fw->phase += updateRate;
        if (fw->phase >= 123) {
            if (fw->interval == 0) {
                func_800079B8(obj);
                return;
            }
            relaunch(obj, fw);
        } else if (light != 0) {
            fade = 1.0f - (fw->phase - 2) / 120.0f;
            func_80021358(light, (s32) (255.0f * fade));
        }
    }
    func_8005F5AC(obj, updateRate);
}
