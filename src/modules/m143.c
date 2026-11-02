/*
 * Runlink module 143: an enemy kind that flies in a linked group.
 */
#include "module.h"

/* What each member's obj->unk68 points at */
typedef struct M143Member {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ Object *next;
    /* 0x2C */ u8 pad2C[0xA6 - 0x2C];
    /* 0xA6 */ s16 health;
} M143Member;

/* The group */
typedef struct M143Group {
    /* 0x00 */ u8 count;
    /* 0x01 */ u8 pad1[0x58 - 0x1];
    /* 0x58 */ Object *first;
    /* 0x5C */ u8 pad5C[0x84 - 0x5C];
    /* 0x84 */ s32 sound;
} M143Group;

extern void sound_stop(s32 handle);

extern s32 rl_m143_02F74; /* a sound playing */

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_0007C);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00280);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00548);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_008C0);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00C2C);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_01940);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_02394);

/* Now and then one living member of the group, picked at random, makes its sound */
void func_m143_02450(M143Group *group) {
    Object *o;
    Object *pick;
    s32 r;
    s32 i;
    M143Member *m;

    if (group->sound == 0 && func_80048E2C(0, 100) >= 96) {
        o = group->first;
        pick = o;
        r = func_80048E2C(0, group->count);
        for (i = 0; o != NULL; i++) {
            if (i == r) {
                pick = o;
            }
            m = o->unk68;
            o = m->next;
        }
        if (pick != NULL && (m = pick->unk68, m->health > 0)) {
            func_80003410(0x1E3, pick->trans.x_position, pick->trans.y_position, pick->trans.z_position, 4,
                          &group->sound);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_02520);

void func_m143_02B88(void) {
    if (rl_m143_02F74 != 0) {
        sound_stop(rl_m143_02F74);
    }
}
#pragma weak __rl_fini = func_m143_02B88

INCLUDE_ASM("asm/nonmatchings/modules/m143", data);

INCLUDE_ASM("asm/nonmatchings/modules/m143", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m143", bss);
