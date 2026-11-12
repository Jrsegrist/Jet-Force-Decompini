/*
 * Runlink module 8: an effect library. It keeps a pool of 64 drops that fly,
 * bounce off the level (with a sound) and leave a mark, and a pool of 5
 * sprites drawn at hit points; other modules add to the pools, and the
 * module updates and draws them.
 */
#include "module.h"

/* A drop (size 0x54) */
typedef struct M008Drop {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 prevX;
    /* 0x10 */ f32 prevY;
    /* 0x14 */ f32 prevZ;
    /* 0x18 */ f32 vx;
    /* 0x1C */ f32 vy;
    /* 0x20 */ f32 vz;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 scale;
    /* 0x2C */ u8 pad2C[0x3C - 0x2C];
    /* 0x3C */ s16 timer;
    /* 0x3E */ u8 pad3E[0x4A - 0x3E];
    /* 0x4A */ u8 state; /* 0: free */
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ s16 unk4C;
    /* 0x4E */ s16 unk4E;
    /* 0x50 */ u8 unk50;
    /* 0x51 */ u8 unk51;
    /* 0x52 */ u8 pad52[2];
} M008Drop;

/* A sprite at a hit point (size 0x18) */
typedef struct M008Hit {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 state; /* 0: free */
    /* 0x17 */ u8 pad17;
} M008Hit;

extern void *func_80055694(s32 id); /* loads a texture */
extern void func_80055A00(void *tex);
extern void *func_80056798(s32 id, s32 arg1);
extern void func_80056C14(void *sprite);
extern u8 D_800A5080;

/* The module's data and bss, still assembly */
extern s32 rl_m008_016C0;     /* set once the textures are loaded */
extern s32 rl_m008_017A0;     /* drops in use */
extern s32 rl_m008_017A4;     /* hit sprites in use */
extern void *rl_m008_017A8[5]; /* textures */
extern void *rl_m008_017BC;   /* sprite */
extern M008Drop rl_m008_017C0[64];
extern M008Hit rl_m008_02CC0[5];
extern s32 rl_m008_02D38;

#define sLoaded rl_m008_016C0
#define sDropCount rl_m008_017A0
#define sHitCount rl_m008_017A4
#define sTextures rl_m008_017A8
#define sSprite rl_m008_017BC
#define sDrops rl_m008_017C0
#define sHits rl_m008_02CC0
#define sUnk2D38 rl_m008_02D38

/*
 * NON_MATCHING (00000 and 000C4): IDO unrolls the 64-drop clearing loop four times
 * and tests the pointer, where the ROM keeps a plain loop on the counter (`slti`);
 * no spelling tried keeps both (a `while (i++ < 64)` condition stops the unrolling
 * but moves the increment; `goto` gives `bne`).
 */
#ifdef NON_MATCHING
/* Loads the textures and clears the pools */
static void func_m008_00000(void) {
    s32 i;

    sTextures[0] = func_80055694(0x170);
    sTextures[1] = func_80055694(0x171);
    sTextures[2] = func_80055694(0x172);
    sTextures[3] = func_80055694(0x173);
    sTextures[4] = func_80055694(0x174);
    sSprite = func_80056798(0x43, 1);
    for (i = 0; i < 64; i++) {
        sDrops[i].state = 0;
    }
    sHits[0].state = 0;
    sLoaded = 1;
    sDropCount = 0;
    sHitCount = 0;
    sUnk2D38 = 0;
}

void func_m008_000C4(void) {
    s32 i;

    if (sLoaded) {
        if (sTextures[0] != NULL) {
            func_80055A00(sTextures[0]);
        }
        if (sTextures[1] != NULL) {
            func_80055A00(sTextures[1]);
        }
        if (sTextures[2] != NULL) {
            func_80055A00(sTextures[2]);
        }
        if (sTextures[3] != NULL) {
            func_80055A00(sTextures[3]);
        }
        if (sTextures[4] != NULL) {
            func_80055A00(sTextures[4]);
        }
        if (sSprite != NULL) {
            func_80056C14(sSprite);
        }
        for (i = 0; i < 64; i++) {
            sDrops[i].state = 0;
        }
        sHits[0].state = 0;
    }
    sLoaded = 0;
    sDropCount = 0;
    sHitCount = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m008", func_m008_00000);
extern void rl_m008_00000(void);
#define func_m008_00000 rl_m008_00000

INCLUDE_ASM("asm/nonmatchings/modules/m008", func_m008_000C4);
#endif

void func_m008_001CC(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, s32 angle, s32 colour, f32 scale) {
    M008Drop *p;
    s32 i;

    if (!sLoaded) {
        func_m008_00000();
    }
    i = 0;
    if (sDropCount < 64) {
        for (p = sDrops; i < 64; i++, p++) {
            if (p->state == 0) {
                break;
            }
        }
        if (i < 64) {
            p->vx = vx;
            p->vy = vy;
            p->vz = vz;
            p->scale = scale;
            p->state = 1;
            p->timer = 300;
            p->prevX = x;
            p->prevY = y;
            p->unk24 = 0.0f;
            p->prevZ = z;
            p->x = x;
            p->y = y;
            p->z = z;
            p->unk4B = func_80048E2C(0, 1);
            p->unk4E = p->unk4C = angle << 5;
            if (D_800A5080) {
                p->unk50 = func_80048E2C(0, 7);
            } else {
                p->unk50 = colour;
            }
            p->unk51 = func_80048E2C(0, 2);
            sDropCount++;
        }
    }
}

void func_m008_00334(f32 x, f32 y, f32 z, s32 arg3, s32 arg4) {
    M008Hit *h;
    s32 i;

    if (!sLoaded) {
        func_m008_00000();
    }
    i = 0;
    if (sHitCount < 5) {
        for (h = sHits; i < 5; i++, h++) {
            if (h->state == 0) {
                break;
            }
        }
        if (i < 5) {
            h->x = x;
            h->y = y;
            h->z = z;
            h->unk14 = arg3;
            h->unk15 = arg4;
            h->state = 1;
            h->unkC = 0.0f;
            sHitCount++;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m008/ro", func_m008_0040C);

INCLUDE_ASM("asm/nonmatchings/modules/m008/ro", func_m008_00AB4);

INCLUDE_ASM("asm/nonmatchings/modules/m008", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m008", bss);
