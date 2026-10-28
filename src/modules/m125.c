/*
 * Runlink module 125: a target switch. Shooting it lowers its hit points
 * (they come back after a while); its model blends through five shapes as
 * it is worn down, and when it is fully down it sets its game flag, plays
 * a sound and may trigger module 85.
 */
#include "module.h"

typedef struct M125Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 unk8;
    /* 0x0B */ u8 scale; /* in 20ths */
    /* 0x0C */ s16 y_rotation;
    /* 0x0E */ s16 x_rotation;
    /* 0x10 */ u8 unk9;
    /* 0x11 */ u8 time;
    /* 0x12 */ u8 hitPoints;
    /* 0x13 */ u8 pad13;
    /* 0x14 */ s16 flag;
} M125Setup;

/* The switch's data (its object's unk68) */
typedef struct M125Target {
    /* 0x0 */ s16 timer; /* until the hit points come back */
    /* 0x2 */ s16 time;
    /* 0x4 */ s16 flag;
    /* 0x6 */ u16 maxHitPoints;
    /* 0x8 */ u8 unk8;
    /* 0x9 */ u8 unk9;
    /* 0xA */ u8 done;
    /* 0xB */ u8 level; /* 0..4, how far it is worn down */
    /* 0xC */ u8 unkC;
} M125Target;

typedef struct M125Part {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 size;
} M125Part;

typedef struct M125PartRef {
    /* 0x0 */ M125Part *part;
    /* 0x4 */ s32 unk4;
} M125PartRef;

typedef struct M125ModelHeader {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M125PartRef *parts;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ u8 numBlends;
} M125ModelHeader;

/* Size 8 */
typedef struct M125Blend {
    /* 0x0 */ s16 weight;
    /* 0x2 */ s16 weight2; /* < 0: none */
    /* 0x4 */ s32 part;    /* low byte: the part */
} M125Blend;

typedef struct M125Model {
    /* 0x00 */ M125ModelHeader *header;
    /* 0x04 */ u8 pad4[0xB - 0x4];
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0x74 - 0xC];
    /* 0x74 */ M125Blend *blends;
    /* 0x78 */ s16 *out[2];
} M125Model;

extern s32 func_80005064(s32);
extern u8 func_80026824(s32, s32);
extern void func_8002662C(s32, s32);
extern void func_m085_00074(s32);
extern void func_80001170(u8 sound);

#ifdef NON_MATCHING
/* The model's weight for each level */
static f32 D_m125_00590[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
#endif

void func_m125_00000(Object *obj, M125Setup *setup) {
    M125Target *t = obj->unk68;

    obj->trans.rotation.y_rotation = setup->y_rotation;
    obj->trans.rotation.x_rotation = setup->x_rotation;
    obj->trans.scale = (u32) setup->scale * 0.05f;
    t->time = func_80005064(setup->time);
    t->timer = t->time;
    t->maxHitPoints = setup->hitPoints << 8;
    t->unk9 = setup->unk9;
    t->done = 0;
    t->unk8 = setup->unk8;
    t->level = 0;
    t->unkC = 0;
    t->flag = setup->flag;
    func_80079D1C(obj, t->maxHitPoints, 1, 0x18, 0, 5, -8, obj->trans.scale * 10.0f, obj->trans.scale * 10.0f, -1.0f,
                  1.0f, 1);
}

#ifdef NON_MATCHING
/*
 * Equivalent, and the frame matches. IDO keeps a second copy of `weight`
 * for the blend loop (spilled next to the first); the original uses one
 * register (t2) for both, and loads 0x193 before computing the level.
 */
void func_m125_00120(Object *obj, s32 updateRate) {
    M125Target *t = obj->unk68;
    M125Blend *b;
    s16 *out;
    s32 w;
    s32 j;
    s32 level;
    s32 hp;
    s32 part;
    s32 step;
    M125Model *model;
    M125ModelHeader *header;
    M125Part *p;
    f32 goal;
    s32 i;
    f32 *weight;

    if (t->unkC == 0) {
        t->unkC = func_80026824(t->unk8, 0x40);
    }
    if (t->done == 0 && t->flag != 0 && game_flag_get(t->flag)) {
        obj->unk28 = 1.0f;
        obj->interactObj->hitPoints = 0;
        t->level = 4;
        t->done = 1;
        func_8002662C(t->unk8, 0);
    }
    step = t->maxHitPoints >> 10;
    if (obj->interactObj->hitPoints < 0) {
        obj->interactObj->hitPoints = 0;
    }
    if (obj->interactObj->hitPoints > 0 && obj->interactObj->hitPoints < t->maxHitPoints) {
        if (updateRate < t->timer) {
            t->timer -= updateRate;
        } else {
            t->timer = t->time;
            obj->interactObj->hitPoints = t->maxHitPoints;
            func_80001248(0x194, 0);
        }
    }
    hp = obj->interactObj->hitPoints >> 8;
    model = obj->modelInstances[obj->modelIndex];
    header = model->header;
    level = 4 - hp / step;
    if (hp % step) {
        level--;
    }
    if (level != t->level) {
        func_80001248(0x193, 0);
        t->level = level;
    }
    weight = &D_m125_00590[level];
    goal = *weight;
    if (goal < obj->unk28) {
        func_8001138C(obj, -0.01f, updateRate);
        if (obj->unk28 < goal) {
            obj->unk28 = goal;
        }
    } else {
        func_8001138C(obj, 0.01f, updateRate);
        if (goal < obj->unk28) {
            obj->unk28 = goal;
        }
    }
    if (weight == &D_m125_00590[4] && t->done == 0) {
        t->done = 1;
        if (t->unk9 != 0) {
            func_m085_00074(t->unk9);
        }
        func_80001170(15);
        func_8002662C(t->unk8, 0x10);
        if (t->flag != 0) {
            func_80047EFC(t->flag, 1);
        }
    }
    for (i = 0; i != 4; i++) {
        b = model->blends;
        out = model->out[model->buffer];
        if (weight >= &D_m125_00590[1]) {
            w = 0x100;
            weight--;
        } else {
            w = 0;
        }
        for (j = 0; j < header->numBlends; j++, b++) {
            part = b->part & 0xFF;
            p = header->parts[part].part;
            if (i + 4 == part) {
                b->weight = w;
            }
            *out++ = (b->weight >> 8) * p->size;
            if (b->weight2 >= 0) {
                *out++ = (b->weight2 >> 8) * p->size;
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m125", func_m125_00120);

INCLUDE_ASM("asm/nonmatchings/modules/m125", data);
#endif
