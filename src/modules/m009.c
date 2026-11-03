/*
 * Runlink module 9: a front-end map screen (still mostly assembly).
 */
#include "module.h"

extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */

extern void menu_assetgroup_load(void *images); /* loads a list of images */
extern void menu_assetgroup_reset(void *images);
extern void func_8005342C(s32 module); /* unloads a runlink module */
extern void func_m005_00010(void);
extern void rl_m009_03D0C(void *icon);
extern s32 D_800FECCC;

/* A world's icon on the map (size 0x30) */
typedef struct M009Icon {
    /* 0x00 */ u8 pad0[0x30];
} M009Icon;

/* The module's .data (still assembly) */
extern u8 rl_m009_043F8[];
extern u8 rl_m009_04400[]; /* the map's icons */
extern u8 rl_m009_04424[];
extern M009Icon rl_m009_0448C[11];
extern s32 rl_m009_04908;

/* The module's .bss (still assembly) */
extern s32 rl_m009_097B0;
extern s32 rl_m009_097B4;

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00000);

void func_m009_003A8(void) {
    func_m005_00010();
    if (rl_m009_097B0) {
        func_8005342C(5);
    }
    menu_assetgroup_free(rl_m009_043F8);
    menu_assetgroup_free(rl_m009_04400);
    D_800FECCC = 0;
    rl_m009_04908 = 0;
}

void func_m009_00408(void) {
    s32 i;
    M009Icon *icon;

    menu_assetgroup_load(rl_m009_04400);
    menu_assetgroup_reset(rl_m009_04424);
    rl_m009_097B4 = 0;
    icon = rl_m009_0448C;
    for (i = 0; i < 11; i++, icon++) {
        rl_m009_03D0C(icon);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00478);

void func_m009_0066C(void) {
    menu_assetgroup_free(rl_m009_04400);
}

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00690);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02880);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02978);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_02FB0);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_030E8);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_0343C);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_037E4);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_03908);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_039F0);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_03D0C);

INCLUDE_ASM("asm/nonmatchings/modules/m009", data);

INCLUDE_ASM("asm/nonmatchings/modules/m009", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m009", bss);

