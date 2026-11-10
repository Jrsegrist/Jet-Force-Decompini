/*
 * Runlink module 61: the deathmatch results and instruments of the front end
 * (the players ranked by kills and other scores).
 */
#include "module.h"

/* A player's multiplayer scores (D_800FED50, 0xA0 each) */
typedef struct M061Stats {
    /* 0x00 */ u16 unk0[8]; /* indexed by unk8C */
    /* 0x10 */ s16 kills;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ u8 rank;
    /* 0x15 */ u8 pad15[0x8C - 0x15];
    /* 0x8C */ u8 unk8C;
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ u32 unk90;
    /* 0x94 */ u8 pad94[0x9C - 0x94];
    /* 0x9C */ u16 unk9C;
    /* 0x9E */ u8 pad9E[2];
} M061Stats;

/* What the players' objects keep in their unk68 */
typedef struct M061Player {
    /* 0x000 */ u8 pad0[0x5C8];
    /* 0x5C8 */ s16 unk5C8;
} M061Player;

extern u8 D_800A4FD0; /* the number of players */
extern s8 D_800A4FEC;
extern M061Stats D_800FED50[];
extern M061Stats *D_800FF1C0[]; /* the players, sorted */

extern void menu_assetgroup_free(s16 *assets); /* frees menu textures */
extern void func_m006_00EFC(void);
extern void func_80055A00(s32 tex); /* frees a texture */
extern void music_volume_fade(f32 volume, s32 arg1); /* fades the music */

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

/* A mark of the dial (0x10 bytes; four pairs of bytes) */
typedef struct M061Mark {
    /* 0x0 */ u8 a0, a1;
    /* 0x2 */ u8 pad2[2];
    /* 0x4 */ u8 b0, b1;
    /* 0x6 */ u8 pad6[2];
    /* 0x8 */ u8 c0, c1;
    /* 0xA */ u8 padA[2];
    /* 0xC */ u8 d0, d1;
    /* 0xE */ u8 padE[2];
} M061Mark;

/* A colour cycle (55130.c's) */
typedef struct M061ColourCycle {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 colour[4];
    /* 0x0C */ void *source;
} M061ColourCycle;

/* A menu item (D_800FF780, size 0x20; the main segment's MenuItem), as used here */
typedef struct M061MenuItem {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ f32 unk8;
    /* 0x0C */ u8 padC[0x20 - 0xC];
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
extern M061Mark rl_m061_03788[4];
extern s32 rl_m061_037C8;
extern s32 rl_m061_03528, rl_m061_03540, rl_m061_03558, rl_m061_03570; /* .data: = sTextures[0, 6, 12, 18] */
#define sOuter rl_m061_03600 /* the dial's outer ring */
#define sInner rl_m061_036B0 /* and inner ring */
#define sCycles rl_m061_03760
#define sBss180 rl_m061_03780
#define sMarks rl_m061_03788
#define sBss1C8 rl_m061_037C8
#define sTriangles ((M061Triangle *) &rl_m061_02FA0[0x250]) /* 16 */
#define sImages ((s16 *) &rl_m061_02FA0[0x500])

#ifdef NON_MATCHING
/* 6 diffs: the marks loop's end; the ROM loads &marks[4] as its own address, IDO here adds 0x40 to the
   start (and with any end IDO cannot measure, it unrolls the loop) */
/* The module's init: the dial's vertices and triangles (16 sectors), its marks, cycles and sounds */
void func_m061_00000(void) {
    M061Vertex *outer;
    M061Vertex *inner;
    M061Triangle *t;
    M061Mark *m;
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
    for (m = sMarks; m != &sMarks[4]; m++) {
        m->b0 = 0;
        m->b1 = 0;
        m->c0 = 0;
        m->c1 = 0;
        m->d0 = 0;
        m->d1 = 0;
        m->a0 = 0;
        m->a1 = 0;
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
#else
INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_00000);
#endif

void func_m061_003D0(void) {
    menu_assetgroup_free(sAssets);
    func_m006_00EFC();
    func_80055A00(sTextures[0]);
    func_80055A00(sTextures[6]);
    func_80055A00(sTextures[12]);
    func_80055A00(sTextures[18]);
}

INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_0043C);

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

#ifdef NON_MATCHING
/* Sorts the players by one of their scores. IDO hoists the address of D_800FF1C0 out of the pass
   loop, which the ROM recomputes on every pass (register numbering then differs) */
void func_m061_010D4(s32 mode) {
    s32 i;
    s32 sorted;
    s32 n;
    M061Stats **p;
    M061Stats *a;
    M061Stats *b;

    for (i = 0; i < D_800A4FD0; i++) {
        D_800FF1C0[i] = &D_800FED50[i];
    }
    n = D_800A4FD0 - 1;
    do {
        sorted = TRUE;
        if (n > 0) {
            p = D_800FF1C0;
            do {
                a = p[0];
                b = p[1];
                if ((mode == 0 && a->kills < b->kills) || (mode == 1 && a->unk90 < b->unk90) ||
                    (mode == 2 && b->unk90 < a->unk90) || (mode == 3 && a->unk9C < b->unk9C) ||
                    (mode == 4 && b->rank < a->rank)) {
                    p[0] = b;
                    p[1] = a;
                    sorted = FALSE;
                }
                p++;
            } while (p < &D_800FF1C0[n]);
        }
    } while (!sorted);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_010D4);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_01218);

INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_019E0);

INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_01F14);

INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_023B8);

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

INCLUDE_ASM("asm/nonmatchings/modules/m061", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m061", bss);
