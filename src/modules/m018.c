/*
 * Runlink module 18: front-end screens (character, weapon and health
 * screens; still mostly assembly).
 */
#include "module.h"

extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */
extern void func_m009_0066C(void);
extern void func_m018_00EF0(void);
extern void rl_m018_011E4(void);
extern s32 rl_m018_0207C(void *player);

/* The module's .data (still assembly) */
extern u8 rl_m018_06684[];
extern u8 rl_m018_06694[];
extern u8 rl_m018_066B0; /* the weapon picked */
extern u8 rl_m018_067E0[];
extern s16 rl_m018_067F4[]; /* image lists, three halfwords each */

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_000CC);

void func_m018_0015C(void) {
    menu_assetgroup_free(rl_m018_06684);
    func_m009_0066C();
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00188);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00AEC);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00EF0);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_01028);

void func_m018_01140(void) {
    Object *player;
    u8 *state;

    func_m018_00EF0();
    player = func_8000E4D0(0);
    if (player != NULL) {
        state = player->unk68;
        state[0x585] = rl_m018_066B0;
    }
    menu_assetgroup_free(rl_m018_06694);
    rl_m018_011E4();
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_01194);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_011E4);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_01218);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_01294);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_0130C);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_0207C);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_020C0);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_023E8);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_02464);

/* Steps from character i in direction dir to the next one that can be picked */
s32 func_m018_03B44(s32 i, s32 dir) {
    do {
        i += dir;
        if (i < 0) {
            i = 2;
        }
        if (i >= 3) {
            i = 0;
        }
    } while (!game_flag_get(i) || game_flag_get(i + 90));
    return i;
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_03BB4);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_0409C);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_040A8);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_042D4);

void func_m018_043AC(void) {
    Object *player;
    void *state;
    s32 i;

    player = func_8000E4D0(0);
    state = player->unk68;
    i = rl_m018_0207C(state);
    menu_assetgroup_free(rl_m018_067E0);
    menu_assetgroup_free(&rl_m018_067F4[i * 3]);
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_04408);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_048E4);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_04AF0);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05788);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_057D8);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05800);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05850);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05880);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05A24);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05B40);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05BD8);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05C0C);

INCLUDE_ASM("asm/nonmatchings/modules/m018", data);

INCLUDE_ASM("asm/nonmatchings/modules/m018", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m018", bss);

