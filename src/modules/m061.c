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

extern void func_80059CCC(s16 *assets); /* frees menu textures */
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

INCLUDE_ASM("asm/nonmatchings/modules/m061", func_m061_00000);

void func_m061_003D0(void) {
    func_80059CCC(sAssets);
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
