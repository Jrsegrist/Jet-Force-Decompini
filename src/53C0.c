/*
 * The object core. Functions in C here are from the Diddy Kong Racing
 * decompilation (CC0 1.0), src/objects.c; see CREDITS.md.
 */
#include "common.h"

#define BHV_RACER 1

typedef struct Vec3s {
    s16 x, y, z;
} Vec3s;

/* DKR include/structs.h, reduced to the fields used here (JFG offsets) */
typedef struct Object_Racer {
    /* 0x000 */ u8 pad0[0x48];
    /* 0x048 */ f32 unk48;
    /* 0x04C */ u8 pad4C[0x140 - 0x4C];
    /* 0x140 */ s16 y_rotation_offset;
    /* 0x142 */ s16 z_rotation_offset;
    /* 0x144 */ s16 x_rotation_offset;
    /* 0x146 */ u8 pad146[0x197 - 0x146];
    /* 0x197 */ u8 unk197;
} Object_Racer;

typedef struct ObjectTransform {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ u8 pad6[0x10 - 0x06];
    /* 0x10 */ f32 y_position;
} ObjectTransform;

typedef struct ObjectUnk5C {
    /* 0x000 */ u8 pad0[0x134];
    /* 0x134 */ s32 unk134;
} ObjectUnk5C;

typedef struct Object {
    /* 0x00 */ ObjectTransform trans;
    /* 0x14 */ u8 pad14[0x48 - 0x14];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ u8 pad4A[0x5C - 0x4A];
    /* 0x5C */ ObjectUnk5C *unk5C;
    /* 0x60 */ u8 pad60[0x68 - 0x60];
    /* 0x68 */ Object_Racer *racer;
    /* 0x6C */ u8 pad6C[0x7C - 0x6C];
    /* 0x7C */ u8 *unk7C;
} Object;

extern f32 gObjectOffsetY;

extern Object **gObjPtrList;
extern Object *(*gRacers)[10]; // Official Name: playerlist
extern Object **gRacersByPort;
extern Vec3s gEnvmapPos[2];
extern s32 *gAssetsMiscSection;    // Official Name: Ftables
extern s32 *gAssetsMiscTable;      // Official Name: Findex
extern s32 gAssetsMiscTableLength; // Official Name: Fmax
extern s32 gNumRacers;
extern s32 gObjectCount;
extern s32 gObjectListStart;
extern s8 gNoBoundsCheck;

INCLUDE_ASM("asm/nonmatchings/53C0", func_800047C0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004838);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004954);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004AF4);

extern u8 D_800A0898;

void func_80004C64(u8 arg0) {
    D_800A0898 = arg0;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004C74);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004F04);

/**
 * Converts the passed value into an accurate countdown value based on the systems region.
 * Since PAL runs at 50Hz, it therefore will reduce the timer to 5/6 to match, keeping
 * it consistent with non PAL timers, running 60Hz.
 * Official Name: objTvTimes
 */
s32 normalise_time(s32 timer) {
    if (osTvType != OS_TV_PAL || timer < 0) {
        return timer;
    } else {
        return (timer * 5) / 6;
    }
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_800050A8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800053F8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000561C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80005BD0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80005CB0);

/**
 * Returns the object at the current offset by ID.
 * Official name: objGetObject
 */
Object *get_object(s32 index) {
    if (index < 0 || index >= gObjectCount) {
        // DKR: stubbed_printf("ObjList (Part) Overflow %d!!!\n"); (not in this ROM)
        return 0;
    }
    return gObjPtrList[index];
}

/**
 * Return the standard object list index and how many objects are in that list.
 */
Object **objGetObjList(s32 *arg0, s32 *cnt) {
    *arg0 = gObjectListStart;
    *cnt = gObjectCount;
    return gObjPtrList;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80005EC0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80005F14);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80006E1C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80006F2C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800072B4);

/* JFG: attach a block to the object, mark it, and return a size. */
s32 func_8000740C(Object *obj, u8 *arg1) {
    obj->unk7C = arg1;
    *arg1 = 2;
    return 0x28;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007420);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007528);

/* JFG: attach a block to the object and return a size. */
s32 func_80007730(Object *obj, ObjectUnk5C *arg1) {
    obj->unk5C = arg1;
    return 0x13C;
}

/* JFG: set a word of the object's block and return a size. */
s32 func_8000773C(Object *obj, s32 arg1) {
    ObjectUnk5C *block = obj->unk5C;
    block->unk134 = arg1;
    return 0x11C;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000774C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800079B8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007A54);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007C68);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80008F70);

void func_80008FE8(void) {
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80008FF0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800095A4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009734);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009928);

/**
 * Make the next call of move_object never mark the object as out of bounds.
 * Official Name: objMoveXYZnocheck
 */
void ignore_bounds_check(void) {
    gNoBoundsCheck = TRUE;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009A04);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009E58);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009FB4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000A154);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000A300);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000A9EC);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000AA34);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000B300);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000B46C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000BC28);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000BE5C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C0B8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C6E0);

extern u8 D_800A0C5C;
extern u8 D_800A0C60;
extern u8 D_800A0C64;
extern u8 D_800A0C68;

void func_8000C9B4(u8 red, u8 green, u8 blue, u8 alpha) {
    D_800A0C5C = red;
    D_800A0C60 = green;
    D_800A0C64 = blue;
    D_800A0C68 = alpha;
}

extern u8 D_800A0C6C;

void func_8000C9E8(s32 arg0) {
    D_800A0C6C = arg0;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C9F4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000CC64);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DB94);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DC28);

/**
 * Official Name: objUndoPlayerTumble
 */
void object_undo_player_tumble(Object *obj) {
    if (obj->behaviorId == BHV_RACER) {
        Object_Racer *racer = obj->racer;
        obj->trans.y_rotation -= racer->y_rotation_offset;
        obj->trans.x_rotation -= racer->x_rotation_offset;
        obj->trans.z_rotation -= racer->z_rotation_offset;
        obj->trans.y_position -= gObjectOffsetY;
    }
}

extern f32 D_800F2E98;

void func_8000DD78(Object *obj) {
    f32 scale = 1.0f;
    if (obj->behaviorId == BHV_RACER) {
        Object_Racer *racer = obj->racer;
        scale = racer->unk48;
        racer->unk197 = 30;
    }
    D_800F2E98 = scale;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DDAC);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DF00);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000E108);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000E278);

/**
 * Returns the group of racer objects.
 * Official Name: objGetPlayerlist
 */
Object **get_racer_objects(s32 *numRacers) {
    *numRacers = gNumRacers;
    return *gRacers;
}

extern s32 D_800F2D1C;
extern void **D_800F2D18;

void func_8000E480(void *obj) {
    D_800F2D18[D_800F2D1C] = obj;
    D_800F2D1C++;
}

extern s32 D_800F2D1C;
extern void **D_800F2D18;

void **func_8000E4B4(s32 *count) {
    *count = D_800F2D1C;
    return D_800F2D18;
}

/**
 * Returns the racer object specified by the player ID.
 * Official name: objGetPlayerNo
 */
Object *get_racer_object_by_port(s32 index) {
    if (gNumRacers == 0) {
        return NULL;
    }
    if (index < 0 || index >= gNumRacers) {
        return NULL;
    }
    return gRacersByPort[index];
}

/**
 * Take the normalised length of the position set by the perspective and set the world angle for the envmap.
 * Official name: setObjectViewNormal
 */
void update_envmap_position(f32 x, f32 y, f32 z) {
    f32 vecLength = sqrtf((x * x) + (y * y) + (z * z));
    f32 normalizedLength;
    if (vecLength != 0.0f) {
        normalizedLength = -8192.0f / vecLength;
        x *= normalizedLength;
        y *= normalizedLength;
        z *= normalizedLength;
    }
    gEnvmapPos[0].x = x;
    gEnvmapPos[0].y = y;
    gEnvmapPos[0].z = z;
}

/**
 * Returns a pointer to the asset in the misc. section. If index is out of range, then this
 * function just returns the pointer to gAssetsMiscSection.
 * Official name: objGetTable
 */
s32 *get_misc_asset(s32 index) {
    if (index < 0 || index >= gAssetsMiscTableLength) {
        return gAssetsMiscSection;
    }
    return (s32 *) &gAssetsMiscSection[gAssetsMiscTable[index]];
}

extern s16 D_800F2CC0;

void func_8000E674(s32 arg0) {
    D_800F2CC0 = arg0;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000E680);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000F028);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000F978);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000FBF0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800104C0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800110E4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8001138C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8001149C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800116BC);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011708);

extern s32 D_800F2CD8[];
extern s32 D_800F2CE0[];

void func_800118F4(s32 *a, s32 *b, s32 index) {
    *a = D_800F2CD8[index];
    *b = D_800F2CE0[index];
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8001191C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011974);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800119B8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011C2C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011C84);

/* JFG: the squared distance between two points. */
f32 func_80011CE0(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
    f32 dx = x1 - x2;
    f32 dy = y1 - y2;
    f32 dz = z1 - z2;
    return dx * dx + dy * dy + dz * dz;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011D1C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011DEC);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011E74);
