/*
 * The object core. Functions in C here are from the Diddy Kong Racing
 * decompilation (CC0 1.0), src/objects.c; see CREDITS.md.
 */
#include "common.h"

typedef struct Object Object;

typedef struct Vec3s {
    s16 x, y, z;
} Vec3s;

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

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004C64);

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

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000740C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007420);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007528);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007730);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000773C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000774C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800079B8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007A54);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007C68);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80008F70);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80008FE8);

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

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C9B4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C9E8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C9F4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000CC64);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DB94);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DC28);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DD14);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DD78);

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

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000E480);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000E4B4);

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

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000E674);

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

INCLUDE_ASM("asm/nonmatchings/53C0", func_800118F4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8001191C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011974);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800119B8);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011C2C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011C84);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011CE0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011D1C);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011DEC);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011E74);
