/*
 * Runlink module 112: an explosive object, either placed in the level or
 * launched (then it flies off in the direction of its setup angles). It has
 * its own transform matrix for drawing (pushed by 013B4); 00000 stops its
 * sound and switches it to the finished state.
 */
#include "module.h"

/* What init gets */
typedef struct M112Setup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 launched; /* 0: placed in the level */
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 type;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s16 yaw;
    /* 0x0E */ s16 pitch;
    /* 0x10 */ f32 speed;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 delay; /* in tenths of a second */
} M112Setup;

/* What obj->unk68 points at */
typedef struct M112State {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 mode;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 pad8[0x48 - 0x8];
    /* 0x48 */ MtxF mtx;
    /* 0x88 */ u8 pad88[0x9A - 0x88];
    /* 0x9A */ s16 unk9A;
    /* 0x9C */ s32 unk9C;
    /* 0xA0 */ s32 unkA0;
    /* 0xA4 */ u8 padA4[0xA8 - 0xA4];
    /* 0xA8 */ s32 sound;
    /* 0xAC */ u8 unkAC;
} M112State;

/* The state kept in the object (OBJ_STATE) */
typedef struct M112ObjState {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 alpha;
} M112ObjState;

/* The model of the blast's sprite */
typedef struct M112BlastModel {
    /* 0x00 */ u8 pad0[0x78];
    /* 0x78 */ u32 dl;
} M112BlastModel;

/* obj->modelInstances[1]: the blast's sprite */
typedef struct M112Blast {
    /* 0x00 */ M112BlastModel *model;
    /* 0x04 */ u32 matrices;
    /* 0x08 */ u8 pad8[0x8C - 0x8];
    /* 0x8C */ s16 size;
} M112Blast;

/* Loads a model's matrices (the game's own command 0xBF) */
#define gSPPartMatrices(pkt, m)                    \
    {                                              \
        Gfx *_g = (Gfx *) (pkt);                   \
        _g->words.w0 = 0xBF000000;                 \
        _g->words.w1 = (u32) (m);                  \
    }

/* The game's own command 0x02 */
#define gSPCommand02(pkt, a)                       \
    {                                              \
        Gfx *_g = (Gfx *) (pkt);                   \
        _g->words.w0 = 0x02000050;                 \
        _g->words.w1 = (u32) (a);                  \
    }

extern s32 D_m112_013E0; /* the module's number */
extern u8 D_800A0838[];

extern void runlink_unload_timer_set(s32, s32, s32);
extern ObjectTransform *func_80041DE0(void); /* the camera */
extern void func_80041CA0(Gfx **dl, Mtx **mtx, ObjectTransform *trans, f32 scale, f32 arg4);
extern void func_80055B80(Gfx **dl); /* resets the render settings */
extern void mtx_pop(Gfx **dl);
extern void func_80003614(s32 handle); /* stops a positioned sound */
extern void func_80011D1C(Object *obj);
extern void func_80049DC0(Object *obj, MtxF *out);
extern void mtx_model_push(Gfx **dList, Mtx **mtx, MtxF *transform);

void func_m112_00000(Object *obj) {
    M112State *s = obj->unk68;

    if (s->mode != 5) {
        if (s->sound != 0) {
            func_80003614(s->sound);
        }
        s->mode = 6;
        s->unkAC = 0;
    }
}

void func_m112_0005C(Object *obj, M112Setup *setup) {
    M112State *s = obj->unk68;
    f32 size;

    if (setup->launched == 0) {
        s->type = 0;
        s->mode = 2;
        s->unk4 = 1;
        s->unk5 = func_80048E2C(0, 0x7F);
        s->unk9C = 0;
        s->unkA0 = 0;
    } else {
        obj->velocity.z = -setup->speed;
        obj->trans.rotation.y_rotation = setup->yaw;
        obj->trans.rotation.x_rotation = setup->pitch;
        obj->trans.rotation.z_rotation = 0;
        func_8004944C(&obj->trans.rotation, &obj->velocity);
        s->type = setup->type;
        s->mode = 0;
        s->unk2 = 60;
        s->unk3 = setup->unkB;
        s->unk4 = setup->unk18;
        s->unk5 = 0;
        s->unk6 = func_80005064(setup->delay * 6);
        s->unk9C = setup->unk14;
        s->unkA0 = setup->unk14;
        obj->level_entry = NULL;
        if (s->type == 0 || s->type == 1) {
            func_80011D1C(obj);
        }
    }
    s->unk9A = 0;
    OBJ_STATE(obj, M112ObjState)->unk0 = 0;
    OBJ_STATE(obj, M112ObjState)->alpha = 0xFF;
    if (s->type == 0) {
        size = 80.0f;
    } else if (s->type == 1) {
        size = 107.0f;
    } else {
        size = 32.0f;
    }
    func_80079D1C(obj, 0x100, 1, 2, 0, 0, 0, size, size, -1.0f, 1.0f, 1);
    func_80049DC0(obj, &s->mtx);
}

INCLUDE_ASM("asm/nonmatchings/modules/m112", func_m112_00220);

/* Draws the blast: a sprite facing the camera, twice (crossed), growing and fading */
void func_m112_01004(Gfx **dList, Mtx **mtx, Object *obj) {
    s32 alpha;
    ObjectTransform trans;
    M112Blast *blast;
    M112BlastModel *model;
    M112State *s;
    ObjectTransform *cam;

    runlink_unload_timer_set(D_m112_013E0, 2, 2);
    s = obj->unk68;
    if (s->type == 0 && s->unk5 < 128) {
        cam = func_80041DE0();
        blast = ((M112Blast **) obj->modelInstances)[1];
        if (blast != NULL) {
            model = blast->model;
            trans.scale = 1.0f - (u32) s->unk5 / 127.0f;
            trans.scale = 1.0f - trans.scale * trans.scale;
            trans.scale *= 80.0f / blast->size;
            trans.x_position = obj->trans.x_position;
            trans.y_position = obj->trans.y_position;
            trans.z_position = obj->trans.z_position;
            alpha = 128 - s->unk5;
            if (alpha > 64) {
                alpha = 64;
            }
            alpha = (OBJ_STATE(obj, M112ObjState)->alpha * alpha) >> 8;
            gDPSetEnvColor((*dList)++, 255, 255, 255, 255);
            if (s->mode != 2 && s->mode != 5) {
                gDPSetPrimColor((*dList)++, 0, 0, 0, 255, 0, alpha);
            } else {
                gDPSetPrimColor((*dList)++, 0, 0, 255, 0, 0, alpha);
            }
            trans.rotation.y_rotation = -0x8000 - cam->rotation.y_rotation;
            trans.rotation.x_rotation = 0;
            trans.rotation.z_rotation = 0;
            func_80041CA0(dList, mtx, &trans, 1.0f, 0.0f);
            gSPPartMatrices((*dList)++, blast->matrices + 0x80000000);
            gSPCommand02((*dList)++, D_800A0838 + 0x80000000);
            gSPDisplayList((*dList)++, model->dl + 0x80000000);
            func_80055B80(dList);
            trans.rotation.y_rotation = -0x8000 - cam->rotation.y_rotation;
            trans.rotation.x_rotation = 0x4000;
            trans.rotation.z_rotation = 0;
            func_80041CA0(dList, mtx, &trans, 1.0f, 0.0f);
            gSPPartMatrices((*dList)++, blast->matrices + 0x80000000);
            gSPCommand02((*dList)++, D_800A0838 + 0x80000000);
            gSPDisplayList((*dList)++, model->dl + 0x80000000);
            func_80055B80(dList);
            gSPPartMatrices((*dList)++, 0);
            mtx_pop(dList);
            func_80055B80(dList);
            gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
        }
    }
}

void func_m112_013B4(Gfx **dList, Mtx **mtx, Object *obj) {
    M112State *s = obj->unk68;

    mtx_model_push(dList, mtx, &s->mtx);
}

INCLUDE_ASM("asm/nonmatchings/modules/m112", data);

INCLUDE_ASM("asm/nonmatchings/modules/m112", rodata);
