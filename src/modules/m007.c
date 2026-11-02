/*
 * Runlink module 7: the hint text shown on screen (normal hints and special
 * ones that interrupt them). Only its small state functions are in C so far.
 */
#include "module.h"

extern void rl_m007_00578(s32 hint, s32 arg1);

/* The module's .data (still assembly), by each variable's module offset */
extern s32 rl_m007_02A24, rl_m007_02A28, rl_m007_02A2C, rl_m007_02A30, rl_m007_02A34;
extern Object *rl_m007_02A60;
#define sHintActive rl_m007_02A24
#define sHintSpecial rl_m007_02A28
#define sHintTriedToEnd rl_m007_02A2C
#define sHintPrevious rl_m007_02A30
#define sHintPreviousArg rl_m007_02A34
#define sHintObject rl_m007_02A60
extern s32 rl_m007_02A20; /* the text's fade (0..256) */
extern s32 rl_m007_02A44, rl_m007_02A58, rl_m007_02A5C;
extern s32 rl_m007_02A64; /* an object to load for the hint, or -1 */
extern s32 rl_m007_02A68; /* a level's start hint, or -1 */

/* A level object entry built on the stack */
typedef struct M007Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 angle;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ f32 scale;
} M007Entry;

void func_m007_00000(void);
void func_m007_02824(void);
extern void rl_m007_00E0C(s32 updateRate);
extern void rl_m007_01018(s32 updateRate);
extern void rl_m007_01214(s32 updateRate);
extern void rl_m007_01F80(s32 arg0);
extern void rl_m007_020D4(s32 arg0);
extern void func_8006FC14(s32 arg0);
extern void func_8003B620(s16 x, s16 y, s16 z, s16 angle);
extern s32 func_800468CC(void);
extern s32 menu_mode_get(void);
extern void func_8004665C(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern Object *func_80005F14(void *entry, s32 arg1);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00148);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_0032C);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00390);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00578);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_0070C);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_007E4);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00988);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00B00);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00C90);

/* Stops the hint text */
void func_m007_00D6C(void) {
    sHintActive = 0;
    sHintTriedToEnd = 0;
}

s32 func_m007_00D80(void) {
    return sHintActive;
}

s32 func_m007_00D90(void) {
    return sHintSpecial;
}

/* Stops the special hint text, going back to the previous one if there was one */
void func_m007_00DA0(void) {
    sHintSpecial = 0;
    if (sHintPrevious != -1) {
        rl_m007_00578(sHintPrevious, sHintPreviousArg);
    }
}

s32 func_m007_00DE0(void) {
    return sHintTriedToEnd;
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00DF0);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_00E0C);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01018);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01214);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_012E0);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01504);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_016B4);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_017E8);

/* Every frame: the hint text's fade in and out, and its drawing */
void func_m007_01CCC(s32 arg0, s32 updateRate) {
    rl_m007_00E0C(updateRate);
    if (sHintActive && !sHintSpecial) {
        if (rl_m007_02A44 != 1 && rl_m007_02A58 != 0) {
            rl_m007_01018(updateRate);
        } else if (rl_m007_02A5C != 0) {
            rl_m007_01214(updateRate);
        }
        rl_m007_02A20 += updateRate * 16;
        if (rl_m007_02A20 > 256) {
            rl_m007_02A20 = 256;
        }
    } else if (rl_m007_02A20 > 0) {
        rl_m007_02A20 -= updateRate * 16;
        if (rl_m007_02A20 <= 0) {
            if (!sHintSpecial) {
                func_m007_00000();
            } else {
                rl_m007_02A20 = 0;
            }
        }
    }
    func_8006FC14(1);
    if (rl_m007_02A20 != 0) {
        if (rl_m007_02A44 != 1) {
            rl_m007_01F80(arg0);
        } else {
            rl_m007_020D4(arg0);
        }
    }
    func_8006FC14(0);
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01E34);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01F80);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_020D4);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_023D4);

/* Loads the hint's object, if one is waiting */
void func_m007_02704(f32 x, f32 y, f32 z, f32 angle, f32 scale) {
    M007Entry entry;
    UNUSED Object *obj;

    if (rl_m007_02A64 != -1) {
        func_m007_02824();
        entry.objectID = rl_m007_02A64;
        entry.size = 0x10;
        entry.unk3 = 0;
        entry.x = x;
        entry.y = y;
        entry.z = z;
        entry.angle = (s32) angle;
        entry.unkB = 0;
        entry.scale = scale;
        sHintObject = func_80005F14(&entry, 1);
        if (sHintObject != NULL) {
            sHintObject->level_entry = NULL;
        }
        rl_m007_02A64 = -1;
    }
}

/* Lets the hint's object go */
void func_m007_02824(void) {
    if (sHintObject != NULL) {
        OBJ_STATE(sHintObject, s32)[0] = 1;
        sHintObject = NULL;
    }
}

/* At a level's start: shows its start hint, if it has one */
void func_m007_0284C(void) {
    Object *player;
    UNUSED s32 pad;
    s32 arg;

    if (rl_m007_02A68 != -1) {
        player = func_8000E4D0(0);
        if (player != NULL) {
            func_8003B620(player->trans.x_position, player->trans.y_position + 2.0f, player->trans.z_position,
                          player->trans.rotation.y_rotation);
        }
        arg = func_800468CC();
        func_8004665C(rl_m007_02A68, arg, 0, menu_mode_get(), 1, 0);
        rl_m007_02A68 = -1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", data);

INCLUDE_ASM("asm/nonmatchings/modules/m007", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m007", bss);

