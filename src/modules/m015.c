/*
 * Runlink module 15: a player character's behaviour (the control function
 * called every frame, the firing check, and the movement states it runs).
 */
#include "module.h"

/* A player's state block (the object's unk68); see 2BAA0.c */
typedef struct Racer {
    /* 0x000 */ u8 pad0[0x13E];
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 pad13F[0x14A - 0x13F];
    /* 0x14A */ s8 unk14A;
    /* 0x14B */ u8 pad14B[0x53D - 0x14B];
    /* 0x53D */ s8 unk53D;
    /* 0x53E */ u8 pad53E[0x568 - 0x53E];
    /* 0x568 */ u8 state;
    /* 0x569 */ u8 pad569[0x576 - 0x569];
    /* 0x576 */ s8 unk576;
    /* 0x577 */ u8 pad577[0x5F0 - 0x577];
} Racer;

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_00000);

/* Whether the character may fire now */
s32 func_m015_01B98(Object *obj, Racer *racer) {
    s32 anim;
    s32 state;
    s16 *model;

    if (racer->unk576 == 0 && racer->unk13E == 0) {
        anim = obj->animationID;
        if (anim != 8) {
            state = racer->state & ~0x20;
            if (state == 0 || state == 5 || state == 0xB || state == 0xA || (state == 3 && racer->unk14A != 0)) {
                return TRUE;
            }
            model = obj->modelInstances[0];
            if ((anim == 0x16 || anim == 0x31) && racer->unk53D != 0) {
                return TRUE;
            }
            if ((anim == 0xE || anim == 0x22) && model[0x5E / 2] == 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_01C80);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_01E14);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_020B4);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_022C4);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_025FC);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_02B2C);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_03274);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_03564);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_03740);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_04088);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_0418C);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_04658);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_04A58);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_04B4C);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_05020);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_05180);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_05328);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_05ED4);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_0663C);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_068BC);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_06B3C);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_06DDC);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_06E40);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_06EBC);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_07724);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_07890);

INCLUDE_ASM("asm/nonmatchings/modules/m015", func_m015_07BC8);

INCLUDE_ASM("asm/nonmatchings/modules/m015", data);

INCLUDE_ASM("asm/nonmatchings/modules/m015", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m015", bss);
