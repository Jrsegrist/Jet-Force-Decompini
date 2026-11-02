/*
 * Runlink module 3: the enemy troops' shared behaviour. It keeps the troop
 * member being processed (and its troop's home object) in module state, moves
 * members between way points (module 154's graph), keeps the level's regions,
 * and handles hits, deaths and missions. Most of it is still in assembly.
 */
#include "module.h"

/* What a troop member keeps in obj->unk68 */
typedef struct M003Member {
    /* 0x00 */ u32 pad0_0 : 28;
    /* 0x00 */ u32 headless : 1;
    /* 0x00 */ u32 pad0_1 : 3;
    /* 0x04 */ u8 pad4[0x7 - 0x4];
    /* 0x07 */ s8 group; /* < 0: not active */
    /* 0x08 */ u8 pad8[0xB - 0x8];
    /* 0x0B */ u8 mission;
    /* 0x0C */ u8 padC[0x1A - 0xC];
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B; /* < 5: heading for a way point */
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ Object *home;
    /* 0x28 */ Object *next; /* the next member of the troop */
    /* 0x2C */ u8 pad2C[0x4A - 0x2C];
    /* 0x4A */ u8 unk4A;
    /* 0x4B */ s8 unk4B;
    /* 0x4C */ u8 pad4C[0x50 - 0x4C];
    /* 0x50 */ u8 node;
    /* 0x51 */ u8 pad51[0x64 - 0x51];
    /* 0x64 */ s32 shotBy;
} M003Member;

/* What a troop's home object keeps in obj->unk68 */
typedef struct M003Home {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ u16 kind;
    /* 0x2E */ u8 pad2E[0x48 - 0x2E];
    /* 0x48 */ f32 x;
    /* 0x4C */ f32 y;
    /* 0x50 */ f32 z;
    /* 0x54 */ u8 pad54[0x58 - 0x54];
    /* 0x58 */ Object *first; /* the troop's first member */
} M003Home;

/* A way point (module 154's graph) */
typedef struct M003Node {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 padC[0x34 - 0xC];
} M003Node;

/* A region of the level (size 0x2C) */
typedef struct M003Region {
    /* 0x00 */ u8 pad0[0x2C];
} M003Region;

/* A member's body description (D_800A38E0, 0x80 bytes each) */
typedef struct M003Body {
    /* 0x00 */ u8 pad0[0x2D];
    /* 0x2D */ u8 headPart; /* 0x1F: none */
} M003Body;

/* What a member's interactObj points at */
typedef struct M003Interact {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u32 pad8_0 : 3;
    /* 0x08 */ u32 unk8_3 : 1;
    /* 0x08 */ u32 sound : 4;
    /* 0x08 */ u32 pad8_1 : 24;
    /* 0x0C */ u8 padC[0x31 - 0xC];
    /* 0x31 */ u8 part;
    /* 0x32 */ u8 pad32[0x3C - 0x32];
    /* 0x3C */ Object *hitBy;
} M003Interact;

extern M003Node *D_800FEA9C; /* the way points */
extern u8 *D_800A38E0;       /* the body descriptions */
extern s32 D_800FE9FC;
extern s32 D_800FE9B0;
extern f32 func_80097140(f32 x); /* sqrtf */
extern s16 func_80050F60(u16 kind);
extern s32 func_80004AF4(s32 arg0, s32 arg1, f32 arg2, s32 arg3);
extern s32 func_80051508(s16 arg0, u8 arg1);
extern void func_80047868(s32 sound);
extern void func_80052798(Object *obj, Object *home);
extern void func_m154_017E8(Object *home);
extern s32 rl_m003_0527C(f32 x, f32 y, f32 z);
extern void rl_m003_053D4(Object *obj);
extern void func_m003_0DF84(void);
extern void func_m003_15ED8(s32 arg0, s32 arg1);
extern void func_m003_16530(void);
extern void *func_80050F44(s32 *count);
extern void func_8004AA80(void *data); /* DKR mempool_free */
extern void *D_800FE9A4;
extern void *D_800FEA18[16];
extern s32 D_800FEA00;
extern s32 D_800FEA04;

/* The module's .data (still assembly) */
extern u8 rl_m003_17A10[];
#define sRegions (*(M003Region **) (rl_m003_17A10 + 0x1A1C))

/* The module's .bss (the assembly functions reach it by offset) */
static u8 sBss0[0x4];
static s32 sRegionCount;
static u8 sBss8[0x3];
static u8 sAlertKind;
static u8 sAlert;
static u8 sBss10[0xAC - 0x10]; /* (IDO aligns arrays to 4) */
static Object *sObj;            /* the member being processed */
static M003Member *sMember;     /* its data */
static Object *sHome;           /* its troop's home object */
static M003Home *sHomeData;     /* and that one's data */
static s16 sBodyIndex;
static M003Body *sBody;
static s32 sShotBy;
static s32 sClosestPlayer;
static u8 sBssD0[0xE00 - 0xD0]; /* (from 0xCC, IDO aligning it to 8) */

s32 func_m003_0D2C0(Object *obj);
void func_m003_10548(Object *obj, s32 shotBy);
void func_m003_0C0A0();

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

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_000D0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00258);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0027C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00328);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00384);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_009C8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00A5C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00AA0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00AF4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00C20);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00C70);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00D44);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_00FD0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01188);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_012E8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0136C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01608);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01764);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01974);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01A50);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01A68);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01ACC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01AF0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01B44);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01C58);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01D3C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_01EB8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_021C8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0237C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02760);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_027A4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_028D4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02A94);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02BBC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02CA8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02D58);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_02DA0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0301C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_030CC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0312C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03170);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0322C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0332C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_033B0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03470);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03628);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_038D4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03AA8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03C84);

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

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_03E90);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_040FC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04260);

/* Ends the current member's mission (1: whatever it is) */
void func_m003_04398(s32 mission, s32 arg1) {
    if (mission == 1 || mission == sMember->mission) {
        sMember->unk1A = 0;
        sMember->unk1B = 0;
        sMember->mission = 0;
        sMember->unk4A = arg1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_043FC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04428);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_044B4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_044F4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04614);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04968);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04D08);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04EC4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_04F88);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0512C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0527C);

s32 func_m003_053C4(void) {
    return sClosestPlayer;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_053D4);

#ifdef NON_MATCHING
/* Makes obj the member being processed; returns whether it is an active member with a body */
/* (temporary registers only: each one is numbered one lower than in the ROM) */
s32 func_m003_05450(Object *obj) {
    sMember = NULL;
    sHome = NULL;
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
    rl_m003_053D4(sObj);
    sShotBy = sMember->shotBy;
    func_80052798(sObj, sHome);
    func_m154_017E8(sHome);
    return func_m003_0D2C0(sObj) != 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_05450);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0556C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_05650);

#ifdef NON_MATCHING
/* the ROM keeps the table in v0 and the result in v1, adds index first and has two return paths */
M003Region *func_m003_0569C(s32 index) {
    M003Region *region = NULL;

    if (sRegions != NULL) {
        region = &sRegions[index];
    }
    return region;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0569C);
#endif

s32 func_m003_056D4(M003Region *region) {
    return region - sRegions;
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_056F4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_057CC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_058A4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0609C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_061FC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_062A0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_06754);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_069B0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_06D28);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07980);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_079D8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07B38);

#ifdef NON_MATCHING
/* the ROM keeps arg2 in its home slot and reloads it into a2; this goes through f12 */
s32 func_m003_07BAC(s32 arg0, s32 arg1, f32 arg2, s32 kind, u8 arg4, u8 arg5) {
    switch (kind) {
        case 1:
            return func_80004AF4(arg0, arg1, arg2, 8);
        case 2:
            return func_80051508(arg4 + 1, arg5);
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07BAC);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07C18);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_07E88);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09680);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09840);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_098A0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09A14);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09CE4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_09FF0);

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
        func_80047868(((M003Interact *) obj->interactObj)->sound);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0C0A0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0C1A0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0C324);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0CAA4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0CD34);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0CF40);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0CFA8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_0D134);

/* Looks up the body description of a member (behaviour 0x18) */
s32 func_m003_0D2C0(Object *obj) {
    Object *home = ((M003Member *) obj->unk68)->home;

    if (obj->behaviorId != 0x18 || home == NULL) {
        return 0;
    }
    sBodyIndex = func_80050F60(((M003Home *) home->unk68)->kind);
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

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_105B8);

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

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_147B8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_148C0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_14D4C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_15BE8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_15E34);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_15ED8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16530);

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

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_166AC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16750);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_167A0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_168D8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16990);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16C10);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16CB4);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16D78);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16EEC);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_16F44);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_170D0);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_17174);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_175E8);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_17910);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_1794C);

INCLUDE_ASM("asm/nonmatchings/modules/m003", func_m003_179B0);

/* Init */
void func_m003_179E0(void) {
    func_m003_15ED8(D_800FEA00, D_800FEA04);
}
#pragma weak __rl_init = func_m003_179E0

INCLUDE_ASM("asm/nonmatchings/modules/m003", data);

INCLUDE_ASM("asm/nonmatchings/modules/m003", rodata);


