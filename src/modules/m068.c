/*
 * Runlink module 68: a large boss and the objects it uses (still mostly
 * assembly).
 */
#include "module.h"
#include "gfx.h"

typedef struct M068Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 xRotation;
    /* 0x0C */ s16 yRotation;
} M068Setup;


/* The boss's state (obj->unk68), as far as used here */
typedef struct M068State {
    /* 0x00 */ u8 pad0[0x58];
    /* 0x58 */ s32 sound58;
    /* 0x5C */ s32 sound5C;
    /* 0x60 */ u8 pad60[0x6C - 0x60];
    /* 0x6C */ s32 sound6C;
    /* 0x70 */ s32 sound70;
    /* 0x74 */ u8 pad74[0x76 - 0x74];
    /* 0x76 */ s16 hitByPlayer;
} M068State;

/* A piece drawn with its own matrix (obj->unk68) */
typedef struct M068Piece {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ MtxF *base;
} M068Piece;

/* A shot's set-up and state */
typedef struct M068ShotSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ f32 speed;
    /* 0x10 */ Vec3f velocity;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ s32 unk20;
} M068ShotSetup;

typedef struct M068Shot {
    /* 0x00 */ f32 speed;
    /* 0x04 */ f32 radius;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ f32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s16 unk14;
} M068Shot;

/* A beam's set-up: it flies at a point */
typedef struct M068BeamSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 targetX;
    /* 0x0C */ s16 targetY;
    /* 0x0E */ s16 targetZ;
    /* 0x10 */ f32 speed;
    /* 0x14 */ s32 unk14;
} M068BeamSetup;

/* A flying piece's set-up and state */
typedef struct M068PieceSetup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 active;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 zRotation;
    /* 0x10 */ Vec3f velocity;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 speed;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ MtxF *mtx;
} M068PieceSetup;

typedef struct M068Flyer {
    /* 0x0 */ f32 unk0;
    /* 0x4 */ f32 unk4;
    /* 0x8 */ f32 unk8;
    /* 0xC */ s16 timer;
    /* 0xE */ s16 unkE;
    /* 0x10 */ MtxF *mtx;
} M068Flyer;

/* A strip of triangles drawn between points (two kinds share the start) */
typedef struct M068Strip {
    /* 0x000 */ Triangle tris[16];
    /* 0x100 */ Vertex verts[2][72];
    /* 0x6A0 */ s16 a[2];
    /* 0x6A4 */ s16 b[2];
    /* 0x6A8 */ s16 c[2];
    /* 0x6AC */ u8 pad6AC[0x6B0 - 0x6AC];
    /* 0x6B0 */ Vec3f pos;
    /* 0x6BC */ u8 pad6BC[0x6C0 - 0x6BC];
    /* 0x6C0 */ s32 unk6C0;
    /* 0x6C4 */ s32 unk6C4;
} M068Strip;

typedef struct M068Strip2 {
    /* 0x000 */ u8 pad0[0x6B8];
    /* 0x6B8 */ s16 unk6B8;
    /* 0x6BA */ u8 pad6BA[0x6BC - 0x6BA];
    /* 0x6BC */ s32 unk6BC;
} M068Strip2;

typedef struct M068StripSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ s16 a[2];
    /* 0x10 */ s16 b[2];
    /* 0x14 */ s16 c[2];
    /* 0x18 */ Vec3f pos;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
} M068StripSetup;

typedef struct M068StripSetup2 {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s16 a[2];
    /* 0x14 */ s16 b[2];
    /* 0x18 */ s16 c[2];
    /* 0x1C */ s32 unk1C;
} M068StripSetup2;

/* A model drawn from a display list (*obj->header->unkAC) */
typedef struct M068DrawModel {
    /* 0x00 */ struct M068DrawLists *lists;
    /* 0x04 */ void *vertices;
    /* 0x08 */ u8 pad8[0xA - 0x8];
    /* 0x0A */ u8 drawn;
    /* 0x0B */ s8 frame;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ void *frames[1];
} M068DrawModel;

typedef struct M068DrawLists {
    /* 0x00 */ u8 pad0[0x74];
    /* 0x74 */ Gfx *dl74;
    /* 0x78 */ Gfx *dl78;
} M068DrawLists;

typedef struct M068Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ M068DrawModel **models;
} M068Header;

/* The colour a piece is drawn in (its obj->unk68) */
typedef struct M068Glow {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ f32 scale;
    /* 0x18 */ u8 pad18[0x6C - 0x18];
    /* 0x6C */ u8 r;
    /* 0x6D */ u8 g;
    /* 0x6E */ u8 b;
} M068Glow;

/* A command with the 0xBF opcode (a model's frame and vertices) */
#define gSPModelFrameJFG(pkt, frame, verts)                                             \
    {                                                                                  \
        Gfx *_g = (Gfx *) (pkt);                                                       \
        _g->words.w0 = ((u32) OS_K0_TO_PHYSICAL(frame) & 0xFFFFFF) | 0xBF000000;       \
        _g->words.w1 = (u32) (verts);                                                  \
    }

/* The same command, clearing the frame */
#define gSPModelFrameEndJFG(pkt)                                                        \
    {                                                                                  \
        Gfx *_g = (Gfx *) (pkt);                                                       \
        _g->words.w0 = 0xBF000000;                                                     \
        _g->words.w1 = 0;                                                              \
    }

/* A sprite with its colour */
typedef struct M068Sprite {
    /* 0x0 */ u8 pad0[8];
    /* 0x8 */ u8 r;
    /* 0x9 */ u8 g;
    /* 0xA */ u8 b;
    /* 0xB */ u8 a;
    /* 0xC */ u8 unkC;
    /* 0xD */ u8 unkD;
} M068Sprite;

extern u8 rl_m068_12630[];
extern void func_8000A300(Gfx **dl, s32 arg1, void *model, void *data, f32 *pos, void *arg5, f32 arg6, s32 arg7,
                          s32 colour, s32 arg9);
extern void func_8005600C(s32 r, s32 g, s32 b, s32 a, s32 arg4, s32 arg5);
extern void render_sprite_billboard_at(Gfx **dl, s32 arg1, s32 arg2, M068Sprite *sprite, s32 x, s32 y, s32 z, s32 arg7, f32 scale,
                          f32 arg9, s32 arg10, s32 alpha);
extern void func_80056064(void);

extern void func_80049BA0(MtxF *mtx); /* identity */
extern void func_80049F08(Object *obj, MtxF *out);
extern void func_80055B80(Gfx **dl);
extern void mtx_pop(Gfx **dl);

/* A block's set-up */
typedef struct M068BlockSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ u8 scale; /* in 64ths, at least 10 */
} M068BlockSetup;

typedef struct M068Model {
    /* 0x00 */ u8 pad0[0x94];
    /* 0x94 */ s16 radius;
} M068Model;

/* The object field this module sets at 0x34 */
typedef struct M068Object {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ f32 radius;
} M068Object;

extern f32 func_80097140(f32 x); /* square root */
extern void func_80003614(s32 handle); /* stops a positioned sound */
extern void func_80049DC0(Object *obj, MtxF *out);
extern void func_80048C7C(MtxF *a, MtxF *b, MtxF *out); /* out = a * b */
extern void mtx_model_push(Gfx **dl, s32 arg1, MtxF *mtx);

/* The module's .data (still assembly): sounds playing */
extern s32 rl_m068_124EC;
extern s32 rl_m068_12600;
extern s32 rl_m068_12614;
extern s32 rl_m068_12618;

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_00000);

void func_m068_00014(Object *obj, M068Setup *setup) {
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0002C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0003C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_004D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0092C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_00C70);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0104C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_01330);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_01628);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_017EC);

/* Stops the boss's sounds */
void func_m068_023D4(Object *obj) {
    M068State *s = obj->unk68;

    if (s->sound6C != 0) {
        func_80003614(s->sound6C);
    }
    if (s->sound58 != 0) {
        func_80003614(s->sound58);
    }
    if (s->sound70 != 0) {
        func_80003614(s->sound70);
    }
    if (s->sound5C != 0) {
        func_80003614(s->sound5C);
    }
}

void func_m068_0246C(void) {
    if (rl_m068_12600 != 0) {
        func_80003614(rl_m068_12600);
    }
    if (rl_m068_12614 != 0) {
        func_80003614(rl_m068_12614);
    }
    if (rl_m068_12618 != 0) {
        func_80003614(rl_m068_12618);
    }
    if (rl_m068_124EC != 0) {
        func_80003614(rl_m068_124EC);
    }
}
#pragma weak __rl_fini = func_m068_0246C

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_024EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_039D4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03A78);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03C70);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03D58);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03E24);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03F0C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03FD8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_040EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0447C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04590);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04600);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_046EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_047F8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04878);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_048C0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04938);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04A60);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05028);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05100);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_053AC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_054A0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05570);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05668);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0577C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05964);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_059F0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05C8C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05F60);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06180);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06244);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06310);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06590);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_067B4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06AE0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06E14);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0714C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0758C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_078F4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_07D68);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_083C8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_08674);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0873C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_087E0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_088DC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_09088);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_09218);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_092F0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_093B8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0A208);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0A5D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0ADC4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0B4D0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BB4C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BDC4);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BE14);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C7DC);

void func_m068_0C874(Object *obj, M068ShotSetup *setup) {
    M068Shot *s = obj->unk68;
    ObjectHeaderUnk50 *hit;

    obj->velocity.x = setup->velocity.x;
    obj->velocity.y = setup->velocity.y;
    obj->velocity.z = setup->velocity.z;
    s->speed = setup->speed;
    s->unkC = setup->unk1C / setup->speed;
    s->unk14 = setup->unkA;
    obj->opacity = 55;
    s->unk10 = setup->unk20;
    hit = obj->header->unk50;
    s->radius = hit->radius * obj->trans.scale;
    func_80079D1C(obj, 0x10, 1, 4, 0, 0, 0, hit->radius, hit->radius, -1.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C944);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0CDD8);

void func_m068_0CF3C(Object *obj, M068BeamSetup *setup) {
    f32 len;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 *beam = obj->unk68;

    *beam = setup->unk14;
    dx = setup->targetX - obj->trans.x_position;
    dy = setup->targetY - obj->trans.y_position;
    dz = setup->targetZ - obj->trans.z_position;
    len = func_80097140(dx * dx + dy * dy + dz * dz);
    dx /= len;
    dy /= len;
    dz /= len;
    obj->velocity.x = setup->speed * dx;
    obj->velocity.y = setup->speed * dy;
    obj->velocity.z = setup->speed * dz;
    func_80079D1C(obj, 0x100, 1, 4, 0, 0, 0, 10.0f, 10.0f, -1.0f, 1.0f, 0x11);
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D05C);

#ifdef NON_MATCHING
/* Registers: every t-register after the model lookup is one lower than the ROM's */
void func_m068_0D3D8(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    s16 *state = obj->unk68;
    M068Sprite *sprite;
    M068Sprite *colour;

    if (state[3] == 0) {
        func_8000A300(dl, arg2, ((void **) obj->modelInstances)[obj->modelIndex], rl_m068_12630,
                      &obj->trans.x_position, (u8 *) obj->interactObj + 0x18, 8.0f, -1, 0x8C0EF4FF, 0x200);
        sprite = *(M068Sprite **) ((M068Header *) obj->header)->models;
        func_8005600C(sprite->r, sprite->g, sprite->b, sprite->a, sprite->unkC, sprite->unkD);
        render_sprite_billboard_at(dl, arg1, arg2, sprite, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 0,
                      2.0f, 0.0f, 12, 255);
        func_80056064();
    } else {
        colour = ((M068Sprite **) ((M068Header *) obj->header)->models)[0];
        sprite = ((M068Sprite **) ((M068Header *) obj->header)->models)[1];
        func_8005600C(colour->r, colour->g, colour->b, colour->a, colour->unkC, colour->unkD);
        render_sprite_billboard_at(dl, arg1, arg2, sprite, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 0,
                      2.0f, obj->unk28, 12, 100);
        func_80056064();
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D3D8);
#endif

#ifdef NON_MATCHING
/* The vertex clear loop: IDO unrolls it twice here, the ROM's is not unrolled */
void func_m068_0D65C(Object *obj, M068StripSetup *setup) {
    M068Strip *m = obj->unk68;
    Triangle *tri;
    s32 i;
    s32 j;
    s32 k;
    s32 v;
    s32 prev;

    m->unk6C0 = setup->unk24;
    m->unk6C4 = setup->unk28;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 72; j++) {
            m->verts[i][j].x = 0;
            m->verts[i][j].y = 0;
            m->verts[i][j].z = 0;
            m->verts[i][j].r = 255;
            m->verts[i][j].g = 255;
            m->verts[i][j].b = 255;
            m->verts[i][j].a = 255;
        }
    }
    tri = m->tris;
    prev = 0;
    k = 0;
    for (i = 64; i < 576; i += 64) {
        v = (i / 8 - 1) << 5;
        tri[0].flags = 0x40;
        tri[0].vi0 = k;
        tri[0].vi1 = k + 1;
        tri[0].vi2 = k + 2;
        tri[0].uv[0][0] = 480;
        tri[0].uv[0][1] = prev;
        tri[0].uv[1][0] = 0;
        tri[0].uv[1][1] = prev;
        tri[0].uv[2][0] = 480;
        tri[0].uv[2][1] = v;
        tri[1].flags = 0x40;
        tri[1].vi0 = k + 1;
        tri[1].vi1 = k + 3;
        tri[1].vi2 = k + 2;
        tri[1].uv[0][0] = 0;
        tri[1].uv[0][1] = prev;
        tri[1].uv[1][0] = 0;
        tri[1].uv[1][1] = v;
        tri[1].uv[2][0] = 480;
        tri[1].uv[2][1] = v;
        tri += 2;
        prev = v;
        k += 2;
    }
    for (i = 0; i < 2; i++) {
        m->a[i] = setup->a[i];
        m->b[i] = setup->b[i];
        m->c[i] = setup->c[i];
    }
    m->pos.x = setup->pos.x;
    m->pos.y = setup->pos.y;
    m->pos.z = setup->pos.z;
    func_80079D1C(obj, 0x100, 1, 4, 0, 0, 0, 5.0f, 5.0f, -1.0f, 1.0f, 0x11);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D65C);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0D828);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E148);

#ifdef NON_MATCHING
/* As 0D65C: the vertex clear loop is unrolled here */
void func_m068_0E520(Object *obj, M068StripSetup2 *setup) {
    M068Strip *m = obj->unk68;
    Triangle *tri;
    s32 i;
    s32 j;
    s32 k;
    s32 v;
    s32 prev;

    ((M068Strip2 *) m)->unk6B8 = setup->unk1C;
    ((M068Strip2 *) m)->unk6BC = setup->unkC;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 72; j++) {
            m->verts[i][j].x = 0;
            m->verts[i][j].y = 0;
            m->verts[i][j].z = 0;
            m->verts[i][j].r = 255;
            m->verts[i][j].g = 255;
            m->verts[i][j].b = 255;
            m->verts[i][j].a = 255;
        }
    }
    tri = m->tris;
    prev = 0;
    k = 0;
    for (i = 64; i < 576; i += 64) {
        v = (i / 8 - 1) << 5;
        tri[0].flags = 0x40;
        tri[0].vi0 = k;
        tri[0].vi1 = k + 1;
        tri[0].vi2 = k + 2;
        tri[0].uv[0][0] = 480;
        tri[0].uv[0][1] = prev;
        tri[0].uv[1][0] = 0;
        tri[0].uv[1][1] = prev;
        tri[0].uv[2][0] = 480;
        tri[0].uv[2][1] = v;
        tri[1].flags = 0x40;
        tri[1].vi0 = k + 1;
        tri[1].vi1 = k + 3;
        tri[1].vi2 = k + 2;
        tri[1].uv[0][0] = 0;
        tri[1].uv[0][1] = prev;
        tri[1].uv[1][0] = 0;
        tri[1].uv[1][1] = v;
        tri[1].uv[2][0] = 480;
        tri[1].uv[2][1] = v;
        tri += 2;
        prev = v;
        k += 2;
    }
    for (i = 0; i < 2; i++) {
        m->a[i] = setup->a[i];
        m->b[i] = setup->b[i];
        m->c[i] = setup->c[i];
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E520);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E668);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0ED10);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0EF2C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0EF70);

void func_m068_0F2FC(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068DrawModel *model;
    M068DrawLists *lists;
    M068Glow *glow;
    MtxF objMtx;
    MtxF mtx;

    model = *((M068Header *) obj->header)->models;
    glow = obj->unk68;
    lists = model->lists;
    func_80049BA0(&mtx);
    mtx[0][0] = glow->scale;
    mtx[2][2] = glow->scale;
    func_80049F08(obj, &objMtx);
    func_80048C7C(&mtx, &objMtx, &mtx);
    gDPSetPrimColor((*dl)++, 0, 0, 0xFA, 0x3C, 0x0A, obj->opacity);
    mtx_model_push(dl, arg1, &mtx);
    gSPModelFrameJFG((*dl)++, model->frames[model->frame], OS_K0_TO_PHYSICAL(model->vertices));
    gSPDisplayList((*dl)++, OS_K0_TO_PHYSICAL(lists->dl78));
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gSPModelFrameEndJFG((*dl)++);
    mtx_pop(dl);
    func_80055B80(dl);
    model->drawn = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F478);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F7C8);

/* Called when something hits the boss: notes a hit by the player */
void func_m068_10734(Object *hitter, Object *obj, s32 arg2) {
    M068State *s = obj->unk68;

    if (hitter->behaviorId == 1) {
        s->hitByPlayer = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10758);

void func_m068_108C8(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068DrawModel *model;
    M068DrawLists *lists;
    M068Glow *glow;
    MtxF mtx;

    model = *((M068Header *) obj->header)->models;
    glow = obj->unk68;
    lists = model->lists;
    func_80049DC0(obj, &mtx);
    gDPSetPrimColor((*dl)++, 0, 0, glow->r, glow->g, glow->b, 255);
    mtx_model_push(dl, arg1, &mtx);
    gSPModelFrameJFG((*dl)++, model->frames[model->frame], OS_K0_TO_PHYSICAL(model->vertices));
    gSPDisplayList((*dl)++, OS_K0_TO_PHYSICAL(lists->dl74));
    gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    gSPModelFrameEndJFG((*dl)++);
    mtx_pop(dl);
    func_80055B80(dl);
    model->drawn = 0;
}

void func_m068_10A28(Object *obj, M068PieceSetup *setup) {
    M068Flyer *f;

    if (setup->active) {
        f = obj->unk68;
        obj->velocity.x = setup->velocity.x * setup->speed;
        obj->velocity.y = setup->velocity.y * setup->speed;
        obj->velocity.z = setup->velocity.z * setup->speed;
        f->unk0 = setup->unk28;
        f->unk4 = setup->unk1C;
        f->unk8 = setup->unk20;
        f->timer = func_80048E2C(8, 16);
        f->unkE = 1;
        obj->trans.x_position = setup->x;
        obj->trans.y_position = setup->y;
        obj->trans.z_position = setup->z;
        obj->trans.rotation.y_rotation = setup->yRotation;
        obj->trans.rotation.x_rotation = setup->xRotation;
        obj->trans.rotation.z_rotation = setup->zRotation;
        obj->trans.flags &= ~0x400;
    } else {
        obj->trans.flags |= 0x400;
    }
}

extern void func_8005F5A0(f32 arg0);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* moves the object */

#ifdef NON_MATCHING
/*
 * A piece flying off: it falls and spins until it lands or its time runs out,
 * then goes. Frame and code match; the piece pointer gets v1 where the ROM
 * has t0 (and the registers after it move along).
 */
void func_m068_10B58(Object *obj, s32 updateRate) {
    UNUSED s32 pad;
    HitResult hit;
    M068Flyer *f;
    Vec3f from;
    f32 rate;
    UNUSED s32 pad2;
    s32 end;

    f = obj->unk68;
    if (f->unkE == 0) {
        return;
    }
    rate = updateRate;
    end = 0;
    f->unk4 -= rate / 60.0f;
    if (f->unk4 <= 0.0f) {
        end = 1;
    } else {
        if (f->unk4 <= f->unk8) {
            obj->trans.rotation.x_rotation += (f->timer * updateRate) << 7;
            obj->particleEmittersEnabled = 1;
            func_8005F5AC(obj, updateRate);
        }
        if (obj->unk7C == NULL) {
            from.x = obj->trans.x_position;
            from.y = obj->trans.y_position;
            from.z = obj->trans.z_position;
            func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
            obj->velocity.y -= rate * f->unk0;
            if (func_80018808(obj, &from, (Vec3f *) &obj->trans.x_position, &hit, 0, 0)) {
                func_80009A04(obj, hit.pos.x - obj->trans.x_position, hit.pos.y - obj->trans.y_position,
                              hit.pos.z - obj->trans.z_position);
                obj->trans.rotation.z_rotation = obj->trans.rotation.x_rotation = obj->trans.rotation.y_rotation = 0;
                end = 2;
                f->unk4 = 0.0f;
            }
        }
    }
    if (end != 0) {
        switch (end) {
            case 1:
                obj->particleEmittersEnabled = 2;
                break;
            case 2:
                obj->particleEmittersEnabled = 2;
                if (rl_m068_124EC == 0) {
                    func_80003410(0x16, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4,
                                  &rl_m068_124EC);
                }
                break;
            default:
                obj->particleEmittersEnabled = 1;
                break;
        }
        func_8005F5A0(2.0f);
        func_8005F5AC(obj, updateRate);
        func_800079B8(obj);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_10B58);
#endif

void func_m068_10DD4(Object *obj, M068PieceSetup *setup) {
    M068Flyer *f;

    if (setup->active) {
        f = obj->unk68;
        obj->velocity.x = setup->velocity.x * setup->speed;
        obj->velocity.y = setup->velocity.y * setup->speed;
        obj->velocity.z = setup->velocity.z * setup->speed;
        f->unk0 = setup->unk28;
        f->unk4 = setup->unk1C;
        f->unk8 = setup->unk20;
        f->timer = func_80048E2C(8, 16);
        f->unkE = 1;
        obj->trans.x_position = setup->x;
        obj->trans.y_position = setup->y;
        obj->trans.z_position = setup->z;
        obj->trans.rotation.y_rotation = setup->yRotation;
        obj->trans.rotation.x_rotation = setup->xRotation;
        obj->trans.rotation.z_rotation = setup->zRotation;
        f->mtx = setup->mtx;
        (*f->mtx)[0][3] = 0.0f;
        (*f->mtx)[1][3] = 0.0f;
        (*f->mtx)[2][3] = 0.0f;
        (*f->mtx)[3][0] = 0.0f;
        (*f->mtx)[3][1] = 0.0f;
        (*f->mtx)[3][2] = 0.0f;
        (*f->mtx)[3][3] = 1.0f;
        if (obj->unk7C != NULL) {
            *obj->unk7C &= ~2;
        }
        obj->trans.flags &= ~0x400;
    } else {
        obj->trans.flags |= 0x400;
    }
}

/* Pushes a piece's matrix: its own transform times its base matrix */
void func_m068_10F90(Gfx **dl, s32 arg1, Object *obj) {
    M068Piece *p = obj->unk68;
    MtxF local;
    MtxF out;

    func_80049DC0(obj, &local);
    func_80048C7C(p->base, &local, &out);
    mtx_model_push(dl, arg1, &out);
}

void func_m068_10FE8(Object *obj, M068BlockSetup *setup) {
    f32 scale = setup->scale & 0xFF;
    M068Model *model;

    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale *= 0.015625f;
    obj->trans.scale = obj->header->scale * scale;
    obj->trans.rotation.y_rotation = setup->yRotation;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    model = ((M068Model **) obj->modelInstances)[obj->modelIndex];
    ((M068Object *) obj)->radius = model->radius * obj->trans.scale;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_110E0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", data);

INCLUDE_ASM("asm/nonmatchings/modules/m068", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m068", bss);

