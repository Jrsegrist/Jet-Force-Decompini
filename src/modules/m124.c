/*
 * Runlink module 124: a spotlight. It can be switched by a game flag, swings
 * to and fro, carries an object light and (optionally) a light at the end of
 * its beam, and can be shot out: then it goes dark (setting a flag for some
 * kinds) and comes back on after a while with a shower of sparks (module 10).
 */
#include "module.h"
#include "gfx.h"

typedef struct M124Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 y_rotation;
    /* 0x0B */ u8 x_rotation;
    /* 0x0C */ s8 swingSpeedY;
    /* 0x0D */ s8 swingSpeedX;
    /* 0x0E */ u8 swingRangeY;
    /* 0x0F */ u8 swingRangeX;
    /* 0x10 */ u8 kind;
    /* 0x11 */ u8 flag;
    /* 0x12 */ u8 swing;
    /* 0x13 */ u8 sparks;
    /* 0x14 */ u8 r;
    /* 0x15 */ u8 g;
    /* 0x16 */ u8 b;
    /* 0x17 */ u8 intensity;
    /* 0x18 */ u8 lightRadius;
    /* 0x19 */ u8 lightFlags;
    /* 0x1A */ u8 lightType;
    /* 0x1B */ u8 pad1B;
    /* 0x1C */ u16 range;
    /* 0x1E */ u8 health;
    /* 0x1F */ u8 scale; /* hundredths */
    /* 0x20 */ u8 beamLength;   /* eighths */
    /* 0x21 */ u8 beamRadius;   /* eighths */
} M124Setup;

/* kind */
#define M124_ALWAYS 0
#define M124_FLAG_ON 1  /* on while the flag is set */
#define M124_FLAG_OFF 2 /* on while the flag is clear; sets it when shot out */
#define M124_UNBREAKABLE 3

typedef struct M124Light {
    /* 0x00 */ u8 kind;
    /* 0x01 */ u8 swing; /* 1: swings in y, 2: in x */
    /* 0x02 */ u8 flag;
    /* 0x03 */ u8 sparks;
    /* 0x04 */ u8 r;
    /* 0x05 */ u8 g;
    /* 0x06 */ u8 b;
    /* 0x07 */ u8 intensity;
    /* 0x08 */ u8 on;
    /* 0x09 */ u8 pad9;
    /* 0x0A */ s16 timer; /* until it comes back on */
    /* 0x0C */ f32 rangeSq;
    /* 0x10 */ s16 swingSpeedY;
    /* 0x12 */ s16 swingSpeedX;
    /* 0x14 */ s16 minY;
    /* 0x16 */ s16 maxY;
    /* 0x18 */ s16 minX;
    /* 0x1A */ s16 maxX;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 objectLight;
    /* 0x24 */ f32 beamLength;
    /* 0x28 */ f32 beamRadius;
    /* 0x2C */ s32 beamLight; /* the light where the beam ends */
} M124Light;

/* An instance of the object's model */
typedef struct M124ModelHeader {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ s16 numberOfVertices;
    /* 0x14 */ u8 pad14[0x1C - 0x14];
    /* 0x1C */ Vertex *vertices;
} M124ModelHeader;

typedef struct M124Model {
    /* 0x0 */ M124ModelHeader *header;
    /* 0x4 */ Vertex *vertices;
} M124Model;

extern s32 func_800210A4(Object *obj, LightParams *params); /* DKR light_add_from_object_header */
extern void light_intensity_set(s32 light, u8 intensity);
extern s32 func_80013E0C(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, s32 r, s32 g, s32 b); /* adds a light */
extern void track_light_set_position(s32 light, f32 x, f32 y, f32 z);
extern void track_light_set_colour(s32 light, u8 r, u8 g, u8 b, s32 a);

/*
 * All functions but update_beam_light match. That one differs in register
 * allocation and spills only (the ROM keeps x, y and the radius in stack
 * slots; frame 0x58 vs 0x48). As .rodata has to come from all C or all asm,
 * the matching build keeps the whole module in asm for now.
 */
#ifdef NON_MATCHING

/* Puts the beam's light where the beam ends (adding it the first time) */
static void update_beam_light(Object *obj, M124Light *l) {
    f32 x;
    f32 y;
    f32 z;
    f32 sinY;
    f32 cosY;
    f32 sinX;
    f32 cosX;
    f32 radius;
    f32 len;
    f32 h;

    if (l->objectLight != 0) {
        sinY = func_80049C34(obj->trans.rotation.y_rotation);
        cosY = func_80049C30(obj->trans.rotation.y_rotation);
        sinX = func_80049C34(obj->trans.rotation.x_rotation);
        cosX = func_80049C30(obj->trans.rotation.x_rotation);
        radius = l->beamRadius;
        len = l->beamLength;
        h = len * cosX;
        x = obj->trans.x_position - h * sinY;
        y = obj->trans.y_position + len * sinX;
        z = obj->trans.z_position - h * cosY;
        if (l->beamLight != 0) {
            track_light_set_position(l->beamLight, x, y, z);
        } else {
            l->beamLight = func_80013E0C(x, y, z, radius, radius * 0.4f, (l->r * l->intensity) >> 8,
                                         (l->g * l->intensity) >> 8, (l->b * l->intensity) >> 8);
        }
    }
}

static s32 is_switched_on(M124Light *l) {
    switch (l->kind) {
        case M124_ALWAYS:
            return l->on;
        case M124_FLAG_ON:
            return game_flag_get(l->flag);
        case M124_FLAG_OFF:
            return !game_flag_get(l->flag);
    }
    return TRUE;
}

static void swing_light(Object *obj, M124Light *l, s32 updateRate) {
    if (l->swing & 1) {
        if (!l->on) {
            l->swingSpeedY -= l->swingSpeedY >> 5;
            if (l->swingSpeedY >= -32 && l->swingSpeedY <= 32) {
                l->swingSpeedY = 0;
            }
        }
        obj->trans.rotation.y_rotation += l->swingSpeedY * updateRate;
    }
    if (l->swing & 2) {
        if (!l->on) {
            l->swingSpeedX -= l->swingSpeedX >> 5;
            if (l->swingSpeedX >= -32 && l->swingSpeedX <= 32) {
                l->swingSpeedX = 0;
            }
        }
        obj->trans.rotation.x_rotation += l->swingSpeedX * updateRate;
    }
}

void func_m124_002B0(Object *obj, M124Setup *setup) {
    M124Light *l = obj->unk68;
    LightParams lp;
    M124Model *model;
    Vertex *src;
    Vertex *dst;
    s32 i;
    f32 range;

    obj->trans.rotation.y_rotation = setup->y_rotation << 8;
    obj->trans.rotation.x_rotation = setup->x_rotation << 8;
    obj->trans.scale = setup->scale * 0.01f;
    l->kind = setup->kind;
    l->swing = setup->swing;
    l->flag = setup->flag;
    l->sparks = setup->sparks;
    l->r = setup->r;
    l->g = setup->g;
    l->b = setup->b;
    l->intensity = setup->intensity;
    l->on = TRUE;
    l->timer = 0;
    range = setup->range;
    l->rangeSq = range * range;
    l->swingSpeedY = setup->swingSpeedY << 6;
    l->swingSpeedX = setup->swingSpeedX << 6;
    l->minY = obj->trans.rotation.y_rotation - (setup->swingRangeY << 7);
    l->maxY = obj->trans.rotation.y_rotation + (setup->swingRangeY << 7);
    l->minX = obj->trans.rotation.x_rotation - (setup->swingRangeX << 7);
    l->maxX = obj->trans.rotation.x_rotation + (setup->swingRangeX << 7);
    l->unk1C = 0;
    l->on = is_switched_on(l);
    l->objectLight = 0;
    l->beamLight = 0;
    lp.type = setup->lightType;
    lp.unk1 = 3;
    lp.flags = ~setup->lightFlags & 0x61;
    lp.unk3 = -1;
    lp.unk4 = 0;
    lp.unk6 = 0;
    lp.unk8 = 0;
    lp.unkA = 0;
    lp.radius = setup->lightRadius * 8;
    lp.radius2 = setup->lightRadius * 8;
    lp.r = setup->r;
    lp.g = setup->g;
    lp.b = setup->b;
    if (l->on) {
        lp.a = setup->intensity;
    } else {
        lp.a = 0;
    }
    lp.unk14 = -1;
    lp.unk16 = 0;
    lp.unk17 = 0;
    if (setup->lightType == 3) {
        lp.flags |= 0x40;
    }
    l->objectLight = func_800210A4(obj, &lp);
    l->beamRadius = setup->beamRadius * 8;
    l->beamLength = setup->beamLength * 8;
    if (setup->lightType == 3 && !(setup->lightFlags & 0x40)) {
        update_beam_light(obj, l);
    }
    obj->modelIndex = 0;
    func_80079D1C(obj, setup->health << 8, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    model = *(M124Model **) obj->unk6C;
    if (model != NULL) {
        dst = model->vertices;
        src = model->header->vertices;
        i = model->header->numberOfVertices;
        while (i--) {
            if (src->r == 0 && src->g == 0 && src->b == 0) {
                dst->r = l->r;
                dst->g = l->g;
                dst->b = l->b;
            }
            src++;
            dst++;
        }
    }
}

void func_m124_00678(Object *obj, s32 updateRate) {
    M124Light *l;
    SparkParams spark;
    s32 beamLight;
    s32 objectLight;
    s32 i;

    l = obj->unk68;
    objectLight = l->objectLight;
    beamLight = l->beamLight;
    if (l->on && l->kind != M124_UNBREAKABLE && obj->interactObj->unk6 > 0) {
        obj->interactObj->unk6 = obj->interactObj->unk6 - obj->unk5C->damage - OBJ_STATE(obj, s32)[0];
        if (obj->interactObj->unk6 <= 0) {
            func_80003410(0xD3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            obj->modelIndex = 1;
            l->on = FALSE;
            if (objectLight != 0) {
                light_intensity_set(objectLight, 0);
            }
            if (beamLight != 0) {
                track_light_set_colour(beamLight, l->r, l->g, l->b, 0);
            }
            if (l->kind == M124_FLAG_OFF) {
                func_80047EFC(l->flag, 1);
            }
        }
    }
    swing_light(obj, l, updateRate);
    if (!l->on) {
        if (l->timer != 0) {
            l->timer -= updateRate;
            if (l->timer <= 0) {
                l->timer = 0;
                obj->modelIndex = 0;
                if (objectLight != 0) {
                    light_intensity_set(objectLight, l->intensity);
                }
                if (beamLight != 0) {
                    track_light_set_colour(beamLight, l->r, l->g, l->b, l->intensity);
                }
                func_80003410(0xD2, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                if (l->sparks != 0) {
                    i = l->sparks;
                    spark.x_position = obj->trans.x_position;
                    spark.y_position = obj->trans.y_position;
                    spark.z_position = obj->trans.z_position;
                    spark.unk18 = 20;
                    spark.unk1A = 10;
                    spark.unk1B = 0xFF;
                    spark.unk1C = 0xFFFF80FF;
                    spark.unk20 = 0xFF0000FF;
                    while (i--) {
                        spark.y_rotation = func_80048E2C(-0x2000, 0x2000) + obj->trans.rotation.y_rotation;
                        spark.x_rotation = func_80048E2C(-0x2000, 0x2000) + obj->trans.rotation.x_rotation;
                        spark.speed = func_80048E2C(10, 20) * 0.1f;
                        spark.unk14 = func_80048E2C(12, 25) * 0.1f;
                        func_m010_0000C(&spark);
                    }
                }
            }
        } else {
            obj->modelIndex = 1;
            l->timer = func_80048E2C(30, 480);
            if (objectLight != 0) {
                light_intensity_set(objectLight, 0);
            }
            if (beamLight != 0) {
                track_light_set_colour(beamLight, l->r, l->g, l->b, 0);
            }
        }
    }
    if (beamLight != 0) {
        update_beam_light(obj, l);
    }
    OBJ_STATE(obj, s32)[0] = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m124", func_m124_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m124", func_m124_00150);

INCLUDE_ASM("asm/nonmatchings/modules/m124", func_m124_001C8);

INCLUDE_ASM("asm/nonmatchings/modules/m124", func_m124_002B0);

INCLUDE_ASM("asm/nonmatchings/modules/m124", func_m124_00678);

INCLUDE_ASM("asm/nonmatchings/modules/m124", data);
#endif
