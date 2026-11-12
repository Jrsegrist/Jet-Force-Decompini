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
    /* 0x00 */ u32 pad0_3 : 2;
    /* 0x00 */ u32 unk0_26 : 1; /* (0B70C) */
    /* 0x00 */ u32 pad0_8 : 4;
    /* 0x00 */ u32 unk0_21 : 1;
    /* 0x00 */ u32 unk0_20 : 1;
    /* 0x00 */ u32 placed : 1; /* its level entry gave a place (0DF84) */
    /* 0x00 */ u32 unk0_18 : 1; /* (0E9F0: moved by 0E3CC) */
    /* 0x00 */ u32 unk0_17 : 1; /* (0A850: the member moves five times as fast) */
    /* 0x00 */ u32 pad0_7 : 2;
    /* 0x00 */ u32 unlinked : 1; /* not in its troop's list */
    /* 0x00 */ u32 pad0_5 : 1;
    /* 0x00 */ u32 busy : 1; /* not ready to move on */
    /* 0x00 */ u32 unk0_11 : 1; /* (0B70C) */
    /* 0x00 */ u32 partsSet : 1; /* set when 10160 or 10354 picked a part (bit 10) */
    /* 0x00 */ u32 unk0_9 : 1; /* cleared when its troop's leader goes */
    /* 0x00 */ u32 pad0_6 : 4;
    /* 0x00 */ u32 unk0_4 : 1;
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
    /* 0x12 */ u8 pad12[0x14 - 0x12];
    /* 0x14 */ u8 troop; /* its troop's tag (0DF84) */
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[0x19 - 0x16];
    /* 0x19 */ u8 animSet;
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B; /* < 5: heading for a way point */
    /* 0x1C */ s16 unk1C; /* the way it moves (03CAC) */
    /* 0x1E */ s16 unk1E; /* and how steeply */
    /* 0x20 */ f32 unk20; /* and how fast */
    /* 0x24 */ Object *home;
    /* 0x28 */ Object *next; /* the next member of the troop */
    /* 0x2C */ f32 lastX; /* where it was before its last move (0E9F0) */
    /* 0x30 */ f32 lastY;
    /* 0x34 */ f32 lastZ;
    /* 0x38 */ f32 unk38;
    /* 0x3C */ f32 unk3C;
    /* 0x40 */ f32 unk40;
    /* 0x44 */ u8 pad44[0x4A - 0x44];
    /* 0x4A */ u8 unk4A;
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ s16 unk4C; /* how fast it turns (0A5EC) */
    /* 0x4E */ u8 pad4E[0x50 - 0x4E];
    /* 0x50 */ u8 node;
    /* 0x51 */ u8 unk51;
    /* 0x52 */ u8 unk52;
    /* 0x53 */ s8 unk53; /* a countdown (00FD0) */
    /* 0x54 */ s8 event; /* the event it heard (sEvents), -1: none */
    /* 0x55 */ u8 unk55;
    /* 0x56 */ s8 unk56;
    /* 0x57 */ u8 unk57; /* from its entry, or its home's unk8C counted up (0DF84) */
    /* 0x58 */ u8 pad58[0x5A - 0x58];
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
    /* 0xAC */ Object *unkAC; /* freed when body 0x20 starts an animation (04968) */
    /* 0xB0 */ struct M003Flags4 *unkB0;
    /* 0xB4 */ Object *follower; /* an object kept at the member's place (0E9F0) */
    /* 0xB8 */ u8 padB8[0xBC - 0xB8];
    /* 0xBC */ s32 sound; /* the burning sound's handle */
    /* 0xC0 */ s32 light; /* the fire's light */
    /* 0xC4 */ s32 unkC4; /* (01764) */
    /* 0xC8 */ Object *spark; /* the electric spark on the member */
} M003Member;

/* The member's word at 0x8 as bit fields */
typedef struct M003Flags8 {
    u32 pad4 : 4;
    u32 unk27 : 1;
    u32 shocked : 1;
    u32 unk24 : 2;
    u32 rest : 24;
} M003Flags8;
#define FLAGS8(m) ((M003Flags8 *) &(m)->flags8)

/* The member's word at 0x4 as bit fields (also the word at its unkB0) */
typedef struct M003Flags4 {
    u32 pad25 : 25;
    u32 unk6 : 1;
    u32 unk5 : 1;
    u32 pad3 : 2;
    u32 unk2 : 1;
    u32 pad0 : 2;
} M003Flags4;
#define FLAGS4(m) ((M003Flags4 *) &(m)->unk4)

/* The member's group byte as bit fields */
typedef struct M003Group {
    u8 inactive : 1; /* the sign bit of `group` */
    u8 rest : 7;
} M003Group;

/* The body's word at 0x24 as bit fields */
typedef struct M003BodyFlags {
    u32 pad20 : 20;
    u32 unk11 : 1;
    u32 unk10 : 1; /* (0B70C) */
    u32 pad6 : 4;
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
    /* 0x13 */ u8 unk13; /* how far it looks round (028D4), in 16s */
    /* 0x14 */ u8 unk14; /* and up and down, in 8s */
    /* 0x15 */ u8 pad15[0x1A - 0x15];
    /* 0x1A */ u16 kind2 : 4;
    /* 0x1A */ u16 unk1A : 12;
    /* 0x1C */ u8 pad1C[0x1E - 0x1C];
    /* 0x1E */ u8 spacing; /* how many trail steps apart its members follow the leader (0237C) */
    /* 0x1F */ u8 pad1F[0x21 - 0x1F];
    /* 0x21 */ u8 unk21; /* how loud an event its members notice must be */
    /* 0x22 */ u8 pad22[0x24 - 0x22];
    /* 0x24 */ s8 maxSpeed; /* its members' (0AD38) */
    /* 0x25 */ s8 minSpeed;
    /* 0x26 */ u8 speed; /* its members' walking speed (0BA20, 0BAD4) */
    /* 0x27 */ u8 pad27;
    /* 0x28 */ u16 flags; /* 0x2000: its members may touch */
    /* 0x2A */ s16 unk2A; /* how fast the troop's members burn (01764) */
    /* 0x2C */ u16 kind;
    /* 0x2E */ s16 unk2E; /* a countdown */
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31; /* 1: its members follow the leader's trail (0237C) */
    /* 0x32 */ u8 unk32; /* its state (0B50C) */
    /* 0x33 */ u8 unk33;
    /* 0x34 */ u8 unk34; /* the low two bits pick module 6's members' animations (04968) */
    /* 0x35 */ u8 unk35; /* set while a way-point command runs (01C58) */
    /* 0x36 */ u8 trailPos; /* the trail entry the leader writes next (0237C) */
    /* 0x37 */ u8 pad37[0x3C - 0x37];
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
    /* 0x70 */ u8 pad70[0x74 - 0x70];
    /* 0x74 */ u8 unk74; /* the last way point with flag 2 it reached (03AA8) */
    /* 0x75 */ u8 unk75;
    /* 0x76 */ u8 pad76;
    /* 0x77 */ u8 unk77; /* its D_800FEA18 slot, 0xFF: none */
    /* 0x78 */ s8 group; /* its path group, 0 to 31 (061FC) */
    /* 0x79 */ u8 pad79[0x7B - 0x79];
    /* 0x7B */ u8 unk7B;
    /* 0x7C */ u8 pad7C[0x8C - 0x7C];
    /* 0x8C */ s32 unk8C; /* a game flag, counted up (14694) */
    /* 0x90 */ u8 pad90;
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
    /* 0x18 */ u8 cmd; /* what a member does there (module 154) */
    /* 0x19 */ u8 unk19; /* how near the troop's first member counts as there (0322C) */
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 pad1D[0x31 - 0x1D];
    /* 0x31 */ u8 radius; /* how near counts as there (02760) */
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
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ f32 unk10; /* a distance (0AF34, 0AFC4) */
    /* 0x14 */ u8 pad14[0x24 - 0x14];
    /* 0x24 */ s16 unk24; /* a height */
    /* 0x26 */ u8 pad26[0x28 - 0x26];
    /* 0x28 */ u8 turnStep; /* how much faster or slower its members turn each step (0A5EC) */
    /* 0x29 */ u8 numParts;
    /* 0x2A */ u8 pad2A;
    /* 0x2B */ u8 unk2B; /* 5: does not burn */
    /* 0x2C */ u8 pad2C;
    /* 0x2D */ u8 headPart; /* 0x1F: none */
    /* 0x2E */ u8 pad2E;
    /* 0x2F */ u8 hitPart; /* the body part the last hit took off, 7: none */
    /* 0x30 */ u8 pad30[0x34 - 0x30];
    /* 0x34 */ u8 turns[3]; /* 0, 0, 0: it turns straight to face an event (00D44) */
    /* 0x37 */ u8 pad37[0x40 - 0x37];
    /* 0x40 */ M003BodyPart parts[8];
} M003Body;

/* What a member's interactObj points at */
typedef struct M003Interact {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u32 pad8_0 : 2;
    /* 0x08 */ u32 unk8_29 : 1; /* (04968: picks other animations) */
    /* 0x08 */ u32 unk8_3 : 1;
    /* 0x08 */ u32 sound : 4;
    /* 0x08 */ u32 pad8_1 : 24;
    /* 0x0C */ f32 centre[3];
    /* 0x18 */ f32 pos[3]; /* where it was hit */
    /* 0x24 */ f32 radius;
    /* 0x28 */ f32 height;
    /* 0x2C */ u8 pad2C[0x31 - 0x2C];
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
extern s32 obj_find_nearest_in_category(f32 x, f32 y, f32 z, s32 arg3);
extern void func_800492DC(Object *obj, Vec3f *v); /* turns v by the object's rotation */
extern s32 troop_find_member(s16 arg0, u8 arg1);
extern void stats_kill_add(s32 sound);
extern void troop_spawn_context_set(Object *obj, Object *home);
extern void func_m154_017E8(Object *home);
static s32 func_m003_0527C(f32 x, f32 y, f32 z);
extern f32 point_distance_squared(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
extern void func_m003_0DF84(); /* (Object *obj, void *entry); one C caller passes nothing */
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
extern s32 func_80051880(u8 arg0);
extern s32 func_m154_00DFC(u8 arg0, u8 arg1, u8 arg2);
extern void func_m155_02BB0(Object *home);
s32 func_m003_04D08(s32 kind, s32 anim1, s32 anim2, s32 anim3);
extern s32 func_m003_04F88(s32 arg0, f32 arg1);
extern void func_m003_06D28(Object *home, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
extern s32 func_m003_0C324(); /* (Object *obj); 0CF40 calls it with no argument */
extern s32 func_m003_15BE8(f32 x, f32 y, f32 z);
extern s32 func_m154_00130(f32 x, f32 y, f32 z, s32 arg3); /* the way point nearest a point */
extern void anim_paths_find_near(f32 x, f32 y, f32 z, s16 *a, s16 *b);
extern s32 func_8004966C(s16 angle); /* sine */
extern s32 func_80049668(s16 angle); /* cosine */
extern s32 troop_find_by_tag(s32 arg0);
extern s32 objflag_get(s32 arg0);
extern Object *func_80005F14(void *entry, s32 arg1);
extern void func_80048130(s32 arg0, s32 arg1);
extern void obj_spawn_bonus(f32 x, f32 y, f32 z, s32 arg3);
extern void spawn_object_from_list(s32 arg0, s16 x, s16 y, s16 z, s32 arg4, s32 arg5);
extern s32 D_800FE9F8;
extern void func_m014_019E4(void);
extern void func_m003_07E88(s32 arg0, s32 arg1, s16 arg2);
extern void func_m003_09A14(Object *obj);
static s32 func_m003_04968(s32 index, s32 anim, s32 adjust);
extern s32 rl_m003_04614(s32 anim, s32 arg1);
extern void rl_m003_044F4(Object *obj);

/* An animation entry (8 bytes) */
typedef struct M003Anim {
    /* 0x00 */ s16 unk0; /* the first entry's: the animation speed, in some unit (044B4) */
    /* 0x02 */ u8 anim;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u32 weight : 5; /* how likely 04614 picks it */
    /* 0x04 */ u32 pad4_5 : 1;
    /* 0x04 */ u32 unk4_25 : 1; /* turn on the spot when it need not get anywhere (0A5EC) */
    /* 0x04 */ u32 unk4_22 : 3; /* 3: the member loses a body part as it starts (04968), 4: one time in three */
    /* 0x04 */ u32 parts : 7; /* its row of rl_m003_17A10, 0x7F: none (09680) */
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
static f32 sBss10;              /* the speed for the member's next step (0BA20, 0BAD4) */
static s32 sBss14;
static f32 sBss18;              /* the height the member steps towards (0A9E8, 0B468) */
static s32 sBss1C;
static u8 sBss20[0xA8 - 0x20]; /* (IDO aligns arrays to 8) */
static f32 sUpdateRate;        /* this frame's, as a float */
static Object *sObj;            /* the member being processed */
static M003Member *sMember;     /* its data */
static Object *sHome;           /* its troop's home object */
static M003Home *sHomeData;     /* and that one's data */
static s16 sBodyIndex;
static M003Body *sBody;
static s32 sShotBy;
static s32 sClosestPlayer;
static f32 sBssCC; /* how near the member must get (0AF34, 0AFC4) */
static f32 sBssD0; /* how near a member must be to fire */
static s16 sBssD4; /* the way it should face when there (0AFC4) */
static s16 sBssD6; /* the way it turns towards (0A564) */
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
static u8 sBssDC8[0xDD8 - 0xDC8];
static f32 sCrossSlope;    /* the line 170D0 meets: z = slope * x + offset */
static f32 sCrossOffset;
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

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_00384);


/* An object's position; slows it and counts down its way point, moving on at 0 */
void func_m003_009C8(Object *obj, f32 *x, f32 *y, f32 *z) {
    f32 k = 0.96f;

    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    obj->velocity.x *= k;
    obj->velocity.z *= k;
    sMember->node--;
    if (sMember->node == 0) {
        func_m003_0C038(obj);
    }
}

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

s32 func_m003_0512C(s32 kind, f32 speed, s32 anim);

/*
 * Faces the event the member heard (at once, or turning, by its body); after a random while, or when
 * the event is gone, it goes back to its mission
 */
s32 func_m003_00D44(Object *obj, s16 *angle, f32 x, f32 z) {
    M003Event *e = &sEvents[sMember->event];

    if (sHomeData->unk32 != 10) {
        func_m003_0512C(3, 1.0f, 0x3B);
    }
    if (sMember->unk4B != 3) {
        func_m003_04F88(sMember->unk4B, 1.0f);
    }
    if (sMember->unk1B == 1) {
        sMember->unk53 = func_80005064(func_80048E2C(50, 65));
        sMember->unk1B = 4;
    } else {
        sMember->unk53 -= D_800FE9F8;
    }
    if (sMember->event == -1 || !e->active || sMember->unk53 < 0 ||
        (sHomeData->unk32 == 17 && (sHomeData->unk30 == 3 || sHomeData->unk30 == 4))) {
        func_m003_04398(1, 0);
        sMember->unk10 = 0;
        sMember->unk4B = 0;
        return FALSE;
    }
    if (sBody->turns[0] == 0 && sBody->turns[1] == 0 && sBody->turns[2] == 0) {
        *angle = func_80049C84(obj->trans.x_position - e->x, obj->trans.z_position - e->z);
        return TRUE;
    }
    *angle = obj->trans.rotation.y_rotation;
    sMember->unk10 = func_80049C84(obj->trans.x_position - e->x, obj->trans.z_position - e->z) -
                     obj->trans.rotation.y_rotation;
    sMember->turning = TRUE;
    return TRUE;
}

/* Turns towards the event the member heard; after a random while it goes back to its mission */
s32 func_m003_00FD0(Object *obj, s16 *angle, f32 x, f32 z) {
    M003Event *e = &sEvents[sMember->event];

    func_m003_04F88(sMember->unk4B, 1.0f);
    if (sMember->event == -1) {
        func_m003_04398(1, 0);
        sMember->unk4B = 0;
        sMember->unk10 = 0;
        return FALSE;
    }
    switch (sMember->unk1B) {
        case 1:
            sMember->unk53 = func_80005064(func_80048E2C(30, 40));
            sMember->unk1B = 4;
            break;
        case 4:
            sMember->unk53 -= D_800FE9F8;
            if (sMember->unk53 < 0) {
                func_m003_04398(1, 0);
                sMember->unk4B = 0;
                if (sHomeData->unk30 == 2 || sHomeData->unk30 == 3) {
                    sHomeData->unk30 = 4;
                }
            }
            break;
    }
    *angle = obj->trans.rotation.y_rotation;
    sMember->unk10 = func_80049C84(obj->trans.x_position - e->x, obj->trans.z_position - e->z) -
                     obj->trans.rotation.y_rotation;
    sMember->turning = TRUE;
    return TRUE;
}

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

extern void rl_m003_03CAC(Object *obj);

/*
 * An object's position; the member jumps off along the way its node, unk51 and unk52 give (yaw, pitch
 * and speed), first lifted by 1
 */
void func_m003_0136C(Object *obj, f32 *x, f32 *y, f32 *z) {
    f32 c;

    obj->trans.rotation.y_rotation = 0xC000 - (sMember->node << 8);
    c = func_80049C30(sMember->unk51 << 8);
    obj->velocity.x = func_80049C30(sMember->node << 8) * c * (u32) sMember->unk52;
    c = func_80049C30(sMember->unk51 << 8);
    obj->velocity.z = func_80049C34(sMember->node << 8) * c * (u32) sMember->unk52;
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    switch (sMember->unk1B) {
        case 1:
            obj->velocity.y = func_80049C34(sMember->unk51 << 8) * (u32) sMember->unk52;
            sMember->unk0_11 = FALSE;
            sMember->unk1B = 4;
            sMember->lastY += 1.0f;
            obj->trans.y_position += 1.0f;
            ((M003Interact *) obj->interactObj)->pos[1] += 1.0f;
            ((M003Interact *) obj->interactObj)->centre[1] += 1.0f;
            func_m003_04D08(3, 20, 0, 0);
            break;
        case 4:
            sHomeData->unk35 = 0;
            if (obj->unk28 < 0.5f) {
                func_m003_04F88(3, 1.0f);
            }
            break;
    }
    rl_m003_03CAC(obj);
}

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

extern void func_m146_00000(Object *obj, s32 arg1);
extern void func_8002179C(s32 handle);
extern void track_light_off(s32 light);

#ifdef NON_MATCHING
/* 2 diffs: the ROM computes `hp > 0` into a register before the store (one branch shorter) */
/*
 * An object's position; while the member burns its hit points go down, and when its time is up it
 * goes back to its mission and the fire's spark, unkC4 and light go
 */
void func_m003_01764(Object *obj, f32 *x, f32 *y, f32 *z) {
    f32 min = 0.2f;
    f32 rate;
    s16 hp;

    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    rate = sHomeData->unk2A * 0.002f;
    if (rate < min) {
        rate = min;
    }
    hp = ((M003Interact *) obj->interactObj)->hitPoints;
    ((M003Interact *) obj->interactObj)->hitPoints = hp - rate * sUpdateRate;
    if (((M003Interact *) obj->interactObj)->hitPoints <= 0 && hp > 0) {
        ((M003Interact *) obj->interactObj)->sound = 4;
        func_m003_0C038(obj);
    }
    if (sMember->node < D_800FE9F8) {
        func_m003_04398(1, 0);
        if (sHomeData->kind == 0x112) {
            sMember->unk4B = 3;
            func_m146_00000(obj, 1);
        } else {
            sMember->unk4B = 0;
        }
        sMember->flags8 &= ~4;
        if (sMember->spark != NULL) {
            func_800079B8(sMember->spark);
            sMember->spark = NULL;
            if (sMember->unkC4 != 0) {
                func_8002179C(sMember->unkC4);
                sMember->unkC4 = 0;
            }
            if (sMember->light != 0) {
                track_light_off(sMember->light);
                sMember->light = 0;
            }
        }
    } else {
        sMember->node -= D_800FE9F8;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_01764);
#endif

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

extern void func_m154_01110(Object *home, M003Node *node); /* starts the way point's command */

#ifdef NON_MATCHING
/* 22 diffs: the ROM computes the final && straight into v0 (the cmd reload in v1), one instruction shorter */
/* Whether the member goes on past way point `node`: its command (not kinds 3/9 and 1) is started on
   reaching it; `arrive` tells which of the two calls this is */
s32 func_m003_01C58(M003Node *node, s32 arrive) {
    u8 cmd = node->cmd;

    if ((1 << cmd) & 0x208) {
        if (arrive) {
            return FALSE;
        }
    } else if (!arrive) {
        return FALSE;
    }
    if (cmd != 1 && sHomeData->unk35 == 0) {
        sHomeData->unk33 = 0x17;
        sHomeData->unk35 = 1;
        sHomeData->unk2E = node->unk1C;
        func_m154_01110(sHome, node);
        return node->cmd != 3 && node->cmd != 9;
    }
    sHomeData->unk35 = 0;
    return FALSE;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01C58);
#endif

/* A node of an animation path (0x2C bytes) */
typedef struct M003PathNode {
    /* 0x00 */ u8 pad0[0x1E];
    /* 0x1E */ u8 junction; /* 0: none; else the id of the junction object that picks the way on */
    /* 0x1F */ u8 pad1F[0x28 - 0x1F];
    /* 0x28 */ struct M003PathNode *next;
} M003PathNode;

/* An animation path (D_801045B0) */
typedef struct M003AnimPath {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ M003PathNode *nodes;
} M003AnimPath;

/* What a junction object (behaviour 0x1B) keeps in obj->unk68 */
typedef struct M003Junction {
    /* 0x00 */ s16 path[4];   /* its four exits: D_801045B0 indices */
    /* 0x08 */ u8 weight[4];  /* how likely each exit is */
    /* 0x0C */ u8 node[4];    /* the node of that path to go on from */
    /* 0x10 */ u8 flags;      /* 1: set up */
    /* 0x11 */ u8 id;
} M003Junction;

extern M003AnimPath **D_801045B0;

/*
 * The node after `node`: at a junction, the start of one of the junction's exits, picked at random by
 * weight. (If no junction object has the id, `j` is used unset, as in the original.)
 */
M003PathNode *func_m003_01D3C(M003PathNode *node) {
    s32 first;
    s32 count;
    s32 i;
    s32 total;
    s32 r;
    M003Junction *j;
    M003PathNode *next;
    Object **objs;
    Object *o;
    u8 id;
    M003AnimPath *path;

    next = node->next;
    objs = func_80005E98(&first, &count);
    id = next->junction;
    total = 0;
    if (id == 0) {
        return next;
    }
    for (r = first; r < count; r++) {
        o = objs[r];
        if (o->freed == 0 && o->behaviorId == 0x1B) {
            j = o->unk68;
            if (j->id == id) {
                break;
            }
        }
    }
    r = 3;
    do {
        total += j->weight[r];
    } while (r--);
    if (total <= 0) {
        i = 0;
    } else {
        r = func_80048E2C(1, total);
        i = 0;
        while (TRUE) {
            r -= j->weight[i];
            if (r <= 0) {
                break;
            }
            i++;
        }
    }
    total = j->path[i];
    path = D_801045B0[total];
    next = path->nodes;
    r = j->node[i];
    while (r--) {
        next = next->next;
    }
    return next;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01EB8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_021C8);

/* A step of the leader's trail (D_800FEA18[home->unk77] holds the troop's ring of them) */
typedef struct M003Trail {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ s16 rotY;
    /* 0x0E */ s16 rotX;
    /* 0x10 */ s16 rotZ;
} M003Trail;

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void rl_m003_01EB8(void);                   /* still assembly */
extern void rl_m003_021C8(void);

/*
 * The troop follows its leader: it keeps the leader's place in unk40-44, or (unk31 1) records the leader in
 * a ring of steps and puts each member that many steps behind it
 */
void func_m003_0237C(void) {
    s32 total;
    s32 i;
    s32 j;
    M003Trail *t;

    if (sHomeData->leader != NULL) {
        if (sHomeData->unk31 != 1) {
            sHomeData->unk40 = sHomeData->leader->trans.x_position;
            sHomeData->unk42 = sHomeData->leader->trans.y_position;
            sHomeData->unk44 = sHomeData->leader->trans.z_position;
        } else {
            total = sHomeData->spacing * 2 * sHomeData->count;
            if (sHomeData->group == -1) {
                rl_m003_01EB8();
            } else {
                rl_m003_021C8();
            }
            if (sHomeData->unk77 == 0xFF) {
                i = 15;
                do {
                    if (D_800FEA18[i] == NULL) {
                        break;
                    }
                } while (i--);
                sHomeData->unk77 = i;
                sHomeData->trailPos = 0;
                D_800FEA18[i] = func_8004A57C(total * sizeof(M003Trail), 0xFFFF00FF);
            }
            t = &((M003Trail *) D_800FEA18[sHomeData->unk77])[sHomeData->trailPos];
            t->x = sHomeData->leader->trans.x_position;
            t->y = sHomeData->leader->trans.y_position;
            t->z = sHomeData->leader->trans.z_position;
            t->rotY = sHomeData->leader->trans.rotation.y_rotation;
            t->rotX = sHomeData->leader->trans.rotation.x_rotation;
            t->rotZ = sHomeData->leader->trans.rotation.z_rotation;
            for (sObj = sHomeData->first; sObj != NULL; sObj = sMember->next) {
                sMember = sObj->unk68;
                if (sMember->unkA6 > 0) {
                    j = sHomeData->trailPos - sMember->unk15 * (sHomeData->spacing << 1);
                    if (j < 0) {
                        j += total;
                    }
                    t = &((M003Trail *) D_800FEA18[sHomeData->unk77])[j];
                    sObj->velocity.x = 0.0f;
                    sObj->velocity.y = 0.0f;
                    sObj->velocity.z = 0.0f;
                    rl_m003_03CAC(sObj);
                    sObj->trans.x_position = t->x;
                    sObj->trans.y_position = t->y;
                    sObj->trans.z_position = t->z;
                    sObj->trans.rotation.y_rotation = t->rotY;
                    sObj->trans.rotation.x_rotation = t->rotX;
                    sObj->trans.rotation.z_rotation = t->rotZ;
                }
            }
            sHomeData->trailPos++;
            if (sHomeData->trailPos >= total) {
                sHomeData->trailPos = 0;
            }
        }
    }
}

/* How near the member must get to way point `node`: farther when its troop's unk6C is 1 */
s32 func_m003_02760(M003Node *node) {
    if (sHomeData->unk6C == 1.0f) {
        return node->radius + 40;
    }
    return node->radius;
}

extern s32 func_m154_01278(void);
extern f32 point_distance(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2); /* the distance between two points */
extern s32 rl_m003_02760(M003Node *node);
extern s32 rl_m003_01C58(M003Node *node, s32 arrive);

/* Once the troop's first member is near enough its way point, starts the point's command and moves on */
void func_m003_027A4(void) {
    UNUSED s32 pad[3];
    f32 dist;
    Object *first;
    M003Node *node;
    s32 lo;
    s32 hi;

    if (func_m154_01278()) {
        first = sHomeData->first;
        node = &D_800FEA9C[sHomeData->unk3C & 0xFF];
        if (!sMember->unk0_11 && sHomeData->unk6C == 1.0f) {
            dist = point_distance(first->trans.x_position, first->trans.y_position, first->trans.z_position,
                                 node->x, node->y, node->z);
            if (dist < rl_m003_02760(node) && !rl_m003_01C58(node, 1)) {
                lo = sHomeData->unk3C;
                hi = sHomeData->unk3C;
                if (func_80051880(func_m154_00DFC(sHomeData->unk30, lo & 0xFF, (u16) hi >> 8))) {
                    rl_m003_01C58(node, 0);
                }
            }
        }
    }
}

extern u8 D_800FE9EE;

/* Counts down the troop's wait; at 0 it turns its look-out point round and goes to state 20 */
void func_m003_028D4(void) {
    s32 x;
    s32 y;
    s32 z;

    sHomeData->unk2E--;
    if (sHomeData->unk2E == 0) {
        sHomeData->unk3C += 0x8C40;
        x = func_80049C34(sHomeData->unk3C) * (f32) (sHomeData->unk13 * 16);
        z = func_80049C30(sHomeData->unk3C) * (f32) (sHomeData->unk13 * 16);
        y = func_80048E2C(-(sHomeData->unk14 * 8), sHomeData->unk14 * 8);
        sHomeData->unk40 = sHomeData->offset[0] * 16 + x;
        sHomeData->unk42 = sHomeData->offset[1] * 16 + y;
        sHomeData->unk44 = sHomeData->offset[2] * 16 + z;
        sHome->velocity.x = 0.0f;
        sHome->velocity.y = 0.0f;
        sHome->velocity.z = 0.0f;
        sHomeData->unk33 = 20;
        D_800FE9EE = 20;
    }
}

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

/*
 * Turns the troop's home (by up to 400 a frame) towards its look-out point; once facing it, sets it
 * moving there at half its top speed
 */
void func_m003_02DA0(void) {
    s16 target;
    s16 d;
    f32 t;

    target = func_80049C84(sHomeData->unk40 - sHomeData->x, sHomeData->unk44 - sHomeData->z);
    d = func_80049D80(sHome->trans.rotation.y_rotation, target);
    if (d > 400) {
        d = 400;
    } else if (d < -400) {
        d = -400;
    }
    sHome->trans.rotation.y_rotation += d;
    if (target == sHome->trans.rotation.y_rotation) {
        t = point_distance(sHomeData->unk40, sHomeData->unk42, sHomeData->unk44, sHomeData->x, sHomeData->y,
                           sHomeData->z) /
            (sHomeData->maxSpeed >> 1);
        sHomeData->unk2E = t;
        t = 1.0f / t;
        if (sHomeData->unk40 != sHomeData->x) {
            sHome->velocity.x = (sHomeData->unk40 - sHomeData->x) * t;
        } else {
            sHome->velocity.x = 0.0f;
        }
        if (sHomeData->unk42 != sHomeData->y) {
            sHome->velocity.y = (sHomeData->unk42 - sHomeData->y) * t;
        } else {
            sHome->velocity.y = 0.0f;
        }
        if (sHomeData->unk44 != sHomeData->z) {
            sHome->velocity.z = (sHomeData->unk44 - sHomeData->z) * t;
        } else {
            sHome->velocity.z = 0.0f;
        }
        sHomeData->unk33 = 0;
    }
}

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

f32 func_m003_03E20(f32 x1, f32 z1, f32 x2, f32 z2);
extern void rl_m003_03CAC(Object *obj);

#ifdef NON_MATCHING
/* 7 diffs: the ROM's constant (a .rodata literal) takes f14, after dx, dy, dz (C: f0, first) */
/* When the troop's first member is near its way point, sends it there (state 30) */
void func_m003_0322C(void) {
    M003Node *node;
    Object *first;
    f32 k;
    f32 dx;
    f32 dy;
    f32 dz;

    first = sHomeData->first;
    node = &D_800FEA9C[sHomeData->unk3C & 0xFF];
    if (func_m003_03E20(first->trans.x_position, first->trans.z_position, node->x, node->z) < node->unk19) {
        k = 0.1f;
        sHomeData->unk33 = 30;
        sHomeData->unk3E = 10;
        dx = node->x - first->trans.x_position;
        dy = node->y - first->trans.y_position;
        dz = node->z - first->trans.z_position;
        first->velocity.x = dx * k;
        first->velocity.y = dy * k;
        first->velocity.z = dz * k;
        rl_m003_03CAC(first);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_0322C);
#endif

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

/*
 * The troop rides its way point's object: each member not unk0_4 takes its place on it and moves with
 * it; once the first member is near the troop's way point, they get off
 */
void func_m003_03628(void) {
    s32 n = sHomeData->unk0;
    Object *obj = sHomeData->first;
    s32 index = (u16) sHomeData->unk3C >> 8;
    M003Node *node;
    Object *ride;
    M003Member *m;

    if (index == 0) {
        func_m003_033B0(sObj);
        index = (u16) sHomeData->unk3C >> 8;
    }
    node = &D_800FEA9C[index];
    if (index == 0) {
        return;
    }
    if (node->obj == NULL) {
        ride = (Object *) func_m003_07BAC(node->x, node->y, node->z, node->kind, node->unk32, node->unk33);
        if (ride != NULL) {
            node->obj = ride;
        }
    }
    if (node->obj == NULL) {
        return;
    }
    ride = node->obj;
    sHomeData->leader = node->obj;
    while (n--) {
        m = obj->unk68;
        if (!m->unk0_4) {
            m->unk0_9 = TRUE;
            if (node->cmd == 4 && func_m003_04260(2, 2, node->unk19, node->unk1A, node->unk1B)) {
                m->unk4A = node->unk1C;
            }
            func_m003_03470(obj);
            obj->velocity.x = ride->velocity.x;
            obj->velocity.y = ride->velocity.y;
            obj->velocity.z = ride->velocity.z;
            rl_m003_03CAC(obj);
        }
        obj = ((M003Member *) obj->unk68)->next;
    }
    node = &D_800FEA9C[sHomeData->unk3C & 0xFF];
    if (func_m003_03E20(sHomeData->first->trans.x_position, sHomeData->first->trans.z_position, node->x, node->z) <=
        node->unk19) {
        sHomeData->unk33 = 0;
        n = sHomeData->unk0;
        obj = sHomeData->first;
        while (n--) {
            m = obj->unk68;
            m->unk0_9 = FALSE;
            if (m->mission == 2) {
                func_m003_04398(2, 0);
                m->unk4B = 0;
            }
            obj = ((M003Member *) obj->unk68)->next;
        }
    }
}


/*
 * Puts the troop's first member back on the way point it came from (the high byte of unk3C), then heads it
 * for the next one (the low byte) if that is still out of reach
 */
#ifdef NON_MATCHING
/* 7 diffs, float registers only: the ROM's multiplier looks like a literal (CSE'd, f14), which needs m003's .rodata in C */
void func_m003_038D4(void) {
    s32 prev;
    M003Home *h;
    Object *first;
    M003Node *node;
    f32 k;
    f32 dx;
    f32 dy;
    f32 dz;

    h = sHomeData;
    prev = (u16) h->unk3C >> 8;
    if (prev != 0) {
        first = h->first;
        node = &D_800FEA9C[prev];
        func_m003_03E20(first->trans.x_position, first->trans.z_position, node->x, node->z);
        first->trans.x_position = node->x;
        first->trans.y_position = node->y;
        first->trans.z_position = node->z;
        first->velocity.x = 0.0f;
        first->velocity.y = 0.0f;
        first->velocity.z = 0.0f;
        rl_m003_03CAC(first);
        node = &D_800FEA9C[sHomeData->unk3C & 0xFF];
        if (func_m003_03E20(first->trans.x_position, first->trans.z_position, node->x, node->z) <= node->unk19) {
            sHomeData->unk33 = 0;
        }
        if (func_m003_03E20(first->trans.x_position, first->trans.z_position, node->x, node->z) < node->unk19) {
            k = 0.1f;
            sHomeData->unk33 = 30;
            sHomeData->unk3E = 10;
            dx = node->x - first->trans.x_position;
            dy = node->y - first->trans.y_position;
            dz = node->z - first->trans.z_position;
            first->velocity.x = dx * k;
            first->velocity.y = dy * k;
            first->velocity.z = dz * k;
            rl_m003_03CAC(first);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_038D4);
#endif

extern s32 func_m154_00A10(u8 node); /* the way point after `node` */
#define NODEWORD14(n) (*(u32 *) (n)->pad14) /* the way point's word at 0x14 (bit 1: wait for the player) */

#ifdef NON_MATCHING
/* 21 diffs: only temp register numbers (the ROM uses one more temp from the start) */
/*
 * Once the troop's first member is near enough its way point: on, unless the point says to wait for
 * a player who is not within 700
 */
void func_m003_03AA8(void) {
    Object *first;
    UNUSED s32 pad[3];
    f32 dist;
    s32 n;
    M003Node *node;
    s32 next;

    if (func_m154_01278()) {
        n = sHomeData->unk3C & 0xFF;
        node = &D_800FEA9C[n];
        first = sHomeData->first;
        if (NODEWORD14(node) & 2) {
            sHomeData->unk74 = n;
        }
        if (sHomeData->unk6C == 1.0f) {
            dist = point_distance(first->trans.x_position, first->trans.y_position, first->trans.z_position,
                                 node->x, node->y, node->z);
            if (dist < rl_m003_02760(node) && !rl_m003_01C58(node, 1)) {
                if (point_distance(first->trans.x_position, first->trans.y_position, first->trans.z_position,
                                  ((Object *) sClosestPlayer)->trans.x_position,
                                  ((Object *) sClosestPlayer)->trans.y_position,
                                  ((Object *) sClosestPlayer)->trans.z_position) < 700.0f ||
                    !(NODEWORD14(node) & 2)) {
                    next = func_m154_00A10(sHomeData->unk3C & 0xFF);
                    if (next != (sHomeData->unk3C & 0xFF)) {
                        func_80051880(next);
                        sMember->unk0_21 = TRUE;
                        return;
                    }
                } else {
                    sMember->unk0_21 = FALSE;
                }
                rl_m003_01C58(node, 0);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03AA8);
#endif

void func_m003_03C84(void) {
    func_m155_02BB0(sHome);
}

/* Keeps the member's heading, climb and speed from its velocity */
void func_m003_03CAC(Object *obj) {
    M003Member *m = obj->unk68;
    f32 flat;

    flat = obj->velocity.x * obj->velocity.x + obj->velocity.z * obj->velocity.z;
    m->unk1C = func_80049C84(obj->velocity.x, obj->velocity.z);
    m->unk1E = func_80049C84(obj->velocity.y, func_80097140(flat));
    m->unk20 = func_80097140(obj->velocity.y * obj->velocity.y + flat);
}

/* Pushes the member `speed` the way `angle` faces and keeps its heading and speed */
void func_m003_03D44(s16 angle, f32 speed) {
    sObj->velocity.x -= func_80049C34(angle) * speed;
    sObj->velocity.z -= func_80049C30(angle) * speed;
    sMember->unk1C = func_80049C84(sObj->velocity.x, sObj->velocity.z);
    sMember->unk20 = func_80097140(sObj->velocity.x * sObj->velocity.x + sObj->velocity.z * sObj->velocity.z);
}

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

/* Stretches the vector (x, y, z) to length `len` if it is shorter (and not zero) */
void func_m003_040FC(s16 *x, s16 *y, s16 *z, f32 len) {
    f32 cur = func_80097140(*x * *x + *y * *y + *z * *z);
    f32 nx;
    f32 ny;
    f32 nz;

    if (cur != 0.0f && cur < len) {
        nx = *x / cur * len;
        ny = *y / cur * len;
        nz = *z / cur * len;
        *x = nx;
        *y = ny;
        *z = nz;
    }
}

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


/* The current member's animation speed: its first animation entry's unk0, scaled */
static f32 func_m003_044B4(void) {
    return func_m003_04428(sObj)->unk0 * 0.0001f;
}

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

/*
 * Starts animation `index` of the member's body (asked for as kind `anim`); with `adjust`, some bodies
 * swap in other entries for kinds 0x21, 0x25, 0x16 and 0x19. Returns the index started, or -1.
 */
static s32 func_m003_04968(s32 index, s32 anim, s32 adjust) {
    M003Anim *a;
    s32 r;

    if (sBodyIndex == 0x13) {
        return index;
    }
    if (adjust) {
        if ((anim == 0x21 || anim == 0x25) &&
            (sBodyIndex == 1 || sBodyIndex == 5 || sBodyIndex == 0xD || sBodyIndex == 0xE || sBodyIndex == 0xF ||
             sBodyIndex == 0x16)) {
            if (((M003Interact *) sObj->interactObj)->unk8_29 || ((M003Interact *) sObj->interactObj)->part == 0) {
                index = 8;
            } else {
                index = (((M003Interact *) sObj->interactObj)->part - 1) << 1;
            }
        } else if ((anim == 0x16 || anim == 0x19) &&
                   (sBodyIndex == 1 || sBodyIndex == 5 || sBodyIndex == 0xD || sBodyIndex == 0x16)) {
            if (((M003Interact *) sObj->interactObj)->unk8_29) {
                index = 0x17;
            } else {
                index = 0x1A;
            }
        } else if (sBodyIndex == 6) {
            if (anim == 0x21) {
                index = 2;
                if ((sHomeData->unk34 & 3) == 2 ||
                    ((sHomeData->unk34 & 3) == 3 && !((M003Interact *) sObj->interactObj)->unk8_29)) {
                    index = 0;
                }
            } else if (anim == 0x16 || anim == 0x19) {
                if (((M003Interact *) sObj->interactObj)->unk8_29) {
                    index = 0xF;
                } else {
                    index = 0x12;
                }
            }
        }
    }
    if (anim == 1 && sHomeData->unk30 == 3) {
        r = rl_m003_04614(0x3B, 1);
        if (r != -1) {
            index = r;
        }
    }
    a = &rl_m003_17DB8[rl_m003_19278[sBodyIndex * 2] + index];
    sMember->unk56 = a->unk4_11;
    if (sBodyIndex == 0x20 && sMember->unkAC != NULL) {
        func_800079B8(sMember->unkAC);
        sMember->unkAC = NULL;
    }
    sObj->particleEmittersEnabled = 0;
    if (a->unk4_22 == 3) {
        rl_m003_044F4(sObj);
    }
    if (!(sHomeData->flags & 0x8000) && a->unk4_22 == 4 && func_80048E2C(1, 3) == 1) {
        rl_m003_044F4(sObj);
    }
    if (index == -1) {
        return -1;
    }
    obj_anim_set(sObj, index, 0.0f);
    sMember->unk60 = 0.0f;
    sMember->animSet = index;
    return index;
}

/* The body's word at 0x24 as bit fields */
typedef struct M003Body24 {
    u32 pad6 : 26;
    u32 unk5 : 1; /* animations 0x16 to 0x1F do not follow each other (04D08) */
    u32 pad0 : 5;
} M003Body24;
#define BODY24(b) ((M003Body24 *) &(b)->unk24)

#ifdef NON_MATCHING
/* ~90 diffs: the ROM keeps the index in a3 and the array pointer as an induction variable */
/*
 * Starts the first of the member's animations anim1, anim2, anim3 (up to a 0) that its body has, for
 * animation kind `kind`; -1 if none can start now
 */
s32 func_m003_04D08(s32 kind, s32 anim1, s32 anim2, s32 anim3) {
    s32 anims[3];
    s32 i;
    s32 anim;
    s32 result;
    s32 last;
    s32 index;

    if (!func_m003_043FC(kind)) {
        return -1;
    }
    index = -1;
    anims[0] = anim3;
    anims[1] = anim2;
    anims[2] = anim1;
    i = 2;
    if (anim1 != 0) {
        do {
            anim = anims[i];
            if (BODY24(sBody)->unk5 && anim >= 0x16 && anim < 0x20 && sMember->unkA >= 0x16 &&
                sMember->unkA < 0x20) {
                return -1;
            }
            if (anim == sMember->unkA) {
                return -1;
            }
            if (sMember->unkA == 0x21 || sMember->unkA == 0x25) {
                return -1;
            }
            index = rl_m003_04614(anim, 0);
            last = anim;
        } while (i-- != 0 && index == -1 && anims[i] != 0);
    }
    result = func_m003_04968(index, last, 1);
    if (result != -1) {
        sMember->unk4B = kind;
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04D08);
#endif

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
    result = func_m003_04968(index, anim, 0);
    if (result != -1) {
        sMember->unk4B = arg0;
    }
    return result;
}

/* Moves the member's animation of kind `kind` on; returns what the animation step returns, or 0 */
s32 func_m003_04F88(s32 kind, f32 speed) {
    s32 result = 0;
    M003Anim *anims;
    Object **flash;

    if (kind == sMember->unk4B) {
        anims = func_m003_04428(sObj);
        sMember->unk60 = sObj->unk28;
        result = func_8001138C(sObj, func_m003_044B4() * speed, sUpdateRate);
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
                    func_m003_04968(sMember->animSet, 0x35, 1);
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
        func_m003_04968(index, anim, 1);
    }
    sMember->unk4B = kind;
    return 0;
}

/* What a player keeps in obj->unk68 (only what is used here) */
typedef struct M003Player {
    /* 0x000 */ u8 pad0[0x10D];
    /* 0x10D */ u8 riding;
    /* 0x10E */ u8 pad10E[0x50C - 0x10E];
    /* 0x50C */ Object *vehicle;
} M003Player;

extern Object **func_8000E464(s32 *count); /* the players */

/* The player (or the player's vehicle) nearest (x, y, z) */
static s32 func_m003_0527C(f32 x, f32 y, f32 z) {
    f32 best = 5.0e9f;
    Object *nearest = NULL;
    Object **players;
    s32 count;
    Object *obj;
    M003Player *data;
    f32 d;

    players = func_8000E464(&count);
    while (count--) {
        obj = *players++;
        data = obj->unk68;
        d = point_distance_squared(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, x, y, z);
        if (d < best) {
            best = d;
            nearest = obj;
        }
        if (data->riding) {
            obj = data->vehicle;
            d = point_distance_squared(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, x, y,
                                       z);
            if (d < best) {
                best = d;
                nearest = obj;
            }
        }
    }
    return (s32) nearest;
}

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
    sClosestPlayer = func_m003_0527C(sObj->trans.x_position, sObj->trans.y_position, sObj->trans.z_position);
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
        sClosestPlayer = func_m003_0527C(sObj->trans.x_position, sObj->trans.y_position, sObj->trans.z_position);
    } else {
        sMember = NULL;
        sShotBy = 0;
        sClosestPlayer = func_m003_0527C(sHome->trans.x_position, sHome->trans.y_position, sHome->trans.z_position);
    }
    troop_spawn_context_set(sObj, sHome);
    func_m154_017E8(sHome);
}

#ifdef NON_MATCHING
/* 11 diffs, registers only: the ROM keeps `point` in v0 and computes `point + 1` again for the test */
/* The points before and after `point` round the region's edge */
void func_m003_05650(M003Region *region, u8 point, u8 *prev, u8 *next) {
    *prev = point - 1;
    *next = point + 1;
    if (point == 0) {
        *prev = region->numEdges - 1;
        return;
    }
    if (region->numEdges == point + 1) {
        *next = 0;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_05650);
void rl_m003_05650(M003Region *region, u8 point, u8 *prev, u8 *next); /* static: called by its module offset */
#define func_m003_05650 rl_m003_05650
#endif

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

/*
 * The first path group no troop home uses, or -1. (The search starts from the count left by the loop,
 * -1, as the original does.) Called from 01EB8.
 */
s32 func_m003_061FC(void) {
    s32 count;
    Object **homes;
    s32 used;

    homes = func_80050F44(&count);
    used = 0;
    while (count--) {
        used |= 1 << ((M003Home *) homes[count]->unk68)->group;
    }
    for (; count < 32; count++) {
        if (!((1 << count) & used)) {
            return count;
        }
    }
    return -1;
}

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

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_06D28);


/* The change from `speed` towards a speed for going from `from` to `to`, at most `max` either way */
f32 func_m003_07980(f32 from, f32 to, f32 speed, f32 max) {
    f32 d = (to - from) * 0.2f - speed;

    if (max < d) {
        d = max;
    }
    if (d < -max) {
        d = -max;
    }
    return d;
}

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
            return obj_find_nearest_in_category(x, y, z, 8);
        case 2:
            return troop_find_member(arg4 + 1, arg5);
    }
    return 0;
}

/*
 * The elevation to throw at `speed` under gravity `g` from (x, y, z) to 50 below (tx, ty, tz): of the two
 * solutions the higher if `high`, else the lower; 0x2000 if the target is out of reach
 */
s16 func_m003_07C18(f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz, f32 speed, f32 g, s32 high) {
    f32 dist;
    f32 dy;
    f32 b;
    f32 dy2;
    f32 root;
    f32 disc;
    f32 denom;
    f32 s;
    s32 i;
    s32 n;
    s16 angles[2];

    n = 0;
    i = 2;
    dist = func_80097140((x - tx) * (x - tx) + (z - tz) * (z - tz));
    dy = ty - y - 50.0f;
    b = dy * g + speed * speed;
    dy2 = dy * dy;
    disc = b * b - g * g * (dist * dist + dy2);
    if (g * g * (dist * dist + dy2) <= b * b) {
        root = func_80097140(disc);
        denom = dy2 / (dist * dist) + 1.0f;
        denom += denom;
        while (i--) {
            /* x: the square of the horizontal speed */
            if ((x = ((i ? root : -root) + b) / denom) >= 0.0f) {
                s = func_80097140(x);
                if (dist < 0.0f) {
                    s = -s;
                }
                angles[n++] = func_80049C84(dy / dist * s - g * dist / (s + s), s);
            }
        }
    }
    switch (n) {
        case 2:
            if (angles[0] < angles[1]) {
                return high ? angles[1] : angles[0];
            }
            return high ? angles[0] : angles[1];
        case 1:
            return angles[0];
    }
    return 0x2000;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_07E88);

/* The member's word at 0x4 as four 3-bit part numbers (7: none) */
typedef struct M003Parts {
    u32 part0 : 3;
    u32 part1 : 3;
    u32 part2 : 3;
    u32 part3 : 3;
    u32 rest : 20;
} M003Parts;

/* A row of rl_m003_17A10 (0x18 bytes) */
typedef struct M003PartRow {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 a[4];
    /* 0x14 */ u8 b[4];
} M003PartRow;

/*
 * Runs 07E88 for each of the member's four parts, from its animation's row of rl_m003_17A10; the
 * first time the troop has flag 0x80, with 40 and the member marked (called from 10600)
 */
void func_m003_09680(void) {
    M003Anim *anim;
    s32 i;
    s32 arg;
    s32 part;

    anim = func_m003_04428(sObj);
    arg = 0;
    i = 4;
    if (!sMember->unk0_20 && (sHomeData->flags & 0x80)) {
        arg = 40;
        sMember->unk0_20 = 1;
    }
    if (anim->parts != 0x7F) {
        M003PartRow *row = &((M003PartRow *) rl_m003_17A10)[anim->parts];

        while (i--) {
            switch (i) {
                case 0:
                    part = ((M003Parts *) &sMember->unk4)->part0;
                    break;
                case 1:
                    part = ((M003Parts *) &sMember->unk4)->part1;
                    break;
                case 2:
                    part = ((M003Parts *) &sMember->unk4)->part2;
                    break;
                case 3:
                    part = ((M003Parts *) &sMember->unk4)->part3;
                    break;
            }
            if (part != 7) {
                func_m003_07E88(row->a[part], row->b[part], arg);
            }
        }
    }
}

#ifdef NON_MATCHING
/* 2 diffs: the ROM masks `flag` to 16 bits in the branch's delay slot (with this prototype IDO masks
   it through its stack slot; with none it does not mask it at all) */
extern void obj_group_countdown(s32 id, u16 flag);

/* Puts the troop in state 7 and, once, counts it down in its object group */
void func_m003_09840(M003Home *home, s32 flag) {
    u8 group = home->unk75;

    home->unk30 = 7;
    if (group != 0 && !(home->flags & 8)) {
        obj_group_countdown(group, flag);
        home->flags |= 8;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09840);
#endif

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

/* The level entry of a lost body part (object 0x89) */
typedef struct M003PartEntry2 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ s8 unk2;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ Object *owner;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 kind;
} M003PartEntry2;

extern void velocity_mass_weighted(Object *a, Object *b, f32 *x, f32 *y, f32 *z);
extern void func_m008_001CC(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, s32 alpha, s32 arg7, f32 scale);

/*
 * The member coming apart: 32 drops around it, then every body part it has not lost yet flies off (while
 * the main pool has over 48 KiB free) with the hit's velocity (called from 10600)
 */
#ifdef NON_MATCHING
/* Two instructions: k is a .rodata literal in the ROM (hoisted after the loop's 1.0f); needs .rodata in C */
void func_m003_09CE4(Object *obj) {
    M003Member *m;
    s32 alpha;
    s32 i;
    s32 dx;
    s32 dy;
    M003PartEntry2 entry;
    UNUSED s32 pad;
    f32 x;
    f32 y;
    f32 z;
    s32 dz;
    s32 vx;
    s32 vy;
    f32 k;
    Object *o;
    M003BodyPart *p;
    u16 bit;

    m = obj->unk68;
    velocity_mass_weighted(obj, ((M003Interact *) obj->interactObj)->hitBy, &x, &y, &z);
    if (m->unk55 == 0) {
        if (obj->shading != NULL) {
            alpha = obj->shading->unk0 * 255.0f;
        } else {
            alpha = 0xFF;
        }
        k = 0.01f;
        i = 31;
        do {
            dx = func_80048E2C(-20, 20);
            dy = func_80048E2C(-20, 20);
            dz = func_80048E2C(-20, 20);
            vx = func_80048E2C(-500, 500);
            vy = func_80048E2C(0, 500);
            func_m008_001CC(dx + obj->trans.x_position, dy + obj->trans.y_position, dz + obj->trans.z_position,
                            vx * k, vy * k, func_80048E2C(-500, 500) * k, alpha, ((u8 *) obj->header)[0x2F],
                            1.0f);
        } while (i--);
        for (i = 0; i < sBody->numParts; i++) {
            p = &sBody->parts[i];
            bit = 1 << p->bit;
            if (!(m->lostParts & bit)) {
                m->lostParts |= bit;
                if (gMainPoolFreeBytes > 0xC000) {
                    entry.objectID = 0x89;
                    entry.unk2 = 20;
                    entry.unk3 = 0;
                    entry.x = 0;
                    entry.y = 0;
                    entry.z = 0;
                    entry.unk10 = p->unk4;
                    entry.unk11 = p->unk5;
                    entry.owner = sObj;
                    entry.kind = p->kind;
                    o = func_80005F14(&entry, 1);
                    if (o != NULL) {
                        o->velocity.x += x;
                        o->velocity.y += y;
                        o->velocity.z += z;
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_09CE4);
#endif

/* A member is killed */
void func_m003_09FF0(Object *obj) {
    M003Interact *interact;

    if (sHomeData->unk0 == 1 && sHomeData->arrived == sHomeData->count) {
        obj_spawn_bonus(obj->trans.x_position, obj->trans.y_position + 10.0f, obj->trans.z_position, sHomeData->unk7B);
    }
    if (sHomeData->kind == 0x2CE) {
        interact = (M003Interact *) sObj->interactObj;
        spawn_object_from_list(0x10, interact->pos[0], interact->pos[1], interact->pos[2], 100, 0);
    }
    func_m003_09A14(obj);
}

/* The data of the object that calls 0A110 */
typedef struct M003Alarm {
    /* 0x00 */ u8 kind;   /* 2: kill the members; 3, 4: send them on missions 11 and 2 */
    /* 0x01 */ u8 troop;  /* the home's unk64 it acts on; 0xFF: every troop */
    /* 0x02 */ u8 pad2;
    /* 0x03 */ u8 range;  /* in 8s */
    /* 0x04 */ u8 height; /* in 4s */
    /* 0x05 */ u8 pad5[3];
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
} M003Alarm;

/* Acts on the members of every troop within the object's range */
void func_m003_0A110(Object *obj, UNUSED s32 arg1) {
    M003Alarm *alarm;
    s32 count;
    Object **homes;
    M003Home *h;
    Object *home;
    Object *o;
    M003Member *m;
    f32 dist;
    f32 dy;

    alarm = obj->unk68;
    homes = func_80050F44(&count);
    while (count--) {
        home = homes[count];
        h = home->unk68;
        if (alarm->troop == 0xFF || alarm->troop == h->unk64) {
            switch (alarm->kind) {
                case 3:
                    for (o = h->first; o != NULL; o = m->next) {
                        m = o->unk68;
                        dist = func_m003_03E20(obj->trans.x_position, obj->trans.z_position, o->trans.x_position,
                                               o->trans.z_position);
                        dy = obj->trans.y_position - o->trans.y_position;
                        if (dy < 0.0f) {
                            dy = -dy;
                        }
                        if (dist < alarm->range * 8 && dy < alarm->height * 4 && func_m003_05450(o) &&
                            !sMember->unk0_26) {
                            if (sMember->unk1A != 13 || sMember->unk38 != obj->trans.x_position ||
                                sMember->unk3C != obj->trans.y_position || sMember->unk40 != obj->trans.z_position) {
                                if (func_m003_04260(11, 13, 0, 0, 0)) {
                                    sMember->unk38 = obj->trans.x_position;
                                    sMember->unk3C = obj->trans.y_position;
                                    sMember->unk40 = obj->trans.z_position;
                                }
                            }
                        }
                    }
                    break;
                case 4:
                    for (o = h->first; o != NULL; o = m->next) {
                        m = o->unk68;
                        dist = func_m003_03E20(obj->trans.x_position, obj->trans.z_position, o->trans.x_position,
                                               o->trans.z_position);
                        dy = obj->trans.y_position - o->trans.y_position;
                        if (dy < 0.0f) {
                            dy = -dy;
                        }
                        if (dist < alarm->range * 8 && dy < alarm->height * 4 && func_m003_05450(o) &&
                            sMember->unk1A != 19) {
                            func_m003_04260(2, 19, alarm->unk8, alarm->unk9, alarm->unkA);
                        }
                    }
                    break;
                case 2:
                    for (o = h->first; o != NULL; o = m->next) {
                        m = o->unk68;
                        dist = func_m003_03E20(obj->trans.x_position, obj->trans.z_position, o->trans.x_position,
                                               o->trans.z_position);
                        dy = obj->trans.y_position - o->trans.y_position;
                        if (dy < 0.0f) {
                            dy = -dy;
                        }
                        if (dist < alarm->range * 8 && dy < alarm->height * 4 && func_m003_05450(o)) {
                            func_m003_09A14(o);
                        }
                    }
                    break;
            }
        }
    }
}

/* Turns the member towards sBssD6, at most 1000 a step */
void func_m003_0A564(void) {
    s16 turn = func_80049D80(sObj->trans.rotation.y_rotation, sBssD6);

    if (turn < -1000) {
        turn = -1000;
    } else if (turn > 1000) {
        turn = 1000;
    }
    sObj->trans.rotation.y_rotation += turn;
}

/*
 * Turns the member towards sBssD6, speeding its turn up or down by the body's turnStep so that it stops
 * facing that way
 */
void func_m003_0A5EC(void) {
    s16 diff;
    UNUSED s32 pad;
    s16 b;
    s16 a;
    s16 step;

    diff = func_80049D80(sObj->trans.rotation.y_rotation, sBssD6);
    a = 0;
    b = 0;
    step = sBody->turnStep;
    if (func_m003_04428(sObj)->unk4_25 && sBssCC == 0.0f) {
        sMember->unk4C = 0;
        sBssD6 = sObj->trans.rotation.y_rotation;
        return;
    }
    if (diff >= 0) {
        do {
            a += step;
            b += a;
        } while (b + a < diff);
    } else {
        do {
            a -= step;
            b += a;
        } while (diff < b + a);
    }
    if (step >= diff && diff >= -step && step >= sMember->unk4C && sMember->unk4C >= -step &&
        ((diff >= 0 && sMember->unk4C >= 0) || (diff <= 0 && sMember->unk4C <= 0))) {
        sMember->unk4C = sBssD6 - sObj->trans.rotation.y_rotation;
    } else if (a - step >= sMember->unk4C + step) {
        sMember->unk4C += step;
    } else if (sMember->unk4C - step < a) {
        sMember->unk4C = a - step;
    } else {
        sMember->unk4C -= step;
    }
    if (sMember->unk4C == 0) {
        if (step < diff) {
            sMember->unk4C = step;
        } else if (diff < -step) {
            sMember->unk4C = -step;
        }
    }
    sObj->trans.rotation.y_rotation += sMember->unk4C;
}


/* The member's step speed: the smaller of sBssCC squared and the troop's speed, each scaled; doubled
   while it is busy, five times while unk0_17 or on fire; 0 in state 20 */
void func_m003_0A850(void) {
    f32 near;
    f32 speed;

    if (sMember->unkA == 20) {
        sBss10 = 0.0f;
        return;
    }
    near = sBssCC * sBssCC * 0.0001f;
    speed = sHomeData->speed * 0.1f;
    if (speed < near) {
        sBss10 = speed;
    } else {
        sBss10 = near;
    }
    if (sMember->busy) {
        sBss10 *= 2.0f;
        return;
    }
    if (sMember->unk0_17 || sMember->onFire) {
        sBss10 *= 5.0f;
    }
}

f32 func_m003_079D8(s32 dist, f32 speed, f32 accel);

/* The member's next speed: 079D8's change for stopping sBssCC away, accelerating by the troop's speed */
void func_m003_0A948(void) {
    sBss10 = func_m003_079D8(sBssCC * 10.0f, sMember->unk20 * 10.0f, sHomeData->speed) * 0.1f;
}


/* Changes the member's climb towards `height` above sBss18 */
void func_m003_0A9E8(s32 height) {
    sObj->velocity.y +=
        func_m003_079D8((height + sBss18 - sObj->trans.y_position) * 10.0f, sObj->velocity.y * 10.0f,
                        sHomeData->speed) *
        0.1f;
}

/* Scales the member's movement by `x` sideways and `z` forwards (in its own frame) */
void func_m003_0AABC(f32 x, f32 z) {
    s16 rel = sMember->unk1C - sObj->trans.rotation.y_rotation;
    f32 dx = func_80049C34(rel) * sMember->unk20 * x;
    f32 dz = func_80049C30(rel) * sMember->unk20 * z;
    s16 angle = func_80049C84(dx, dz);
    f32 d = dx * dx + dz * dz;

    sMember->unk1C = angle + sObj->trans.rotation.y_rotation;
    sMember->unk20 = func_80097140(d);
    sObj->velocity.x = func_80049C34(sMember->unk1C) * sMember->unk20;
    sObj->velocity.z = func_80049C30(sMember->unk1C) * sMember->unk20;
}

/* Slows the member the more it faces away from sBssD4 (full speed within 0x1000, none past 0x3000) and
   sets its velocity from its heading */
void func_m003_0ABF4(void) {
    s16 turn = func_80049D80(sObj->trans.rotation.y_rotation, sBssD4);
    s32 away;
    s16 over;
    f32 f;

    if (turn < 0) {
        away = -turn;
    } else {
        away = turn;
    }
    over = away - 0x1000;
    if (over < 0) {
        over = 0;
    }
    f = (0x2000 - over) * (1.0f / 8192);
    if (f > 1.0f) {
        f = 1.0f;
    }
    if (f < 0.0f) {
        f = 0.0f;
    }
    sMember->unk20 *= f;
    sObj->velocity.x = func_80049C34(sMember->unk1C) * sMember->unk20;
    sObj->velocity.z = func_80049C30(sMember->unk1C) * sMember->unk20;
}


/* Keeps the member's speed within its troop's limits (scaling it first while the body's distance is
   above 0 or sBssCC above 20) and sets its velocity from its heading */
void func_m003_0AD38(void) {
    f32 max = sHomeData->maxSpeed;
    f32 min = sHomeData->minSpeed;

    if (sBody->unk10 > 0.0f || sBssCC > 20.0f) {
        sMember->unk20 *= 0.98f;
    }
    if (max < sMember->unk20) {
        sMember->unk20 = max;
    }
    if (sMember->unk20 < min) {
        sMember->unk20 = min;
    }
    sObj->velocity.x = func_80049C34(sMember->unk1C) * sMember->unk20;
    sObj->velocity.z = func_80049C30(sMember->unk1C) * sMember->unk20;
}

/* Scales the member's flat velocity by `rate` and keeps its heading and speed */
void func_m003_0AE80(f32 rate) {
    sObj->velocity.x *= rate;
    sObj->velocity.z *= rate;
    sMember->unk1C = func_80049C84(sObj->velocity.x, sObj->velocity.z);
    sMember->unk20 = func_80097140(sObj->velocity.x * sObj->velocity.x + sObj->velocity.z * sObj->velocity.z);
}

extern void rl_m003_03D44(s16 angle, f32 speed); /* still assembly */
extern void rl_m003_0AE80(f32 rate);             /* still assembly */

/* A member step: 0AE80 while the body's distance (0 for the troop's first member) is over sBssCC, else 03D44
   the way the member faces at sBss10 */
void func_m003_0AF34(void) {
    if (sBssCC < ((sObj == sHomeData->first) ? 0.0f : sBody->unk10)) {
        rl_m003_0AE80(0.8f);
    } else {
        rl_m003_03D44(sObj->trans.rotation.y_rotation, sBss10);
    }
}

/* A member step: 03D44 the way sBssD4 at sBss10 while the body's distance is under sBssCC */
void func_m003_0AFC4(void) {
    if (((sObj == sHomeData->first) ? 0.0f : sBody->unk10) < sBssCC) {
        rl_m003_03D44(sBssD4, sBss10);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0B040);

extern f32 rl_m003_07980(f32 from, f32 to, f32 speed, f32 max);

/* Changes the member's climb towards sBss18 (twice as fast while it is busy) */
void func_m003_0B468(void) {
    f32 max = sHomeData->speed * 0.2f;

    if (sMember->busy) {
        max *= 2.0f;
    }
    sObj->velocity.y += rl_m003_07980(sObj->trans.y_position, sBss18, sObj->velocity.y, max);
}

/*
 * The height the member walks at: in states 2, 0x13 and 0xC, between the troop's way point and the one
 * before, by how far along the member is; else the troop's height (called from 0B70C)
 */
static f32 func_m003_0B50C(void) {
    M003Node *b;
    M003Node *a;
    s32 i;
    f32 dx;
    f32 dz;
    f32 ox;
    f32 oz;
    f32 dist;
    f32 c;
    f32 t;
    s32 angle;
    s16 n;

    if (sHomeData->unk32 == 2 || sHomeData->unk32 == 0x13 || sHomeData->unk32 == 0xC) {
        n = sHomeData->unk3C;
        if (n & 0xFF) {
            i = n & 0xFF;
            if (D_800FEA9C[i].cmd == 8 || (sMember->unk1A == 0x13 && sMember->unk1B == 4)) {
                return D_800FEA9C[i].y;
            }
            if ((n & 0xFFFF) >> 8) {
                b = &D_800FEA9C[(n & 0xFFFF) >> 8];
            } else {
                b = NULL;
            }
            a = &D_800FEA9C[i];
            if (b == NULL) {
                return a->y;
            }
            if (b->y == a->y) {
                return a->y;
            }
            dx = b->x - a->x;
            dz = b->z - a->z;
            ox = sObj->trans.x_position - b->x;
            oz = sObj->trans.z_position - b->z;
            dist = func_80097140(dx * dx + dz * dz);
            angle = func_80049C84(ox, oz);
            c = func_80049C30(angle - func_80049C84(dx, dz));
            t = func_80097140(ox * ox + oz * oz) * c;
            return -(t / ((t + dist) - t)) * (a->y - b->y) + b->y;
        }
        return sHomeData->y;
    }
    return sHomeData->y;
}


/*
 * The member's fall: towards the walking height 0B50C gives (while it rises, only slowing), landing on it
 * (called from 0BBC4 and 10600)
 */
void func_m003_0B70C(void) {
    f32 h;

    if (!(sHomeData->flags & 0x30) && !sMember->unk0_18 && ((M003BodyFlags *) &sBody->unk24)->unk10) {
        h = func_m003_0B50C();
        if (sMember->unk0_11 && sObj->velocity.y > 0.0f) {
            sObj->velocity.y -= 1.0f;
            return;
        }
        if (h < sObj->trans.y_position) {
            sObj->velocity.y -= 1.0f;
            sMember->unk0_26 = 1;
        } else {
            sObj->velocity.y = 0.0f;
            sMember->unk1E = 0;
            sMember->unk0_26 = 0;
            sMember->unk0_11 = 0;
            if (sMember->unk1A == 0x13 && sMember->unk1B == 4) {
                sMember->unk4B = 0;
                func_m003_04398(2, 0);
            }
        }
        if (sObj->trans.y_position + sObj->velocity.y < h) {
            if (h - 12.0f < sObj->trans.y_position + sObj->velocity.y) {
                sObj->velocity.y = 0.0f;
                sMember->unk1E = 0;
            } else {
                sObj->velocity.y *= -0.3f;
            }
            sObj->trans.y_position = h;
            sMember->unk0_26 = 0;
            sMember->unk0_11 = 0;
            if (sMember->unk1A == 0x13 && sMember->unk1B == 4) {
                sMember->unk4B = 0;
                func_m003_04398(2, 0);
            }
        }
    }
}

/* The member steps of 0B9A0 and its kin (still assembly) */
extern void rl_m003_0A564(void);
extern void rl_m003_0A5EC(void);
extern void rl_m003_0A850(void);
extern void rl_m003_0A948(void);
extern void rl_m003_0A9E8(s32 arg0);
extern void rl_m003_0AD38(void);
extern void rl_m003_0AFC4(void);
extern void rl_m003_0B468(void);

/* One of the member updates that 10600 picks from */
void func_m003_0B9A0(void) {
    rl_m003_0A564();
    rl_m003_0A850();
    rl_m003_0AFC4();
    rl_m003_0B468();
    rl_m003_0AD38();
}

/* As 0B9A0, but starting with 0A5EC */
void func_m003_0B9E0(void) {
    rl_m003_0A5EC();
    rl_m003_0A850();
    rl_m003_0AFC4();
    rl_m003_0B468();
    rl_m003_0AD38();
}

extern void rl_m003_0AABC(f32 a, f32 b); /* still assembly */
extern void rl_m003_0AF34(void);

/* Another member update: walking at five times the troop's speed */
void func_m003_0BA20(void) {
    rl_m003_0A564();
    sBss10 = sHomeData->speed * 5;
    rl_m003_0AF34();
    rl_m003_0B468();
    rl_m003_0AABC(0.0f, 1.0f);
    rl_m003_0AD38();
}

/* Another of the member updates that 10600 picks from */
void func_m003_0BA94(void) {
    rl_m003_0A564();
    rl_m003_0A948();
    rl_m003_0AFC4();
    rl_m003_0A9E8(0);
    rl_m003_0AD38();
}

/* Another member update: walking at the troop's speed */
void func_m003_0BAD4(void) {
    rl_m003_0A5EC();
    sBss10 = sHomeData->speed;
    rl_m003_0AF34();
    rl_m003_0A9E8(0);
    rl_m003_0AABC(0.0f, 1.0f);
    rl_m003_0AD38();
}

/* Another member update; a falling member's fall is scaled */
void func_m003_0BB50(void) {
    rl_m003_0A5EC();
    rl_m003_0A948();
    rl_m003_0AFC4();
    rl_m003_0A9E8(0);
    if (sObj->velocity.y < 0.0f) {
        sObj->velocity.y *= 0.73f;
    }
    rl_m003_0AD38();
}

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_0BBC4);

/* As 0BB50, with another factor */
void func_m003_0BFC4(void) {
    rl_m003_0A5EC();
    rl_m003_0A948();
    rl_m003_0AFC4();
    rl_m003_0A9E8(0);
    if (sObj->velocity.y < 0.0f) {
        sObj->velocity.y *= 0.73f;
    }
    rl_m003_0AD38();
}

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

/* An object header, as 0C1A0 uses it */
typedef struct M003TargetHeader {
    /* 0x00 */ u8 pad0[0x1E];
    /* 0x1E */ s8 useOrigin; /* aim at the object itself, not its interaction centre */
} M003TargetHeader;

extern f32 point_distance(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);

/* Where the member hears/aims at `target` from: the way to it, its distance and its point (each output
   is optional); FALSE with no target */
s32 func_m003_0C1A0(Object *obj, Object *target, s16 *yaw, s16 *pitch, f32 *dist, f32 *tx, f32 *ty,
                    f32 *tz) {
    M003Member *m = obj->unk68;
    f32 ex = m->earPos[0];
    f32 ey = m->earPos[1];
    f32 ez = m->earPos[2];
    f32 x;
    f32 y;
    f32 z;

    if (yaw != NULL) {
        *yaw = obj->trans.rotation.y_rotation;
    }
    if (pitch != NULL) {
        *pitch = 0;
    }
    if (dist != NULL) {
        *dist = 0.0f;
    }
    if (target == NULL) {
        return FALSE;
    }
    if (((M003TargetHeader *) target->header)->useOrigin) {
        x = target->trans.x_position;
        y = target->trans.y_position;
        z = target->trans.z_position;
    } else {
        x = ((M003Interact *) target->interactObj)->centre[0];
        y = ((M003Interact *) target->interactObj)->centre[1];
        z = ((M003Interact *) target->interactObj)->centre[2];
    }
    if (yaw != NULL) {
        *yaw = func_80049C84(ex - x, ez - z);
    }
    if (dist != NULL) {
        *dist = point_distance(ex, ey, ez, x, y, z);
    }
    if (tx != NULL) {
        *tx = x;
        *ty = y;
        *tz = z;
    }
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_0C324);

extern u8 D_800FE9EE;
extern s32 obj_id_is_special(s16 id);

/* Whether the current member wants to fire at its target, `dist` away (and turned at most `maxAngle` off) */
int func_m003_0CAA4(f32 dist, f32 arg1, s16 maxAngle) {
    Object *target = (Object *) sMember->shotBy;
    s16 angle;

    if (target == NULL) {
        return FALSE;
    }
    angle = func_80049D80(sObj->trans.rotation.y_rotation,
                          func_80049C84(sObj->trans.x_position - target->trans.x_position,
                                        sObj->trans.z_position - target->trans.z_position));
    if (angle < 0) {
        angle = -angle;
    }
    if (((M003BodyFlags *) &sBody->unk24)->unk11 && FLAGS4(sMember)->unk2 && !FLAGS8(sMember)->unk27) {
        if (!FLAGS4(sMember)->unk6 && !FLAGS4(sMember)->unk5) {
            return FALSE;
        }
    } else if (maxAngle < angle) {
        return FALSE;
    }
    if (!func_m003_0C324(sObj) || sHomeData->unk30 == 2 || sBssD0 <= dist ||
        (arg1 > 0.0f && arg1 < sBssD0) || sMember->unk4A || (sHomeData->flags & 0x1000) || sMember->onFire ||
        sMember->unk0_21 ||
        (sMember->unkB0 != NULL && !sMember->unkB0->unk2 &&
         !obj_id_is_special(((Object *) sMember->shotBy)->objectID)) ||
        sMember->unk0_29) {
        return FALSE;
    }
    return sHomeData->unk30 == 3 || D_800FE9EE == 7 || D_800FE9EE == 12 || D_800FE9EE == 9;
}

/* .data (still assembly; relocated against the .data start): the animation 0512C fires with, -1: none */
#define sFireAnim (*(s32 *) (rl_m003_17A10 + 0x1A24))

/*
 * A member's standard attack: aims at whoever shot it (or the closest player), and fires animation 5's
 * shot when 0CAA4 allows it (from farther away when one of the animation's parts fires)
 */
void func_m003_0CD34(Object *obj) {
    s32 anim;
    s32 i;
    f32 dist;
    f32 arg1;
    s32 found;
    UNUSED s32 pad[2];
    s32 shotBy;

    anim = rl_m003_04614(5, 1);
    found = FALSE;
    i = 4;
    if (!(sMember->unk4 & 1)) {
        shotBy = sMember->shotBy;
        sMember->shotBy = sClosestPlayer;
        func_m003_0C324(obj);
        if (sMember->shotBy == 0 ||
            (((Object *) sMember->shotBy)->behaviorId == 1 && !(sHomeData->unk91 & 1))) {
            sMember->shotBy = shotBy;
        }
    }
    if (anim != -1) {
        M003Anim *a = &rl_m003_17DB8[rl_m003_19278[sBodyIndex * 2] + anim];
        M003PartRow *row = &((M003PartRow *) rl_m003_17A10)[a->parts];

        while (i--) {
            if (row->b[i] == 1) {
                found = TRUE;
                break;
            }
        }
    }
    if (found) {
        dist = 275.0f;
        arg1 = 1200.0f;
    } else {
        dist = 0.0f;
        arg1 = dist;
    }
    if (func_m003_0CAA4(dist, arg1, 0x2000)) {
        if (anim == -1) {
            func_m003_04260(4, 0x11, 0, 0, 0);
            return;
        }
        if (sMember->unk1A != 5 && func_m003_04260(4, 5, 0, 0, 0)) {
            sFireAnim = anim;
            func_m003_0512C(6, 1.0f, 5);
            sFireAnim = -1;
        }
    }
}

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

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_0D360);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0D810);

/* A member's level entry */
typedef struct M003MemberEntry {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ s16 x; /* all three 0: no place given */
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 yaw;   /* the way it is thrown, in 256ths of a turn */
    /* 0x0B */ u8 pitch; /* and how steeply */
    /* 0x0C */ u8 speed; /* and how fast */
    /* 0x0D */ u8 rotY;  /* its rotation, in 256ths of a turn */
    /* 0x0E */ u8 rotX;
    /* 0x0F */ u8 rotZ;
    /* 0x10 */ u8 model;
    /* 0x11 */ u8 troop; /* its troop's tag, 0: the nearest troop's */
    /* 0x12 */ s16 unk12; /* (member unk38-40) */
    /* 0x14 */ s16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ u8 unk18;
} M003MemberEntry;

extern Object *obj_find_nearest_of_type(f32 x, f32 y, f32 z, u8 type);
extern void rl_m003_0D360(Object *home); /* still assembly */

/*
 * A member is set up from its level entry: its troop, place, throw and rotation (an old-style definition:
 * 1477C calls it with nothing)
 */
void func_m003_0DF84(obj, entry)
    Object *obj;
    M003MemberEntry *entry;
{
    f32 c;

    sMember = (sObj = obj)->unk68;
    if (entry->x != 0 || entry->y != 0 || entry->z != 0) {
        sMember->placed = TRUE;
        if (entry->troop != 0) {
            sHome = (Object *) troop_find_by_tag((s16) entry->troop);
            sMember->unk38 = entry->unk12;
            sMember->unk3C = entry->unk14;
            sMember->unk40 = entry->unk16;
        } else {
            sHome = obj_find_nearest_of_type(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position,
                                             0x17);
            if (sHome != NULL) {
                sHomeData = sHome->unk68;
                sMember->troop = sHomeData->unk64;
                entry->troop = sHomeData->unk64;
            }
        }
        if (sHome != NULL) {
            sMember->home = sHome;
            sHomeData = sHome->unk68;
            sObj->modelIndex = entry->model;
        }
    } else {
        sMember->placed = FALSE;
    }
    if (sHome != NULL && sHomeData->unk33 == 0x1F && !sMember->unk0_4) {
        sObj->velocity.x = 0.0f;
        sObj->velocity.y = 0.0f;
        sObj->velocity.z = 0.0f;
        func_m003_03470(sObj);
    } else {
        sObj->trans.x_position = entry->x;
        sObj->trans.y_position = entry->y;
        sObj->trans.z_position = entry->z;
        c = func_80049C30(entry->pitch << 8);
        sObj->velocity.x = func_80049C30(entry->yaw << 8) * c * (u32) entry->speed;
        sObj->velocity.y = func_80049C34(entry->pitch << 8) * (u32) entry->speed;
        c = func_80049C30(entry->pitch << 8);
        sObj->velocity.z = func_80049C34(entry->yaw << 8) * c * (u32) entry->speed;
        rl_m003_03CAC(sObj);
        sObj->trans.rotation.y_rotation = entry->rotY << 8;
        sObj->trans.rotation.x_rotation = entry->rotX << 8;
        sObj->trans.rotation.z_rotation = entry->rotZ << 8;
    }
    if (sMember->placed) {
        sMember->unk57 = entry->unk18;
    } else {
        sMember->unk57 = sHomeData->unk8C;
        sHomeData->unk8C++;
    }
    rl_m003_0D360(sHome);
}

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_0E3CC);

extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern void func_m149_01D20(Object *obj, f32 updateRate);
extern void rl_m003_0E3CC(void); /* still assembly */
extern void rl_m003_03CAC(Object *obj);

/* Moves the member by its velocity (module 149 moves troop 0x67's; 0E3CC those that walk on the level),
   and the object that follows it with it */
void func_m003_0E9F0(Object *obj) {
    M003Member *m = sMember;

    if (m->unk0_9 && !m->unk0_4) {
        return;
    }
    if (sHomeData->kind == 0x67) {
        func_m149_01D20(obj, sUpdateRate);
        return;
    }
    if ((sHomeData->flags & 0x30) || m->unk0_18) {
        rl_m003_0E3CC();
        rl_m003_03CAC(obj);
    } else {
        m->lastX = obj->trans.x_position;
        sMember->lastY = obj->trans.y_position;
        sMember->lastZ = obj->trans.z_position;
        func_80009A04(obj, obj->velocity.x, obj->velocity.y, obj->velocity.z);
    }
    if (sMember->follower != NULL) {
        sMember->follower->trans.x_position = obj->trans.x_position;
        sMember->follower->trans.y_position = obj->trans.y_position;
        sMember->follower->trans.z_position = obj->trans.z_position;
        sMember->follower->segmentID = obj->segmentID;
    }
}

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

/*
 * Picks the member's parts for its animation: those of its animation's row whose frame lies between the
 * object's frame and the same distance past it (or 0 at frame 0), in order, the rest 7 (none). Called
 * from 10600.
 */
void func_m003_10160(void) {
    M003Anim *anim;
    f32 frame;
    f32 end;
    s32 n;
    s32 i;
    f32 *row;

    anim = func_m003_04428(sObj);
    frame = sObj->unk28;
    end = sObj->unk28 + (frame - sMember->unk60);
    ((M003Parts *) &sMember->unk4)->part0 = 7;
    ((M003Parts *) &sMember->unk4)->part1 = 7;
    n = 0;
    ((M003Parts *) &sMember->unk4)->part2 = 7;
    ((M003Parts *) &sMember->unk4)->part3 = 7;
    if (anim->parts != 0x7F) {
        row = (f32 *) &((M003PartRow *) rl_m003_17A10)[anim->parts];
        i = 3;
        do {
            if ((sObj->unk28 <= row[i] && row[i] <= end) || (sMember->unk60 == 0.0f && row[i] == 0.0f)) {
                sMember->partsSet = TRUE;
                switch (n++) {
                    case 0:
                        ((M003Parts *) &sMember->unk4)->part0 = i;
                        break;
                    case 1:
                        ((M003Parts *) &sMember->unk4)->part1 = i;
                        break;
                    case 2:
                        ((M003Parts *) &sMember->unk4)->part2 = i;
                        break;
                    case 3:
                        ((M003Parts *) &sMember->unk4)->part3 = i;
                        break;
                }
            }
        } while (i--);
    }
}

extern void obj_spawn_particle(Object *obj, s32 updateRate);

/*
 * The member's particles: while it burns (or in state 0x33) from a random point of its hit shape
 * instead of its position (called from 10600)
 */
void func_m003_10354(void) {
    f32 x;
    f32 y;
    f32 z;
    f32 r;
    s16 angle;
    f32 dy;

    if (sMember->onFire || sMember->unkA == 0x33) {
        x = sObj->trans.x_position;
        y = sObj->trans.y_position;
        z = sObj->trans.z_position;
        r = func_80048E2C(0, ((M003Interact *) sObj->interactObj)->radius);
        angle = func_80048E2C(-0x8000, 0x7FFF);
        sObj->trans.x_position += func_80049C34(angle) * r;
        if (sMember->unkA == 0x33) {
            dy = 0.0f;
        } else {
            dy = func_80048E2C(0, 2.0f * ((M003Interact *) sObj->interactObj)->height);
        }
        sObj->trans.y_position += dy;
        sObj->trans.z_position += func_80049C30(angle) * r;
        obj_spawn_particle(sObj, D_800FE9F8);
        sObj->trans.x_position = x;
        sObj->trans.y_position = y;
        sObj->trans.z_position = z;
    } else {
        obj_spawn_particle(sObj, D_800FE9F8);
    }
}

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

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_10600);

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_1398C);

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_13D94);

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_13FE4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_142F0);

/* What troop_ring_take gives, as far as 14694 uses it */
typedef struct M003RingEntry {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ u8 flag; /* a game flag: set means the member is not spawned */
} M003RingEntry;

extern M003RingEntry *troop_ring_take(s32 id);
extern Object *rl_m003_0D810(void); /* still assembly */

/* Spawns the troop's next member from the ring (or 0D810's way); counts the ones whose flag is set */
Object *func_m003_14694(void) {
    M003RingEntry *entry = troop_ring_take(sHomeData->unk64);
    Object *obj;

    if (entry != NULL) {
        if (objflag_get(entry->flag)) {
            sHomeData->arrived++;
            return NULL;
        }
        obj = func_80005F14(entry, 1);
        func_m003_0DF84(obj, entry);
        return obj;
    }
    if (objflag_get(sHomeData->unk8C)) {
        sHomeData->arrived++;
        sHomeData->unk8C++;
        return NULL;
    }
    return rl_m003_0D810();
}

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

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", func_m003_14D4C);


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
            data->weight[i] = entry[0xC + i];
            data->path[i] = a[i];
            data->node[i] = b[i];
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

/* Where the line through (x1, z1) and (x2, z2) meets the line sCrossSlope/sCrossOffset */
void func_m003_170D0(f32 x1, f32 z1, f32 x2, f32 z2, f32 *x, f32 *z) {
    f32 slope;
    f32 offset;

    if (x1 == x2) {
        *x = x1;
        *z = sCrossSlope * x1 + sCrossOffset;
        return;
    }
    slope = (z2 - z1) / (x2 - x1);
    offset = z1 - slope * x1;
    *x = (sCrossOffset - offset) / (slope - sCrossSlope);
    *z = *x * slope + offset;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_17174);

extern void rl_m003_17174(f32 x1, f32 z1, f32 x2, f32 z2, void *node);

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
        func_m003_05650(to, out->point, &out->unk5, &out->unk6);
        if (point != -1) {
            p = &from->points[point];
            if (sCrossX1 == p->x && sCrossZ1 == p->z &&
                !func_m003_16F44(((sCrossX - sCrossX1) >> 1) + sCrossX1, ((sCrossZ - sCrossZ1) >> 1) + sCrossZ1,
                                 region)) {
                out->x = p->x;
                out->z = p->z;
                out->point = point;
                out->region = regionIndex;
                func_m003_05650(from, point, &out->unk5, &out->unk6);
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

INCLUDE_ASM("asm/nonmatchings/modules/m003/ro", rodata);


