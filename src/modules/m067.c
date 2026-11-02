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

extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80069BE8(Gfx **dl, void *flame, s32 arg2, s32 alpha);
extern void mtx_pop(Gfx **dl);
extern void effect_texture_pair_free(void *flame);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00008);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_00130);

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

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_038C0);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_0399C);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_03B7C);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_03F08);

INCLUDE_ASM("asm/nonmatchings/modules/m067", func_m067_03FF0);

INCLUDE_ASM("asm/nonmatchings/modules/m067", data);

INCLUDE_ASM("asm/nonmatchings/modules/m067", rodata);

