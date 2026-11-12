/*
 * Runlink module 126: an object effect of four rotating beams, each with
 * its own light. Init builds (once) a shared table of textured triangles for
 * a ring of 16 vertices in three bands, sets each beam's angles, heights,
 * scale and light, control animates them, and the draw function draws them.
 */
#include "module.h"
#include "gfx.h"

/* What init gets */
typedef struct M126Setup {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 unkA[4];
    /* 0x12 */ s16 unk12[4];
    /* 0x1A */ u8 unk1A[4];
    /* 0x1E */ u8 unk1E[4];
    /* 0x22 */ s16 height;
    /* 0x24 */ u16 scaleA; /* percent */
    /* 0x26 */ u16 scaleB; /* percent */
} M126Setup;

/* The colour cycle of 55130.c (colour_cycle_init) */
typedef struct M126ColourCycle {
    /* 0x0 */ u8 pad0[8];
    /* 0x8 */ u8 r; /* the current colour */
    /* 0x9 */ u8 g;
    /* 0xA */ u8 b;
    /* 0xB */ u8 padB[0x10 - 0xB];
} M126ColourCycle;

/* Kept in obj->unk68 */
typedef struct M126State {
    /* 0x000 */ Vertex verts[2][4][48]; /* two buffers of each beam's three bands */
    /* 0xF00 */ s16 unkF00[4];
    /* 0xF08 */ s16 unkF08[4];
    /* 0xF10 */ s16 unkF10[4];
    /* 0xF18 */ s16 unkF18[4];
    /* 0xF20 */ f32 unkF20[4];
    /* 0xF30 */ s32 lights[4];
    /* 0xF40 */ u8 unkF40[4];
    /* 0xF44 */ u8 unkF44[4];
    /* 0xF48 */ M126ColourCycle cycles[4];
    /* 0xF88 */ s16 angles[4];
    /* 0xF90 */ s16 halfHeight;
    /* 0xF92 */ u8 padF92[2];
    /* 0xF94 */ f32 scaleA;
    /* 0xF98 */ f32 scaleB;
    /* 0xF9C */ f32 unkF9C[4];
    /* 0xFAC */ f32 unkFAC[4];
    /* 0xFBC */ f32 unkFBC;
    /* 0xFC0 */ u8 padFC0[0xFC8 - 0xFC0];
    /* 0xFC8 */ u8 buffer; /* which of verts the beams use */
    /* 0xFC9 */ u8 unkFC9;
    /* 0xFCA */ s16 unkFCA;
} M126State;

extern void colour_cycle_init(void *arg0, s32 arg1);
extern s32 func_800210A4(Object *obj, LightParams *params); /* adds a light */

/*
 * NON_MATCHING: init. The code matches except that the ROM keeps its 0.2f
 * (a literal, the first word of .rodata) in a saved register, where this
 * version reads it from sConsts after each call (frame 8 bytes smaller, f22
 * not saved), and the order of two increments in the triangle loop. With a
 * literal, IDO places it after sConsts; the literal works once control and
 * draw are C too. The triangle loop is written with a break so that IDO
 * does not unroll it (the ROM's isn't).
 */
#ifdef NON_MATCHING
static u8 sTrisMade = FALSE;
static u8 sControlData[8] = { 0x40, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0x05 }; /* control passes it to transition_begin */
/* the texture rows of the three bands (top, bottom) */
s16 D_m126_0125C[5][2] = { { 0, 0xC0 }, { 0xC0, 0x100 }, { 0x100, 0x1E0 } };

static Triangle sTris[3][16];

/*
 * The module's .rodata: init's 0.2f, then the constants of the control and
 * draw functions (still assembly).
 */
static const f32 sConsts[] = { 0.2f, 100000000.0f, 0.2f, 0.1f, -0.4f, -0.4f, 0.4f, 0.4f, 0.01f, 4.8f };

void func_m126_00000(Object *obj, M126Setup *setup) {
    s16 angle;
    s32 i;
    s32 j;
    s32 v;
    s32 u;
    s16 unused;
    s16 *row;
    Triangle *tri;
    LightParams light;
    M126State *s;

    s = obj->unk68;
    angle = 0;
    if (!sTrisMade) {
        tri = sTris[0];
        for (i = 0; i < 3; i++) {
            row = D_m126_0125C[i];
            u = 0x7E0;
            v = 0;
            unused = 0;
            j = 0;
            while (TRUE) {
                tri[0].flags = 0x40;
                tri[0].vi0 = v;
                tri[0].vi1 = (v + 2) % 16;
                tri[0].vi2 = (v + 1) % 16;
                tri[0].uv[0][0] = u;
                tri[0].uv[0][1] = row[0];
                tri[0].uv[1][0] = u - 0xFC;
                tri[0].uv[1][1] = row[0];
                tri[0].uv[2][0] = u;
                tri[0].uv[2][1] = row[1];
                tri[1].flags = 0x40;
                tri[1].vi0 = (v + 2) % 16;
                tri[1].vi1 = (v + 1) % 16;
                tri[1].vi2 = (v + 3) % 16;
                tri[1].uv[0][0] = u - 0xFC;
                tri[1].uv[0][1] = row[0];
                tri[1].uv[1][0] = u;
                tri[1].uv[1][1] = row[1];
                tri[1].uv[2][0] = u - 0xFC;
                tri[1].uv[2][1] = row[1];
                unused += 0x2000;
                v += 2;
                u -= 0xFC;
                tri += 2;
                if (++j == 8) {
                    break;
                }
            }
        }
        sTrisMade = TRUE;
    }
    s->halfHeight = setup->height >> 1;
    s->scaleA = (obj->header->scale * setup->scaleA) / 100.0f;
    s->scaleB = (obj->header->scale * setup->scaleB) / 100.0f;
    s->unkFC9 = 0;
    s->unkFBC = 0.5f;
    for (i = 0; i < 4; i++) {
        s->angles[i] = angle;
        s->unkF00[i] = -0x4000;
        s->unkF08[i] = setup->unkA[i];
        s->unkF10[i] = 0;
        angle += 0x1000;
        s->unkF18[i] = setup->unk12[i];
        s->unkF44[i] = setup->unk1A[i];
        s->unkF40[i] = 0;
        s->unkF20[i] = (s16) (obj->trans.y_position + setup->height);
        s->unkFAC[i] = 1.0f;
        s->unkF9C[i] = s->scaleA - sConsts[0];
        colour_cycle_init(&s->cycles[i], setup->unk1E[i]);
        light.type = 1;
        light.unk1 = 3;
        light.flags = 0x40;
        light.unk3 = -1;
        light.unk4 = 0;
        light.unk6 = 0;
        light.unk8 = 0;
        light.unkA = 0;
        light.a = 0xFF;
        light.radius = 120;
        light.radius2 = 120;
        light.unk14 = -1;
        light.unk16 = 0;
        light.unk17 = 0;
        s->lights[i] = func_800210A4(NULL, &light);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m126/ro", func_m126_00000);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m126/ro", func_m126_003D0);

extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 texOffset); /* sets a texture */
extern void func_8004959C(MtxF *mtx, f32 x, f32 y, f32 z);                 /* translation */
extern void func_80049874(f32 x, f32 y, f32 z, MtxF *mtx);                 /* scale */
extern void func_80049AE8(s32 angle, MtxF *mtx);                           /* rotation */
extern void mtx_model_push(Gfx **dl, Mtx **mtx, MtxF *mtxf);
extern void mtx_pop(Gfx **dl);
extern void func_80054EA0(u32 *width, u32 *height);
extern void menu_fill_box(Gfx **dl, s32 x, s32 y, s32 width, s32 height, u32 colour);
extern Triangle rl_m126_012A0[3][16];

/* Draws the beams, furthest first, then fades the screen while the effect starts */
void func_m126_00E74(Gfx **dList, Mtx **mtxList, Vertex **vtxList, Object *obj) {
    Gfx *dl;
    Mtx *mtx;
    Vertex *vtx;
    MtxF mtxf;
    M126State *s;
    s32 i;
    s32 j;
    s32 swapped;
    s32 alpha;
    s32 tmp;
    s32 order[4];
    Vertex *verts;
    f32 y;
    u32 width;
    u32 height;

    dl = *dList;
    mtx = *mtxList;
    vtx = *vtxList;
    s = obj->unk68;
    if (s->unkFCA < 0x164) {
        func_80055C00(&dl, obj->modelInstances[0], 0x207, 0);
        y = obj->trans.y_position;
        for (i = 0; i < 4; i++) {
            order[i] = i;
        }
        do {
            swapped = FALSE;
            for (j = 0; j < 3; j++) {
                tmp = order[j + 1];
                if (s->unkF9C[tmp] < s->unkF9C[order[j]]) {
                    swapped = order[j];
                    order[j] = tmp;
                    order[j + 1] = swapped;
                    swapped = TRUE;
                }
            }
        } while (swapped);
        for (i = 0; i < 4; i++) {
            s32 beam;

            beam = order[i];
            verts = s->verts[0][beam] + s->buffer * (4 * 48);
            if (s->unkF40[beam] != 0) {
                Triangle *tris;

                obj->trans.rotation.y_rotation = s->unkF10[beam];
                obj->trans.y_position = s->unkF20[beam] + y;
                func_8004959C(&mtxf, obj->trans.x_position, s->unkF20[beam], obj->trans.z_position);
                func_80049874(s->unkF9C[beam], s->scaleB, s->unkF9C[beam], &mtxf);
                func_80049AE8(s->unkF10[beam], &mtxf);
                mtx_model_push(&dl, &mtx, &mtxf);
                gDPPipeSync(dl++);
                gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, s->unkF40[beam]);
                gDPSetEnvColor(dl++, s->cycles[beam].r, s->cycles[beam].g, s->cycles[beam].b, s->unkF40[beam]);
                tris = rl_m126_012A0[0];
                for (j = 0; j < 3; j++) {
                    gSPVertexJFG(dl++, (u8 *) verts + 0x80000000, 17, 0);
                    gSPPolygon(dl++, (u8 *) tris + 0x80000000, 16, 1);
                    verts += 16;
                    tris += 16;
                }
            }
        }
        mtx_pop(&dl);
        obj->trans.y_position = y;
        func_80054EA0(&width, &height);
        if (!(s->unkFBC > 4.8f)) {
            alpha = s->unkFBC * 255.0f / 10.0f;
            menu_fill_box(&dl, 0, 0, width, height, alpha | ~0xFF);
        }
    }
    *dList = dl;
    *mtxList = mtx;
    *vtxList = vtx;
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m126", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m126", bss);
#endif
