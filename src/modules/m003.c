/*
 * Runlink module 3: the enemy troops' shared behaviour. It keeps the troop
 * member being processed (and its troop's home object) in module state, moves
 * members between way points (module 154's graph), keeps the level's regions,
 * and handles hits, deaths and missions. Most of it is still in assembly.
 */
#include "module.h"

/* What a troop member keeps in obj->unk68 */
typedef struct M003Member {
    /* 0x00 */ u32 pad0_0 : 1;
    /* 0x00 */ u32 onFire : 1;
    /* 0x00 */ u32 unk0_29 : 1;
    /* 0x00 */ u32 pad0_3 : 8;
    /* 0x00 */ u32 unk0_20 : 1;
    /* 0x00 */ u32 pad0_2 : 5;
    /* 0x00 */ u32 unlinked : 1; /* not in its troop's list */
    /* 0x00 */ u32 pad0_5 : 1;
    /* 0x00 */ u32 busy : 1; /* not ready to move on */
    /* 0x00 */ u32 pad0_4 : 2;
    /* 0x00 */ u32 unk0_9 : 1; /* cleared when its troop's leader goes */
    /* 0x00 */ u32 pad0_6 : 5;
    /* 0x00 */ u32 headless : 1;
    /* 0x00 */ u32 pad0_1 : 1;
    /* 0x00 */ u32 unk0_1 : 1;
    /* 0x00 */ u32 unk0_0 : 1;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u8 unk6_7 : 1;
    /* 0x06 */ u8 turning : 1; /* unk10 holds a turn */
    /* 0x06 */ u8 unk6_5 : 1;
    /* 0x06 */ u8 unk6_4 : 1;
    /* 0x06 */ u8 partLost : 1; /* it has lost a body part */
    /* 0x06 */ u8 unk6_0 : 3;
    /* 0x07 */ s8 group; /* < 0: not active */
    /* 0x08 */ u8 flags8; /* bit flags, read as the word M003Flags8 */
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 mission;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 pad12[0x15 - 0x12];
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[0x19 - 0x16];
    /* 0x19 */ u8 animSet;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B; /* < 5: heading for a way point */
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[0x24 - 0x1E];
    /* 0x24 */ Object *home;
    /* 0x28 */ Object *next; /* the next member of the troop */
    /* 0x2C */ u8 pad2C[0x38 - 0x2C];
    /* 0x38 */ f32 unk38;
    /* 0x3C */ f32 unk3C;
    /* 0x40 */ f32 unk40;
    /* 0x44 */ u8 pad44[0x4A - 0x44];
    /* 0x4A */ u8 unk4A;
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ u8 pad4C[0x50 - 0x4C];
    /* 0x50 */ u8 node;
    /* 0x51 */ u8 unk51;
    /* 0x52 */ u8 unk52;
    /* 0x53 */ u8 pad53[0x55 - 0x53];
    /* 0x55 */ u8 unk55;
    /* 0x56 */ s8 unk56;
    /* 0x57 */ u8 pad57[0x5A - 0x57];
    /* 0x5A */ u16 lostParts; /* bit per body part */
    /* 0x5C */ u8 pad5C[0x60 - 0x5C];
    /* 0x60 */ f32 unk60; /* the body's animation frame */
    /* 0x64 */ s32 shotBy;
    /* 0x68 */ f32 earPos[3]; /* where it hears events from */
    /* 0x74 */ u8 pad74[0xA0 - 0x74];
    /* 0xA0 */ u16 unkA0; /* body parts hit (kind 15) */
    /* 0xA2 */ u16 unkA2; /* body parts hit (other kinds) */
    /* 0xA4 */ u8 padA4[0xA6 - 0xA4];
    /* 0xA6 */ s16 unkA6;
    /* 0xA8 */ Object *unkA8;
    /* 0xAC */ u8 padAC[0xBC - 0xAC];
    /* 0xBC */ s32 sound; /* the burning sound's handle */
    /* 0xC0 */ u8 padC0[0xC8 - 0xC0];
    /* 0xC8 */ Object *spark; /* the electric spark on the member */
} M003Member;

/* The member's word at 0x8 as bit fields */
typedef struct M003Flags8 {
    u32 pad5 : 5;
    u32 shocked : 1;
    u32 unk24 : 2;
    u32 rest : 24;
} M003Flags8;
#define FLAGS8(m) ((M003Flags8 *) &(m)->flags8)

/* The member's group byte as bit fields */
typedef struct M003Group {
    u8 inactive : 1; /* the sign bit of `group` */
    u8 rest : 7;
} M003Group;

/* The body's word at 0x24 as bit fields */
typedef struct M003BodyFlags {
    u32 pad26 : 26;
    u32 unk5 : 1; /* animations 0x16-0x1F may not replace each other */
    u32 fireproof : 1;
    u32 rest : 4;
} M003BodyFlags;

/* What a troop's home object keeps in obj->unk68 */
typedef struct M003Home {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 count; /* members */
    /* 0x02 */ u8 pad2[0x10 - 0x2];
    /* 0x10 */ s8 offset[3];
    /* 0x13 */ u8 pad13[0x1A - 0x13];
    /* 0x1A */ u16 kind2 : 4;
    /* 0x1A */ u16 unk1A : 12;
    /* 0x1C */ u8 pad1C[0x21 - 0x1C];
    /* 0x21 */ u8 unk21; /* how loud an event its members notice must be */
    /* 0x22 */ u8 pad22[0x28 - 0x22];
    /* 0x28 */ u16 flags; /* 0x2000: its members may touch */
    /* 0x2A */ u8 pad2A[0x2C - 0x2A];
    /* 0x2C */ u16 kind;
    /* 0x2E */ s16 unk2E; /* a countdown */
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 pad31[0x33 - 0x31];
    /* 0x33 */ u8 unk33;
    /* 0x34 */ u8 pad34[0x3C - 0x34];
    /* 0x3C */ s16 unk3C;
    /* 0x3E */ s16 unk3E;
    /* 0x40 */ s16 unk40;
    /* 0x42 */ s16 unk42;
    /* 0x44 */ s16 unk44;
    /* 0x46 */ u8 pad46[0x48 - 0x46];
    /* 0x48 */ f32 x;
    /* 0x4C */ f32 y;
    /* 0x50 */ f32 z;
    /* 0x54 */ Object *leader;
    /* 0x58 */ Object *first; /* the troop's first member */
    /* 0x5C */ u8 pad5C[0x60 - 0x5C];
    /* 0x60 */ u8 unk60;
    /* 0x61 */ u8 unk61;
    /* 0x62 */ u8 pad62;
    /* 0x63 */ u8 arrived; /* members in place */
    /* 0x64 */ u8 unk64;
    /* 0x65 */ u8 pad65[0x6C - 0x65];
    /* 0x6C */ f32 unk6C;
    /* 0x70 */ u8 pad70[0x75 - 0x70];
    /* 0x75 */ u8 unk75;
    /* 0x76 */ u8 pad76;
    /* 0x77 */ u8 unk77; /* its D_800FEA18 slot, 0xFF: none */
    /* 0x78 */ u8 pad78[0x7B - 0x78];
    /* 0x7B */ u8 unk7B;
    /* 0x7C */ u8 pad7C[0x91 - 0x7C];
    /* 0x91 */ u8 unk91; /* bit 0: members blame the closest player */
} M003Home;

/* A way point (module 154's graph) */
typedef struct M003Node {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ Object *obj; /* the object spawned there for it */
    /* 0x14 */ u8 pad14[0x16 - 0x14];
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 kind : 2; /* the kind of object spawned for it */
    /* 0x17 */ u8 unk17 : 6;
    /* 0x18 */ u8 pad18[0x1C - 0x18];
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 pad1D[0x32 - 0x1D];
    /* 0x32 */ u8 unk32;
    /* 0x33 */ u8 unk33;
} M003Node;

/* A corner of a region */
typedef struct M003Point {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 z;
} M003Point;

/* A region of the level (size 0x2C) */
typedef struct M003Region {
    /* 0x00 */ M003Point *points;
    /* 0x04 */ u32 numEdges;
    /* 0x08 */ u8 pad8[0x14 - 0x8];
    /* 0x14 */ u32 flags;
    /* 0x18 */ void *data; /* a tree of lines down to the edge lists */
    /* 0x1C */ void *edges;
    /* 0x20 */ union {
        s16 pos[3]; /* a point in it */
        s32 node;   /* replaced by what func_m003_15BE8 finds there */
    } at;
    /* 0x28 */ u8 pad28[0x2C - 0x28];
} M003Region;

/* An event the troops react to (size 0x10) */
typedef struct M003Event {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 active; /* frames left */
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ u32 id; /* the low two bits are the kind */
} M003Event;

/* A body part (8 bytes) */
typedef struct M003BodyPart {
    /* 0x00 */ u8 bit;
    /* 0x01 */ u8 part; /* the hit part it stands for */
    /* 0x02 */ u8 kind; /* 2: hitting it kills */
    /* 0x03 */ u8 pad3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 pad7;
} M003BodyPart;

/* A member's body description (D_800A38E0, 0x80 bytes each) */
typedef struct M003Body {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ s16 unk24; /* a height */
    /* 0x26 */ u8 pad26[0x29 - 0x26];
    /* 0x29 */ u8 numParts;
    /* 0x2A */ u8 pad2A;
    /* 0x2B */ u8 unk2B; /* 5: does not burn */
    /* 0x2C */ u8 pad2C;
    /* 0x2D */ u8 headPart; /* 0x1F: none */
    /* 0x2E */ u8 pad2E;
    /* 0x2F */ u8 hitPart; /* the body part the last hit took off, 7: none */
    /* 0x30 */ u8 pad30[0x40 - 0x30];
    /* 0x40 */ M003BodyPart parts[8];
} M003Body;

/* What a member's interactObj points at */
typedef struct M003Interact {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u32 pad8_0 : 3;
    /* 0x08 */ u32 unk8_3 : 1;
    /* 0x08 */ u32 sound : 4;
    /* 0x08 */ u32 pad8_1 : 24;
    /* 0x0C */ f32 centre[3];
    /* 0x18 */ f32 pos[3]; /* where it was hit */
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0x31 - 0x28];
    /* 0x31 */ u8 part;
    /* 0x32 */ u8 pad32[0x3C - 0x32];
    /* 0x3C */ Object *hitBy;
} M003Interact;

extern M003Node *D_800FEA9C; /* the way points */
extern u8 *D_800A38E0;       /* the body descriptions */
extern s32 D_800FE9FC;
extern s32 D_800FE9B0;
extern f32 func_80097140(f32 x); /* sqrtf */
extern s16 troop_slot_get(u16 kind);
extern s32 func_80004AF4(f32 x, f32 y, f32 z, s32 arg3);
extern void func_800492DC(Object *obj, Vec3f *v); /* turns v by the object's rotation */
extern s32 troop_find_member(s16 arg0, u8 arg1);
extern void stats_kill_add(s32 sound);
extern void troop_spawn_context_set(Object *obj, Object *home);
extern void func_m154_017E8(Object *home);
extern s32 rl_m003_0527C(f32 x, f32 y, f32 z);
extern void func_m003_0DF84(void);
extern void func_m003_15ED8(s32 arg0, s32 arg1);
extern void func_m003_16530(void);
extern void *func_80050F44(s32 *count);
extern void func_8004AA80(void *data); /* DKR mempool_free */
extern s32 func_m003_16F44(f32 x, f32 z, M003Region *region);
/* Where a segment leaves a region */
typedef struct M003Crossing {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 z;
    /* 0x04 */ u8 point;  /* 0xFF: none */
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 region;
    /* 0x08 */ u8 atPoint; /* it crosses at the point itself */
} M003Crossing;

extern s32 func_m003_175E8(f32 x1, f32 z1, f32 x2, f32 z2, M003Region *region, M003Crossing *out, s32 point,
                           u8 regionIndex);
extern s32 func_m003_04260(s32 mission, s32 kind, u8 arg2, u8 arg3, u8 arg4);
extern Object **func_80005E98(s32 *first, s32 *count);
extern void *D_800FE9A4;
extern void *D_800FEA18[16];
extern s32 D_800FEA00;
extern s32 D_800FEA04;
extern s32 func_80049C84(f32 x, f32 z); /* the angle of a direction */
extern s32 func_80049D80(s32 a, s32 b); /* the difference of two angles */
extern void func_80051880(u8 arg0);
extern s32 func_m154_00DFC(u8 arg0, u8 arg1, u8 arg2);
extern void func_m155_02BB0(Object *home);
extern void func_m003_04D08(s32 kind, s32 anim1, s32 anim2, s32 anim3);
extern s32 func_m003_04F88(s32 arg0, f32 arg1);
extern void func_m003_06D28(Object *home, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
extern void func_m003_0C324(void);
extern s32 func_m003_15BE8(f32 x, f32 y, f32 z);
extern s32 func_m154_00130(f32 x, f32 y, f32 z, s32 arg3); /* the way point nearest a point */
extern void anim_paths_find_near(f32 x, f32 y, f32 z, s16 *a, s16 *b);
extern s32 func_8004966C(s16 angle); /* sine */
extern s32 func_80049668(s16 angle); /* cosine */
extern s32 troop_find_by_tag(s32 arg0);
extern s32 objflag_get(s32 arg0);
extern Object *func_80005F14(void *entry, s32 arg1);
extern void func_80048130(s32 arg0, s32 arg1);
extern void func_80011708(f32 x, f32 y, f32 z, s32 arg3);
extern void spawn_object_from_list(s32 arg0, s16 x, s16 y, s16 z, s32 arg4, s32 arg5);
extern s32 D_800FE9F8;
extern void func_m014_019E4(void);
extern void func_m003_07E88(s32 arg0, s32 arg1, s32 arg2);
extern void func_m003_09A14(Object *obj);
extern s32 rl_m003_04968(s32 index, s32 anim, s32 arg2);
extern s32 rl_m003_04614(s32 anim, s32 arg1);
extern void rl_m003_044F4(Object *obj);

/* An animation entry (8 bytes) */
typedef struct M003Anim {
    /* 0x00 */ u8 unk0[2];
    /* 0x02 */ u8 anim;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u32 pad4_0 : 17;
    /* 0x04 */ u32 unk4_11 : 4;
    /* 0x04 */ u32 pad4_1 : 11;
} M003Anim;

/* Parts of the module's .data (still assembly), by module offset */
extern M003Anim rl_m003_17DB8[]; /* the animation entries */
extern u16 rl_m003_19278[];      /* per body: its first animation entry (and a second u16) */
extern s32 rl_m003_197F8;        /* the next event slot to reuse */
extern u16 rl_m003_197D4[];      /* per animation kind: a bit per kind it may follow */

/* The module's .data (still assembly) */
extern u8 rl_m003_17A10[];
extern void *rl_m003_19428;
#define sRegionData rl_m003_19428
extern M003Region *rl_m003_1942C;
#define sRegions rl_m003_1942C

/* The module's .bss (the assembly functions reach it by offset) */
static u8 sBss0[0x4];
static s32 sRegionCount;
static u8 sBss8[0x2];
static u8 sBssA; /* 10 or more: memory is low */
static u8 sAlertKind;
static u8 sAlert;
static u8 sBss10[0xA8 - 0x10]; /* (IDO aligns arrays to 4) */
static f32 sUpdateRate;        /* this frame's, as a float */
static Object *sObj;            /* the member being processed */
static M003Member *sMember;     /* its data */
static Object *sHome;           /* its troop's home object */
static M003Home *sHomeData;     /* and that one's data */
static s16 sBodyIndex;
static M003Body *sBody;
static s32 sShotBy;
static s32 sClosestPlayer;
static s32 sBssCC;
static s32 sBssD0;
static s32 sBssD4;
static M003Event sEvents[16];
static s32 sChangeArg1;
static s32 sChangeArg0;
static s32 sChangePending;
static s32 sBss1E4;
static Object *sTargets[32];
static s32 sTargetCount;
static u8 sBss270[0xDB0 - 0x270]; /* (from 0x26C, IDO aligning it to 8) */
/* The segment-against-region test's state (func_m003_175E8 and the asm 17174) */
static void *sCrossData;   /* the region's tree */
static void *sCrossEdges;  /* and its edges */
static s32 sCrossX1;       /* the segment, in whole units */
static s32 sCrossZ1;
static s32 sCrossX2;
static s32 sCrossZ2;
static u8 sBssDC8[0xDE0 - 0xDC8];
static struct M003Crossing *sCrossOut;
static s32 sCrossX;        /* where it leaves the region */
static s32 sCrossZ;
static s32 sCrossEdge;     /* the edge it crosses, -1: none */
static u8 sCrossPoint;     /* the region point it crosses near */
static u8 sCrossRegion;    /* the region on the other side */
static s16 sBssDF2;
static s32 sBssDF4;
static s32 sBssDF8;
static s32 sBssDFC;

s32 func_m003_0D2C0(Object *obj);
void func_m003_10548(Object *obj, s32 shotBy);
void func_m003_0C0A0();
void func_m003_04398(s32 mission, s32 arg1);
s32 func_m003_05450(Object *obj);
void func_m003_09FF0(Object *obj);
f32 func_m003_03E68(f32 x1, f32 z1, f32 x2, f32 z2);
void func_m003_0C038(Object *obj);

/* Where the current member is heading: its way point, or its troop's home */
void func_m003_00000(Object *obj, f32 *x, f32 *y, f32 *z) {
    M003Node *node = &D_800FEA9C[sMember->node];

    if (sMember->unk1B < 5) {
        *x = node->x;
        *y = node->y;
        *z = node->z;
        return;
    }
    *x = ((M003Home *) sMember->home->unk68)->x;
    *y = ((M003Home *) sMember->home->unk68)->y;
    *z = ((M003Home *) sMember->home->unk68)->z;
}

/* Circles the troop's home for a while (once near a point), then heads off */
s32 func_m003_000D0(Object *obj, s16 *angle, f32 x, f32 z) {
    M003Home *home;
    f32 dist;
    s32 turn;

    if (sMember->unk1B == 4) {
        sMember->unk51--;
        if (sMember->unk51 == 0) {
            sMember->unk1B = 7;
        }
        home = sHome->unk68;
        *angle = func_80049C84(home->x - obj->trans.x_position, home->z - obj->trans.z_position);
        if ((sMember->unk51 / 50) & 1) {
            turn = 0x3E80;
        } else {
            turn = -0x3E80;
        }
        *angle += turn;
        return TRUE;
    }
    dist = func_m003_03E68(obj->trans.x_position, obj->trans.z_position, x, z);
    if (sMember->unk1B == 3 && dist < 500.0f) {
        sMember->unk1B = 4;
        sMember->unk51 = 150;
    } else if (sMember->unk1B == 7 && dist < 2000.0f) {
        func_m003_04398(1, 0);
        ((M003Home *) sHome->unk68)->unk33 = 0;
    }
    return FALSE;
}

/* An object's position */
void func_m003_00258(Object *obj, f32 *x, f32 *y, f32 *z) {
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
}

/* Turns to the next way point or, when that is done, ends the mission; keeps the facing */
s32 func_m003_0027C(Object *obj, s32 arg1, s32 arg2, s32 arg3) {
    if (sMember->unk1B == 1) {
        sMember->unk1B = 4;
        sMember->unk4B = 2;
        func_m003_04D08(2, sMember->node, sMember->unk51, sMember->unk52);
    } else if (func_m003_04F88(2, 1.0f)) {
        func_m003_04398(1, 0);
        sMember->unk4B = 0;
    }
    return obj->trans.rotation.y_rotation;
}

/* An object's position; ends the current mission unless it is mission 5 */
void func_m003_00328(Object *obj, f32 *x, f32 *y, f32 *z) {
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    if (sMember->unkA != 5) {
        func_m003_04398(1, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00384);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_009C8);

/* The current member's target point */
void func_m003_00A5C(Object *obj, f32 *x, f32 *y, f32 *z) {
    *x = sMember->unk38;
    *y = sMember->unk3C;
    *z = sMember->unk40;
}

/* The facing towards the current member's target point */
s32 func_m003_00AA0(Object *obj, s16 *angle, s32 arg2, s32 arg3) {
    *angle = func_80049C84(obj->trans.x_position - sMember->unk38, obj->trans.z_position - sMember->unk40);
    return TRUE;
}

/* An object's position; heads for way point 0x24 and makes a sound once; may end the troop */
void func_m003_00AF4(Object *obj, f32 *x, f32 *y, f32 *z) {
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    obj->particleEmittersEnabled = 8;
    func_m003_04D08(3, 0x24, 0, 0);
    if (!sMember->unk0_20) {
        func_80003410(0xDB, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        func_m014_019E4();
        sMember->unk0_20 = 1;
    }
    func_m003_04F88(3, 1.0f);
    if (0xFF - D_800FE9F8 * 4 < sMember->unk55) {
        func_80048130(sHomeData->unk64, 1);
        func_m003_09FF0(obj);
    } else {
        sMember->unk55 += D_800FE9F8 * 4;
    }
}

void func_m003_00C20(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    func_m003_06D28(sHome, sMember->unk15, arg1, arg2, arg3);
}

/* An object's position; heads for way point 0x20 */
void func_m003_00C70(Object *obj, f32 *x, f32 *y, f32 *z) {
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    switch (sMember->unk1B) {
        case 1:
            func_m003_04D08(3, 0x20, 0, 0);
            sMember->unk1B = 4;
            sMember->unk0_29 = 0;
            break;
        case 4:
            if (sMember->unk4B == 0) {
                func_m003_04D08(3, 0x20, 0, 0);
            } else {
                func_m003_04F88(3, 1.0f);
            }
            break;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00D44);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00FD0);

/* An object's position; the mission that waits at a way point */
void func_m003_01188(Object *obj, f32 *x, f32 *y, f32 *z) {
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    switch (sMember->unk1B) {
        case 1:
            if (sHomeData->unk30 == 2 && !(((u16 *) obj->header)[0x14 / 2] & 0x80)) {
                sHomeData->unk30 = 3;
            }
            if (sHomeData->unk0 < 2) {
                sMember->unk0_29 = 0;
                sMember->unk4B = 0;
                func_m003_04398(1, 0);
            } else {
                sMember->unk1B = 4;
                func_m003_04D08(3, 2, 0, 0);
            }
            break;
        case 4:
            if (sMember->unk4B != 3 || func_m003_04F88(3, 1.0f)) {
                sMember->unk0_29 = 0;
                sMember->unk4B = 0;
                func_m003_04398(1, 0);
                func_m003_04D08(0, 5, 6, 1);
            }
            break;
    }
}

/* Keeps the facing; records the turn towards the closest player */
s32 func_m003_012E8(Object *obj, s16 *angle, s32 arg2, s32 arg3) {
    *angle = obj->trans.rotation.y_rotation;
    sMember->unk10 = func_80049C84(obj->trans.x_position - ((Object *) sClosestPlayer)->trans.x_position,
                                   obj->trans.z_position - ((Object *) sClosestPlayer)->trans.z_position) -
                     obj->trans.rotation.y_rotation;
    sMember->turning = 1;
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0136C);

extern s32 func_m003_0512C(s32 kind, f32 speed, s32 anim);

/* An object's position; counts the current member through its mission's three stages */
void func_m003_01608(Object *obj, f32 *x, f32 *y, f32 *z) {
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    switch (sMember->unk1B) {
        case 1:
            sMember->unk9 = 0;
            if (sMember->node <= D_800FE9F8) {
                sMember->unk1B = 4;
                sMember->node = 45;
            } else {
                sMember->node -= D_800FE9F8;
            }
            func_m003_0512C(3, 1.0f, 0x3B);
            break;
        case 4:
            if (sMember->node <= D_800FE9F8) {
                sMember->unk1B = 5;
                sMember->node = 30;
            } else {
                sMember->node -= D_800FE9F8;
            }
            func_m003_04F88(3, 1.0f);
            break;
        case 5:
            sMember->unk9 = 3;
            if (sMember->node <= D_800FE9F8) {
                sMember->unk1A = 16;
                sMember->unk1B = 1;
                sMember->unk9 = 1;
            } else {
                sMember->node -= D_800FE9F8;
            }
            func_m003_04F88(3, 1.0f);
            break;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01764);

/* Keeps the facing; records the turn towards whoever shot the member (no one: the mission ends) */
s32 func_m003_01974(Object *obj, s16 *angle, s32 arg2, s32 arg3) {
    Object *by = (Object *) sShotBy;
    s16 target;

    if (by == NULL) {
        func_m003_04398(1, 0);
        return FALSE;
    }
    target = func_80049C84(obj->trans.x_position - by->trans.x_position, obj->trans.z_position - by->trans.z_position);
    *angle = obj->trans.rotation.y_rotation;
    if (sMember->unk1B != 4) {
        sMember->unk10 = func_80049D80(obj->trans.rotation.y_rotation, target);
    } else {
        sMember->unk10 = 0;
    }
    sMember->turning = 1;
    return TRUE;
}

s32 func_m003_01A50(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    return FALSE;
}

/* The facing towards whoever shot the member, or the current facing */
s32 func_m003_01A68(Object *obj, s16 *angle, s32 arg2, s32 arg3) {
    Object *by = (Object *) sShotBy;

    if (by != NULL) {
        *angle = func_80049C84(obj->trans.x_position - by->trans.x_position,
                               obj->trans.z_position - by->trans.z_position);
    } else {
        *angle = obj->trans.rotation.y_rotation;
    }
    return TRUE;
}

s32 func_m003_01ACC(Object *obj, s16 *angle, s32 arg2, s32 arg3) {
    *angle = sMember->unk1C;
    return TRUE;
}

/* The facing towards the closest player */
s32 func_m003_01AF0(Object *obj, s16 *angle, s32 arg2, s32 arg3) {
    *angle = func_80049C84(obj->trans.x_position - ((Object *) sClosestPlayer)->trans.x_position,
                           obj->trans.z_position - ((Object *) sClosestPlayer)->trans.z_position);
    return TRUE;
}

/* Turns towards whoever shot the member; once facing them (or with no one), the mission ends */
s32 func_m003_01B44(Object *obj, s16 *angle, s32 arg2, s32 arg3) {
    Object *by = (Object *) sShotBy;
    s16 diff;

    if (by == NULL) {
        func_m003_04398(1, func_80048E2C(sHomeData->unk60, sHomeData->unk61));
        return FALSE;
    }
    *angle = func_80049C84(obj->trans.x_position - by->trans.x_position, obj->trans.z_position - by->trans.z_position);
    diff = func_80049D80(*angle, obj->trans.rotation.y_rotation);
    if (diff < 0) {
        diff = -diff;
    }
    if (diff < 11) {
        func_m003_07E88(0, 0, 0);
        func_m003_04398(1, func_80048E2C(sHomeData->unk60, sHomeData->unk61));
    }
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01C58);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01D3C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01EB8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_021C8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0237C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02760);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_027A4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_028D4);

/* The troop's home takes a point in front of the closest player (at the body's height) */
void func_m003_02A94(void) {
    sHomeData->unk40 = ((Object *) sClosestPlayer)->trans.x_position -
                       (func_8004966C(((Object *) sClosestPlayer)->trans.rotation.y_rotation) >> 7);
    sHomeData->unk42 = ((Object *) sClosestPlayer)->trans.y_position + sBody->unk24;
    sHomeData->unk44 = ((Object *) sClosestPlayer)->trans.z_position -
                       (func_80049668(((Object *) sClosestPlayer)->trans.rotation.y_rotation) >> 7);
}

#ifdef NON_MATCHING
/* The troop's home takes its own position plus its offset (in 16s) */
/* (the ROM converts the offset after loading the position: other registers, add.s operands swapped) */
void func_m003_02BBC(void) {
    sHomeData->unk40 = sHome->trans.x_position + sHomeData->offset[0] * 16;
    sHomeData->unk42 = sHome->trans.y_position + sHomeData->offset[1] * 16;
    sHomeData->unk44 = sHome->trans.z_position + sHomeData->offset[2] * 16;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02BBC);
#endif

/* The troop's home takes the closest player's position (at the body's height above it) */
void func_m003_02CA8(void) {
    sHomeData->unk40 = ((Object *) sClosestPlayer)->trans.x_position;
    sHomeData->unk42 = ((Object *) sClosestPlayer)->trans.y_position + sBody->unk24;
    sHomeData->unk44 = ((Object *) sClosestPlayer)->trans.z_position;
}

void func_m003_02D58(void) {
    if (sHomeData->unk2E != 0) {
        sHomeData->unk2E--;
        if (sHomeData->unk2E == 0) {
            sHomeData->unk33 = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02DA0);

/* Turns the troop's home towards its target facing, 400 at most */
void func_m003_0301C(void) {
    s16 turn = func_80049D80(sHome->trans.rotation.y_rotation, sHomeData->unk3E);

    if (turn > 400) {
        turn = 400;
    } else if (turn < -400) {
        turn = -400;
    }
    sHome->trans.rotation.y_rotation += turn;
    if (sHome->trans.rotation.y_rotation == sHomeData->unk3E) {
        sHomeData->unk33 = 0x16;
    }
}

void func_m003_030CC(void) {
    if (sHomeData->unk0 == 1) {
        sHomeData->unk33 = 0;
    }
    sHomeData->unk2E--;
    if (sHomeData->unk2E == 0) {
        sHomeData->unk33 = 0;
    }
}

void func_m003_0312C(void) {
    sHomeData->unk2E--;
    if (sHomeData->unk2E == 0) {
        sHomeData->unk33 = 0;
    }
}

/* When no member is busy, the troop moves on to its next way point */
void func_m003_03170(void) {
    Object *obj = sHomeData->first;
    M003Member *m;
    s32 ready = TRUE;

    sHomeData->flags &= ~0x1000;
    do {
        m = obj->unk68;
        if (m->busy || m->unk1A == 10) {
            ready = FALSE;
            break;
        }
        obj = m->next;
    } while (obj != NULL);
    if (ready) {
        sHomeData->unk33 = 9;
        sHomeData->unk2E = D_800FEA9C[sHomeData->unk3C & 0xFF].unk1C;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0322C);

/* Counts down the troop's wait, then sends it on from its way point (low byte) towards the next (high byte) */
void func_m003_0332C(void) {
    u32 lo;
    u16 hi;

    sHomeData->unk3E--;
    if (sHomeData->unk3E <= 0) {
        sHomeData->unk33 = 0x1F;
        lo = sHomeData->unk3C;
        hi = sHomeData->unk3C;
        func_80051880(func_m154_00DFC(sHomeData->unk30, lo & 0xFF, hi >> 8));
    }
}

/* Sets up obj's troop at the way point nearest its home */
void func_m003_033B0(Object *obj) {
    M003Node *node;
    Object *saved = sObj;

    if (func_m003_05450(obj)) {
        node = &D_800FEA9C[func_m154_00130(sHome->trans.x_position, sHome->trans.y_position, sHome->trans.z_position, 0)];
        sHomeData->unk3C = node->unk16;
        func_80051880(func_m154_00DFC(sHomeData->unk30, sHomeData->unk3C & 0xFF, 0));
        sHomeData->unk33 = 0x1F;
    }
    func_m003_05450(saved);
}

s32 func_m003_07BAC(f32 x, f32 y, f32 z, s32 kind, u8 arg4, u8 arg5);

/* Puts a member at its place in the troop: by its way point's object, or beside its home */
void func_m003_03470(Object *obj) {
    M003Node *node;
    Object *anim;
    f32 pos[3];
    M003Member *m = obj->unk68;
    Object *home;
    M003Home *h;
    s32 index;

    home = m->home;
    if (home == NULL) {
        return;
    }
    h = home->unk68;
    if (!(h->unk3C & 0xFF)) {
        func_m003_033B0(obj);
    }
    index = (u16) h->unk3C >> 8;
    if (index == 0) {
        obj->trans.x_position = home->trans.x_position + m->unk15 * 100;
        obj->trans.y_position = home->trans.y_position;
        obj->trans.z_position = home->trans.z_position;
        return;
    }
    node = &D_800FEA9C[index];
    anim = node->obj;
    if (anim == NULL) {
        anim = (Object *) func_m003_07BAC(node->x, node->y, node->z, node->kind, node->unk32, node->unk33);
        if (anim == NULL) {
            return;
        }
        node->obj = anim;
    }
    pos[0] = m->unk38;
    pos[1] = m->unk3C;
    pos[2] = m->unk40;
    func_800492DC(anim, (Vec3f *) pos);
    obj->trans.x_position = pos[0] + anim->trans.x_position;
    obj->trans.y_position = pos[1] + anim->trans.y_position;
    obj->trans.z_position = pos[2] + anim->trans.z_position;
    obj->trans.rotation.y_rotation = anim->trans.rotation.y_rotation;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03628);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_038D4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03AA8);

void func_m003_03C84(void) {
    func_m155_02BB0(sHome);
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03CAC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03D44);

/* The distance between two points on the ground plane */
f32 func_m003_03E20(f32 x1, f32 z1, f32 x2, f32 z2) {
    f32 dx = x1 - x2;
    f32 dz = z1 - z2;

    return func_80097140(dx * dx + dz * dz);
}

/* Its square */
f32 func_m003_03E68(f32 x1, f32 z1, f32 x2, f32 z2) {
    f32 dx = x1 - x2;
    f32 dz = z1 - z2;

    return dx * dx + dz * dz;
}

/*
 * Whether the segments (x1, y1)-(x2, y2) and (x3, y3)-(x4, y4) cross: 0 no, 2 collinear, 1 yes (the
 * crossing point, rounded, in *x and *y if x is not NULL). The usual integer test: each segment's line
 * equation must give the other segment's ends opposite signs.
 */
s32 func_m003_03E90(s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 x4, s32 y4, s16 *x, s16 *y) {
    s32 a1, a2, b1, b2, c1, c2;
    s32 r1, r2, r3, r4;
    s32 denom, offset, num;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;
    r3 = a1 * x3 + b1 * y3 + c1;
    r4 = a1 * x4 + b1 * y4 + c1;
    if (r3 != 0 && r4 != 0 && (r3 ^ r4) >= 0) {
        return 0;
    }
    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;
    r1 = a2 * x1 + b2 * y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;
    if (r1 != 0 && r2 != 0 && (r1 ^ r2) >= 0) {
        return 0;
    }
    denom = a1 * b2 - a2 * b1;
    if (denom == 0) {
        return 2;
    }
    if (x != NULL) {
        offset = denom < 0 ? -denom / 2 : denom / 2;
        num = b1 * c2 - b2 * c1;
        *x = (num < 0 ? num - offset : num + offset) / denom;
        num = a2 * c1 - a1 * c2;
        *y = (num < 0 ? num - offset : num + offset) / denom;
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_040FC);

extern s32 rl_m003_1976C[]; /* per mission kind: a bit per kind it may replace */
extern s32 func_m146_00CD8(s32 kind);

/* Gives the current member a new mission, if its present one may be replaced by it */
s32 func_m003_04260(s32 mission, s32 kind, u8 arg2, u8 arg3, u8 arg4) {
    s32 mask = rl_m003_1976C[kind];

    if (((1 << kind) & 0x18000) && sBody->unk2B >= 5) {
        return FALSE;
    }
    if (sBodyIndex == 0x31 && !func_m146_00CD8(kind)) {
        return FALSE;
    }
    if (kind != sMember->unk1A) {
        if ((1 << sMember->unk1A) & mask) {
            sMember->mission = mission;
            sMember->unk1A = kind;
            sMember->unk1B = 1;
            sMember->node = arg2;
            sMember->unk51 = arg3;
            sMember->unk52 = arg4;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

/* Ends the current member's mission (1: whatever it is) */
void func_m003_04398(s32 mission, s32 arg1) {
    if (mission == 1 || mission == sMember->mission) {
        sMember->unk1A = 0;
        sMember->unk1B = 0;
        sMember->mission = 0;
        sMember->unk4A = arg1;
    }
}

/* Whether the member may go from its current animation kind to `kind` */
static s32 func_m003_043FC(s32 kind) {
    s32 mask = rl_m003_197D4[kind];

    return mask & (1 << sMember->unk4B);
}

/* A member's animation entries (by its troop's body), or the first ones */
M003Anim *func_m003_04428(Object *obj) {
    M003Member *m = obj->unk68;
    Object *home;
    M003Home *h;

    if (m == NULL) {
        return NULL;
    }
    home = m->home;
    if (home != NULL) {
        h = home->unk68;
        return &rl_m003_17DB8[rl_m003_19278[troop_slot_get(h->kind) * 2]] + m->animSet;
    }
    return rl_m003_17DB8;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_044B4);

/* The level entry of a lost body part (size 0x14) */
typedef struct M003PartEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0xC - 0xA];
    /* 0x0C */ Object *owner;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 kind;
} M003PartEntry;

extern s32 gMainPoolFreeBytes;
extern void func_m003_058A4(Object *obj);

#ifdef NON_MATCHING
/* IDO folds the `+ 0.0f`s the ROM keeps, and sBody/index swap registers */
/* The member loses the body part the last hit took off: a flying piece of it is spawned if there's memory */
static void func_m003_044F4(Object *obj) {
    u8 index = sBody->hitPart;
    M003BodyPart *part = &sBody->parts[index];
    M003Member *m = obj->unk68;
    M003PartEntry entry;
    u16 bit = 1 << part->bit;
    f32 zero;
    Object *piece;

    if (index == 7) {
        return;
    }
    if ((m->lostParts & bit) || m->unk55 != 0) {
        return;
    }
    m->lostParts |= bit;
    m->partLost = TRUE;
    func_m003_058A4(obj);
    if (gMainPoolFreeBytes > 0xC000) {
        entry.objectID = 0x89;
        entry.size = 0x14;
        entry.unk3 = 0;
        entry.x = 0;
        entry.y = 0;
        entry.z = 0;
        entry.unk10 = part->unk4;
        entry.unk11 = part->unk5;
        entry.owner = sObj;
        entry.kind = part->kind;
        piece = func_80005F14(&entry, 1);
        if (piece != NULL) {
            zero = 0.0f;
            piece->velocity.x += zero;
            piece->velocity.y += zero;
            piece->velocity.z += zero;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_044F4);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04614);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04968);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04D08);

/* Starts animation `index` of the member's body, unless it may not move now; returns what 04968 does */
s32 func_m003_04EC4(s32 arg0, s32 index) {
    s32 anim = rl_m003_17DB8[rl_m003_19278[sBodyIndex * 2] + index].anim;
    s32 result;

    if (!func_m003_043FC(arg0)) {
        return -1;
    }
    if (sMember->unkA == 0x21 || sMember->unkA == 0x25) {
        return -1;
    }
    result = rl_m003_04968(index, anim, 0);
    if (result != -1) {
        sMember->unk4B = arg0;
    }
    return result;
}

extern f32 rl_m003_044B4(void);

/* Moves the member's animation of kind `kind` on; returns what the animation step returns, or 0 */
s32 func_m003_04F88(s32 kind, f32 speed) {
    s32 result = 0;
    M003Anim *anims;
    Object **flash;

    if (kind == sMember->unk4B) {
        anims = func_m003_04428(sObj);
        sMember->unk60 = sObj->unk28;
        result = func_8001138C(sObj, rl_m003_044B4() * speed, sUpdateRate);
        if (result) {
            if (sHomeData->kind == 0xBD) {
                flash = &sMember->unkA8;
                sObj->particleEmittersEnabled = 0;
                if (*flash != NULL) {
                    func_800079B8(*flash);
                    *flash = NULL;
                }
            }
            if (sMember->unkA6 > 0) {
                if (anims->unk4_11 && sMember->unk56 != 0) {
                    if (sMember->unk56 != 15) {
                        sMember->unk56--;
                    }
                    return 0;
                }
                if (anims[1].anim == 0x35 && sMember->unk4B != 9) {
                    sMember->animSet++;
                    rl_m003_04968(sMember->animSet, 0x35, 1);
                    return 0;
                }
            } else if (sMember->unk55 == 0) {
                sMember->unk55 = 1;
            }
        }
    }
    return result;
}

extern s32 rl_m003_19434; /* an animation entry forced on every member, or -1 */

/* Starts animation `anim` of kind `kind` on the current member, or keeps the one running */
s32 func_m003_0512C(s32 kind, f32 speed, s32 anim) {
    s32 index;
    M003Anim *e;

    if (!func_m003_043FC(kind)) {
        return 0;
    }
    if (kind == sMember->unk4B && anim == sMember->unkA) {
        if ((index = func_m003_04F88(sMember->unk4B, speed)) == 0) {
            return index;
        }
    }
    if (rl_m003_19434 != -1) {
        sMember->unkA = anim;
        index = rl_m003_19434;
    } else {
        index = rl_m003_04614(anim, 0);
    }
    if (index == -1) {
        return 0;
    }
    e = &rl_m003_17DB8[index];
    if (e->unk4_11) {
        sMember->unk56 = e->unk3 & 0x7F;
    }
    if (index != sMember->animSet) {
        rl_m003_04968(index, anim, 1);
    }
    sMember->unk4B = kind;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0527C);

s32 func_m003_053C4(void) {
    return sClosestPlayer;
}

extern void func_m140_00000(Object *obj);

/* Sets who the member blames for its hits, unless it already has someone */
static void func_m003_053D4(Object *obj) {
    M003Member *m = obj->unk68;

    if (m->shotBy == 0) {
        if (obj->objectID == 0x8D) {
            func_m140_00000(obj);
        } else if (sHomeData->unk91 & 1) {
            m->shotBy = sClosestPlayer;
        } else {
            m->shotBy = 0;
        }
    }
}

/* Makes obj the member being processed; returns whether it is an active member with a body */
s32 func_m003_05450(Object *obj) {
    sMember = NULL;
    sHome = (Object *) sMember;
    sHomeData = NULL;
    sObj = obj;
    if (sObj == NULL) {
        return FALSE;
    }
    sMember = sObj->unk68;
    if (sMember->group < 0) {
        return FALSE;
    }
    sHome = sMember->home;
    if (sHome == NULL) {
        return FALSE;
    }
    sHomeData = sHome->unk68;
    sClosestPlayer = rl_m003_0527C(sObj->trans.x_position, sObj->trans.y_position, sObj->trans.z_position);
    func_m003_053D4(sObj);
    sShotBy = sMember->shotBy;
    troop_spawn_context_set(sObj, sHome);
    func_m154_017E8(sHome);
    return func_m003_0D2C0(sObj) != 0;
}

/* Makes a troop's home (and its first member, if any) the one being processed */
static void func_m003_0556C(Object *home) {
    sHome = home;
    sHomeData = home->unk68;
    if ((sObj = sHomeData->first) != NULL) {
        sMember = sObj->unk68;
        sShotBy = sMember->shotBy;
        sClosestPlayer = rl_m003_0527C(sObj->trans.x_position, sObj->trans.y_position, sObj->trans.z_position);
    } else {
        sMember = NULL;
        sShotBy = 0;
        sClosestPlayer = rl_m003_0527C(sHome->trans.x_position, sHome->trans.y_position, sHome->trans.z_position);
    }
    troop_spawn_context_set(sObj, sHome);
    func_m154_017E8(sHome);
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_05650);

/* Region `index`, or NULL before the regions are loaded */
M003Region *func_m003_0569C(s32 index) {
    return sRegions ? &sRegions[index] : NULL;
}

s32 func_m003_056D4(M003Region *region) {
    return region - sRegions;
}

/* The region (after the first) that a point is in, or 0 */
s32 func_m003_056F4(s16 x, s16 z) {
    s32 i = sRegionCount - 1;
    M003Region *region = &sRegions[1];

    while (i-- > 0) {
        if (region->data != NULL && func_m003_16F44(x, z, region)) {
            return region - sRegions;
        }
        region++;
    }
    return 0;
}

/* Whether another member of the troop is within the current member's radius of a point */
s32 func_m003_057CC(f32 x, f32 z) {
    Object *obj = sHomeData->first;
    f32 radius = ((M003Interact *) sObj->interactObj)->radius * ((M003Interact *) sObj->interactObj)->radius;
    M003Member *m;

    while (obj != NULL) {
        m = obj->unk68;
        if (obj != sObj && func_m003_03E68(obj->trans.x_position, obj->trans.z_position, x, z) <= radius) {
            return TRUE;
        }
        obj = m->next;
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_058A4);

/* A model instance as used here */
typedef struct M003Model {
    /* 0x00 */ u8 pad0[0x2D];
    /* 0x2D */ u8 numJoints;
} M003Model;

typedef struct M003ModelInst {
    /* 0x00 */ M003Model *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA; /* the joints need updating */
    /* 0x0B */ u8 padB[0x68 - 0xB];
    /* 0x68 */ Vec3f *joints;
} M003ModelInst;

extern void func_m003_058A4(Object *obj);

/* Where joint `joint` of the current member's model is (or its hit centre) */
void func_m003_0609C(s32 joint, f32 *x, f32 *y, f32 *z) {
    M003ModelInst *inst;
    M003Model *model;

    inst = sObj->modelInstances[sObj->modelIndex];
    if (inst == NULL) {
        goto centre;
    }
    if (inst->unkA != 0) {
        func_m003_058A4(sObj);
    }
    model = inst->model;
    if (joint >= model->numJoints) {
        goto centre;
    }
    if (inst->joints != NULL) {
        *x = inst->joints[joint].x;
        *y = inst->joints[joint].y;
        *z = inst->joints[joint].z;
    }
    return;
centre:
    *x = ((M003Interact *) sObj->interactObj)->centre[0];
    *y = ((M003Interact *) sObj->interactObj)->centre[1];
    *z = ((M003Interact *) sObj->interactObj)->centre[2];
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_061FC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_062A0);

extern s32 audspat_volume_at_distance(s32 kind, f32 *from, f32 *to); /* how loud a sound of a kind is from `from` at `to` */

#ifdef NON_MATCHING
/* IDO puts the loop counter's dead copy in the branch delay slots the ROM leaves empty (one word short) */
/* The nearest event the member hears loudly enough (its loudness in *loudness), or NULL */
M003Event *func_m003_06754(Object *obj, f32 *loudness) {
    M003Member *m = obj->unk68;
    M003Event *e = sEvents;
    u32 best = -1;
    s32 i = 15;
    s32 loud;
    u32 dist;
    M003Event *found = NULL;
    f32 from[3];
    f32 to[3];

    *loudness = 0.0f;
    do {
        if (e->active == 0) {
        } else if ((e->id & ~3) != 0 && ((Object *) (e->id & ~3))->freed) {
            e->active = 0;
        } else if (e->unk6 & 1) {
            from[0] = e->x;
            from[1] = e->y;
            from[2] = e->z;
            to[0] = m->earPos[0];
            to[1] = m->earPos[1];
            to[2] = m->earPos[2];
            loud = audspat_volume_at_distance(e->unk9, from, to);
            if (loud >= sHomeData->unk21) {
                dist = func_m003_03E68(m->earPos[0], m->earPos[2], e->x, e->z);
                if (dist <= best) {
                    found = e;
                    best = dist;
                    *loudness = loud;
                }
            }
        }
        e++;
    } while (i--);
    return found;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_06754);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_069B0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_06D28);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07980);

#ifdef NON_MATCHING
/* The speed change that brings a mover to a stop `dist` away, accelerating by `accel` */
/* (31 words: speed and the result take f12/f14 the other way round; the early ternary return already
   gives the ROM's single exit and the saved f20) */
f32 func_m003_079D8(s32 dist, f32 speed, f32 accel) {
    f32 zero = 0.0f;
    f32 pos = zero;
    f32 step = zero;
    s32 neg = dist < 0;
    f32 fdist;
    f32 result;

    if (neg) {
        dist = -dist;
        speed = -speed;
    }
    if (speed < zero) {
        result = neg ? -accel : accel;
        return result;
    }
    fdist = dist;
    do {
        step += accel;
        pos += step;
    } while (pos + step < fdist);
    if (fdist <= accel && speed <= accel && ((dist >= 0 && zero <= speed) || (dist <= 0 && speed <= zero))) {
        result = zero;
    } else if (speed + accel <= step - accel) {
        result = speed + accel;
    } else if (speed - accel < step) {
        result = step - accel;
        if (step == accel) {
            result = accel;
        }
    } else {
        result = speed - accel;
        if (result == zero) {
            result = accel;
        }
    }
    if (neg) {
        result = -result;
        speed = -speed;
    }
    return result - speed;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_079D8);
#endif

/* The troop's member whose number is the troop's count (or the last one looked at) */
static Object *func_m003_07B38(Object *home) {
    M003Home *h = home->unk68;
    s32 i = h->unk0;
    Object *obj = h->first;
    M003Member *m;

    if (obj == NULL) {
        return NULL;
    }
    while (i--) {
        m = obj->unk68;
        if (m->unk15 == h->unk0) {
            return obj;
        }
        if ((home = m->next) == NULL) {
            break;
        }
        obj = home;
    }
    return obj;
}

/* Spawns the object of a way point of kind `kind` at (x, y, z) */
s32 func_m003_07BAC(f32 x, f32 y, f32 z, s32 kind, u8 arg4, u8 arg5) {
    if (&z) {} /* keeps z in its home slot (reloaded into a2) */
    switch (kind) {
        case 1:
            return func_80004AF4(x, y, z, 8);
        case 2:
            return troop_find_member(arg4 + 1, arg5);
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07C18);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07E88);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09680);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09840);

extern void rl_m003_09840(M003Home *home, s32 arg1);

/* Takes a member out of its troop's list */
void func_m003_098A0(Object *obj) {
    M003Member *m = obj->unk68;
    M003Member *om;
    Object *o;
    Object *home;
    s32 pad1;
    s32 pad2;
    M003Home *h;
    s32 i;

    if (m->unlinked) {
        return;
    }
    home = m->home;
    if (home == NULL) {
        return;
    }
    h = home->unk68;
    if (--h->unk0 == 0 && h->arrived == h->count) {
        if ((h->flags & 4) || h->unk30 == 9) {
            h->unk30 = 5;
            h->unk2E = 60;
            h->arrived = 0;
        } else {
            rl_m003_09840(h, 0x10);
        }
    }
    if (h->flags & 1) {
        ((M003Member *) func_m003_07B38(home)->unk68)->unk15 = ((M003Member *) obj->unk68)->unk15;
    }
    o = h->first;
    if (o == obj) {
        h->first = ((M003Member *) o->unk68)->next;
    } else {
        i = h->unk0;
        while (i--) {
            om = o->unk68;
            o = om->next;
            if (o == obj) {
                m = o->unk68;
                om->next = m->next;
                break;
            }
        }
    }
}

extern void audspat_point_handle_clear(s32 sound);

/* Removes a member from the game: its troop's slot, its sound, every reference to it, then the object */
void func_m003_09A14(Object *obj) {
    s32 first;
    s32 count;
    s32 i;
    Object **objs;
    s32 numHomes;
    Object **homes;
    M003Member *m;
    Object *home;
    M003Home *h;
    Object *o;
    UNUSED s32 pad0;
    UNUSED s32 pad1;

    objs = func_80005E98(&first, &count);
    homes = func_80050F44(&numHomes);
    m = obj->unk68;
    ((M003Group *) &m->group)->inactive = TRUE;
    home = m->home;
    if (home != NULL) {
        h = home->unk68;
        if (h->unk0 == 1 && h->arrived == h->count) {
            if (h->unk77 != 0xFF) {
                func_8004AA80(D_800FEA18[h->unk77]);
                D_800FEA18[h->unk77] = NULL;
            }
            h->unk77 = 0xFF;
        }
    }
    if (m->sound != 0) {
        audspat_point_handle_clear(m->sound);
    }
    for (i = first; i < count; i++) {
        o = objs[i];
        if (o->behaviorId == 0x18) {
            m = o->unk68;
            if (m->shotBy == (s32) obj) {
                m->shotBy = 0;
            }
        }
    }
    i = numHomes;
    while (i--) {
        home = homes[i];
        h = home->unk68;
        if (h->leader == obj) {
            h->leader = NULL;
            h->unk33 = 0;
            for (o = h->first; o != NULL; o = m->next) {
                m = o->unk68;
                m->unk0_9 = FALSE;
            }
        }
    }
    func_800079B8(obj);
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09CE4);

/* A member is killed */
void func_m003_09FF0(Object *obj) {
    M003Interact *interact;

    if (sHomeData->unk0 == 1 && sHomeData->arrived == sHomeData->count) {
        func_80011708(obj->trans.x_position, obj->trans.y_position + 10.0f, obj->trans.z_position, sHomeData->unk7B);
    }
    if (sHomeData->kind == 0x2CE) {
        interact = (M003Interact *) sObj->interactObj;
        spawn_object_from_list(0x10, interact->pos[0], interact->pos[1], interact->pos[2], 100, 0);
    }
    func_m003_09A14(obj);
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0A110);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0A564);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0A5EC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0A850);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0A948);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0A9E8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0AABC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0ABF4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0AD38);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0AE80);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0AF34);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0AFC4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0B040);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0B468);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0B50C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0B70C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0B9A0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0B9E0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0BA20);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0BA94);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0BAD4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0BB50);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0BBC4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0BFC4);

/* A member dies */
void func_m003_0C038(Object *obj) {
    if (obj->interactObj->hitPoints > 0) {
        obj->interactObj->hitPoints = 0;
    }
    if (((M003Interact *) obj->interactObj)->unk8_3 != 15) {
        stats_kill_add(((M003Interact *) obj->interactObj)->sound);
    }
}

/* A member's body part is hit */
void func_m003_0C0A0(Object *obj) {
    M003Member *m = obj->unk68;
    M003BodyPart *part = NULL;
    M003Interact *interact = (M003Interact *) obj->interactObj;
    s32 i;
    u16 bit;

    if (interact != NULL) {
        for (i = 0; i < sBody->numParts; i++) {
            if (interact->part == sBody->parts[i].part) {
                part = &sBody->parts[i];
                break;
            }
        }
        if (part != NULL) {
            bit = 1 << part->bit;
            if (!(m->lostParts & bit)) {
                if (part->unk6 == 15) {
                    m->unkA0 |= bit;
                } else {
                    m->unkA2 |= bit;
                }
            }
            if (part->kind == 2 && obj->interactObj->hitPoints > 0) {
                func_m003_0C038(obj);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0C1A0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0C324);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0CAA4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0CD34);

void func_m003_0CF40(void) {
    s32 shotBy = sMember->shotBy;

    if (!(sMember->unk4 & 1)) {
        sMember->shotBy = sClosestPlayer;
        func_m003_0C324();
        if (sMember->shotBy == 0) {
            sMember->shotBy = shotBy;
        }
    }
}

extern void func_80003614(s32 handle);

/* A member is set on fire (or dies at once if its body cannot burn) */
void func_m003_0CFA8(Object *obj) {
    if (func_m003_05450(obj)) {
        if (sHomeData->kind != 0x112) {
            if (sBody->unk2B == 5 || ((M003BodyFlags *) &sBody->unk24)->fireproof) {
                ((M003Interact *) obj->interactObj)->sound = 6;
                func_m003_0C038(obj);
            } else {
                if (!sMember->headless && !sMember->onFire) {
                    if (sMember->sound != 0) {
                        func_80003614(sMember->sound);
                    }
                    func_80003410(func_80048E2C(0, 1) + 0x191, obj->trans.x_position, obj->trans.y_position,
                                  obj->trans.z_position, 4, &sMember->sound);
                }
                rl_m003_044F4(obj);
                func_m003_04D08(8, 15, 13, 0);
                func_m003_04260(9, 11, 0, 0, 0);
            }
            sMember->onFire = 1;
            sMember->unk6_4 = 0;
            sMember->unk3C = 0.0f;
        }
    }
}

/* A level object entry built on the stack (a spark on a member) */
typedef struct M003Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ Object *owner;
} M003Entry;

extern s32 func_80005064(s32 arg0);

/* A member is electrocuted */
void func_m003_0D134(Object *obj) {
    M003Entry entry;

    if (func_m003_05450(obj)) {
        if (!FLAGS8(sMember)->shocked && sHomeData->kind != 0xE6) {
            if (func_m003_04260(0x12, 0x19, (u8) func_80005064(func_80048E2C(120, 140)), 0, 0)) {
                func_m003_04D08(0xB, 0x15, 0, 0);
            }
            FLAGS8(sMember)->shocked = 1;
            FLAGS8(sMember)->unk24 = 3;
            entry.objectID = 0x29A;
            entry.size = 0x10;
            entry.unk3 = 0;
            entry.x = obj->trans.x_position;
            entry.y = obj->trans.y_position;
            entry.z = obj->trans.z_position;
            entry.owner = obj;
            if ((sMember->spark = func_80005F14(&entry, 1)) != NULL) {
                sMember->spark->level_entry = NULL;
            }
        }
    }
}

/* Looks up the body description of a member (behaviour 0x18) */
s32 func_m003_0D2C0(Object *obj) {
    Object *home = ((M003Member *) obj->unk68)->home;

    if (obj->behaviorId != 0x18 || home == NULL) {
        return 0;
    }
    sBodyIndex = troop_slot_get(((M003Home *) home->unk68)->kind);
    if (sBodyIndex != -1) {
        sBody = (M003Body *) (D_800A38E0 + (sBodyIndex << 7));
    } else {
        sBody = NULL;
    }
    return (s32) sBody;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0D360);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0D810);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0DF84);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0E3CC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0E9F0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0EB40);

/* A member loses its head to `by` */
void func_m003_100E0(Object *obj, Object *by) {
    s32 *byData = by->unk68;

    obj->interactObj->hitPoints = 0;
    ((M003Interact *) obj->interactObj)->hitBy = by;
    func_m003_0D2C0(obj);
    func_m003_10548(obj, *byData);
    if (sBody->headPart != 0x1F) {
        ((M003Interact *) obj->interactObj)->part = sBody->headPart;
        func_m003_0C0A0(obj);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_10160);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_10354);

/* A player shoots a member */
void func_m003_10548(Object *obj, s32 shotBy) {
    M003Member *m = obj->unk68;

    if (obj->objectID != 0x112) {
        if (m->headless) {
            sAlert = 1;
            sAlertKind = 10;
        } else if (D_800FE9FC == 1) {
            sAlert = 2;
            sAlertKind = 10;
        }
        m->shotBy = shotBy;
    }
}

/* Makes every member of the current troop blame `shotBy` */
static void func_m003_105B8(Object *shotBy) {
    Object *obj = sHomeData->first;
    M003Member *m;

    while (obj != NULL) {
        m = obj->unk68;
        obj = m->next;
        m->shotBy = (s32) shotBy;
    }
    sHomeData->unk30 = 3;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_10600);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_1398C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_13D94);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_13FE4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_142F0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_14694);

void func_m003_1477C(void) {
    if (!(D_800FE9B0 & 1) && !(D_800FE9B0 & 8)) {
        func_m003_0DF84();
    }
}

/* A member of the current troop is set up from its level entry (or spawned when that fails) */
void func_m003_147B8(LevelObjectEntryCommon *entry) {
    func_m003_0556C((Object *) troop_find_by_tag(((u8 *) entry)[0x11]));
    if (objflag_get(((u8 *) entry)[0x18])) {
        sHomeData->arrived++;
        if (sHomeData->arrived >= sHomeData->count) {
            if (sHomeData->unk0 == 0) {
                rl_m003_09840(sHomeData, 0);
            } else if (sHomeData->flags & 0x40) {
                func_m003_105B8(func_8000E4D0(0));
            } else if (sHomeData->kind2 == 2) {
                sHomeData->unk30 = 8;
            } else {
                sHomeData->unk30 = 2;
            }
        }
    } else {
        func_80005F14(entry, 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_148C0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_14D4C);

extern f32 point_distance_squared(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);

/* The object of one of the kinds troops can stand on that is nearest (x, y, z), or NULL */
s32 func_m003_15BE8(f32 x, f32 y, f32 z) {
    Object *obj;
    s32 first;
    s32 count;
    Object **objs;
    Object *nearest;
    u32 best;
    u32 dist;
    s32 i;

    best = -1;
    objs = func_80005E98(&first, &count);
    nearest = NULL;
    for (i = first; i < count; i++) {
        obj = objs[i];
        if (obj->freed) {
            continue;
        }
        if (obj->objectID == 0xA8 || obj->objectID == 0x87 || obj->objectID == 0x4A || obj->objectID == 0xAE ||
            obj->objectID == 0x2C3 || obj->objectID == 0x168 || obj->objectID == 0x135 ||
            obj->objectID == 0x171 || obj->objectID == 0x23A || obj->objectID == 0x1E3 ||
            obj->objectID == 0x1E4 || obj->objectID == 0x85 || obj->objectID == 0x86 ||
            obj->objectID == 0x1A8 || obj->objectID == 0x16F) {
            dist = point_distance_squared(x, y, z, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
            if (dist < best) {
                best = dist;
                nearest = obj;
            }
        }
    }
    return (s32) nearest;
}

/* Finds what each region's point lies in */
void func_m003_15E34(void) {
    s32 i = sRegionCount - 1;
    M003Region *region = &sRegions[1];

    while (i-- > 0) {
        if (region->flags & 0x20) {
            region->at.node = func_m003_15BE8(region->at.pos[0], region->at.pos[1], region->at.pos[2]);
        } else {
            region->at.node = 0;
        }
        region++;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_15ED8);

/* Frees the regions */
void func_m003_16530(void) {
    s32 i;
    M003Region *region;

    if (sRegionCount != 0) {
        i = sRegionCount - 1;
        region = &sRegions[1];
        while (i-- > 0) {
            if (region->data != NULL) {
                func_8004AA80(region->data);
            }
            region++;
        }
        func_8004AA80(sRegions);
        func_8004AA80(sRegionData);
        sRegions = NULL;
        sRegionData = NULL;
    }
}

/* Fini: frees the way points, the region data and the per-level tables */
void func_m003_165D8(void) {
    s32 i;
    void *data;

    data = func_80050F44(&i);
    if (D_800FEA9C != NULL) {
        func_8004AA80(D_800FEA9C);
        D_800FEA9C = NULL;
    }
    if (D_800FE9A4 != NULL) {
        func_8004AA80(D_800FE9A4);
        D_800FE9A4 = NULL;
    }
    if (data != NULL) {
        func_8004AA80(data);
    }
    i = 16;
    while (i--) {
        if (D_800FEA18[i] != NULL) {
            func_8004AA80(D_800FEA18[i]);
        }
        D_800FEA18[i] = NULL;
    }
    func_m003_16530();
}
#pragma weak __rl_fini = func_m003_165D8

/* Whether two members may collide */
s32 func_m003_166AC(Object *a, Object *b) {
    M003Home *dataA = a->unk68;
    M003Home *dataB = b->unk68;
    Object *homeA = ((M003Member *) dataA)->home;
    Object *homeB = ((M003Member *) dataB)->home;

    if (homeA != NULL) {
        dataA = homeA->unk68;
    } else {
        dataA = NULL;
    }
    if (homeB != NULL) {
        dataB = homeB->unk68;
    } else {
        dataB = NULL;
    }

    if (homeA == homeB && dataA != NULL && !(dataA->flags & 0x2000)) {
        return FALSE;
    }
    if ((dataA != NULL && b == dataA->leader) || (dataB != NULL && a == dataB->leader)) {
        return FALSE;
    }
    return TRUE;
}

/* Clears the events with this id */
void func_m003_16750(u32 id) {
    M003Event *event = sEvents;
    s32 eventId;
    s32 i = 16;

    while (i--) {
        if (event->active && (eventId = event->id & ~3) == id) {
            event->active = FALSE;
        }
        event++;
    }
}

#ifdef NON_MATCHING
/* Adds an event the troops react to (or renews the one with this id) */
/* (IDO keeps the extern counter's address in a register; the ROM addresses the .data word at each use) */
void func_m003_167A0(u32 id, s16 x, s16 y, s16 z, u8 arg4, u8 arg5, u8 arg6) {
    M003Event *event;
    s32 found = FALSE;
    s32 i;

    if (id != 0) {
        event = sEvents;
        i = 15;
        do {
            if (event->active && id == event->id) {
                found = TRUE;
                break;
            }
            event++;
        } while (i--);
    }
    if (found) {
        event->active = 10;
    } else {
        event = &sEvents[rl_m003_197F8++];
        if (rl_m003_197F8 >= 16) {
            rl_m003_197F8 = 0;
        }
        event->active = func_80048E2C(30, 40) * (u32) D_800FE9F8;
    }
    event->x = x;
    event->y = y;
    event->z = z;
    event->unk6 = arg4;
    event->id = id;
    event->unk7 = arg5;
    event->unk9 = arg6;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_167A0);
#endif

/* Lists the objects with behaviour 0x9B (up to 32) */
void func_m003_168D8(void) {
    s32 first;
    s32 end;
    Object **objs;
    Object **target;
    Object *obj;
    s32 isTarget;
    s32 i;

    objs = func_80005E98(&first, &end);
    target = sTargets;
    sTargetCount = 0;
    for (i = first; i < end; i++) {
        obj = objs[i];
        if (!obj->freed) {
            isTarget = FALSE;
            switch (obj->behaviorId) {
                case 0x9B:
                    isTarget = TRUE;
                    break;
            }
            if (isTarget) {
                *target++ = obj;
                if (++sTargetCount >= 32) {
                    break;
                }
            }
        }
    }
}

extern u8 rl_m003_19448[4];  /* cleared every frame */
extern s32 rl_m003_1944C;    /* flips every frame */
extern u8 rl_m003_19430;     /* a countdown */
extern s32 D_800FEA98;       /* the number of way points */
extern void func_8004665C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_m154_00000(void);
extern void func_m139_00030(void);
extern void rl_m003_13FE4(s32 updateRate);

/* The troops' per-frame update */
void func_m003_16990(s32 updateRate) {
    M003Event *e = sEvents;
    s32 i;
    M003Node *node;
    Object *player;
    u8 *p;

    player = func_8000E4D0(0);
    for (p = rl_m003_19448; p < rl_m003_19448 + 4; p++) {
        *p = 0;
    }
    if (sChangePending) {
        if (D_800FE9F8 >= sChangeArg1) {
            func_8004665C(sChangeArg0, ((s8 *) ((Object *) sClosestPlayer)->unk68)[1], 0, 0x10, 1, 0);
            sChangePending = 0;
        } else {
            sChangeArg1 -= D_800FE9F8;
        }
    }
    rl_m003_1944C ^= 1;
    if (sAlertKind != 0) {
        if (--sAlertKind == 0) {
            sAlert = 0;
        }
    }
    if (player != NULL) {
        s8 *data = player->unk68;

        if (D_800FE9FC != data[0x189]) {
            D_800FE9FC = data[0x189];
        }
    }
    i = 15;
    do {
        if (e->active != 0) {
            e->active--;
        }
        e++;
    } while (i--);
    if (sBssA < 10) {
        sBssA++;
    }
    if (rl_m003_19430 != 0) {
        rl_m003_19430--;
    }
    if (D_800FEA98 != 0) {
        func_m154_00000();
    }
    if (D_800FEA98 != 0) {
        i = D_800FEA98 - 1;
        node = &D_800FEA9C[1];
        while (i--) {
            if (node != NULL && node->obj != NULL) {
                node->x = node->obj->trans.x_position;
                node->y = node->obj->trans.y_position;
                node->z = node->obj->trans.z_position;
            }
            node++;
        }
    }
    if (D_800FE9B0 & 4) {
        func_m139_00030();
    }
    if (D_800FE9B0 & 0x10) {
        func_m003_168D8();
    }
    rl_m003_13FE4(updateRate);
}

void func_m003_16C10(Object *obj) {
    Object *saved = sObj;

    if (func_m003_05450(obj) && sMember->unk1A != 14) {
        func_m003_04260(12, 14, 0, 0, 0);
        sMember->unk0_20 = 0;
    }
    if (D_800FE9FC != 0) {
        sAlert = 3;
        sAlertKind = 10;
    }
    func_m003_05450(saved);
}

/* What a junction object keeps in obj->unk68 */
typedef struct M003Junction {
    /* 0x00 */ s16 unk0[4];
    /* 0x08 */ u8 unk8[4];
    /* 0x0C */ s8 unkC[4];
    /* 0x10 */ u8 flags; /* 1: set up */
} M003Junction;

/* A junction's set-up (once): its four exits */
void func_m003_16CB4(Object *obj, s32 updateRate) {
    M003Junction *data = obj->unk68;
    s32 i = 4;
    s16 a[4];
    s16 b[4];
    u8 *entry = (u8 *) obj->level_entry;

    if (!(data->flags & 1)) {
        anim_paths_find_near(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, a, b);
        while (i--) {
            data->unk8[i] = entry[0xC + i];
            data->unk0[i] = a[i];
            data->unkC[i] = b[i];
        }
        data->flags |= 1;
    }
}

extern u8 D_800F31E0[];
extern s32 rl_m003_148C0(Object *home, s32 arg1);

/* When memory is low, sends the waiting troops none of whose members can be seen or is busy away */
void func_m003_16D78(void) {
    Object **homes;
    Object *home;
    M003Home *h;
    Object *obj;
    M003Member *m;
    s32 count;
    s32 ok;

    homes = func_80050F44(&count);
    if (sBssA < 10) {
        return;
    }
    while (count--) {
        home = homes[count];
        h = home->unk68;
        if (h->unk30 >= 2 && h->unk30 < 5) {
            ok = TRUE;
            for (obj = h->first; obj != NULL; obj = m->next) {
                m = obj->unk68;
                if (!obj->freed &&
                    (h->unk75 || D_800F31E0[obj->segmentID + 1] || (m->unk0_1 && m->unk0_0) || m->headless)) {
                    ok = FALSE;
                    break;
                }
            }
            if (ok && !(rl_m003_148C0(home, 0) & 1)) {
                h->unk30 = 9;
                h->unk2E = 10;
            }
        }
    }
}

/* Sets the current member and home directly */
void func_m003_16EEC(Object *obj, Object *home) {
    sObj = obj;
    if (sObj != NULL) {
        sMember = sObj->unk68;
    } else {
        sMember = NULL;
    }
    sHome = home;
    if (sHome != NULL) {
        sHomeData = sHome->unk68;
    } else {
        sHomeData = NULL;
    }
}

/* A node of a region's tree: a line z = slope * x + offset, or (leaf) a run of edges */
typedef struct M003RegionNode {
    /* 0x00 */ struct M003RegionNode *below; /* leaf: u16 first edge, u16 edge count */
    /* 0x04 */ struct M003RegionNode *above;
    /* 0x08 */ f32 slope;
    /* 0x0C */ f32 offset;
    /* 0x10 */ u8 leaf;
} M003RegionNode;

/* A leaf of that tree: its run of edges */
typedef struct M003RegionLeaf {
    /* 0x00 */ u16 first;
    /* 0x02 */ u16 count;
} M003RegionLeaf;

/* An edge of a region (0x14 bytes) */
typedef struct M003Edge {
    /* 0x00 */ f32 x0;
    /* 0x04 */ f32 z0;
    /* 0x08 */ f32 x1;
    /* 0x0C */ f32 z1;
    /* 0x10 */ u8 pad10[4];
} M003Edge;

/* Whether a point is in a region (inside all the edges of its tree's leaf) */
s32 func_m003_16F44(f32 x, f32 z, M003Region *region) {
    M003RegionNode *node = region->data;
    M003Edge *edge;
    s32 i;
    s16 a;
    s16 b;

    if (region < sRegions || sRegionCount < region - sRegions || region->numEdges < 3) {
        return FALSE;
    }
    while (!node->leaf) {
        if (z < node->slope * x + node->offset) {
            node = node->below;
        } else {
            node = node->above;
        }
    }
    edge = (M003Edge *) region->edges + ((M003RegionLeaf *) node)->first;
    i = ((M003RegionLeaf *) node)->count;
    while (i--) {
        a = func_80049C84(edge->z0 - z, edge->x0 - x);
        b = func_80049C84(edge->z1 - z, edge->x1 - x);
        if (func_80049D80(a, b) < 0) {
            return FALSE;
        }
        edge++;
    }
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_170D0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_17174);

extern void rl_m003_17174(f32 x1, f32 z1, f32 x2, f32 z2, void *node);
extern void rl_m003_05650(M003Region *region, s32 point, u8 *a, u8 *b);

#ifdef NON_MATCHING
/* 2 words: the ROM computes `point & 0xFF` one store earlier */
/*
 * Where the segment (x1, z1)-(x2, z2) leaves a region (into *out if out is not NULL); 0 if it doesn't. `point`
 * (not -1) is a point of region `regionIndex` the segment starts at, used if the crossing is next to it.
 */
s32 func_m003_175E8(f32 x1, f32 z1, f32 x2, f32 z2, M003Region *region, M003Crossing *out, s32 point,
                    u8 regionIndex) {
    M003Region *to;
    M003Region *from = func_m003_0569C(regionIndex);
    M003Point *p;

    if (region < sRegions || sRegionCount < region - sRegions || region->numEdges < 3) {
        return FALSE;
    }
    sCrossData = region->data;
    sCrossEdges = region->edges;
    sCrossX1 = x1;
    sCrossZ1 = z1;
    sCrossX2 = x2;
    sCrossZ2 = z2;
    sCrossEdge = -1;
    sCrossOut = out;
    rl_m003_17174(x1, z1, x2, z2, sCrossData);
    if (sCrossEdge == -1) {
        return FALSE;
    }
    if (out != NULL) {
        out->x = sCrossX;
        out->z = sCrossZ;
        out->point = sCrossPoint;
        out->region = sCrossRegion;
        to = func_m003_0569C(sCrossRegion);
        rl_m003_05650(to, out->point, &out->unk5, &out->unk6);
        if (point != -1) {
            p = &from->points[point];
            if (sCrossX1 == p->x && sCrossZ1 == p->z &&
                !func_m003_16F44(((sCrossX - sCrossX1) >> 1) + sCrossX1, ((sCrossZ - sCrossZ1) >> 1) + sCrossZ1,
                                 region)) {
                out->x = p->x;
                out->point = point;
                out->z = p->z;
                out->region = regionIndex;
                rl_m003_05650(from, point & 0xFF, &out->unk5, &out->unk6);
                out->atPoint = FALSE;
                return TRUE;
            }
        }
        p = &to->points[out->point];
        if (sCrossX == p->x && sCrossZ == p->z) {
            out->atPoint = TRUE;
        } else {
            out->unk5 = out->point;
            out->point = 0xFF;
            out->atPoint = FALSE;
        }
    }
    return TRUE;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_175E8);
#endif

s32 func_m003_17910(f32 x, f32 z, s32 index) {
    return func_m003_16F44(x, z, index + sRegions);
}

s32 func_m003_1794C(f32 x1, f32 z1, f32 x2, f32 z2, s32 index, s32 arg5, s32 arg6, u8 arg7) {
    return func_m003_175E8(x1, z1, x2, z2, index + sRegions, (M003Crossing *) arg5, arg6, arg7);
}

/* Asks for a level change (once) */
void func_m003_179B0(s32 arg0, s32 arg1) {
    if (sChangePending == 0) {
        sChangeArg1 = arg1;
        sChangeArg0 = arg0;
        sChangePending = 1;
    }
}

/* Init */
void func_m003_179E0(void) {
    func_m003_15ED8(D_800FEA00, D_800FEA04);
}
#pragma weak __rl_init = func_m003_179E0

INCLUDE_ASM("asm/nonmatchings/modules/m003", data);

INCLUDE_ASM("asm/nonmatchings/modules/m003", rodata);


