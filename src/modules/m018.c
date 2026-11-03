/*
 * Runlink module 18: front-end screens (character, weapon and health
 * screens; still mostly assembly).
 */
#include "module.h"

/* The menu items (D_800FF780, size 0x20) */
typedef struct M018MenuItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 pad2[6 - 2];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 pad14[0x20 - 0x14];
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
extern void func_80054F90(void);
extern s32 runlink_module_loaded(s32 module); /* a module's load address, 0 when not loaded */
extern M018MenuItem D_800FF780[];
extern Object *D_800FF3F8[];

static void func_m018_01194(s32 weapon);
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

extern u8 *get_settings(void);

/*
 * Sets the d-pad's four images from the character's record: frees the old
 * images that are no longer used and loads the new ones (each once).
 */
#ifdef NON_MATCHING
/* Same code, frame and slots; the argument/temp registers of the inner loops are one off (ROM: image a3, j a1, found a2) */
void func_m018_00AEC(void) {
    s32 i;
    s32 nOld;
    s32 nNew;
    s32 j;
    s32 image;
    s32 found;
    s16 old[4];
    s16 new[4];
    u8 *record;

    record = get_settings();
    nOld = 0;
    for (i = 0; i < 4; i++) {
        image = rl_m018_06C98[i];
        if (image != -1) {
            found = 0;
            for (j = 0; j < nOld; j++) {
                if (image == old[j]) {
                    found = 1;
                }
            }
            if (!found) {
                old[nOld++] = image;
            }
        }
    }
    nNew = 0;
    for (i = 0; i < 4; i++) {
        image = record[0x534 + i] + 10;
        rl_m018_06C98[i] = image;
        found = 0;
        for (j = 0; j < nNew; j++) {
            if (image == new[j]) {
                found = 1;
            }
        }
        if (!found) {
            new[nNew++] = image;
        }
    }
    for (i = 0; i < nOld; i++) {
        image = old[i];
        found = 0;
        for (j = 0; j < nNew; j++) {
            if (image == new[j]) {
                found = 1;
            }
        }
        if (!found) {
            menu_item_free(image);
        }
    }
    for (i = 0; i < nNew; i++) {
        image = new[i];
        found = 0;
        for (j = 0; j < nOld; j++) {
            if (image == old[j]) {
                found = 1;
            }
        }
        if (!found) {
            menu_item_load(image);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_00AEC);
#endif

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
    func_m018_01194(character->weapon);
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

/* Picks a weapon: loads its image and shows it */
static void func_m018_01194(s32 weapon) {
    rl_m018_066AC = weapon + 0x4A;
    rl_m018_066B0 = weapon;
    menu_item_load(rl_m018_066AC);
    D_800FF780[24].unk6 = rl_m018_066AC;
    func_80054F90();
}

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

/* A level entry for the objects this screen spawns */
typedef struct M018Entry {
    /* 0x0 */ s16 objectID;
    /* 0x2 */ u8 size;
    /* 0x3 */ u8 pad3;
    /* 0x4 */ s16 x;
    /* 0x6 */ s16 y;
    /* 0x8 */ s16 z;
} M018Entry;

/* The light settings in the shown object's header */
typedef struct M018LitHeader {
    /* 0x00 */ u8 pad0[0x90];
    /* 0x90 */ f32 unk90;
    /* 0x94 */ f32 unk94;
    /* 0x98 */ u8 pad98[0x9C - 0x98];
    /* 0x9C */ s16 unk9C;
    /* 0x9E */ s16 unk9E;
    /* 0xA0 */ s16 unkA0;
} M018LitHeader;

/* The shown object's state in the object (at 0x88) */
typedef struct M018ShownState {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s32 unk4;
} M018ShownState;

/* An entry of the list this screen allocates (40 of them) */
typedef struct M018ListEntry {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 pad12[0x1C - 0x12];
} M018ListEntry;

typedef struct M018Pair {
    /* 0x0 */ u8 pad0[6];
    /* 0x6 */ s16 unk6;
} M018Pair;

extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern void func_80022520(void *light, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void *func_8004A57C(s32 size, u32 colour); /* DKR mempool_alloc_safe */

extern s32 rl_m018_06680;
extern u8 rl_m018_06724[];
extern f32 rl_m018_0675C[];
extern M018Pair rl_m018_06774[8];
extern f32 rl_m018_06B70;
extern f32 rl_m018_06CD0[8];
extern f32 rl_m018_06CF0;
extern f32 rl_m018_06CF4;
extern s32 rl_m018_06CFC;
extern s32 rl_m018_06D00;
extern u8 rl_m018_06D04;
extern s8 rl_m018_06D05;
extern s8 rl_m018_06D07;
extern u8 rl_m018_06D08;
extern u8 rl_m018_06D09;
extern s16 rl_m018_06D0A;
extern s16 rl_m018_06D0C;
extern s16 rl_m018_06D0E;
extern s16 rl_m018_06D14;
extern s16 rl_m018_06D16;

void func_m018_020C0(void) {
    s8 *state;
    s32 character;
    Object *obj;
    s32 i;
    s32 loaded;
    UNUSED s32 pad[2];
    M018Entry entry;

    loaded = runlink_module_loaded(0x14);
    state = func_8000E4D0(0)->unk68;
    character = func_m018_0207C(state);
    if (state[1] & 4) {
        rl_m018_06CF8 = character + 0x9A;
    } else {
        rl_m018_06CF8 = character + 0x5E;
    }
    rl_m018_06CFC = 0;
    rl_m018_06D00 = 0;
    menu_item_load(rl_m018_06CF8);
    menu_assetgroup_load(rl_m018_06718);
    menu_assetgroup_reset(rl_m018_06724);
    obj = D_800FF3F8[0x65];
    obj->modelIndex = 0;
    D_800FF780[26].unk6 = rl_m018_06CF8;
    D_800FF780[26].unk8 = rl_m018_0675C[character];
    obj = D_800FF3F8[rl_m018_06CF8];
    if (obj->shading != NULL) {
        func_80022520((u8 *) obj->shading + 0x10, ((M018LitHeader *) obj->header)->unk94 * 255.0f,
                      ((M018LitHeader *) obj->header)->unk90 * 255.0f, ((M018LitHeader *) obj->header)->unk9C,
                      ((M018LitHeader *) obj->header)->unk9E, ((M018LitHeader *) obj->header)->unkA0);
        ((u8 *) obj)[0xA1] = 1;
    }
    OBJ_STATE(obj, M018ShownState)->unk4 = 0;
    entry.objectID = 0x1B0;
    entry.size = 10;
    entry.x = 0;
    entry.y = 0;
    entry.z = 0;
    for (i = 0; i < 8; i++) {
        rl_m018_06CB0[i] = func_80005F14(&entry, 0);
        rl_m018_06CB0[i]->modelIndex = i & 1;
        rl_m018_06CD0[i] = 0.0f;
        rl_m018_06774[i].unk6 = 0;
    }
    if (loaded == 0) {
        func_8005342C(0x14);
    }
    rl_m018_06D04 = 0;
    rl_m018_06D05 = -1;
    rl_m018_06CF0 = rl_m018_06B70;
    rl_m018_06CF4 = 0.0f;
    rl_m018_06D07 = -1;
    rl_m018_06D0A = 0;
    rl_m018_06D0C = 0xFF;
    rl_m018_06D08 = 0;
    rl_m018_06D09 = 0;
    rl_m018_06D0E = D_800FF780[26].unk0;
    rl_m018_06D10 = func_8004A57C(0x460, -1);
    for (i = 0; i < 40; i++) {
        ((M018ListEntry *) rl_m018_06D10)[i].unk10 = -1;
    }
    rl_m018_06D14 = 0;
    rl_m018_06D16 = 0;
    rl_m018_06680 = 0;
}

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

/* A part of the shown object's model (12 bytes) */
typedef struct M018Part {
    /* 0x0 */ u8 pad0[0xC];
} M018Part;

/* The shown object's model instance */
typedef struct M018ShownModel {
    /* 0x00 */ u8 pad0[0x68];
    /* 0x68 */ M018Part *parts;
} M018ShownModel;

/* A highlighted part (three in the module's .bss) */
typedef struct M018Glow {
    /* 0x0 */ s8 active;
    /* 0x1 */ u8 r;
    /* 0x2 */ u8 g;
    /* 0x3 */ u8 b;
    /* 0x4 */ M018Part *part;
} M018Glow;

/* This screen's .data, .rodata and .bss (still assembly) */
extern u8 rl_m018_0681C[];
extern s16 rl_m018_0682C[];
extern s8 rl_m018_06874[];
extern s8 rl_m018_06898[][2];
extern f32 rl_m018_068E0[];
extern s8 rl_m018_06920[];
extern f32 rl_m018_06B88;
extern s8 rl_m018_06BD0;
extern s8 rl_m018_06BD2;
extern s8 rl_m018_06BD3;
extern u8 rl_m018_06BD4;
extern u8 rl_m018_06BD5;
extern u8 rl_m018_06BD6;
extern s16 rl_m018_06BD8;
extern s16 rl_m018_06BE0;
extern s16 rl_m018_06BE8;
extern f32 rl_m018_06BEC;
extern f32 rl_m018_06BF0;
extern u8 rl_m018_06BF4;
extern u8 rl_m018_06BF6;
extern M018Glow rl_m018_06BF8[3];

static void func_m018_057D8(void);
static void func_m018_05800(M018Part *part);

/* Sets up the screen that shows object 0x7B: its images, first part highlighted, its animation */
void func_m018_048E4(void) {
    s32 loaded;
    M018ShownModel *model;
    Object *obj;

    loaded = runlink_module_loaded(0x14);
    rl_m018_06BF5 = runlink_module_loaded(0x2B);
    menu_assetgroup_load(rl_m018_06810);
    menu_assetgroup_reset(rl_m018_0681C);
    rl_m018_06BCC = 0;
    if (loaded == 0) {
        func_8005342C(0x14);
    }
    func_m018_057D8();
    obj = D_800FF3F8[0x7B];
    model = *(M018ShownModel **) obj->modelInstances;
    rl_m018_06BD3 = 0;
    rl_m018_06BD1 = rl_m018_06874[rl_m018_06BD3];
    rl_m018_06BD0 = rl_m018_06BD1;
    menu_item_load(0x6F);
    D_800FF780[37].unk6 = 0x6F;
    func_m018_05800(&model->parts[rl_m018_06920[rl_m018_06BD0]]);
    D_800FF780[37].unkC = 0.0f;
    D_800FF780[37].unk10 = 0.0f;
    rl_m018_06BD8 = rl_m018_0682C[rl_m018_06BD1 * 3];
    D_800FF780[38].unk0 = rl_m018_06BD8;
    rl_m018_06BD2 = 0;
    rl_m018_06BEC = rl_m018_06B88;
    rl_m018_06BE0 = rl_m018_06898[rl_m018_06BD0][0];
    if (rl_m018_06BE0 == -1) {
        rl_m018_06BE0 = 0;
        rl_m018_06BF0 = 0.0f;
    } else {
        rl_m018_06BF0 = rl_m018_068E0[rl_m018_06BE0];
    }
    func_8001149C(D_800FF3F8[0x7B], rl_m018_06BE0, 0.0f);
    rl_m018_06BD4 = 0x78;
    rl_m018_06BE8 = 0x100;
    rl_m018_06BD5 = 0x1E;
    rl_m018_06BD6 = 0;
    rl_m018_06BF4 = 0;
    rl_m018_06BF6 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_04AF0);

void func_m018_05788(void) {
    menu_assetgroup_free(rl_m018_06810);
    menu_item_free(rl_m018_06BD1 + 0x6F);
    if (!rl_m018_06BF5) {
        func_8005342C(0x2B);
    }
}

/* Clears the highlights */
static void func_m018_057D8(void) {
    M018Glow *g = rl_m018_06BF8;
    s32 i = 3;

    while (i--) {
        g->active = 0;
        g++;
    }
}

/* Highlights a part in the first free slot */
static void func_m018_05800(M018Part *part) {
    M018Glow *g = rl_m018_06BF8;
    s32 i = 3;

    while (i--) {
        if (g->active == 0) {
            g->active = 1;
            g->r = 30;
            g->g = 30;
            g->b = 30;
            g->part = part;
            return;
        }
        g++;
    }
}

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

