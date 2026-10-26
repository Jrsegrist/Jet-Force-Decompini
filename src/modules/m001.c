/* Runlink module 1: a pool of 32 small particles, each drawn as two textured quads. */
#include "common.h"

/* The game's 10-byte vertex and 16-byte textured triangle */
typedef struct M001Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} M001Vertex;

typedef struct M001Triangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    s16 uv[3][2];
} M001Triangle;

typedef struct M001Particle {
    u8 pad0[4];
    s16 unk4;
    u8 unk6; /* 0: free */
    u8 unk7;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 unk28;
    s32 unk2C;
    M001Vertex verts[2][4];
} M001Particle;

extern void *func_80056798(s32, s32);
extern void func_80056C14(void *);
extern u8 *func_80055694(s32);
extern void func_80055A00(void *);
void func_m001_002A0(M001Particle *, s32);

static M001Triangle D_m001_00C30[2] = {
    { 0x40, 0, 1, 3, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 0, 3, 2, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};
static void *D_m001_00C50 = NULL; /* loaded with func_80056798 */
static void *D_m001_00C54 = NULL; /* loaded with func_80056798 */
static u8 *D_m001_00C58 = NULL;   /* texture, loaded with func_80055694 */
static s32 D_m001_00C5C = 0;      /* particles in use */

static M001Particle D_m001_00C70[32];
static s32 D_m001_01C70; /* set once the resources are loaded */

static void func_m001_00000(void) {
    s32 i;
    s32 j;
    s32 k;
    M001Particle *p;
    s32 w;
    s32 h;

    D_m001_00C50 = func_80056798(0x48, 0);
    D_m001_00C54 = func_80056798(0x45, 0);
    D_m001_00C58 = func_80055694(0x1AC);
    for (i = 0, p = D_m001_00C70; i < 32; i++, p++) {
        p->unk6 = 0;
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 4; k++) {
                p->verts[j][k].r = 0xFF;
                p->verts[j][k].g = 0xFF;
                p->verts[j][k].b = 0xFF;
                p->verts[j][k].a = 0xFF;
            }
        }
    }
    D_m001_00C5C = 0;
    if (D_m001_00C58 != NULL) {
        w = (D_m001_00C58[0] - 1) << 5;
        h = (D_m001_00C58[1] - 1) << 5;
        D_m001_00C30[0].uv[1][0] = w;
        D_m001_00C30[0].uv[2][0] = w;
        D_m001_00C30[0].uv[2][1] = h;
        D_m001_00C30[1].uv[1][0] = w;
        D_m001_00C30[1].uv[1][1] = h;
        D_m001_00C30[1].uv[2][1] = h;
    }
    D_m001_01C70 = 1;
}

void func_m001_00120(void) {
    if (D_m001_00C50 != NULL) {
        func_80056C14(D_m001_00C50);
    }
    if (D_m001_00C54 != NULL) {
        func_80056C14(D_m001_00C54);
    }
    if (D_m001_00C58 != NULL) {
        func_80055A00(D_m001_00C58);
    }
    D_m001_01C70 = 0;
}

M001Particle *func_m001_00188(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, f32 unk20, f32 unk24) {
    M001Particle *p;
    s32 i;

    if (!D_m001_01C70) {
        func_m001_00000();
    }
    if (D_m001_00C5C >= 32) {
        return NULL;
    }
    for (i = 0, p = D_m001_00C70; i < 32; i++, p++) {
        if (p->unk6 == 0) {
            break;
        }
    }
    if (i < 32) {
        p->unk4 = 0;
        p->unk6 = 1;
        p->unk7 = 0x80;
        p->unk8 = x;
        p->unkC = y;
        p->unk10 = z;
        p->unk14 = vx;
        p->unk18 = vy;
        p->unk1C = vz;
        p->unk20 = unk20;
        p->unk24 = unk24;
        p->unk2C = 1;
        p->unk28 = 0.0f;
        D_m001_00C5C++;
    }
    return p;
}

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_002A0);

void func_m001_00600(s32 updateRate) {
    M001Particle *p;
    s32 i;

    if (D_m001_00C5C != 0) {
        p = D_m001_00C70;
        for (i = 0; i < 32; i++, p++) {
            if (p->unk6 != 0) {
                func_m001_002A0(p, updateRate);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00678);

INCLUDE_ASM("asm/nonmatchings/modules/m001", func_m001_00978);

/* The module's shutdown function (runlink fini) */
void func_m001_00C0C(void) {
    func_m001_00120();
}
#pragma weak __rl_fini = func_m001_00C0C

INCLUDE_ASM("asm/nonmatchings/modules/m001", rodata);
