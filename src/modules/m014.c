/*
 * Runlink module 14: part of the HUD (still mostly assembly).
 */
#include "module.h"
#include "gfx.h"

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

/* The part of the player's state that 00074 reads */
typedef struct M014Player {
    /* 0x000 */ s8 unk0; /* (with unk1: picks the weapons, func_80047C1C) */
    /* 0x001 */ s8 unk1;
    /* 0x002 */ u8 pad2[0xFC - 0x2];
    /* 0x0FC */ s32 unkFC;
    /* 0x100 */ u8 pad100[0x10D - 0x100];
    /* 0x10D */ u8 unk10D;
    /* 0x10E */ u8 pad10E[0x1CA - 0x10E];
    /* 0x1CA */ s16 yaw;
    /* 0x1CC */ u8 pad1CC[0x1F9 - 0x1CC];
    /* 0x1F9 */ u8 unk1F9;
    /* 0x1FA */ u8 pad1FA[0x50C - 0x1FA];
    /* 0x50C */ Object *unk50C;
    /* 0x510 */ u8 pad510[0x53E - 0x510];
    /* 0x53E */ s16 unk53E; /* the ammo of weapons 3, 4 and 6 */
    /* 0x540 */ u16 unk540;
    /* 0x542 */ u8 unk542; /* the level of weapon 0 */
    /* 0x543 */ u8 pad543[0x568 - 0x543];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x58C - 0x569];
    /* 0x58C */ s16 unk58C; /* (out of 3600) */
    /* 0x58E */ u8 pad58E[0x592 - 0x58E];
    /* 0x592 */ s16 unk592; /* a timer */
} M014Player;

extern Gfx *D_800FF398;
extern void func_80055B80(Gfx **dl);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 arg3); /* sets a texture */
extern s32 rl_m014_04528;     /* .data (still assembly): the arrow's blink timer */
extern u8 rl_m014_044D8[];    /* its triangle */
extern u8 rl_m014_044E8[];    /* its vertices, pointing left */
extern u8 rl_m014_04508[];    /* and right */

/* Blinks an arrow to the left or right when `angle` is more than 0xA00 off the player's yaw (called from
   00B30) */
static void func_m014_00074(s16 *angle, M014Player *p, s32 updateRate) {
    rl_m014_04528 += updateRate;
    if ((p->unk540 & 0xF) && (rl_m014_04528 & 0x10)) {
        s16 d = *angle - p->yaw;

        if (d < -0xA00 || d > 0xA00) {
            func_80055B80(&D_800FF398);
            func_80055C00(&D_800FF398, NULL, 7, 0);
            gDPSetEnvColor(D_800FF398++, 0xFF, 0xFF, 0xFF, 0x00);
            gDPSetPrimColor(D_800FF398++, 0, 0, 0xFF, 0xFF, 0xFF, 0x80);
            if (d < -0x800) {
                gSPVertexJFG(D_800FF398++, rl_m014_044E8 + 0x80000000, 4, 0);
            } else {
                gSPVertexJFG(D_800FF398++, rl_m014_04508 + 0x80000000, 4, 0);
            }
            gSPPolygon(D_800FF398++, rl_m014_044D8 + 0x80000000, 1, 0);
            gDPPipeSync(D_800FF398++);
            gDPSetPrimColor(D_800FF398++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }
}

/* What func_80047C1C returns: the player's weapons */
typedef struct M014Weapons {
    /* 0x00 */ u8 pad0[0x2];
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ u16 owned; /* a bit per weapon */
    /* 0x0C */ u8 padC[0x14 - 0xC];
    /* 0x14 */ u16 unk14[16]; /* per weapon: the first number shown */
    /* 0x34 */ u16 unk34[16]; /* and the second */
    /* 0x54 */ u8 pad54[0x70 - 0x54];
    /* 0x70 */ u8 weapon; /* the weapon in hand */
} M014Weapons;

/* A menu item (D_800FF780, size 0x20; the main segment's MenuItem), as used here */
typedef struct M014MenuItem {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 image;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 posX;
    /* 0x10 */ f32 posY;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
} M014MenuItem;

extern M014MenuItem D_800FF780[];
extern Object **func_8000E464(s32 *count); /* the players */
extern M014Weapons *func_80047C1C(s32 a, s32 b);

INCLUDE_ASM("asm/nonmatchings/modules/m014/ro", func_m014_00220);

extern s32 is_game_paused(void);    /* is the game paused */
extern u8 D_800A321C;
extern Object *D_800A32B8;          /* the cabinet whose screen is open */
extern char **gMenuText;            /* the game's texts */
extern Mtx *D_800FF39C;
extern u8 rl_m014_03E70[]; /* .data (still assembly): the panel's vertices and triangles */
extern void func_80040D64(Gfx **dl, Mtx **mtx); /* mtx_ortho */
extern void func_80042134(MtxF *mtx);                      /* identity */
extern void func_800498E8(f32 x, f32 y, f32 z, MtxF *mtx); /* translation */
extern void func_80048D84(MtxF *mtx, Mtx *out);            /* to a fixed-point Mtx */
extern void func_80056000(s32 arg0);
extern void func_8006FC54(s32 font);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern void screen_line_add_raw(s32 x, s32 y, s32 number, s32 digits, s32 flag); /* screen_line_add_raw */
extern void func_m013_00008(Object *obj, M014Player *p, s32 updateRate);
extern void func_m006_00000(Object *obj, s32 health, s32 arg2, s32 updateRate);
extern void func_m006_012D4(M014Player *p, s32 updateRate);
extern void func_m006_0107C(s32 *x, s32 *y);
extern void rl_m014_00220(Object *obj, s32 arg1);
static void func_m014_010F4(s32 updateRate);
static s32 func_m014_02CF4(s32 updateRate);
static void func_m014_0292C(s32 updateRate);
static void func_m014_019F4(s32 updateRate);
static void func_m014_03C68(s32 value, s32 max, s32 x1, s32 y1, s32 x2, s32 y2, s32 alpha);

/* The behaviour data of what the player holds, as 00B30 reads it */
typedef struct M014Held {
    /* 0x00 */ u8 pad0[0x94];
    /* 0x94 */ u32 unk94; /* bit 31 */
} M014Held;

/*
 * The one-player HUD: the weapon panel, the arrow, the health and a timer bar that fades in while
 * unk592 runs (or 02CF4's screen while rl_m014_04896 is set)
 */
void func_m014_00B30(Object *obj, s32 updateRate) {
    M014Player *p;
    UNUSED s32 pad;
    MtxF mtx;
    s32 x;
    s32 y;

    if (rl_m014_04896 != 0) {
        func_m014_02CF4(updateRate);
    } else if (obj != NULL) {
        if (!is_game_paused()) {
            if (D_800FF310 == 0 && D_800A321C == 0 && D_800A32B8 == NULL) {
                p = obj->unk68;
                if (p->unk568 == 5 || p->unk568 == 11 || p->unkFC != 0) {
                    func_m013_00008(obj, p, updateRate);
                }
                func_80040D64(&D_800FF398, &D_800FF39C);
                if (p->unk50C != NULL &&
                    (p->unk10D == 1 || (((M014Held *) p->unk50C->unk68)->unk94 >> 31))) {
                    rl_m014_00220(p->unk50C, p->unk10D & 1);
                }
                if (p->unk10D == 0 && p->unk1F9 == 0) {
                    if (p->unk568 != 5) {
                        func_m014_00074((s16 *) obj, p, updateRate);
                    }
                    func_m014_010F4(updateRate);
                    func_m014_0292C(updateRate);
                    if (p->unk592 > 0) {
                        p->unk592 -= updateRate;
                        rl_m014_04898 += updateRate * 2;
                        if (rl_m014_04898 >= 256) {
                            rl_m014_04898 = 255;
                        }
                    }
                    if (rl_m014_04898 > 0) {
                        if (p->unk592 <= 0) {
                            rl_m014_04898 -= updateRate * 2;
                            if (rl_m014_04898 < 0) {
                                rl_m014_04898 = 0;
                            }
                        }
                        func_80055B80(&D_800FF398);
                        func_80042134(&mtx);
                        func_800498E8(-50.0f, -80.0f, 0.0f, &mtx);
                        func_80048D84(&mtx, D_800FF39C);
                        {
                            Gfx *g = D_800FF398++;

                            g->words.w0 = 0x01000040;
                            g->words.w1 = (u32) D_800FF39C + 0x80000000;
                            D_800FF39C = (Mtx *) ((u8 *) D_800FF39C + 0x40);
                        }
                        func_80055C00(&D_800FF398, D_800FF3F8[26], 0, 0);
                        gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, rl_m014_04898);
                        gDPSetEnvColor(D_800FF398++, 0, 0, 0, 255);
                        gSPVertexJFG(D_800FF398++, rl_m014_03E70 + 0x80000340, 25, 0);
                        gSPPolygon(D_800FF398++, rl_m014_03E70 + 0x80000430, 10, 1);
                        func_80055C00(&D_800FF398, D_800FF3F8[27], 0, 0);
                        gSPPolygon(D_800FF398++, rl_m014_03E70 + 0x800004D0, 12, 1);
                        func_80055B80(&D_800FF398);
                        func_80055C00(&D_800FF398, NULL, 4, 0);
                        gDPSetPrimColor(D_800FF398++, 0, 0, 0, 0x28, 0x32, (rl_m014_04898 * 0x60) >> 8);
                        gSPPolygon(D_800FF398++, rl_m014_03E70 + 0x80000590, 4, 1);
                        gDPPipeSync(D_800FF398++);
                        func_80055B80(&D_800FF398);
                        func_m014_03C68(p->unk58C, 3600, -45, -86, 66, -92, rl_m014_04898);
                        func_80040D64(&D_800FF398, &D_800FF39C);
                        func_8006FC54(2);
                        func_8006FC68(0, 255, 255, 255, rl_m014_04898);
                        x = -50;
                        y = -80;
                        func_m006_0107C(&x, &y);
                        x += 5;
                        y -= 10;
                        func_8006FCA8(&D_800FF398, x, y, gMenuText[457], 0);
                        screen_line_add_raw(x + 128, y + 9, (s32) ((p->unk58C * 100.0f) / 3600.0f), 3,
                                      (s32) ((p->unk58C * 100.0f) / 3600.0f) < 10);
                    }
                    func_80056000(0);
                    func_m006_00000(obj, obj->interactObj->hitPoints >> 8,
                                    func_80047C1C(p->unk0, p->unk1)->unk2, updateRate);
                    func_80056000(1);
                    func_m006_012D4(p, updateRate);
                    func_m014_019F4(updateRate);
                }
            }
        }
    }
}

static void func_m014_010D8(void) {
    rl_m014_0489C = 0;
    rl_m014_04890 = 0;
    D_800FF310 = 0;
}

/* The player's flags at 0x540, as 010F4 reads them */
typedef struct M014Flags {
    /* 31 */ u32 pad : 2;
    /* 29 */ u32 turn : 2; /* 1: left, 2: right */
    /*  0 */ u32 rest : 28;
} M014Flags;

/* Swings the weapon wheel (item 0) left or right with a sound, slides item 4 out while its timer (the word
   at its 0x1C) runs and back after, and keeps the weapon in hand (called from 00B30) */
static void func_m014_010F4(s32 updateRate) {
    s32 count;
    M014MenuItem *wheel;
    M014MenuItem *item;
    M014Player *p;
    Object **players;
    M014Weapons *w;
    s32 *sound; /* (a pointer set before the calls: IDO then reloads the address at each use, as the ROM does) */
    UNUSED s32 pad1;

    wheel = &D_800FF780[0];
    item = &D_800FF780[4];
    sound = &rl_m014_04890;
    players = func_8000E464(&count);
    if (count > 0) {
        p = (*players)->unk68;
        w = func_80047C1C(p->unk0, p->unk1);
        if (((M014Flags *) &p->unk540)->turn & 1) {
            *(s16 *) &item->unk1C = func_80005064(240);
            wheel->unk1C -= 27.0f;
            if (rl_m014_04890 != 0) {
                sound_stop(rl_m014_04890);
            }
            func_80001248(0x8B, sound);
        }
        if (((M014Flags *) &p->unk540)->turn & 2) {
            *(s16 *) &item->unk1C = func_80005064(240);
            wheel->unk1C += 27.0f;
            if (rl_m014_04890 != 0) {
                sound_stop(rl_m014_04890);
            }
            func_80001248(0x8B, sound);
        }
        wheel->unk1C *= 0.875f;
        if (*(s16 *) &item->unk1C != 0) {
            if (item->posY > -30.0f) {
                item->posY -= updateRate * 4;
            }
            if (item->posY < -30.0f) {
                item->posY = -30.0f;
            }
            *(s16 *) &item->unk1C -= updateRate;
            if (*(s16 *) &item->unk1C < 0) {
                *(s16 *) &item->unk1C = 0;
            }
        } else if (item->posY < 46.0f) {
            item->posY += updateRate * 4;
            if (item->posY > 46.0f) {
                item->posY = 46.0f;
            }
        }
        rl_m014_0489C = w->weapon;
    }
}

extern void func_80042134(MtxF *mtx);                      /* identity */
extern void func_80049874(f32 x, f32 y, f32 z, MtxF *mtx); /* scale */
extern void func_800498E8(f32 x, f32 y, f32 z, MtxF *mtx); /* translation */
extern void func_80048D84(MtxF *mtx, Mtx *out);            /* to a fixed-point Mtx */
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_800408C0(Gfx **dl); /* sets the viewport's scissor */
extern void menu_item_load(s32 image);
extern void func_80056000(s32 arg0);
extern void func_8005A0D0(s32 item); /* draws a menu item */
extern Mtx *D_800FF39C;
extern s32 D_800A51CC; /* the text opacity */
extern u8 rl_m014_03E70[]; /* .data (still assembly): the weapon wheel's display list, vertices and triangles */

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

/* Slides menu item 6 in while 019E4's timer runs, and out again after */
static void func_m014_019F4(s32 updateRate) {
    if (rl_m014_04894 != 0) {
        if (D_800FF780[6].posX < 69.0f) {
            D_800FF780[6].posX += updateRate * 8;
            if (D_800FF780[6].posX > 69.0f) {
                D_800FF780[6].posX = 69.0f;
            }
        }
        rl_m014_04894 -= updateRate;
        if (rl_m014_04894 < 0) {
            rl_m014_04894 = 0;
        }
    } else if (D_800FF780[6].posX > -91.0f) {
        D_800FF780[6].posX -= updateRate * 8;
        if (D_800FF780[6].posX < -91.0f) {
            D_800FF780[6].posX = -91.0f;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m014/ro", func_m014_01AD8);

/* A filled box (59060.c's MenuBox) */
typedef struct M014Box {
    s16 x1, y1, x2, y2;
    s32 colour;
} M014Box;

extern s32 func_800373E4(M014Weapons *w, s32 weapon); /* the weapon's ammo capacity */
extern void menu_fill_boxes(Gfx **dl, s32 count, M014Box *boxes, s32 arg3);
extern void menu_digits_draw(s32 number, s32 x, s32 y, s32 font, s32 digits, s32 arg5, s32 arg6); /* draws a number */
extern void rl_m014_01AD8(M014Player *p, s32 updateRate, s32 image);
#define rl_m014_04810 (*(s32 *) (rl_m014_03E70 + 0x9A0)) /* .data (still assembly): the weapon last shown */
extern M014Box rl_m014_0447C[7]; /* the ammo bar's boxes */

/*
 * The weapon panel: the weapon's image (loaded when it changes), the ammo bar (six boxes from red to
 * green as the ammo fills), item 7's fade and the weapon's two numbers (called from 00B30)
 */
static void func_m014_0292C(s32 updateRate) {
    s32 count;
    s32 shown;
    s32 ammo;
    s32 alpha;
    s32 red;
    s32 green;
    s32 image; /* then the ammo bar's level */
    M014Box *b;
    M014MenuItem *item;
    M014Weapons *weapons;
    M014Player *p;
    M014Weapons *w;

    p = (*func_8000E464(&count))->unk68;
    w = func_80047C1C(p->unk0, p->unk1);
    shown = rl_m014_04810;
    weapons = w; /* (a copy: the ROM spills the pointer after reading the old weapon) */
    image = weapons->weapon + 10;
    if (shown != weapons->weapon) {
        shown = weapons->weapon;
        rl_m014_04810 = weapons->weapon;
        if (shown != -5) { /* (always true: weapon is a u8) */
            menu_item_free(image);
        }
    }
    D_800FF780[1].image = image;
    rl_m014_01AD8(p, updateRate, image);
    if (D_800FF3F8[image] == NULL) {
        menu_item_load(image);
    }
    func_80056000(0);
    func_8005A0D0(1);
    if (weapons->weapon == 0) {
        image = p->unk542;
        alpha = 0xFF;
    } else if (weapons->weapon == 3 || weapons->weapon == 6 || weapons->weapon == 4) {
        ammo = func_800373E4(weapons, weapons->weapon);
        image = p->unk53E;
        if (weapons->weapon == 3) {
            ammo -= 32;
            image -= 32;
        }
        image = 252 - image * 252 / ammo;
        alpha = 0xFF;
    } else {
        image = 0;
        alpha = 0x55;
    }
    b = &rl_m014_0447C[1];
    ammo = 5; /* (now the box counter) */
    do {
        if (image >= 42) {
            red = 0xFF;
            green = 0;
            image -= 42;
        } else {
            red = image * 6;
            green = 0xFF - red;
            image = 0;
        }
        b->colour = (red << 24) | (green << 16) | alpha;
        b++;
    } while (ammo--);
    menu_fill_boxes(&D_800FF398, 7, rl_m014_0447C, 0);
    item = &D_800FF780[7];
    if (D_800FF780[4].posY < 46.0f) {
        item->unk18 -= item->unk18 * 0.125f;
    } else {
        item->unk18 += (1.0f - item->unk18) * 0.125f;
    }
    func_8005A0D0(7);
    func_80056000(1);
    menu_digits_draw(weapons->unk14[weapons->weapon], D_800FF780[9].posX, D_800FF780[9].posY, 8, 3, 0xFFFF00A0, 0xFFFF0040);
    menu_digits_draw(weapons->unk34[weapons->weapon], D_800FF780[10].posX, D_800FF780[10].posY, 9, 3, -0x60, -0xC0);
}

void func_m014_02CC0(void) {
    menu_assetgroup_free(rl_m014_04440);
    free_images();
    func_m006_00EFC();
}

/* A level's best result (size 8) */
typedef struct M014Record {
    /* 0x0 */ u8 score;
    /* 0x1 */ u8 unk1;
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u32 time;
} M014Record;

/* The saved game, as far as used here */
typedef struct M014Save {
    /* 0x000 */ u8 pad0[3];
    /* 0x003 */ s8 unk3;
    /* 0x004 */ u8 score;
    /* 0x005 */ u8 unk5;
    /* 0x006 */ u8 unk6;
    /* 0x007 */ u8 pad7[0x334 - 0x7];
    /* 0x334 */ M014Record records[32];
    /* 0x434 */ u8 pad434[0x474 - 0x434];
    /* 0x474 */ u32 unk474[16];
    /* 0x4B4 */ u32 unk4B4[16];
    /* 0x4F4 */ u16 unk4F4[16];
    /* 0x514 */ u32 time;
} M014Save;

extern M014Save *get_settings(void);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void func_m006_010D8(s32 *x, s32 *y);
extern void func_8005A0D0(s32 item); /* draws a menu item */
static void func_m014_03418(s32 index, s32 weapon, s32 x, s32 y);
extern u8 rl_m014_04814[]; /* .data (still assembly) */
extern s32 rl_m014_04820;  /* the screen's timer */
extern s32 rl_m014_04824;  /* the blink timer */
extern s32 rl_m014_048A0;  /* .bss (still assembly) */
extern s32 rl_m014_048A4;
extern s32 rl_m014_048A8;
extern s32 rl_m014_048AC;
extern s32 rl_m014_048B0;

/*
 * The end-of-level results screen: each line appears (with a sound) as the timer passes it: the stats, the
 * three best stats and the time, with the new records blinking
 */
static s32 func_m014_02CF4(s32 updateRate) {
    s32 old;
    char buf[80];
    s32 x;
    s32 y;
    M014Save *save;
    M014MenuItem *item;

    save = get_settings();
    old = rl_m014_04820;
    {
        if (rl_m014_04820 < 300) {
            rl_m014_04820 += updateRate;
        }
        if (old == 0 || (old < 30 && rl_m014_04820 >= 30) ||
            (old < 60 && rl_m014_04820 >= 60 && rl_m014_048A0 != -1) ||
            (old < 90 && rl_m014_04820 >= 90 && rl_m014_048A4 != -1) ||
            (old < 120 && rl_m014_04820 >= 120 && rl_m014_048A8 != -1) || (old < 150 && rl_m014_04820 >= 150)) {
            func_80001248(0x190, 0);
        }
    }
    menu_fill_boxes(&D_800FF398, 1, (M014Box *) rl_m014_04814, 0);
    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    func_8006FC68(255, 255, 0, 255, 255);
    func_8006FCA8(&D_800FF398, 160, 33, gMenuText[509], 12);
    func_80040D64(&D_800FF398, &D_800FF39C);
    func_80056000(0);
    rl_m014_04824 -= updateRate;
    if (rl_m014_04824 < 0) {
        rl_m014_04824 += 90;
    }
    if (rl_m014_04820 >= 30) {
        func_8006FCA8(&D_800FF398, 25, 50, gMenuText[387], 0);
        func_8006FC68(255, 255, 255, 255, 255);
        func_8006FCA8(&D_800FF398, 85, 70, gMenuText[505], 0);
        func_8006FCA8(&D_800FF398, 85, 90, gMenuText[506], 0);
        func_8006FCA8(&D_800FF398, 85, 110, gMenuText[507], 0);
        func_800639E8(buf, "%d", save->score);
        func_8006FCA8(&D_800FF398, 78, 70, buf, 1);
        func_800639E8(buf, "%d", save->unk5);
        func_8006FCA8(&D_800FF398, 78, 90, buf, 1);
        func_800639E8(buf, "%d", save->unk6);
        func_8006FCA8(&D_800FF398, 78, 110, buf, 1);
        if (rl_m014_048AC != 0 && rl_m014_04824 >= 31) {
            func_8006FC68(255, 255, 0, 255, 255);
            func_8006FCA8(&D_800FF398, 78, 80, gMenuText[508], 0);
        }
        item = &D_800FF780[13];
        x = 45;
        y = 72;
        func_m006_010D8(&x, &y);
        item->posX = x;
        item->posY = y;
        item->unk18 = 1.0f;
        func_8005A0D0(13);
        item->posY -= 20.0f;
        item->unk18 = 2.0f;
        func_8005A0D0(13);
        item->posY -= 20.0f;
        item->unk18 = 0.0f;
        func_8005A0D0(13);
    }
    if (rl_m014_04820 >= 60 && rl_m014_048A0 != -1) {
        func_m014_03418(0, rl_m014_048A0, 140, 50);
    }
    if (rl_m014_04820 >= 90 && rl_m014_048A4 != -1) {
        func_m014_03418(1, rl_m014_048A4, 140, 110);
    }
    if (rl_m014_04820 >= 120 && rl_m014_048A8 != -1) {
        func_m014_03418(2, rl_m014_048A8, 140, 170);
    }
    if (rl_m014_04820 >= 150) {
        f32 t;
        s32 h;
        s32 m;
        s32 s;

        func_8006FC68(255, 255, 0, 255, 255);
        func_8006FCA8(&D_800FF398, 25, 130, gMenuText[510], 0);
        func_8006FC68(255, 255, 255, 255, 255);
        t = save->time / (f32) 60; /* ((f32) 60: the ROM loads 60.0f again for the minutes) */
        h = (s32) t / 3600;
        t -= h * 3600.0f;
        m = (s32) t / 60;
        t -= m * 60.0f;
        s = t;
        func_800639E8(buf, "%02d:%02d:%02d.%02d", h, m, s, (s32) ((t - s) * 100.0f));
        func_8006FCA8(&D_800FF398, 25, 150, buf, 0);
        if (rl_m014_048B0 != 0 && rl_m014_04824 >= 31) {
            func_8006FC68(255, 255, 0, 255, 255);
            func_8006FCA8(&D_800FF398, 25, 140, gMenuText[508], 0);
        }
    }
    func_80056000(1);
    return 1;
}

extern void menu_item_load(s32 image); /* loads a menu image */
extern s32 D_800A51CC;                /* the text opacity */

/*
 * One of the results screen's three best stats: its title, the stat (the most used weapon's count, its
 * accuracy or its kills) and the weapon's name and image (called from 02CF4)
 */
static void func_m014_03418(s32 index, s32 weapon, s32 x, s32 y) {
    f32 savedX;
    f32 savedY;
    f32 accuracy;
    s32 imageX;
    s32 imageY;
    char buf[80];
    M014MenuItem *item;
    M014Save *save;

    save = get_settings();
    func_8006FC68(255, 255, 0, 255, 255);
    func_8006FCA8(&D_800FF398, x, y, gMenuText[index + 459], 0);
    func_8006FC68(255, 255, 255, 255, 255);
    switch (index) {
        case 0:
            func_8006FCA8(&D_800FF398, x + 40, y + 30, gMenuText[334], 0);
            func_800639E8(buf, "%d", save->unk4F4[weapon]);
            func_8006FCA8(&D_800FF398, x + 140, y + 30, buf, 1);
            break;
        case 1:
            func_8006FCA8(&D_800FF398, x + 40, y + 30, gMenuText[178], 0);
            if (save->unk474[weapon] == 0) {
                accuracy = 0.0f;
            } else {
                accuracy = ((f32) save->unk4B4[weapon] / (f32) save->unk474[weapon]) * 100.0f;
            }
            func_800639E8(buf, "%.2f%%", &accuracy);
            func_8006FCA8(&D_800FF398, x + 140, y + 30, buf, 1);
            break;
        case 2:
            func_8006FCA8(&D_800FF398, x + 40, y + 30, gMenuText[462], 0);
            func_800639E8(buf, "%d", save->unk474[weapon]);
            func_8006FCA8(&D_800FF398, x + 140, y + 30, buf, 1);
            break;
    }
    func_8006FCA8(&D_800FF398, x + 40, y + 13, gMenuText[weapon + 57], 0);
    if (D_800FF3F8[weapon + 10] == NULL) {
        menu_item_load(weapon + 10);
    }
    D_800FF780[0].scale = 1.0f;
    D_800A51CC = 255;
    savedX = D_800FF780[0].posX;
    savedY = D_800FF780[0].posY;
    imageX = x + 18;
    imageY = y + 24;
    item = &D_800FF780[0];
    func_m006_010D8(&imageX, &imageY);
    item->posX = imageX;
    item->posY = imageY;
    item->image = weapon + 10;
    func_8005A0D0(0);
    item->posX = savedX;
    item->posY = savedY;
    D_800A51CC = 255;
    func_80056000(0);
}

/* What func_8004470C returns, as used here */
typedef struct M014Level {
    /* 0x000 */ u8 pad0[0x102];
    /* 0x102 */ u8 record;
} M014Level;

extern M014Save *get_settings(void);
extern M014Level *func_8004470C(void);
extern s32 game_flag_get(s32 flag);
extern void func_80047EFC(s32 flag, s32 value);
extern void func_80000510();
extern void func_8004842C(s32 arg0);
extern s8 *D_800FD7B8;

/* .bss, still assembly */
extern s16 rl_m014_04896;
extern s32 rl_m014_048A0;
extern s32 rl_m014_048A4;
extern s32 rl_m014_048A8;
extern s32 rl_m014_048AC;
extern s32 rl_m014_048B0;

/* At the end of a level: finds the best of the stats and updates the level's record */
void func_m014_037F4(void) {
    M014Level *level;
    M014Record *rec;
    s32 i;
    s32 changed;
    s32 maxA;
    u32 maxB;
    f32 best;
    f32 ratio;
    s32 sum;
    M014Save *save;

    save = get_settings();
    level = func_8004470C();
    rl_m014_04896 = 1;
    best = 0.0f;
    maxA = 0;
    maxB = 0;
    rl_m014_048A0 = -1;
    rl_m014_048A4 = -1;
    rl_m014_048A8 = -1;
    sum = 0;
    for (i = 0; i < 16; i++) {
        if (maxB < save->unk474[i]) {
            maxB = save->unk474[i];
            rl_m014_048A8 = i;
        }
        if (save->unk4B4[i] != 0) {
            ratio = (f32) save->unk4B4[i] / (f32) save->unk474[i];
            if (best < ratio) {
                best = ratio;
                rl_m014_048A4 = i;
            }
        }
        if (maxA < save->unk4F4[i]) {
            maxA = save->unk4F4[i];
            rl_m014_048A0 = i;
        }
        sum += save->unk4F4[i];
    }
    changed = 0;
    rec = &save->records[level->record];
    if (rec->unk2 < sum) {
        rec->unk2 = sum;
        changed = 1;
    }
    rl_m014_048AC = 0;
    rl_m014_048B0 = 0;
    if (rec->score < save->score) {
        s32 total;
        s32 target;

        rec->score = save->score;
        changed = 1;
        rec->unk1 = save->unk5;
        rec->time = save->time;
        rl_m014_048AC = 1;
        target = 0;
        total = 0;
        for (i = 0; i < 31; i++) {
            target += D_800FD7B8[i];
            total += save->records[i].score;
        }
        if (target == total && !game_flag_get(0x58)) {
            func_80047EFC(0x58, 1);
        }
    } else if (save->time < rec->time && save->score >= rec->score) {
        rec->time = save->time;
        rl_m014_048B0 = 1;
        changed = 1;
    }
    if (changed) {
        func_80000510(0x2A);
        func_8004842C(save->unk3);
    } else {
        func_80000510(0x2A);
    }
}

extern char **gMenuText; /* the game's texts */
extern s32 func_800706D4(char *text, s32 x, s32 font); /* DKR get_text_width */
extern s32 func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void func_m157_00000(Gfx **dl, u8 shape, s32 x, s32 y, s32 width, s32 height, u32 colour);
extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern char *get_world_name(s32 world);

/* Draws a box with the region's name, its world's name and a numbered line under it */
void func_m014_03AD4(Gfx **dl, s32 region, s32 world, s32 number, s32 alpha) {
    char buf[32];
    char *name;
    s32 width;
    s32 width2;

    name = gMenuText[region + 407];
    width = func_800706D4(name, 2, 1) + 40;
    func_800639E8(buf, "%d %s\n", number, gMenuText[458]);
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

/* A colour cycle (55130.c's) */
typedef struct M014ColourCycle {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4];
    /* 0x0C */ void *source;
} M014ColourCycle;

extern void func_m006_0107C(s32 *x, s32 *y);
extern void colour_cycle_init(M014ColourCycle *cycle, s32 assetID);
extern void update_colour_cycle(M014ColourCycle *cycle, s32 updateRate);

/*
 * A bar from (x1, y1) to (x2, y2) in 4-pixel steps, coloured along colour cycle 0x3B, bright up to
 * value / max of its length and dim after (called from 00B30)
 */
static void func_m014_03C68(s32 value, s32 max, s32 x1, s32 y1, s32 x2, s32 y2, s32 alpha) {
    M014Box *b;
    s32 x;
    s32 width;
    s32 split;
    s32 n;
    s32 left;
    s32 range;
    M014ColourCycle cycle;
    M014Box boxes[64];
    UNUSED s32 padB[3];

    n = 0;
    if (max != 0) {
        func_m006_0107C(&x1, &y1);
        func_m006_0107C(&x2, &y2);
        b = boxes;
        width = x2 - x1;
        split = (((x2 - x1) * ((value << 16) / max)) >> 16) + x1;
        /* (one line: IDO schedules by line) */
        range = width; for (x = x1 + 4; x < x2; x += 4) {
            left = x - 2;
            if (x < split) {
                max = 0xFF;
            } else {
                max = 0x30;
            }
            max = (max * alpha) >> 8;
            colour_cycle_init(&cycle, 0x3B);
            update_colour_cycle(&cycle, (func_80005064(60) * (left - x1)) / range);
            b->x1 = left;
            b->y1 = y1;
            b->x2 = x;
            b->y2 = y2;
            b->colour = ((cycle.colour[2] << 8) | ((cycle.colour[1] << 16) | (cycle.colour[0] << 24))) | max;
            b++;
            n++;
        }
        menu_fill_boxes(&D_800FF398, n, boxes, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m014", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m014", bss);

