/*
 * Runlink module 17: a player character's behaviour (the control function
 * called every frame, the firing check, and the movement states it runs).
 * A sibling of modules 15 and 16.
 */
#include "module.h"

/* A player's state block (the object's unk68); see 2BAA0.c */
typedef struct Racer {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x13E - 0x2];
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 pad13F[0x14A - 0x13F];
    /* 0x14A */ s8 unk14A;
    /* 0x14B */ u8 pad14B[0x568 - 0x14B];
    /* 0x568 */ u8 state;
    /* 0x569 */ u8 pad569[0x576 - 0x569];
    /* 0x576 */ s8 unk576;
    /* 0x577 */ u8 pad577[0x5F0 - 0x577];
} Racer;

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_00000);

/* Whether the character may fire now */
s32 func_m017_01990(Object *obj, Racer *racer) {
    s32 state;

    if (racer->unk576 == 0 && racer->unk13E == 0) {
        state = racer->state & ~0x20;
        if (state == 0 || state == 5 || state == 0xB || state == 0xA || (state == 3 && racer->unk14A != 0)) {
            return TRUE;
        }
        if (racer->character == 6 && state == 4) {
            return TRUE;
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_01A30);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_01C44);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_01F7C);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_02564);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_02848);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_02A40);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_02A54);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_0313C);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_03258);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_04178);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_04270);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_0430C);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_04614);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_04A44);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_04B84);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_04C50);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_050BC);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_056D4);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_05930);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_05F08);

INCLUDE_ASM("asm/nonmatchings/modules/m017", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m017", bss);
