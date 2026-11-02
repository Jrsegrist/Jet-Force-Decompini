/*
 * Runlink module 68: a large boss and the objects it uses (still mostly
 * assembly).
 */
#include "module.h"

typedef struct M068Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 xRotation;
    /* 0x0C */ s16 yRotation;
} M068Setup;


/* The boss's state (obj->unk68), as far as used here */
typedef struct M068State {
    /* 0x00 */ u8 pad0[0x58];
    /* 0x58 */ s32 sound58;
    /* 0x5C */ s32 sound5C;
    /* 0x60 */ u8 pad60[0x6C - 0x60];
    /* 0x6C */ s32 sound6C;
    /* 0x70 */ s32 sound70;
    /* 0x74 */ u8 pad74[0x76 - 0x74];
    /* 0x76 */ s16 hitByPlayer;
} M068State;

/* A piece drawn with its own matrix (obj->unk68) */
typedef struct M068Piece {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ MtxF *base;
} M068Piece;

/* A shot's set-up and state */
typedef struct M068ShotSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ f32 speed;
    /* 0x10 */ Vec3f velocity;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ s32 unk20;
} M068ShotSetup;

typedef struct M068Shot {
    /* 0x00 */ f32 speed;
    /* 0x04 */ f32 radius;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ f32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s16 unk14;
} M068Shot;

/* A beam's set-up: it flies at a point */
typedef struct M068BeamSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 targetX;
    /* 0x0C */ s16 targetY;
    /* 0x0E */ s16 targetZ;
    /* 0x10 */ f32 speed;
    /* 0x14 */ s32 unk14;
} M068BeamSetup;

/* A flying piece's set-up and state */
typedef struct M068PieceSetup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 active;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 zRotation;
    /* 0x10 */ Vec3f velocity;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 speed;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ MtxF *mtx;
} M068PieceSetup;

typedef struct M068Flyer {
    /* 0x0 */ f32 unk0;
    /* 0x4 */ f32 unk4;
    /* 0x8 */ f32 unk8;
    /* 0xC */ s16 timer;
    /* 0xE */ s16 unkE;
    /* 0x10 */ MtxF *mtx;
} M068Flyer;

/* A model drawn from a display list (*obj->header->unkAC) */
typedef struct M068DrawModel {
    /* 0x00 */ struct M068DrawLists *lists;
    /* 0x04 */ void *vertices;
    /* 0x08 */ u8 pad8[0xA - 0x8];
    /* 0x0A */ u8 drawn;
    /* 0x0B */ s8 frame;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ void *frames[1];
} M068DrawModel;

typedef struct M068DrawLists {
    /* 0x00 */ u8 pad0[0x74];
    /* 0x74 */ Gfx *dl74;
    /* 0x78 */ Gfx *dl78;
} M068DrawLists;

typedef struct M068Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ M068DrawModel **models;
} M068Header;

/* The colour a piece is drawn in (its obj->unk68) */
typedef struct M068Glow {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ f32 scale;
    /* 0x18 */ u8 pad18[0x6C - 0x18];
    /* 0x6C */ u8 r;
    /* 0x6D */ u8 g;
    /* 0x6E */ u8 b;
} M068Glow;

/* A command with the 0xBF opcode (a model's frame and vertices) */
#define gSPModelFrameJFG(pkt, frame, verts)                                             \
    {                                                                                  \
        Gfx *_g = (Gfx *) (pkt);                                                       \
        _g->words.w0 = ((u32) OS_K0_TO_PHYSICAL(frame) & 0xFFFFFF) | 0xBF000000;       \
        _g->words.w1 = (u32) (verts);                                                  \
    }

/* The same command, clearing the frame */
#define gSPModelFrameEndJFG(pkt)                                                        \
    {                                                                                  \
        Gfx *_g = (Gfx *) (pkt);                                                       \
        _g->words.w0 = 0xBF000000;                                                     \
        _g->words.w1 = 0;                                                              \
    }

extern void func_80049BA0(MtxF *mtx); /* identity */
extern void func_80049F08(Object *obj, MtxF *out);
extern void func_80055B80(Gfx **dl);
extern void mtx_pop(Gfx **dl);

/* A block's set-up */
typedef struct M068BlockSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ u8 scale; /* in 64ths, at least 10 */
} M068BlockSetup;

typedef struct M068Model {
    /* 0x00 */ u8 pad0[0x94];
    /* 0x94 */ s16 radius;
} M068Model;

/* The object field this module sets at 0x34 */
typedef struct M068Object {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ f32 radius;
} M068Object;

extern f32 func_80097140(f32 x); /* square root */
extern void func_80003614(s32 handle); /* stops a positioned sound */
extern void func_80049DC0(Object *obj, MtxF *out);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out); /* out = a * b */
extern void mtx_model_push(Gfx **dl, s32 arg1, MtxF *mtx);

/* The module's .data (still assembly): sounds playing */
extern s32 rl_m068_124EC;
extern s32 rl_m068_12600;
extern s32 rl_m068_12614;
extern s32 rl_m068_12618;

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_00000);

void func_m068_00014(Object *obj, M068Setup *setup) {
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0002C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0003C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_004D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0092C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_00C70);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0104C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_01330);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_01628);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_017EC);

/* Stops the boss's sounds */
void func_m068_023D4(Object *obj) {
    M068State *s = obj->unk68;

    if (s->sound6C != 0) {
        func_80003614(s->sound6C);
    }
    if (s->sound58 != 0) {
        func_80003614(s->sound58);
    }
    if (s->sound70 != 0) {
        func_80003614(s->sound70);
    }
    if (s->sound5C != 0) {
        func_80003614(s->sound5C);
    }
}

void func_m068_0246C(void) {
    if (rl_m068_12600 != 0) {
        func_80003614(rl_m068_12600);
    }
    if (rl_m068_12614 != 0) {
        func_80003614(rl_m068_12614);
    }
    if (rl_m068_12618 != 0) {
        func_80003614(rl_m068_12618);
    }
    if (rl_m068_124EC != 0) {
        func_80003614(rl_m068_124EC);
    }
}
#pragma weak __rl_fini = func_m068_0246C

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_024EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_039D4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03A78);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03C70);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03D58);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03E24);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03F0C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03FD8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_040EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0447C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04590);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04600);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_046EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_047F8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04878);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_048C0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04938);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04A60);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05028);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05100);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_053AC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_054A0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05570);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05668);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0577C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05964);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_059F0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05C8C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05F60);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06180);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06244);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06310);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06590);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_067B4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06AE0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06E14);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0714C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0758C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_078F4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_07D68);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_083C8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_08674);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0873C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_087E0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_088DC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_09088);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_09218);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_092F0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_093B8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0A208);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0A5D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0ADC4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0B4D0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BB4C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BDC4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BE14);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C7DC);

void func_m068_0C874(Object *obj, M068ShotSetup *setup) {
    M068Shot *s = obj->unk68;
    ObjectHeaderUnk50 *hit;

    obj->velocity.x = setup->velocity.x;
    obj->velocity.y = setup->velocity.y;
    obj->velocity.z = setup->velocity.z;
    s->speed = setup->speed;
    s->unkC = setup->unk1C / setup->speed;
    s->unk14 = setup->unkA;
    obj->opacity = 55;
    s->unk10 = setup->unk20;
    hit = obj->header->unk50;
    s->radius = hit->radius * obj->trans.scale;
    func_80079D1C(obj, 0x10, 1, 4, 0, 0, 0, hit->radius, hit->radius, -1.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C944);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0CDD8);

void func_m068_0CF3C(Object *obj, M068BeamSetup *setup) {
    f32 len;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 *beam = obj->unk68;

    *beam = setup->unk14;
    dx = setup->targetX - obj->trans.x_position;
    dy = setup->targetY - obj->trans.y_position;
    dz = setup->targetZ - obj->trans.z_position;
    len = func_80097140(dx * dx + dy * dy + dz * dz);
    dx /= len;
    dy /= len;
    dz /= len;
    obj->velocity.x = setup->speed * dx;
    obj->velocity.y = setup->speed * dy;
    obj->velocity.z = setup->speed * dz;
    func_80079D1C(obj, 0x100, 1, 4, 0, 0, 0, 10.0f, 10.0f, -1.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D05C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D3D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D65C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D828);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E148);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E520);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E668);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0ED10);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0EF2C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0EF70);

void func_m068_0F2FC(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068DrawModel *model;
    M068DrawLists *lists;
    M068Glow *glow;
    MtxF objMtx;
    MtxF mtx;

    model = *((M068Header *) obj->header)->models;
    glow = obj->unk68;
    lists = model->lists;
    func_80049BA0(&mtx);
    mtx[0][0] = glow->scale;
    mtx[2][2] = glow->scale;
    func_80049F08(obj, &objMtx);
    func_80048C7C(&mtx, &objMtx, &mtx);
    gDPSetPrimColor((*dl)++, 0, 0, 0xFA, 0x3C, 0x0A, obj->opacity);
    mtx_model_push(dl, arg1, &mtx);
    gSPModelFrameJFG((*dl)++, model->frames[model->frame], OS_K0_TO_PHYSICAL(model->vertices));
    gSPDisplayList((*dl)++, OS_K0_TO_PHYSICAL(lists->dl78));
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gSPModelFrameEndJFG((*dl)++);
    mtx_pop(dl);
    func_80055B80(dl);
    model->drawn = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F478);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F7C8);

/* Called when something hits the boss: notes a hit by the player */
void func_m068_10734(Object *hitter, Object *obj, s32 arg2) {
    M068State *s = obj->unk68;

    if (hitter->behaviorId == 1) {
        s->hitByPlayer = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10758);

void func_m068_108C8(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068DrawModel *model;
    M068DrawLists *lists;
    M068Glow *glow;
    MtxF mtx;

    model = *((M068Header *) obj->header)->models;
    glow = obj->unk68;
    lists = model->lists;
    func_80049DC0(obj, &mtx);
    gDPSetPrimColor((*dl)++, 0, 0, glow->r, glow->g, glow->b, 255);
    mtx_model_push(dl, arg1, &mtx);
    gSPModelFrameJFG((*dl)++, model->frames[model->frame], OS_K0_TO_PHYSICAL(model->vertices));
    gSPDisplayList((*dl)++, OS_K0_TO_PHYSICAL(lists->dl74));
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gSPModelFrameEndJFG((*dl)++);
    mtx_pop(dl);
    func_80055B80(dl);
    model->drawn = 0;
}

void func_m068_10A28(Object *obj, M068PieceSetup *setup) {
    M068Flyer *f;

    if (setup->active) {
        f = obj->unk68;
        obj->velocity.x = setup->velocity.x * setup->speed;
        obj->velocity.y = setup->velocity.y * setup->speed;
        obj->velocity.z = setup->velocity.z * setup->speed;
        f->unk0 = setup->unk28;
        f->unk4 = setup->unk1C;
        f->unk8 = setup->unk20;
        f->timer = func_80048E2C(8, 16);
        f->unkE = 1;
        obj->trans.x_position = setup->x;
        obj->trans.y_position = setup->y;
        obj->trans.z_position = setup->z;
        obj->trans.rotation.y_rotation = setup->yRotation;
        obj->trans.rotation.x_rotation = setup->xRotation;
        obj->trans.rotation.z_rotation = setup->zRotation;
        obj->trans.flags &= ~0x400;
    } else {
        obj->trans.flags |= 0x400;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10B58);

void func_m068_10DD4(Object *obj, M068PieceSetup *setup) {
    M068Flyer *f;

    if (setup->active) {
        f = obj->unk68;
        obj->velocity.x = setup->velocity.x * setup->speed;
        obj->velocity.y = setup->velocity.y * setup->speed;
        obj->velocity.z = setup->velocity.z * setup->speed;
        f->unk0 = setup->unk28;
        f->unk4 = setup->unk1C;
        f->unk8 = setup->unk20;
        f->timer = func_80048E2C(8, 16);
        f->unkE = 1;
        obj->trans.x_position = setup->x;
        obj->trans.y_position = setup->y;
        obj->trans.z_position = setup->z;
        obj->trans.rotation.y_rotation = setup->yRotation;
        obj->trans.rotation.x_rotation = setup->xRotation;
        obj->trans.rotation.z_rotation = setup->zRotation;
        f->mtx = setup->mtx;
        (*f->mtx)[0][3] = 0.0f;
        (*f->mtx)[1][3] = 0.0f;
        (*f->mtx)[2][3] = 0.0f;
        (*f->mtx)[3][0] = 0.0f;
        (*f->mtx)[3][1] = 0.0f;
        (*f->mtx)[3][2] = 0.0f;
        (*f->mtx)[3][3] = 1.0f;
        if (obj->unk7C != NULL) {
            *obj->unk7C &= ~2;
        }
        obj->trans.flags &= ~0x400;
    } else {
        obj->trans.flags |= 0x400;
    }
}

/* Pushes a piece's matrix: its own transform times its base matrix */
void func_m068_10F90(Gfx **dl, s32 arg1, Object *obj) {
    M068Piece *p = obj->unk68;
    MtxF local;
    MtxF out;

    func_80049DC0(obj, &local);
    func_80048C7C(p->base, &local, &out);
    mtx_model_push(dl, arg1, &out);
}

void func_m068_10FE8(Object *obj, M068BlockSetup *setup) {
    f32 scale = setup->scale & 0xFF;
    M068Model *model;

    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale *= 0.015625f;
    obj->trans.scale = obj->header->scale * scale;
    obj->trans.rotation.y_rotation = setup->yRotation;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    model = ((M068Model **) obj->modelInstances)[obj->modelIndex];
    ((M068Object *) obj)->radius = model->radius * obj->trans.scale;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_110E0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", data);

INCLUDE_ASM("asm/nonmatchings/modules/m068", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m068", bss);

