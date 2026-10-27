/* Runlink module 123: a rotating light (a model with an attached light and a light beam). */
#include "module.h"

typedef struct M123Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 model;
    /* 0x0B */ u8 y_rotation;
    /* 0x0C */ u8 x_rotation;
    /* 0x0D */ u8 r;
    /* 0x0E */ u8 g;
    /* 0x0F */ u8 b;
    /* 0x10 */ u8 lightAlpha;
    /* 0x11 */ u8 lightUnk1;
    /* 0x12 */ u8 lightRadius;
    /* 0x13 */ u8 beamAlpha;
    /* 0x14 */ u8 pad14;
    /* 0x15 */ u8 beamLength;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 mode;
    /* 0x19 */ u8 pad19;
    /* 0x1A */ s16 period;
} M123Setup;

typedef struct M123Light {
    /* 0x0 */ void *light;
    /* 0x4 */ void *beam;
    /* 0x8 */ u8 mode;
    /* 0x9 */ u8 y_rotation;
    /* 0xA */ u8 x_rotation;
    /* 0xB */ u8 z_rotation;
    /* 0xC */ s16 timer;
    /* 0xE */ s16 period;
} M123Light;

typedef struct M123BeamParams {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 r;
    /* 0x15 */ u8 g;
    /* 0x16 */ u8 b;
    /* 0x17 */ u8 a;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 length;
    /* 0x1C */ s16 length2;
    /* 0x1E */ s8 unk1E;
} M123BeamParams;

extern void *func_800210A4(Object *obj, LightParams *params); /* DKR light_add_from_object_header */
extern void *func_80023A48(Object *obj, M123BeamParams *params);
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */

void func_m123_00000(Object *obj, M123Setup *setup) {
    LightParams lp;
    M123BeamParams bp;
    M123Light *l = obj->unk68;

    if (setup->model < obj->header->numberOfModelIds) {
        obj->modelIndex = setup->model;
    } else {
        obj->modelIndex = obj->header->numberOfModelIds - 1;
    }
    obj->trans.rotation.y_rotation = setup->y_rotation << 8;
    obj->trans.rotation.x_rotation = setup->x_rotation << 8;
    l->y_rotation = obj->trans.rotation.y_rotation >> 8;
    l->x_rotation = obj->trans.rotation.x_rotation >> 8;
    l->z_rotation = obj->trans.rotation.z_rotation >> 8;
    l->light = NULL;
    l->beam = NULL;
    if (setup->lightRadius != 0) {
        lp.type = 3;
        lp.unk1 = setup->lightUnk1;
        lp.flags = 0x61;
        lp.unk3 = -1;
        lp.unk4 = 0;
        lp.unk6 = 0;
        lp.unk8 = 0;
        lp.unkA = 0;
        lp.radius2 = lp.radius = setup->lightRadius * 4;
        lp.r = setup->r;
        lp.g = setup->g;
        lp.b = setup->b;
        lp.a = setup->lightAlpha;
        lp.unk14 = -1;
        lp.unk16 = setup->unk16;
        lp.unk17 = setup->unk17;
        l->light = func_800210A4(obj, &lp);
    }
    if (setup->beamLength != 0) {
        bp.unk0 = 0xC4;
        bp.unk18 = 0x61;
        bp.unk1 = 0x50;
        bp.unk2 = setup->unk16 << 8;
        bp.unk4 = setup->unk17 << 8;
        bp.unk6 = 0;
        bp.unk8 = 0.0f;
        bp.unkC = 0.0f;
        bp.unk10 = 0.0f;
        bp.r = setup->r;
        bp.g = setup->g;
        bp.b = setup->b;
        bp.a = setup->beamAlpha;
        bp.length = setup->beamLength * 4;
        bp.length2 = setup->beamLength * 4;
        bp.unk1E = -1;
        l->beam = func_80023A48(obj, &bp);
    }
    l->mode = setup->mode;
    l->timer = 0;
    l->period = setup->period;
}

void func_m123_00208(Object *obj, s32 updateRate) {
    M123Light *l = obj->unk68;
    s32 phase;

    l->timer += updateRate;
    while (l->timer >= l->period) {
        l->timer -= l->period;
    }
    obj->trans.rotation.y_rotation = l->y_rotation << 8;
    obj->trans.rotation.x_rotation = l->x_rotation << 8;
    phase = (l->timer << 16) / l->period;
    switch (l->mode) {
        default:
            obj->trans.rotation.y_rotation += phase;
            break;
        case 1:
            obj->trans.rotation.y_rotation += (s16) (16384.0f * func_80049C34(phase));
            break;
        case 2:
            obj->trans.rotation.y_rotation += (s16) (8192.0f * func_80049C34(phase));
            break;
        case 3:
            obj->trans.rotation.x_rotation += (s16) (16384.0f * func_80049C34(phase));
            break;
        case 4:
            obj->trans.rotation.x_rotation += (s16) (8192.0f * func_80049C34(phase));
            break;
    }
}
