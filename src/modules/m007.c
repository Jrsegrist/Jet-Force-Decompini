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

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01CCC);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01E34);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_01F80);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_020D4);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_023D4);

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_02704);

/* Lets the hint's object go */
void func_m007_02824(void) {
    if (sHintObject != NULL) {
        OBJ_STATE(sHintObject, s32)[0] = 1;
        sHintObject = NULL;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m007", func_m007_0284C);

INCLUDE_ASM("asm/nonmatchings/modules/m007", data);

INCLUDE_ASM("asm/nonmatchings/modules/m007", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m007", bss);

