/*
 * Runlink module 37: the options page of the front end (sound, music, screen
 * and the cheats), with its volume bars.
 */
#include "module.h"

#include "gfx.h"

/* the .rodata strings the data refers to (first in .rodata) */
INCLUDE_ASM("asm/nonmatchings/modules/m037/ro", rodata);

typedef struct M037Words4 {
    s32 w[4];
} M037Words4;
typedef struct M037Words2 {
    s32 w[2];
} M037Words2;

/* What gMenuText points at (only the fields used here) */
typedef struct M037Settings {
    /* 0x000 */ M037Words4 unk0;
    /* 0x010 */ u8 pad10[0x264 - 0x10];
    /* 0x264 */ M037Words2 unk264;
    /* 0x26C */ u8 pad26C[0x274 - 0x26C];
    /* 0x274 */ M037Words2 unk274;
    /* 0x27C */ u8 pad27C[0x4BC - 0x27C];
    /* 0x4BC */ M037Words2 unk4BC;
} M037Settings;

extern M037Settings *gMenuText;
extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */

/* The game's texts, by number */
#define MENU_TEXT(n) (((char **) gMenuText)[n])

#ifdef NON_MATCHING
/* The module's .data */
static Gfx sBoxDL[] = {
    { { 0xFC71FEE3, 0xFFFEFB7D } },
    { { 0xEF000C0F, 0x00504240 } },
};
static Gfx sBarDL[] = {
    { { 0xFCFFFFFF, 0xFFFDF6FB } },
    { { 0xEF000C0F, 0x0F0A4000 } },
};
static char *sControllerLabels[4] = { "1 - ", "2 - ", "3 - ", "4 - " };
static s16 sAssets[] = { 0x65, 0x7C, 0x7D, 0x7E, -1 };
static s16 sCheatAssets[] = { 0x43, 0x44, -1 };
static s16 sImages[] = { 0x1F, 0x27, 0x28, 0x29, 0x2A, -1 };
static char sButtonL[] = "\x80\x42"; /* button glyphs */
static char sButtonR[] = "\x80\x44";
static s8 sPartKinds[8] = { 6, 0, -1, 7, -1, -1, 4, -1 }; /* the model parts that show each setting */
static s8 sPartAngles[8] = { 0 };
static s8 sSoundAngles[4] = { 0, 1, 3, 2 };
static s8 sScreenAngles[4] = { 0, 1, 0, 0 };
static s8 sItemBatches[8] = { 2, 7, 3, 8, 4, 9, 5, 10 }; /* the model batch lit for each item */
static s8 sBoxHeights[10] = { 16, 44, 36, 16, 36, 36, 36, 36, 7, 0 };
static s32 sMode = 8; /* the item being changed, or 8: choosing one */
static s16 sPhase = 0; /* of the selected item's pulse */
static s16 sGlow = 0;  /* the yes/no choice's flashing */
u32 D_m037_01F40[3] = { 0x00FF00FF, 0xFFFF00FF, 0xFF0000FF }; /* the volume bar's steps' colours */
#define ITEM_SPEED 0.01f
#else
/*
 * The module's .data and .rodata are still assembly in the matching build (func_m037_001A0's jump
 * table and its local arrays' initial values are in them): reached from their first symbol.
 */
extern s16 rl_m037_01EB0[]; /* .data */
#define sAssets (&rl_m037_01EB0[0x18])      /* 0x01EE0 */
#define sCheatAssets (&rl_m037_01EB0[0x1E]) /* 0x01EEC */
#define sImages (&rl_m037_01EB0[0x22])      /* 0x01EF4 */
#define sBarDL ((u8 *) rl_m037_01EB0 + 0x10) /* 0x01EC0 */
extern u32 D_m037_01F40[3]; /* the volume bar's steps' colours (exported) */
#define ITEM_SPEED 0.01f /* 0.01f */
#endif

/* The page's state */
static s32 sSaved0[4]; /* copies of settings */
static s32 sSaved1[2];
static s32 sSaved2[4];
static s32 sSaved3[2];
static s32 sUnk30;
static s32 sUnk34;
static s16 sUnk38;
static s16 sUnk3A;
static f32 sUnk3C;
static s32 sUnk40;
static s32 sUnk44;
static s32 sUnk48;
static s32 sNoCheats; /* func_m037_00000's argument */
static s32 sUnk50;
static s32 sUnk54;
static s32 sUnk58;
static f32 sUnk5C;
static f32 sUnk60;

void func_m037_00000(s32 arg0) {
    menu_assetgroup_load(sAssets);
    menu_assetgroup_reset(sImages);
    if (arg0 == 0) {
        menu_assetgroup_load(sCheatAssets);
    }
    sUnk5C = ITEM_SPEED;
    sUnk40 = 0;
    sUnk54 = 1;
    sUnk50 = 0;
    sUnk44 = -1;
    sUnk38 = 0x36;
    sUnk3C = 7.0f;
    sUnk48 = 0;
    sSaved0[0] = gMenuText->unk0.w[0];
    sSaved0[1] = gMenuText->unk0.w[1];
    sSaved0[2] = gMenuText->unk0.w[2];
    sSaved0[3] = gMenuText->unk0.w[3];
    sSaved1[0] = gMenuText->unk4BC.w[0];
    sSaved1[1] = gMenuText->unk4BC.w[1];
    sSaved2[0] = gMenuText->unk264.w[0];
    sSaved2[1] = gMenuText->unk264.w[1];
    sSaved2[2] = gMenuText->unk264.w[0];
    sSaved2[3] = gMenuText->unk264.w[1];
    sSaved3[0] = gMenuText->unk274.w[0];
    sSaved3[1] = gMenuText->unk274.w[1];
    sNoCheats = arg0;
    sUnk30 = 0;
    sUnk3A = 10;
    sUnk60 = 0.0f;
    sUnk58 = 0;
}

void func_m037_00160(void) {
    menu_assetgroup_free(sAssets);
    if (sNoCheats == 0) {
        menu_assetgroup_free(sCheatAssets);
    }
}

/* The options console's model, as used here */
typedef struct M037Part {
    /* 0x0 */ s16 angle;
    /* 0x2 */ s16 angle2; /* -1: none */
    /* 0x4 */ s32 kind;   /* the low byte */
} M037Part;
typedef struct M037Texture {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 scale;
} M037Texture;
typedef struct M037TexEntry {
    /* 0x0 */ M037Texture *texture;
    /* 0x4 */ u8 pad4[4];
} M037TexEntry;
typedef struct M037Batch {
    /* 0x0 */ u8 pad0;
    /* 0x1 */ s8 kind;
    /* 0x2 */ u8 pad2[4];
    /* 0x6 */ s16 firstVertex;
    /* 0x8 */ u8 pad8[8];
} M037Batch;
typedef struct M037Model {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 batchCount;
    /* 0x18 */ M037TexEntry *textures;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ M037Batch *batches;
    /* 0x28 */ u8 pad28[4];
    /* 0x2C */ u8 partCount;
} M037Model;
typedef struct M037Instance {
    /* 0x00 */ M037Model *model;
    /* 0x04 */ u8 pad4[6];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0x18 - 0xC];
    /* 0x18 */ Vertex *vertices[2];
    /* 0x20 */ u8 pad20[0x74 - 0x20];
    /* 0x74 */ M037Part *parts;
    /* 0x78 */ s16 *angles[2];
} M037Instance;

extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern Object *D_800FF3F8[]; /* the menu's objects */
extern u8 D_800FF318[];      /* the global flags */
extern s32 D_800FF3C8[];     /* the buttons pressed this frame, per player */
extern s32 D_800FF3B8[];     /* the buttons held, per player */
extern s8 D_800FF3D8[];      /* the stick moves this frame, per player (x) */
extern s8 D_800FF3DC[];      /* (y) */
extern s32 D_800FB10C;       /* the number of controllers */
extern s32 D_800FECD0;
extern u8 D_800A5080; /* the cheats' switches */
extern u8 D_800A5084;
extern u8 D_800A5088;

/* A menu item (D_800FF780, size 0x20; the main segment's MenuItem), as used here */
typedef struct M037MenuItem {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 posY;
    /* 0x14 */ u8 pad14[0x20 - 0x14];
} M037MenuItem;

extern M037MenuItem D_800FF780[];

extern s32 options_screen_mode_get(void);
extern void func_8005A834(s32 arg0);
extern s32 options_screen_offset_get(void);
extern void options_screen_offset_set(s32 arg0);
extern s32 func_8005A98C(void);
extern void func_8005A99C(s32 arg0);
extern s32 func_8005A9E4(void);
extern void func_8005A9F4(s32 arg0);
extern s32 options_music_volume_get(void);
extern void options_music_volume_set(s32 arg0);
extern s32 func_8005AA7C(void);
extern void func_8005AA94(s32 arg0);
extern u8 func_8005AAB8(s32 player);
extern void func_8005AAD0(s32 player, s32 arg1);
extern void func_80054EA0(u32 *width, u32 *height); /* the frame buffer size */
extern void func_8004DFC4(Gfx **dl, Mtx **mtx, s32 arg2); /* draws the background */
extern void func_80055B80(Gfx **dl);
extern void func_m038_00000();
extern void set_active_camera(s32 arg0);
extern void cam_fov_scale_set(f32 arg0, f32 arg1);
extern void func_8003F7F4(f32 fov, s32 arg1); /* sets the field of view */
extern void viewport_apply(Gfx **dl, s32 arg1);
extern void func_80040E64(Gfx **dl, Mtx **mtx);
extern void func_800408C0(Gfx **dl);
extern void func_8005A0D0(s32 item); /* draws a menu item */
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3); /* sets a texture */
extern void func_8006FC54(s32 font);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern void func_80055228(void);
extern void func_8005483C(s32 arg0);
extern void load_first_unlocked_level(void);
extern void mark_write_eeprom_settings(void);
extern s8 *get_settings(void);
extern void func_8004842C(s32 arg0);
extern void func_m019_001B0(s32 screen);
extern void func_m019_00324(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_m037_01ACC(s32 value, s32 arg1, s32 top);

#ifdef NON_MATCHING
/* register allocation (the ROM keeps more of the pointers and flags in s-registers) and the frame */
/* The options page: moves the console's parts, draws the box and the items, and handles the pad */
void func_m037_001A0(s32 updateRate) {
    s32 pad124;
    s32 pad120;
    s32 pad11C;
    s32 k;
    s32 pad114;
    s32 pad110;
    s32 pad10C;
    s32 pad108;
    s32 bright;
    s32 left;
    s32 right;
    s32 up;
    s32 down;
    s32 pressA;
    s32 pressB;
    s32 cDown;
    s32 cUp;
    s32 boxLeft;
    s32 boxTop;
    s32 boxRight;
    s32 boxBottom;
    s32 padD0;
    s32 padCC;
    s32 alpha;
    Object *obj;
    s32 c;
    u32 width;
    u32 height;
    s32 i;
    s32 j;
    s32 n;
    s32 y;
    M037Instance *inst;
    s32 kind;
    M037Part *part;
    M037Model *model;
    s16 *out;
    M037Batch *batch;
    Vertex *v;
    f32 boxHeight;

    if (sUnk48 == 0) {
        if (sUnk58 < 30) {
            sUnk58 += updateRate;
        }
        if (sMode == 6 && !(options_screen_mode_get() & 1)) {
            boxHeight = 16.0f;
        } else if (sMode == 7 || sMode == 9) {
            boxHeight = 26.0f;
        } else {
            boxHeight = sBoxHeights[sMode];
        }
        for (i = 0; i < updateRate; i++) {
            sUnk3C += (boxHeight - sUnk3C) * 0.25f;
        }
        func_80054EA0(&width, &height);
        width >>= 1;
        height >>= 1;
        if (sUnk60 == 0.0f) {
            pressA = D_800FF3C8[0] & 0x9000;
            pressB = D_800FF3C8[0] & 0x4000;
            cDown = D_800FF3B8[0] & 8;
            cUp = D_800FF3B8[0] & 4;
            up = D_800FF3DC[0] > 0;
            down = D_800FF3DC[0] < 0;
            right = D_800FF3D8[0] > 0;
            left = D_800FF3D8[0] < 0;
        } else {
            pressA = 0;
            pressB = 0;
            cDown = 0;
            cUp = 0;
            up = 0;
            down = 0;
            right = 0;
            left = 0;
        }
        func_8004DFC4(&D_800FF398, &D_800FF39C, 0);
        func_80055B80(&D_800FF398);
        gDPSetFogColor(D_800FF398++, 255, 255, 255, 255);
        gSPFogFactor(D_800FF398++, 0x6400, 0x9867);
        gDPPipeSync(D_800FF398++);
        func_m038_00000();
        set_active_camera(0);
        cam_fov_scale_set(1.0f, 1.0f);
        func_8003F7F4(52.0f, 1);
        viewport_apply(&D_800FF398, 0);
        boxLeft = width - sUnk38;
        boxTop = height - (s32) sUnk3C;
        boxRight = sUnk38 + width;
        boxBottom = (s32) sUnk3C + height;
        obj = D_800FF3F8[125];
        inst = *(M037Instance **) obj->modelInstances;
        model = inst->model;
        inst->unkA = updateRate;
        func_8001138C(obj, sUnk5C, updateRate);
        if (options_screen_mode_get() & 1) {
            sPartAngles[6] = 1;
            sPartAngles[3] = 2;
        } else {
            sPartAngles[6] = 0;
            sPartAngles[3] = sScreenAngles[func_8005AA7C()];
        }
        sPartAngles[0] = sSoundAngles[func_8005A98C()];
        for (i = 0; i < 8; i++) {
            part = inst->parts;
            out = inst->angles[inst->buffer];
            for (j = 0; j < model->partCount; j++, part++) {
                kind = part->kind & 0xFF;
                if (kind == sPartKinds[i]) {
                    part->angle = sPartAngles[i] << 8;
                }
                *out++ = (part->angle >> 8) * model->textures[kind].texture->scale;
                if (part->angle2 >= 0) {
                    *out++ = (part->angle2 >> 8) * model->textures[kind].texture->scale;
                }
            }
        }
        batch = model->batches;
        for (k = 0; k < model->batchCount; k++, batch++) {
            kind = batch->kind;
            if (kind == sItemBatches[sUnk40]) {
                alpha = 0xFF;
                bright = func_80049C34(sPhase) * 55.0f + 200.0f;
                sPhase += updateRate << 10;
            } else {
                bright = 0x60;
                if ((options_screen_mode_get() & 1) && sUnk40 == 3) {
                    alpha = 0x70;
                } else {
                    alpha = 0xFF;
                }
            }
            if (kind != 6 && kind != 1) {
                n = batch[1].firstVertex - batch->firstVertex;
                v = &inst->vertices[inst->buffer][batch->firstVertex];
                while (n--) {
                    v->r = bright;
                    v->g = bright;
                    v->b = bright;
                    v->a = alpha;
                    v++;
                }
            }
        }
        func_80040E64(&D_800FF398, &D_800FF39C);
        func_8005A0D0(41);
        func_8005A0D0(31);
        D_800FF780[40].posY = -sUnk3C;
        D_800FF780[39].posY = sUnk3C;
        func_8005A0D0(39);
        func_8005A0D0(40);
        func_80055C00(&D_800FF398, NULL, 0, 0);
        gDPSetEnvColor(D_800FF398++, 0, 0, 0, 0xA0);
        gDPSetPrimColor(D_800FF398++, 0, 0, 0x28, 0x28, 0x28, 0);
        gDma1p(D_800FF398++, 7, (u8 *) sBoxDL + 0x80000000, 16, 2);
        gDPFillRectangle(D_800FF398++, boxLeft, boxTop, boxRight, boxBottom);
        gDPSetScissor(D_800FF398++, G_SC_NON_INTERLACE, boxLeft, boxTop, boxRight, boxBottom);
        y = height - (s32) sUnk3C + 3;
        func_8006FC54(2);
        func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0xFF);
        gDPPipeSync(D_800FF398++);
        func_8006FCA8(&D_800FF398, width, y, MENU_TEXT(308 + sUnk40), 4);
        y += 15;
        switch (sMode) {
            case 8:
                if (pressA) {
                    sMode = sUnk40;
                    func_80001248(0x16D, 0);
                } else if (pressB) {
                    if (sNoCheats == 0 && sUnk58 >= 30) {
                        func_80001248(0x16D, 0);
                        load_first_unlocked_level();
                        mark_write_eeprom_settings();
                        sUnk60 = 1.0f;
                    } else if (sNoCheats == 1) {
                        func_m019_001B0(1);
                        mark_write_eeprom_settings();
                        sUnk60 = 1.0f;
                    }
                } else {
                    i = sUnk40;
                    if (down) {
                        if (sUnk40 == 6) {
                            sUnk40 = 1;
                        } else {
                            sUnk40 += 2;
                        }
                    }
                    if (up) {
                        if (sUnk40 == 1) {
                            sUnk40 = 6;
                        } else {
                            sUnk40 -= 2;
                        }
                    }
                    if (left) {
                        sUnk40--;
                    }
                    if (right) {
                        sUnk40++;
                    }
                    if (sUnk40 < 0) {
                        sUnk40 = 7;
                    }
                    if (sUnk40 >= 8) {
                        sUnk40 = 0;
                    }
                    if ((options_screen_mode_get() & 1) && sUnk40 == 3) {
                        if (down) {
                            sUnk40 = 5;
                        } else if (up) {
                            sUnk40 = 1;
                        } else if (right) {
                            sUnk40 = 4;
                        } else {
                            sUnk40 = 2;
                        }
                    }
                    if (i != sUnk40) {
                        func_80001248(0x16B, 0);
                    }
                }
                break;
            case 0:
                i = func_8005A98C();
                if (left) {
                    func_80001248(0x16C, 0);
                    i--;
                    if (i < 0) {
                        i = 3;
                    }
                }
                if (right) {
                    func_80001248(0x16C, 0);
                    i++;
                    if (i >= 4) {
                        i = 0;
                    }
                }
                func_8005A99C(i);
                func_8006FCA8(&D_800FF398, width, y, (char *) sSaved0[func_8005A98C()], 4);
                if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                break;
            case 1:
                if (up) {
                    if (sUnk50 > 0) {
                        func_80001248(0x16C, 0);
                        sUnk50--;
                    } else {
                        func_80001248(0x16E, 0);
                    }
                }
                if (down) {
                    if (sUnk50 < D_800FB10C - 1) {
                        func_80001248(0x16C, 0);
                        sUnk50++;
                    } else {
                        func_80001248(0x16E, 0);
                    }
                }
                if (sUnk50 < 0) {
                    sUnk50 = 0;
                }
                if (sUnk50 >= D_800FB10C) {
                    sUnk50 = D_800FB10C - 1;
                }
                if (left || right) {
                    func_80001248(0x16C, 0);
                    func_8005AAD0(sUnk50, (func_8005AAB8(sUnk50) ^ 1) & 1);
                }
                y += 4;
                for (i = 0; i < 4; i++) {
                    if (i == sUnk50) {
                        func_8006FC68(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                    }
                    if (i >= D_800FB10C) {
                        func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0x62);
                    }
                    func_8006FCA8(&D_800FF398, width - 32, y, sControllerLabels[i], 1);
                    func_8006FCA8(&D_800FF398, width - 32, y, (char *) sSaved1[func_8005AAB8(i)], 0);
                    func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0xFF);
                    y += 15;
                }
                if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                break;
            case 2:
                options_music_volume_set(func_m037_01ACC(options_music_volume_get(), 160, y));
                if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                break;
            case 3:
                if (left || right) {
                    func_80001248(0x16C, 0);
                    if (func_8005AA7C() != 0) {
                        func_8005AA94(0);
                    } else {
                        func_8005AA94(1);
                    }
                }
                func_8006FCA8(&D_800FF398, width, y, (char *) sSaved3[func_8005AA7C()], 4);
                if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                break;
            case 4:
                func_8005A9F4(func_m037_01ACC(func_8005A9E4(), 160, y));
                if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                break;
            case 5: {
                u8 masks[5] = { 0, 1, 2, 4, 0 };
                char *names[5] = { NULL, "Rainbow Blood", "Jet Force Kids", "Ants as Pants", NULL };

                kind = D_800FF318[0x7A];
                if (kind != 0) {
                    for (c = 1; masks[c] != 0; c++) {
                        if (c == sUnk54) {
                            if (up) {
                                for (i = c - 1; masks[i] != 0; i--) {
                                    if (masks[i] & kind) {
                                        sUnk54 = i;
                                        up = FALSE;
                                        func_80001248(0x16C, 0);
                                        break;
                                    }
                                }
                            } else if (down) {
                                for (i = c + 1; masks[i] != 0; i++) {
                                    if (masks[i] & kind) {
                                        sUnk54 = i;
                                        down = FALSE;
                                        func_80001248(0x16C, 0);
                                        break;
                                    }
                                }
                            } else if (left || right) {
                                if (sUnk54 == 1) {
                                    D_800A5080 = (D_800A5080 + 1) & 1;
                                }
                                if (sUnk54 == 2) {
                                    D_800A5084 = (D_800A5084 + 1) & 1;
                                }
                                if (sUnk54 == 3) {
                                    D_800A5088 = (D_800A5088 + 1) & 1;
                                }
                                func_80001248(0x16C, 0);
                            }
                            func_8006FC68(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                        } else {
                            func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0xFF);
                        }
                        i = 0x13E;
                        if (masks[c] & kind) {
                            if ((c == 1 && D_800A5080 != 0) || (c == 2 && D_800A5084 != 0) ||
                                (c == 3 && D_800A5088 != 0)) {
                                i = 0x13D;
                            }
                            func_8006FCA8(&D_800FF398, width - 50, y, names[c], 0);
                            func_8006FCA8(&D_800FF398, width + 50, y, MENU_TEXT(i), 1);
                            y += 15;
                        }
                    }
                }
                if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                break;
            }
            case 6:
                i = options_screen_mode_get();
                if (left || right) {
                    i ^= 1;
                    func_80055228();
                    D_800FECD0 = 1;
                    func_8005483C(i);
                    func_80001248(0x16C, 0);
                }
                func_8005A834(i);
                i = options_screen_offset_get();
                if (cDown) {
                    i--;
                }
                if (cUp) {
                    i++;
                }
                options_screen_offset_set(i);
                func_8006FCA8(&D_800FF398, width, y, (char *) sSaved2[options_screen_mode_get() & 3], 4);
                y += 15;
                func_8006FC68(0xFF, 0xFF, 0, 0xFF, 0xFF);
                func_8006FCA8(&D_800FF398, width - 45, y, sButtonL, 4);
                func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0xFF);
                func_8006FCA8(&D_800FF398, width, y, MENU_TEXT(306), 4);
                y += 15;
                func_8006FC68(0xFF, 0xFF, 0, 0xFF, 0xFF);
                func_8006FCA8(&D_800FF398, width - 45, y, sButtonR, 4);
                func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0xFF);
                func_8006FCA8(&D_800FF398, width, y, MENU_TEXT(307), 4);
                if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                break;
            case 7:
                if (pressA) {
                    func_80001248(0x16D, 0);
                    if (sUnk30 == 0) {
                        sMode = 8;
                    } else if (sNoCheats == 1) {
                        sUnk34 = 0;
                        sMode = 9;
                    } else {
                        sMode = 10;
                        load_first_unlocked_level();
                    }
                } else if (pressB) {
                    sUnk30 = 0;
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                if (left || right) {
                    sUnk30 ^= 1;
                    func_80001248(0x16C, 0);
                }
                sGlow += updateRate * sUnk3A;
                if (sGlow >= 0x100) {
                    sGlow = 0x1FE - sGlow;
                    sUnk3A = -sUnk3A;
                } else if (sGlow < 0) {
                    sGlow = -sGlow;
                    sUnk3A = -sUnk3A;
                }
                func_8006FCA8(&D_800FF398, width, y, MENU_TEXT(316), 4);
                y += 15;
                if (sUnk30 == 0) {
                    func_8006FC68(0, sGlow, 0xFF, 0xFF, 0xFF);
                }
                func_8006FCA8(&D_800FF398, width + 25, y, MENU_TEXT(318), 4);
                if (sUnk30 == 1) {
                    func_8006FC68(0, sGlow, 0xFF, 0xFF, 0xFF);
                } else {
                    func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                func_8006FCA8(&D_800FF398, width - 25, y, MENU_TEXT(317), 4);
                break;
            case 9:
                if (pressA) {
                    func_80001248(0x16D, 0);
                    if (sUnk34 == 0) {
                        func_80001248(0x16D, 0);
                        func_80001248(0x16D, 0);
                        mark_write_eeprom_settings();
                        func_8004842C(get_settings()[3]);
                    }
                    func_m019_00324(0, 0, 0, 3);
                    sMode = 10;
                } else if (pressB) {
                    func_80001248(0x16D, 0);
                    sMode = 8;
                }
                if (left || right) {
                    sUnk34 ^= 1;
                    func_80001248(0x16C, 0);
                }
                sGlow += updateRate * sUnk3A;
                if (sGlow >= 0x100) {
                    sGlow = 0x1FE - sGlow;
                    sUnk3A = -sUnk3A;
                } else if (sGlow < 0) {
                    sGlow = -sGlow;
                    sUnk3A = -sUnk3A;
                }
                func_8006FCA8(&D_800FF398, width, y, MENU_TEXT(529), 4);
                y += 15;
                if (sUnk34 == 1) {
                    func_8006FC68(0, sGlow, 0xFF, 0xFF, 0xFF);
                }
                func_8006FCA8(&D_800FF398, width + 25, y, MENU_TEXT(318), 4);
                if (sUnk34 == 0) {
                    func_8006FC68(0, sGlow, 0xFF, 0xFF, 0xFF);
                } else {
                    func_8006FC68(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                func_8006FCA8(&D_800FF398, width - 25, y, MENU_TEXT(317), 4);
                break;
        }
        func_800408C0(&D_800FF398);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m037/ro", func_m037_001A0);
#endif

/* A colour cycle (55130.c's) */
typedef struct M037ColourCycle {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4];
    /* 0x0C */ void *source;
} M037ColourCycle;

extern s8 D_800FF3E0; /* the stick's left/right */
extern void func_m006_010D8(s32 *x, s32 *y);
extern void func_m006_0107C(s32 *x, s32 *y);
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void colour_cycle_init(M037ColourCycle *cycle, s32 assetID);
extern void update_colour_cycle(M037ColourCycle *cycle, s32 updateRate);

#define sStepColours D_m037_01F40

/*
 * Draws a volume bar for `value` (0..256, moved by 8 with the stick): a strip coloured along colour
 * cycle 0x39, then up to three steps in sStepColours; returns the new value
 */
s32 func_m037_01ACC(s32 value, s32 arg1, s32 top) {
    s32 x;
    s32 y;
    s32 i;
    s32 n;
    s32 y2;
    s32 m;
    M037ColourCycle cycle;

    y = top;
    x = 160;
    if (D_800FF3E0 < 0) {
        value -= 8;
        if (value < 0) {
            value = 0;
        }
    }
    if (D_800FF3E0 > 0) {
        value += 8;
        if (value > 256) {
            value = 256;
        }
    }
    func_m006_010D8(&x, &y);
    D_800FF780[42].posY = y - 23;
    func_80040D64(&D_800FF398, &D_800FF39C);
    func_8005A0D0(42);
    n = (f32) value * 38.0f * (1.0f / 256.0f);
    func_80055C00(&D_800FF398, NULL, 0, 0);
    gDma1p(D_800FF398++, 7, (u8 *) sBarDL + 0x80000000, 16, 2);
    x -= 18;
    y -= 25;
    func_m006_0107C(&x, &y);
    for (i = 0; i < n; i += 3) {
        colour_cycle_init(&cycle, 0x39);
        update_colour_cycle(&cycle, func_80005064(60) * i / 38);
        gDPSetColor(D_800FF398++, G_SETPRIMCOLOR,
                    _SHIFTL(cycle.colour[1], 16, 8) | _SHIFTL(cycle.colour[2], 8, 8) | _SHIFTL(cycle.colour[0], 24, 8) |
                        _SHIFTL(0xFF, 0, 8));
        gDPFillRectangle(D_800FF398++, x, y - 8, x + 1, y - 3);
        x += 3;
    }
    x = 178;
    y += 16;
    m = (f32) value * 21.0f * (1.0f / 256.0f);
    for (i = 0; i < 3 && m > 0; i++, m -= 7) {
        gDPSetColor(D_800FF398++, G_SETPRIMCOLOR, sStepColours[i]);
        if (m >= 8) {
            y2 = y - 7;
        } else {
            y2 = y - m;
        }
        gDPFillRectangle(D_800FF398++, x + 1, y2, x + 2, y);
        x += 4;
    }
    return value;
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m037", data);

/* (.rodata: C, and the assembly functions' own from ro/) */
#endif
