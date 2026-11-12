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
    /* 0x00 */ Vec3f pos; /* where it goes */
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ f32 timer; /* counts down, in seconds */
    /* 0x14 */ s32 unk14;
    /* 0x18 */ f32 unk18; /* > 0: the particle emitters run, in seconds */
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ f32 unk30;
    /* 0x34 */ f32 unk34;
    /* 0x38 */ f32 unk38; /* vertical speed */
    /* 0x3C */ f32 unk3C;
    /* 0x40 */ f32 unk40; /* the height it goes to */
    /* 0x44 */ f32 unk44;
    /* 0x48 */ s32 flags; /* 0x20: (unknown) */
    /* 0x4C */ s32 events;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s16 unk54;
    /* 0x56 */ s16 soundPoint;
    /* 0x58 */ s32 sound58;
    /* 0x5C */ s32 sound5C;
    /* 0x60 */ u16 unk60;
    /* 0x62 */ u8 pad62[0x6C - 0x62];
    /* 0x6C */ s32 sound6C;
    /* 0x70 */ s32 sound70;
    /* 0x74 */ u8 pad74[0x76 - 0x74];
    /* 0x76 */ s16 hitByPlayer;
    /* 0x78 */ u8 pad78[0x7E - 0x78];
    /* 0x7E */ s16 unk7E; /* the shake angle */
    /* 0x80 */ s16 unk80;
    /* 0x82 */ s8 unk82;
    /* 0x83 */ s8 unk83;
    /* 0x84 */ struct M068ColourCycle *cycle; /* the colour cycle running */
    /* 0x88 */ s16 cycleKind; /* 1-4: which one to run, 0: none */
    /* 0x8A */ s16 unk8A;
    /* 0x8C */ s16 cycleFirst; /* the vertices it colours */
    /* 0x8E */ s16 cycleEnd;
    /* 0x90 */ s16 unk90;
    /* 0x92 */ s16 unk92;
    /* 0x94 */ s16 unk94;
    /* 0x96 */ s16 unk96;
    /* 0x98 */ s16 unk98;
    /* 0x9A */ s16 unk9A;
    /* 0x9C */ u8 pad9C[0x9E - 0x9C];
    /* 0x9E */ s16 unk9E; /* the yaw unk9A turns to */
    /* 0xA0 */ u8 padA0[0xA8 - 0xA0];
    /* 0xA8 */ s16 unkA8;
    /* 0xAA */ s16 unkAA;
    /* 0xAC */ s16 unkAC;
    /* 0xAE */ s16 unkAE;
    /* 0xB0 */ s16 unkB0;
    /* 0xB2 */ u8 padB2[0xB4 - 0xB2];
    /* 0xB4 */ s16 unkB4;
    /* 0xB6 */ s16 unkB6;
    /* 0xB8 */ s16 unkB8;
    /* 0xBA */ s16 unkBA;
    /* 0xBC */ s16 unkBC;
    /* 0xBE */ s16 unkBE;
    /* 0xC0 */ f32 unkC0;
    /* 0xC4 */ f32 unkC4;
    /* 0xC8 */ s32 unkC8; /* damage taken this frame */
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
extern s32 rl_m068_12614[2];
extern s32 rl_m068_12618;

/* A part's state (.bss, 11 of them) */
typedef struct M068PartState {
    /* 0x00 */ s16 target[3]; /* the angles it turns to */
    /* 0x06 */ s16 angle[3];
    /* 0x0C */ f32 scale;
    /* 0x10 */ s16 hp;
    /* 0x12 */ s8 hit; /* set: damaged this frame */
    /* 0x13 */ s8 alive;
    /* 0x14 */ f32 rate[3]; /* how slowly each angle turns */
} M068PartState;

/*
 * The module's .bss, in address order (the assembly functions reach it by
 * offset). IDO puts each variable of 8 bytes or more on an 8-byte boundary.
 */
static s16 *sJointCmdNext;       /* 12A30: the next joint command */
static u8 sJointCmds[0x100];     /* 12A38: the joint (tilt) command list */
static s32 sBss12B38;
static Object *sBss12B40[6];
static Object *sBss12B58[6];
static f32 sMtx12B70[4][4];      /* copies of some joints' matrices */
static f32 sMtx12BB0[4][4];
static Object *sBss12BF0[9];
static f32 sMtx12C18[9][4][4];
static Object *sBss12E58[3];
static f32 sMtx12E68[3][4][4];
static Object *sBss12F28;
static f32 sMtx12F30[4][4];
static Object *sPieces[6];       /* 12F70: the six pieces */
static M068PartState sParts[11]; /* 12F88 */
static u8 sBss130E8[0x230];
static f32 sJointPos[126];       /* 13318: the joints' positions, x y z each */
static Vec3f sDropPoints[4];     /* 13510: where they fall */
static Vec3f sBss13540;
static f32 sPlanes[2][4];        /* 13550: two planes (x, y, z, d) */
static u8 sBss13570[0x80];
static Vec3f sLandPoints[10];    /* 135F0: the landing points */
static Vec3f sDropPoint;         /* 13668 */
static Vec3f sBss13678;
static u8 sBss13688[0x68];
static Vec3f sTrailStart[5];     /* 136F0: the trail's start */
static Vec3f sStripEnd;          /* 13730: where the electric strips reach to */
static s16 sBss1373C;
static Vec3f sCentre;            /* 13740: the centre */
static f32 sCentreY;             /* 1374C: the ground's height there */
static f32 sBss13750;
static u8 sLightColour[3];       /* 13754: the strips' light and the glow sprite's colour */

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
    return sJointCmds;
}

/* A light source of the level (what func_800218A8 lists) */
typedef struct M068Light {
    /* 0x00 */ u8 type;   /* 0: directional, 2: a column, 3: a cone, else a sphere */
    /* 0x01 */ u8 falloff;
    /* 0x02 */ u8 pad2;
    /* 0x03 */ u8 flags;  /* 1: on, 0x20: counts as the brightest */
    /* 0x04 */ u8 pad4[0x18 - 0x4];
    /* 0x18 */ f32 x;
    /* 0x1C */ f32 y;
    /* 0x20 */ f32 z;
    /* 0x24 */ u8 pad24[0x30 - 0x24];
    /* 0x30 */ f32 radiusSq;
    /* 0x34 */ f32 unk34;
    /* 0x38 */ f32 minY;
    /* 0x3C */ f32 maxY;
    /* 0x40 */ u8 pad40[0x43 - 0x40];
    /* 0x43 */ u8 intensity;
    /* 0x44 */ f32 unk44;
    /* 0x48 */ u8 pad48[0x60 - 0x48];
    /* 0x60 */ f32 dirX;
    /* 0x64 */ f32 dirY;
    /* 0x68 */ f32 dirZ;
    /* 0x6C */ u8 pad6C[0x70 - 0x6C];
    /* 0x70 */ s32 colour;
} M068Light;

/* One light shining on an object (size 0x20) */
typedef struct M068ShadeLight {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 intensity;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ s32 colour;
    /* 0x1C */ u8 pad1C[4];
} M068ShadeLight;

/* The lights shining on an object; lights[0] is the ambient light (its unk14: a shift) */
typedef struct M068Shade {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ u8 ambient;
    /* 0x0D */ u8 ambientBase;
    /* 0x0E */ s16 count;
    /* 0x10 */ M068ShadeLight lights[4];
} M068Shade;

/* The ambient light the level sets (what func_8004470C returns), as used here */
typedef struct M068Level {
    /* 0x00 */ u8 pad0[0xDB];
    /* 0xDB */ u8 ambient;
    /* 0xDC */ u8 ambientBase;
    /* 0xDD */ u8 padDD[2];
    /* 0xDF */ u8 hasAmbient;
} M068Level;

extern M068Level *func_8004470C(void);
extern M068Light **func_800218A8(s32 *count);
extern f32 light_falloff_scale(f32 scale, f32 arg1, f32 arg2, s32 type);
extern f32 func_8002215C(f32 dirX, f32 dirY, f32 dirZ, f32 dx, f32 dy, f32 dz, f32 dist);
#ifdef NON_MATCHING
static void func_m068_004D8(Object *obj, M068Light *light, f32 *x, f32 *y, f32 *z);
#define rl_m068_004D8 func_m068_004D8
#else
extern void rl_m068_004D8(Object *obj, M068Light *light, f32 *x, f32 *y, f32 *z);
#endif

/* Picks the (up to four) lights that shine on the boss and sets its ambient light */
void func_m068_0003C(Object *obj, M068Shade *sh) {
    M068Light **lights;
    M068Light *light;
    M068ShadeLight *e;
    M068Level *level;
    s32 i;
    s32 j;
    s32 s;
    s32 brightest;
    f32 k;
    f32 px;
    f32 py;
    f32 pz;
    s32 min;
    s32 b;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    s32 count;
    UNUSED s32 pad;

    sh->count = 1;
    brightest = 0;
    level = func_8004470C();
    lights = func_800218A8(&count);
    for (i = 0; i < count; i++) {
        light = lights[i];
        if ((light->flags & 1) && light->intensity != 0) {
            if (light->type != 0) {
                rl_m068_004D8(obj, light, &px, &py, &pz);
                s = 0;
                dx = light->x - px;
                dy = light->y - py;
                dz = light->z - pz;
                d = dx * dx + dz * dz;
                switch (light->type) {
                    case 2:
                        if (d < light->radiusSq && light->minY < py && py <= light->maxY) {
                            s = light_falloff_scale(light->unk44, func_80097140(d), light->unk34, light->falloff);
                        }
                        break;
                    case 3:
                        d += dy * dy;
                        if (d < light->radiusSq) {
                            d = func_80097140(d);
                            k = func_8002215C(light->dirX, light->dirY, light->dirZ, dx, dy, dz, d);
                            if (k > 0.0f) {
                                s = light_falloff_scale(light->unk44, d, light->unk34, light->falloff) * k;
                            }
                        }
                        break;
                    default:
                        d += dy * dy;
                        if (d < light->radiusSq) {
                            s = light_falloff_scale(light->unk44, func_80097140(d), light->unk34, light->falloff);
                        }
                        break;
                }
            } else {
                dx = light->dirX;
                dy = light->dirY;
                dz = light->dirZ;
                s = light->intensity;
            }
            if (s != 0) {
                if ((light->flags & 0x20) && brightest < s) {
                    brightest = s;
                }
                if (sh->count < 4) {
                    e = &sh->lights[sh->count];
                    sh->count++;
                } else {
                    e = &sh->lights[1];
                    min = sh->lights[1].intensity;
                    for (j = 2; j < 4; j++) {
                        if (sh->lights[j].intensity < min) {
                            e = &sh->lights[j];
                            min = sh->lights[j].intensity;
                        }
                    }
                    if (e->intensity >= s) {
                        e = NULL;
                    }
                }
                if (e != NULL) {
                    e->x = dx;
                    e->y = dy;
                    e->z = dz;
                    e->unk14 = 0;
                    e->unkC = (1 << (8 - sh->lights[0].unk14)) << 6;
                    e->intensity = s;
                    e->unk17 = 0;
                    e->unk16 = s;
                    e->unk10 = s << sh->lights[0].unk14;
                    e->colour = light->colour;
                }
            }
        }
    }
    if (level->hasAmbient) {
        b = level->ambientBase;
        s = level->ambient - b;
    } else {
        b = sh->ambientBase;
        s = sh->ambient - b;
    }
    if (brightest >= 64) {
        b = ((319 - brightest) * b) >> 8;
        s = ((319 - brightest) * s) >> 8;
    }
    sh->lights[0].intensity = b + s;
    sh->lights[0].unk16 = s;
    sh->lights[0].unk17 = b;
    sh->lights[0].unk10 = s << sh->lights[0].unk14;
}

/* A model as the boss's instance sees it: its box and joint count */
typedef struct M068BoxModel {
    /* 0x00 */ u8 pad0[0x3C];
    /* 0x3C */ s16 min[3];
    /* 0x42 */ s16 max[3];
    /* 0x48 */ u8 pad48[0x4F - 0x48];
    /* 0x4F */ s8 jointCount;
} M068BoxModel;

typedef struct M068BoxInstance {
    /* 0x00 */ M068BoxModel *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 hidden;
    /* 0x0B */ s8 frame;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *frames[1];
} M068BoxInstance;


#ifdef NON_MATCHING
/*
 * The point of the boss nearest a light: its nearest joint, else the nearest corner of its box.
 * The corner loop's registers differ: the ROM keeps the light's x/y in f28/f30 and the best corner on the stack.
 */
static void func_m068_004D8(Object *obj, M068Light *light, f32 *x, f32 *y, f32 *z) {
    M068BoxInstance *inst;
    M068BoxModel *model;
    s32 i;
    MtxF *best;
    f32 corners[8][3];
    f32 bx;
    f32 by;
    f32 bz;
    MtxF *m;
    f32 *c;
    f32 min;
    f32 d;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 x0;
    f32 y0;
    f32 z0;
    f32 x1;
    f32 y1;
    f32 z1;

    inst = (M068BoxInstance *) obj->modelInstances[obj->modelIndex];
    model = inst->model;
    if (model->jointCount != 0 && inst->hidden == 0) {
        min = 1.024e9f;
        for (i = 0; i < model->jointCount; i++) {
            m = inst->frames[inst->frame] + i;
            dx = light->x - (*m)[3][0];
            dy = light->y - (*m)[3][1];
            dz = light->z - (*m)[3][2];
            d = dx * dx + dy * dy + dz * dz;
            if (d < min) {
                best = m;
                min = d;
            }
        }
        *x = (*best)[3][0];
        *y = (*best)[3][1];
        *z = (*best)[3][2];
        return;
    }
    x0 = obj->trans.x_position + model->min[0] * obj->trans.scale;
    y0 = obj->trans.y_position + model->min[1] * obj->trans.scale;
    z0 = obj->trans.z_position + model->min[2] * obj->trans.scale;
    x1 = obj->trans.x_position + model->max[0] * obj->trans.scale;
    y1 = obj->trans.y_position + model->max[1] * obj->trans.scale;
    z1 = obj->trans.z_position + model->max[2] * obj->trans.scale;
    corners[0][0] = x0;
    corners[0][1] = y0;
    corners[0][2] = z0;
    corners[1][0] = x1;
    corners[1][1] = y0;
    corners[1][2] = z0;
    corners[2][0] = x1;
    corners[2][1] = y0;
    corners[2][2] = z1;
    corners[3][0] = x0;
    corners[3][1] = y0;
    corners[3][2] = z1;
    corners[4][0] = x0;
    corners[4][1] = y1;
    corners[4][2] = z0;
    corners[5][0] = x1;
    corners[5][1] = y1;
    corners[5][2] = z0;
    corners[6][0] = x1;
    corners[6][1] = y1;
    corners[6][2] = z1;
    corners[7][0] = x0;
    corners[7][1] = y1;
    corners[7][2] = z1;
    bx = x0;
    by = y0;
    bz = z0;
    min = 1.024e9f;
    c = corners[0];
    for (i = 0; i < 8; i++, c += 3) {
        dx = light->x - c[0];
        dy = light->y - c[1];
        dz = light->z - c[2];
        if (dx * dx + dy * dy + dz * dz < min) {
            bx = c[0];
            by = c[1];
            bz = c[2];
            min = dx * dx + dy * dy + dz * dz;
        }
    }
    *x = bx;
    *y = by;
    *z = bz;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_004D8);
#endif

/* The boss's state fields its effects use (obj->unk68) */
typedef struct M068Effects {
    /* 0x00 */ u8 pad0[0x80];
    /* 0x80 */ s16 alpha;
    /* 0x82 */ s8 glowing;
    /* 0x83 */ s8 glowKind;
    /* 0x84 */ u8 pad84[0x88 - 0x84];
    /* 0x88 */ s16 trails;
} M068Effects;

/* The boss's model instance */
typedef struct M068Instance {
    /* 0x00 */ void *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 hidden;
    /* 0x0B */ s8 frame;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ MtxF *frames[1];
    /* 0x14 */ u8 pad14[0x68 - 0x14];
    /* 0x68 */ Vec3f *points; /* where its sounds play */
} M068Instance;

extern s16 rl_m068_12564[]; /* the joints the two glows are drawn at (.data) */
extern u8 rl_m068_124CC[]; /* the two trails' parameters (.data) */
extern u8 rl_m068_124DC[];
extern void rl_m068_00C70(Object *obj, M068Effects *s, void *params, Gfx **dl, s32 arg1, s32 arg2);

#ifdef NON_MATCHING
/*
 * Draws the boss's trails and, while it glows, two glow models at its joints.
 * Length and frame match; the temporaries are numbered one higher from the first joint load.
 */
void func_m068_0092C(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068Effects *s;
    M068DrawLists *lists;
    M068DrawModel *model;
    M068Instance *inst;
    MtxF *mtx;

    s = obj->unk68;
    if (s->trails != 0) {
        rl_m068_00C70(obj, s, rl_m068_124CC, dl, arg1, arg2);
        rl_m068_00C70(obj, s, rl_m068_124DC, dl, arg1, arg2);
    }
    inst = obj->modelInstances[obj->modelIndex];
    if (inst->hidden == 0 && s->glowing != 0) {
        if (s->glowKind != 0) {
            model = ((M068Header *) obj->header)->models[1];
        } else {
            model = ((M068Header *) obj->header)->models[2];
        }
        lists = model->lists;
        mtx = inst->frames[inst->frame] + rl_m068_12564[0];
        (*mtx)[0][3] = 0.0f;
        (*mtx)[1][3] = 0.0f;
        (*mtx)[2][3] = 0.0f;
        (*mtx)[3][3] = 1.0f;
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, s->alpha);
        mtx_model_push(dl, arg1, mtx);
        gSPModelFrameJFG((*dl)++, model->frames[model->frame], OS_K0_TO_PHYSICAL(model->vertices));
        gSPDisplayList((*dl)++, OS_K0_TO_PHYSICAL(lists->dl74));
        if (s->glowKind != 0) {
            model = ((M068Header *) obj->header)->models[3];
        } else {
            model = ((M068Header *) obj->header)->models[4];
        }
        lists = model->lists;
        mtx = inst->frames[inst->frame] + rl_m068_12564[6];
        (*mtx)[0][3] = 0.0f;
        (*mtx)[1][3] = 0.0f;
        (*mtx)[2][3] = 0.0f;
        (*mtx)[3][3] = 1.0f;
        mtx_model_push(dl, arg1, mtx);
        gSPModelFrameJFG((*dl)++, model->frames[model->frame], OS_K0_TO_PHYSICAL(model->vertices));
        gSPDisplayList((*dl)++, OS_K0_TO_PHYSICAL(lists->dl74));
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        gSPModelFrameEndJFG((*dl)++);
        mtx_pop(dl);
        func_80055B80(dl);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0092C);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_00C70);

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
    /* 0x2E */ u8 pad2E;
    /* 0x2F */ u8 unk2F;
    /* 0x30 */ u8 bit7 : 1;
    /* 0x30 */ u8 bit6 : 1;
    /* 0x30 */ u8 off : 1; /* set: the joint can't be hit */
    /* 0x30 */ u8 bits : 5;
    /* 0x31 */ u8 pad31[0x34 - 0x31];
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
    /* 0x0 */ s16 joint;
    /* 0x2 */ u8 pad2[4 - 2];
    /* 0x4 */ s16 first; /* < 0: none */
    /* 0x6 */ s16 last;
    /* 0x8 */ u8 pad8[0xC - 0x8];
} M068Section;


/* An explosion near the boss */
typedef struct M068Blast {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 radius;
    /* 0x10 */ f32 falloff; /* angle units per unit of distance */
} M068Blast;

extern M068Section rl_m068_1254C[11];
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

    limit = 1.024e9f;
    for (i = 0; i < 11; i++) {
        sec = &rl_m068_1254C[i];
        if (sec->first >= 0) {
            best = -1;
            if (sParts[i].alive) {
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
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_01330);
#endif

extern s16 rl_m068_1260C;     /* the cut camera's angle round the centre */
extern Vec3f *rl_m068_1261C;   /* what it looks at */
extern f32 rl_m068_12620;      /* its height over the centre */
extern f32 rl_m068_12624;      /* its distance from the centre */

/* A cut-scene camera: moves towards its place round the centre and turns to its target */
void func_m068_01628(s32 arg0, Object *cam, s32 updateRate) {
    f32 x;
    f32 y;
    f32 z;
    f32 f;
    s16 yaw;
    s16 pitch;

    x = func_80049C34(rl_m068_1260C) * rl_m068_12624 + sCentre.x;
    y = sCentreY + rl_m068_12620;
    z = func_80049C30(rl_m068_1260C) * rl_m068_12624 + sCentre.z;
    f = 1.0f - func_80049BEC(0.97f, updateRate);
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

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_017EC);

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
    if (rl_m068_12614[0] != 0) {
        func_80003614(rl_m068_12614[0]);
    }
    if (rl_m068_12618 != 0) {
        func_80003614(rl_m068_12618);
    }
    if (rl_m068_124EC != 0) {
        func_80003614(rl_m068_124EC);
    }
}
#pragma weak __rl_fini = func_m068_0246C

extern s32 func_8001B078(f32 x, f32 z, s32 arg2, f32 ***heights);
extern s32 func_80082D80(f32 x, f32 z, s32 arg2, f32 ***heights);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_8003D21C(M068Instance *inst, void *model, Object *obj, f32 arg3, f32 arg4);
extern void func_800221F0(Object *obj, M068Instance *inst, void *shading, MtxF *mtx);
extern void obj_tex_animate(Object *obj, void *model, M068Instance *inst, s32 arg3);

/* The player's state (player object's unk68), as the boss's helpers use it */
typedef struct M068Player {
    /* 0x000 */ u8 pad0[0x18E];
    /* 0x18E */ u8 unk18E;
    /* 0x18F */ u8 pad18F[0x568 - 0x18F];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x5C8 - 0x569];
    /* 0x5C8 */ s16 unk5C8;
} M068Player;

extern s32 func_800468CC(void);
extern s32 menu_mode_get(void);
extern void level_goto(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_m006_015E0(s32 hide);
extern void chain_object_unlink_from_players(Object *obj);
extern void func_800035E8(s32 sound, f32 x, f32 y, f32 z); /* moves a positioned sound */

extern s32 rl_m068_125E0;      /* (.data) set: ... */
extern s16 rl_m068_125E4;      /* (.data) */
extern Object *rl_m068_12508;  /* (.data) */
#ifdef NON_MATCHING
static s32 func_m068_05C8C(Object *obj, M068State *s, Vec3f *target, s16 yaw, s32 updateRate, f32 rate);
#define rl_m068_05C8C func_m068_05C8C
#else
extern s32 rl_m068_05C8C(Object *obj, M068State *s, Vec3f *target, s16 yaw, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static s16 func_m068_0C7DC(s16 angle, s16 target, s16 speed, s32 updateRate);
#define rl_m068_0C7DC func_m068_0C7DC
#else
extern s16 rl_m068_0C7DC(s16 angle, s16 target, s16 speed, s32 updateRate);
#endif

/* A colour cycle (main segment, as m107 uses it) */
typedef struct M068ColourCycle {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ u8 pad4[0x8 - 0x4];
    /* 0x8 */ u8 r; /* the current colour */
    /* 0x9 */ u8 g;
    /* 0xA */ u8 b;
    /* 0xB */ u8 padB;
    /* 0xC */ s32 *unkC;
} M068ColourCycle;

extern void colour_cycle_reset(M068ColourCycle *cycle);
extern void func_80057D58(M068ColourCycle *cycle, s32 updateRate); /* steps a colour cycle */
extern M068ColourCycle rl_m068_1248C; /* (.data) the boss's four colour cycles */
extern M068ColourCycle rl_m068_1249C;
extern M068ColourCycle rl_m068_124AC;
extern M068ColourCycle rl_m068_124BC;

/* What func_80005F14 spawns the falling objects (0x13B) from */
typedef struct M068DropEntry {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
} M068DropEntry;

extern void cam_shake_start(s32 index, f32 phase1, f32 phase2, f32 phase3, s32 strength);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */

/* The boss's helpers */
#ifdef NON_MATCHING
static void func_m068_039D4(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_03A78(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_03D58(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_03F0C(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
extern void rl_m068_040EC(Object *obj, M068State *s, s32 updateRate, f32 rate);
#ifdef NON_MATCHING
static void func_m068_04590(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_04600(Object *obj, M068State *s);
#endif
#ifdef NON_MATCHING
static void func_m068_046EC(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_04878(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_048C0(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_04938(Object *obj, M068State *s);
#define rl_m068_04938 func_m068_04938
#else
extern void rl_m068_04938(Object *obj, M068State *s);
#endif
#ifdef NON_MATCHING
static void func_m068_047F8(Object *obj, M068State *s);
#define rl_m068_047F8 func_m068_047F8
#else
extern void rl_m068_047F8(Object *obj, M068State *s);
#endif
#ifdef NON_MATCHING
static void func_m068_05028(Object *obj, M068State *s);
#define rl_m068_05028 func_m068_05028
#else
extern void rl_m068_05028(Object *obj, M068State *s);
#endif
extern s32 rl_m068_125DC;      /* (.data) how many landing points */
extern void rl_m068_04A60(Object *obj, M068State *s, s32 updateRate, f32 rate);
#ifdef NON_MATCHING
static void func_m068_05100(Object *obj, M068State *s, s32 updateRate, f32 rate);
#define rl_m068_05100 func_m068_05100
#else
extern void rl_m068_05100(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_053AC(Object *obj, M068State *s);
#define rl_m068_053AC func_m068_053AC
#else
extern void rl_m068_053AC(Object *obj, M068State *s);
#endif
#ifdef NON_MATCHING
static void func_m068_05570(Object *obj, M068State *s);
#define rl_m068_05570 func_m068_05570
#else
extern void rl_m068_05570(Object *obj, M068State *s);
#endif
#ifdef NON_MATCHING
static void func_m068_054A0(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_05668(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_0577C(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_059F0(Object *obj, M068State *s, s32 updateRate, f32 rate);
#define rl_m068_059F0 func_m068_059F0
#else
extern void rl_m068_059F0(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_05F60(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_06180(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_06244(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_06310(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_06590(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_087E0(Object *obj, M068State *s, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_092F0(Object *obj, s32 index, s32 on);
#endif
#ifdef NON_MATCHING
static void func_m068_0BDC4(Object *obj, M068State *s, s32 anim, f32 frame);
#endif
extern void rl_m068_0BE14(Object *obj, M068State *s, s32 updateRate, f32 rate);
extern void rl_m068_067B4(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_06AE0(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_06E14(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_0714C(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_0758C(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_078F4(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_07D68(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_083C8(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#ifdef NON_MATCHING
static void func_m068_08674(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#endif
#ifdef NON_MATCHING
static void func_m068_0873C(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#endif
extern void rl_m068_088DC(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#ifdef NON_MATCHING
static void func_m068_09088(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#define rl_m068_09088 func_m068_09088
#else
extern void rl_m068_09088(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#endif
extern void rl_m068_093B8(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_0A208(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_0A5D8(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_0ADC4(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
extern void rl_m068_0B4D0(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#ifdef NON_MATCHING
static void func_m068_0BB4C(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate);
#endif

/* The module's .data (still assembly) */
extern s16 rl_m068_12610;       /* set: find the centre's height again */

#ifdef NON_MATCHING
/*
 * The boss's control: its centre's height, its actions, its parts' damage and angles, and its joints.
 * Same length as the ROM; still differs in register choice (see the round 18 notes).
 */
void func_m068_024EC(Object *obj, s32 updateRate) {
    M068Instance *inst;
    void *model;
    M068State *s;
    s32 i;
    s32 j;
    f32 rate;
    f32 **heights;
    f32 **heights2;
    f32 h1;
    f32 h2;
    M068Section *sec;
    M068PartState *part;
    Object *player;
    s16 sum;
    M068Joint *joint;
    s32 r;
    f32 *dst;
    f32 *src;
    s32 k;

    inst = obj->modelInstances[obj->modelIndex];
    model = inst->model;
    s = obj->unk68;
    sJointCmdNext = (s16 *) sJointCmds;
    sBss12B38 = 0;
    s->unk1C = 0;
    s->unk60 = 0;
    s->unk50 = 0;
    s->unk54 = 0;
    s->unkC8 = 0;
    s->unk90 = s->unk96;
    s->unk92 = s->unk98;
    s->unk94 = s->unk9A;
    if (rl_m068_12610 != 0) {
        if (s->flags & 0x20) {
            func_8001B078(sCentre.x, sCentre.z, 0x800, &heights);
            h1 = **heights;
            sCentreY = h1;
        } else {
            r = func_80082D80(sCentre.x, sCentre.z, 0x800, &heights2);
            if (func_8001B078(sCentre.x, sCentre.z, 0x800, &heights)) {
                h1 = **heights;
            } else {
                h1 = -32000.0f;
            }
            if (r) {
                h2 = **heights2;
            } else {
                h2 = -32000.0f;
            }
            if (h1 < h2) {
                sCentreY = h2;
            } else {
                sCentreY = h1;
            }
        }
        rl_m068_12610 = 0;
    }
    player = func_8000E4D0(0)->unk68;
    if (s->flags & 0x20) {
        s->unk60 |= 0x10;
        if (((u8 *) player)[0x568] == 0x2A && !(s->flags & 0x200)) {
            func_m068_04600(obj, s);
        }
        switch (s->unkAA) {
            case 0:
                rate = updateRate;
                func_m068_039D4(obj, s, updateRate, rate);
                break;
            case 10:
                rate = updateRate;
                break;
            case 11:
                rate = updateRate;
                func_m068_03D58(obj, s, updateRate, rate);
                break;
            case 12:
                rate = updateRate;
                func_m068_03F0C(obj, s, updateRate, rate);
                break;
            case 13:
                rate = updateRate;
                rl_m068_040EC(obj, s, updateRate, rate);
                break;
            case 14:
                rate = updateRate;
                func_m068_04590(obj, s, updateRate, rate);
                break;
            case 15:
                rate = updateRate;
                func_m068_046EC(obj, s, updateRate, rate);
                break;
            case 0xFF:
                rate = updateRate;
                break;
            default:
                rate = updateRate;
                break;
        }
        if (s->unkA8 != 0 && s->unkA8 == 3) {
            func_m068_03A78(obj, s, updateRate, rate);
        }
        switch (s->unkB0) {
            case 1:
                rl_m068_067B4(obj, s, model, inst, updateRate, rate);
                break;
            case 2:
                rl_m068_06E14(obj, s, model, inst, updateRate, rate);
                break;
            case 3:
                rl_m068_0758C(obj, s, model, inst, updateRate, rate);
                break;
            case 6:
                func_m068_08674(obj, s, model, inst, updateRate, rate);
                break;
            case 7:
                func_m068_0873C(obj, s, model, inst, updateRate, rate);
                break;
            case 0:
                break;
        }
    } else {
        s->unk60 |= 0x10;
        if (((u8 *) player)[0x568] == 0x2A) {
            func_m068_092F0(obj, 2, 0);
        }
        switch (s->unkAA) {
            case 0:
                rate = updateRate;
                func_m068_04878(obj, s, updateRate, rate);
                break;
            case 1:
                rate = updateRate;
                func_m068_048C0(obj, s, updateRate, rate);
                break;
            case 2:
                rate = updateRate;
                rl_m068_05100(obj, s, updateRate, rate);
                break;
            case 3:
                rate = updateRate;
                func_m068_054A0(obj, s, updateRate, rate);
                break;
            case 4:
                rate = updateRate;
                rl_m068_059F0(obj, s, updateRate, rate);
                break;
            case 5:
                rate = updateRate;
                rl_m068_04A60(obj, s, updateRate, rate);
                break;
            case 6:
                rate = updateRate;
                func_m068_05F60(obj, s, updateRate, rate);
                break;
            case 7:
                rate = updateRate;
                func_m068_06180(obj, s, updateRate, rate);
                break;
            case 8:
                rate = updateRate;
                func_m068_05668(obj, s, updateRate, rate);
                break;
            case 9:
                rate = updateRate;
                func_m068_0577C(obj, s, updateRate, rate);
                break;
            case 15:
                rate = updateRate;
                func_m068_046EC(obj, s, updateRate, rate);
                break;
            case 0xFF:
                rate = updateRate;
                func_m068_06244(obj, s, updateRate, rate);
                break;
            default:
                rate = updateRate;
                break;
        }
        switch (s->unkA8) {
            case 0:
                break;
            case 1:
                func_m068_06310(obj, s, updateRate, rate);
                break;
            case 2:
                func_m068_06590(obj, s, updateRate, rate);
                break;
        }
        switch (s->unkB0) {
            case 1:
                rl_m068_06AE0(obj, s, model, inst, updateRate, rate);
                break;
            case 2:
                rl_m068_0714C(obj, s, model, inst, updateRate, rate);
                break;
            case 3:
                rl_m068_078F4(obj, s, model, inst, updateRate, rate);
                break;
            case 4:
                rl_m068_07D68(obj, s, model, inst, updateRate, rate);
                break;
            case 5:
                rl_m068_083C8(obj, s, model, inst, updateRate, rate);
                break;
            case 0:
                break;
        }
        func_m068_087E0(obj, s, updateRate, rate);
    }
    rl_m068_088DC(obj, s, model, inst, updateRate, rate);
    if (((M068Hits *) obj->interactObj)->hitPoints != 32000) {
        for (i = 0; i != 11; i++) {
            sec = &rl_m068_1254C[i];
            if (sec->first >= 0) {
                part = &sParts[i];
                if (part->alive) {
                    sum = 0;
                    for (k = sec->first; k <= sec->last; k++) {
                        joint = &((M068Hits *) obj->interactObj)->joints[k];
                        sum += joint->damage;
                        joint->damage = 0;
                    }
                    if (sum != 0) {
                        part->hit = 1;
                        part->hp -= sum;
                        s->unkC8 += sum;
                    }
                }
            }
        }
        ((M068Hits *) obj->interactObj)->hitPoints = 32000;
    }
    if (!(s->flags & 0x200)) {
        if (s->flags & 0x20) {
            rl_m068_0A5D8(obj, s, model, inst, updateRate, rate);
            rl_m068_0ADC4(obj, s, model, inst, updateRate, rate);
            rl_m068_0B4D0(obj, s, model, inst, updateRate, rate);
            if (s->unkC8 > 500) {
                s->unk50 |= 0x1000;
            }
        } else {
            rl_m068_093B8(obj, s, model, inst, updateRate, rate);
        }
    }
    rl_m068_0A208(obj, s, model, inst, updateRate, rate);
    if (!(obj->trans.flags & 0x400)) {
        s->unk18 -= rate / 60.0f;
        if (s->unk18 > 0.0f) {
            s->unk1C |= 1;
        } else {
            s->unk18 = 0.0f;
        }
        obj->particleEmittersEnabled = s->unk1C;
        func_8005F5AC(obj, updateRate);
        obj->trans.rotation.x_rotation = 0;
        obj->trans.rotation.z_rotation = 0;
    }
    obj->trans.rotation.y_rotation = s->unk9A;
    *sJointCmdNext++ = rl_m068_1254C[0].joint * 6;
    *sJointCmdNext++ = s->unk98;
    *sJointCmdNext++ = rl_m068_1254C[0].joint * 6 + 4;
    *sJointCmdNext++ = s->unk96;
    for (i = 0; i < 11; i++) {
        if (sParts[i].target[0] != sParts[i].angle[0]) {
            sParts[i].angle[0] = angle_lerp(sParts[i].angle[0], sParts[i].target[0], 1.0f - func_80049BEC(sParts[i].rate[0], updateRate));
        }
        if (sParts[i].target[1] != sParts[i].angle[1]) {
            sParts[i].angle[1] = angle_lerp(sParts[i].angle[1], sParts[i].target[1], 1.0f - func_80049BEC(sParts[i].rate[1], updateRate));
        }
        if (sParts[i].target[2] != sParts[i].angle[2]) {
            sParts[i].angle[2] = angle_lerp(sParts[i].angle[2], sParts[i].target[2], 1.0f - func_80049BEC(sParts[i].rate[2], updateRate));
        }
        if (sParts[i].angle[0] != 0) {
            *sJointCmdNext++ = rl_m068_1254C[i].joint * 6 + 2;
            *sJointCmdNext++ = sParts[i].angle[0];
        }
        if (sParts[i].angle[1] != 0) {
            *sJointCmdNext++ = rl_m068_1254C[i].joint * 6;
            *sJointCmdNext++ = sParts[i].angle[1];
        }
        if (sParts[i].angle[2] != 0) {
            *sJointCmdNext++ = rl_m068_1254C[i].joint * 6 + 4;
            *sJointCmdNext++ = sParts[i].angle[2];
        }
        if (sParts[i].scale != 1.0f) {
            sec = &rl_m068_1254C[i];
            *sJointCmdNext++ = (sec->joint * 6) | 0x4000;
            *sJointCmdNext++ = (u16) (sParts[i].scale * 32768.0f);
            *sJointCmdNext++ = (sec->joint * 6 + 2) | 0x4000;
            *sJointCmdNext++ = (u16) (sParts[i].scale * 32768.0f);
            *sJointCmdNext++ = (sec->joint * 6 + 4) | 0x4000;
            *sJointCmdNext++ = (u16) (sParts[i].scale * 32768.0f);
        }
    }
    rl_m068_0BE14(obj, s, updateRate, rate);
    *sJointCmdNext++ = 0x1000;
    rl_m068_09088(obj, s, model, inst, updateRate, rate);
    func_m068_0BB4C(obj, s, model, inst, updateRate, rate);
    func_8003D21C(inst, model, obj, 1.0f, 0.0f);
    func_800221F0(obj, inst, obj->shading, inst->frames[inst->frame]);
    obj_tex_animate(obj, model, inst, inst->hidden);
    inst->hidden = 0;
    for (i = 0; i < 11; i++) {
        sParts[i].hit = 0;
    }
    dst = sJointPos;
    for (i = 0; i < 42; i++) {
        src = (f32 *) &inst->frames[inst->frame][i];
        dst[0] = src[12];
        dst[1] = src[13];
        dst[2] = src[14];
        dst += 3;
    }
    if (sParts[2].scale != 0.01f && !(s->flags & 0x20)) {
        src = (f32 *) &inst->frames[inst->frame][rl_m068_1254C[2].joint];
        for (k = 0; k < 12; k++) {
            ((f32 *) sMtx12B70)[k] = src[k];
        }
    }
    if (sParts[3].scale != 0.01f && !(s->flags & 0x20)) {
        src = (f32 *) &inst->frames[inst->frame][rl_m068_1254C[3].joint];
        for (k = 0; k < 12; k++) {
            ((f32 *) sMtx12BB0)[k] = src[k];
        }
    }
    if (sParts[6].scale != 0.01f && (s->flags & 0x20)) {
        src = (f32 *) &inst->frames[inst->frame][rl_m068_1254C[6].joint];
        dst = (f32 *) sMtx12C18;
        for (i = 0; i < 9; i++) {
            for (k = 0; k < 12; k++) {
                dst[k] = src[k];
            }
            dst += 16;
            src += 16;
        }
    }
    if (sParts[5].scale != 0.01f && (s->flags & 0x20)) {
        src = (f32 *) &inst->frames[inst->frame][rl_m068_1254C[5].joint];
        dst = (f32 *) sMtx12E68;
        for (i = 0; i < 3; i++) {
            for (k = 0; k < 12; k++) {
                dst[k] = src[k];
            }
            dst += 16;
            src += 16;
        }
    }
    if (sParts[4].scale != 0.01f) {
        if (s->flags & 0x20) {
            src = (f32 *) &inst->frames[inst->frame][rl_m068_1254C[4].joint];
            for (k = 0; k < 12; k++) {
                ((f32 *) sMtx12F30)[k] = src[k];
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_024EC);
#endif

/* Phase 0 of the second form: when its event comes, parts 4-6 go and the player's control is handed back */
#ifdef NON_MATCHING
static void func_m068_039D4(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    M068Player *player;

    if (s->events & 0x8000) {
        func_m068_092F0(obj, 4, 1);
        s->unkA8 = 3;
        s->unkAA = 10;
        if (s->unkF == 0) {
            func_m068_092F0(obj, 6, 1);
            func_m068_092F0(obj, 5, 1);
            player = func_8000E4D0(0)->unk68;
            player->unk18E = 0;
            player->unk5C8 = updateRate;
            func_m006_015E0(0);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_039D4);
#endif

/* The second form's head (part 1) turns to the player; picks its next attack animation */
#ifdef NON_MATCHING
static void func_m068_03A78(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    Object *player;
    Vec3f *head;
    f32 dx;
    f32 dz;
    s32 anim;
    M068PartState *parts;

    player = func_8000E4D0(0);
    head = (Vec3f *) &sJointPos[rl_m068_1254C[1].joint * 3];
    dx = player->trans.x_position - head->x;
    dz = player->trans.z_position - head->z;
    parts = sParts;
    parts[1].rate[0] = 0.9f;
    if (s->unkB0 == 1 || s->unkB0 == 6 || s->unkB0 == 7) {
        parts[1].target[0] = 0;
    } else {
        parts[1].target[0] = func_80049C84(dx, dz) - s->unk9A + 0x8000;
    }
    if (s->unkB6 == 0 && ((s->events & 0x8000) || s->unkB0 == 0)) {
        if (s->flags & 0x80) {
            anim = func_80048E2C(10, 29);
        } else {
            anim = func_80048E2C(0, 29);
        }
        if (anim < 10) {
            anim = 1;
        } else if (anim < 20) {
            anim = 2;
        } else if (anim < 30) {
            anim = 3;
        }
        s->unkBC = 0;
        s->unkBE = 0;
        s->unkB0 = anim;
        s->timer = 0.0f;
    } else if (s->unkB6 == 1 && ((s->events & 0x8000) || s->unkB0 == 0)) {
        if ((s->flags & 0x80) && (s->flags & 0x40)) {
            anim = 7;
        } else {
            anim = 6;
        }
        s->unkBC = 0;
        s->unkBE = 0;
        s->timer = 0.0f;
        s->unkB0 = anim;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_03A78);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03C70);

/* Phase 11 of the second form */
#ifdef NON_MATCHING
static void func_m068_03D58(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    M068Player *player;

    sParts[1].target[0] = 0;
    if (s->events & 0x800) {
        sParts[6].hit = 1;
    } else if (s->events & 0x8000) {
        func_m068_092F0(obj, 4, 1);
        if (!(s->flags & 0x80)) {
            func_m068_092F0(obj, 5, 1);
        }
        s->unkA8 = 3;
        s->unkAA = 10;
        player = func_8000E4D0(0)->unk68;
        player->unk18E = 0;
        player->unk5C8 = updateRate;
        func_m006_015E0(0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03D58);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03E24);

/* Phase 12 of the second form */
#ifdef NON_MATCHING
static void func_m068_03F0C(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    M068Player *player;

    sParts[1].target[0] = 0;
    if (s->events & 0x1000) {
        sParts[5].hit = 1;
    } else if (s->events & 0x8000) {
        func_m068_092F0(obj, 4, 1);
        if (!(s->flags & 0x40)) {
            func_m068_092F0(obj, 6, 1);
        }
        s->unkA8 = 3;
        s->unkAA = 10;
        player = func_8000E4D0(0)->unk68;
        player->unk18E = 0;
        player->unk5C8 = updateRate;
        func_m006_015E0(0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03F0C);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_03FD8);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_040EC);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0447C);

/* Phase 14 of the second form: its event ends the level */
#ifdef NON_MATCHING
static void func_m068_04590(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    UNUSED s32 pad;

    sParts[1].target[0] = 0;
    if (s->events & 0x4000) {
        level_goto(0x184, func_800468CC(), 0, menu_mode_get(), 1, 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04590);
#endif

/* The boss is beaten: parts 2 and 4-6 go and the cut camera looks at joint section 1 */
#ifdef NON_MATCHING
static void func_m068_04600(Object *obj, M068State *s) {
    UNUSED s32 pad;
    M068Player *player;

    player = func_8000E4D0(0)->unk68;
    s->unkAA = 15;
    s->unkA8 = 0;
    s->flags |= 0x200;
    chain_object_unlink_from_players(obj);
    func_m068_092F0(obj, 4, 0);
    func_m068_092F0(obj, 6, 0);
    func_m068_092F0(obj, 5, 0);
    func_m068_092F0(obj, 2, 0);
    player->unk18E = 1;
    func_m006_015E0(1);
    rl_m068_1261C = (Vec3f *) &sJointPos[rl_m068_1254C[1].joint * 3];
    func_m068_0BDC4(obj, s, 0x31, 0.0f);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04600);
#endif

/* Phase 15: parts 5 and 6 grow back to full size */
#ifdef NON_MATCHING
static void func_m068_046EC(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    M068PartState *parts = sParts;

    parts[1].target[0] = 0;
    if (parts[6].scale != 1.0f) {
        parts[6].scale += (1.0f - parts[6].scale) * (1.0f - func_80049BEC(0.97f, updateRate));
        if (parts[6].scale > 0.97f) {
            parts[6].scale = 1.0f;
        }
    }
    parts = sParts; /* (set again: the ROM rebuilds the base after the call) */
    if (parts[5].scale != 1.0f) {
        parts[5].scale += (1.0f - parts[5].scale) * (1.0f - func_80049BEC(0.97f, updateRate));
        if (parts[5].scale > 0.97f) {
            parts[5].scale = 1.0f;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_046EC);
#endif

#ifdef NON_MATCHING
/* The player is caught: the cut camera looks at the boss, which jumps to its start */
/* Matches; its only C caller is 059F0 (NON_MATCHING), so it stays asm until 059F0 is C. */
static void func_m068_047F8(Object *obj, M068State *s) {
    M068Player *player;

    player = func_8000E4D0(0)->unk68;
    s->flags |= 0x200;
    player->unk18E = 1;
    func_m006_015E0(1);
    rl_m068_1261C = &s->pos;
    chain_object_unlink_from_players(obj);
    func_m068_092F0(obj, 2, 0);
    rl_m068_05028(obj, s);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_047F8);
#endif

/* Phase 0 of the first form */
#ifdef NON_MATCHING
static void func_m068_04878(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    if (s->events & 0x8000) {
        s->unkBE = 0;
        s->unkBC = 0;
        s->unkB0 = 5;
        s->unkB4 = 1;
        s->unkAA = 7;
        s->timer = 0.0f;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04878);
#endif

/* Phase 1 of the first form */
#ifdef NON_MATCHING
static void func_m068_048C0(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    if (s->unkAE == 0) {
        s->unkA8 = 1;
        s->unk40 = 500.0f;
    } else if (s->unkAE == 3) {
        rl_m068_04938(obj, s);
        ((u8 *) obj->shadow)[0xC] = 0;
    }
    s->unk80 = 255;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_048C0);
#endif

#ifdef NON_MATCHING
/* Phase 1 of the first form, step 3: it jumps to the nearer of two walls */
/* 72 diffs: the ROM has an 8-word local block (frame 0x38, here 0x18), stores the second plane distance in it and loads both plane coordinates before negating. */
static void func_m068_04938(Object *obj, M068State *s) {
    f32 *plane;
    f32 a;
    f32 b;

    plane = sPlanes[0];
    a = obj->trans.x_position * sPlanes[0][0] + obj->trans.z_position * sPlanes[0][2] + sPlanes[0][3];
    b = obj->trans.x_position * sPlanes[1][0] + obj->trans.z_position * sPlanes[1][2] + sPlanes[1][3];
    if (a < b) {
        plane = sPlanes[1];
        s->unkBA = 1;
        s->unkA8 = 1;
        s->unkBC = 0;
        s->unk34 = 10.0f;
    } else {
        s->unkBA = 0;
        s->unkA8 = 2;
        s->unkBC = 1;
        s->unk34 = 20.0f;
    }
    s->unk24 = -plane[0];
    s->unk28 = -plane[2];
    s->unk20 = 1.0f;
    func_m068_092F0(obj, 2, 1);
    s->unk14 = 0;
    s->unkAC = 2;
    s->unkAA = 5;
    s->unkB0 = 1;
    s->timer = 0.0f;
    s->unk40 = 310.0f;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_04938);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_04A60);

#ifdef NON_MATCHING
/* Starts the first form's jump (phase 2); with flag 8 the player watches it from joint section 2 */
/* Matches; its only C caller is 047F8 (only C with 059F0), so it stays asm until 059F0 is C. */
static void func_m068_05028(Object *obj, M068State *s) {
    M068Player *player;

    s->unk40 = 500.0f;
    func_m068_092F0(obj, 2, 0);
    s->unkBC = 0;
    s->unkAC = 2;
    s->unkB0 = 0;
    s->unkA8 = 1;
    s->unkAA = 2;
    s->unk34 = 20.0f;
    if (s->flags & 8) {
        s->unk9E = rl_m068_125E4;
        player = func_8000E4D0(0)->unk68;
        player->unk18E = 1;
        player->unk5C8 = 0x7D00;
        func_m006_015E0(1);
        rl_m068_1261C = (Vec3f *) &sJointPos[rl_m068_1254C[2].joint * 3];
    } else {
        s->unk9E = s->unkC;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05028);
#endif

#ifdef NON_MATCHING
/* Phase 2 of the first form: slides to a stop, turning its body into the motion, then turns to unk9E */
/* 20 diffs: from the else branch on the t-registers are two lower than here, and s->unk9E/unk9A load in the other order. */
static void func_m068_05100(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    f32 dir[2];
    f32 rot[2];
    f32 speed;
    f32 dot;

    speed = func_80097140(s->unk2C * s->unk2C + s->unk30 * s->unk30);
    if (speed > 0.0f) {
        dir[0] = s->unk2C / speed;
        dir[1] = s->unk30 / speed;
        s->unk20 = 1.0f;
        s->unk24 = -dir[0];
        s->unk28 = -dir[1];
        if (speed <= rate) {
            s->unk20 = 0.0f;
            s->unk2C = 0.0f;
            s->unk30 = 0.0f;
            speed = 0.0f;
        }
        rot[0] = -func_80049C34(s->unk9A);
        rot[1] = -func_80049C30(s->unk9A);
        dot = rot[0] * dir[0] + rot[1] * dir[1];
        s->unk98 = rl_m068_0C7DC(s->unk98, -204.8f * speed * dot, 0x100, updateRate);
        rot[0] = -func_80049C34(s->unk9A + 0x4000);
        rot[1] = -func_80049C30(s->unk9A + 0x4000);
        dot = rot[0] * dir[0] + rot[1] * dir[1];
        s->unk96 = rl_m068_0C7DC(s->unk96, 204.8f * speed * dot, 0x100, updateRate);
        s->unk80 = (s32) (7.75f * speed) + 100;
    } else {
        if (s->flags & 8) {
            if (s->unk9E == s->unk9A) {
                rl_m068_05570(obj, s);
            }
        } else if (s->unk9E == s->unk9A && obj->animationID == 0xD) {
            rl_m068_053AC(obj, s);
        }
        s->unk80 = 100;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_05100);
#endif

#ifdef NON_MATCHING
/* Jumps towards the boss's own target point (pos), or straight up if it is there */
/* Matches; its only C caller is 05100 (NON_MATCHING), so it stays asm until 05100 is C. */
static void func_m068_053AC(Object *obj, M068State *s) {
    f32 dx;
    f32 dz;
    f32 d;

    dx = s->pos.x - obj->trans.x_position;
    dz = s->pos.z - obj->trans.z_position;
    d = func_80097140(dx * dx + dz * dz);
    if (d > 0.0f) {
        dx /= d;
        dz /= d;
        s->unkAC = 2;
        s->unk20 = 1.0f;
        s->unk24 = dx;
        s->unkC0 = -dx;
        s->unk28 = dz;
        s->unkC4 = -dz;
    } else {
        s->unkAC = 1;
    }
    func_m068_092F0(obj, 2, 0);
    s->unkBC = 0;
    s->unkB0 = 0;
    s->unkA8 = 1;
    s->unkAA = 3;
    s->unk40 = 500.0f;
    s->unk34 = 20.0f;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_053AC);
#endif

/* Phase 3 of the first form */
#ifdef NON_MATCHING
static void func_m068_054A0(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    s->unk60 |= 0xF;
    if (s->unkAC == 1) {
        ((u8 *) obj->shadow)[0xC] = s->unkE;
        func_m068_0BDC4(obj, s, 0x12, 0.0f);
        s->unkAA = 6;
        s->unk80 = 100;
    } else if (s->unkAC == 2) {
        if (rl_m068_05C8C(obj, s, &s->pos, s->unkC, updateRate, rate) == 0) {
            obj->trans.x_position = s->pos.x;
            obj->trans.z_position = s->pos.z;
            s->unkAC = 1;
            s->unk20 = 0.0f;
            s->unk2C = 0.0f;
            s->unk30 = 0.0f;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_054A0);
#endif

#ifdef NON_MATCHING
/* Jumps towards the drop point sDropPoint (phase 8) */
/* Matches; its only C caller is 05100 (NON_MATCHING), so it stays asm until 05100 is C. */
static void func_m068_05570(Object *obj, M068State *s) {
    f32 dx;
    f32 dz;
    f32 d;

    dx = sDropPoint.x - obj->trans.x_position;
    dz = sDropPoint.z - obj->trans.z_position;
    d = func_80097140(dx * dx + dz * dz);
    if (d > 0.0f) {
        dx /= d;
        dz /= d;
    }
    s->unk20 = 1.0f;
    s->unk24 = dx;
    s->unk28 = dz;
    s->unkC0 = -dx;
    s->unkC4 = -dz;
    s->unk40 = sDropPoint.y;
    chain_object_unlink_from_players(obj);
    func_m068_092F0(obj, 2, 0);
    s->unkBC = 0;
    s->unkAC = 2;
    s->unkB0 = 0;
    s->unkA8 = 2;
    s->unkAA = 8;
    s->unk34 = 5.0f;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05570);
#endif

/* Phase 8 of the first form */
#ifdef NON_MATCHING
static void func_m068_05668(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    if (s->unkAC == 1) {
        if (s->events & 0x400) {
            sParts[2].hit = 1;
        } else if (obj->animationID == 0x23) {
            ((u8 *) obj->shadow)[0xC] = s->unkE;
            s->unkAA = 9;
            s->unkAE = 0;
            s->unkA8 = 0;
            s->unkAC = 0;
            s->unk80 = 100;
            s->unk3C = 0.0f;
            rl_m068_1261C = (Vec3f *) &rl_m068_12508->trans.x_position;
        }
    } else if (s->unkAC == 2) {
        if (rl_m068_05C8C(obj, s, &sDropPoint, rl_m068_125E4, updateRate, rate) == 0) {
            obj->trans.x_position = sDropPoint.x;
            obj->trans.z_position = sDropPoint.z;
            s->unkAC = 1;
            s->unk20 = 0.0f;
            s->unk2C = 0.0f;
            s->unk30 = 0.0f;
            func_m068_0BDC4(obj, s, 0x22, 0.0f);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05668);
#endif

/* Phase 9 of the first form: it falls; objects 0x13B drop at four points and the second form starts */
#ifdef NON_MATCHING
static void func_m068_0577C(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    Object *o;
    Vec3f *p;
    s32 i;
    M068DropEntry entry;

    if (s->unkB8 != 0) {
        if ((o = rl_m068_12508) != NULL) {
            s32 *st = o->unk68;

            *st = 1;
            rl_m068_12508 = NULL;
            rl_m068_12610 = 1;
        }
        cam_shake_start(0, 0.0f, 0.0f, 4.0f, 25);
        func_80003410(0x1CC, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        entry.common.objectID = 0x13B;
        entry.unkA = 0;
        entry.unkC = -0x4000;
        entry.unkE = 30;
        entry.unk10 = 0.5f;
        for (i = 0, p = sDropPoints; i != 4; i++, p++) {
            entry.common.x = p->x;
            entry.common.y = p->y;
            entry.common.z = p->z;
            o = func_80005F14(&entry, 1);
            if (o != NULL) {
                o->level_entry = NULL;
            }
        }
        s->unk14 = 0;
        s->unkBE = 0;
        s->unkBC = 0;
        s->unkA8 = 0;
        s->unkB0 = 0;
        s->unkAA = 0;
        s->flags |= 0x20;
        s->timer = 0.0f;
        rl_m068_1261C = (Vec3f *) &sJointPos[rl_m068_1254C[4].joint * 3];
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0577C);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_05964);

#ifdef NON_MATCHING
/* Phase 4 of the first form: jumps between the landing points; the player in reach is caught */
/* 166 diffs: the ROM keeps dx in f20 (the same register later holds the 0.0f stores) and has an 8-word local block (frame 0x50, here 0x40). */
static void func_m068_059F0(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    Object *player;
    M068Player *pstate;
    Vec3f *target;
    s32 n;
    f32 dx;
    f32 dz;
    f32 d;
    s16 r;
    s16 anim;

    s->unk60 |= 0xF;
    player = func_8000E4D0(0);
    pstate = player->unk68;
    if (s->unkAC == 1) {
        if (s->unkB0 == 0) {
            if (pstate->unk568 == 0x2A && !(s->flags & 0x200)) {
                rl_m068_047F8(obj, s);
            } else if (s->unk14 == 3) {
                rl_m068_04938(obj, s);
            } else {
                do {
                    n = func_80048E2C(0, rl_m068_125DC - 1);
                } while (n == s->unkBA);
                target = &sLandPoints[n];
                dx = target->x - obj->trans.x_position;
                dz = target->z - obj->trans.z_position;
                d = func_80097140(dx * dx + dz * dz);
                dx /= d;
                dz /= d;
                s->unk20 = 1.0f;
                s->unk24 = dx;
                s->unkC0 = -dx;
                s->unk28 = dz;
                s->unkC4 = -dz;
                s->unkAC = 2;
                s->unk14++;
                s->unkBA = n;
                s->unk40 = target->y;
            }
        }
        s->unk80 = 100;
    } else if (s->unkAC == 2) {
        target = &sLandPoints[s->unkBA];
        if (rl_m068_05C8C(obj, s, target,
                            func_80049C84(player->trans.x_position - obj->trans.x_position,
                                          player->trans.z_position - obj->trans.z_position) + 0x8000,
                            updateRate, rate) == 0) {
            s->unk20 = 0.0f;
            s->unk2C = 0.0f;
            s->unk30 = 0.0f;
            anim = r = func_80048E2C(0, 29);
            if (r < 10) {
                anim = 2;
            } else if (r < 20) {
                anim = 3;
            } else if (r < 30) {
                anim = 4;
            }
            s->unkBC = 0;
            s->unkBE = 0;
            s->timer = 0.0f;
            s->unkB0 = anim;
            s->unkAC = 1;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_059F0);
#endif

#ifdef NON_MATCHING
/* Steers the first form towards target: slows to stop there, turns its body (unk98/unk96) into the motion; 1 while it moves */
/* 5 diffs: the first dot product loads s->unkC4 before dir[1] in the ROM (operands of the multiply swapped). */
static s32 func_m068_05C8C(Object *obj, M068State *s, Vec3f *target, s16 yaw, s32 updateRate, f32 rate) {
    f32 dir[2];
    f32 rot[2];
    f32 speed;
    f32 dx;
    f32 dz;
    f32 dist;
    s32 result;
    UNUSED s32 pad;
    UNUSED s32 pad2;
    f32 along;

    speed = func_80097140(s->unk2C * s->unk2C + s->unk30 * s->unk30);
    if (speed > 0.0f) {
        dir[0] = s->unk2C / speed;
        dir[1] = s->unk30 / speed;
    } else {
        dir[0] = 0.0f;
        dir[1] = 0.0f;
    }
    along = dir[0] * s->unkC0 + s->unkC4 * dir[1];
    dx = obj->trans.x_position - target->x;
    dz = obj->trans.z_position - target->z;
    dist = func_80097140(dx * dx + dz * dz);
    if (speed * along < 0.0f) {
        along = s->unkC4 * s->unk28 + s->unk24 * s->unkC0;
        if (dist <= -(speed * speed) / (along + along)) {
            s->unk24 = -dir[0];
            s->unk28 = -dir[1];
            s->unk20 = 1.0f;
        }
        s->unk9E = yaw;
        rot[0] = -func_80049C34(s->unk9A);
        rot[1] = -func_80049C30(s->unk9A);
        along = rot[0] * dir[0] + rot[1] * dir[1];
        s->unk98 = rl_m068_0C7DC(s->unk98, -204.8f * speed * along, 0x100, updateRate);
        rot[0] = -func_80049C34(s->unk9A + 0x4000);
        rot[1] = -func_80049C30(s->unk9A + 0x4000);
        along = rot[0] * dir[0] + rot[1] * dir[1];
        s->unk96 = rl_m068_0C7DC(s->unk96, 204.8f * speed * along, 0x100, updateRate);
        result = 1;
    } else {
        result = 0;
    }
    s->unk80 = (s32) (7.75f * speed) + 100;
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_05C8C);
#endif

/* Phase 6 of the first form: when it lands from its jump, objects 0x13B drop at four points */
#ifdef NON_MATCHING
static void func_m068_05F60(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    Object *o;
    Vec3f *p;
    M068DropEntry entry;
    s32 i;

    if (s->events & 0x20) {
        s->unk40 = s->pos.y + 269.0f;
    } else if (s->unkB8 != 0 && s->unk82 != 0) {
        s->unk82 = 0;
        s->unk83 = 0;
        s->unkA8 = 0;
        s->unk3C = -10.0f;
        cam_shake_start(0, 0.0f, 0.0f, 2.0f, 15);
        func_80003410(0x1CC, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        func_m068_0BDC4(obj, s, 0x13, 0.0f);
        entry.common.objectID = 0x13B;
        entry.unkA = 0;
        entry.unkC = -0x4000;
        entry.unkE = 30;
        entry.unk10 = 0.5f;
        for (i = 0, p = sDropPoints; i != 4; i++, p++) {
            entry.common.x = p->x;
            entry.common.y = p->y;
            entry.common.z = p->z;
            o = func_80005F14(&entry, 1);
            if (o != NULL) {
                o->level_entry = NULL;
            }
        }
    } else if (s->unkB8 != 0 && (s->events & 0x8000)) {
        if (s->flags & 0x200) {
            s->unkB0 = 0;
            s->unk3C = 0.0f;
            func_m068_04600(obj, s);
        } else {
            s->unkAA = 7;
            s->unkB4 = 0;
            s->unk3C = 0.0f;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_05F60);
#endif

/* Phase 7 of the first form */
#ifdef NON_MATCHING
static void func_m068_06180(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    M068Player *player;

    if (s->unkB4 == 0) {
        s->unkBE = 0;
        s->unkBC = 0;
        s->unkB0 = 5;
        s->unkB4 = 1;
        s->timer = 0.0f;
    } else if (s->unkB4 == 1 && s->unkB0 == 0) {
        player = func_8000E4D0(0)->unk68;
        if (player->unk568 == 0x2A && !(s->flags & 0x200)) {
            s->unkB0 = 0;
            func_m068_04600(obj, s);
        } else {
            s->unkAE = 0;
            s->unkAC = 0;
            s->unkAA = 1;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06180);
#endif

/* Phase 0xFF of the first form */
#ifdef NON_MATCHING
static void func_m068_06244(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    if (rl_m068_125E0 != 0) {
        if (s->unkA8 == 0) {
            obj->trans.x_position = sBss13678.x;
            obj->trans.z_position = sBss13678.z;
            s->unk40 = sBss13678.y;
            func_m068_0BDC4(obj, s, 13, 0.0f);
            s->unk82 = 1;
            ((u8 *) obj->shadow)[0xC] = 0;
            s->unk9A = 0;
            s->unkA8 = 2;
            s->unk80 = 255;
        } else {
            s->unk9E = func_8000E4D0(0)->trans.rotation.y_rotation;
            func_m068_092F0(obj, 2, 1);
        }
        s->unk60 |= 0x1F;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06244);
#endif

/* A jump of the first form: takes off on event 8 with the object's vertical speed, lands at unk40 */
#ifdef NON_MATCHING
static void func_m068_06310(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    f32 g;
    f32 off;
    f32 limit;
    f32 d;

    if (s->unkAE == 1 && s->unk38 <= 0.0f) {
        s->unkAE = 3;
    } else if (s->unkAE == 2 && 0.0f <= s->unk38) {
        s->unkAE = 3;
    }
    if (s->unkAE == 0) {
        if (obj->animationID != 0xE) {
            func_m068_0BDC4(obj, s, 0xE, 0.0f);
        } else {
            if (s->events & 4) {
                s->unk82 = 1;
            }
            if (s->events & 8) {
                if (0.0f < obj->velocity.y) {
                    s->unk38 = obj->velocity.y;
                } else {
                    s->unk38 = 0.0f;
                }
                s->unkAE = 1;
            }
        }
    }
    if (s->unkAE != 0) {
        if (0.0f <= s->unk38) {
            g = -1.0f;
            off = 0.0f;
        } else {
            off = -s->unk44;
            g = 0.20000005f;
        }
        limit = -(s->unk38 * s->unk38) / (g + g);
        d = s->unk40 + off - obj->trans.y_position;
        if (d <= limit) {
            s->unk3C = 0.0f;
        } else {
            s->unk3C = 1.2f;
        }
    }
    if (s->unk82 != 0) {
        if (s->unk83 != 0) {
            s->unk7E = 0x3300;
        } else {
            s->unk7E = 0;
        }
        s->unk83 ^= 1;
    }
    if (s->unk7E != 0) {
        *sJointCmdNext++ = rl_m068_1254C[2].joint * sizeof(Vec3s) + 2;
        *sJointCmdNext++ = s->unk7E;
        *sJointCmdNext++ = rl_m068_1254C[3].joint * sizeof(Vec3s) + 2;
        *sJointCmdNext++ = -s->unk7E;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_06310);
#endif

/* Rises or falls to unk40 (speed unk38, acceleration factor unk3C); shakes joint sections 2 and 3 while unk82 is set */
#ifdef NON_MATCHING
static void func_m068_06590(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    f32 limit;
    f32 d;
    f32 g;
    f32 y;

    if (obj->trans.y_position != s->unk40) {
        if (0.0f < s->unk38) {
            g = -1.0f;
        } else {
            g = 0.20000005f;
        }
        limit = -(s->unk38 * s->unk38) / (g + g);
        d = s->unk40 - obj->trans.y_position;
        if (d <= limit) {
            s->unk3C = 0.0f;
        } else {
            s->unk3C = 1.2f;
        }
        y = s->unk3C + -1.0f;
        g = obj->trans.y_position + s->unk38 * rate + y * 0.5f * rate * rate;
        if (obj->trans.y_position <= s->unk40 && s->unk40 <= g) {
            obj->trans.y_position = s->unk40;
            s->unk38 = 0.0f;
            s->unk3C = 1.0f;
        } else if (s->unk40 <= obj->trans.y_position && g <= s->unk40) {
            if (!obj->trans.y_position) { /* (fake: no code; puts y in f2 as in the ROM) */
            }
            obj->trans.y_position = s->unk40;
            s->unk38 = 0.0f;
            s->unk3C = 1.0f;
        }
    } else {
        s->unk3C = 1.0f;
    }
    if (s->unk82 != 0) {
        if (s->unk83 != 0) {
            s->unk7E = 0x3300;
        } else {
            s->unk7E = 0;
        }
        s->unk83 ^= 1;
    }
    if (s->unk7E != 0) {
        *sJointCmdNext++ = rl_m068_1254C[2].joint * sizeof(Vec3s) + 2;
        *sJointCmdNext++ = s->unk7E;
        *sJointCmdNext++ = rl_m068_1254C[3].joint * sizeof(Vec3s) + 2;
        *sJointCmdNext++ = -s->unk7E;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_06590);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_067B4);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_06AE0);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_06E14);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0714C);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0758C);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_078F4);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_07D68);

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_083C8);

/* Animation 6 of the second form */
#ifdef NON_MATCHING
static void func_m068_08674(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate) {
    if (s->unkAA != 10) {
        s->unkB0 = 0;
        s->unkB6 = 0;
        s->timer = 0.0f;
    } else {
        if (s->unkBC == 0) {
            func_m068_0BDC4(obj, s, 0x24, 0.0f);
            s->unkBC |= 1;
            s->timer = 5.0f;
        }
        if (s->unkBC != 0) {
            s->timer -= rate / 60.0f;
            if (s->timer <= 0.0f) {
                s->unkB0 = 0;
                s->unkB6 = 0;
                s->timer = 0.0f;
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_08674);
#endif

/* Animation 7 of the second form */
#ifdef NON_MATCHING
static void func_m068_0873C(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate) {
    if (s->unkBC == 0) {
        func_m068_0BDC4(obj, s, 0x2E, 0.0f);
        s->unkBC |= 1;
        s->timer = 5.0f;
    }
    if (s->unkBC != 0) {
        s->timer -= rate / 60.0f;
        if (s->timer <= 0.0f) {
            s->unkB0 = 0;
            s->unkB6 = 0;
            s->timer = 0.0f;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0873C);
#endif

/* Turns the first form's yaw towards unk9E; snaps it there within 20 */
#ifdef NON_MATCHING
static void func_m068_087E0(Object *obj, M068State *s, s32 updateRate, f32 rate) {
    s32 a;
    s32 b;
    f32 d;

    if (s->unk9A != s->unk9E) {
        s->unk9A = angle_lerp(s->unk9A, s->unk9E, 1.0f - func_80049BEC(0.97f, updateRate));
        a = (u16) (s->unk9E - s->unk9A);
        b = (u16) (s->unk9A - s->unk9E);
        if (b < a) {
            d = -(f32) b;
        } else {
            d = a;
        }
        if (-20.0f <= d && d <= 20.0f) {
            s->unk9A = s->unk9E;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_087E0);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_088DC);

#ifdef NON_MATCHING
/* Runs the colour cycle cycleKind picks over the model's vertices cycleFirst..cycleEnd, until it ends */
/* 4 diffs: the end test loads cycle->unkC into t6 and cycle->unk0 into t7 in the ROM (swapped here). */
static void func_m068_09088(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate) {
    M068ColourCycle *cycle;
    M068DrawModel *m;
    s32 i;

    switch (s->cycleKind) {
        case 1:
            cycle = &rl_m068_1248C;
            break;
        case 2:
            cycle = &rl_m068_1249C;
            break;
        case 3:
            cycle = &rl_m068_124AC;
            break;
        case 4:
            cycle = &rl_m068_124BC;
            break;
        default:
            cycle = NULL;
            break;
    }
    if (cycle != NULL) {
        if (obj->state[0x9F - 0x88] == 0) {
            colour_cycle_reset(cycle);
            obj->state[0x9F - 0x88] = 2;
            s->cycle = cycle;
        }
        func_80057D58(cycle, updateRate);
        m = obj->modelInstances[obj->modelIndex];
        for (i = s->cycleFirst; i < s->cycleEnd; i++) {
            ((Vertex *) m->vertices)[i].r = cycle->r;
            ((Vertex *) m->vertices)[i].g = cycle->g;
            ((Vertex *) m->vertices)[i].b = cycle->b;
        }
        s->unk8A = updateRate << 10;
        if (*cycle->unkC == cycle->unk0 + 1) {
            s->cycleKind = 0;
            obj->state[0x9F - 0x88] = 0;
            s->cycle = NULL;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_09088);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_09218);

/* Turns joint section `index` on (it can be hit, 100 damage) or off */
#ifdef NON_MATCHING
static void func_m068_092F0(Object *obj, s32 index, s32 on) {
    M068Section *sec = &rl_m068_1254C[index];
    M068Hits *hits = (M068Hits *) obj->interactObj;
    s32 damage;
    s32 off;
    s32 j;
    M068Joint *joint;

    if (on) {
        damage = 100;
        off = 0;
        sParts[index].alive = 1;
    } else {
        damage = 0;
        off = 1;
        sParts[index].alive = 0;
    }
    for (j = sec->first; j <= sec->last; j++) {
        joint = &hits->joints[j];
        joint->unk2F = damage;
        joint->off = off;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_092F0);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_093B8);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0A208);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0A5D8);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0ADC4);

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0B4D0);

/* Starts the sound unk50's lowest set bit asks for at one of the model's points, or moves the one playing */
#ifdef NON_MATCHING
static void func_m068_0BB4C(Object *obj, M068State *s, void *model, M068Instance *inst, s32 updateRate, f32 rate) {
    s32 sound;
    s32 volume;

    if (s->sound58 == 0) {
        sound = -1;
        volume = 0;
        if (s->unk50 & 1) {
            s->soundPoint = 4;
            sound = 0x1CE;
            volume = 4;
        } else if (s->unk50 & 2) {
            s->soundPoint = 4;
            sound = 0x1CF;
            volume = 4;
        } else if (s->unk50 & 4) {
            s->soundPoint = 4;
            sound = 0x1C9;
            volume = 4;
        } else if (s->unk50 & 8) {
            s->soundPoint = 4;
            sound = 0x1CA;
            volume = 4;
        } else if (s->unk50 & 0x10) {
            s->soundPoint = 4;
            sound = 0x1CB;
            volume = 1;
        } else if (s->unk50 & 0x20) {
            s->soundPoint = 0x21;
            sound = 0x1D1;
            volume = 1;
        } else if (s->unk50 & 0x40) {
            s->soundPoint = 4;
            sound = 0x222;
            volume = 4;
            s->unk18 = 1.0f;
        } else if (s->unk50 & 0x80) {
            s->soundPoint = 4;
            sound = 0x223;
            volume = 4;
            s->unk18 = 1.0f;
        } else if (s->unk50 & 0x100) {
            s->soundPoint = 4;
            sound = 0x207;
            volume = 4;
        } else if (s->unk50 & 0x200) {
            s->soundPoint = 4;
            sound = 0x224;
            volume = 4;
        } else if (s->unk50 & 0x400) {
            s->soundPoint = 4;
            sound = 0x225;
            volume = 4;
        } else if (s->unk50 & 0x800) {
            s->soundPoint = 4;
            sound = 0x226;
            volume = 4;
        } else if (s->unk50 & 0x1000) {
            s->soundPoint = 4;
            sound = 0x227;
            volume = 4;
        } else if (s->unk50 & 0x2000) {
            s->soundPoint = 4;
            sound = 0x22B;
            volume = 4;
        }
        if (sound != -1) {
            func_80003410(sound, inst->points[s->soundPoint].x, inst->points[s->soundPoint].y,
                          inst->points[s->soundPoint].z, volume, &s->sound58);
        }
    } else if (s->unk54 != 0) {
        func_80003614(s->sound58);
    } else {
        func_800035E8(s->sound58, inst->points[s->soundPoint].x, inst->points[s->soundPoint].y,
                      inst->points[s->soundPoint].z);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BB4C);
#endif

/* Starts animation `anim` unless it is playing already (and counts the change) */
#ifdef NON_MATCHING
static void func_m068_0BDC4(Object *obj, M068State *s, s32 anim, f32 frame) {
    if (anim != obj->animationID) {
        obj_anim_set(obj, anim, frame);
        sBss12B38++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0BDC4);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0BE14);

#ifdef NON_MATCHING
/* Turns angle towards target by speed * updateRate the short way round */
/* Matches; its only C callers are 05100 and 05C8C (NON_MATCHING), so it stays asm until one of them is C. */
static s16 func_m068_0C7DC(s16 angle, s16 target, s16 speed, s32 updateRate) {
    s32 up;
    s32 down;
    s32 result;

    speed = speed * (u32) updateRate;
    up = (u16) (target - angle);
    down = (u16) (angle - target);
    if (up <= down) {
        if (up < speed) {
            result = target;
        } else {
            result = angle + speed;
        }
    } else {
        if (down < speed) {
            result = target;
        } else {
            result = angle - speed;
        }
    }
    return result & 0xFFFF;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0C7DC);
#endif

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

/* A fire ball's state (obj->unk68) */
typedef struct M068Ball {
    /* 0x00 */ f32 speed;
    /* 0x04 */ f32 radius;
    /* 0x08 */ f32 time;
    /* 0x0C */ f32 duration;
    /* 0x10 */ u8 pad10[0x14 - 0x10];
    /* 0x14 */ s16 spin;
    /* 0x16 */ s8 fading;
    /* 0x17 */ s8 state; /* 0: flying, 1: bursting, 2: done */
} M068Ball;

extern s32 rl_m068_12608; /* set: the fire balls fade out */
extern s32 rl_m068_12628; /* which of the two burst sounds is next */
extern s32 rl_m068_1262C; /* set while a burst plays */
extern s32 sprite_anim_advance(s32 anim, s32 *flags, s32 arg2, f32 *frame, s32 updateRate);
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* moves the object */

/* A fire ball: flies (growing) until it hits something or fades out, then bursts */
void func_m068_0C944(Object *obj, s32 updateRate) {
    M068Ball *b;
    HitResult hit;
    Vec3f to;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 *sound;
    f32 t; /* the frame time, then how far the ball has grown (0..1) */
    u32 flags;

    t = updateRate;
    b = obj->unk68;
    if (rl_m068_12608 != 0) {
        b->fading = 1;
    }
    if (b->state == 0) {
        if (((ObjectInteraction *) obj->interactObj)->hitPoints <= 0) {
            if (rl_m068_1262C == 0) {
                rl_m068_1262C = 1;
                b->state = 1;
                obj->unk28 = 0.0f;
            } else {
                b->state = 2;
            }
            sound = &rl_m068_12614[rl_m068_12628];
            if (*sound == 0) {
                func_80003410(0x214, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, sound);
                rl_m068_12628 ^= 1;
            }
        } else {
            dx = b->speed * obj->velocity.x * t;
            dy = b->speed * obj->velocity.y * t;
            dz = b->speed * obj->velocity.z * t;
            to.x = obj->trans.x_position + dx;
            to.y = obj->trans.y_position + dy;
            to.z = obj->trans.z_position + dz;
            func_8001CE10(1, (Vec3f *) &obj->trans.x_position, &to, &b->radius, 0, 0, 0);
            if (func_80016208(obj, (Vec3f *) &obj->trans.x_position, &to, b->radius, 1, &hit)) {
                obj->trans.x_position = 0.0f;
                obj->trans.y_position = 0.0f;
                obj->trans.z_position = 0.0f;
                func_80009A04(obj, to.x, to.y, to.z);
                if (rl_m068_1262C == 0) {
                    rl_m068_1262C = 1;
                    b->state = 1;
                    obj->unk28 = 0.0f;
                } else {
                    b->state = 2;
                }
                sound = &rl_m068_12614[rl_m068_12628];
                if (*sound == 0) {
                    func_80003410(0x214, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4,
                                  sound);
                    rl_m068_12628 ^= 1;
                }
            } else {
                func_80009A04(obj, dx, dy, dz);
                b->time += t;
                if (b->duration <= b->time) {
                    t = 1.0f;
                } else {
                    t = b->time / b->duration;
                }
                if (b->fading) {
                    s16 alpha;

                    b->speed *= func_80049BEC(0.93f, updateRate);
                    alpha = obj->opacity - updateRate * 4;
                    if (alpha <= 0) {
                        obj->opacity = 0;
                        b->state = 2;
                    } else {
                        obj->opacity = alpha;
                    }
                    obj->trans.scale = 5.0f * t + 1.0f;
                } else {
                    obj->trans.rotation.z_rotation += b->spin * updateRate;
                    obj->trans.scale = 5.0f * t + 1.0f;
                    obj->opacity = (s32) (200.0f * t) + 55;
                    if (t == 1.0f) {
                        b->state = 2;
                    }
                }
                flags = 9;
                sprite_anim_advance((s32) ((M068Header *) obj->header)->models[0], (s32 *) &flags, 20, &obj->unk28,
                                    updateRate);
            }
        }
    }
    if (b->state == 1) {
        obj->modelIndex = 1;
        obj->trans.rotation.z_rotation = 0;
        obj->opacity = 100;
        flags = 1;
        if (sprite_anim_advance((s32) ((M068Header *) obj->header)->models[1], (s32 *) &flags, 10, &obj->unk28,
                                updateRate)) {
            b->state = 2;
            rl_m068_1262C = 0;
        }
    } else if (b->state == 2) {
        func_800079B8(obj);
    }
}


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
        scale = obj->trans.scale * 0.7f;
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
        rate = 0.1f;
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

/* The electric strip's state (obj->unk68): the 0D65C set-up's fields, as its control uses them */
typedef struct M068ElectroState {
    /* 0x000 */ Triangle tris[16];
    /* 0x100 */ Vertex verts[2][72];
    /* 0x6A0 */ s16 phase[2];
    /* 0x6A4 */ s16 speed[2];
    /* 0x6A8 */ s16 step[2];
    /* 0x6AC */ f32 t;        /* how far it reaches, 0..1 */
    /* 0x6B0 */ f32 growEnd;  /* seconds */
    /* 0x6B4 */ f32 holdEnd;
    /* 0x6B8 */ f32 end;
    /* 0x6BC */ f32 time;
    /* 0x6C0 */ Vec3f *from;
    /* 0x6C4 */ u8 pad6C4[0x6C8 - 0x6C4];
    /* 0x6C8 */ Vec3f target;
    /* 0x6D4 */ Vec3f tip;
    /* 0x6E0 */ s32 buffer;
} M068ElectroState;

/* The light an object carries (*obj->light) */
typedef struct M068LightData {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
    /* 0x14 */ u8 pad14[0x40 - 0x14];
    /* 0x40 */ u8 r;
    /* 0x41 */ u8 g;
    /* 0x42 */ u8 b;
} M068LightData;

typedef struct M068LightHandle {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ M068LightData *data;
} M068LightHandle;

extern f32 rl_m068_12650[9]; /* the strips' widths along them (.data) */
extern void func_800492DC(Object *obj, Vec3f *vec);

#ifdef NON_MATCHING
/*
 * An electric strip: grows from its source towards a point, holds, shrinks; sparks when it starts.
 * Draft: the frame is 8 bytes small and the float registers differ (the ROM keeps dx*dx in f28).
 */
void func_m068_0D828(Object *obj, s32 updateRate) {
    f32 dx;
    f32 dy;
    f32 dz;
    M068ElectroState *s;
    M068LightData *light;
    SparkParams spark;
    Vec3f vec;
    f32 saved[18];
    f32 r;
    f32 hw;
    f32 sx;
    f32 radius;
    f32 sy;
    f32 *width;
    f32 pos[3];
    f32 *q;
    s32 angle;
    Vertex *v;
    s16 a;
    Vertex *w;
    s16 z;
    s32 i;
    s32 j;
    s32 n;
    Triangle *t;
    s16 dzStep;
    f32 k;

    s = obj->unk68;
    obj->trans.x_position = s->from->x;
    obj->trans.y_position = s->from->y;
    obj->trans.z_position = s->from->z;
    s->buffer ^= 1;
    if (((ObjectInteraction *) obj->interactObj)->hitPoints <= 0) {
        ((ObjectInteraction *) obj->interactObj)->hitPoints = 0x100;
        dx = s->target.x - obj->trans.x_position;
        dy = s->target.y - obj->trans.y_position;
        dz = s->target.z - obj->trans.z_position;
    } else {
        dx = sStripEnd.x - obj->trans.x_position;
        dy = sStripEnd.y - obj->trans.y_position;
        dz = sStripEnd.z - obj->trans.z_position;
    }
    obj->trans.rotation.y_rotation = func_80049C84(dx, dz) + 0x8000;
    obj->trans.rotation.x_rotation = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
    if (s->t == 0.0f) {
        spark.x_position = obj->trans.x_position;
        spark.y_position = obj->trans.y_position;
        spark.z_position = obj->trans.z_position;
        spark.unk18 = 20;
        spark.unk1A = 20;
        spark.unk1B = 190;
        spark.unk1C = -1;
        spark.unk20 = 0xFF64FF;
        spark.x_rotation = obj->trans.rotation.x_rotation;
        a = obj->trans.rotation.y_rotation - 0x4000;
        r = 0.1f;
        for (i = 0; i < 6; i++) {
            spark.y_rotation = a;
            spark.speed = func_80048E2C(80, 100) * r;
            spark.unk14 = func_80048E2C(80, 100) * r;
            func_m010_0000C(&spark);
            a += 0x1554;
        }
    }
    s->time += updateRate / 60.0f;
    if (s->time < s->growEnd) {
        s->t = s->time / s->growEnd;
    } else if (s->growEnd <= s->time && s->time <= s->holdEnd) {
        s->t = 1.0f;
    } else if (s->holdEnd < s->time) {
        s->t = 1.0f - (s->time - s->holdEnd) / (s->end - s->holdEnd);
    }
    if (s->t > 1.0f) {
        s->t = 1.0f;
    } else if (s->t < 0.0f) {
        s->t = 0.0f;
    }
    k = s->t;
    radius = func_80097140(dx * dx + dy * dy + dz * dz) * k;
    n = 0;
    dzStep = -(radius / (f32) 8);
    for (j = 0; j < 2; j++) {
        angle = s->phase[j];
        s->phase[j] = angle + updateRate * s->speed[j];
        a = 0;
        z = 0;
        v = &s->verts[s->buffer][j * 36];
        w = v + 18;
        q = saved;
        for (width = rl_m068_12650; width < &rl_m068_12650[9]; width++) {
            r = func_80049C34(a) * 20.0f * k;
            a += 0x1000;
            pos[0] = func_80049C34(angle) * r;
            pos[1] = func_80049C30(angle) * r;
            pos[2] = 0.0f;
            if (j == 0) {
                saved[n++] = pos[0];
                saved[n++] = pos[1];
                pos[0] = 0.0f;
                pos[1] = 0.0f;
            }
            sx = *width * k;
            hw = 18.0f * sx;
            sx = q[0];
            sy = q[1];
            q += 2;
            v[0].x = pos[0] - hw + sx;
            v[0].y = pos[1] + sy;
            v[0].z = z;
            v[1].x = pos[0] + hw + sx;
            v[1].y = pos[1] + sy;
            v[1].z = z;
            w[0].x = pos[0] + sx;
            w[0].y = pos[1] - hw + sy;
            w[0].z = z;
            w[1].x = pos[0] + sx;
            w[1].y = pos[1] + hw + sy;
            w[1].z = z;
            if (width == &rl_m068_12650[8]) {
                s->tip.x = pos[0] + sx;
                s->tip.y = pos[1] + sy;
                s->tip.z = z;
            }
            v += 2;
            w += 2;
            z += dzStep;
            angle += s->step[j];
        }
    }
    t = s->tris;
    for (i = 0; i < 16; i++) {
        t->uv[0][1] -= updateRate << 5;
        t->uv[1][1] -= updateRate << 5;
        t->uv[2][1] -= updateRate << 5;
        t++;
        if (t->uv[0][1] < -0x800) {
            t->uv[0][1] += 0x1000;
            t->uv[1][1] += 0x1000;
            t->uv[2][1] += 0x1000;
        }
    }
    if (obj->light != NULL) {
        light = ((M068LightHandle *) obj->light)->data;
        light->x = 0.0f;
        light->y = 0.0f;
        light->z = -radius;
        sLightColour[0] = light->r;
        sLightColour[1] = light->g;
        sLightColour[2] = light->b;
    }
    s->target.x = obj->trans.x_position + (sStripEnd.x - obj->trans.x_position) * s->t;
    s->target.y = obj->trans.y_position + (sStripEnd.y - obj->trans.y_position) * s->t;
    s->target.z = obj->trans.z_position + (sStripEnd.z - obj->trans.z_position) * s->t;
    if (s->t == 1.0f) {
        dx = obj->trans.x_position;
        dy = obj->trans.y_position;
        dz = obj->trans.z_position;
        vec.x = s->tip.x;
        vec.y = s->tip.y;
        vec.z = s->tip.z;
        func_800492DC(obj, &vec);
        obj->trans.x_position += vec.x;
        obj->trans.y_position += vec.y;
        obj->trans.z_position += vec.z;
        obj->particleEmittersEnabled = 1;
        func_8005F5AC(obj, updateRate);
        obj->trans.x_position = dx;
        obj->trans.y_position = dy;
        obj->trans.z_position = dz;
    }
    ((M068Object *) obj)->radius = radius;
    if (s->end <= s->time) {
        func_800079B8(obj);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0D828);
#endif

/* The boss's electric effect (obj->unk68) */
typedef struct M068Electro {
    /* 0x000 */ Triangle tris[16];
    /* 0x100 */ Vertex verts[2][72];
    /* 0x6A0 */ u8 pad6A0[0x6AC - 0x6A0];
    /* 0x6AC */ f32 scale; /* 0: not drawn */
    /* 0x6B0 */ u8 pad6B0[0x6D4 - 0x6B0];
    /* 0x6D4 */ f32 x;
    /* 0x6D8 */ f32 y;
    /* 0x6DC */ f32 z;
    /* 0x6E0 */ s32 buffer;
} M068Electro;

extern Vertex rl_m068_12674[];   /* the effect's core (.data) */
extern Triangle rl_m068_126C0[];
extern u32 rl_m068_12720[2];     /* the effect's two colours */
extern void func_80041CA0(Gfx **dl, s32 arg1, Object *obj, f32 scale, f32 arg4);
extern void func_80055C00(Gfx **dl, void *model, s32 flags, s32 arg3);
extern void func_80041DC0(Gfx **dl);

#ifdef NON_MATCHING
/*
 * Draws the boss's electric effect: a core model, the arcs in two colours and a glow sprite.
 * Length and frame match; the effect, the colour table's end and 0xFB000000 rotate through
 * s1..s3 (as in 0ED10).
 */
void func_m068_0E148(Gfx **dl, s32 arg1, s32 arg2, Object *obj) {
    M068Electro *e = obj->unk68;
    s32 pad;
    Vertex *v;
    s32 i;
    M068Sprite *sprite;

    if (e->scale != 0) {
        obj->trans.scale = e->scale;
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        func_80055C00(dl, ((M068Header *) obj->header)->models[1], 0x16, 0);
        gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(rl_m068_12674), 8, 0);
        gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(rl_m068_126C0), 6, 1);
        gDPPipeSync((*dl)++);
        func_80041DC0(dl);
        obj->trans.scale = 1.0f;
        func_80041CA0(dl, arg1, obj, 1.0f, 0.0f);
        v = e->verts[e->buffer];
        func_80055C00(dl, *((M068Header *) obj->header)->models, 0x206, 0);
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 190);
        for (i = 0; i < 2; i++) {
            gDPSetColor((*dl)++, G_SETENVCOLOR, rl_m068_12720[i]);
            gSPVertexJFG((*dl)++, (u32) v + 0x80000000, 19, 0);
            gSPPolygon((*dl)++, (u32) e + 0x80000000, 16, 1);
            gDPPipeSync((*dl)++);
            v += 18;
            gSPVertexJFG((*dl)++, (u32) v + 0x80000000, 19, 0);
            gSPPolygon((*dl)++, (u32) e + 0x80000000, 16, 1);
            gDPPipeSync((*dl)++);
            v += 18;
        }
        sprite = ((M068Sprite **) ((M068Header *) obj->header)->models)[2];
        func_8005600C(255, 255, 255, sLightColour[0], sLightColour[1], sLightColour[2]);
        render_sprite_billboard_at(dl, arg1, arg2, sprite, e->x, e->y, e->z, 0, e->scale * 3.0f, 0.0f, 12, 255);
        func_80056064();
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        func_80041DC0(dl);
        func_80055B80(dl);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E148);
#endif

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

/* The second kind of strip's state (obj->unk68): a bolt from a random joint of a part to another part */
typedef struct M068Bolt {
    /* 0x000 */ Triangle tris[16];
    /* 0x100 */ Vertex verts[2][72];
    /* 0x6A0 */ s16 phase[2];
    /* 0x6A4 */ s16 speed[2];
    /* 0x6A8 */ s16 step[2];
    /* 0x6AC */ Vec3f target;
    /* 0x6B8 */ s16 section;
    /* 0x6BA */ u8 active;
    /* 0x6BB */ u8 buffer;
    /* 0x6BC */ Object *owner;
} M068Bolt;

/* The model whose vertices a bolt starts at */
typedef struct M068BoltModel {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ Vec3f *points;
} M068BoltModel;


#ifdef NON_MATCHING
/*
 * A bolt: from a random point of the owner's section to section 4's joint, a zig-zag strip of two crossed
 * ribbons. Same length; the ROM keeps the saved-corner pointer in s3 (count s4, angle s5) and has its
 * two loop temporaries 16 bytes higher.
 */
void func_m068_0E668(Object *obj, s32 updateRate) {
    f32 dx;
    f32 dy;
    f32 dz;
    M068Bolt *s;
    M068Section *sec;
    M068BoltModel *model;
    f32 *to;
    Vec3f *from;
    f32 saved[18];
    f32 r;
    f32 hw;
    f32 sx;
    f32 len;
    f32 sy;
    f32 scale;
    f32 *width;
    f32 pos[3];
    f32 *q;
    s32 angle;
    Vertex *v;
    s16 a;
    Vertex *w;
    s16 z;
    s32 i;
    s32 j;
    s32 n;
    Triangle *t;
    s16 dzStep;
    UNUSED s32 pad[4];

    s = obj->unk68;
    model = *(M068BoltModel **) s->owner->modelInstances;
    sec = &rl_m068_1254C[s->section];
    to = &sJointPos[rl_m068_1254C[4].joint * 3];
    from = &model->points[func_80048E2C(sec->first, sec->last)];
    obj->trans.x_position = from->x;
    obj->trans.y_position = from->y;
    obj->trans.z_position = from->z;
    s->target.x = to[0];
    s->target.y = to[1];
    s->target.z = to[2];
    s->buffer ^= 1;
    dx = s->target.x - obj->trans.x_position;
    dy = s->target.y - obj->trans.y_position;
    dz = s->target.z - obj->trans.z_position;
    obj->trans.rotation.y_rotation = func_80049C84(dx, dz) + 0x8000;
    obj->trans.rotation.x_rotation = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
    s->active = 1;
    len = func_80097140(dx * dx + dy * dy + dz * dz);
    n = 0;
    dzStep = -(len / (f32) 8);
    scale = 1.0f;
    for (j = 0; j < 2; j++) {
        angle = s->phase[j];
        s->phase[j] = angle + updateRate * s->speed[j];
        a = 0;
        z = 0;
        v = &s->verts[s->buffer][j * 36];
        w = v + 18;
        q = saved;
        for (width = rl_m068_12650; width < &rl_m068_12650[9]; width++) {
            r = func_80049C34(a) * 70.0f * scale;
            a += 0x1000;
            pos[0] = func_80049C34(angle) * r;
            pos[1] = func_80049C30(angle) * r;
            pos[2] = 0.0f;
            if (j == 0) {
                saved[n++] = pos[0];
                saved[n++] = pos[1];
                pos[0] = 0.0f;
                pos[1] = 0.0f;
            }
            sx = *width * scale;
            hw = 18.0f * sx;
            sx = q[0];
            sy = q[1];
            q += 2;
            v[0].x = pos[0] - hw + sx;
            v[0].y = pos[1] + sy;
            v[0].z = z;
            v[1].x = pos[0] + hw + sx;
            v[1].y = pos[1] + sy;
            v[1].z = z;
            w[0].x = pos[0] + sx;
            w[0].y = pos[1] - hw + sy;
            w[0].z = z;
            w[1].x = pos[0] + sx;
            w[1].y = pos[1] + hw + sy;
            w[1].z = z;
            v += 2;
            w += 2;
            z += dzStep;
            angle += s->step[j];
        }
    }
    t = s->tris;
    for (i = 0; i < 16; i++) {
        t->uv[0][1] -= updateRate << 5;
        t->uv[1][1] -= updateRate << 5;
        t->uv[2][1] -= updateRate << 5;
        t++;
        if (t->uv[0][1] < -0x800) {
            t->uv[0][1] += 0x1000;
            t->uv[1][1] += 0x1000;
            t->uv[2][1] += 0x1000;
        }
    }
    ((M068Object *) obj)->radius = len;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068", func_m068_0E668);
#endif

/* What the boss's death effect keeps in obj->unk68 */
typedef struct M068Death {
    /* 0x000 */ Triangle tris[16];
    /* 0x100 */ Vertex verts[2][72]; /* drawn from one while the other is filled */
    /* 0x6A0 */ u8 pad6A0[0x6BA - 0x6A0];
    /* 0x6BA */ u8 visible;
    /* 0x6BB */ u8 buffer;
} M068Death;

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

    ring->scale = 0.1f;
    ring->damage = setup->damage;
    ring->fadeStart = setup->fadeStart;
    ring->end = setup->end;
    ring->growth = 0.02f;
}

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
    /* 0x50 */ Vec3f hit; /* where it hit */
    /* 0x5C */ f32 radius;
    /* 0x60 */ f32 timer; /* seconds since it left the trail */
    /* 0x64 */ f32 lastDist; /* from the camera, for its sound's pitch */
    /* 0x68 */ f32 pitch;
    /* 0x6C */ u8 r;
    /* 0x6D */ u8 g;
    /* 0x6E */ u8 b;
    /* 0x6F */ u8 pad6F;
    /* 0x70 */ s16 unk70; /* the trail point it heads for; -1: past the trail */
    /* 0x72 */ s16 unk72; /* its spin */
    /* 0x74 */ s16 kind;
    /* 0x76 */ s16 state; /* 0: flying, 1/2/3: breaking up (shot, spent, hit something) */
} M068Boulder;

/* A boulder's level entry */
typedef struct M068BoulderSetup {
    /* 0x0 */ u8 pad0[3];
    /* 0x3 */ u8 kind;
} M068BoulderSetup;

extern void func_80049394(Vec3s *rotation, Vec3f *vec);
extern s32 rl_m068_125E8;       /* how many trail points (.data) */
extern s32 rl_m068_125FC;       /* the boulders made (.data) */

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
    for (i = 0, t = b->trail, src = sTrailStart; i < rl_m068_125E8; i++, t++, src++) {
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

/* The debris a boulder breaks into (an object entry) */
typedef struct M068DebrisEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ s16 pitch;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 scale;
    /* 0x14 */ f32 speed;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 pad1D[0x20 - 0x1D];
    /* 0x20 */ s32 unk20;
} M068DebrisEntry;

/* The dust a boulder leaves where it hits (an object entry) */
typedef struct M068DustEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 yaw;
    /* 0x0C */ f32 speed;
    /* 0x10 */ f32 scale;
} M068DustEntry;

extern s32 rl_m068_12600;        /* the boulders' rumble (a positioned sound) (.data) */
extern Object *rl_m068_12604;    /* the boulder it follows (.data) */
extern void cam_shake_start(s32 index, f32 phase1, f32 phase2, f32 phase3, s32 strength);
extern void audspat_point_set_pitch(s32 sound, s32 pitch);

#ifdef NON_MATCHING
static f32 func_m068_10758(Object *obj, M068Boulder *b, f32 rate);

/*
 * A boulder: follows its trail (then flies on), breaks up when shot, spent or when it hits something.
 * Same length and frame; registers differ in the flying part (the ROM keeps its turn factor on the stack).
 */
void func_m068_0F7C8(Object *obj, s32 updateRate) {
    Object *o;
    s32 i;
    s32 lo;
    s32 hi;
    M068Boulder *b;
    HitResult hit;
    SparkParams spark;
    M068DebrisEntry debris;
    M068DustEntry dust;
    Vec3f from;
    Vec3f to;
    f32 tx;
    f32 ty;
    f32 tz;
    f32 rate;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 k;
    f32 soundPitch;
    s32 yaw[2]; /* the sparks' angle ranges */
    s32 pitch[2];
    s32 debrisYaw[2];
    s32 debrisPitch[2];
    s16 a;
    s32 flags;

    b = obj->unk68;
    rate = updateRate;
    if (b->state == 0 && ((ObjectInteraction *) obj->interactObj)->hitPoints <= 0) {
        b->state = 2;
    }
    if (rl_m068_12600 == 0) {
        func_80003410(0x1C5, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &rl_m068_12600);
        rl_m068_12604 = obj;
    }
    if (b->colour != NULL) {
        k = b->colour[11] / 255.0f;
        b->r = b->colour[8] * k;
        b->g = b->colour[9] * k;
        b->b = b->colour[10] * k;
    }
    if (b->state != 0) {
        switch (b->state) {
            case 1:
                flags = 0x11;
                yaw[0] = lo = -0x8000;
                yaw[1] = hi = 0x7FFF;
                pitch[0] = -0x3000;
                pitch[1] = 0x3000;
                break;
            case 2:
                flags = 0x23;
                yaw[0] = -0x8000;
                yaw[1] = 0x7FFF;
                pitch[0] = -0x3000;
                pitch[1] = 0x3000;
                a = func_80049C84(obj->velocity.x, obj->velocity.z) + 0x8000;
                debrisYaw[0] = a - 0x1000;
                debrisYaw[1] = a + 0x1000;
                a = func_80049C84(obj->velocity.y, func_80097140(obj->velocity.x * obj->velocity.x +
                                                                 obj->velocity.z * obj->velocity.z));
                debrisPitch[0] = a - 0x1000;
                debrisPitch[1] = a + 0x1000;
                break;
            case 3:
                flags = 0x13;
                if (b->unk70 == -1 || rl_m068_125E8 == b->unk70) {
                    flags = 0x1B;
                }
                if (0.866f <= b->hit.y) {
                    flags |= 4;
                }
                a = func_80049C84(b->hit.x, b->hit.z) + 0x8000;
                lo = a - 0x1000;
                hi = a + 0x1000;
                yaw[0] = lo;
                yaw[1] = hi;
                debrisYaw[0] = lo;
                debrisYaw[1] = hi;
                a = func_80049C84(b->hit.y, func_80097140(b->hit.z * b->hit.z + b->hit.x * b->hit.x));
                lo = a - 0x1000;
                hi = a + 0x1000;
                pitch[0] = lo;
                pitch[1] = hi;
                debrisPitch[0] = lo;
                debrisPitch[1] = hi;
                break;
            default:
                flags = 0;
                break;
        }
        if (flags & 1) {
            spark.x_position = obj->trans.x_position;
            spark.y_position = obj->trans.y_position;
            spark.z_position = obj->trans.z_position;
            spark.unk18 = 20;
            spark.unk1A = 10;
            spark.unk1B = 255;
            if (b->kind == 0) {
                spark.unk1C = 0xFFFF80FF;
                spark.unk20 = 0xFFFF;
            } else if (b->kind == 1) {
                spark.unk1C = 0xFFFF80FF;
                spark.unk20 = 0xFF0000FF;
            } else if (b->kind == 2) {
                spark.unk1C = 0xFFFF80FF;
                spark.unk20 = 0xFF00FF;
            }
            for (i = 0; i < 6; i++) {
                spark.y_rotation = func_80048E2C(yaw[0], yaw[1]);
                spark.x_rotation = func_80048E2C(pitch[0], pitch[1]);
                spark.speed = b->speed;
                spark.unk14 = func_80048E2C(25, 50) * 0.1f;
                func_m010_0000C(&spark);
            }
        }
        if (flags & 2) {
            debris.common.objectID = 0x2EE;
            debris.unkE = 60;
            debris.unk1C = 100;
            debris.unk20 = 0;
            debris.unk18 = 0.15f;
            for (i = 0; i < 4; i++) {
                debris.common.x = func_80048E2C(-15, 15) + (s32) obj->trans.x_position;
                debris.common.y = func_80048E2C(-15, 15) + (s32) obj->trans.y_position;
                debris.common.z = func_80048E2C(-15, 15) + (s32) obj->trans.z_position;
                debris.yaw = func_80048E2C(debrisYaw[0], debrisYaw[1]);
                debris.pitch = func_80048E2C(debrisPitch[0], debrisPitch[1]);
                debris.speed = b->speed;
                debris.scale = obj->trans.scale * 0.75f;
                o = func_80005F14(&debris, 1);
                if (o != NULL) {
                    o->level_entry = NULL;
                }
            }
        }
        if (flags & 4) {
            dust.common.objectID = 0x1CF;
            dust.common.x = obj->trans.x_position;
            dust.common.y = obj->trans.y_position;
            dust.common.z = obj->trans.z_position;
            dust.yaw = func_80049C84(obj->velocity.x, obj->velocity.z);
            dust.scale = obj->trans.scale;
            dust.speed = b->speed / 10.0f;
            o = func_80005F14(&dust, 1);
            if (o != NULL) {
                o->level_entry = NULL;
            }
        }
        if (flags & 8) {
            cam_shake_start(0, 0.0f, 0.0f, 2.0f, b->speed * obj->trans.scale);
        }
        if (flags & 0x10) {
            func_80003410(0x22, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        }
        if (flags & 0x20) {
            func_80003410(0x1C6, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        }
        if (obj == rl_m068_12604 && rl_m068_12600 != 0) {
            func_80003614(rl_m068_12600);
            rl_m068_12604 = NULL;
        }
        obj->particleEmittersEnabled = 2;
        func_8005F5A0(obj->trans.scale);
        func_8005F5AC(obj, updateRate);
        rl_m068_125FC--;
        func_800079B8(obj);
        return;
    }
    if (b->unk70 >= 0 && b->unk70 < rl_m068_125E8) {
        Vec3f *p = &b->trail[b->unk70];

        tx = p->x;
        ty = p->y;
        tz = p->z;
    } else if (rl_m068_125E8 == b->unk70) {
        o = func_8000E4D0(0);
        tx = o->trans.x_position;
        ty = o->trans.y_position + 40.0f;
        tz = o->trans.z_position;
    } else {
        tx = obj->trans.x_position + obj->velocity.x * rate;
        ty = obj->trans.y_position + obj->velocity.y * rate;
        tz = obj->trans.z_position + obj->velocity.z * rate;
    }
    dx = obj->trans.x_position;
    dx = tx - dx;
    dy = ty - obj->trans.y_position;
    dz = tz - obj->trans.z_position;
    d = func_80097140(dx * dx + dy * dy + dz * dz);
    if (d > 0.0f) {
        dx /= d;
        dy /= d;
        dz /= d;
        k = 1.0f - func_80049BEC(0.95f, updateRate);
        b->velocity.x += (dx - b->velocity.x) * k;
        b->velocity.y += (dy - b->velocity.y) * k;
        b->velocity.z += (dz - b->velocity.z) * k;
        d = func_80097140(b->velocity.x * b->velocity.x + b->velocity.y * b->velocity.y +
                          b->velocity.z * b->velocity.z);
        b->velocity.x /= d;
        b->velocity.y /= d;
        b->velocity.z /= d;
        obj->velocity.x = b->velocity.x * b->speed;
        obj->velocity.y = b->velocity.y * b->speed;
        obj->velocity.z = b->velocity.z * b->speed;
        from.x = obj->trans.x_position;
        from.y = obj->trans.y_position;
        from.z = obj->trans.z_position;
        to.x = obj->trans.x_position + obj->velocity.x * rate;
        to.y = obj->trans.y_position + obj->velocity.y * rate;
        to.z = obj->trans.z_position + obj->velocity.z * rate;
        func_8001CE10(1, &from, &to, &b->radius, 0xCE000000, 0, 0);
        if (func_80016208(obj, &from, &to, b->radius, 1, &hit)) {
            b->state = 3;
            b->hit.x = hit.pos.x;
            b->hit.y = hit.pos.y;
            b->hit.z = hit.pos.z;
            obj->trans.x_position = 0.0f;
            obj->trans.y_position = 0.0f;
            obj->trans.z_position = 0.0f;
            obj->trans.rotation.x_rotation += b->unk72 * updateRate;
            func_80009A04(obj, to.x, to.y, to.z);
        } else {
            dx = to.x - from.x;
            dy = to.y - from.y;
            dz = to.z - from.z;
            obj->trans.rotation.x_rotation += b->unk72 * updateRate;
            func_80009A04(obj, dx, dy, dz);
            k = dx * dx + dy * dy + dz * dz;
            k = ((tx - from.x) * dx + (ty - from.y) * dy + (tz - from.z) * dz) / k;
            if (b->unk70 != -1 && k >= 0.0f && k <= 1.0f) {
                b->unk70++;
                if (rl_m068_125E8 == b->unk70) {
                    ((ObjectInteraction *) obj->interactObj)->flags |= 1;
                } else if (rl_m068_125E8 < b->unk70) {
                    b->unk70 = -1;
                }
            }
            if (b->kind != 0 && rl_m068_125E8 == b->unk70) {
                func_8005F5A0(obj->trans.scale);
                obj->particleEmittersEnabled = 1;
                func_8005F5AC(obj, updateRate);
            }
        }
    } else {
        b->unk70++;
        if (rl_m068_125E8 == b->unk70) {
            ((ObjectInteraction *) obj->interactObj)->flags |= 1;
        } else if (rl_m068_125E8 < b->unk70) {
            b->unk70 = -1;
        }
    }
    if (obj == rl_m068_12604 && rl_m068_12600 != 0) {
        soundPitch = func_m068_10758(obj, b, rate);
        func_800035E8(rl_m068_12600, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        audspat_point_set_pitch(rl_m068_12600, (u32) soundPitch & 0xFF);
    }
    if (b->unk70 == -1) {
        b->timer += rate / 60.0f;
        if (b->timer > 1.0f) {
            b->state = 2;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_0F7C8);
#endif

/* Called when something hits the boss: notes a hit by the player */
void func_m068_10734(Object *hitter, Object *obj, s32 arg2) {
    M068State *s = obj->unk68;

    if (hitter->behaviorId == 1) {
        s->hitByPlayer = 1;
    }
}

extern ObjectTransform *func_80041DE0(void); /* the camera */
extern f32 func_8008B200(s32 x);

#ifdef NON_MATCHING
/*
 * The followed boulder's rumble pitch: from how fast it closes on the camera (a Doppler shift).
 * This matches, but it is a static that only 0F7C8 calls, so it waits for 0F7C8.
 */
static f32 func_m068_10758(Object *obj, M068Boulder *b, f32 rate) {
    ObjectTransform *cam;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 v;
    f32 max;

    cam = func_80041DE0();
    dx = obj->trans.x_position - cam->x_position;
    dy = obj->trans.y_position - cam->y_position;
    dz = obj->trans.z_position - cam->z_position;
    d = func_80097140(dx * dx + dy * dy + dz * dz);
    v = (b->lastDist - d) / rate;
    v = func_8008B200((40.0f + v) / (40.0f - v) * 315.0f);
    max = 0.8f;
    b->pitch += (v - 1.2f - b->pitch) * 0.5f;
    if (b->pitch > max) {
        b->pitch = max;
    } else if (b->pitch < -0.8f) {
        b->pitch = -0.8f;
    }
    b->lastDist = d;
    return (b->pitch + 1.0f) * 100.0f;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_10758);
#endif

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
extern MtxF rl_m068_1250C;       /* (.data) */

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
        entry.unk28 = 0.15f;
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
            piece = sPieces[i];
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
INCLUDE_ASM("asm/nonmatchings/modules/m068/ro", func_m068_110E0);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m068", data);

/* (.rodata: C, and the assembly functions' own from ro/) */
