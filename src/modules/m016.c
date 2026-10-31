/*
 * Runlink module 16: a player character's behaviour (the control function
 * called every frame, the firing check, and the movement states it runs).
 * A sibling of module 15.
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

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_00000);

/* Whether the character may fire now */
s32 func_m016_01A60(Object *obj, Racer *racer) {
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
            if ((anim == 0x15 || anim == 0x2E) && racer->unk53D != 0) {
                return TRUE;
            }
            if ((anim == 0xE || anim == 0x21) && model[0x5E / 2] == 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_01B48);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_01D58);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_02220);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_02708);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_02EB4);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_0321C);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_03464);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_03DB0);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_03F20);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_03F28);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_03F30);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04440);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04840);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04934);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04E08);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04F78);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_05120);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_05BB8);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_06290);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_0650C);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_06AEC);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_06E24);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_070A4);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_07344);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_073A8);

INCLUDE_ASM("asm/nonmatchings/modules/m016", data);

INCLUDE_ASM("asm/nonmatchings/modules/m016", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m016", bss);
