/*
 * Runlink module 18: front-end screens (character, weapon and health
 * screens; still mostly assembly).
 */
#include "module.h"
#include "gfx.h"

/* The menu items (D_800FF780, size 0x20) */
typedef struct M018MenuItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 pad14[0x20 - 0x14];
} M018MenuItem;

/* A character's record (func_80047C1C) */
typedef struct M018Character {
    /* 0x00 */ s16 unk0; /* in 1/256ths */
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u8 pad4[2];
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u16 unk8;
    /* 0x0A */ u16 unkA; /* the weapons held, a bit each */
    /* 0x0C */ u8 padC[0x12 - 0xC];
    /* 0x12 */ s16 unk12;
    /* 0x14 */ u8 pad14[0x70 - 0x14];
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
extern s8 rl_m018_06D1A;
extern s8 rl_m018_06D1B;

extern Gfx *D_800FF398; /* the display list being built */
extern Mtx *D_800FF39C;
extern char **gMenuText;
extern void func_8004DFC4(Gfx **dl, Mtx **mtx, s32 arg2); /* draws the background */
extern void func_80055B80(Gfx **dl);                     /* resets the render settings */
extern void set_active_camera(s32 camera);                   /* sets the active camera */
extern void cam_fov_scale_set(f32 x, f32 y);                 /* scales the field of view */
extern void func_8003F7F4(f32 fov, s32 arg1);            /* sets the field of view */
extern void func_8004046C(Gfx **dl, s32 arg1);
extern void func_80040E64(Gfx **dl, Mtx **mtx);
extern void func_8003F1C8(void *model, s32 id, s32 value); /* sets a texture's frame */
extern void func_8005A0D0(s32 item);                     /* draws a menu item */
extern void func_8006FC54(s32 font);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern s32 func_80043510(s32 player);               /* the buttons pressed */
extern void func_m041_008B4(s32 arg0, s32 arg1, s32 updateRate);
extern void func_m019_001B0(s32 screen);
extern s32 D_m019_013D8;

/* Draws the background and sets up the camera for the screen's objects */
static void func_m018_00000(void) {
    func_8004DFC4(&D_800FF398, &D_800FF39C, 0);
    func_80055B80(&D_800FF398);
    gDPSetFogColor(D_800FF398++, 255, 255, 255, 255);
    gSPFogFactor(D_800FF398++, 0x6400, 0x9867);
    gDPPipeSync(D_800FF398++);
    set_active_camera(0);
    func_8003F7F4(52.0f, 1);
    func_8004046C(&D_800FF398, 0);
    func_80040E64(&D_800FF398, &D_800FF39C);
}

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

extern u16 rl_m018_066F8[15]; /* each weapon's bit */

/* Picks the next weapon the character holds */
static void func_m018_01218(M018Character *record) {
    s32 weapon = rl_m018_066B0;

    do {
        weapon++;
        if (weapon >= 15) {
            weapon = 0;
        }
    } while (!(rl_m018_066F8[weapon] & record->unkA));
    if (weapon != rl_m018_066B0) {
        func_m018_011E4();
        func_m018_01194(weapon);
    }
}

/* Picks the previous weapon the character holds */
static void func_m018_01294(M018Character *record) {
    s32 weapon = rl_m018_066B0;

    do {
        weapon--;
        if (weapon < 0) {
            weapon = 14;
        }
    } while (!(rl_m018_066F8[weapon] & record->unkA));
    if (weapon != rl_m018_066B0) {
        func_m018_011E4();
        func_m018_01194(weapon);
    }
}

/* A model instance whose texture frames these screens set */
typedef struct M018Model {
    /* 0x0 */ u8 pad0[0xA];
    /* 0xA */ s8 unkA;
} M018Model;

/* What render_ortho_triangle_image draws an image with */
typedef struct M018Sprite {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 unk28;
} M018Sprite;

extern Vertex *D_800FF3A0;
extern s32 D_800A51CC; /* the text opacity */
extern s8 D_800FF3D8[]; /* the stick moves this frame, per player */
extern s32 func_800434E4(s32 player); /* the buttons held */
extern s32 func_80048E2C(s32 min, s32 max); /* a random number */
extern void func_8001138C(Object *obj, f32 speed, f32 updateRate); /* advances the object's animation */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_80056000(s32 arg0);
extern void render_ortho_triangle_image(Gfx **dl, Mtx **mtx, Vertex **vtx, M018Sprite *sprite, void *tex, s32 arg5, s32 alpha);
extern f32 rl_m018_06B6C;
extern char rl_m018_06AF8[];
extern char rl_m018_06B00[];
extern char rl_m018_06B08[];
extern char rl_m018_06B14[];
void func_m018_00AEC(void);
extern void rl_m018_040A8(f32 rate, s32 item); /* the static at 040A8 (still assembly) */

/* Picks a weapon with the stick and assigns it to a d-pad direction */
void func_m018_0130C(s32 updateRate) {
    UNUSED s32 pad;
    s32 sel;
    s32 oldWeapon;
    s32 buttons;
    char buf[12];
    Gfx *gfx;
    f32 pct;
    M018Character *record;
    s8 *state;
    s32 weapon;
    Object *obj;
    M018Model *model;
    u8 *settings;
    M018Sprite sprite;
    UNUSED s32 pad3;

    settings = get_settings();
    buttons = func_80043510(D_m019_013D8);
    func_m018_00000();
    rl_m018_040A8(updateRate, 0x21);
    if (rl_m018_066AC && rl_m018_06BCC) {
        state = func_8000E4D0(0)->unk68;
        record = func_80047C1C(state[0], func_m018_0207C(state));
        oldWeapon = rl_m018_066B0;
        if (D_800FF3D8[D_m019_013D8] < -32 && rl_m018_06C94 == 0) {
            func_m018_01294(record);
        }
        if (D_800FF3D8[D_m019_013D8] > 32 && rl_m018_06C94 == 0) {
            func_m018_01218(record);
        }
        if (oldWeapon != rl_m018_066B0) {
            func_80001248(0x16B, 0);
            rl_m018_06C88 = 255;
            rl_m018_06C8C = 0;
            rl_m018_06C90 = 0;
        }
        D_800FF780[24].unk0 += updateRate << 8;
        obj = D_800FF3F8[rl_m018_066AC];
        model = *obj->modelInstances;
        if (rl_m018_066B0 == 10) {
            if (rl_m018_06B6C < obj->unk28) {
                D_800FF780[24].unk4 += (0x3000 - D_800FF780[24].unk4) >> 2;
            } else {
                D_800FF780[24].unk4 += (0 - D_800FF780[24].unk4) >> 2;
            }
        } else {
            obj->particleEmittersEnabled = 0;
            func_8005F5AC(obj, updateRate);
            D_800FF780[24].unk4 += (0 - D_800FF780[24].unk4) >> 2;
        }
        if (rl_m018_06CA8 > 0) {
            if (updateRate < rl_m018_06CA8) {
                rl_m018_06CA8 -= updateRate;
            } else {
                rl_m018_06CA8 = 0;
                rl_m018_06CA0 = rl_m018_06CA4;
            }
        } else if (obj->unk28 == 0.0f && rl_m018_06CA0 < 0.0f) {
            rl_m018_06CA8 = func_80048E2C(120, 300);
            rl_m018_06CA4 = rl_m018_066B4[rl_m018_066B0];
        } else if (obj->unk28 == 1.0f && rl_m018_06CA0 > 0.0f) {
            rl_m018_06CA8 = func_80048E2C(120, 300);
            rl_m018_06CA4 = -rl_m018_066B4[rl_m018_066B0];
        }
        if (rl_m018_06C90 > 0) {
            rl_m018_06C88 -= updateRate * 32;
            rl_m018_06C8C += updateRate * 32;
            if (rl_m018_06C88 < 0) {
                rl_m018_06C88 = 0;
            }
            if (rl_m018_06C8C >= 256) {
                rl_m018_06C8C = 255;
            }
            rl_m018_06C90 -= updateRate;
        } else if (rl_m018_06C88 != 255) {
            rl_m018_06C88 += updateRate << 5;
            rl_m018_06C8C -= updateRate << 5;
            if (rl_m018_06C88 >= 256) {
                rl_m018_06C88 = 255;
            }
            if (rl_m018_06C8C < 0) {
                rl_m018_06C8C = 0;
            }
        }
        func_8001138C(obj, rl_m018_06CA0, updateRate);
        model->unkA = updateRate;
        if (rl_m018_06C88) {
        }
        if (rl_m018_06C88 > 0) {
            D_800A51CC = rl_m018_06C88;
            func_8005A0D0(0x18);
            D_800A51CC = 255;
        }
        func_8006FC54(2);
        func_8006FC68(255, 255, 255, 255, 255);
        func_8006FCA8(&D_800FF398, 0x52, 0x2D, gMenuText[175], 4);
        func_8006FCA8(&D_800FF398, 0x52, 0x4B, gMenuText[176], 4);
        func_8006FC68(255, 255, 0, 255, 255);
        weapon = rl_m018_066B0;
        func_800639E8(buf, rl_m018_06AF8, ((u16 *) ((u8 *) record + 0x14))[weapon]);
        func_8006FCA8(&D_800FF398, 0x52, 0x3A, buf, 4);
        func_800639E8(buf, rl_m018_06B00, ((u16 *) ((u8 *) record + 0x34))[weapon]);
        func_8006FCA8(&D_800FF398, 0x52, 0x58, buf, 4);
        func_8006FC68(255, 255, 255, 255, 255);
        if (rl_m018_066B0 == 0 || rl_m018_066B0 == 1 || rl_m018_066B0 == 5 || rl_m018_066B0 == 2 ||
            rl_m018_066B0 == 3 || rl_m018_066B0 == 7) {
            func_8006FC68(255, 255, 255, 255, 255);
            func_8006FCA8(&D_800FF398, 0x52, 0x69, gMenuText[178], 4);
            func_8006FC68(255, 255, 0, 255, 255);
            if (((u32 *) (settings + 0x4B4))[rl_m018_066B0] == 0) {
                pct = 0.0f;
            } else {
                pct = (100.0f * ((u32 *) (settings + 0x4B4))[rl_m018_066B0]) /
                      ((u32 *) (settings + 0x474))[rl_m018_066B0];
            }
            func_800639E8(buf, rl_m018_06B08, &pct);
            func_8006FCA8(&D_800FF398, 0x52, 0x76, buf, 4);
        } else {
            func_8006FC68(255, 255, 255, 255, 255);
            func_8006FCA8(&D_800FF398, 0x52, 0x69, gMenuText[177], 4);
            func_8006FC68(255, 255, 0, 255, 255);
            func_800639E8(buf, rl_m018_06B14, ((u16 *) (settings + 0x4F4))[weapon]);
            func_8006FCA8(&D_800FF398, 0x52, 0x76, buf, 4);
        }
        func_8006FCA8(&D_800FF398, 0x37, 0x96, gMenuText[179 + rl_m018_066B0 * 5], 0);
        func_8006FCA8(&D_800FF398, 0x37, 0xA0, gMenuText[180 + rl_m018_066B0 * 5], 0);
        func_8006FCA8(&D_800FF398, 0x37, 0xAA, gMenuText[181 + rl_m018_066B0 * 5], 0);
        func_8006FCA8(&D_800FF398, 0x37, 0xB4, gMenuText[182 + rl_m018_066B0 * 5], 0);
        func_8006FCA8(&D_800FF398, 0x37, 0xBE, gMenuText[183 + rl_m018_066B0 * 5], 0);
        func_8006FC68(255, 255, 255, 255, rl_m018_06C88);
        func_8006FCA8(&D_800FF398, 0xD2, 0x26, gMenuText[57 + rl_m018_066B0], 4);
    }
    sel = -1;
    if (rl_m018_06C94 == 0) {
        if (buttons & 0x4000) {
            func_m019_001B0(1);
            rl_m018_06C94 = 1;
        } else if (buttons & 0x800) {
            sel = 0;
        } else if (buttons & 0x100) {
            sel = 1;
        } else if (buttons & 0x400) {
            sel = 2;
        } else if (buttons & 0x200) {
            sel = 3;
        }
    }
    if (sel >= 0 && sel < 4) {
        settings[0x534 + sel] = rl_m018_066B0;
        func_80001248(0x16D, 0);
        rl_m018_06C90 = 300;
        func_m018_00AEC();
    }
    if (func_800434E4(D_m019_013D8) & 0x2000) {
        if (rl_m018_06C94 == 0) {
            if (rl_m018_06C90 == 0) {
                func_m018_00AEC();
            }
            rl_m018_06C90 = 60;
        }
    }
    cam_fov_scale_set(1.0f, 1.0f);
    func_8003F7F4(52.0f, 1);
    func_80040E64(&D_800FF398, &D_800FF39C);
    func_8005A0D0(0x21);
    func_8005A0D0(0x22);
    func_8005A0D0(0x23);
    func_8005A0D0(0x24);
    func_8003F7F4(52.0f, 1);
    if (rl_m018_06BCC && rl_m018_06C8C > 0) {
        func_80040D64(&D_800FF398, &D_800FF39C);
        func_80056000(0);
        gfx = D_800FF398++;
        gfx->words.w0 = 0xFA000000;
        gfx->words.w1 = (rl_m018_06C8C & 0xFF) | ~0xFF;
        sprite.trans.rotation.y_rotation = 0;
        sprite.trans.rotation.x_rotation = 0;
        sprite.trans.rotation.z_rotation = 0;
        sprite.trans.scale = 1.0f;
        sprite.trans.z_position = 1.0f;
        sprite.unk28 = 0.0f;
        sprite.trans.x_position = 46.0f;
        sprite.trans.y_position = 40.0f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[0x99], 0, 255);
        sprite.trans.x_position = 46.0f;
        sprite.trans.y_position = 70.0f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[rl_m018_06C98[0]], 0, 255);
        sprite.trans.x_position = 80.0f;
        sprite.trans.y_position = 40.0f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[rl_m018_06C98[1]], 0, 255);
        sprite.trans.x_position = 46.0f;
        sprite.trans.y_position = 10.0f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[rl_m018_06C98[2]], 0, 255);
        sprite.trans.x_position = 12.0f;
        sprite.trans.y_position = 40.0f;
        render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[rl_m018_06C98[3]], 0, 255);
        func_80056000(0);
        gfx = D_800FF398++;
        gfx->words.w0 = 0xFA000000, gfx->words.w1 = 0xFFFFFFFF;
    }
}

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
extern void dir_light_setup(void *light, s32 a, s32 b, s32 c, s32 d, s32 e);
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
        dir_light_setup((u8 *) obj->shading + 0x10, ((M018LitHeader *) obj->header)->unk94 * 255.0f,
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

/* A texture of a shown model */
typedef struct M018Tex {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 pad8[0x12 - 8];
    /* 0x12 */ u16 unk12;
    /* 0x14 */ u8 pad14[2];
    /* 0x16 */ s16 unk16;
} M018Tex;

typedef struct M018TexRef {
    /* 0x0 */ M018Tex *tex;
    /* 0x4 */ u8 pad4[4];
} M018TexRef;

typedef struct M018ModelHeader {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M018TexRef *textures;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ u8 numBatches;
} M018ModelHeader;

/* A model batch's texture animation */
typedef struct M018TexAnim {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s32 unk4;
} M018TexAnim;

typedef struct M018AnimModel {
    /* 0x00 */ M018ModelHeader *header;
    /* 0x04 */ u8 pad4[0xB - 4];
    /* 0x0B */ s8 unkB;
    /* 0x0C */ u8 padC[0x74 - 0xC];
    /* 0x74 */ M018TexAnim *anims;
    /* 0x78 */ s16 *frames[1];
} M018AnimModel;

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


extern s8 rl_m018_06808[4];
extern char rl_m018_06B1C[];

void func_m018_04408(s32 updateRate) {
    s8 *state;
    s32 health;
    s32 character;
    s32 count;
    M018Character *record;
    s32 i;
    Object *obj;
    M018Model *model;
    s32 part;
    s32 full;
    s32 frame;
    char buf[80];

    func_m018_00000();
    state = func_8000E4D0(0)->unk68;
    character = func_m018_0207C(state);
    record = func_80047C1C(state[0], character);
    health = record->unk0 >> 8;

    count = record->unk2;
    D_800FF780[47].unk0 += updateRate << 8;
    D_800FF780[48].unk0 -= updateRate << 8;
    obj = D_800FF3F8[0x88];
    model = *obj->modelInstances;
    model->unkA = updateRate;
    part = health % (count + 1);
    full = health / (count + 1);
    if (rl_m018_06D18 < 360) {
        if (part == 0 && rl_m018_06D18 < 240) {
            rl_m018_06D18 = 240;
        } else {
            rl_m018_06D18 += updateRate;
        }
    }
    if (rl_m018_06D1A != health % (count + 1)) {
        i = rl_m018_06D18;
        if ((i /= 12) < part) {
            part = i;
        } else {
            rl_m018_06D18 = 240;
        }
    }
    if (rl_m018_06D1A < part) {
        func_80001248(0x16B, 0);
    }
    rl_m018_06D1A = part;
    if (rl_m018_06D18 > 240) {
        i = rl_m018_06D18 - 240;
        if (rl_m018_06D1B != health / (count + 1)) {
            if ((i /= 12) < health / (count + 1)) {
                full = i;
            }
        }
        if (full == health / (count + 1)) {
            rl_m018_06D18 = 480;
        }
    } else {
        full = 0;
    }
    if (rl_m018_06D1B < full) {
        func_80001248(0x16C, 0);
    }
    rl_m018_06D1B = full;
    frame = 4 - full;
    frame <<= 8;
    func_8003F1C8(model, 0, frame);
    for (i = 0; i < 4; i++) {
        if (full) {
            func_8003F1C8(model, rl_m018_06808[i], 0);
            full--;
        } else {
            func_8003F1C8(model, rl_m018_06808[i], 0x100);
        }
    }
    cam_fov_scale_set(1.0f, 1.0f);
    func_8003F7F4(52.0f, 1);
    func_80040E64(&D_800FF398, &D_800FF39C);
    func_8005A0D0(0x2F);
    obj = D_800FF3F8[0x87];
    model = *obj->modelInstances;
    model->unkA = updateRate;
    for (i = 0; i < 6; i++) {
        if (part) {
            part--;
            func_8003F1C8(model, i, 0);
        } else {
            func_8003F1C8(model, i, 0x100);
        }
    }
    for (i = 6; i < count; i++) {
        obj->modelIndex++;
        model = obj->modelInstances[obj->modelIndex];
        if (part) {
            part--;
            func_8003F1C8(model, 0, 0);
        } else {
            func_8003F1C8(model, 0, 0x100);
        }
        func_8005A0D0(0x2F);
    }
    obj->modelIndex = 0;
    func_8005A0D0(0x30);
    func_8005A0D0(0x1F);
    func_m041_008B4(1, rl_m018_06D18 > 360, updateRate);
    func_8006FC54(2);
    func_8006FC68(255, 255, 255, 255, 255);
    func_8006FCA8(&D_800FF398, 0x2C, 0xA0, gMenuText[character + 397], 0);
    func_8006FC68(255, 255, 0, 255, 255);
    i = health / 8;
    if (i >= 8) {
        i = 7;
    }
    func_800639E8(buf, rl_m018_06B1C, gMenuText[i + 400], health, count * 5 + 4);
    func_8006FCA8(&D_800FF398, 0x3E, 0xAF, buf, 0);
    if (func_80043510(D_m019_013D8) & 0x4000) {
        func_m019_001B0(1);
    }
}

/* A part of the shown object's model (12 bytes) */
typedef struct M018Part {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
} M018Part;

/* The shown object's model instance */
typedef struct M018ShownModel {
    /* 0x00 */ u8 pad0[0x68];
    /* 0x68 */ M018Part *parts;
} M018ShownModel;

/* A highlighted part (three in the module's .bss) */
typedef struct M018Glow {
    /* 0x0 */ s8 active;
    /* 0x1 */ s8 r;
    /* 0x2 */ s8 g;
    /* 0x3 */ s8 b;
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

/* The first highlight's fields, as the functions below read them */
#define rl_m018_06BF8_active (*(s8 *) rl_m018_06BF8)
extern s8 rl_m018_06BF9;
extern s8 rl_m018_06BFA;
extern s8 rl_m018_06BFB;
extern M018Part *rl_m018_06BFC;

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05850);

/* A rectangle for menu_fill_boxes */
typedef struct M018Rect {
    /* 0x0 */ s16 x0;
    /* 0x2 */ s16 y0;
    /* 0x4 */ s16 x1;
    /* 0x6 */ s16 y1;
    /* 0x8 */ u32 colour;
} M018Rect;

extern s32 func_80041E58(f32 x, f32 y, f32 z, f32 *screenX, f32 *screenY, s32 arg5); /* projects a point */
extern void menu_fill_boxes(Gfx **dl, s32 count, M018Rect *rects, s32 arg3);      /* draws rectangles */


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

/* A position on the screen (four per page) */
typedef struct M018Pos {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} M018Pos;

extern s32 character_has_item(s32 player, s32 character, s32 item);
extern void mempool_free_timer(s32 frames);
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_80055C00(Gfx **dl, void *tex, s32 arg2, s32 arg3);
extern s32 D_800A51CC; /* the text opacity */
extern s8 D_800FF3D8[]; /* the stick moves this frame, per player */
extern M018Pos rl_m018_06C10[];
extern f32 rl_m018_067B8;
extern s16 rl_m018_06938[];
extern s32 rl_m018_069D0[];
extern s16 rl_m018_069EC[];
extern Vertex rl_m018_069F0[];
extern Triangle rl_m018_06A30[];
extern char rl_m018_06B2C[];
extern char rl_m018_06B30[];
extern char rl_m018_06B34[];
extern f32 rl_m018_06B90;
extern f32 rl_m018_06B94;
extern f32 rl_m018_06B98;
extern f32 rl_m018_06B9C;
extern f32 rl_m018_06BA0;

INCLUDE_ASM("asm/nonmatchings/modules/m018", func_m018_05C0C);

INCLUDE_ASM("asm/nonmatchings/modules/m018", data);

INCLUDE_ASM("asm/nonmatchings/modules/m018", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m018", bss);

