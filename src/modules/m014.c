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
extern u8 D_800FF310;
static void func_m014_010D8(void);

/* The module's .data (still assembly) */
extern u8 rl_m014_04440[]; /* images */
extern u8 rl_m014_04460[];

/* The module's .bss (still assembly) */
extern s32 rl_m014_04890;
extern s32 rl_m014_0489C;
extern s16 rl_m014_04894;
extern s16 rl_m014_04896;
extern s16 rl_m014_04898;

void func_m014_00000(void) {
    func_m013_00000();
    func_8004DDAC(0, 0, 0);
    menu_assetgroup_load(rl_m014_04440);
    menu_assetgroup_reset(rl_m014_04460);
    func_m014_010D8();
    func_m006_00C74();
    func_m006_01134();
    rl_m014_04894 = 0;
    rl_m014_04896 = 0;
    rl_m014_04898 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00074);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00220);

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_00B30);

static void func_m014_010D8(void) {
    rl_m014_0489C = 0;
    rl_m014_04890 = 0;
    D_800FF310 = 0;
}

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

extern char **gMenuText; /* the game's texts */
extern s32 func_800706D4(char *text, s32 x, s32 font); /* DKR get_text_width */
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void func_m157_00000(Gfx **dl, u8 shape, s32 x, s32 y, s32 width, s32 height, u32 colour);
extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern char *get_world_name(s32 world);
extern char rl_m014_0487C[]; /* "%d %s\n" */

/* Draws a box with the region's name, its world's name and a numbered line under it */
void func_m014_03AD4(Gfx **dl, s32 region, s32 world, s32 number, s32 alpha) {
    char buf[32];
    char *name;
    s32 width;
    s32 width2;

    name = gMenuText[region + 407];
    width = func_800706D4(name, 2, 1) + 40;
    func_800639E8(buf, rl_m014_0487C, number, gMenuText[458]);
    width2 = func_800706D4(buf, 2, 1) + 40;
    if (width < width2) {
        width = width2;
    }
    func_m157_00000(dl, 0, 160 - (width >> 1), 51, width, 46, (alpha * 164) >> 8);
    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    func_8006FC68(0, 192, 192, 255, alpha);
    func_8006FCA8(dl, 159, 61, name, 12);
    func_8006FCA8(dl, 159, 74, get_world_name(world), 12);
    func_8006FC68(192, 192, 192, 255, alpha);
    func_8006FCA8(dl, 159, 87, buf, 12);
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", func_m014_03C68);

INCLUDE_ASM("asm/nonmatchings/modules/m014", data);

INCLUDE_ASM("asm/nonmatchings/modules/m014", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m014", bss);

