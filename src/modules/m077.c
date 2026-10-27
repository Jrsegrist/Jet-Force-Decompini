/*
 * Runlink module 77: a sliding two-part barrier. Its two halves slide apart
 * along its facing; it starts apart when its condition (a level state or a
 * player stat) holds, and other code can tell every barrier with an id to
 * part or shut. While shut it blocks (interaction flag 1).
 */
#include "module.h"

typedef struct M077Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 id;
    /* 0x0B */ u8 yRotation; /* in 1/256 turns */
    /* 0x0C */ s16 scale;    /* in 1/100 */
    /* 0x0E */ u8 brightness;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 startCondition;
    /* 0x11 */ u8 travel;   /* in 4 units */
    /* 0x12 */ u8 partTime; /* in 1/10 seconds */
    /* 0x13 */ u8 shutTime; /* in 1/10 seconds */
} M077Setup;

/* The barrier's state */
enum M077State { SLIDER_SHUT, SLIDER_APART, SLIDER_SHUTTING, SLIDER_PARTING };

typedef struct M077Slider {
    /* 0x00 */ u8 id;
    /* 0x01 */ u8 state;
    /* 0x02 */ u8 brightness;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ f32 travel;
    /* 0x08 */ f32 partSpeed;
    /* 0x0C */ f32 shutSpeed;
    /* 0x10 */ f32 offset; /* how far each half has slid */
    /* 0x14 */ f32 speed;
} M077Slider;

/* What func_8004470C returns */
typedef struct M077Env {
    /* 0x000 */ u8 pad0[0x107];
    /* 0x107 */ u8 unk107;
} M077Env;

/* What func_80047C1C returns */
typedef struct M077Stats {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ u16 unk4;
} M077Stats;

/* A half's model: its display lists */
typedef struct M077PartModel {
    /* 0x00 */ u8 pad0[0x74];
    /* 0x74 */ u32 opaqueList;
    /* 0x78 */ u32 translucentList;
} M077PartModel;

typedef struct M077Part {
    /* 0x0 */ M077PartModel *model;
    /* 0x4 */ u32 matrices;
} M077Part;

extern M077Env *func_8004470C(void);
extern s32 func_800468CC(void);
extern M077Stats *func_80047C1C(s32 player, s32 arg1);
extern f32 func_80049BEC(f32 base, s32 exponent);
extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 arg3, f32 arg4);
extern void mtx_pop(Gfx **dl);
extern void func_80055B80(Gfx **dl);

/* Loads a model's matrices (the game's own command 0xBF) */
#define gSPPartMatrices(pkt, m)                    \
    {                                              \
        Gfx *_g = (Gfx *) (pkt);                   \
        _g->words.w0 = 0xBF000000;                 \
        _g->words.w1 = (u32) (m);                  \
    }

void func_m077_00000(s32 id, s32 apart) {
    s32 first;
    s32 count;
    Object **list;
    Object *o;
    M077Slider *d;

    list = func_80005E98(&first, &count);
    count -= first;
    list = first + list;
    while (count--) {
        o = *list++;
        if (o->behaviorId == 0x81) {
            d = o->unk68;
            if (id == d->id) {
                if (apart == 1) {
                    if (d->state == SLIDER_SHUTTING || d->state == SLIDER_SHUT) {
                        d->state = SLIDER_PARTING;
                    }
                } else {
                    if (d->state == SLIDER_PARTING || d->state == SLIDER_APART) {
                        d->state = SLIDER_SHUTTING;
                    }
                }
            }
        }
    }
}

s32 func_m077_000FC(s32 condition) {
    M077Env *env = func_8004470C();
    M077Stats *stats = func_80047C1C(0, func_800468CC());

    switch (condition) {
        case 2:
            if (env->unk107) {
                return 1;
            }
            break;
        case 3:
            if (!env->unk107) {
                return 1;
            }
            break;
        case 4:
            if ((stats->unk4 & 0x1F) == 0) {
                return 1;
            }
            break;
        case 5:
            if ((stats->unk4 & 0x1F) == 1) {
                return 1;
            }
            break;
        case 6:
            if ((stats->unk4 & 0x1F) == 2) {
                return 1;
            }
            break;
        case 1:
            return 1;
    }
    return 0;
}

void func_m077_00200(Object *obj, M077Setup *setup) {
    M077Slider *d = obj->unk68;

    d->id = setup->id;
    obj->trans.rotation.y_rotation = setup->yRotation << 8;
    obj->trans.scale = setup->scale * 0.01f;
    d->brightness = setup->brightness;
    d->unk3 = setup->unkF;
    d->travel = setup->travel * 4;
    d->partSpeed = d->travel / (setup->partTime * 6);
    d->shutSpeed = -d->travel / (setup->shutTime * 6);
    if (func_m077_000FC(setup->startCondition)) {
        d->state = SLIDER_APART;
        d->offset = d->travel;
    } else {
        d->state = SLIDER_SHUT;
        d->offset = 0.0f;
    }
    d->speed = 0.0f;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, obj->trans.scale * 32.0f, obj->trans.scale * 32.0f, -1.0f, 1.0f, 1);
}

void func_m077_00374(Object *obj, s32 updateRate) {
    M077Slider *d = obj->unk68;

    switch (d->state) {
        case SLIDER_PARTING:
            obj->interactObj->flags &= ~1;
            d->speed += (d->partSpeed - d->speed) * (1.0f - func_80049BEC(0.95f, updateRate));
            d->offset += d->speed * updateRate;
            if (d->travel <= d->offset) {
                d->offset = d->travel;
                d->state = SLIDER_APART;
                d->speed = 0.0f;
            }
            break;
        case SLIDER_SHUTTING:
            d->speed += (d->shutSpeed - d->speed) * (1.0f - func_80049BEC(0.95f, updateRate));
            d->offset += d->speed * updateRate;
            if (d->offset <= 0.0f) {
                d->offset = 0.0f;
                d->speed = 0.0f;
                d->state = SLIDER_SHUT;
                obj->interactObj->flags |= 1;
            }
            break;
        case SLIDER_APART:
            obj->interactObj->flags &= ~1;
            break;
        case SLIDER_SHUT:
            obj->interactObj->flags |= 1;
            break;
    }
}

void func_m077_00548(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    f32 x;
    f32 z;
    f32 dz;
    f32 dx;
    M077Part *part;
    M077Slider *d;

    d = obj->unk68;
    x = obj->trans.x_position;
    z = obj->trans.z_position;
    dz = func_80049C34(obj->trans.rotation.y_rotation) * d->offset;
    dx = func_80049C30(obj->trans.rotation.y_rotation) * d->offset;
    gDPPipeSync((*dl)++);
    gDPSetPrimColor((*dl)++, 0, 0, d->brightness, d->brightness, d->brightness, obj->opacity);
    obj->trans.x_position = x - dx;
    obj->trans.z_position = z + dz;
    part = ((M077Part **) obj->modelInstances)[1];
    gSPPartMatrices((*dl)++, part->matrices + 0x80000000);
    func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
    if (obj->opacity == 0xFF) {
        gSPDisplayList((*dl)++, part->model->opaqueList + 0x80000000);
    } else {
        gSPDisplayList((*dl)++, part->model->translucentList + 0x80000000);
    }
    obj->trans.x_position = x + dx;
    obj->trans.z_position = z - dz;
    part = ((M077Part **) obj->modelInstances)[2];
    gSPPartMatrices((*dl)++, part->matrices + 0x80000000);
    func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
    if (obj->opacity == 0xFF) {
        gSPDisplayList((*dl)++, part->model->opaqueList + 0x80000000);
    } else {
        gSPDisplayList((*dl)++, part->model->translucentList + 0x80000000);
    }
    gSPPartMatrices((*dl)++, 0);
    mtx_pop(dl);
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    func_80055B80(dl);
    obj->trans.x_position = x;
    obj->trans.z_position = z;
}
