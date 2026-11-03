/*
 * Runlink module 14: part of the HUD (still mostly assembly).
 */
#include "module.h"

extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */
extern void func_m006_00EFC();
extern void *D_800FF3F8[]; /* the loaded menu images */
extern void menu_item_free(s32 image);

extern void menu_assetgroup_load(void *images); /* loads a list of images */
extern void menu_assetgroup_reset(void *images);
extern s32 func_m013_00000(void);
extern void func_8004DDAC(s32 arg0, s32 arg1, s32 arg2);
extern void func_m006_00C74(void);
extern void func_m006_01134(void);
extern void rl_m014_010D8(void);

/* The module's .data (still assembly) */
extern u8 rl_m014_04440[]; /* images */
extern u8 rl_m014_04460[];

/* The module's .bss (still assembly) */
extern s16 rl_m014_04894;
extern s16 rl_m014_04896;
extern s16 rl_m014_04898;

void func_m014_00000(void) {
    func_m013_00000();
    func_8004DDAC(0, 0, 0);
    menu_assetgroup_load(rl_m014_04440);
    menu_assetgroup_reset(rl_m014_04460);
    rl_m014_010D8();
    func_m006_00C74();
    func_m006_01134();
    rl_m014_04894 = 0;
    rl_m014_04896 = 0;
    rl_m014_04898 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00074);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00220);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00B30);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_010D8);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_010F4);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_01328);

/* Frees the images 10 to 25 */
static void free_images(void) {
    s32 i;

    for (i = 0; i < 16; i++) {
        if (D_800FF3F8[i + 10] != NULL) {
            menu_item_free(i + 10);
        }
    }
}

void func_m014_019E4(void) {
    rl_m014_04894 = 0xF0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_019F4);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_01AD8);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_0292C);

void func_m014_02CC0(void) {
    menu_assetgroup_free(rl_m014_04440);
    free_images();
    func_m006_00EFC();
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_02CF4);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_03418);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_037F4);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_03AD4);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_03C68);

INCLUDE_ASM("asm/nonmatchings/modules/m014", data);

INCLUDE_ASM("asm/nonmatchings/modules/m014", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m014", bss);

