/*
 * Runlink module 126: an object effect of four rotating beams, each with
 * its own light. Init builds (once) a shared table of textured triangles for
 * a ring of 16 vertices in three bands, sets each beam's angles, heights,
 * scale and light, control animates them, and the draw function draws them.
 * Family: other object behaviours (docs/runlink.md, "Module families").
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
    /* 0xFC0 */ Object *unkFC0; /* the nearest player or queued object */
    /* 0xFC4 */ s32 unkFC4;     /* a sound handle */
    /* 0xFC8 */ u8 buffer; /* which of verts the beams use */
    /* 0xFC9 */ u8 unkFC9;
    /* 0xFCA */ s16 unkFCA;
} M126State;

extern void colour_cycle_init(void *arg0, s32 arg1);
extern s32 func_800210A4(Object *obj, LightParams *params); /* adds a light */

/*
 * NON_MATCHING: init. ROM frame, one word long (245/244, 46 aligned diffs):
 * setup stays in a3 through the beams loop (stored to its home at each call)
 * where the ROM reloads it from its home for each height, and the triangle
 * loop's increments are scheduled differently. Its 0.2f is a literal now that
 * control and draw are C (it was a .rodata stand-in). The triangle loop is
 * written with a break so that IDO does not unroll it (the ROM's isn't).
 */
#ifdef NON_MATCHING
static u8 sTrisMade = FALSE;
static u8 sControlData[8] = { 0x40, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0x05 }; /* control passes it to transition_begin */
/* the texture rows of the three bands (top, bottom) */
s16 D_m126_0125C[5][2] = { { 0, 0xC0 }, { 0xC0, 0x100 }, { 0x100, 0x1E0 } };

static Triangle sTris[3][16];

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
                j++;
                unused += 0x2000;
                v += 2;
                u -= 0xFC;
                tri += 2;
                if (j == 8) {
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
        s->unkF9C[i] = s->scaleA - 0.2f;
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

/* A light as the beams move it (func_800210A4's) */
typedef struct M126Light {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ f32 x;
    /* 0x1C */ f32 y;
    /* 0x20 */ f32 z;
} M126Light;

extern Object **func_8000E464(s32 *count); /* the players */
extern Object **obj_queue_get(s32 *count);
extern void func_800212D0(M126Light *light); /* turns a light off */
extern void func_800212E4(M126Light *light); /* turns it on */
extern void light_colour_set(M126Light *light, u8 red, u8 green, u8 blue);
extern void light_intensity_set(M126Light *light, u8 intensity);
extern void func_80057D58(M126ColourCycle *cycle, s32 updateRate); /* steps a colour cycle */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void transition_begin(void *transition);
extern void sound_stop(s32 handle);
extern void func_80001500(s32 handle, f32 pitch);
extern u8 rl_m126_01254[]; /* control's transition (.data) */

/* A vertex's colour and alpha, all `c` */
#define VERTEX_COLOUR(v, c) do { (v)->r = (c); (v)->g = (c); (v)->b = (c); (v)->a = (c); } while (0)
/* Bounces `pos` back down off `top` / up off `bottom`, turning `step` round */
#define BOUNCE_DOWN(pos, top, step) do { (pos) = (top) - ((pos) - (top)); (step) = -(step); } while (0)
#define BOUNCE_UP(pos, bottom, step) do { (pos) = ((bottom) - (pos)) + (bottom); (step) = -(step); } while (0)

/*
 * Control: finds the nearest player or queued object once, swaps the vertex
 * buffers, then for each beam turns and lifts it (or lets it fall back),
 * fades it in, moves its light and builds its three bands of vertices; at the
 * end of the effect it lifts the object through its height and fades out.
 */
void func_m126_003D0(Object *obj, s32 updateRate) {
    M126State *s;
    Object **list;
    Object *o;
    s32 count;
    s32 k;
    s32 i;
    f32 bands[3][2];
    s32 n;
    s32 a;
    f32 wobble;
    f32 prevY;
    f32 vel;
    f32 rate;
    f32 dx;
    f32 minDist;
    f32 dz;
    f32 d;
    UNUSED f32 pad; /* (the ROM's frame has this word) */
    f32 top;
    f32 y;
    f32 step;
    Vertex *v;
    M126Light *light;

    s = obj->unk68;
    minDist = 100000000.0f;
    if (s->unkFC0 == NULL) {
        list = func_8000E464(&count);
        for (k = 0; k < 2; k++) {
            for (i = 0; i < count; i++) {
                o = list[i];
                dx = o->trans.x_position - obj->trans.x_position;
                step = o->trans.y_position - obj->trans.y_position;
                dz = o->trans.z_position - obj->trans.z_position;
                d = dx * dx + step * step + dz * dz;
                if (d < minDist) {
                    minDist = d;
                    s->unkFC0 = o;
                }
            }
            list = obj_queue_get(&count);
        }
    }
    s->buffer ^= 1;
    v = s->verts[s->buffer][0];
    rate = updateRate;
    for (i = 0; i < 4; i++) {
        s->angles[i] += updateRate << 8;
        light = (M126Light *) s->lights[i];
        if (updateRate < s->unkF44[i]) {
            s->unkF44[i] -= updateRate;
            func_800212D0(light);
            continue;
        }
        s->unkF44[i] = 0;
        prevY = s->unkF20[i];
        if (s->unkFAC[i] == 0.0f) {
            s->unkF00[i] += s->unkF08[i] * updateRate * s->unkFBC;
            s->angles[i] += updateRate << 8;
            s->unkF10[i] += s->unkF18[i] * updateRate;
            wobble = func_80049C34(s->angles[i]);
            s->unkF9C[i] = func_80049C30(s->unkF00[i]) * 0.2f + (s->scaleA + 0.2f * wobble);
            s->unkF20[i] = (s16) (func_80049C34(s->unkF00[i]) * s->halfHeight + (obj->trans.y_position + s->halfHeight));
        } else {
            s->unkF20[i] -= rate * s->unkFAC[i];
            s->unkFAC[i] += rate * 0.1f;
            if (s->unkF20[i] < obj->trans.y_position) {
                s->unkF20[i] = obj->trans.y_position;
                s->unkFAC[i] = 0.0f;
            }
        }
        for (count = 0; count < updateRate; count++) {
            s->unkF40[i] += (0xFF - s->unkF40[i]) >> 3;
        }
        s->unkF40[i] = 0xFF;
        func_80057D58(&s->cycles[i], updateRate);
        light->x = obj->trans.x_position;
        light->y = s->unkF20[i];
        light->z = obj->trans.z_position;
        func_800212E4(light);
        light_colour_set(light, s->cycles[i].r, s->cycles[i].g, s->cycles[i].b);
        light_intensity_set(light, s->unkF40[i]);
        bands[0][0] = 8.0f;
        bands[0][1] = 1;
        bands[1][0] = 1;
        bands[1][1] = -1;
        bands[2][0] = -1;
        bands[2][1] = -8.0f;
        vel = (s->unkF20[i] - prevY) / rate;
        if (vel < 0.0f) {
            if (vel < -0.4f) {
                vel = -0.4f;
            }
            bands[0][0] *= 1.0f - vel;
            bands[2][1] = -4.0f;
        } else {
            if (vel > 0.4f) {
                vel = 0.4f;
            }
            bands[2][1] *= 1.0f + vel;
            bands[0][0] = 4.0f;
        }
        for (k = 0; k < 3; k++) {
            a = 0;
            for (n = 0; n < 8; n++) {
                v->x = func_80049C34(a) * 15.0f;
                v->y = bands[k][0];
                v->z = func_80049C30(a) * 15.0f;
                VERTEX_COLOUR(v, 0xFF);
                v++;
                v->x = func_80049C34(a) * 15.0f;
                v->y = bands[k][1];
                v->z = func_80049C30(a) * 15.0f;
                VERTEX_COLOUR(v, 0xFF);
                v++;
                a += 0x2000;
            }
        }
    }
    s->unkFBC += 0.01f * rate;
    s->unkFCA += updateRate;
    if (s->unkFCA >= 0x164) {
        y = obj->trans.y_position;
        obj->trans.y_position += s->unkF20[0];
        obj->particleEmittersEnabled = 1;
        func_8005F5AC(obj, updateRate);
        obj->trans.y_position = y;
        if (!s->unkFC9) {
            transition_begin(rl_m126_01254);
            top = s->halfHeight * 2.0f + y;
            step = s->halfHeight * 2.0f / 5.0f;
            s->unkFC9 = 1;
            obj->particleEmittersEnabled = 3;
            for (i = 0; i < 20; i++) {
                func_8005F5AC(obj, 2);
                obj->trans.y_position += step;
                if (top < obj->trans.y_position) {
                    BOUNCE_DOWN(obj->trans.y_position, top, step);
                } else if (obj->trans.y_position < y) {
                    BOUNCE_UP(obj->trans.y_position, y, step);
                }
            }
        }
        if (s->unkFCA >= 0x168) {
            s->unkFCA = 0x168;
            s->unkFC0->trans.flags |= 0x400;
            if (s->unkFC4 != 0) {
                sound_stop(s->unkFC4);
            }
        }
    } else {
        if (s->unkFC4 == 0) {
            func_80001248(0xDC, (s32) &s->unkFC4);
        }
        func_80001500(s->unkFC4, s->unkFCA / 360.0f + 0.5f);
        if (s->unkFCA >= 0x12D) {
            func_80001248(0xDB, 0);
        }
    }
    if (s->unkFBC >= 5.0f) {
        s->unkFBC = 5.0f;
    }
}

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
