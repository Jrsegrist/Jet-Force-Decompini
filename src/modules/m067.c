/*
 * Runlink module 67: an enemy and its missiles (still mostly assembly).
 */
#include "module.h"

/* A missile's state (obj->unk68), as far as used here */
typedef struct M067Missile {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ f32 thrust;
    /* 0x8 */ void *flame;
} M067Missile;

/* A jump in progress */
typedef struct M067Jump {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ s8 kind;
    /* 0x02 */ s8 target;
    /* 0x03 */ u8 pad3[0x10 - 0x3];
    /* 0x10 */ Vec3f start;
    /* 0x1C */ s16 yRotation;
    /* 0x1E */ s16 xRotation;
    /* 0x20 */ Vec3f dest;
    /* 0x2C */ s16 yRotation2;
    /* 0x2E */ s16 xRotation2;
} M067Jump;

/* An entry of the jump table (size 8) */
typedef struct M067JumpKind {
    /* 0x0 */ s32 anim;
    /* 0x4 */ s32 unk4;
} M067JumpKind;

/* The set-up of a shot (a bolt or a rock) */
typedef struct M067ShotSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 strength;
    /* 0x0B */ u8 time;
    /* 0x0C */ s16 yRotation;
    /* 0x0E */ s16 xRotation;
    /* 0x10 */ f32 scale;
    /* 0x14 */ f32 speed;
    /* 0x18 */ s32 unk18;
} M067ShotSetup;

typedef struct M067Shot {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s16 timer;
    /* 0x6 */ u8 strength;
    /* 0x7 */ u8 unk7;
} M067Shot;

extern s32 rl_m067_00008(s32 from, s32 to, f32 *x, f32 *y, f32 *z, s16 *yRotation, s16 *xRotation);
extern M067JumpKind rl_m067_04430[];
extern s16 func_80049D80(s16 angle, s16 target);

extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80069BE8(Gfx **dl, void *flame, s32 arg2, s32 alpha);
extern void mtx_pop(Gfx **dl);
extern void effect_texture_pair_free(void *flame);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00008);

/* Starts a jump to target (an index into the jump table); returns whether there was one */
s32 func_m067_00130(Object *obj, M067Jump *j, s32 target) {
    s32 kind;

    kind = rl_m067_00008(j->target, target, &j->dest.x, &j->dest.y, &j->dest.z, &j->yRotation2, &j->xRotation2);
    if (kind != -1) {
        j->kind = kind;
        j->target = target;
        func_8001149C(obj, rl_m067_04430[j->kind].anim, 0.0f);
        j->start.x = obj->trans.x_position;
        j->dest.x -= obj->trans.x_position;
        j->start.y = obj->trans.y_position;
        j->dest.y -= obj->trans.y_position;
        j->start.z = obj->trans.z_position;
        j->dest.z -= obj->trans.z_position;
        j->yRotation = obj->trans.rotation.y_rotation;
        j->yRotation2 = func_80049D80(obj->trans.rotation.y_rotation, j->yRotation2);
        j->xRotation = obj->trans.rotation.x_rotation;
        j->xRotation2 = func_80049D80(obj->trans.rotation.x_rotation, j->xRotation2);
        return TRUE;
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_0025C);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00300);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_0064C);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00728);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_008C4);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00D70);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00EF4);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_02BF0);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_02E38);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_02FDC);

/* Draws a missile's flame */
void func_m067_03808(Gfx **dl, s32 arg1, Object *obj) {
    M067Missile *m = obj->unk68;

    if (m->flame != NULL && m->thrust > 0.0f) {
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        func_80069BE8(dl, m->flame, 6, 255);
        mtx_pop(dl);
    }
}

/* Frees a missile's flame */
void func_m067_03888(Object *obj) {
    M067Missile *m = obj->unk68;

    if (m->flame != NULL) {
        effect_texture_pair_free(m->flame);
    }
}

void func_m067_038C0(Object *obj, M067ShotSetup *setup) {
    M067Shot *s = obj->unk68;

    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    obj->trans.scale = setup->scale;
    s->timer = setup->time * 6;
    s->unk0 = setup->unk18;
    s->strength = setup->strength;
    s->unk7 = 0;
    func_80079D1C(obj, s->strength << 8, 1, 4, 0, 0, 0, 80.0f, 80.0f, 100.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_0399C);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_03B7C);

void func_m067_03F08(Object *obj, M067ShotSetup *setup) {
    M067Shot *s;

    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    obj->trans.rotation.z_rotation = 0;
    s = obj->unk68;
    obj->trans.scale *= setup->scale;
    obj->velocity.z = -setup->speed;
    func_8004944C(&obj->trans.rotation, &obj->velocity);
    s->unk0 = setup->unk18;
    s->timer = setup->time * 6;
    s->strength = setup->strength;
    s->unk7 = 0;
    func_80079D1C(obj, s->strength << 8, 1, 4, 0, 0, 0, 10.0f, 10.0f, 100.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_03FF0);

INCLUDE_ASM("asm/nonmatchings/modules/m067", data);

INCLUDE_ASM("asm/nonmatchings/modules/m067", rodata);

