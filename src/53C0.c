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
    /* 0x000 */ u8 pad0;
    /* 0x001 */ s8 unk1; // JFG: an index into D_800A0C2C (6 bytes each)
    /* 0x002 */ u8 pad2[0x3C - 0x2];
    /* 0x03C */ f32 prevX; // JFG: the position at the start of the frame
    /* 0x040 */ f32 prevY;
    /* 0x044 */ f32 prevZ;
    /* 0x048 */ f32 unk48;
    /* 0x04C */ u8 pad4C[0x64 - 0x4C];
    /* 0x064 */ f32 unk64; // DKR: unkD0
    /* 0x068 */ u8 pad68[0xA4 - 0x68];
    /* 0x0A4 */ u16 fogFlags; // JFG: a bit per player: draw with the flash fog colour
    /* 0x0A6 */ u8 padA6[0x122 - 0xA6];
    /* 0x122 */ s16 prevRotation[3]; // JFG: the angles at the start of the frame
    /* 0x128 */ u8 pad128[0x140 - 0x128];
    /* 0x140 */ s16 y_rotation_offset;
    /* 0x142 */ s16 z_rotation_offset;
    /* 0x144 */ s16 x_rotation_offset;
    /* 0x146 */ s16 unk146; // DKR: unk166
    /* 0x148 */ u8 pad148[0x197 - 0x148];
    /* 0x197 */ u8 unk197;
    /* 0x198 */ u8 pad198[0x1FA - 0x198];
    /* 0x1FA */ u8 unk1FA;
    /* 0x1FB */ u8 pad1FB[0x53C - 0x1FB];
    /* 0x53C */ s8 unk53C;
    /* 0x53D */ u8 pad53D[0x544 - 0x53D];
    /* 0x544 */ u8 unk544[0x550 - 0x544];
    /* 0x550 */ s32 unk550;
    /* 0x554 */ u8 pad554[0x598 - 0x554];
    /* 0x598 */ s32 effects[4];  // JFG: four effect meshes drawn at model points (0: none)
    /* 0x5A8 */ u8 pad5A8[0x5AC - 0x5A8];
    /* 0x5AC */ u8 effectAlpha[4]; // JFG: 0: off
    /* 0x5B0 */ s8 effectPoint[4]; // JFG: the model point each is drawn at
    /* 0x5B4 */ s8 effectMtx[4];   // JFG: the joint matrix each is drawn with
    /* 0x5B8 */ u8 pad5B8[0x5C0 - 0x5B8];
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

/* DKR include/structs.h */
typedef struct ObjHeaderParticleEntry {
    /* 0x00 */ s32 upper;
    /* 0x04 */ s32 lower;
} ObjHeaderParticleEntry;

/* JFG: the object header, reduced */
typedef struct ObjectHeader {
    /* 0x00 */ f32 scale;
    /* 0x04 */ u8 pad4[0x14 - 0x4];
    /* 0x14 */ u16 flags;
    /* 0x16 */ u8 pad16[0x1B - 0x16];
    /* 0x1B */ u8 category; // JFG: obj_find_nearest_in_category searches by it
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 numAttachments;
    /* 0x20 */ s8 numChildren;    // JFG: child objects (types in unk34)
    /* 0x21 */ s8 numSpriteSlots; // JFG: sprite slots (definitions in unk3C)
    /* 0x22 */ s8 particleCount;
    /* 0x23 */ u8 pad23[0x30 - 0x23];
    /* 0x30 */ s32 unk30; // JFG: offsets in the asset, made into pointers when loaded
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ struct ObjHeaderParticleEntry *objectParticles;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ f32 unk54;
    /* 0x58 */ u8 pad58[0x61 - 0x58];
    /* 0x61 */ u8 unk61;
    /* 0x62 */ u8 unk62;
    /* 0x63 */ u8 unk63;
    /* 0x64 */ s16 unk64;
    /* 0x66 */ u16 unk66;
    /* 0x68 */ u8 pad68[0xA2 - 0x68];
    /* 0xA2 */ u8 scrollTexture;
    /* 0xA3 */ u8 scrollModel; // 0xFF: all
    /* 0xA4 */ s8 scrollU;
    /* 0xA5 */ s8 scrollV;
    /* 0xA6 */ u8 numberOfModelIds;
    /* 0xA7 */ u8 padA7;
    /* 0xA8 */ u16 *modelIds;
    /* 0xAC */ void **assets; // JFG: the loaded model, texture or sprite of each ID
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ u8 padB4[0xD0 - 0xB4];
    /* 0xD0 */ s32 unkD0; // 0: none
    /* 0xD4 */ void *unkD4;
    /* 0xD8 */ s16 unkD8; // -1: none
} ObjectHeader;

/* JFG: what an object's attachment list points at, reduced */
typedef struct ObjectAttachment {
    /* 0x00 */ struct ObjectModel53 *model;
    /* 0x04 */ u8 *vertices;
    /* 0x08 */ s8 unk8; // nonzero: the animation loops
    /* 0x09 */ s8 prevUnk8;
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ u8 *matrices[5]; // JFG: 64-byte joint matrices, one buffer per matrixIndex
    /* 0x24 */ u16 animIndex;
    /* 0x26 */ u16 prevAnimIndex;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 prevUnk28;
    /* 0x30 */ f32 unk30;
    /* 0x34 */ f32 prevUnk30;
    /* 0x38 */ f32 animLength;
    /* 0x3C */ f32 prevAnimLength;
    /* 0x40 */ f32 unk40;
    /* 0x44 */ f32 prevUnk40;
    /* 0x48 */ u8 pad48[0x5C - 0x48];
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 unk5E;
    /* 0x60 */ u8 *animData;
    /* 0x64 */ u8 *prevAnimData;
    /* 0x68 */ f32 *point; // JFG: a point (x, y, z) of the model
    /* 0x6C */ u8 pad6C[0x78 - 0x6C];
    /* 0x78 */ s16 *texOffsets[2];
    /* 0x80 */ void *unk80;
    /* 0x84 */ void *unk84;
    /* 0x88 */ s16 unk88;
    /* 0x8A */ s16 unk8A;
} ObjectAttachment;

typedef struct Object {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x20 - 0x18];
    /* 0x20 */ f32 unk20;
    /* 0x24 */ u8 pad24[0x28 - 0x24];
    /* 0x28 */ f32 animFrame;
    /* 0x2C */ s16 headerIndex; // JFG: the loaded header's index (try_free_object_header)
    /* 0x2E */ s16 segmentID;
    /* 0x30 */ f32 distanceToCamera; // JFG: negated view depth (sort_objects_by_dist)
    /* 0x34 */ u8 pad34[0x39 - 0x34];
    /* 0x39 */ u8 opacity;
    /* 0x3A */ s8 attachmentIndex;
    /* 0x3B */ s8 animIndex;
    /* 0x3C */ s32 levelEntry; // JFG: the level object entry it was spawned from

    /* 0x40 */ struct ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ struct ObjectPoints *unk4C;
    /* 0x50 */ struct ObjectSprite *sprite;
    /* 0x54 */ void *shade; // JFG: the lighting the object is drawn with
    /* 0x58 */ void *unk58; // JFG: a ripple it makes (func_8006B090)
    /* 0x5C */ ObjectUnk5C *unk5C;
    /* 0x60 */ struct ObjectUnk60 *unk60;
    /* 0x64 */ struct ObjectSpriteSlot *spriteSlots;
    /* 0x68 */ Object_Racer *racer;
    /* 0x6C */ struct ObjectAttachment **attachments; // JFG
    /* 0x70 */ struct ParticleEmitter *particleEmitter;
    /* 0x74 */ u8 pad74[0x7C - 0x74];
    /* 0x7C */ u8 *unk7C;
    /* 0x80 */ u8 pad80[0x90 - 0x80];
    /* 0x90 */ u8 numSpriteSlots;
    /* 0x91 */ u8 pad91[0xA0 - 0x91];
    /* 0xA0 */ u8 freed; // JFG: already on the free list
    /* 0xA1 */ u8 padA1[0xA4 - 0xA1];
    /* 0xA4 */ struct Object *prev;
    /* 0xA8 */ struct Object *next;
    /* 0xAC */ f32 prevAnimFrame;
    /* 0xB0 */ f32 animSpeed;
} Object;

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

/* DKR include/structs.h (Size 0x10) */
typedef struct Triangle {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 vi0;
    /* 0x02 */ u8 vi1;
    /* 0x03 */ u8 vi2;
    /* 0x04 */ s16 uv[3][2];
} Triangle;

extern Gfx *gObjectCurrDisplayList;
extern Mtx *gObjectCurrMatrix;
extern Vertex *gObjectCurrVertexList;

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

/* JFG: reset the object system's variables (0x800047C0). */
void clear_object_pointers(void) {
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

extern s32 D_800F2D14;
extern Object **D_800F2CB4; // JFG: a second object list (D_800F2CB8 entries)
extern void *D_800F2CF0[2];
void gParticlePtrList_flush(void);
void func_80007C68(Object *obj, s32 arg1);
void mempool_free(void *data);

/* JFG: free every object (and the type 0x18 ones of the second list), then reset the object system. */
void free_all_objects(void) {
    s32 i;
    s32 count;
    Object *obj;

    D_800F2D14 = 0;
    gParticlePtrList_flush();
    count = gObjectCount;
    for (i = 0; i < count; i++) {
        func_80007C68(gObjPtrList[i], 1);
    }
    count = D_800F2CB8;
    for (i = 0; i < count; i++) {
        obj = D_800F2CB4[i];
        if (obj->behaviorId == 0x18) {
            func_80007C68(obj, 1);
        }
    }
    D_800F2D08 = 0;
    gObjectCount = 0;
    gObjectListStart = 0;
    clear_object_pointers();
    mempool_free(D_800F2CF0[0]);
    mempool_free(D_800F2CF0[1]);
}

extern Object *D_800A0834; // JFG: an object the searches skip
extern f32 D_800AA3FC;     // the search's starting distance, in .rodata
extern f32 D_800AA400;     // the search's starting distance, in .rodata
Object **objGetObjList(s32 *arg0, s32 *cnt);
f32 point_distance_squared(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);

/* JFG: the first word of a type 0x18 object's block */
typedef struct ObjectFlags53 {
    /* 0x00 */ u32 unk0_31 : 4;
    /* 0x00 */ u32 skip : 1; // bit 27: nearest-object searches skip it
} ObjectFlags53;

/* JFG: the nearest live object of the type (type 0x18 ones with bit 0x08000000 of their block's first
   word set are skipped). */
Object *obj_find_nearest_of_type(f32 x, f32 y, f32 z, u8 type) {
    Object *obj;
    Object *nearest;
    s32 start;
    s32 count;
    Object **list;
    s32 i;
    f32 best;
    f32 dist;

    list = objGetObjList(&start, &count);
    best = D_800AA3FC;
    nearest = NULL;
    for (i = start; i < count; i++) {
        obj = list[i];
        if (obj->freed) {
            continue;
        }
        if (obj != D_800A0834 && obj->behaviorId == type) {
            if (type == 0x18 && ((ObjectFlags53 *) obj->racer)->skip) {
                continue;
            }
            dist = point_distance_squared(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position,
                                          x, y, z);
            if (dist < best) {
                best = dist;
                nearest = obj;
            }
        }
    }
    return nearest;
}

/* JFG: the nearest live object whose header has the category (header byte 0x1B). */
Object *obj_find_nearest_in_category(f32 x, f32 y, f32 z, u8 category) {
    Object *obj;
    Object *nearest;
    s32 start;
    s32 count;
    Object **list;
    s32 i;
    f32 best;
    f32 dist;

    list = objGetObjList(&start, &count);
    best = D_800AA400;
    nearest = NULL;
    for (i = start; i < count; i++) {
        obj = list[i];
        if (obj->freed) {
            continue;
        }
        if (obj != D_800A0834 && obj->header->category == category) {
            dist = point_distance_squared(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position,
                                          x, y, z);
            if (dist < best) {
                best = dist;
                nearest = obj;
            }
        }
    }
    return nearest;
}

extern s8 D_800A0898;

void func_80004C64(u8 arg0) {
    D_800A0898 = arg0;
}

extern u16 *D_800F2C9C;           // DKR: gObjectHeaderReferences
extern ObjectHeader **D_800F2C98; // DKR: gLoadedObjectHeaders
extern s32 *D_800F2C74;           // DKR: gAssetsObjectHeadersTable
extern void *D_800F2CB0;          // DKR: gObjectMemoryPool
void *mempool_alloc_pool(void *pool, s32 size);
void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);
void *load_texture(s32 id);
void *tex_load_sprite(s32 id, s32 arg1);
void *func_8003B9E8(s32 id, s32 arg1);
void *asset_cache2_get(s32 id);

/**
 * Set the object's header.
 * Search if the intended header is already loaded and use that.
 * Otherwise, allocate space and load it into ROM and set it to that.
 * JFG: falls back to the main pool, more offsets, and loads the header's models, textures and sprites.
 * DKR: load_object_header
 */
ObjectHeader *load_object_header(s32 index) {
    s32 assetOffset;
    s32 size;
    ObjectHeader *address;
    s32 i;

    if (D_800F2C9C[index] != 0) {
        D_800F2C9C[index]++;
        return D_800F2C98[index];
    }
    assetOffset = D_800F2C74[index];
    size = D_800F2C74[index + 1] - assetOffset;
    address = mempool_alloc_pool(D_800F2CB0, size);
    if (address == NULL && D_800A0898) {
        address = mempool_alloc_safe(size, 0xFFFF);
    }
    if (address != NULL) {
        asset_load(0x2F, (u32) address, assetOffset, size);
        address->unk48 = (u32) address + address->unk48;
        address->unk4C = (u32) address + address->unk4C;
        address->objectParticles = (ObjHeaderParticleEntry *) ((u32) address + (u32) address->objectParticles);
        address->unkB0 = (u32) address + address->unkB0;
        address->unk34 = (u32) address + address->unk34;
        address->unk38 = (u32) address + address->unk38;
        address->unk3C = (u32) address + address->unk3C;
        address->unk30 = (u32) address + address->unk30;
        address->modelIds = (u16 *) ((u32) address + (u32) address->modelIds);
        address->assets = (void **) ((u32) address + (u32) address->assets);
        address->unk50 = (u32) address + address->unk50;
        if (address->unkD0 != 0) {
            address->unkD0 = (u32) address + address->unkD0;
        }
        for (i = 0; i < address->numberOfModelIds; i++) {
            if ((address->modelIds[i] & 0xC000) == 0xC000) {
                address->assets[i] = load_texture(address->modelIds[i] & 0x3FFF);
            } else if (address->modelIds[i] & 0x8000) {
                address->assets[i] = tex_load_sprite(address->modelIds[i] & 0x3FFF, 1);
            } else {
                address->assets[i] = func_8003B9E8(address->modelIds[i], 0);
            }
        }
        if (address->unkD8 != -1) {
            address->unkD4 = asset_cache2_get(address->unkD8);
        } else {
            address->unkD4 = NULL;
        }
        D_800F2C98[index] = address;
        D_800F2C9C[index] = 1;
    } else {
        return NULL;
    }
    return address;
}

extern u16 *D_800F2C9C;           // DKR: gObjectHeaderReferences
extern ObjectHeader **D_800F2C98; // DKR: gLoadedObjectHeaders
void tex_free(void *tex);
void sprite_free(void *sprite);
void free_3d_model(void *model);
void asset_cache2_release(void *arg0);

/**
 * Remove this object from the loaded header's references.
 * If the reference number is zero, free the header.
 * JFG: also frees the header's models, textures and sprites.
 * DKR: try_free_object_header
 */
void try_free_object_header(s32 index) {
    ObjectHeader *header;
    s32 i;

    if (D_800F2C9C[index] != 0) {
        D_800F2C9C[index]--;
        if (D_800F2C9C[index] == 0) {
            header = D_800F2C98[index];
            for (i = 0; i < header->numberOfModelIds; i++) {
                if ((header->modelIds[i] & 0xC000) == 0xC000) {
                    tex_free(header->assets[i]);
                } else if (header->modelIds[i] & 0x8000) {
                    sprite_free(header->assets[i]);
                } else {
                    free_3d_model(header->assets[i]);
                }
            }
            if (header->unkD4 != NULL) {
                asset_cache2_release(header->unkD4);
            }
            mempool_free(header);
        }
    }
}

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

/* returns int, not s32 (long): with int IDO computes each compare straight into v0, as the ROM does */
/* JFG: whether an object type is spawned by troop_buffer_push rather than spawn_object. */
int obj_id_is_troop_spawned(s32 objectID) {
    return objectID == 0x29 || objectID == 0x2A || objectID == 0x2B || objectID == 0x2D || objectID == 0x54 ||
           objectID == 0x149 || objectID == 0x150 || objectID == 0x151 || objectID == 0x152 || objectID == 0x153 ||
           objectID == 0x166 || objectID == 0x5F || objectID == 0x67 || objectID == 0xDF || objectID == 0xE0 ||
           objectID == 0xE1 || objectID == 0x78 || objectID == 0x8C || objectID == 0x83 || objectID == 0x66 ||
           objectID == 0x11C || objectID == 0x11D || objectID == 0x11E || objectID == 0x11F || objectID == 0x120 ||
           objectID == 0x70 || objectID == 0x90 || objectID == 0xA5 || objectID == 0x97 || objectID == 0x157 ||
           objectID == 0x68 || objectID == 0x6D || objectID == 0xBE || objectID == 0xBD || objectID == 0x269 ||
           objectID == 0xEE || objectID == 0xEF || objectID == 0x8D || objectID == 0x99 || objectID == 0x9C ||
           objectID == 0x12F || objectID == 0xE6 || objectID == 0xF2 || objectID == 0xF3 || objectID == 0x121 ||
           objectID == 0xF1 || objectID == 0xF5 || objectID == 0xF6 || objectID == 0x122 || objectID == 0x143 ||
           objectID == 0x144 || objectID == 0x112 || objectID == 0x1E5;
}

/* JFG: a level object entry, reduced */
typedef struct LevelObjectEntry53 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3[0x11 - 3];
    /* 0x11 */ u8 unk11;
} LevelObjectEntry53;

extern s32 D_800F2CD8[]; // JFG: two lists of level object entries
extern s32 D_800F2CE0[]; // JFG: their sizes in bytes
extern s32 D_800F2CE8[]; // JFG: their map IDs
extern s32 D_800F2D00;   // DKR: gObjectMapIndex
extern s32 D_800F2C8C;
void level_entry_remove(LevelObjectEntry53 *entry);
s32 *asset_table_load(s32 tableID);
int obj_id_is_troop_spawned(s32 objectID);
void troop_buffer_push(LevelObjectEntry53 *entry);
void spawn_level_replacement(LevelObjectEntry53 *entry);
void level_records_scan(s32 list, s32 size);
Object *spawn_object(LevelObjectEntry53 *entry, s32 arg1);
void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/**
 * Load the object map into RAM, then start spawning objects into the world.
 * JFG: uncompressed maps, two lists, some objects spawned by other functions; no silver coins.
 * DKR: track_spawn_objects
 */
void track_spawn_objects(s32 mapID, s32 index) {
    s32 assetSize;
    s32 i;
    s32 var_s0;
    s32 *mem;
    u32 assetOffset;
    s32 *objMapTable;
    s32 temp_t3;

    D_800F2CF0[index] = mempool_alloc_safe(0x3000, 0xFFFF);
    // clang-format off
    mem = D_800F2CF0[index]; D_800F2CD8[index] = (s32) mem + 0x10; // Required to be one line to match
    // clang-format on
    D_800F2CE0[index] = 0;
    D_800F2CE8[index] = mapID;
    objMapTable = asset_table_load(0x1C);
    for (i = 0; objMapTable[i] != -1; i++) {}
    i--;
    if (mapID >= i) {
        mapID = 0;
    }
    assetOffset = objMapTable[mapID];
    assetSize = objMapTable[mapID + 1] - assetOffset;
    mempool_free(objMapTable);
    if (assetSize != 0) {
        asset_load(0x1D, (u32) mem, assetOffset, assetSize);
        D_800F2CD8[index] = (s32) D_800F2CF0[index] + 0x10;
        D_800F2CE0[index] = *mem;
        D_800F2D00 = index;
        if (index == 1) {
            level_records_scan(D_800F2CD8[index], D_800F2CE0[index]);
        }
        for (var_s0 = 0; var_s0 < D_800F2CE0[index]; var_s0 += temp_t3) {
            if (obj_id_is_troop_spawned(((LevelObjectEntry53 *) D_800F2CD8[index])->objectID)) {
                troop_buffer_push((LevelObjectEntry53 *) D_800F2CD8[index]);
            } else if (((LevelObjectEntry53 *) D_800F2CD8[index])->objectID == 0x12E) {
                spawn_level_replacement((LevelObjectEntry53 *) D_800F2CD8[index]);
            } else {
                spawn_object((LevelObjectEntry53 *) D_800F2CD8[index], 1);
            }
            D_800F2CD8[index] = (s32) &((u8 *) D_800F2CD8[index])[temp_t3 = ((u8 *) D_800F2CD8[index])[2]];
        }
        D_800F2CD8[index] = (s32) D_800F2CF0[index] + 0x10;
        if (D_800F2C8C == 0) {
            gParticlePtrList_flush();
        }
        D_800F2C8C = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000561C);

/**
 * JFG: take every entry of the object and kind out of both lists (the second first).
 */
void level_entries_remove_object(s32 objectID, s32 kind) {
    s32 *list;
    s32 *size;
    s32 i;
    u8 *entry;
    u8 *end;

    list = &D_800F2CD8[1], size = &D_800F2CE0[1]; // one statement: size's address is set up first
    i = 2;
    while (i--) {
        entry = (u8 *) *list;
        end = *size + entry;
        while (entry < end) {
            if (objectID == ((LevelObjectEntry53 *) entry)->objectID &&
                kind == ((LevelObjectEntry53 *) entry)->unk11) {
                level_entry_remove((LevelObjectEntry53 *) entry);
                end = (u8 *) *list + *size;
            } else {
                entry += ((LevelObjectEntry53 *) entry)->size;
            }
        }
        list--;
        size--;
    }
}

/* JFG: take an entry out of the level object entry list it is in, moving the later entries down
   and fixing the objects' pointers to them. */
void level_entry_remove(LevelObjectEntry53 *entry) {
    s32 i;
    Object *obj;
    s32 size;
    s32 ends[2];
    u8 *src;
    u8 *dst;
    u8 *end;
    s32 listIndex;
    s32 levelEntry;
    UNUSED s32 pad;

    size = entry->size;
    ends[0] = D_800F2CD8[0] + D_800F2CE0[0];
    ends[1] = D_800F2CD8[1] + D_800F2CE0[1];
    if ((s32) entry >= D_800F2CD8[0] && (s32) entry < ends[0]) {
        listIndex = 0;
    } else if ((s32) entry >= D_800F2CD8[1] && (s32) entry < ends[1]) {
        listIndex = 1;
    }
    dst = (u8 *) entry;
    src = dst + size;
    end = (u8 *) ends[listIndex];
    while (src < end) {
        *dst++ = *src++;
    }
    D_800F2CE0[listIndex] -= size;
    for (i = 0; i < gObjectCount; i++) {
        obj = gObjPtrList[i];
        if (obj != NULL && obj->levelEntry != 0) {
            levelEntry = obj->levelEntry;
            if ((s32) entry < obj->levelEntry && obj->levelEntry < (s32) end) {
                obj->levelEntry -= size;
            } else if ((s32) entry == levelEntry) {
                obj->levelEntry = 0;
            }
        }
    }
}

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
s16 obj_header_field_get(s32 type) {
    UNUSED s32 pad;
    s32 offset;
    ObjectHeaderAsset header;

    offset = D_800F2C74[D_800F2CF8[type]];
    asset_load(0x2F, (u32) &header, offset, sizeof(ObjectHeaderAsset));
    return header.unk1C;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80005F14);

void free_3d_model(void *model);
void tex_free(void *tex);
void sprite_free(void *sprite);

/* JFG: free the object's count attachments: models (kind 0), textures (kind 4) or sprites.
   The list is read as words (a pointer-typed read loads into another register). */
void obj_free_assets(Object *obj, s32 count, s32 kind) {
    s32 i;

    if (kind == 0) {
        for (i = 0; i < count; i++) {
            if (((s32 *) obj->attachments)[i] != 0) {
                free_3d_model((void *) ((s32 *) obj->attachments)[i]);
            }
        }
    } else if (kind == 4) {
        for (i = 0; i < count; i++) {
            if (((s32 *) obj->attachments)[i] != 0) {
                tex_free((void *) ((s32 *) obj->attachments)[i]);
            }
        }
    } else {
        for (i = 0; i < count; i++) {
            if (((s32 *) obj->attachments)[i] != 0) {
                sprite_free((void *) ((s32 *) obj->attachments)[i]);
            }
        }
    }
}

/* JFG: an object's child objects (obj->unk60) */
typedef struct ObjectUnk60 {
    /* 0x00 */ s32 count;
    /* 0x04 */ Object *children[10]; // JFG: [0] is the object carried at the model's joints
    /* 0x2C */ s32 unk2C;
} ObjectUnk60;

/* JFG: how a sprite slot's first frame is picked: a random frame, or slot index's frame (bits
   22-27) plus or minus amount (bits 16-21) */
#define SPRITE_SLOT_RANDOM 0x80000000
#define SPRITE_SLOT_AFTER 0x40000000
#define SPRITE_SLOT_BEFORE 0x20000000

/* JFG: a sprite slot definition of an object header (12 bytes) */
typedef struct SpriteSlotDef {
    /* 0x00 */ u16 spriteID;
    /* 0x02 */ s8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 size; // the scale times 500
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ u32 flags;
} SpriteSlotDef;

/* JFG: an object's sprite slot (0x14 bytes) */
typedef struct ObjectSpriteSlot {
    /* 0x00 */ u8 *sprite; // the first byte is the frame count
    /* 0x04 */ s8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 pad6[2];
    /* 0x08 */ f32 scale;
    /* 0x0C */ u32 flags;
    /* 0x10 */ f32 frame;
} ObjectSpriteSlot;

Object *obj_spawn_attachment(s32 type);
void try_free_object_header(s32 index);
s32 rand_range(s32 min, s32 max);

#ifdef NON_MATCHING
/* JFG: create the object's child objects and load its sprite slots (picking each slot's first
   frame); if anything failed, free what was made and return 1.
   NON_MATCHING: IDO sinks index/amount into the two branches that use them (the target computes
   them once, into a0/a1, before the tests); everything after is renumbered. */
s32 func_80006F2C(Object *obj) {
    ObjectHeader *header;
    ObjectUnk60 *children;
    ObjectSpriteSlot *slot;
    s32 failed;
    s32 i;
    Object *child;
    SpriteSlotDef *def;
    u32 flags;
    s32 index;
    s32 amount;
    s32 numFrames;

    header = obj->header;
    children = NULL;
    slot = NULL;
    failed = FALSE;
    if (header->numChildren > 0 && header->numChildren < 10) {
        i = 0;
        children = obj->unk60;
        children->count = header->numChildren;
        for (; i < children->count; i++) {
            child = obj_spawn_attachment(((s32 *) obj->header->unk34)[i]);
            if (child != NULL) {
                child->trans.scale *= obj->trans.scale;
            }
            children->children[i] = child;
            if (child == NULL) {
                failed = TRUE;
            }
        }
        children->unk2C = obj->header->unk38;
        header = obj->header;
    }
    obj->numSpriteSlots = header->numSpriteSlots;
    if (obj->numSpriteSlots > 0 && header->numSpriteSlots < 10) {
        slot = obj->spriteSlots;
        def = (SpriteSlotDef *) header->unk3C;
        for (i = 0; i < obj->numSpriteSlots; i++) {
            slot->sprite = tex_load_sprite(def->spriteID, 1);
            slot->unk4 = def->unk2;
            flags = def->flags;
            slot->flags = flags;
            index = (flags >> 22) & 0x3F;
            amount = (flags >> 16) & 0x3F;
            slot->scale = (def->size / 500.0f) * obj->trans.scale;
            slot->unk5 = def->unk3;
            numFrames = slot->sprite[0];
            if (flags & SPRITE_SLOT_RANDOM) {
                slot->frame = rand_range(0, numFrames);
            } else if (flags & SPRITE_SLOT_AFTER) {
                slot->frame = obj->spriteSlots[index].frame + amount;
            } else if (flags & SPRITE_SLOT_BEFORE) {
                slot->frame = obj->spriteSlots[index].frame - amount;
            }
            def++;
            if (numFrames < slot->frame) {
                slot->frame -= numFrames;
            } else if (slot->frame < 0.0f) {
                slot->frame += numFrames;
            }
            if (slot->sprite == NULL) {
                failed = TRUE;
            }
            slot++;
        }
    }
    if (failed) {
        if (children != NULL) {
            for (i = 0; i < children->count; i++) {
                child = children->children[i];
                if (child != NULL) {
                    obj_free_assets(child, child->header->numAttachments, child->header->unk1E);
                    try_free_object_header(child->headerIndex);
                    mempool_free(child);
                }
            }
            children->unk2C = 0;
        }
        if (slot != NULL) {
            slot = obj->spriteSlots;
            for (i = 0; i < obj->numSpriteSlots; i++) {
                // JFG: every pass frees slot 0 (the slot pointer never advances)
                if (slot->sprite != NULL) {
                    sprite_free(slot->sprite);
                    slot->sprite = NULL;
                }
            }
        }
    }
    return failed;
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_80006F2C);
#endif

/* DKR include/structs.h, reduced (Size: 0x24 bytes) */
typedef struct ParticleEmitter {
    /* 0x00 */ u8 pad0[0x24];
} ParticleEmitter;

void emitter_init(ParticleEmitter *emitter, s32 behaviourID, s32 particleID);
void emitter_init_with_pos(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s16 posX, s16 posY, s16 posZ);
void emitter_init_at_origin(ParticleEmitter *emitter, s32 behaviourID, s32 particleID, s32 arg3);

#ifdef NON_MATCHING
/**
 * JFG: a third kind of entry (0xFFFE....) goes to emitter_init_at_origin.
 * DKR: obj_init_emitter
 * NON_MATCHING: 2 words: the two induction variables are stepped in the other order at the loop end.
 */
s32 func_800072B4(Object *obj, ParticleEmitter *emitter) {
    ObjHeaderParticleEntry *particleDataEntry;
    s32 i;

    obj->particleEmitter = emitter;
    particleDataEntry = obj->header->objectParticles;
    for (i = 0; i < obj->header->particleCount; i++) {
        if ((particleDataEntry[i].upper & 0xFFFF0000) == 0xFFFF0000) {
            emitter_init(&obj->particleEmitter[i], (particleDataEntry[i].upper >> 8) & 0xFF,
                         particleDataEntry[i].upper & 0xFF);
        } else if ((particleDataEntry[i].upper & 0xFFFF0000) == 0xFFFE0000) {
            emitter_init_at_origin(&obj->particleEmitter[i], (particleDataEntry[i].upper >> 8) & 0xFF,
                          particleDataEntry[i].upper & 0xFF, particleDataEntry[i].lower & 0xFF);
        } else {
            emitter_init_with_pos(&obj->particleEmitter[i], (particleDataEntry[i].upper >> 0x18) & 0xFF,
                                  (particleDataEntry[i].upper >> 0x10) & 0xFF, particleDataEntry[i].upper & 0xFFFF,
                                  (particleDataEntry[i].lower >> 0x10) & 0xFFFF, particleDataEntry[i].lower & 0xFFFF);
        }
    }
    return ((obj->header->particleCount * sizeof(ParticleEmitter)) + 3) & ~3;
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_800072B4);
#endif

/* JFG: attach a block to the object, mark it, and return a size. */
s32 func_8000740C(Object *obj, u8 *arg1) {
    obj->unk7C = arg1;
    *arg1 = 2;
    return 0x28;
}

/* JFG: an object's sprite block (obj_sprite_init) */
typedef struct ObjectSprite {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ void *texture;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ u16 unkA;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 pad10[0x1C - 0x10];
    /* 0x1C */ s32 unk1C;
} ObjectSprite;

extern u8 D_800A4FC4;
extern void *D_800F2CA0;
void *load_texture(s32 id);

/* JFG: set up the object's sprite block from its header and load its texture; returns the block's size (0 on failure). */
s32 obj_sprite_init(Object *obj, ObjectSprite *sprite) {
    ObjectHeader *header;
    UNUSED s32 pad;
    s32 size;

    header = obj->header;
    obj->sprite = sprite;
    sprite->unk0 = header->unk54;
    sprite->unkC = header->unk61;
    sprite->unkD = header->unk62;
    sprite->unkE = header->unk63;
    sprite->unkF = 0;
    sprite->unk8 = 0;
    sprite->unkA = header->unk66;
    sprite->unk1C = 0;
    size = 0x20;
    if (D_800A4FC4 == 0) {
        if (sprite->unkC & 8) {
            sprite->unk1C = (((s32) sprite + 0x20) & ~3) + 4;
            size = sprite->unk1C - (s32) sprite + 0xBC;
        } else if (sprite->unkC & 0x10) {
            sprite->unk1C = (((s32) sprite + 0x20) & ~3) + 4;
            size = sprite->unk1C - (s32) sprite + 0x38;
        }
    } else {
        sprite->unkC = 5;
        sprite->unkD = 1;
    }
    sprite->texture = load_texture(header->unk64);
    if (sprite->texture == NULL) {
        return 0;
    }
    D_800F2CA0 = sprite->texture;
    return (size & ~3) + 4;
}

/* JFG: a texture of an object model, reduced */
typedef struct ModelTexture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0x12 - 2];
    /* 0x12 */ u16 numOfTextures;
    /* 0x14 */ u8 pad14[2];
    /* 0x16 */ s16 frameSize;
    /* 0x18 */ u8 loops;
} ModelTexture;

typedef struct ModelTextureEntry {
    /* 0x00 */ ModelTexture *texture;
    /* 0x04 */ u8 pad4[4];
} ModelTextureEntry;

/* DKR include/structs.h TriangleBatchInfo, reduced (Size 0x10) */
typedef struct ModelBatch {
    /* 0x00 */ u8 textureIndex;
    /* 0x01 */ u8 pad1[7];
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[6];
} ModelBatch;

/* JFG: a point of an object model (12 bytes) */
typedef struct ModelPoint {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ s8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u16 unk6;
    /* 0x08 */ f32 size;
} ModelPoint;

typedef struct ObjectModel53 {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numberOfTextures;
    /* 0x11 */ u8 hasTexAnims;
    /* 0x12 */ u8 pad12[0x16 - 0x12];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ ModelTextureEntry *textures;
    /* 0x1C */ u8 pad1C[0x20 - 0x1C];
    /* 0x20 */ Triangle *triangles;
    /* 0x24 */ ModelBatch *batches;
    /* 0x28 */ u8 pad28[0x2C - 0x28];
    /* 0x2C */ u8 numTexAnims;
    /* 0x2D */ u8 pad2D[2];
    /* 0x2F */ u8 numPoints;
    /* 0x30 */ u16 *unk30; // JFG: [1] is a joint index
    /* 0x34 */ u8 pad34[0x38 - 0x34];
    /* 0x38 */ ModelPoint *points;
    /* 0x3C */ u8 pad3C[0x4E - 0x3C];
    /* 0x4E */ s8 numAnims;
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ s8 **anims;
    /* 0x54 */ u8 pad54[0x5C - 0x54];
    /* 0x5C */ s16 *animIds;
    /* 0x60 */ u8 pad60[0x64 - 0x60];
    /* 0x64 */ u8 unk64; // nonzero: animated another way
    /* 0x65 */ u8 pad65[0x74 - 0x65];
    /* 0x74 */ Gfx *dl;
    /* 0x78 */ Gfx *dlTransparent;
} ObjectModel53;

/* JFG: an object's point state (0x34 bytes) */
typedef struct ObjectPoint {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u16 unk6;
    /* 0x08 */ s8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 padC[0x24 - 0xC];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ s16 unk2C;
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ u8 unk2F;
    /* 0x30 */ u8 unk30_7 : 1;
    /* 0x30 */ u8 unk30_6 : 1;
} ObjectPoint;

/* JFG: an object's block of point states (obj->unk4C) */
typedef struct ObjectPoints {
    /* 0x00 */ u8 pad0[0x9];
    /* 0x09 */ u8 flags9; // JFG: 8 = the object is updated elsewhere
    /* 0x0A */ u8 padA[0x32 - 0xA];
    /* 0x32 */ s16 count;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ ObjectPoint *points;
} ObjectPoints;

#ifdef NON_MATCHING
/* JFG: set up the object's point states (one per model point, 40 more for type 0xE6) after the
   block's 0x44-byte head; returns the size used.
   NON_MATCHING: the right length; the register allocation differs (the target keeps a copy of
   block in v0 for the first loop's bound and the model in a2). */
s32 func_80007528(Object *obj, ObjectPoints *block, s32 type) {
    s32 size;
    ObjectModel53 *model;
    ObjectPoint *points;
    ObjectPoints *b;
    s32 i;

    obj->unk4C = block;
    b = block;
    size = 0x44;
    if (obj->header->unk1E == 0) {
        points = (ObjectPoint *) ((u8 *) block + 0x44);
        model = obj->attachments[0]->model;
        if (model->numPoints > 0) {
            block->count = model->numPoints;
            block->points = points;
            size = block->count * 0x34 + 0x44;
            for (i = 0; i < b->count; i++) {
                points[i].unk0 = 0;
                points[i].unk2 = 0;
                points[i].unk4 = 0;
                points[i].unk6 = model->points[i].unk0;
                points[i].unk8 = model->points[i].unk2;
                points[i].unk9 = model->points[i].unk3;
                points[i].unkA = 0;
                points[i].unk24 = model->points[i].size * obj->trans.scale;
                points[i].unk28 = model->points[i].size * obj->trans.scale;
                points[i].unk2E = model->points[i].unk4;
                points[i].unk30_7 = 1;
                points[i].unk2C = 0;
                points[i].unk30_6 = 0;
                points[i].unk2F = model->points[i].unk6;
            }
        }
    }
    if (type == 0xE6) {
        b = obj->unk4C;
        points = (ObjectPoint *) ((u8 *) block + size);
        b->count = 40;
        size += b->count * 0x34;
        b->points = points;
        for (i = 0; i < b->count; i++) {
            points[i].unk0 = 0;
            points[i].unk30_7 = 1;
            points[i].unk2 = 0;
            points[i].unk4 = 0;
            points[i].unk6 = 0;
            points[i].unk8 = -1;
            points[i].unk9 = i;
            points[i].unkA = 0;
            points[i].unk24 = 40.0f;
            points[i].unk28 = 40.0f;
            points[i].unk2E = 100;
            points[i].unk2F = 0;
            points[i].unk2C = 0;
            points[i].unk30_6 = 0;
        }
    }
    return size;
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_80007528);
#endif

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

extern s32 D_800F2C78;     // JFG: the number of object types
extern Object *D_800F2C64; // JFG: the buffer an attachment object is built in
extern s32 D_800F2C68;     // JFG: its size in bytes
s32 func_8000F978(s32 id);
void *mempool_alloc(s32 size, u32 colourTag);

/**
 * Attempts to spawn an attachment object.
 * JFG: built in a scratch buffer (cleared first), then copied into an allocation of its size
 * rounded up to 16; the models get flags from func_8000F978.
 * DKR: obj_spawn_attachment
 */
Object *obj_spawn_attachment(s32 objID) {
    ObjectHeader *header;
    Object *obj;
    s32 modelFlags;
    s32 i;
    u8 *end;
    s32 failed;
    s32 size;
    s8 numModels;
    s8 modelType;
    s32 *p;
    Object *result;
    s32 *src;

    if (objID >= D_800F2C78) {
        objID = 0;
    }
    header = load_object_header(objID);
    if (header == NULL) {
        return NULL;
    }
    size = header->numAttachments * 4 + 0xB4;
    end = (u8 *) D_800F2C64 + size;
    i = D_800F2C68 >> 2;
    obj = D_800F2C64;
    p = (s32 *) obj;
    while (i--) {
        *p++ = 0;
    }
    obj->header = header;
    obj->trans.flags = 2;
    obj->headerIndex = objID;
    obj->unk4A = objID;
    obj->trans.scale = header->scale;
    obj->trans.flags |= header->flags & 0x20C;
    modelFlags = func_8000F978(header->unk1C) & 7;
    numModels = obj->header->numAttachments;
    modelType = obj->header->unk1E;
    obj->attachments = (ObjectAttachment **) ((u8 *) obj + 0xB4);
    i = 0;
    failed = FALSE;
    for (; i < numModels; i++) {
        if (modelType == 0) {
            obj->attachments[i] = func_8003B9E8(((s32 *) obj->header->unk30)[i], modelFlags);
        } else {
            obj->attachments[i] = tex_load_sprite(((s32 *) obj->header->unk30)[i], 10);
        }
        if (obj->attachments[i] == NULL) {
            failed = TRUE;
        }
    }
    if (!failed) {
        size = end - (u8 *) obj;
        if (size & 0xF) {
            size = (size + 0xF) & ~0xF;
        }
        result = mempool_alloc(size, 0xFFFF);
        if (result != NULL) {
            src = (s32 *) D_800F2C64;
            p = (s32 *) result;
            i = size >> 2;
            while (i--) {
                *p++ = *src++;
            }
            result->attachments = (ObjectAttachment **) ((u8 *) result + 0xB4);
        } else {
            failed = TRUE;
        }
    }
    if (failed) {
        obj_free_assets(obj, numModels, modelType);
        try_free_object_header(objID);
        result = NULL;
    }
    return result;
}

extern Object **gParticlePtrList;
extern s32 gFreeListCount;
extern s32 gFreedObjectIds[16];
extern u8 gFreedObjectIdIndex;
void chain_object_unlink_from_players(Object *obj);

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
        chain_object_unlink_from_players(object);
        gFreedObjectIds[gFreedObjectIdIndex] = (object->behaviorId << 16) | object->unk4A;
        gFreedObjectIdIndex++;
        gFreedObjectIdIndex &= 0xF;
    }
}

/**
 * Take every object on the free list out of the object list (or else the second list) and destroy it.
 * JFG: the second list, and the search stops at the object found.
 * DKR: gParticlePtrList_flush
 */
void gParticlePtrList_flush(void) {
    s32 j, i, k, search_indx;
    Object *searchObj;

    D_800F2CC4 = 0;
    for (i = 0; i < gFreeListCount; i++) {
        search_indx = -1;
        searchObj = gParticlePtrList[i];
        for (j = 0; j < gObjectCount; j++) {
            if (searchObj == gObjPtrList[j]) {
                search_indx = j;
                if (j < D_800F2CC2) {
                    D_800F2CC2--;
                }
                gObjectCount--;
                for (k = j; k < gObjectCount; k++) {
                    gObjPtrList[k] = gObjPtrList[k + 1];
                }
                func_80007C68(searchObj, 0);
                j = gObjectCount;
            }
        }
        if (search_indx == -1) {
            for (j = 0; j < D_800F2CB8; j++) {
                if (searchObj == D_800F2CB4[j]) {
                    D_800F2CB8--;
                    for (k = j; k < D_800F2CB8; k++) {
                        D_800F2CB4[k] = D_800F2CB4[k + 1];
                    }
                    func_80007C68(searchObj, 0);
                    j = D_800F2CB8;
                }
            }
        }
    }
    gFreeListCount = 0;
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_80007C68);

extern void **D_800F2D18; // JFG: the animated objects

/* JFG: take an object off the animated list (the last one takes its place). */
void anim_obj_list_remove(void *obj) {
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

/* JFG: an object's sprite as the object update animates it */
typedef struct ObjSpriteAnim {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ struct {
        u8 pad0[0x12];
        u16 numFrames; // in 1/256 frames
    } *texture;
    /* 0x08 */ u16 frame;
    /* 0x0A */ u16 speed;
} ObjSpriteAnim;

extern s32 D_800F2C90; // DKR: gEventCountdown
extern s32 D_800F2C94; // DKR: gEventStartTimer
extern u8 D_800A4FD0;  // JFG: the number of players
extern u8 D_800A4FD8;  // JFG: the multiplayer game type
extern u8 D_800A4FB4;
extern s32 D_800F2C8C;
extern f32 D_800F2C88; // DKR: gObjectUpdateRateF
extern u8 D_800A0974;
extern u8 D_800A0978;
extern s32 D_800F2EA0[16];
extern s32 D_800F2EE0[16];
void func_8007A534(void);
void func_80029CFC(s32 updateRate);
Object **collision_objects_get(s32 *count);
void func_8000FBF0(Object *obj, s32 updateRate);
void func_8007E504(void);
void func_8000DF00(Object *obj, s32 updateRate);
void func_8006B090(Object *obj, s32 updateRate);
void func_80032A48(Object *obj, s32 updateRate);
void light_update_all(s32 updateRate);
void objects_shade_all(void);
s32 runlink_module_loaded(s32 module);
void sound_nearest_emitter_update(void);
void audspat_update_all(Object **racers, s32 numRacers, s32 updateRate);
s32 sprite_anim_advance(void *sprite, s32 *flags, s32 updateRate, f32 *frame, s32 speed);
void func_80054530(); // a module function (patched in by the runtime linker)

/**
 * Updates all objects in the game.
 * JFG: the racers' previous positions and angles, the collision objects, then the other objects (but types 9,
 * 0x1E and 0x25: their sprite and sprite-slot animations too), the players, types 9 and 0x25, the lights and
 * the sounds; then three ring buffers get their next slot cleared.
 * DKR: obj_update
 */
void obj_update(s32 updateRate) {
    s32 j;
    s32 i;
    s32 k;
    Object **objs;
    Object *obj;
    Object_Racer *racer;
    ObjSpriteAnim *sprite;
    ObjectSpriteSlot *slot;

    D_800F2C94 = D_800F2C90;
    if (D_800F2C90 > 0) {
        D_800F2C90 -= updateRate;
    }
    if (D_800F2C90 < 0) {
        D_800F2C90 = 0;
    }
    for (i = 0; i < gNumRacers; i++) {
        racer = (*gRacers)[i]->racer;
        racer->prevX = (*gRacers)[i]->trans.x_position;
        racer->prevY = (*gRacers)[i]->trans.y_position;
        racer->prevZ = (*gRacers)[i]->trans.z_position;
        racer->prevRotation[0] = (*gRacers)[i]->trans.y_rotation;
        racer->prevRotation[1] = (*gRacers)[i]->trans.x_rotation;
        racer->prevRotation[2] = (*gRacers)[i]->trans.z_rotation;
    }
    func_8007A534();
    func_80029CFC(updateRate);
    objs = collision_objects_get(&j);
    for (i = 0; i < j; i++) {
        func_8000FBF0(objs[i], updateRate);
    }
    func_8007E504();
    i = gObjectCount;
    for (j = gObjectListStart; j < i; j++) {
        obj = gObjPtrList[j];
        if (obj->behaviorId != 0x1E && obj->behaviorId != 9 && obj->behaviorId != 0x25) {
                if (obj->unk4C == NULL || (obj->unk4C != NULL && !(obj->unk4C->flags9 & 8))) {
                    func_8000FBF0(obj, updateRate);
                }
                if (obj->header->unk1E == 0 && obj->header->scrollTexture != 0xFF) {
                    func_8000DF00(obj, updateRate);
                }
                if (obj->behaviorId != 1 && obj->unk58 != NULL) {
                    func_8006B090(obj, updateRate);
                }
                sprite = (ObjSpriteAnim *) obj->sprite;
                if (sprite != NULL && sprite->speed != 0 && sprite->texture != NULL) {
                    sprite->frame += sprite->speed * updateRate;
                    while (sprite->frame >= sprite->texture->numFrames) {
                        sprite->frame -= sprite->texture->numFrames;
                    }
                }
                if (obj->spriteSlots != NULL) {
                    slot = obj->spriteSlots;
                    for (k = 0; k < obj->numSpriteSlots; k++) {
                        sprite_anim_advance(slot->sprite, (s32 *) &slot->flags, slot->unk5, &slot->frame, updateRate);
                        slot++;
                    }
                }
        }
    }
    if (D_800A4FC4 && D_800A4FD8 == 6) {
        func_80054530(D_800A4FD0, updateRate);
    } else {
        for (j = 0; j < gNumRacers; j++) {
            func_80032A48((*gRacers)[j], updateRate);
        }
        if ((D_800A4FC4 && D_800A4FD8 == 4) || D_800A4FB4) {
            func_80054530(updateRate);
        }
    }
    for (j = gObjectListStart; j < i; j++) {
        obj = gObjPtrList[j];
        if (obj->behaviorId == 9 || obj->behaviorId == 0x25) {
            func_8000FBF0(obj, updateRate);
        }
    }
    light_update_all(updateRate);
    objects_shade_all();
    if (runlink_module_loaded(0x1C)) {
        func_80054530();
    }
    sound_nearest_emitter_update();
    audspat_update_all(*gRacers, gNumRacers, updateRate);
    if (D_800F2C8C == 0) {
        gParticlePtrList_flush();
    }
    D_800F2C8C = 1;
    D_800F2C88 = updateRate;
    D_800F2EA0[D_800A0974] = -1;
    D_800A0974++;
    D_800A0974 &= 0xF;
    gFreedObjectIds[gFreedObjectIdIndex] = -1;
    gFreedObjectIdIndex++;
    gFreedObjectIdIndex &= 0xF;
    D_800F2EE0[D_800A0978] = -1;
    D_800A0978++;
    D_800A0978 &= 0xF;
}

extern s32 D_800F2C8C;
extern f32 D_800F2C88;
extern s32 D_800F6F54;
void group_light_update(Object *obj, s32 updateRate);
void audspat_update_all(Object **racers, s32 numRacers, s32 updateRate);
void func_80054530(); // a module function (patched in by the runtime linker)

/* JFG: update the objects some types need first (one of type 0x80 last), then the racers. */
void objects_update_special(s32 updateRate) {
    s32 i;
    s32 count;
    Object *obj;
    Object *deferred;

    deferred = NULL;
    count = gObjectCount;
    for (i = gObjectListStart; i < count; i++) {
        obj = gObjPtrList[i];
        switch (obj->behaviorId) {
            case 0x64:
                func_80054530(obj, updateRate);
                break;
            case 0x43:
                group_light_update(obj, updateRate);
                break;
            case 0x80:
                deferred = obj;
                break;
            case 0x81:
                func_80054530(obj, updateRate);
                break;
        }
    }
    if (deferred != NULL) {
        func_80054530(deferred, updateRate);
    }
    audspat_update_all(*gRacers, gNumRacers, updateRate);
    if (D_800F2C8C == 0) {
        gParticlePtrList_flush();
    }
    D_800F2C8C = 1;
    D_800F2C88 = updateRate;
    D_800F6F54 = 0;
}

/* JFG: an animated texture of an object model (8 bytes) */
typedef struct ModelTexAnim {
    /* 0x00 */ s16 offset;
    /* 0x02 */ s16 nextOffset; // < 0: none
    /* 0x04 */ u32 flags; // the texture index in the low byte
} ModelTexAnim;

typedef struct ModelInstance53 {
    /* 0x00 */ u8 pad0[0xB];
    /* 0x0B */ s8 outIndex;
    /* 0x0C */ u8 padC[0x74 - 0xC];
    /* 0x74 */ ModelTexAnim *texAnims;
    /* 0x78 */ s16 *texOffsets[2];
} ModelInstance53;

typedef struct ObjectTexAnimState {
    /* 0x00 */ u8 pad0[0x9F];
    /* 0x9F */ u8 unk9F;
} ObjectTexAnimState;

extern u8 D_800A4E78;
s32 is_game_paused(void);
void tex_animate_texture(ModelTexture *texture, u32 *triangleBatchInfoFlags, s32 *arg2, s32 updateRate);

/* JFG: animate an object model's textures and write out the offsets (and next offsets) for drawing. */
void obj_tex_animate(ObjectTexAnimState *obj, ObjectModel53 *model, ModelInstance53 *inst, s32 updateRate) {
    s32 texIndex;
    s32 offset;
    ModelTexAnim *anim;
    s16 *out;
    s32 i;
    ModelTexture *tex;

    if (is_game_paused()) {
        updateRate = 0;
    }
    anim = inst->texAnims;
    if (anim != NULL) {
        out = inst->texOffsets[inst->outIndex];
        for (i = 0; i < model->numTexAnims; i++) {
            texIndex = anim->flags & 0xFF;
            tex = model->textures[texIndex].texture;
            if (anim->flags & 0x10000) {
                offset = anim->offset;
                if (anim->flags & 0x20000) {
                    D_800A4E78 = obj->unk9F;
                }
                tex_animate_texture(tex, &anim->flags, &offset, updateRate);
                anim->offset = offset;
                if (anim->nextOffset >= 0) {
                    offset += 0x100;
                    if (offset >= tex->numOfTextures) {
                        if (tex->loops) {
                            offset = 0;
                        } else {
                            offset -= 0x100;
                        }
                    }
                    anim->nextOffset = offset;
                }
            }
            if (anim->nextOffset >= 0) {
                *out++ = (anim->nextOffset >> 8) * tex->frameSize;
            }
            *out++ = (anim->offset >> 8) * tex->frameSize;
            anim++;
        }
        if (obj->unk9F == 1) {
            obj->unk9F = 2;
        }
    }
}

/**
 * JFG: set a byte of every attachment of the objects whose header has unk1E clear.
 */
void objects_set_attachment_byte(s32 value) {
    s32 i;
    s32 j;
    Object *obj;
    ObjectAttachment *attachment;

    i = gObjectListStart; // assigned before the loop so it is loaded before gObjectCount
    for (; i < gObjectCount; i++) {
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
void player_restart(Object *obj, Object_Racer *racer); // JFG: restart a player
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
            player_restart(obj, obj->racer);
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

/* DKR include/f3ddkr.h: a raw two-word command */
#define fast3d_cmd(pkt, word0, word1) \
    {                                 \
        Gfx *_g = (Gfx *) (pkt);      \
        _g->words.w0 = word0;         \
        _g->words.w1 = word1;         \
    }

/* JFG: the game's own vertex load (include/modules/gfx.h gSPVertexJFG), here given the index of
   the last vertex */
#define gSPVertexLast(pkt, v, last, v0) \
    gDma1p(pkt, G_VTX, v, ((last) << 3) + ((last) << 1) + 8, ((last) << 3) | ((u32) (v) & 6) | (v0))

/* DKR include/f3ddkr.h */
#define gSPPolygon(dl, ptr, numTris, texEnabled)                                                                   \
    {                                                                                                              \
        Gfx *_g = (Gfx *) (dl);                                                                                    \
        _g->words.w0 =                                                                                             \
            _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(5, 24, 8) | _SHIFTL(((numTris) * 16), 0, 16); \
        _g->words.w1 = (unsigned int) (ptr);                                                                       \
    }

/* JFG: an object drawn from its own display lists */
typedef struct ObjectDisplayLists {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Gfx *dl;
    /* 0x10 */ Gfx *dlTransparent;
    /* 0x14 */ u8 pad14[0x1C - 0x14];
    /* 0x1C */ u8 unk1C;
} ObjectDisplayLists;

void mtx_cam_push(Gfx **dList, Mtx **mtx, void *trans, f32 scaleY, f32 offsetY);
void rendermode_reset(Gfx **dList);
void mtx_pop(Gfx **dList);
void func_80041DC0(Gfx **dList); // the same as mtx_pop
void func_80055C00(Gfx **dList, void *tex, s32 flags, s32 texOffset); // DKR: material_set
extern u8 D_A0838[];

/* JFG: draw the object's display list at the origin, after D_A0838 when unk1C is set. */
void obj_draw_displaylist_origin(Object *obj) {
    ObjectDisplayLists *lists;

    lists = (ObjectDisplayLists *) obj->racer;
    obj->trans.x_position = 0.0f;
    obj->trans.y_position = 0.0f;
    obj->trans.z_position = 0.0f;
    mtx_cam_push(&gObjectCurrDisplayList, &gObjectCurrMatrix, obj, 1.0f, (f32) 0);
    rendermode_reset(&gObjectCurrDisplayList);
    gDPSetPrimColor(gObjectCurrDisplayList++, 0, 0, 255, 255, 255, obj->opacity);
    gDPSetEnvColor(gObjectCurrDisplayList++, 255, 255, 255, 0);
    fast3d_cmd(gObjectCurrDisplayList++, 0xBF000000, 0);
    if (lists->unk1C) {
        fast3d_cmd(gObjectCurrDisplayList++, 0x02000050, (u32) D_A0838);
    }
    gSPDisplayList(gObjectCurrDisplayList++, OS_K0_TO_PHYSICAL(lists->dl));
    mtx_pop(&gObjectCurrDisplayList);
    rendermode_reset(&gObjectCurrDisplayList);
    gDPSetPrimColor(gObjectCurrDisplayList++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(gObjectCurrDisplayList++, 255, 255, 255, 0);
}

/* JFG: draw the object's display list (its second one while it is see-through). */
void obj_draw_displaylist(Object *obj) {
    ObjectDisplayLists *lists;

    lists = (ObjectDisplayLists *) obj->racer;
    rendermode_reset(&gObjectCurrDisplayList);
    gDPSetPrimColor(gObjectCurrDisplayList++, 0, 0, 255, 255, 255, obj->opacity);
    gDPSetEnvColor(gObjectCurrDisplayList++, 255, 255, 255, 0);
    mtx_cam_push(&gObjectCurrDisplayList, &gObjectCurrMatrix, obj, 1.0f, 0.0f);
    fast3d_cmd(gObjectCurrDisplayList++, 0xBF000000, 0);
    fast3d_cmd(gObjectCurrDisplayList++, 0x02000050, (u32) D_A0838);
    if (obj->opacity == 255) {
        gSPDisplayList(gObjectCurrDisplayList++, OS_K0_TO_PHYSICAL(lists->dl));
    } else {
        gSPDisplayList(gObjectCurrDisplayList++, OS_K0_TO_PHYSICAL(lists->dlTransparent));
    }
    fast3d_cmd(gObjectCurrDisplayList++, 0xBF000000, 0);
    mtx_pop(&gObjectCurrDisplayList);
    gDPSetPrimColor(gObjectCurrDisplayList++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(gObjectCurrDisplayList++, 255, 255, 255, 0);
    rendermode_reset(&gObjectCurrDisplayList);
}

/**
 * Set up the basic model view matrix, load a texture, then render the mesh.
 * A much simpler, faster way to render an object model as opposed to render_3d_model
 * JFG: a grey level and alpha for the colour (unless flags 0x240), and numVertices
 * is the index of the last vertex.
 * DKR: render_misc_model
 */
void render_misc_model(Object *obj, Vertex *verts, u32 numVertices, Triangle *triangles, u32 numTriangles,
                   void *tex, u32 flags, u32 texOffset, f32 scaleY, s32 grey, s32 alpha) {
    s32 hasTexture = FALSE;
    mtx_cam_push(&gObjectCurrDisplayList, &gObjectCurrMatrix, &obj->trans, scaleY, 0.0f);
    if (!(flags & 0x240)) {
        gDPSetPrimColor(gObjectCurrDisplayList++, 0, 0, grey, grey, grey, alpha);
        gDPSetEnvColor(gObjectCurrDisplayList++, 255, 255, 255, 0);
    }
    if (tex != NULL) {
        hasTexture = TRUE;
    }
    if (alpha < 255) {
        flags |= 4;
    }
    func_80055C00(&gObjectCurrDisplayList, tex, flags, texOffset);
    gSPVertexLast(gObjectCurrDisplayList++, OS_K0_TO_PHYSICAL(verts), numVertices, 0);
    gSPPolygon(gObjectCurrDisplayList++, OS_K0_TO_PHYSICAL(triangles), numTriangles, hasTexture);
    func_80041DC0(&gObjectCurrDisplayList);
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000A300);

void screen_line_add_3d(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, s32 colour); // JFG: draw a 3D line

/* JFG: draw a line from the point in the object's block at 0x68 to the object. */
void obj_debug_line_draw(Object *obj) {
    f32 *from = (f32 *) obj->racer;

    screen_line_add_3d(from[1], from[2], from[3], obj->trans.x_position, obj->trans.y_position,
                  obj->trans.z_position, 0x13);
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000AA34);

/* JFG: the block of a type 7 object (drawn twice, or scaled) */
typedef struct ObjectTwinDraw {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ f32 scale;
    /* 0x08 */ u8 pad8[0x34 - 0x8];
    /* 0x34 */ s16 offset;
    /* 0x36 */ u8 pad36[0x42 - 0x36];
    /* 0x42 */ u8 kind;
} ObjectTwinDraw;

f32 func_80049C30(s32 angle); // cosine of a 16-bit angle
f32 func_80049C34(s32 angle); // sine of a 16-bit angle
void func_8000CC64(Object *obj);
extern f32 D_800F2E98;

/* JFG: draw a type 7 object: kind 3 draws attachments 1 and 2 either side of it along its facing,
   kind 5 draws it at its block's scale. */
void obj_draw_twin(Object *obj) {
    ObjectTwinDraw *block;
    f32 dx;
    f32 dz;

    block = (ObjectTwinDraw *) obj->racer;
    if (block->kind == 3) {
        if (block->offset == 0) {
            obj->attachmentIndex = 0;
            func_8000CC64(obj);
            return;
        }
        dx = func_80049C30(-obj->trans.y_rotation) * block->offset;
        dz = func_80049C34(-obj->trans.y_rotation) * block->offset;
        obj->attachmentIndex = 1;
        obj->trans.x_position -= dx;
        obj->trans.z_position -= dz;
        func_8000CC64(obj);
        obj->attachmentIndex = 2;
        obj->trans.x_position += dx + dx;
        obj->trans.z_position += dz + dz;
        func_8000CC64(obj);
        obj->attachmentIndex = 0;
        obj->trans.x_position -= dx;
        obj->trans.z_position -= dz;
    } else if (block->kind == 5) {
        D_800F2E98 = block->scale;
        func_8000CC64(obj);
        D_800F2E98 = 1.0f;
    } else {
        func_8000CC64(obj);
    }
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000B46C);

extern s16 D_800A0BD8[]; // JFG: the type 0x33 object IDs that take the point's position, -1 ends
s32 func_8003B740(Object *obj, Object_Racer *racer);
void func_80048B60(f32 *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz);

#ifdef NON_MATCHING
/* JFG: build the matrix of the object's held item (index slot, 8 past the first nine or while the
   player's unk53C is set, 0 while unk1FA's check is under 2) from the model's joint matrix, and
   transform the item's point by it; returns whether it did.
   NON_MATCHING: one word longer: the copy loop counts with != (target: slti 15), and slot / model
   swap between register and stack. */
s32 func_8000BC28(Object *obj, ObjectAttachment *inst, ObjectModel53 *model, s32 slot, f32 *mtx, f32 *x, f32 *y,
                  f32 *z) {
    Object_Racer *racer;
    s32 result;
    Object *held;
    f32 *dst;
    f32 *src;
    s32 i;
    s16 *id;
    ObjectAttachment *item;

    racer = obj->racer;
    result = 0;
    if (obj->behaviorId == 1) {
        if (racer->unk1FA != 0 && func_8003B740(obj, racer) < 2) {
            slot = 0;
        } else if (slot >= 9 || racer->unk53C > 0) {
            slot = 8;
        }
    }
    if (obj->unk60 != NULL) {
        held = obj->unk60->children[0];
        if (held != NULL) {
            src = (f32 *) ((model->unk30[1] << 6) + inst->matrices[inst->matrixIndex]);
            dst = mtx;
            i = 0;
            while (TRUE) {
                *dst++ = *src++;
                if (++i >= 15) {
                    break;
                }
            }
            mtx[3] = 0.0f;
            mtx[7] = 0.0f;
            mtx[11] = 0.0f;
            mtx[15] = 1.0f;
            if (obj->behaviorId == 0x33 && model->unk30 != NULL && D_800A0BD8[0] != -1) {
                for (id = D_800A0BD8; *id != -1; id++) {
                    if (obj->unk4A == *id) {
                        mtx[12] = inst->point[0];
                        mtx[13] = inst->point[1];
                        mtx[14] = inst->point[2];
                        break;
                    }
                }
            }
            item = held->attachments[slot];
            if (item != NULL && item->point != NULL) {
                func_80048B60(mtx, item->point[0], item->point[1], item->point[2], x, y, z);
                result = 1;
            }
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_8000BC28);
#endif

extern u8 D_800A0A20;
extern s16 D_800A0838[]; // JFG: the frame list used for a model without animated textures
void func_800221F0(Object *obj, ObjectAttachment *inst, void *shade, f32 *mtx);
void obj_tex_animate(struct ObjectTexAnimState *obj, ObjectModel53 *model, struct ModelInstance53 *inst,
                     s32 updateRate);
void mtx_model_push(Gfx **dList, Mtx **mtx, f32 *in);
void mtx_pop(Gfx **dList);
void rendermode_reset(Gfx **dList);

#ifdef NON_MATCHING
/* JFG: draw the object's held item (slot as in func_8000BC28) with the matrix, lighting it and
   animating its textures first when it asks.
   NON_MATCHING: the right frame; obj->racer is reloaded (target: in s0, then reused for the item)
   and the temporaries are numbered differently. */
void func_8000BE5C(Object *obj, s32 arg1, s32 arg2, s32 slot, f32 *mtx) {
    Object *held;
    ObjectAttachment *item;
    Gfx *dl;
    s16 *texOffsets;

    if (D_800A0A20 == 0) {
        if (obj->behaviorId == 1) {
            Object_Racer *racer = obj->racer;

            if (racer->unk1FA != 0 && func_8003B740(obj, racer) < 2) {
                slot = 0;
            } else if (slot >= 9 || racer->unk53C > 0) {
                slot = 8;
            }
        }
        if (obj->unk60 != NULL) {
            held = obj->unk60->children[0];
            if (held != NULL) {
                item = held->attachments[slot];
                if (item != NULL) {
                    if (item->unkA != 0) {
                        func_800221F0(held, item, obj->shade, mtx);
                        if (item->model->hasTexAnims) {
                            obj_tex_animate((struct ObjectTexAnimState *) held, item->model,
                                            (struct ModelInstance53 *) item, item->unkA);
                        }
                        item->unkA = 0;
                    }
                    if (obj->opacity < 255) {
                        dl = item->model->dlTransparent;
                    } else {
                        dl = item->model->dl;
                    }
                    if (item->model->hasTexAnims) {
                        texOffsets = item->texOffsets[item->matrixIndex];
                    } else {
                        texOffsets = D_800A0838;
                    }
                    mtx_model_push(&gObjectCurrDisplayList, &gObjectCurrMatrix, mtx);
                    fast3d_cmd(gObjectCurrDisplayList++,
                               (((u32) item->matrices[item->matrixIndex] + 0x80000000) & 0xFFFFFF) | 0xBF000000,
                               (u32) item->vertices + 0x80000000);
                    fast3d_cmd(gObjectCurrDisplayList++, 0x02000050, (u32) texOffsets + 0x80000000);
                    fast3d_cmd(gObjectCurrDisplayList++, 0x06000000, (u32) dl + 0x80000000);
                    fast3d_cmd(gObjectCurrDisplayList++, 0xBF000000, 0);
                    mtx_pop(&gObjectCurrDisplayList);
                    rendermode_reset(&gObjectCurrDisplayList);
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_8000BE5C);
#endif

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C0B8);

f32 get_distance_to_camera(f32 x, f32 y, f32 z);
void func_80069BE8(Gfx **dList, void *mesh, s32 flags, u8 alpha);
extern u8 D_800A0C2C[];

#ifdef NON_MATCHING
/* NON_MATCHING: from the joint matrix statement on, every t-register is one lower than the target's */
/* JFG: draw a player's (up to four) effect meshes at their model points, farthest first, each with its joint
   matrix; then the module's extra effect (racer 0x550) if there is one. */
void func_8000C6E0(Gfx **dList, s32 arg1, Object *obj) {
    s16 i;
    s16 j;
    s16 count;
    s16 order[8];
    s16 alpha[8];
    u32 mtx[8];
    f32 depth[8];
    s32 eff;
    s32 effects[8];
    ObjectAttachment *model;
    Object_Racer *racer;
    f32 *point;

    racer = obj->racer;
    model = obj->attachments[0];
    for (i = 0, count = 0; i < 4; i++) {
        if (racer->effectAlpha[i] != 0 && (eff = racer->effects[i]) != 0) {
            point = (f32 *) ((racer->effectPoint[i] * 12) + (u32) model->point);
            order[count] = count;
            depth[count] = get_distance_to_camera(point[0], point[1], point[2]);
            effects[count] = eff;
            mtx[count] = (racer->effectMtx[i] << 6) + (u32) model->matrices[model->matrixIndex];
            alpha[count] = racer->effectAlpha[i];
            count++;
        }
    }
    if (count > 0) {
        for (i = count - 1; i > 0; i--) {
            for (j = 0; j < i; j++) {
                s16 a = order[j + 1];
                s16 b = order[j];
                if (depth[a] < depth[b]) {
                    order[j] = a;
                    order[j + 1] = b;
                }
            }
        }
        for (i = 0; i < count; i++) {
            fast3d_cmd((*dList)++, 0x01810040, mtx[order[i]] + 0x80000000);
            func_80069BE8(dList, (void *) effects[order[i]], 6, alpha[order[i]]);
        }
        fast3d_cmd((*dList)++, 0xBC00000A, 0);
    }
    if (racer->unk550 != 0) {
        func_80054530(dList, arg1, racer->unk550, &D_800A0C2C[racer->unk1 * 6], racer->unk544, (u8 *) racer + 0x78);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C6E0);
#endif

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

void get_fog_settings(s32 playerID, s16 *near, s16 *far, s16 *unk, u8 *r, u8 *g, u8 *b, u8 *unk2);
void func_80054530(); // a module function (patched in by the runtime linker)

/* JFG: set the fog for drawing a type 0x18 object (its flash colour while its player's bit is set,
   counting the flash down), or let a module do it for type 0x82. The fog factor word (0x500, 0x500)
   goes through the playerID parameter, which this branch no longer needs. */
void obj_flash_fog_set(Object *obj, s32 playerID) {
    s16 near;
    s16 far;
    s16 unk;
    u8 r;
    u8 g;
    u8 b;
    u8 unk2;

    get_fog_settings(0, &near, &far, &unk, &r, &g, &b, &unk2);
    if (obj->behaviorId == 0x18) {
        Object_Racer *racer = obj->racer;
        s32 bit = 1 << playerID;
        playerID = (0x500 << 16) | 0x500;
        if (racer->fogFlags & bit) {
            gDPSetFogColor(gObjectCurrDisplayList++, D_800A0C5C, D_800A0C60, D_800A0C64, 255);
            gMoveWd(gObjectCurrDisplayList++, G_MW_FOG, G_MWO_FOG, playerID);
            if (D_800A0C6C) {
                racer->fogFlags &= ~bit;
                if (D_800A0C68 != 0) {
                    D_800A0C68--;
                    if (D_800A0C68 == 0) {
                        D_800A0C5C = 255;
                        D_800A0C60 = 50;
                        D_800A0C64 = 50;
                    }
                }
            }
        } else {
            gDPSetFogColor(gObjectCurrDisplayList++, r, g, b, 255);
            gSPFogPosition(gObjectCurrDisplayList++, near, far);
        }
    } else if (obj->behaviorId == 0x82) {
        func_80054530(&gObjectCurrDisplayList, obj, playerID);
    }
}

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000CC64);

void render_object_parts(Object *obj);

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
    render_object_parts(obj);
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

void obj_render_scale_set(Object *obj) {
    f32 scale = 1.0f;
    if (obj->behaviorId == BHV_RACER) {
        Object_Racer *racer = obj->racer;
        scale = racer->unk48;
        racer->unk197 = 30;
    }
    D_800F2E98 = scale;
}

void obj_draw_twin(Object *obj);
void func_8000B46C(Object *obj);
void func_8000CC64(Object *obj);
void func_8000AA34(Object *obj);
void func_80054530(); // a module function (patched in by the runtime linker)

/**
 * Determine which model type the object is using, then call the related function to render it.
 * JFG: some object types are drawn by modules or their own functions; no undo afterwards.
 * DKR: render_object_parts
 */
void render_object_parts(Object *obj) {
    f32 y;

    obj_render_scale_set(obj);
    if (obj->behaviorId == 0x72) {
        func_80054530(&gObjectCurrDisplayList, &gObjectCurrMatrix, obj);
    } else if (obj->behaviorId == 0x4E) {
        func_80054530(&gObjectCurrDisplayList, &gObjectCurrMatrix, obj);
    } else if (obj->behaviorId == 0x81) {
        func_80054530(&gObjectCurrDisplayList, &gObjectCurrMatrix, &gObjectCurrVertexList, obj);
    } else if (obj->behaviorId == 7) {
        obj_draw_twin(obj);
    } else if (obj->behaviorId == 0x7A) {
        y = obj->trans.y_position;
        obj->trans.y_position += 40.0f;
        func_8000CC64(obj);
        obj->trans.y_position = y;
    } else {
        switch (obj->header->unk1E) {
            case 0:
                func_8000CC64(obj);
                break;
            case 1:
                func_8000B46C(obj);
                break;
            case 4:
                func_8000AA34(obj);
                break;
        }
    }
}

#ifdef NON_MATCHING
/* JFG: scroll the texture coordinates of the triangles using the header's scroll texture, in one
   attachment's model or all of them.
   NON_MATCHING: the same logic, other register allocation (target keeps obj in s0). */
void func_8000DF00(Object *obj, s32 updateRate) {
    ObjectHeader *header;
    s16 du, dv;
    s32 first, last;
    s32 texIndex;
    s32 i, j, k;
    ObjectModel53 *model;
    ModelTexture *tex;
    s16 maskU, maskV;
    Triangle *tri;
    s32 uv;
    s32 d1, d2;

    header = obj->header;
    du = header->scrollU * updateRate;
    dv = header->scrollV * updateRate;
    first = header->scrollModel;
    texIndex = header->scrollTexture;
    if (first == 0xFF || first < header->numAttachments) {
        last = first + 1;
        if (first == 0xFF) {
            last = header->numAttachments;
            first = 0;
        }
        for (i = first; i < last; i++) {
            model = obj->attachments[i]->model;
            if (texIndex < model->numberOfTextures) {
                tex = model->textures[texIndex].texture;
                maskU = tex->width << 5;
                maskV = tex->height << 5;
                for (j = 0; j < model->numberOfBatches; j++) {
                    if (texIndex == model->batches[j].textureIndex) {
                        for (k = model->batches[j].facesOffset; k < model->batches[j + 1].facesOffset; k++) {
                            tri = &model->triangles[k];
                            if (du != 0) {
                                uv = tri->uv[0][0];
                                d2 = tri->uv[2][0] - uv;
                                d1 = tri->uv[1][0] - uv;
                                tri->uv[0][0] = (uv + du) & (maskU - 1);
                                uv = tri->uv[0][0];
                                tri->uv[1][0] = uv + d1;
                                tri->uv[2][0] = uv + d2;
                            }
                            if (dv != 0) {
                                uv = tri->uv[0][1];
                                d2 = tri->uv[2][1] - uv;
                                d1 = tri->uv[1][1] - uv;
                                tri->uv[0][1] = (uv + dv) & (maskV - 1);
                                uv = tri->uv[0][1];
                                tri->uv[1][1] = uv + d1;
                                tri->uv[2][1] = uv + d2;
                            }
                        }
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_8000DF00);
#endif

/**
 * Sort the object list so the objects whose header has flag 1 come first; returns where the
 * others start (cached in D_800F2CC2).
 * JFG: no particle checks.
 * DKR: get_first_active_object
 */
s32 get_first_active_object(s32 *retObjCount) {
    s32 i, j;
    s32 minIndex, maxIndex;
    s32 breakLoop;

    *retObjCount = gObjectCount;
    if (D_800F2CC2 != 0) {
        // Already sorted
        return D_800F2CC2;
    }

    i = gObjectListStart;
    j = gObjectCount - 1;
    minIndex = i;
    maxIndex = j;

    while (i <= j) {
        breakLoop = 0;
        while (i <= maxIndex && breakLoop == 0) {
            if (gObjPtrList[i]->header->flags & 1) {
                i++;
            } else {
                breakLoop = -1;
            }
        }

        breakLoop = 0;
        while (j >= minIndex && breakLoop == 0) {
            if (!(gObjPtrList[j]->header->flags & 1)) {
                j--;
            } else {
                breakLoop = -1;
            }
        }

        if (i < j) {
            // Swap active and inactive objects
            Object *tempObject = gObjPtrList[i];
            gObjPtrList[i] = gObjPtrList[j];
            gObjPtrList[j] = tempObject;
            i++;
            j--;
        }
    }

    D_800F2CC2 = i;
    return i;
}

f32 get_distance_to_camera(f32 x, f32 y, f32 z);

/**
 * Sort the objects from first to last by their distance to the camera (nearest last).
 * JFG: every object's distance is the view depth, walked by pointer; each pass is one shorter.
 * DKR: sort_objects_by_dist
 */
void sort_objects_by_dist(s32 startIndex, s32 lastIndex) {
    s32 n;
    s32 i;
    s32 sorted;
    Object *obj;
    Object *next;
    Object **list;

    n = lastIndex - startIndex;
    if (n <= 0) {
        return;
    }
    list = &gObjPtrList[startIndex];
    i = lastIndex - startIndex + 1;
    while (i != 0) {
        obj = *list++;
        if (obj != NULL) {
            obj->distanceToCamera =
                -get_distance_to_camera(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        }
        i--;
    }
    do {
        i = n;
        list = &gObjPtrList[startIndex];
        obj = *list;
        n--;
        sorted = TRUE;
        do {
            next = list[1];
            if (next->distanceToCamera < obj->distanceToCamera) {
                list[0] = next;
                list[1] = obj;
                sorted = FALSE;
            } else {
                obj = next;
            }
            list++;
        } while (--i != 0);
    } while (!sorted && n != 0);
}

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

void obj_queue_add(void *obj) {
    D_800F2D18[D_800F2D1C] = obj;
    D_800F2D1C++;
}

extern s32 D_800F2D1C;
extern void **D_800F2D18;

void **obj_queue_get(s32 *count) {
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

/* JFG: an animation event (6 bytes) */
typedef struct AnimTrigger {
    /* 0x00 */ u16 animId;
    /* 0x02 */ u16 frame; // times D_800AB508: 0..1
    /* 0x04 */ u16 event;
} AnimTrigger;

/* JFG: the header's animation events (header->unkD4) */
typedef struct AnimTriggerList {
    /* 0x00 */ s16 pad0;
    /* 0x02 */ s16 count;
    /* 0x04 */ AnimTrigger events[1];
} AnimTriggerList;

/* JFG: what a type 0x33 object's block holds at 0x4C */
typedef struct ObjectKind53 {
    /* 0x00 */ u8 pad0[0x4C];
    /* 0x4C */ s16 kind;
} ObjectKind53;

extern f32 D_800AB508; // JFG: the event frame scale, in .rodata
void func_80027120(Object *obj, s32 event);
void func_800271A0(Object *obj, s32 event);
void func_80027220(Object *obj, s32 event);

/* JFG: send the object the events of its current animation whose frames it passed since the last
   frame (in either direction, wrapping), to a module (types 0x5A, 0x18) or one of three handlers
   picked by a type 0x33 object's kind. */
void obj_anim_triggers_fire(Object *obj) {
    AnimTriggerList *list;
    f32 frame;
    f32 prevFrame;
    f32 t;
    s32 i;
    s32 animId;
    s32 wrapped;
    AnimTrigger *event;
    ObjectAttachment *att;
    ObjectModel53 *model;
    ObjectKind53 *kindBlock;

    list = obj->header->unkD4;
    att = obj->attachments[obj->attachmentIndex];
    model = att->model;
    animId = model->animIds[obj->animIndex];
    frame = obj->animFrame;
    prevFrame = obj->prevAnimFrame;
    event = list->events;
    if (obj->animSpeed < 0.0f) {
        frame = 1.0f - frame;
        prevFrame = 1.0f - prevFrame;
    }
    if (frame < prevFrame) {
        frame += 1.0f;
        wrapped = TRUE;
    } else {
        wrapped = FALSE;
    }
    for (i = 0; i < list->count; i++, event++) {
        if (animId == event->animId) {
            t = event->frame * D_800AB508;
            if (wrapped && t <= prevFrame) {
                t += 1.0f;
            }
            if (prevFrame <= t && t <= frame && prevFrame != frame) {
                switch (obj->behaviorId) {
                    case 0x5A:
                        func_80054530(obj, event->event);
                        break;
                    case 0x18:
                        func_80054530(obj, event->event);
                        break;
                    case 0x33:
                        kindBlock = (ObjectKind53 *) obj->racer;
                        switch (kindBlock->kind) {
                            case 0xC5:
                            case 0x118:
                                func_80027120(obj, event->event);
                                break;
                            case 0xC6:
                            case 0x119:
                                func_800271A0(obj, event->event);
                                break;
                            case 0xC4:
                            case 0x11A:
                            case 0x1F6:
                            case 0x2AE:
                            case 0x2BA:
                                func_80027220(obj, event->event);
                                break;
                        }
                        break;
                }
            }
        }
    }
}

/* JFG: advance the animation frame by speed * rate; past either end it wraps (looping attachment) or stops there.
   Returns whether it passed an end. */
s32 func_8001138C(Object *obj, f32 speed, f32 rate) {
    ObjectAttachment *attachment;
    s32 result;

    attachment = obj->attachments[obj->attachmentIndex];
    obj->prevAnimFrame = obj->animFrame;
    obj->animFrame += speed * rate;
    result = 0;
    obj->animSpeed = speed;
    if (obj->animFrame >= 1.0f) {
        if (attachment->unk8) {
            while (obj->animFrame >= 1.0f) {
                obj->animFrame -= 1.0f;
            }
        } else {
            obj->animFrame = 1.0f;
        }
        result = 1;
    } else if (obj->animFrame < 0.0f) {
        if (attachment->unk8) {
            while (obj->animFrame < 0.0f) {
                obj->animFrame += 1.0f;
            }
        } else {
            obj->animFrame = 0.0f;
        }
        result = 1;
    }
    return result;
}

void asset_cache_release(void *arg0);

/* JFG: start animation anim (clamped to the model's) of the object's current attachment at frame
   0..1, keeping the previous animation's state. */
s32 obj_anim_set(Object *obj, s32 anim, f32 frame) {
    ObjectAttachment *att;
    ObjectModel53 *model;
    s8 *data;

    if (frame > 1.0f) {
        frame = 1.0f;
    } else if (frame < 0.0f) {
        frame = 0.0f;
    }
    obj->animFrame = frame;
    att = obj->attachments[obj->attachmentIndex];
    model = att->model;
    if (model->numAnims == 0) {
        return 0;
    }
    att->prevAnimIndex = att->animIndex;
    att->prevUnk28 = att->unk28;
    att->prevAnimLength = att->animLength;
    att->prevUnk30 = att->unk30;
    att->prevUnk40 = att->unk40;
    att->prevAnimData = att->animData;
    att->prevUnk8 = att->unk8;
    if (anim >= model->numAnims) {
        anim = model->numAnims - 1;
    }
    if (anim < 0) {
        anim = 0;
    }
    obj->animIndex = anim;
    att->animIndex = anim;
    if (model->unk64 == 0) {
        data = model->anims[att->animIndex];
        att->animData = (u8 *) (data + 8);
        att->unk8 = data[1] & 0xF0;
        att->animLength = att->animData[3];
        if (att->unk8 == 0) {
            att->animLength -= 1.0f;
        }
        if (data[1] & 0xF) {
            att->unk5C = 0x3FF / (data[1] & 0xF);
            att->unk5E = 0x3FF;
        }
        att->unk28 = att->animLength * frame;
    } else {
        if (att->unk84 != NULL) {
            asset_cache_release(att->unk84);
        }
        att->unk8A = att->unk88;
        att->unk84 = att->unk80;
        att->unk80 = NULL;
    }
    att->unk30 = 0.0f;
    att->unk40 = 0.0f;
    return 0;
}

#ifdef NON_MATCHING
/* JFG: a block with a level at 0x28 (func_800116BC) */
typedef struct LevelBlock28 {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ f32 level;
} LevelBlock28;

extern f32 D_800AB50C; // the highest level

/* JFG: set the block's level, clamped to 0..D_800AB50C.
   NON_MATCHING: 2 words: the limit is a float literal (needs this file's .rodata), loaded before the mtc1. */
s32 func_800116BC(LevelBlock28 *block, f32 level) {
    if (level > D_800AB50C) {
        level = D_800AB50C;
    } else if (level < 0.0f) {
        level = 0.0f;
    }
    block->level = level;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_800116BC);
#endif

/* JFG: the level object entry of a bonus item */
typedef struct LevelObjectEntryBonus {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
} LevelObjectEntryBonus;

extern s16 D_800A0CB0[16]; // JFG: the object type of each pickup kind 1..16

/* JFG: spawn a bonus item at the point: kinds 1..16 are pickups (D_800A0CB0), 20..39 are type 0xA9
   with a sub-kind (23 is type 0xE9), 40 and above nothing. */
void obj_spawn_bonus(f32 x, f32 y, f32 z, u8 kind) {
    LevelObjectEntryBonus entry;

    if (kind >= 40) {
        if (kind == 40) {
        }
    } else if (kind >= 20) {
        Object *obj;

        if (kind == 23) {
            entry.objectID = 0xE9;
            entry.unkA = 0;
        } else {
            entry.objectID = 0xA9;
            entry.unkA = kind - 20;
        }
        entry.unk3 = 1;
        entry.size = 12;
        entry.unkB = 0;
        entry.x = x;
        entry.y = y;
        entry.z = z;
        obj = spawn_object((LevelObjectEntry53 *) &entry, 1);
        if (obj != NULL) {
            obj->levelEntry = 0;
            if (obj->unk7C != NULL) {
                obj->unk20 = 1.0f;
                *obj->unk7C &= ~2;
            }
        }
    } else if (kind > 0) {
        Object *obj;

        kind = kind - 1; // not kind--: that narrows kind into a3 at the entry
        if (kind >= 16) {
            kind = 15;
        }
        entry.objectID = D_800A0CB0[kind];
        entry.size = 12;
        entry.unkA = 0;
        entry.unkB = 0;
        entry.x = x;
        entry.y = y;
        entry.z = z;
        obj = spawn_object((LevelObjectEntry53 *) &entry, 1);
        if (obj != NULL) {
            obj->levelEntry = 0;
        }
    }
}

extern s32 D_800F2CD8[];
extern s32 D_800F2CE0[];

void func_800118F4(s32 *a, s32 *b, s32 index) {
    *a = D_800F2CD8[index];
    *b = D_800F2CE0[index];
}

/* JFG: link obj into the list after its head, unless it is already in it (0x8001191C). */
void obj_list_insert_after(Object *obj, Object *list) {
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

/* Unlinks an object from its list (0x80011974). */
void obj_list_unlink(Object *obj) {
    if (obj->next != NULL) {
        obj->next->prev = obj->prev;
    }
    if (obj->prev != NULL) {
        obj->prev->next = obj->next;
    }
    obj->next = NULL;
    obj->prev = NULL;
}

/* JFG: the blocks of the object types that keep a pointer to another object, reduced */
typedef struct ObjectLinks53 {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ Object *target; // type 0x99
    /* 0x08 */ s16 unk8;       // types 0x9B, 0xA1
    /* 0x0A */ u8 padA[0xD - 0xA];
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 padE[0x1A - 0xE];
    /* 0x1A */ u8 unk1A; // type 0x18
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 pad1C[0x23 - 0x1C];
    /* 0x23 */ u8 unk23; // type 0x25
    /* 0x24 */ Object *unk24; // types 0x9E, 0x18
    /* 0x28 */ u8 pad28[0x2C - 0x28];
    /* 0x2C */ u16 kind;
} ObjectLinks53;

void troop_members_clear_tag(Object *obj);
void troop_member_clear_link(Object *obj);

/* JFG: take the object out of its list, and out of the list following it every object that points
   at it (each type dropping its pointer; a type 0x99 then joins the list of the nearest type 0x18). */
void obj_unlink_dependants(Object *obj) {
    ObjectLinks53 *links;
    s32 wasFirst;
    Object *prev;
    Object *next;
    Object *cur;
    s32 found;
    ObjectLinks53 *block;
    ObjectLinks53 *other;

    if (obj->behaviorId == 1 || obj->behaviorId == 0x18 || obj->behaviorId == 0x4D || obj->behaviorId == 0x40) {
        troop_members_clear_tag(obj);
    }
    if (obj->behaviorId == 0x9B || obj->behaviorId == 0xA1) {
        troop_member_clear_link(obj);
    }
    prev = obj->prev;
    next = obj->next;
    wasFirst = prev == NULL;
    if (prev != NULL) {
        prev->next = next;
    }
    if (next != NULL) {
        next->prev = prev;
    }
    obj->prev = NULL;
    obj->next = NULL;
    while (next != NULL) {
        cur = next;
        found = FALSE;
        switch (cur->behaviorId) {
            case 0x99:
                links = (ObjectLinks53 *) cur->racer;
                if (obj == links->target) {
                    links->target = NULL;
                    found = TRUE;
                }
                break;
            case 0x9E:
                block = (ObjectLinks53 *) cur->racer;
                if (obj == block->unk24) {
                    block->unk24 = NULL;
                    found = TRUE;
                }
                break;
            case 0x9B:
            case 0xA1:
                if (wasFirst) {
                    block = (ObjectLinks53 *) cur->racer;
                    found = TRUE;
                    block->unk8 = 0;
                    block->unkD = 0;
                }
                break;
            case 0x25:
                block = (ObjectLinks53 *) cur->racer;
                found = TRUE;
                block->unk23 = 1;
                break;
            case 0x18:
                block = (ObjectLinks53 *) cur->racer;
                other = (ObjectLinks53 *) block->unk24->racer;
                if (other->kind == 0x112) {
                    func_80054530(cur); // one argument: a1 still holds found (0) in the ROM
                    found = TRUE;
                } else if (other->kind == 0xE1) {
                    found = TRUE;
                    if (block->unk1A == 0x14) {
                        block->unk1B = 5;
                    }
                }
                break;
        }
        prev = cur->prev;
        next = cur->next;
        if (found) {
            cur->prev = NULL;
            cur->next = NULL;
            if (prev != NULL) {
                prev->next = next;
            }
            if (next != NULL) {
                next->prev = prev;
            }
            if (cur->behaviorId == 0x99) {
                links = (ObjectLinks53 *) cur->racer;
                D_800A0834 = obj;
                links->target = obj_find_nearest_of_type(cur->trans.x_position, cur->trans.y_position, cur->trans.z_position, 0x18);
                D_800A0834 = NULL;
                if (links->target != NULL) {
                    obj_list_insert_after(cur, links->target);
                }
            }
        }
    }
}

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

/* JFG: the distance between two points (0x80011C84). */
f32 point_distance(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
    f32 dx = x1 - x2;
    f32 dy = y1 - y2;
    f32 dz = z1 - z2;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

/* JFG: the squared distance between two points (0x80011CE0). */
f32 point_distance_squared(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2) {
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
void entry_list_remove(s32 entry) {
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

/* JFG: a camera, reduced (0x4C bytes) */
typedef struct ObjectCamera53 {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x_position;
    /* 0x10 */ f32 y_position;
    /* 0x14 */ f32 z_position;
    /* 0x18 */ u8 pad18[0x4C - 0x18];
} ObjectCamera53;

/* JFG: the block of a type 0xA7 object, reduced */
typedef struct ObjectSoundSource53 {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0xAC */ u8 active;
} ObjectSoundSource53;

extern void *D_800A089C; // JFG: the sound of the nearest type 0xA7 object
extern f32 D_800AB510;   // JFG: the search's starting distance, in .rodata
ObjectCamera53 *cam_get_cameras(void);
u8 func_80048548(void);
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, void **handlePtr);
void audspat_point_set_position(void *audioPoint, f32 x, f32 y, f32 z);
void audspat_point_stop(void *handle);

/* JFG: keep sound 0xFB playing at the active type 0xA7 object nearest to any camera (stop it when
   there is none). */
void sound_nearest_emitter_update(void) {
    ObjectCamera53 *cams;
    s32 numCams;
    Object **list;
    s32 i;
    s32 start;
    s32 end;
    s32 j;
    Object *obj;
    Object *nearest;
    f32 best;
    f32 dx;
    f32 dy;
    f32 dz;
    ObjectSoundSource53 *src;

    best = D_800AB510;
    nearest = NULL;
    cams = cam_get_cameras();
    numCams = func_80048548();
    list = objGetObjList(&start, &end);
    for (i = start; i < end; i++) {
        obj = list[i];
        if (obj->behaviorId == 0xA7 && (src = (ObjectSoundSource53 *) obj->racer)->active) {
            for (j = 0; j < numCams; j++) {
                dx = obj->trans.x_position - cams[j].x_position;
                dy = obj->trans.y_position - cams[j].y_position;
                dz = obj->trans.z_position - cams[j].z_position;
                if (dx * dx + dy * dy + dz * dz < best || nearest == NULL) {
                    best = dx * dx + dy * dy + dz * dz;
                    nearest = obj;
                }
            }
        }
    }
    if (nearest != NULL) {
        if (D_800A089C == NULL) {
            audspat_play_sound_at_position(0xFB, nearest->trans.x_position, nearest->trans.y_position,
                                           nearest->trans.z_position, 1, &D_800A089C);
        } else {
            audspat_point_set_position(D_800A089C, nearest->trans.x_position, nearest->trans.y_position,
                                       nearest->trans.z_position);
        }
    } else if (D_800A089C != NULL) {
        audspat_point_stop(D_800A089C);
    }
}
