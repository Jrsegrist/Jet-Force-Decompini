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
    /* 0x04C */ u8 pad4C[0x64 - 0x4C];
    /* 0x064 */ f32 unk64; // DKR: unkD0
    /* 0x068 */ u8 pad68[0x140 - 0x68];
    /* 0x140 */ s16 y_rotation_offset;
    /* 0x142 */ s16 z_rotation_offset;
    /* 0x144 */ s16 x_rotation_offset;
    /* 0x146 */ s16 unk146; // DKR: unk166
    /* 0x148 */ u8 pad148[0x197 - 0x148];
    /* 0x197 */ u8 unk197;
    /* 0x198 */ u8 pad198[0x5C0 - 0x198];
    /* 0x5C0 */ s32 unk5C0;
} Object_Racer;

typedef struct ObjectTransform {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
} ObjectTransform;

typedef struct ObjectUnk5C {
    /* 0x000 */ u8 pad0[0x134];
    /* 0x134 */ s32 unk134;
} ObjectUnk5C;

typedef struct Object {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x2E - 0x18];
    /* 0x2E */ s16 segmentID;
    /* 0x30 */ u8 pad30[0x40 - 0x30];
    /* 0x40 */ struct ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ u8 pad4C[0x5C - 0x4C];
    /* 0x5C */ ObjectUnk5C *unk5C;
    /* 0x60 */ u8 pad60[0x68 - 0x60];
    /* 0x68 */ Object_Racer *racer;
    /* 0x6C */ struct ObjectAttachment **attachments; // JFG
    /* 0x70 */ u8 pad70[0x7C - 0x70];
    /* 0x7C */ u8 *unk7C;
    /* 0x80 */ u8 pad80[0xA0 - 0x80];
    /* 0xA0 */ u8 freed; // JFG: already on the free list
    /* 0xA1 */ u8 padA1[0xA4 - 0xA1];
    /* 0xA4 */ struct Object *prev;
    /* 0xA8 */ struct Object *next;
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

extern s8 D_800F2C60;
extern s32 D_800F2C6C;
extern s32 D_800F2C70;
extern s32 D_800F2CA8;
extern s32 D_800F2CAC;
extern s32 D_800F2CB8;
extern s32 D_800F2CBC;
extern s16 D_800F2CC0;
extern s16 D_800F2CC2;
extern s32 D_800F2CC4;
extern s32 D_800F2D08;
extern s32 D_800F2D1C; // JFG: the number of animated objects in D_800F2D18
extern s32 D_800F2D40;

/* JFG: reset the object system's variables. */
void func_800047C0(void) {
    D_800F2C60 = 1;
    D_800F2C6C = 0;
    D_800F2C70 = 0;
    D_800F2D08 = 0;
    gNumRacers = 0;
    D_800F2D1C = 0;
    D_800F2CA8 = 0;
    D_800F2CAC = 0;
    D_800F2CB8 = 0;
    D_800F2CBC = 0;
    D_800F2CC4 = 0;
    D_800F2CC0 = 0;
    D_800F2CC2 = 0;
    D_800F2D40 = 0;
}

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

/* JFG: a level object entry, reduced */
typedef struct LevelObjectEntry53 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3[0x11 - 3];
    /* 0x11 */ u8 unk11;
} LevelObjectEntry53;

extern s32 D_800F2CD8[]; // JFG: two lists of level object entries
extern s32 D_800F2CE0[]; // JFG: their sizes in bytes
void func_80005CB0(LevelObjectEntry53 *entry);

#ifdef NON_MATCHING
/**
 * JFG: take every entry of the object and kind out of both lists (the second first).
 * NON_MATCHING: 4 words: the two list pointers are set up, and *size / *list reloaded, in the other order.
 */
void func_80005BD0(s32 objectID, s32 kind) {
    s32 *list;
    s32 *size;
    s32 i;
    u8 *entry;
    u8 *end;

    list = &D_800F2CD8[1];
    size = &D_800F2CE0[1];
    i = 2;
    while (i--) {
        entry = (u8 *) *list;
        end = *size + entry;
        while (entry < end) {
            if (objectID == ((LevelObjectEntry53 *) entry)->objectID &&
                kind == ((LevelObjectEntry53 *) entry)->unk11) {
                func_80005CB0((LevelObjectEntry53 *) entry);
                end = *size + (u8 *) *list;
            } else {
                entry += ((LevelObjectEntry53 *) entry)->size;
            }
        }
        list--;
        size--;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_80005BD0);
#endif

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

/* JFG: the object header asset (only its size and one field are known) */
typedef struct ObjectHeaderAsset {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[0xC0 - 0x1E];
} ObjectHeaderAsset;

extern s16 *D_800F2CF8; // JFG: object type -> header index
extern s32 *D_800F2C74; // JFG: header offsets
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/* JFG: load an object type's header and return one of its fields. */
s16 func_80005EC0(s32 type) {
    UNUSED s32 pad;
    s32 offset;
    ObjectHeaderAsset header;

    offset = D_800F2C74[D_800F2CF8[type]];
    asset_load(0x2F, (u32) &header, offset, sizeof(ObjectHeaderAsset));
    return header.unk1C;
}

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

extern Object **gParticlePtrList;
extern s32 gFreeListCount;
extern s32 D_800F2F20[16];
extern u8 D_800A097C;
void func_80026E20(Object *obj);

/**
 * Adds the object to the free list.
 * This object will be deallocated on the next update cycle.
 * JFG: only once per object; keeps a ring of the last 16 freed objects' IDs.
 * Official Name: objFreeObject
 */
void free_object(Object *object) {
    if (object->freed == FALSE) {
        gParticlePtrList[gFreeListCount] = object;
        gFreeListCount++;
        object->freed = TRUE;
        func_80026E20(object);
        D_800F2F20[D_800A097C] = (object->behaviorId << 16) | object->unk4A;
        D_800A097C++;
        D_800A097C &= 0xF;
    }
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007A54);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007C68);

extern void **D_800F2D18; // JFG: the animated objects

/* JFG: take an object off the animated list (the last one takes its place). */
void func_80008F70(void *obj) {
    s32 i;

    if (D_800F2D1C > 0) {
        for (i = 0; i < D_800F2D1C; i++) {
            if (obj == D_800F2D18[i]) {
                D_800F2D18[i] = D_800F2D18[D_800F2D1C - 1];
            }
        }
        D_800F2D1C--;
    }
}

void func_80008FE8(void) {
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80008FF0);

INCLUDE_ASM("asm/nonmatchings/53C0", func_800095A4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009734);

/* JFG: the object header, reduced */
typedef struct ObjectHeader {
    /* 0x00 */ u8 pad0[0x1E];
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 numAttachments;
} ObjectHeader;

/* JFG: what an object's attachment list points at, reduced */
typedef struct ObjectAttachment {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s8 unkA;
} ObjectAttachment;

#ifdef NON_MATCHING
/**
 * JFG: set a byte of every attachment of the objects whose header has unk1E clear.
 * NON_MATCHING: 2 words: gObjectCount is loaded before gObjectListStart (target: the other order).
 */
void func_80009928(s32 value) {
    s32 i;
    s32 j;
    Object *obj;
    ObjectAttachment *attachment;

    for (i = gObjectListStart; i < gObjectCount; i++) {
        obj = gObjPtrList[i];
        if (obj->header->unk1E == 0) {
            for (j = 0; j < obj->header->numAttachments; j++) {
                attachment = obj->attachments[j];
                if (attachment != NULL) {
                    attachment->unkA = value;
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_80009928);
#endif

/**
 * Make the next call of move_object never mark the object as out of bounds.
 * Official Name: objMoveXYZnocheck
 */
void ignore_bounds_check(void) {
    gNoBoundsCheck = TRUE;
}

/* DKR include/structs.h (JFG: the bounds at 0x20) */
typedef struct LevelModelSegmentBoundingBox {
    /* 0x00 */ s16 x1;
    /* 0x02 */ s16 y1;
    /* 0x04 */ s16 z1;
    /* 0x06 */ s16 x2;
    /* 0x08 */ s16 y2;
    /* 0x0A */ s16 z2;
} LevelModelSegmentBoundingBox;

typedef struct LevelModel {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ s16 lowerXBounds;
    /* 0x22 */ s16 upperXBounds;
    /* 0x24 */ s16 lowerYBounds;
    /* 0x26 */ s16 upperYBounds;
    /* 0x28 */ s16 lowerZBounds;
    /* 0x2A */ s16 upperZBounds;
} LevelModel;

LevelModel *get_current_level_model(void);
LevelModelSegmentBoundingBox *block_boundbox(s32 segmentID);
s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos);
void func_800339D8(Object *obj, Object_Racer *racer); // JFG: restart a player
extern f32 D_800AA840; // 3000.0f, in .rodata (not split out yet)

/**
 * Sets the new position of the object using the differences given.
 * Compares against the outer edges of the level geometry to decide wether or not to apply a segment ID.
 * JFG: single-precision bounds (all 1000 but the top, 3000); some objects out of bounds
 * are freed, and a racer is restarted; the segment lookup takes floats.
 * Official Name: objMoveXYZ
 */
s32 move_object(Object *obj, f32 xPos, f32 yPos, f32 zPos) {
    s32 segmentID;
    f32 x1, x2, y1, y2, z1, z2;
    f32 newXPos;
    f32 newYPos;
    f32 newZPos;
    LevelModel *levelModel;
    LevelModelSegmentBoundingBox *box;
    s32 outsideBBox;
    s32 outOfBounds;
    s32 intXPos, intYPos, intZPos;

    levelModel = get_current_level_model();
    newXPos = obj->trans.x_position + xPos;
    newYPos = obj->trans.y_position + yPos;
    newZPos = obj->trans.z_position + zPos;
    if (levelModel == NULL) {
        gNoBoundsCheck = FALSE;
        return FALSE;
    }
    outOfBounds = FALSE;
    x2 = levelModel->upperXBounds + 1000.0f;
    //!@bug should've compared against "obj->trans.x_position"
    if (newXPos > x2) {
        outOfBounds = TRUE;
    }
    x1 = levelModel->lowerXBounds - 1000.0f;
    if (obj->trans.x_position < x1) {
        outOfBounds = TRUE;
    }
    y2 = levelModel->upperYBounds + D_800AA840;
    if (obj->trans.y_position > y2) {
        outOfBounds = TRUE;
    }
    y1 = levelModel->lowerYBounds - 1000.0f;
    if (obj->trans.y_position < y1) {
        outOfBounds = TRUE;
    }
    z2 = levelModel->upperZBounds + 1000.0f;
    if (obj->trans.z_position > z2) {
        outOfBounds = TRUE;
    }
    z1 = levelModel->lowerZBounds - 1000.0f;
    if (obj->trans.z_position < z1) {
        outOfBounds = TRUE;
    }
    if (1) {}
    if (1) {}
    if (1) {} // Fakematch (as in DKR)
    if (gNoBoundsCheck) {
        outOfBounds = FALSE;
    }

    gNoBoundsCheck = FALSE;
    if (outOfBounds) {
        if (obj->behaviorId == 0x99 || obj->behaviorId == 0x9B || obj->behaviorId == 0x3D ||
            obj->behaviorId == 0x18 || obj->behaviorId == 0xAB || obj->behaviorId == 0xAD ||
            obj->behaviorId == 0x97 || obj->behaviorId == 0x86 || obj->behaviorId == 0x59 ||
            obj->behaviorId == 0xA6 || obj->behaviorId == 0xA7) {
            free_object(obj);
            return TRUE;
        }
        if (obj->behaviorId == BHV_RACER) {
            func_800339D8(obj, obj->racer);
        }
        obj->segmentID = -1;
        return TRUE;
    }

    obj->trans.x_position = newXPos;
    obj->trans.y_position = newYPos;
    obj->trans.z_position = newZPos;
    box = block_boundbox(obj->segmentID);

    // For some reason the XYZ positions are converted into integers for the next section
    intXPos = newXPos, intYPos = newYPos, intZPos = newZPos;

    if (box == NULL) {
        obj->segmentID = get_level_segment_index_from_position(intXPos, intYPos, intZPos);
        return FALSE;
    } else {
        outsideBBox = FALSE;
        if (box->x2 < intXPos || intXPos < box->x1) {
            outsideBBox = TRUE;
        }
        if (box->y2 < intYPos || intYPos < box->y1) {
            outsideBBox = TRUE;
        }
        if (box->z2 < intZPos || intZPos < box->z1) {
            outsideBBox = TRUE;
        }
        if (outsideBBox) {
            segmentID = get_level_segment_index_from_position(intXPos, intYPos, intZPos);
            if (segmentID != -1) {
                obj->segmentID = segmentID;
            }
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009E58);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80009FB4);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000A154);

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000A300);

void func_8006D7F0(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, s32 colour); // JFG: draw a 3D line

/* JFG: draw a line from the point in the object's block at 0x68 to the object. */
void func_8000A9EC(Object *obj) {
    f32 *from = (f32 *) obj->racer;

    func_8006D7F0(from[1], from[2], from[3], obj->trans.x_position, obj->trans.y_position,
                  obj->trans.z_position, 0x13);
}

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

/* DKR include/structs.h */
typedef struct Vertex {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} Vertex;

extern Gfx *gObjectCurrDisplayList;
extern Mtx *gObjectCurrMatrix;
extern Vertex *gObjectCurrVertexList;
void func_8000DDAC(Object *obj);

/**
 * Set the draw pointers, then begin rendering the object.
 * JFG: no stack trace, and the scale is not saved.
 * Official Name: objPrintObject
 */
void render_object(Gfx **dList, Mtx **mtx, Vertex **verts, Object *obj) {
    if (obj->trans.flags & 0xC00) { // JFG's flag bits (DKR: OBJ_FLAGS_INVISIBLE | OBJ_FLAGS_SHADOW_ONLY)
        return;
    }
    gObjectCurrDisplayList = *dList;
    gObjectCurrMatrix = *mtx;
    gObjectCurrVertexList = *verts;
    func_8000DDAC(obj);
    *dList = gObjectCurrDisplayList;
    *mtx = gObjectCurrMatrix;
    *verts = gObjectCurrVertexList;
}

#ifdef NON_MATCHING
f32 func_80049C30(s32 angle); // cosine of a 16-bit angle

/**
 * JFG: no vehicle check.
 * Official Name: objDoPlayerTumble
 * NON_MATCHING: the right length, but the cosine is kept at sp+0x18 (target 0x1C) and the
 * ternary's 0.0f reuses the compare's register (target: a second mtc1 into $f0).
 */
void object_do_player_tumble(Object *this) {
    UNUSED s32 unused1;
    Object_Racer *sp_20;
    f32 tmp_f2;
    f32 offsetY;

    if (this->behaviorId == BHV_RACER) {
        sp_20 = this->racer;
        this->trans.y_rotation += sp_20->y_rotation_offset;
        this->trans.x_rotation += sp_20->x_rotation_offset;
        this->trans.z_rotation += sp_20->z_rotation_offset;
        offsetY = func_80049C30(sp_20->z_rotation_offset);
        tmp_f2 = func_80049C30(sp_20->x_rotation_offset - sp_20->unk146) * offsetY;
        offsetY = (1.0f - ((tmp_f2 < 0.0f) ? 0.0f : tmp_f2 * tmp_f2)) * 24.0f + sp_20->unk64;
        this->trans.y_position = this->trans.y_position + offsetY;
        gObjectOffsetY = offsetY;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DC28);
#endif

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

/* JFG: link obj into the list after its head, unless it is already in it. */
void func_8001191C(Object *obj, Object *list) {
    Object *it = list;

    if (list != NULL) {
        while (it != NULL) {
            if (it == obj) {
                return;
            }
            it = it->next;
        }
        obj->prev = list;
        obj->next = list->next;
        if (list->next != NULL) {
            list->next->prev = obj;
        }
        list->next = obj;
    }
}

void func_80011974(Object *obj) {
    if (obj->next != NULL) {
        obj->next->prev = obj->prev;
    }
    if (obj->prev != NULL) {
        obj->prev->next = obj->next;
    }
    obj->next = NULL;
    obj->prev = NULL;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_800119B8);

/* JFG: whether any racer has its unk5C0 set. */
s32 func_80011C2C(void) {
    s32 i;
    Object_Racer *racer;

    for (i = 0; i < gNumRacers; i++) {
        racer = (*gRacers)[i]->racer;
        if (racer->unk5C0 != 0) {
            return 1;
        }
    }
    return 0;
}

/* JFG: the distance between two points. */
f32 func_80011C84(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
    f32 dx = x1 - x2;
    f32 dy = y1 - y2;
    f32 dz = z1 - z2;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

/* JFG: the squared distance between two points. */
f32 func_80011CE0(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
    f32 dx = x1 - x2;
    f32 dy = y1 - y2;
    f32 dz = z1 - z2;
    return dx * dx + dy * dy + dz * dz;
}

extern s32 D_800F2D20[];
void func_80054530(); // a module function (patched in by the runtime linker)

#ifdef NON_MATCHING
/**
 * JFG: add an entry to the D_800F2D20 list; when full, the oldest goes (to a module function first).
 * NON_MATCHING: one extra lui: the target shares %hi of D_800F2D20 between two stores of the
 * unrolled shift (likely needs the list defined in this file).
 */
void func_80011D1C(s32 entry) {
    s32 i;

    if (D_800F2D40 >= 8) {
        func_80054530(D_800F2D20[0]);
        for (i = 0; i < 7; i++) {
            D_800F2D20[i] = D_800F2D20[i + 1];
        }
        D_800F2D40 = 7;
    }
    D_800F2D20[D_800F2D40] = entry;
    D_800F2D40++;
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_80011D1C);
#endif

/* JFG: remove an entry from the D_800F2D20 list (the later ones move down). */
void func_80011DEC(s32 entry) {
    s32 i;

    for (i = 0; i < D_800F2D40; i++) {
        if (entry == D_800F2D20[i]) {
            break;
        }
    }
    if (i < D_800F2D40) {
        for (; i < D_800F2D40 - 1; i++) {
            D_800F2D20[i] = D_800F2D20[i + 1];
        }
        D_800F2D40--;
    }
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011E74);
