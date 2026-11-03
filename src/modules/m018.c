/*
 * Runlink module 18: front-end screens (character, weapon and health
 * screens; still mostly assembly).
 */
#include "module.h"

/* The menu items (D_800FF780, size 0x20) */
typedef struct M018MenuItem {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ f32 unkC;
    /* 0x10 */ u8 pad10[0x20 - 0x10];
} M018MenuItem;

/* A character's record (func_80047C1C) */
typedef struct M018Character {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ u8 weapon;
} M018Character;

extern void menu_assetgroup_load(void *images); /* loads a list of images */
extern void menu_assetgroup_reset(void *images);
extern void menu_assetgroup_free(void *images); /* frees a list of loaded images */
extern void menu_item_free(s32 arg0);
extern void menu_item_load(s32 arg0);
extern void func_8005342C(s32 module); /* unloads a runlink module */
extern void gParticlePtrList_flush(void);
extern void func_8004AA80(void *data); /* frees */
extern u8 *get_settings(void);       /* the players' saved settings (0x76 bytes each) */
extern void func_m009_00408(void);
extern void func_m009_0066C(void);
extern M018Character *func_80047C1C(s32 player, s32 character);
extern s32 options_screen_mode_get(void);
extern void rl_m018_01194(s32 weapon);
extern M018MenuItem D_800FF780[];
extern Object *D_800FF3F8[];

static void func_m018_011E4(void);
static s32 func_m018_0207C(s8 *state);

/* The module's .data (still assembly) */
extern u8 rl_m018_06684[];
extern u8 rl_m018_0668C[];
extern u8 rl_m018_06694[];
extern u8 rl_m018_066A0[];
extern u8 rl_m018_066AC; /* the weapon's loaded image */
extern u8 rl_m018_066B0; /* the weapon picked */
extern f32 rl_m018_066B4[];
extern u8 rl_m018_06718[];
extern u8 rl_m018_067E0[];
extern u8 rl_m018_067E8[];
extern s16 rl_m018_067F4[]; /* image lists, three halfwords each */
extern u8 rl_m018_06810[];
extern u8 rl_m018_0692C[];
extern u8 rl_m018_06930[];

/* The module's .rodata (still assembly) */
extern f32 rl_m018_06B68;

/* The module's .bss (still assembly) */
extern u8 rl_m018_06BB0[];
extern s32 rl_m018_06BCC;
extern s8 rl_m018_06BD1;
extern s32 rl_m018_06BDC;
extern s32 rl_m018_06BE4;
extern u8 rl_m018_06BF5;
extern s32 rl_m018_06C60;
extern s32 rl_m018_06C64;
extern s32 rl_m018_06C68;
extern s32 rl_m018_06C6C;
extern s32 rl_m018_06C70;
extern s32 rl_m018_06C74;
extern s32 rl_m018_06C78;
extern s32 rl_m018_06C7C;
extern s32 rl_m018_06C80;
extern s32 rl_m018_06C84;
extern s32 rl_m018_06C88;
extern s32 rl_m018_06C8C;
extern s32 rl_m018_06C90;
extern s32 rl_m018_06C94;
extern s16 rl_m018_06C98[4]; /* the images loaded for the d-pad */
extern f32 rl_m018_06CA0;
extern f32 rl_m018_06CA4;
extern s16 rl_m018_06CA8;
extern Object *rl_m018_06CB0[8];
extern u8 rl_m018_06CF8;
extern void *rl_m018_06D10;
extern s16 rl_m018_06D18;
extern u8 rl_m018_06D1A;
extern u8 rl_m018_06D1B;

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00000);

void func_m018_000CC(void) {
    Object *player;
    void *state;
    u8 *settings;
    s32 i;

    player = func_8000E4D0(0);
    state = player->unk68;
    i = func_m018_0207C(state);
    menu_assetgroup_load(rl_m018_06684);
    menu_assetgroup_reset(rl_m018_0668C);
    func_m009_00408();
    settings = get_settings();
    rl_m018_06BDC = (settings + 0x1CD)[i * 0x76];
    D_800FF780[31].unk6 = 0x90;
    rl_m018_06BE4 = 0;
}

void func_m018_0015C(void) {
    menu_assetgroup_free(rl_m018_06684);
    func_m009_0066C();
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00188);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00AEC);

/* Frees the d-pad's images, each once */
void func_m018_00EF0(void) {
    s32 i;
    s32 j;
    s32 image;

    i = 0;
    while (i < 4) {
        image = rl_m018_06C98[i++];
        if (image != -1) {
            menu_item_free(image);
            for (j = i; j < 4; j++) {
                if (image == rl_m018_06C98[j]) {
                    rl_m018_06C98[j] = -1;
                }
            }
        }
    }
}

void func_m018_01028(void) {
    Object *player;
    s8 *state;
    M018Character *character;

    player = func_8000E4D0(0);
    state = player->unk68;
    menu_assetgroup_load(rl_m018_06694);
    menu_assetgroup_reset(rl_m018_066A0);
    character = func_80047C1C(state[0], func_m018_0207C(state));
    rl_m018_01194(character->weapon);
    rl_m018_06CA0 = 0.0f;
    rl_m018_06CA4 = rl_m018_066B4[character->weapon];
    rl_m018_06CA8 = 30;
    rl_m018_06BCC = 0;
    if (options_screen_mode_get() & 1) {
        D_800FF780[24].unkC *= rl_m018_06B68;
    }
    rl_m018_06C88 = 0xFF;
    rl_m018_06C8C = 0;
    rl_m018_06C90 = 0;
    rl_m018_06C98[0] = -1;
    rl_m018_06C98[1] = -1;
    rl_m018_06C98[2] = -1;
    rl_m018_06C98[3] = -1;
    rl_m018_06C94 = 0;
}

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
    func_m018_011E4();
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_01194);

/* Frees the weapon's image */
static void func_m018_011E4(void) {
    if (rl_m018_066AC) {
        s32 image = rl_m018_066AC;
        menu_item_free(image);
        rl_m018_066AC = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_01218);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_01294);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_0130C);

/* The player's character (its settings' when the state says 3) */
static s32 func_m018_0207C(s8 *state) {
    s32 character = state[1] & 3;

    if (character == 3) {
        return (u16) (*(u16 *) (get_settings() + 0x2C2) & 3);
    }
    return character;
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_020C0);

void func_m018_023E8(void) {
    s32 i;

    menu_item_free(rl_m018_06CF8);
    menu_assetgroup_free(rl_m018_06718);
    for (i = 0; i < 8; i++) {
        func_800079B8(rl_m018_06CB0[i]);
    }
    gParticlePtrList_flush();
    func_8004AA80(rl_m018_06D10);
}

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

void *func_m018_0409C(void) {
    return rl_m018_06BB0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_040A8);

void func_m018_042D4(void) {
    Object *player;
    void *state;
    s32 i;

    menu_assetgroup_load(rl_m018_067E0);
    menu_assetgroup_reset(rl_m018_067E8);
    rl_m018_06BCC = 0;
    func_8001149C(D_800FF3F8[0x87], 0, 0.0f);
    func_8001149C(D_800FF3F8[0x88], 0, 0.0f);
    player = func_8000E4D0(0);
    state = player->unk68;
    i = func_m018_0207C(state);
    menu_assetgroup_load(&rl_m018_067F4[i * 3]);
    D_800FF780[49].unk6 = rl_m018_067F4[i * 3 + 1];
    D_800FF780[31].unk6 = 0x8F;
    rl_m018_06D18 = 0;
    rl_m018_06D1A = 0;
    rl_m018_06D1B = 0;
}

void func_m018_043AC(void) {
    Object *player;
    void *state;
    s32 i;

    player = func_8000E4D0(0);
    state = player->unk68;
    i = func_m018_0207C(state);
    menu_assetgroup_free(rl_m018_067E0);
    menu_assetgroup_free(&rl_m018_067F4[i * 3]);
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_04408);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_048E4);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_04AF0);

void func_m018_05788(void) {
    menu_assetgroup_free(rl_m018_06810);
    menu_item_free(rl_m018_06BD1 + 0x6F);
    if (!rl_m018_06BF5) {
        func_8005342C(0x2B);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_057D8);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05800);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05850);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05880);

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05A24);

void func_m018_05B40(void) {
    menu_assetgroup_load(rl_m018_0692C);
    menu_item_load(0x91);
    rl_m018_06C74 = 0x91;
    menu_assetgroup_reset(rl_m018_06930);
    D_800FF780[31].unk6 = 0x92;
    rl_m018_06C60 = 0;
    rl_m018_06C70 = 0;
    rl_m018_06C68 = 0xFF;
    rl_m018_06C64 = 0;
    rl_m018_06C6C = 0;
    rl_m018_06C78 = 0;
    rl_m018_06C80 = 0;
    rl_m018_06C7C = 0;
    rl_m018_06C84 = 0;
}

void func_m018_05BD8(void) {
    menu_assetgroup_free(rl_m018_0692C);
    menu_item_free(rl_m018_06C74);
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05C0C);

INCLUDE_ASM("asm/nonmatchings/modules/m018", data);

INCLUDE_ASM("asm/nonmatchings/modules/m018", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m018", bss);

