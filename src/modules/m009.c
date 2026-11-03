/*
 * Runlink module 9: a front-end map screen (still mostly assembly).
 */
#include "module.h"

extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */

/* The module's .data (still assembly) */
extern u8 rl_m009_04400[]; /* the map's icons */

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_003A8);

INCLUDE_ASM("asm/nonmatchings/modules/m009", func_m009_00408);

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

