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
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ s32 flags; /* 0x20: (unknown) */
    /* 0x4C */ u8 pad4C[0x58 - 0x4C];
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
    /* 0x00 */ u8 pad0[0x92];
    /* 0x92 */ s16 unk92;
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

/* The module's .rodata and .bss (still assembly) */
extern f32 rl_m068_129F8;
extern f32 rl_m068_129FC;
extern u8 rl_m068_12A38[];

/* A part's set-up: only its turn */
typedef struct M068TurnSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 turn; /* in 1/64 turns */
} M068TurnSetup;

void func_m068_00000(Object *obj, M068TurnSetup *setup) {
    obj->trans.rotation.y_rotation = setup->turn << 2 << 8;
}

void func_m068_00014(Object *obj, M068Setup *setup) {
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
}

/* The tilt list (in the module's .bss) */
void *func_m068_0002C(s32 arg0) {
    return rl_m068_12A38;
}

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0003C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_004D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0092C);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_00C70);

/* What hit the boss */
typedef struct M068HitSource {
    /* 0x00 */ u8 pad0[0x2F];
    /* 0x2F */ u8 damage;
} M068HitSource;

/* A hit on the boss */
typedef struct M068Hit {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ M068HitSource *source;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ f32 x;
    /* 0x14 */ f32 y;
    /* 0x18 */ f32 z;
} M068Hit;

extern f32 func_80049BEC(f32 base, s32 exponent);
extern s32 func_80049C84(f32 x, f32 z); /* the angle of a vector */
extern s16 angle_lerp(s16 from, s16 to, f32 t);
extern void func_m008_001CC(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, s32 alpha, s32 arg7, f32 scale);
extern void func_m008_00334(f32 x, f32 y, f32 z, s32 alpha, s32 arg4);

/* The boss is hit: a shot of behaviour 0x9E throws sparks back along its path */
void func_m068_0104C(Object *obj, Object *shot, M068Hit *hit) {
    M068State *s;
    s32 i;
    Vec3f v;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 len;
    s32 alpha;
    s16 yaw;
    s16 pitch;
    s32 lo;
    s32 hi;
    UNUSED s32 pad;
    Vec3s rot;

    s = obj->unk68;
    if (hit->source->damage <= 0) {
        return;
    }
    if (shot->behaviorId == 0x9E) {
        alpha = obj->shading->unk0 * 255.0f;
        dx = -shot->velocity.x;
        dy = -shot->velocity.y;
        dz = -shot->velocity.z;
        len = func_80097140(dx * dx + dy * dy + dz * dz);
        dx /= len;
        dz /= len;
        dy /= len;
        yaw = func_80049C84(dx, dz) + 0x8000;
        pitch = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
        if (s->flags & 0x20) {
            lo = 1;
            hi = 4;
        } else {
            lo = 5;
            hi = 10;
        }
        for (i = 0; i < 6; i++) {
            v.x = 0.0f;
            v.y = 0.0f;
            v.z = -(f32) func_80048E2C(lo, hi);
            rot.y_rotation = func_80048E2C(-0x1000, 0x1000) + yaw;
            rot.x_rotation = func_80048E2C(-0x1000, 0x1000) + pitch;
            func_8004944C(&rot, &v);
            func_m008_001CC(hit->x, hit->y, hit->z, v.x, v.y, v.z, alpha, 4, 2.0f);
        }
    } else if (s->flags & 0x20) {
        func_m008_00334(hit->x, hit->y, hit->z, obj->shading->unk0 * 255.0f, 4);
        func_80003410(func_80048E2C(0xAB, 0xAD), hit->x, hit->y, hit->z, 4, NULL);
    }
}

/* A joint of the boss's hit model (0x34 bytes) */
typedef struct M068Joint {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x24 - 0x18];
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0x2C - 0x28];
    /* 0x2C */ s16 damage;
    /* 0x2E */ u8 pad2E[0x34 - 0x2E];
} M068Joint;

/* What the boss's obj->interactObj points at */
typedef struct M068Hits {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0x40 - 0x8];
    /* 0x40 */ M068Joint *joints;
} M068Hits;

/* A run of joints forming one part of the boss (.data, 11 of them) */
typedef struct M068Section {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s16 first; /* < 0: none */
    /* 0x6 */ s16 last;
    /* 0x8 */ u8 pad8[0xC - 0x8];
} M068Section;

/* A part's state (.bss, 11 of them) */
typedef struct M068PartState {
    /* 0x00 */ u8 pad0[0x13];
    /* 0x13 */ s8 alive;
    /* 0x14 */ u8 pad14[0x20 - 0x14];
} M068PartState;

/* An explosion near the boss */
typedef struct M068Blast {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 radius;
    /* 0x10 */ f32 falloff; /* angle units per unit of distance */
} M068Blast;

extern M068Section rl_m068_1254C[11];
extern M068PartState rl_m068_12F88[11];
extern f32 rl_m068_12748; /* a .rodata float: larger than any distance */
extern f32 func_80049C30(s32 angle); /* cosine */

/* An explosion: the joint nearest it in each live part takes damage falling off with distance */
#ifdef NON_MATCHING
/*
 * Same code and frame; the saved registers are assigned in another order (the
 * ROM has blast in s0, hits s1, the part s2) and the limit (a float literal in
 * the ROM, kept in f30) swaps with damage (f28).
 */
void func_m068_01330(Object *obj, M068Blast *blast, f32 damage) {
    M068Hits *hits = (M068Hits *) obj->interactObj;
    M068Section *sec;
    M068Joint *joint;
    s32 i;
    s32 j;
    s32 best;
    f32 bestDist;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;
    f32 f;
    f32 limit;

    limit = rl_m068_12748;
    for (i = 0; i < 11; i++) {
        sec = &rl_m068_1254C[i];
        if (sec->first >= 0) {
            best = -1;
            if (rl_m068_12F88[i].alive) {
                bestDist = limit;
                for (j = sec->first; j <= sec->last; j++) {
                    joint = &hits->joints[j];
                    dx = blast->x - joint->x;
                    dy = blast->y - joint->y;
                    dz = blast->z - joint->z;
                    dist = dx * dx + dy * dy + dz * dz - joint->radius * joint->radius;
                    if (dist < bestDist) {
                        bestDist = dist;
                        best = j;
                    }
                }
                if (best >= 0) {
                    if (bestDist < 0.0f) {
                        dist = 0.0f;
                    } else {
                        dist = func_80097140(bestDist);
                    }
                    if (dist <= blast->radius) {
                        f = func_80049C30(blast->falloff * dist) * damage;
                        hits->hitPoints -= (s32) f;
                        f /= (f32) (sec->last - sec->first + 1);
                        for (j = sec->first; j <= sec->last; j++) {
                            hits->joints[j].damage += f;
                        }
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_01330);
#endif

extern s16 rl_m068_1260C;     /* the cut camera's angle round the centre */
extern Vec3f *rl_m068_1261C;   /* what it looks at */
extern f32 rl_m068_12620;      /* its height over the centre */
extern f32 rl_m068_12624;      /* its distance from the centre */
extern f32 rl_m068_1274C;      /* a .rodata float: how slowly it settles */
extern f32 rl_m068_13740;      /* the centre (.bss) */
extern f32 rl_m068_13748;
extern f32 rl_m068_1374C;

/* A cut-scene camera: moves towards its place round the centre and turns to its target */
void func_m068_01628(s32 arg0, Object *cam, s32 updateRate) {
    f32 x;
    f32 y;
    f32 z;
    f32 f;
    s16 yaw;
    s16 pitch;

    x = func_80049C34(rl_m068_1260C) * rl_m068_12624 + rl_m068_13740;
    y = rl_m068_1374C + rl_m068_12620;
    z = func_80049C30(rl_m068_1260C) * rl_m068_12624 + rl_m068_13748;
    f = 1.0f - func_80049BEC(rl_m068_1274C, updateRate);
    cam->trans.x_position += (x - cam->trans.x_position) * f;
    cam->trans.y_position += (y - cam->trans.y_position) * f;
    cam->trans.z_position += (z - cam->trans.z_position) * f;
    x = cam->trans.x_position - rl_m068_1261C->x;
    y = cam->trans.y_position - rl_m068_1261C->y;
    z = cam->trans.z_position - rl_m068_1261C->z;
    yaw = 0x8000 - func_80049C84(x, z);
    pitch = func_80049C84(y, func_80097140(x * x + z * z));
    cam->trans.rotation.y_rotation = angle_lerp(cam->trans.rotation.y_rotation, yaw, f);
    cam->trans.rotation.x_rotation = angle_lerp(cam->trans.rotation.x_rotation, pitch, f);
    cam->trans.rotation.z_rotation = angle_lerp(cam->trans.rotation.z_rotation, 0, f);
}

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

extern f32 rl_m068_129EC; /* a .rodata float: the scale of the later frames */

/* Draws a shot as a sprite: its first frame opaque, the later ones see-through and scaled */
void func_m068_0CDD8(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068Sprite *sprite;
    f32 scale;

    sprite = ((M068Sprite **) ((M068Header *) obj->header)->models)[obj->modelIndex];
    if (obj->modelIndex == 0) {
        func_8005600C(255, 255, 255, 0, 255, 40);
        scale = obj->trans.scale;
    } else {
        func_8005600C(255, 255, 255, 150, 255, 150);
        scale = obj->trans.scale * rl_m068_129EC;
    }
    render_sprite_billboard_at(dl, arg1, arg2, sprite, obj->trans.x_position, obj->trans.y_position,
                               obj->trans.z_position, 0, scale, obj->unk28, 12, obj->opacity);
    func_80056064();
}

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

/* A beam's state (obj->unk68) */
typedef struct M068BeamState {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s16 mode; /* flying: 0 until it hits something (2: it was shot down) */
    /* 0x6 */ s16 burst; /* 0 while flying, then 1 while the burst plays */
} M068BeamState;

/* The entry of the object a beam leaves where it hits */
typedef struct M068MarkEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
} M068MarkEntry;

/* The object fields this module uses past the common ones */
typedef struct M068ObjectLight {
    /* 0x00 */ u8 pad0[0x78];
    /* 0x78 */ s32 *light;
} M068ObjectLight;

typedef struct M068AnimModel {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s32 anim;
} M068AnimModel;

extern s32 sprite_anim_advance(s32 anim, s32 *flags, s32 arg2, f32 *frame, s32 updateRate);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* moves the object */
extern f32 rl_m068_129F0; /* a .rodata float: the sparks' speed scale */
extern void corona_free(s32 light);

/* A beam: flies until it hits, then bursts into sparks (and leaves a mark where it hit), then plays out */
void func_m068_0D05C(Object *obj, s32 updateRate) {
    M068BeamState *s;
    UNUSED s32 pad1;
    SparkParams spark;
    M068MarkEntry entry;
    UNUSED s32 pad2;
    HitResult hit;
    Vec3f from;
    f32 rate;
    s32 i;
    s32 flags;
    s16 angle;

    s = obj->unk68;
    if (s->burst == 0) {
        if (((ObjectInteraction *) obj->interactObj)->hitPoints <= 0) {
            s->mode = 2;
        }
        if (s->mode == 0) {
            from.x = obj->trans.x_position;
            rate = updateRate;
            from.y = obj->trans.y_position;
            from.z = obj->trans.z_position;
            func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
            s->mode = func_80018808(obj, &from, (Vec3f *) &obj->trans.x_position, &hit, 0, 0);
            if (s->mode != 0) {
                func_80009A04(obj, hit.pos.x - obj->trans.x_position, hit.pos.y - obj->trans.y_position,
                              hit.pos.z - obj->trans.z_position);
            }
            obj->particleEmittersEnabled = 1;
            func_8005F5AC(obj, updateRate);
            return;
        }
        spark.x_position = obj->trans.x_position;
        spark.y_position = obj->trans.y_position;
        spark.z_position = obj->trans.z_position;
        spark.unk18 = 20;
        spark.unk1A = 20;
        spark.unk1B = 190;
        spark.unk1C = -1;
        spark.unk20 = 0x8C0EF4FF;
        spark.x_rotation = 0x2000;
        angle = obj->trans.rotation.y_rotation - 0x4000;
        rate = rl_m068_129F0;
        for (i = 0; i < 8; i++) {
            spark.y_rotation = angle;
            spark.speed = func_80048E2C(10, 50) * rate;
            spark.unk14 = func_80048E2C(10, 50) * rate;
            func_m010_0000C(&spark);
            angle += 0x2000;
        }
        if (s->mode == 1) {
            Object *mark;

            entry.common.objectID = 0x327;
            entry.common.x = obj->trans.x_position;
            entry.common.y = obj->trans.y_position;
            entry.common.z = obj->trans.z_position;
            entry.unkA = 0;
            entry.unk10 = 1.5f;
            entry.unkC = 2.0f;
            mark = func_80005F14(&entry, 1);
            if (mark != NULL) {
                mark->level_entry = NULL;
            }
        }
        func_80003410(0x1D2, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        corona_free(*((M068ObjectLight *) obj)->light);
        s->mode = 0;
        s->burst = 1;
    } else if (s->mode == 0) {
        obj->trans.rotation.z_rotation = 0;
        obj->trans.scale = 1.0f;
        flags = 1;
        if (sprite_anim_advance((s32) ((M068Header *) obj->header)->models[1], &flags, 10, &obj->unk28,
                          updateRate)) {
            s->mode = 1;
        }
    } else {
        func_800079B8(obj);
    }
}

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

/* What the boss's death effect keeps in obj->unk68 */
typedef struct M068Death {
    /* 0x000 */ Triangle tris[16];
    /* 0x100 */ Vertex verts[2][72]; /* drawn from one while the other is filled */
    /* 0x6A0 */ u8 pad6A0[0x6BA - 0x6A0];
    /* 0x6BA */ u8 visible;
    /* 0x6BB */ u8 buffer;
} M068Death;

extern u32 rl_m068_12720[2]; /* the effect's two colours */
extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80055C00(Gfx **dl, void *model, s32 flags, s32 arg3);
extern void func_80041DC0(Gfx **dl);

#ifdef NON_MATCHING
/*
 * Draws the boss's death effect: the same triangles over four runs of
 * vertices, in two colours. The code matches but for registers: the ROM keeps
 * the end of the colour table in s3 (one more saved register, frame 0x40).
 */
void func_m068_0ED10(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068Death *d = obj->unk68;
    Vertex *v;
    s32 i;

    if (d->visible) {
        obj->trans.scale = 1.0f;
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        v = d->verts[d->buffer];
        func_80055C00(dl, *((M068Header *) obj->header)->models, 0x206, 0);
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 190);
        for (i = 0; i < 2; i++) {
            gDPSetColor((*dl)++, G_SETENVCOLOR, rl_m068_12720[i]);
            gSPVertexJFG((*dl)++, (u32) v + 0x80000000, 19, 0);
            gSPPolygon((*dl)++, (u32) d + 0x80000000, 16, 1);
            gDPPipeSync((*dl)++);
            v += 18;
            gSPVertexJFG((*dl)++, (u32) v + 0x80000000, 19, 0);
            gSPPolygon((*dl)++, (u32) d + 0x80000000, 16, 1);
            gDPPipeSync((*dl)++);
            v += 18;
        }
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        func_80041DC0(dl);
        func_80055B80(dl);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0ED10);
#endif

/* A shock ring spreading from the boss's landing (obj->unk68) */
typedef struct M068Ring {
    /* 0x00 */ s32 sound;
    /* 0x04 */ f32 time;
    /* 0x08 */ f32 fadeStart;
    /* 0x0C */ f32 end;
    /* 0x10 */ f32 growth;
    /* 0x14 */ f32 scale;
    /* 0x18 */ s32 damage; /* 0 once it has hit */
} M068Ring;

/* What the ring's init gets */
typedef struct M068RingSetup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 damage;
    /* 0x0C */ f32 fadeStart;
    /* 0x10 */ f32 end;
} M068RingSetup;

void func_m068_0EF2C(Object *obj, M068RingSetup *setup) {
    M068Ring *ring = obj->unk68;

    ring->scale = rl_m068_129F8;
    ring->damage = setup->damage;
    ring->fadeStart = setup->fadeStart;
    ring->end = setup->end;
    ring->growth = rl_m068_129FC;
}

/* What the player keeps in obj->unk68, as far as used here */
typedef struct M068Player {
    /* 0x000 */ u8 pad0[0x5C8];
    /* 0x5C8 */ s16 unk5C8;
} M068Player;

extern void func_8005F5A0(f32 arg0);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* moves the object */
extern void func_800035E8(s32 sound, f32 x, f32 y, f32 z); /* moves a positioned sound */
extern void audspat_point_set_volume(s32 sound, s32 volume);

#ifdef NON_MATCHING
/*
 * The ring grows and fades; it hurts the player once if they stand on its
 * edge. Same operations, but the float registers are numbered differently
 * (the rate is f0 in the ROM) and the frame is 0x18 bytes smaller.
 */
void func_m068_0EF70(Object *obj, s32 updateRate) {
    f32 rate = updateRate;
    M068Ring *r = obj->unk68;
    M068Model *model = *((M068Header *) obj->header)->models;
    f32 k;
    s32 damage;
    Object *player;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;

    r->time += rate / 60.0f;
    if (r->fadeStart < r->time) {
        k = 1.0f - (r->time - r->fadeStart) / (r->end - r->fadeStart);
    } else {
        k = 1.0f;
    }
    obj->opacity = (s32) (255.0f * k);
    r->scale += r->growth * k * rate;
    obj->trans.rotation.y_rotation += (s32) (512.0f * rate * k);
    if (k == 1.0f) {
        obj->particleEmittersEnabled = 1;
        func_8005F5AC(obj, updateRate);
    }
    if (r->end <= r->time) {
        func_800079B8(obj);
    }
    if (r->damage != 0) {
        damage = r->damage * k;
        player = func_8000E4D0(0);
        if (((M068Player *) player->unk68)->unk5C8 == 0) {
            dx = player->trans.x_position - obj->trans.x_position;
            dy = player->trans.y_position - obj->trans.y_position;
            dz = player->trans.z_position - obj->trans.z_position;
            d = func_80097140(dx * dx + dz * dz) - model->unk92 * r->scale;
            if (d > -30.0f && d < 30.0f && dy < model->radius) {
                ((s16 *) player->interactObj)[3] -= damage;
                r->damage = 0;
            }
        }
    }
    ((M068Object *) obj)->radius = model->unk92 * r->scale;
    if (r->sound == 0) {
        func_80003410(0x1E6, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &r->sound);
    } else {
        func_800035E8(r->sound, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        audspat_point_set_volume(r->sound, (s32) (127.0f * k) & 0xFF);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0EF70);
#endif

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

/* A falling boulder's state (obj->unk68) */
typedef struct M068Boulder {
    /* 0x00 */ u8 *colour; /* its light's colour, or NULL */
    /* 0x04 */ Vec3f trail[5];
    /* 0x40 */ Vec3f velocity;
    /* 0x4C */ f32 speed;
    /* 0x50 */ u8 pad50[0x5C - 0x50];
    /* 0x5C */ f32 radius;
    /* 0x60 */ u8 pad60[0x6C - 0x60];
    /* 0x6C */ u8 r;
    /* 0x6D */ u8 g;
    /* 0x6E */ u8 b;
    /* 0x6F */ u8 pad6F;
    /* 0x70 */ s16 unk70;
    /* 0x72 */ s16 unk72;
    /* 0x74 */ s16 kind;
} M068Boulder;

/* A boulder's level entry */
typedef struct M068BoulderSetup {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ u8 kind;
} M068BoulderSetup;

extern void func_80049394(Vec3s *rotation, Vec3f *vec);
extern s32 rl_m068_125E8;       /* how many trail points (.data) */
extern s32 rl_m068_125FC;       /* the boulders made (.data) */
extern Vec3f rl_m068_136F0[];   /* the trail's start (.bss) */

#ifdef NON_MATCHING
/*
 * A boulder: starts 1000 units back along a random direction, with a trail, a
 * speed and a size. The frame is 8 bytes bigger (rotation 4 higher) and the
 * temporaries after the trail end are one lower than the ROM's.
 */
void func_m068_0F478(Object *obj, M068BoulderSetup *setup) {
    M068Boulder *b;
    s32 i;
    f32 f;
    ObjectHeaderUnk50 *hit;
    Vec3f *t;
    Vec3f *src;
    Vec3f v;
    Vec3s rot;

    b = obj->unk68;
    b->unk70 = 0;
    b->kind = setup->kind;
    rot.y_rotation = 0;
    rot.x_rotation = 0x3000;
    rot.z_rotation = func_80048E2C(-0x7FBC, 0x7FBC);
    v.x = 0.0f;
    v.y = 0.0f;
    v.z = -1000.0f;
    func_80049394(&rot, &v);
    obj->trans.x_position += v.x;
    obj->trans.y_position += v.y;
    obj->trans.z_position += v.z;
    for (i = 0, t = b->trail, src = rl_m068_136F0; i < rl_m068_125E8; i++, t++, src++) {
        t->x = src->x;
        t->y = src->y;
        t->z = src->z;
    }
    if (rl_m068_125E8 > 0) {
        rot.y_rotation = 0;
        rot.x_rotation = 0x4000;
        rot.z_rotation = func_80048E2C(-0x3000, 0x3000);
        v.x = 0.0f;
        v.y = 0.0f;
        v.z = func_80048E2C(-400, -200);
        func_80049394(&rot, &v);
        t = &b->trail[rl_m068_125E8 - 1];
        t->x += v.x;
        t->y += v.y;
        t->z += v.z;
    }
    f = func_80048E2C(100, 200) / 10.0f;
    b->speed = f;
    b->velocity.x = -v.x;
    b->velocity.y = -v.y;
    b->velocity.z = -v.z;
    b->unk72 = f * 100.0f;
    if (obj->light != NULL && *obj->light != 0) {
        b->colour = (u8 *) *obj->light + 0x48;
        b->r = b->colour[8];
        b->g = b->colour[9];
        b->b = b->colour[10];
    } else {
        b->colour = NULL;
        b->r = 155;
        b->g = 155;
        b->b = 155;
    }
    obj->trans.scale = func_80048E2C(30, 100) / 100.0f;
    hit = obj->header->unk50;
    b->radius = hit->radius * obj->trans.scale;
    func_80079D1C(obj, 0x100, 1, 5, 0, 0, 0, hit->radius, hit->radius, -1.0f, 1.0f, 0x10);
    if (b->kind == 2 || b->kind == 1) {
        ((u8 *) obj->interactObj)[0x2F] = 0;
    } else {
        ((u8 *) obj->interactObj)[0x2F] = 100;
    }
    rl_m068_125FC++;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0F478);
#endif

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

/* A point the pieces fly from: a vertex of the model */
typedef struct M068Point {
    /* 0x0 */ u16 vertex;
    /* 0x2 */ u16 unk2;
} M068Point;

/* A model vertex (size 0xA) */
typedef struct M068Vertex {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ u8 pad6[4];
} M068Vertex;

/* The model of the object that breaks up */
typedef struct M068BreakModel {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ M068Vertex *vertices;
    /* 0x20 */ u8 pad20[0x30 - 0x20];
    /* 0x30 */ M068Point *points;
} M068BreakModel;

/* What obj->unk5C points at here: the object's matrices */
typedef struct M068Matrices {
    /* 0x000 */ u8 pad0[0x80];
    /* 0x080 */ MtxF mtx[2];
    /* 0x100 */ u8 pad100[0x12B - 0x100];
    /* 0x12B */ u8 current;
} M068Matrices;

extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* transforms a point */
extern Object *rl_m068_12F70[6]; /* the six pieces (.bss) */
extern MtxF rl_m068_1250C;       /* (.data) */
extern f32 rl_m068_12A20;        /* a .rodata float */

/* Breaks the object up: sends each of the six pieces flying out from its point of the model, then frees it */
#ifdef NON_MATCHING
/* Frame and slots match; the t-registers of the set-up are one higher and two loads swap places */
void func_m068_110E0(Object *obj, s32 updateRate) {
    s32 *state;
    M068BreakModel *model;
    MtxF *mtx;
    Object *piece;
    f32 dx;
    f32 dy;
    f32 dz;
    M068PieceSetup entry;
    f32 x;
    f32 y;
    f32 z;
    s32 i;
    f32 d;
    M068Vertex *v;
    UNUSED s32 pad[1];

    state = obj->unk68;
    if (*state != 0) {
        model = *(M068BreakModel **) obj->modelInstances[obj->modelIndex];
        mtx = &((M068Matrices *) obj->unk5C)->mtx[((M068Matrices *) obj->unk5C)->current];
        entry.objectID = 0x65;
        entry.active = 1;
        entry.yRotation = obj->trans.rotation.y_rotation;
        entry.xRotation = obj->trans.rotation.x_rotation;
        entry.mtx = &rl_m068_1250C;
        entry.unk1C = 3.0f;
        entry.zRotation = obj->trans.rotation.z_rotation;
        entry.unk20 = 2.0f;
        entry.unk28 = rl_m068_12A20;
        for (i = 0; i < 6; i++) {
            v = &model->vertices[model->points[i].vertex];
            func_80048B60(mtx, v->x, v->y, v->z, &x, &y, &z);
            entry.x = x;
            entry.y = y;
            entry.z = z;
            dx = x - obj->trans.x_position;
            dy = y - obj->trans.y_position;
            dz = z - obj->trans.z_position;
            d = func_80097140(dx * dx + dy * dy + dz * dz);
            piece = rl_m068_12F70[i];
            dx /= d;
            dy /= d;
            dz /= d;
            entry.velocity.x = dx;
            entry.velocity.y = dy;
            entry.velocity.z = dz;
            entry.speed = d / 10.0f;
            func_m068_10DD4(piece, &entry);
            piece->trans.scale = obj->trans.scale;
        }
        func_800079B8(obj);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_110E0);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", data);

INCLUDE_ASM("asm/nonmatchings/modules/m068", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m068", bss);

