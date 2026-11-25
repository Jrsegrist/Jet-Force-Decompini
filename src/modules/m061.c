/*
 * Runlink module 61: the deathmatch results and instruments of the front end
 * (the players ranked by kills and other scores).
 * Family: menus and screens (docs/runlink.md, "Module families").
 */
#include "module.h"
#include "gfx.h"

/* the .rodata strings the data refers to (first in .rodata) */
INCLUDE_ASM("asm/nonmatchings/modules/m061/ro", rodata);

/* A player's multiplayer scores (D_800FED50, 0xA0 each) */
typedef struct M061Stats {
    /* 0x00 */ u16 unk0[8]; /* indexed by unk8C */
    /* 0x10 */ s16 kills;
    /* 0x12 */ u8 pad12;
    /* 0x13 */ s8 unk13; /* still in the round (game type 0x30) */
    /* 0x14 */ u8 rank;
    /* 0x15 */ u8 unk15; /* out of the round */
    /* 0x16 */ u8 pad16[0x8C - 0x16];
    /* 0x8C */ u8 unk8C;
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ u32 unk90;
    /* 0x94 */ u8 pad94[0x9C - 0x94];
    /* 0x9C */ u16 unk9C;
    /* 0x9E */ u8 pad9E[2];
} M061Stats;

/* What the players' objects keep in their unk68 */
typedef struct M061Player {
    /* 0x000 */ s8 unk0; /* the player */
    /* 0x001 */ s8 unk1; /* the character */
    /* 0x002 */ u8 pad2[0x10E - 0x2];
    /* 0x10E */ s16 unk10E; /* (not 0: not shown on the radars) */
    /* 0x110 */ u8 pad110[0x540 - 0x110];
    /* 0x540 */ u32 unk540;
    /* 0x544 */ u8 pad544[0x568 - 0x544];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x5C8 - 0x569];
    /* 0x5C8 */ s16 unk5C8;
} M061Player;

extern u8 D_800A4FD0; /* the number of players */
extern s8 D_800A4FEC;
extern M061Stats D_800FED50[];
extern M061Stats *D_800FF1C0[]; /* the players, sorted */

extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */
extern void func_m006_00EFC(void);
extern void func_80055A00(s32 tex); /* frees a texture */
extern void music_volume_fade(f32 volume, u8 arg1); /* fades the music */

/*
 * The module's .data is still assembly: it is reached from its first symbol
 * (the ROM's relocations are section-relative).
 */
extern u8 rl_m061_02FA0[];
#define sAssets ((s16 *) &rl_m061_02FA0[0x4B0])
#define sTextures ((s32 *) &rl_m061_02FA0[0x588]) /* one every 0x18 bytes */

/* A vertex of the dial (10 bytes) */
typedef struct M061Vertex {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ u8 r;
    /* 0x7 */ u8 g;
    /* 0x8 */ u8 b;
    /* 0x9 */ u8 a;
} M061Vertex;

/* A triangle of the dial (0x10 bytes) */
typedef struct M061Triangle {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s16 uv[3][2];
} M061Triangle;

/* A dot on a player's radar (sDots[player] holds four) */
typedef struct M061Dot {
    /* 0x0 */ s8 x;
    /* 0x1 */ s8 y;
    /* 0x2 */ u8 alpha;  /* 0: not shown on its own radar */
    /* 0x3 */ u8 colour; /* an index into the colours at .data 0x618 */
} M061Dot;

/* A colour cycle (55130.c's) */
typedef struct M061ColourCycle {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4];
    /* 0x0C */ void *source;
} M061ColourCycle;

/* A menu item (D_800FF780, size 0x20; the main segment's MenuItem), as used here */
typedef struct M061MenuItem {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ u8 pad14[0x20 - 0x14];
} M061MenuItem;

extern M061MenuItem D_800FF780[];
extern void menu_assetgroup_load(s16 *assets); /* loads menu textures */
extern void menu_assetgroup_reset(s16 *assets);
extern void func_m006_00C74(void);
extern void func_m006_01134(void);
extern void colour_cycle_init(M061ColourCycle *cycle, s32 assetID);
extern s32 func_80055694(s32 id); /* loads a texture */

/* The module's .bss (still assembly) */
extern M061Vertex rl_m061_03600[17];
extern M061Vertex rl_m061_036B0[17];
extern M061ColourCycle rl_m061_03760[2];
extern f32 rl_m061_03780;
extern M061Dot rl_m061_03788[4][4];
extern s32 rl_m061_037C8;
extern s32 rl_m061_03528, rl_m061_03540, rl_m061_03558, rl_m061_03570; /* .data: = sTextures[0, 6, 12, 18] */
#define sOuter rl_m061_03600 /* the dial's outer ring */
#define sInner rl_m061_036B0 /* and inner ring */
#define sCycles rl_m061_03760
#define sBss180 rl_m061_03780
#define sDots rl_m061_03788 /* each player's radar dots */
#define sBss1C8 rl_m061_037C8
#define sTriangles ((M061Triangle *) &rl_m061_02FA0[0x250]) /* 16 */
#define sImages ((s16 *) &rl_m061_02FA0[0x500])

/* The module's init: the dial's vertices and triangles (16 sectors), the radar dots, cycles and sounds */
void func_m061_00000(void) {
    M061Vertex *outer;
    M061Vertex *inner;
    M061Triangle *t;
    s32 j;
    s32 angle;
    s32 i;
    f32 s;
    f32 c;

    menu_assetgroup_load(sAssets);
    menu_assetgroup_reset(sImages);
    D_800FF780[0].unk8 *= 0.75f;
    angle = 0;
    sOuter[0].x = 0;
    sOuter[0].y = 0;
    sOuter[0].z = 0;
    outer = &sOuter[1];
    sInner[0].x = 0;
    sInner[0].y = 0;
    sInner[0].z = 0;
    sInner[0].r = 0xFF;
    sInner[0].g = 0xFF;
    sInner[0].b = 0xFF;
    sInner[0].a = 0xFF;
    inner = &sInner[1];
    t = sTriangles;
    for (i = 1; i != 17; i++) {
        s = func_80049C34(angle);
        c = func_80049C30(angle);
        outer->z = 0;
        outer->x = 17.0f * s;
        outer->y = 17.0f * c;
        outer++;
        inner->z = 0;
        inner->r = 0xFF;
        inner->g = 0xFF;
        inner->b = 0xFF;
        inner->a = 0xFF;
        inner->x = 15.0f * s;
        inner->y = 15.0f * c;
        inner++;
        t->uv[0][0] = 0x220;
        t->uv[0][1] = 0x220;
        t->uv[1][0] = 512.0f * s + 544.0f;
        t->uv[1][1] = 512.0f * c + 544.0f;
        t->uv[2][0] = func_80049C34(angle + 0x1000) * 512.0f + 544.0f;
        t->uv[2][1] = func_80049C30(angle + 0x1000) * 512.0f + 544.0f;
        t++;
        angle += 0x1000;
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            sDots[i][j].x = 0;
            sDots[i][j].y = 0;
        }
    }
    func_m006_00C74();
    colour_cycle_init(&sCycles[0], 0x29);
    colour_cycle_init(&sCycles[1], 0x2A);
    sBss180 = 0.0f;
    func_m006_01134();
    rl_m061_03528 = func_80055694(0x26A);
    rl_m061_03540 = func_80055694(0x26B);
    rl_m061_03558 = func_80055694(0x26C);
    rl_m061_03570 = func_80055694(0x26D);
    sBss1C8 = 0;
}

void func_m061_003D0(void) {
    menu_assetgroup_free(sAssets);
    func_m006_00EFC();
    func_80055A00(sTextures[0]);
    func_80055A00(sTextures[6]);
    func_80055A00(sTextures[12]);
    func_80055A00(sTextures[18]);
}

/* What char_record_lookup gives, as used here */
typedef struct M061Record {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ u16 unk14[(0x70 - 0x14) / 2]; /* indexed by unk70 */
    /* 0x70 */ u8 unk70;
} M061Record;

/* The player's flags at 0x540 (as m014's 010F4 reads them) */
typedef struct M061Flags {
    /* 31 */ u32 pad : 2;
    /* 29 */ u32 turn : 2;
    /*  0 */ u32 rest : 28;
} M061Flags;

extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern u8 D_800A4FD8;  /* the multiplayer game type */
extern s32 D_800A4FDC;
extern u16 D_800A4FE4; /* the kills to win */
extern u16 D_800FF1B8[]; /* each player's results fade */
extern s32 D_800FF3C8[]; /* the buttons pressed this frame, per player */
extern s32 fb_mode(void);
extern void func_80054EA0(s32 *width, s32 *height); /* the frame buffer size */
extern s32 get_current_viewport(void);
extern s32 cam_get_active_camera_id(void);
extern s32 cam_set_layout(s32 mode);
extern void set_active_camera(s32 camera);
extern void func_800408C0(Gfx **dl); /* sets the viewport's scissor */
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_80055B80(Gfx **dl);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC54(s32 font);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern void func_80048518(s32 arg0);
extern M061Record *char_record_lookup(s32 player, s32 character);
extern s32 options_split_vertical_get(void);
extern void func_m006_00F30(s32 corner, s32 player, s32 *x, s32 *y);
extern void func_m006_012D4(M061Player *player, s32 updateRate);
extern void func_m006_00000(Object *obj, s32 health, s32 arg2, s32 updateRate);
extern void func_8005A0D0(s32 item); /* draws a menu item */
extern void menu_digits_draw(s32 number, s32 x, s32 y, s32 font, s32 digits, s32 colour, s32 padColour);
extern void menu_draw_layout_box(Gfx **dl, s32 index, s32 colour);
extern void menu_layout_box_get(s32 index, s32 *x1, s32 *y1, s32 *x2, s32 *y2);
extern void func_8004F02C(Gfx **dl, void *image, f32 x, f32 y, f32 xScale, f32 yScale, u32 colour, s32 flags);
extern void sound_stop(s32 handle);
extern void set_game_paused(s32 mode);
extern void func_800468C0(s32 arg0);
extern void func_80047608(s32 arg0, s32 arg1, s32 arg2);
extern void func_m013_00008(Object *obj, M061Player *player, s32 updateRate);
extern void rl_m061_019E0(Object **objs, s32 updateRate);
extern s32 rl_m061_023B8(s32 count, s32 arg1, s32 updateRate);
extern void rl_m061_01218(M061Player *player, s32 updateRate);
void func_m061_00F50(void);
void func_m061_010D4(s32 mode);
void func_m061_01F14(Object *obj, Object **objs, s32 updateRate);
void func_m061_02E84(s32 count, Object **objs);
#define sState (*(u8 *) &D_800A4FEC)
#define sRankImages ((void **) &rl_m061_02FA0[0x5E4])
#define sRankTexts ((char **) &rl_m061_02FA0[0x5F4])
#define sRankColours ((u32 *) &rl_m061_02FA0[0x604])

#ifdef NON_MATCHING
/*
 * 2 aligned diffs (709/709 words, frame right): the record's reload one t-register low (t5, ROM t6); everything
 * else is the ROM's. (record->unk70 is a u8, so its & 0xFF changes nothing; it gives the ROM's temporaries after
 * the reload.)
 */
/*
 * The deathmatch screen each frame: the end of the round (by kills, by the last player standing (game type 0x30) or
 * 023B8's test (0x10)), then each player's HUD with its kills, its results once the round is over, and a button to go
 * on.
 */
void func_m061_0043C(s32 count, Object **objs, s32 updateRate) {
    s32 i;
    s32 alpha;
    s32 n;
    s32 x;
    s32 y;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 textX;
    s32 textY;
    s32 k;
    s32 viewport;
    s32 mode;
    s32 camera;
    Object *obj;
    s32 sound;
    M061Player *player;
    s32 width;
    s32 height;
    M061Stats *stats;
    s32 shade;
    M061Record *record;
    UNUSED s32 pad; /* (a word in the ROM's frame) */

    /*
     * (Game type 0x20 sets nothing up here: the test is empty, but IDO keeps its 0x20 for case 2's switch, as the
     * ROM does, where without it the constant goes to a saved register.)
     */
    if ((D_800A4FD8 & 0x30) == 0x20) {
    }
    sound = FALSE;
    rl_m061_019E0(objs, updateRate);
    mode = fb_mode();
    func_80054EA0(&width, &height);
    viewport = get_current_viewport();
    camera = cam_get_active_camera_id();
    cam_set_layout(0);
    set_active_camera(0);
    func_800408C0(&D_800FF398);
    func_80040D64(&D_800FF398, &D_800FF39C);
    func_8006FC8C(0, 0, 0, 0);
    if (!sState) {
        switch (D_800A4FD8 & 0x30) {
            case 0x10:
                if (rl_m061_023B8(count, D_800A4FDC, updateRate)) {
                    func_m061_00F50();
                    func_m061_02E84(count, objs);
                }
                break;
            case 0x30:
                n = 0;
                stats = D_800FED50;
                i = count;
                while (i--) {
                    if (stats->unk13 != 0) {
                        n++;
                    }
                    stats++;
                }
                stats = D_800FED50;
                i = count;
                while (i--) {
                    if (stats->unk13 == 0) {
                        if (stats->unk15 == 0) {
                            stats->rank = n + 1;
                            stats->unk15 = 1;
                        }
                    } else {
                        stats->unk90 += updateRate;
                        if (stats->unk90 >= 360000) {
                            stats->unk90 = 359999;
                        }
                        stats->rank = 1;
                    }
                    stats++;
                }
                if (n < 2) {
                    func_m061_010D4(4);
                    func_m061_02E84(count, objs);
                }
                break;
            default:
                for (i = 0, n = 0; i < count; i++) {
                    if (D_800FED50[i].kills >= D_800A4FE4) {
                        n++;
                    }
                }
                if (n != 0) {
                    i = 0;
                    while (i < count) {
                        D_800FED50[i].unk15 = 1;
                        i++;
                    }
                    func_m061_00F50();
                    func_m061_02E84(count, objs);
                }
                break;
        }
    }
    if (sState) {
        func_80048518(10);
    }
    for (i = 0; i < count; i++) {
        obj = objs[i];
        if (obj != NULL) {
            player = obj->unk68;
            func_m006_012D4(player, updateRate);
            rl_m061_01218(player, updateRate);
            record = char_record_lookup(player->unk0, player->unk1);
            if (D_800A4FD0) {} /* (the player count read once for both: the ROM's load ahead of the mode test) */
            if ((mode & 1) || (D_800A4FD0 == 2 && options_split_vertical_get())) {
            }
            func_8006FC54(2);
            func_8006FC68(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            func_80040D64(&D_800FF398, &D_800FF39C);
            func_80055B80(&D_800FF398);
            D_800FF780[0].unk6 = (record->unk70 & 0xFF) + 10;
            if ((((M061Flags *) &player->unk540)->turn & 2) || (((M061Flags *) &player->unk540)->turn & 1)) {
                sound = TRUE;
            }
            func_m006_00F30(1, player->unk0, &x, &y);
            D_800FF780[0].unkC = x;
            D_800FF780[0].unk10 = y;
            func_8005A0D0(0);
            menu_digits_draw(record->unk14[record->unk70], x + 12, y - 9, 9, 3, 0xFFFF00FF, 0xFFFF0080);
            func_m006_00000(obj, obj->interactObj->hitPoints >> 8, 12, updateRate);
            if ((D_800A4FD8 & 0x30) == 0x30) {
                func_m006_00F30(0, player->unk0, &x, &y);
                menu_digits_draw(D_800FED50[i].unk13, x + 5, y + 3, 9, 1, 0xFFFF00FF, 0xFFFF0080);
            }
            func_m061_01F14(obj, objs, updateRate);
            if ((sState > 0 && sState < 10) || D_800FED50[i].unk15 != 0) {
                D_800FF1B8[i] += updateRate * 2;
                if (D_800FF1B8[i] >= 256) {
                    D_800FF1B8[i] = 255;
                }
                shade = 0xFF - D_800FF1B8[i];
                shade = (shade << 24) | (shade << 16) | (shade << 8);
                shade |= sRankColours[D_800FED50[i].rank];
                menu_draw_layout_box(&D_800FF398, i, shade);
                menu_layout_box_get(i, &x1, &y1, &x2, &y2);
                textX = ((x1 + x2) >> 1) + 10;
                textY = ((y1 + y2) >> 1) - 13;
                func_8006FC54(2);
                func_8006FC68(0, 0, 0, 0xFF, D_800FF1B8[i]);
                func_8006FCA8(&D_800FF398, textX, textY, sRankTexts[D_800FED50[i].rank], 12);
                func_8006FC68(0, 0xFF, 0x80, 0xFF, D_800FF1B8[i]);
                func_8006FCA8(&D_800FF398, textX - 1, textY - 1, sRankTexts[D_800FED50[i].rank], 12);
                alpha = D_800FF1B8[i];
                if (alpha == 0xFF) {
                    alpha = 0xFE;
                }
                func_8004F02C(&D_800FF398, sRankImages[D_800FED50[i].rank], textX - 20, textY + 7, 1.5f, 1.5f, alpha, 1);
                func_8004F02C(&D_800FF398, sRankImages[D_800FED50[i].rank], textX - 19, textY + 6, 1.5f, 1.5f,
                              alpha | 0xFF8000, 1);
            }
        }
    }
    if (sound) {
        if (sBss1C8 != 0) {
            sound_stop(sBss1C8);
        }
        func_80001248(0x8B, (s32) &sBss1C8); /* (module.h's prototype: the handle's address as an s32) */
    }
    switch (sState) {
        case 1:
            n = 0;
            for (i = 0; i < count; i++) {
                if (D_800FF1B8[i] >= 0xFF) {
                    n++;
                }
            }
            if (n == count) {
                sState = 2;
                set_game_paused(2);
            }
            break;
        case 2:
            for (i = 0; i < count && sState == 2; i++) {
                if (D_800FF3C8[i] & 0x9000) {
                    func_800468C0(1);
                    switch (D_800A4FD8 & 0x30) {
                        case 0x10:
                            k = 0x13;
                            break;
                        case 0x20:
                            k = 0x12;
                            break;
                        default:
                            k = 0x14;
                            break;
                    }
                    func_80047608(k, 0, 0);
                    sState = 3;
                }
            }
            break;
    }
    cam_set_layout(viewport);
    set_active_camera(camera);
    for (i = 0; i < count; i++) {
        obj = objs[i];
        if (obj != NULL) {
            player = obj->unk68;
            if (player->unk568 == 5 || player->unk568 == 11) {
                func_m013_00008(obj, player, updateRate);
            }
        }
    }
    if (sState == 3) {
        cam_set_layout(0);
        set_active_camera(0);
        func_800408C0(&D_800FF398);
        cam_set_layout(viewport);
        set_active_camera(camera);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_0043C);
#endif

/* Sorts the players by kills, then by their unk0[unk8C], and gives them their ranks */
void func_m061_00F50(void) {
    s32 i;
    s32 sorted;
    s32 rank;
    M061Stats *a;
    M061Stats *b;

    for (i = 0; i < D_800A4FD0; i++) {
        D_800FF1C0[i] = &D_800FED50[i];
    }
    do {
        sorted = TRUE;
        i = D_800A4FD0 - 1;
        while (i--) {
            a = D_800FF1C0[i];
            b = D_800FF1C0[i + 1];
            if (a->kills < b->kills ||
                (b->kills == a->kills && b->unk0[b->unk8C] < a->unk0[a->unk8C])) {
                D_800FF1C0[i] = b;
                D_800FF1C0[i + 1] = a;
                sorted = FALSE;
            }
        }
    } while (!sorted);
    i = 0;
    b = NULL;
    while (i < D_800A4FD0) {
        a = D_800FF1C0[i];
        i++;
        if (b == NULL) {
            rank = i;
        } else if (b->kills != a->kills || b->unk0[b->unk8C] != a->unk0[a->unk8C]) {
            rank = i;
        }
        a->rank = rank;
        b = a;
    }
}

/* Sorts the players by one of their scores (the pass loop's `continue; break;` shape keeps IDO
   from unrolling it) */
void func_m061_010D4(s32 mode) {
    s32 i;
    s32 sorted;
    s32 n;
    M061Stats *a;
    M061Stats *b;

    for (i = 0; i < D_800A4FD0; i++) {
        D_800FF1C0[i] = &D_800FED50[i];
    }
    n = D_800A4FD0 - 1;
    do {
        sorted = TRUE;
        i = 0;
        if (n > 0) {
            while (TRUE) {
                a = D_800FF1C0[i];
                b = D_800FF1C0[i + 1];
                if ((mode == 0 && a->kills < b->kills) || (mode == 1 && a->unk90 < b->unk90) ||
                    (mode == 2 && b->unk90 < a->unk90) || (mode == 3 && a->unk9C < b->unk9C) ||
                    (mode == 4 && b->rank < a->rank)) {
                    D_800FF1C0[i] = b;
                    D_800FF1C0[i + 1] = a;
                    sorted = FALSE;
                }
                i++;
                if (i < n) continue;
                break;
            }
        }
    } while (!sorted);
}

#ifdef NON_MATCHING
/* A player's message queue (D_800FF1D0, one per player; m006's) */
typedef struct M061Queue {
    /* 0x00 */ s16 timer;
    /* 0x02 */ s16 current; /* the message shown (an index into gMenuText), or -1 */
    /* 0x04 */ u8 pad4[0x12 - 0x4];
    /* 0x12 */ char text[0x4E - 0x12];
    /* 0x4E */ s16 width; /* how much of the text the bar shows */
} M061Queue;

/* What render_ortho_triangle_image draws an image with */
typedef struct M061Sprite {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 frame;
} M061Sprite;

extern u8 D_800A4FCC; /* the number of cameras */
extern char **gMenuText;
extern M061Queue D_800FF1D0[];
extern Vertex *D_800FF3A0;
extern Object *D_800FF3F8[]; /* the menu's images */
extern s8 *func_m006_011C8(void);  /* each player's message bar position (75: hidden) */
extern s16 *func_m006_011D4(void); /* each player's message bar width */
extern void func_m006_0107C(s32 *x, s32 *y);
extern void func_80042134(MtxF *mtx); /* identity */
extern int func_800639E8(char *buf, char *fmt, ...); /* sprintf */
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 texOffset); /* DKR material_set */
extern void render_ortho_triangle_image(Gfx **dl, Mtx **mtx, Vertex **vtx, M061Sprite *sprite, void *tex, s32 arg5,
                                        u8 alpha);
extern s32 func_800706D4(char *text, s32 x, s32 font); /* DKR get_text_width */
/* The dialogue boxes (as DKR's font.c) */
extern void func_8007082C(s32 box, s32 font);                                 /* set_dialogue_font */
extern void func_80070BF8(s32 box);                                           /* dialogue_clear */
extern void func_8007078C(s32 box, s32 x1, s32 y1, s32 x2, s32 y2);           /* set_current_dialogue_box_coords */
extern void func_80070848(s32 box, s32 r, s32 g, s32 b, s32 a);               /* set_current_dialogue_background_colour */
extern void func_800708D0(s32 box, s32 r, s32 g, s32 b, s32 a);               /* set_current_text_background_colour */
extern void func_80070888(s32 box, s32 r, s32 g, s32 b, s32 a, s32 opacity);  /* set_current_text_colour */
extern void func_80070910(s32 box, s32 x, s32 y, char *text, s32 number, s32 flags); /* render_dialogue_text */
extern void func_80070EAC(Gfx **dl, Mtx **mtx, Vertex **vtx, s32 box);      /* render_dialogue_box */

/*
 * Draws a player's message bar (the queue's current message, in a box that opens to the text's width) in the
 * player's part of the screen, with its two end caps (called from 0043C).
 * NON_MATCHING: a first body, 499/498 words with the ROM's frame and slots, 374 aligned diffs (mostly the code
 * generator's temporaries one or two off); the default build uses the assembly.
 */
void func_m061_01218(M061Player *player, s32 updateRate) {
    MtxF mtx;
    s32 x;
    s32 y;
    s32 x1;
    s32 x2;
    s32 y1;
    s32 y2;
    s8 *barY;
    Vertex *v;
    UNUSED s32 pad;
    s32 textX;
    M061Queue *queue;
    UNUSED u8 pad2[0x88];
    M061Sprite sprite;
    UNUSED s32 pad3[2];
    s16 *widths;

    barY = func_m006_011C8();
    widths = func_m006_011D4();
    queue = &D_800FF1D0[player->unk0];
    if (queue->timer == 0 && barY[player->unk0] == 75) {
        return;
    }
    func_80042134(&mtx);
    /* (with one camera x and y are left unset, as in the original) */
    switch (D_800A4FCC) {
        case 2:
            if ((fb_mode() & 1) || options_split_vertical_get()) {
                /* split side by side: as for four (the ROM tests the player here, then goes on either way) */
                if (player->unk0 != 0) {
                    goto quad;
                }
                goto quad;
            }
            /* split top and bottom */
            x = 0;
            if (player->unk0 == 0) {
                y = barY[player->unk0] + 60;
            } else {
                y = barY[player->unk0] - 60;
            }
            break;
        case 3:
        case 4:
        quad:
            if (player->unk0 == 0 || player->unk0 == 2) {
                x = -94;
            } else {
                x = 66;
            }
            if (player->unk0 == 0 || player->unk0 == 1) {
                y = barY[player->unk0] + 60;
            } else {
                y = barY[player->unk0] - 60;
            }
            break;
    }
    if (queue->current == -1) {
        /* (no message: the bar shows what its text already holds) */
    } else if (queue->current == 0x9F) {
        func_800639E8(queue->text, "%s%d", gMenuText[0x9F], D_800FED50[player->unk0].kills);
    } else {
        func_800639E8(queue->text, "%s", gMenuText[queue->current]);
    }
    cam_set_layout(D_800A4FCC - 1);
    set_active_camera(player->unk0);
    func_800408C0(&D_800FF398);
    func_80040D64(&D_800FF398, &D_800FF39C);
    if (widths[player->unk0] >= 2) {
        /* the bar: a quad as wide as the bar is open */
        gSPVertexJFG(D_800FF398++, (u32) D_800FF3A0 + 0x80000000, 5, 0);
        v = D_800FF3A0;
        v[0].y = y + 6;
        v[0].x = x - (widths[player->unk0] >> 1);
        v[1].y = y + 6;
        v[1].x = (widths[player->unk0] >> 1) + x;
        v[2].y = y - 7;
        v[2].x = x - (widths[player->unk0] >> 1);
        v += 4;
        v[-1].y = y - 7;
        v[-1].x = (widths[player->unk0] >> 1) + x;
        do {
            D_800FF3A0->z = 0;
            D_800FF3A0->r = 0xFF;
            D_800FF3A0->g = 0xFF;
            D_800FF3A0->b = 0xFF;
            D_800FF3A0->a = 0xFF;
        } while (++D_800FF3A0 < v);
        func_80055C00(&D_800FF398, NULL, 0, 0);
        gDPPipeSync(D_800FF398++);
        gDPSetEnvColor(D_800FF398++, 0x00, 0x28, 0x32, 0x60);
        gDPSetPrimColor(D_800FF398++, 0, 0, 0x00, 0x1E, 0x5A, 0x00);
        /* a display list of two commands in .data */
        { Gfx *_g = D_800FF398++; _g->words.w0 = 0x07020010; _g->words.w1 = (u32) (rl_m061_02FA0 + 0x80000480); }
        gSPPolygon(D_800FF398++, rl_m061_02FA0 + 0x800001E8, 2, 1);
    }
    /* the end caps */
    sprite.trans.x_position = (widths[player->unk0] >> 1) + x + 2;
    sprite.trans.y_position = y;
    sprite.trans.rotation.z_rotation = 0;
    sprite.trans.rotation.x_rotation = 0;
    sprite.trans.rotation.y_rotation = 0;
    sprite.trans.z_position = 0.0f;
    sprite.frame = 0.0f;
    sprite.trans.scale = 0.5f;
    func_80055B80(&D_800FF398);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[4], 0, 255);
    sprite.trans.rotation.y_rotation = -0x8000;
    sprite.trans.x_position = x - (widths[player->unk0] >> 1) - 2;
    render_ortho_triangle_image(&D_800FF398, &D_800FF39C, &D_800FF3A0, &sprite, D_800FF3F8[4], 0, 255);
    /* the text, scrolling in as the bar opens */
    x1 = x - (widths[player->unk0] >> 1);
    x2 = widths[player->unk0] + x1;
    y1 = y + 5;
    y2 = y - 6;
    func_m006_0107C(&x1, &y1);
    func_m006_0107C(&x2, &y2);
    func_8007082C(1, 2);
    func_80070BF8(1);
    func_8007078C(1, x1, y1, x2, y2);
    func_80070848(1, 0, 0, 0, 0);
    func_800708D0(1, 0, 0, 0, 0);
    if (queue->width < func_800706D4(queue->text, 2, 1)) {
        queue->width += updateRate;
    }
    textX = x2 - x1 - queue->width - 5;
    func_80070BF8(1);
    func_80070888(1, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    func_80070910(1, textX, 1, queue->text, 1, 0);
    func_80070888(1, 0, 0, 0x80, 0xFF, 0xFF);
    func_80070910(1, textX + 1, 2, queue->text, 1, 0);
    func_80070EAC(&D_800FF398, &D_800FF39C, &D_800FF3A0, 1);
    func_80070BF8(1);
    cam_set_layout(0);
    set_active_camera(0);
    func_800408C0(&D_800FF398);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m061/ro", func_m061_01218);
#endif

/* A camera (func_80041E0C's array, 0x4C each), as used here */
typedef struct M061Camera {
    /* 0x00 */ s16 yaw;
    /* 0x02 */ u8 pad2[0x4C - 0x2];
} M061Camera;

extern M061Camera *func_80041E0C(void); /* the cameras */
extern s32 func_80049C84(f32 x, f32 z);  /* the angle of a direction */
extern f32 func_80097140(f32 x);         /* sqrtf */
extern u8 D_800A5064;
extern s16 rl_m061_03418; /* .data 0x478: the radar sweep's angle */
extern s32 rl_m061_0341C; /* .data 0x47C: the sweep's angle before this frame's step */

#ifdef NON_MATCHING
/*
 * Turns the radar sweep and moves every player's dots: each other player's offset, turned to the player's
 * camera and clamped to the radar's edge, with a colour for above / level / below; a dot lights up as the
 * sweep passes it (with the sweep on) and fades, and is hidden for a player who is out.
 * (Matches, staged: only the assembly func_m061_0043C calls it, so IDO would drop it. With 0043C in C,
 * its call becomes a direct call of this function.)
 */
static void func_m061_019E0(Object **objs, s32 updateRate) {
    s32 i;
    s32 j;
    s32 colour;
    s32 alpha;
    s32 shown;
    s32 sweepStart;
    s32 sweepEnd;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 rx;
    f32 ry;
    f32 c;
    s32 angle;
    Object *obj;
    M061Player *player;
    Object *other;
    M061Player *otherState;
    M061Camera *cams;
    UNUSED s32 pad;

    cams = func_80041E0C();
    rl_m061_0341C = rl_m061_03418;
    rl_m061_03418 -= updateRate << 10;
    if (rl_m061_03418 > 20000 && rl_m061_0341C < 20000) {
        rl_m061_0341C += 0x10000;
    }
    sweepStart = rl_m061_03418;
    if (sweepStart < 0) {
        sweepStart = -sweepStart;
    } else {
        sweepStart = 0x10000 - sweepStart;
    }
    sweepEnd = rl_m061_0341C;
    if (sweepEnd < 0) {
        sweepEnd = -sweepEnd;
    } else {
        sweepEnd = 0x10000 - sweepEnd;
    }
    if (sweepStart < sweepEnd) {
        sweepEnd -= 0x10000;
    }
    rl_m061_03418 += updateRate << 9;
    for (i = 0; i < D_800A4FD0; i++) {
        obj = objs[i];
        player = obj->unk68;
        for (j = 0; j < D_800A4FD0; j++) {
            other = objs[j];
            otherState = other->unk68;
            dx = (other->trans.x_position - obj->trans.x_position) / 100.0f;
            dy = other->trans.y_position - obj->trans.y_position;
            dz = (other->trans.z_position - obj->trans.z_position) / 100.0f;
            d = func_80097140(dx * dx + dz * dz);
            if (15.0f < d) {
                d = 15.0f / d;
                if (obj) {} /* (steers the register allocation: obj is always set) */
                dx *= d;
                dz *= d;
            }
            if (d == 0.0f) {
                sDots[player->unk0][j].x = 0;
                sDots[player->unk0][j].y = 0;
            } else {
                shown = FALSE;
                if (dy < -30.0f) {
                    colour = 0;
                } else if (30.0f < dy) {
                    colour = 2;
                } else {
                    colour = 1;
                }
                angle = 0x8000 - cams[player->unk0].yaw;
                dz = -dz;
                c = func_80049C30(angle);
                rx = func_80049C34(angle) * dz + dx * c;
                c = func_80049C30(angle);
                ry = dz * c - func_80049C34(angle) * dx;
                switch (D_800A5064) {
                    case 1:
                        alpha = sDots[player->unk0][j].alpha;
                        angle = func_80049C84(rx, ry);
                        if (angle <= sweepStart && angle >= sweepEnd) {
                            alpha = 255;
                            shown = TRUE;
                        } else if ((updateRate << 3) < alpha) {
                            alpha -= updateRate << 3;
                        } else {
                            alpha = 0;
                        }
                        break;
                    case 0:
                        shown = TRUE;
                        alpha = 255;
                        break;
                }
                if (shown) {
                    sDots[player->unk0][j].x = rx;
                    sDots[player->unk0][j].y = ry;
                    sDots[player->unk0][j].colour = colour;
                }
                if (otherState->unk10E != 0 || other->interactObj->hitPoints < 0x100) {
                    sDots[player->unk0][j].alpha = 0;
                } else {
                    sDots[player->unk0][j].alpha = alpha;
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_019E0);
#endif

extern Gfx *D_800FF398;
extern Mtx *D_800FF39C;
extern Object *D_800FF3F8[]; /* the menu's images */
extern u8 D_800A5064;
extern void func_m006_00F30(s32 corner, s32 player, s32 *x, s32 *y);
extern void func_m006_0107C(s32 *x, s32 *y);
extern void func_80042134(MtxF *mtx);                      /* identity */
extern void func_800498E8(f32 x, f32 y, f32 z, MtxF *mtx); /* translation */
extern void func_80049978(s16 angle, MtxF *mtx);
extern void func_80048D84(MtxF *mtx, Mtx *out);            /* to a fixed-point Mtx */
extern void func_80040D64(Gfx **dl, Mtx **mtx);
extern void func_80055B80(Gfx **dl);
extern void menu_fill_box(Gfx **dl, s32 x1, s32 y1, s32 x2, s32 y2, s32 colour); /* fills a rectangle */
#define sRadarAngle (*(s16 *) &rl_m061_02FA0[0x478])
extern u32 rl_m061_035B8[]; /* .data (still assembly): the dots' colours */
#define sDotColours rl_m061_035B8

/* Draws a player's radar in its corner of the screen, with a dot for each other player (called from 0043C) */
void func_m061_01F14(Object *obj, UNUSED Object **objs, UNUSED s32 updateRate) {
    UNUSED s32 pad;
    s32 x;
    s32 y;
    s32 dotX;
    s32 dotY;
    MtxF mtx;
    s8 *player = obj->unk68;
    M061Dot *dot;
    s32 i;
    u32 colour;
    Object *tex;

    func_m006_00F30(3, *player, &x, &y);
    func_80042134(&mtx);
    func_800498E8(x, y, 0.0f, &mtx);
    func_80048D84(&mtx, D_800FF39C);
    tex = D_800FF3F8[0x33];
    {
        Gfx *_g = D_800FF398++;
        _g->words.w0 = 0x07070038;
        _g->words.w1 = *(u32 *) ((u8 *) tex + 0xC) + 0x80000000;
    }
    {
        Gfx *_g = D_800FF398++;
        _g->words.w0 = 0x01000040;
        _g->words.w1 = (u32) D_800FF39C + 0x80000000;
    }
    D_800FF39C = (Mtx *) ((u8 *) D_800FF39C + 0x40);
    gDPPipeSync(D_800FF398++);
    gDPSetEnvColor(D_800FF398++, 0x00, 0x28, 0x32, 0x60);
    gDPSetPrimColor(D_800FF398++, 0, 0, 0x00, 0x1E, 0x5A, 0x00);
    /* a display list of two commands in .data */
    { Gfx *_g = D_800FF398++; _g->words.w0 = 0x07020010; _g->words.w1 = (u32) (rl_m061_02FA0 + 0x80000490); }
    gSPVertexJFG(D_800FF398++, rl_m061_02FA0 + 0x800003D0, 10, 0);
    gSPPolygon(D_800FF398++, rl_m061_02FA0 + 0x80000350, 8, 1);
    func_80040D64(&D_800FF398, &D_800FF39C);
    func_80055B80(&D_800FF398);
    dot = sDots[*player];
    for (i = 0; i < D_800A4FD0; i++, dot++) {
        if (i != *player || dot->alpha != 0) {
            colour = sDotColours[dot->colour] | dot->alpha;
            dotX = dot->x + x;
            dotY = dot->y + y;
            func_m006_0107C(&dotX, &dotY);
            menu_fill_box(&D_800FF398, dotX - 1, dotY - 1, dotX + 1, dotY + 1, colour);
        }
    }
    if (D_800A5064 == 1) {
        func_80049978(sRadarAngle, &mtx);
        func_80048D84(&mtx, D_800FF39C);
        {
            Gfx *_g = D_800FF398++;
            _g->words.w0 = 0x01000040;
            _g->words.w1 = (u32) D_800FF39C + 0x80000000;
        }
        D_800FF39C = (Mtx *) ((u8 *) D_800FF39C + 0x40);
        tex = D_800FF3F8[0x32];
        { Gfx *_g = D_800FF398++; _g->words.w0 = 0x07020010; _g->words.w1 = (u32) (rl_m061_02FA0 + 0x800004A0); }
        gDPSetPrimColor(D_800FF398++, 0, 0, 0x00, 0xFF, 0xFF, 0xFF);
        {
            Gfx *_g = D_800FF398++;
            _g->words.w0 = 0x07070038;
            _g->words.w1 = *(u32 *) ((u8 *) tex + 0xC) + 0x80000000;
        }
        gSPVertexJFG(D_800FF398++, rl_m061_02FA0 + 0x80000450, 5, 0);
        gSPPolygon(D_800FF398++, rl_m061_02FA0 + 0x80000430, 2, 1);
        func_80040D64(&D_800FF398, &D_800FF39C);
        func_80055B80(&D_800FF398);
    }
    dotX = x;
    dotY = y;
    func_m006_0107C(&dotX, &dotY);
    menu_fill_box(&D_800FF398, dotX - 1, dotY - 1, dotX + 1, dotY + 1, 0xFFFF00FF);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
}

#ifdef NON_MATCHING
extern s32 is_game_paused(void);
extern s32 gMenuTimer; /* the timed round's frames so far */
extern Vertex *D_800FF3A0;
extern Triangle *D_800FF3A4;
extern void colour_cycle_reset(M061ColourCycle *cycle);
extern void update_colour_cycle(M061ColourCycle *cycle, s32 updateRate);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 texOffset); /* DKR material_set */
extern M061ColourCycle rl_m061_03770; /* = sCycles[1] */

/*
 * The timed round's clock (game type 0x10; called from 0043C): the time counts up to `limit` unless the game is
 * paused; the dial shows the time left as whole sectors of the inner ring and one part sector, its colour (two
 * colour cycles) turning to the second cycle's in the last 1200 frames. TRUE once the time is up.
 * NON_MATCHING: 691/691 words with the ROM's frame and slots, every float register right; 121 diffs, all one
 * t-register shift from the part sector's vertex command on (the ROM has one more temporary before it, after the
 * whole-sector gSPPolygon). The default build uses the assembly.
 * (x and y start from an int 0 and the shadow is offset by an int 1: as float constants of their own, IDO doesn't
 * share them with the 0.0f arguments and the colour sums' 1.0f, as in the ROM. sCycles[1] is named by its own
 * symbol, which the ROM addresses with its own base register.)
 */
s32 func_m061_023B8(s32 count, s32 limit, s32 updateRate) {
    s32 done;
    s32 angle;
    s32 i;
    s32 r;
    UNUSED s32 pad;
    s32 g;
    s32 b;
    UNUSED s32 pad2[4];
    MtxF mtx;
    f32 s;
    f32 c;
    UNUSED s32 pad3;
    f32 x;
    f32 y;

    if (!is_game_paused()) {
        gMenuTimer += updateRate;
    }
    if (gMenuTimer >= limit) {
        gMenuTimer = limit;
        done = TRUE;
    } else {
        done = FALSE;
    }
    func_80042134(&mtx);
    x = 0;
    y = 0;
    if (count == 2) {
        if (!(fb_mode() & 1) && !options_split_vertical_get()) {
            x = 100.0f;
        } else {
            y = -85.0f;
        }
    }
    func_800498E8(x, y, 0.0f, &mtx);
    func_80048D84(&mtx, D_800FF39C);
    { Gfx *_g = D_800FF398++; _g->words.w0 = 0x01000040; _g->words.w1 = (u32) D_800FF39C + 0x80000000; }
    D_800FF39C = (Mtx *) ((u8 *) D_800FF39C + 0x40);
    angle = ((f32) (limit - gMenuTimer) * 65536.0f) / (f32) limit;
    colour_cycle_reset(&sCycles[0]);
    update_colour_cycle(&sCycles[0], ((0x10000 - angle) * 60) / 0x10000);
    if (limit - gMenuTimer <= 1200) {
        update_colour_cycle(&rl_m061_03770, updateRate);
        for (i = 0; i < updateRate; i++) {
            sBss180 += (1.0f - sBss180) * 0.02f;
        }
    }
    r = sBss180 * rl_m061_03770.colour[0] + sCycles[0].colour[0] * (1.0f - sBss180);
    g = sBss180 * rl_m061_03770.colour[1] + sCycles[0].colour[1] * (1.0f - sBss180);
    b = sBss180 * rl_m061_03770.colour[2] + sCycles[0].colour[2] * (1.0f - sBss180);
    /* the dial's face */
    func_80055C00(&D_800FF398, D_800FF3F8[0x31], 0x205, 0);
    gDPSetPrimColor(D_800FF398++, 0, 0, r, g, b, 45.0f + 100.0f * sBss180);
    gSPVertexJFG(D_800FF398++, rl_m061_02FA0 + 0x80000208, 5, 0);
    gSPPolygon(D_800FF398++, rl_m061_02FA0 + 0x80000230, 2, 1);
    if (!done) {
        /* the time left, one pixel down and right */
        func_80042134(&mtx);
        func_800498E8(x + 1, y + 1, 0.0f, &mtx);
        func_80048D84(&mtx, D_800FF39C);
        { Gfx *_g = D_800FF398++; _g->words.w0 = 0x01000040; _g->words.w1 = (u32) D_800FF39C + 0x80000000; }
        D_800FF39C = (Mtx *) ((u8 *) D_800FF39C + 0x40);
        func_80055C00(&D_800FF398, D_800FF3F8[0x2F], 5, 0);
        gDPSetPrimColor(D_800FF398++, 0, 0, 0xFF, 0xFF, 0xFF, 0xC0);
        gSPVertexJFG(D_800FF398++, (u8 *) sInner + 0x80000000, 18, 0);
        if (angle / 0x1000 != 0) {
            gSPPolygon(D_800FF398++, rl_m061_02FA0 + 0x80000250, angle / 0x1000, 1);
        }
        if (angle != (angle / 0x1000) * 0x1000) {
            /* the part sector: one more vertex on the ring (loaded as vertices 17 and 18) */
            s = func_80049C34(angle);
            c = func_80049C30(angle);
            gDma1p(D_800FF398++, G_VTX, (u32) D_800FF3A0 + 0x80000000, (17 << 9) | (2 * 10 - 2),
                   ((2 - 1) << 3) | (((u32) D_800FF3A0 + 0x80000000) & 6));
            D_800FF3A0->x = 15.0f * s;
            D_800FF3A0->y = 15.0f * c;
            D_800FF3A0->z = 0;
            D_800FF3A0->r = 0xFF;
            D_800FF3A0->g = 0xFF;
            D_800FF3A0->b = 0xFF;
            D_800FF3A0->a = 0xFF;
            D_800FF3A0++;
            D_800FF3A4->flags = 0x40;
            D_800FF3A4->vi0 = 0;
            D_800FF3A4->vi1 = angle / 0x1000 + 1;
            D_800FF3A4->vi2 = 17;
            D_800FF3A4->uv[0][0] = 0x220;
            D_800FF3A4->uv[0][1] = 0x220;
            D_800FF3A4->uv[1][0] = func_80049C34((angle / 0x1000) * 0x1000) * 512.0f + 544.0f;
            D_800FF3A4->uv[1][1] = func_80049C30((angle / 0x1000) * 0x1000) * 512.0f + 544.0f;
            D_800FF3A4->uv[2][0] = 512.0f * s + 544.0f;
            D_800FF3A4->uv[2][1] = 512.0f * c + 544.0f;
            gSPPolygon(D_800FF398++, (u32) D_800FF3A4 + 0x80000000, 1, 1);
            D_800FF3A4++;
        }
    }
    /* the dial's rim */
    func_80042134(&mtx);
    func_800498E8(x, y, 0.0f, &mtx);
    func_80048D84(&mtx, D_800FF39C);
    { Gfx *_g = D_800FF398++; _g->words.w0 = 0x01000040; _g->words.w1 = (u32) D_800FF39C + 0x80000000; }
    D_800FF39C = (Mtx *) ((u8 *) D_800FF39C + 0x40);
    gDPSetPrimColor(D_800FF398++, 0, 0, 255, 255, 255, 255);
    func_80055C00(&D_800FF398, D_800FF3F8[0x30], 0, 0);
    gSPVertexJFG(D_800FF398++, rl_m061_02FA0 + 0x80000208, 5, 0);
    gSPPolygon(D_800FF398++, rl_m061_02FA0 + 0x80000230, 2, 1);
    return done;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m061/ro", func_m061_023B8);
#endif

void func_m061_02E84(s32 count, Object **objs) {
    s32 i;
    M061Player *player;

    D_800A4FEC = 1;
    for (i = 0; i < count; i++) {
        if (objs[i] != NULL) {
            player = objs[i]->unk68;
            player->unk5C8 = 240;
        }
    }
    music_volume_fade(2.0f, 0);
}

INCLUDE_ASM("asm/nonmatchings/modules/m061", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m061", bss);
