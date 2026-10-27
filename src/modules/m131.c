/*
 * Runlink module 131: a spinning shot with its own light. It flies along its
 * launch direction until it hits something or runs out of time, swinging two
 * coloured sprites around itself; its strength (1..4) picks the swing radius
 * and hit box size.
 */
#include "module.h"
#include "gfx.h"

typedef struct M131Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 strength;
    /* 0x0B */ u8 lifetime; /* in seconds / 10 */
    /* 0x0C */ s16 yRotation;
    /* 0x0E */ s16 xRotation;
    /* 0x10 */ f32 speed;
    /* 0x14 */ Object *shooter;
} M131Setup;

typedef struct M131Shot {
    /* 0x00 */ f32 x; /* where it hit */
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ s32 hitFlags;
    /* 0x10 */ u8 hit;
    /* 0x11 */ u8 glow; /* counts up to 0xFF while the glow particles run */
    /* 0x12 */ s16 timer;
    /* 0x14 */ f32 swingRadius;
    /* 0x18 */ Object *shooter;
} M131Shot;

typedef struct M131State {
    /* 0x0 */ s32 swingAngle;
    /* 0x4 */ s32 light;
} M131State;

typedef struct M131Interaction {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f oldPos;
} M131Interaction;

typedef struct M131Hit {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ Vec3f pos;
    /* 0x10 */ Vec3f normal;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ s32 flags;
    /* 0x28 */ s32 unk28;
} M131Hit;

/* What object->0x70 points at */
typedef struct M131Joints {
    /* 0x00 */ u8 pad0[0x1A];
    /* 0x1A */ s16 tilt;
    /* 0x1C */ u8 pad1C[0x3E - 0x1C];
    /* 0x3E */ s16 counterTilt;
} M131Joints;
#define OBJ_JOINTS(obj) (*(M131Joints **) (obj)->pad70)

/* What the sprite draw takes: a transform and more */
typedef struct M131Sprite {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 unk28;
} M131Sprite;

typedef struct M131Colours {
    u8 r, g, b, r2, g2, b2;
} M131Colours;

extern void func_800541B0(s32, s32, s32);
extern s32 func_80005064(s32);
extern f32 *func_80013E0C(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, s32 arg5, s32 arg6, s32 arg7); /* adds a light */
extern void func_80013F08(f32 *light);
extern void func_80013FE0(f32 *light, f32 x, f32 y, f32 z);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern s32 func_80018808(Object *obj, Vec3f *from, Vec3f *to, M131Hit *hit, s32, s32);
extern void func_80052734(Object *obj, s16 x, s16 y, s16 z, s32, s32, s32);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_800496B0(s32 arg0, Object *obj, f32 *in, f32 *out);
extern void func_8005600C(s32 r, s32 g, s32 b, s32 r2, s32 g2, s32 b2);
extern void func_80056064(void);
extern void func_80041174(Gfx **dl, s32 arg1, s32 arg2, M131Sprite *sprite, void *model, s32 arg5, s32 arg6);

s32 D_m131_007C0 = 131; /* the module's number */
static f32 sSwingRadius[4] = { 8.0f, 12.0f, 16.0f, 20.0f };
static f32 sHitSize[4] = { 4.0f, 6.0f, 8.0f, 10.0f };
static M131Colours sSpriteColours[2] = {
    { 0xFF, 0xC0, 0xC0, 0xFF, 0x20, 0x40 },
    { 0xE0, 0xC0, 0xC0, 0x40, 0x20, 0xFF },
};

void func_m131_00000(Object *obj, M131Setup *setup) {
    s32 size;
    M131Shot *s;

    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    s = obj->unk68;
    obj->velocity.z = -setup->speed;
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->trans.rotation.z_rotation = 0;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    size = setup->strength - 1;
    if (size < 0) {
        size = 0;
    }
    if (size > 3) {
        size = 3;
    }
    s->hit = 0;
    s->x = obj->trans.x_position;
    s->y = obj->trans.y_position;
    s->z = obj->trans.z_position;
    s->timer = func_80005064(setup->lifetime * 6);
    s->shooter = setup->shooter;
    s->swingRadius = sSwingRadius[size];
    s->glow = 0xFE - size;
    func_80079D1C(obj, setup->strength << 8, 1, 4, 0, 0, 0, sHitSize[size], sHitSize[size], -1.0f, 1.0f, 0x11);
    OBJ_STATE(obj, M131State)->swingAngle = 0;
    OBJ_STATE(obj, M131State)->light = (s32) func_80013E0C(obj->trans.x_position, obj->trans.y_position,
                                                           obj->trans.z_position, 400.0f, 250.0f, 0x40, 0x20, 0xFF);
}

void func_m131_001A4(Object *obj, s32 updateRate) {
    M131Hit hit;
    s32 done;
    f32 step;
    M131Shot *s;

    func_800541B0(D_m131_007C0, 2, 2);
    s = obj->unk68;
    done = 0;
    if (s->timer > 0 && obj->interactObj->unk6 > 0) {
        obj->particleEmittersEnabled = 0;
        if (s->glow < 0xFF) {
            obj->particleEmittersEnabled |= 4;
            s->glow++;
        }
        if (updateRate >= s->timer) {
            step = s->timer;
        } else {
            step = updateRate;
        }
        if (s->hit == 0) {
            func_80009A04(obj, obj->velocity.x * step, obj->velocity.y * step, obj->velocity.z * step);
            if (OBJ_STATE(obj, M131State)->light != 0) {
                func_80013FE0((f32 *) OBJ_STATE(obj, M131State)->light, obj->trans.x_position,
                              obj->trans.y_position, obj->trans.z_position);
            }
            OBJ_STATE(obj, M131State)->swingAngle += updateRate << 11;
            OBJ_JOINTS(obj)->tilt = func_80049C34(OBJ_STATE(obj, M131State)->swingAngle) * -s->swingRadius;
            OBJ_JOINTS(obj)->counterTilt = -OBJ_JOINTS(obj)->tilt;
            obj->particleEmittersEnabled |= 3;
            if (s->shooter->behaviorId == 1) {
                func_80052734(obj, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, 1, 0);
            }
            if (s->hit = func_80018808(obj, &((M131Interaction *) obj->interactObj)->oldPos,
                                       (Vec3f *) &obj->trans.x_position, &hit, 0, 0)) {
                s->x = hit.pos.x;
                s->y = hit.pos.y;
                s->z = hit.pos.z;
                s->hitFlags = hit.flags;
                func_80009A04(obj, s->x - obj->trans.x_position, s->y - obj->trans.y_position,
                              s->z - obj->trans.z_position);
                if (!(s->hitFlags & 0x2000) || (s->hitFlags & 0x8000)) {
                    obj->particleEmittersEnabled |= 4;
                }
            } else {
                s->x = obj->trans.x_position;
                s->y = obj->trans.y_position;
                s->z = obj->trans.z_position;
            }
            s->timer -= updateRate;
        } else {
            if (s->hitFlags & 0x8000) {
                obj->particleEmittersEnabled |= 0xC;
            } else if (s->hitFlags & 0x2000) {
                obj->trans.rotation.x_rotation = 0;
                obj->particleEmittersEnabled |= 0x18;
                obj->trans.rotation.z_rotation = 0;
            } else {
                obj->particleEmittersEnabled |= 0x24;
            }
            done = 1;
        }
    } else {
        done = 1;
    }
    func_8005F5AC(obj, 2);
    if (done) {
        if (OBJ_STATE(obj, M131State)->light != 0) {
            func_80013F08((f32 *) OBJ_STATE(obj, M131State)->light);
        }
        func_800079B8(obj);
    }
}

void func_m131_005BC(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M131Shot *s;
    M131Sprite sprite;
    s32 i;
    M131Colours *colour;
    void *model;

    func_800541B0(D_m131_007C0, 2, 2);
    s = obj->unk68;
    sprite.trans.rotation.y_rotation = 0;
    sprite.trans.rotation.x_rotation = 0;
    sprite.trans.rotation.z_rotation = 0;
    sprite.trans.scale = s->swingRadius * 0.1f;
    sprite.unk28 = 0.0f;
    colour = sSpriteColours;
    model = obj->unk6C[0];
    for (i = 0; i < 2; i++, colour++) {
        sprite.trans.x_position = func_80049C34(OBJ_STATE(obj, M131State)->swingAngle) * s->swingRadius;
        if (i == 0) {
            sprite.trans.x_position = -sprite.trans.x_position;
        }
        sprite.trans.y_position = 0.0f;
        sprite.trans.z_position = 0.0f;
        func_800496B0(1, obj, &sprite.trans.x_position, &sprite.trans.x_position);
        sprite.trans.x_position += obj->trans.x_position;
        sprite.trans.y_position += obj->trans.y_position;
        sprite.trans.z_position += obj->trans.z_position;
        func_8005600C(colour->r, colour->g, colour->b, colour->r2, colour->g2, colour->b2);
        func_80041174(dl, arg1, arg2, &sprite, model, 14, 255);
    }
    func_80056064();
    gDPPipeSync((*dl)++);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
}
