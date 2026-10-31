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
    /* 0x068 */ u8 pad68[0xA4 - 0x68];
    /* 0x0A4 */ u16 fogFlags; // JFG: a bit per player: draw with the flash fog colour
    /* 0x0A6 */ u8 padA6[0x140 - 0xA6];
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

/* DKR include/structs.h */
typedef struct ObjHeaderParticleEntry {
    /* 0x00 */ s32 upper;
    /* 0x04 */ s32 lower;
} ObjHeaderParticleEntry;

/* JFG: the object header, reduced */
typedef struct ObjectHeader {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ u16 flags;
    /* 0x16 */ u8 pad16[0x1E - 0x16];
    /* 0x1E */ s8 unk1E;
    /* 0x1F */ s8 numAttachments;
    /* 0x20 */ u8 pad20[2];
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
    /* 0x04 */ u8 pad4[0x8 - 0x4];
    /* 0x08 */ s8 unk8;
    /* 0x09 */ u8 pad9;
    /* 0x0A */ s8 unkA;
} ObjectAttachment;

typedef struct Object {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ u8 pad18[0x28 - 0x18];
    /* 0x28 */ f32 animFrame;
    /* 0x2C */ u8 pad2C[0x2E - 0x2C];
    /* 0x2E */ s16 segmentID;
    /* 0x30 */ u8 pad30[0x39 - 0x30];
    /* 0x39 */ u8 opacity;
    /* 0x3A */ s8 attachmentIndex;
    /* 0x3B */ u8 pad3B;
    /* 0x3C */ s32 levelEntry; // JFG: the level object entry it was spawned from

    /* 0x40 */ struct ObjectHeader *header;
    /* 0x44 */ u8 pad44[0x48 - 0x44];
    /* 0x48 */ s16 behaviorId;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ u8 pad4C[0x50 - 0x4C];
    /* 0x50 */ struct ObjectSprite *sprite;
    /* 0x54 */ u8 pad54[0x5C - 0x54];
    /* 0x5C */ ObjectUnk5C *unk5C;
    /* 0x60 */ u8 pad60[0x68 - 0x60];
    /* 0x68 */ Object_Racer *racer;
    /* 0x6C */ struct ObjectAttachment **attachments; // JFG
    /* 0x70 */ struct ParticleEmitter *particleEmitter;
    /* 0x74 */ u8 pad74[0x7C - 0x74];
    /* 0x7C */ u8 *unk7C;
    /* 0x80 */ u8 pad80[0xA0 - 0x80];
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

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004954);

INCLUDE_ASM("asm/nonmatchings/53C0", func_80004AF4);

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
void func_8003F0E4(void *arg0);

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
                func_8003F0E4(header->unkD4);
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

#ifdef NON_MATCHING
/* JFG: whether an object type is spawned by func_8005131C rather than spawn_object.
   NON_MATCHING: the result goes through v1 and the branch delay slots are filled (target: v0, nops). */
#define IS_TYPE(x) (ret = (objectID == (x)))
s32 func_800050A8(s32 objectID) {
    s32 ret;

    IS_TYPE(0x29) || IS_TYPE(0x2A) || IS_TYPE(0x2B) || IS_TYPE(0x2D) || IS_TYPE(0x54) || IS_TYPE(0x149) ||
        IS_TYPE(0x150) || IS_TYPE(0x151) || IS_TYPE(0x152) || IS_TYPE(0x153) || IS_TYPE(0x166) || IS_TYPE(0x5F) ||
        IS_TYPE(0x67) || IS_TYPE(0xDF) || IS_TYPE(0xE0) || IS_TYPE(0xE1) || IS_TYPE(0x78) || IS_TYPE(0x8C) ||
        IS_TYPE(0x83) || IS_TYPE(0x66) || IS_TYPE(0x11C) || IS_TYPE(0x11D) || IS_TYPE(0x11E) || IS_TYPE(0x11F) ||
        IS_TYPE(0x120) || IS_TYPE(0x70) || IS_TYPE(0x90) || IS_TYPE(0xA5) || IS_TYPE(0x97) || IS_TYPE(0x157) ||
        IS_TYPE(0x68) || IS_TYPE(0x6D) || IS_TYPE(0xBE) || IS_TYPE(0xBD) || IS_TYPE(0x269) || IS_TYPE(0xEE) ||
        IS_TYPE(0xEF) || IS_TYPE(0x8D) || IS_TYPE(0x99) || IS_TYPE(0x9C) || IS_TYPE(0x12F) || IS_TYPE(0xE6) ||
        IS_TYPE(0xF2) || IS_TYPE(0xF3) || IS_TYPE(0x121) || IS_TYPE(0xF1) || IS_TYPE(0xF5) || IS_TYPE(0xF6) ||
        IS_TYPE(0x122) || IS_TYPE(0x143) || IS_TYPE(0x144) || IS_TYPE(0x112) || IS_TYPE(0x1E5);
    return ret;
}
#undef IS_TYPE
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_800050A8);
#endif

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
s32 func_800050A8(s32 objectID);
void func_8005131C(LevelObjectEntry53 *entry);
void spawn_level_replacement(LevelObjectEntry53 *entry);
void func_8005269C(s32 list, s32 size);
Object *spawn_object(LevelObjectEntry53 *entry, s32 arg1);
void *mempool_alloc_safe(s32 size, u32 colourTag);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

#ifdef NON_MATCHING
/**
 * Load the object map into RAM, then start spawning objects into the world.
 * JFG: uncompressed maps, two lists, some objects spawned by other functions; no silver coins.
 * DKR: track_spawn_objects
 * NON_MATCHING: 10 words: two address temporaries spill one word lower, the mem spill is scheduled
 * earlier and the list compare takes its registers the other way round.
 */
void func_800053F8(s32 mapID, s32 index) {
    s32 assetSize;
    UNUSED s32 pad; // DKR: settings
    s32 i;
    s32 *mem;
    s32 var_s0;
    u32 assetOffset;
    s32 *objMapTable;
    s32 temp_t3;

    mem = mempool_alloc_safe(0x3000, 0xFFFF);
    D_800F2CF0[index] = mem;
    D_800F2CD8[index] = (s32) mem + 0x10;
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
        if (&D_800F2CE8[index] == &D_800F2CE8[1]) {
            func_8005269C(D_800F2CD8[index], D_800F2CE0[index]);
        }
        for (var_s0 = 0; var_s0 < D_800F2CE0[index]; var_s0 += temp_t3) {
            if (func_800050A8(((LevelObjectEntry53 *) D_800F2CD8[index])->objectID)) {
                func_8005131C((LevelObjectEntry53 *) D_800F2CD8[index]);
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
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_800053F8);
#endif

INCLUDE_ASM("asm/nonmatchings/53C0", func_8000561C);

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
                level_entry_remove((LevelObjectEntry53 *) entry);
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
s16 func_80005EC0(s32 type) {
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

INCLUDE_ASM("asm/nonmatchings/53C0", func_80006F2C);

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

/* JFG: an object's sprite block (func_80007420) */
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

#ifdef NON_MATCHING
/* JFG: set up the object's sprite block from its header and load its texture; returns the block's size (0 on failure).
   NON_MATCHING: the flags copy and the zero stores are scheduled differently (same instructions). */
s32 func_80007420(Object *obj, ObjectSprite *sprite) {
    ObjectHeader *header;
    s32 flags;
    s32 size;

    header = obj->header;
    obj->sprite = sprite;
    sprite->unk0 = header->unk54;
    sprite->unkC = header->unk61;
    flags = sprite->unkC;
    sprite->unkD = header->unk62;
    sprite->unkF = 0;
    sprite->unk8 = 0;
    sprite->unkE = header->unk63;
    sprite->unk1C = 0;
    sprite->unkA = header->unk66;
    size = 0x20;
    if (D_800A4FC4 == 0) {
        if (flags & 8) {
            sprite->unk1C = (((s32) sprite + 0x20) & ~3) + 4;
            size = sprite->unk1C - (s32) sprite + 0xBC;
        } else if (flags & 0x10) {
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
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_80007420);
#endif

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

extern s32 D_800F2C8C;
extern f32 D_800F2C88;
extern s32 D_800F6F54;
void func_80026A50(Object *obj, s32 updateRate);
void func_80002AB8(Object **racers, s32 numRacers, s32 updateRate);
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
                func_80026A50(obj, updateRate);
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
    func_80002AB8(*gRacers, gNumRacers, updateRate);
    if (D_800F2C8C == 0) {
        gParticlePtrList_flush();
    }
    D_800F2C8C = 1;
    D_800F2C88 = updateRate;
    D_800F6F54 = 0;
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

/* JFG: an animated texture of an object model (8 bytes) */
typedef struct ModelTexAnim {
    /* 0x00 */ s16 offset;
    /* 0x02 */ s16 nextOffset; // < 0: none
    /* 0x04 */ u32 flags; // the texture index in the low byte
} ModelTexAnim;

/* DKR include/structs.h TriangleBatchInfo, reduced (Size 0x10) */
typedef struct ModelBatch {
    /* 0x00 */ u8 textureIndex;
    /* 0x01 */ u8 pad1[7];
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA[6];
} ModelBatch;

typedef struct ObjectModel53 {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numberOfTextures;
    /* 0x11 */ u8 pad11[0x16 - 0x11];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ ModelTextureEntry *textures;
    /* 0x1C */ u8 pad1C[0x20 - 0x1C];
    /* 0x20 */ Triangle *triangles;
    /* 0x24 */ ModelBatch *batches;
    /* 0x28 */ u8 pad28[0x2C - 0x28];
    /* 0x2C */ u8 numTexAnims;
} ObjectModel53;

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

void get_fog_settings(s32 playerID, s16 *near, s16 *far, s16 *unk, u8 *r, u8 *g, u8 *b, u8 *unk2);
void func_80054530(); // a module function (patched in by the runtime linker)

#ifdef NON_MATCHING
/* JFG: set the fog for drawing a type 0x18 object (its flash colour while its player's bit is set,
   counting the flash down), or let a module do it for type 0x82.
   NON_MATCHING: the frame is 8 bytes bigger (racer and bit take stack words; without them the
   registers differ). */
void func_8000C9F4(Object *obj, s32 playerID) {
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
        if (racer->fogFlags & bit) {
            gDPSetFogColor(gObjectCurrDisplayList++, D_800A0C5C, D_800A0C60, D_800A0C64, 255);
            gSPFogFactor(gObjectCurrDisplayList++, 0x500, 0x500);
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
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_8000C9F4);
#endif

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

void func_8000DD78(Object *obj) {
    f32 scale = 1.0f;
    if (obj->behaviorId == BHV_RACER) {
        Object_Racer *racer = obj->racer;
        scale = racer->unk48;
        racer->unk197 = 30;
    }
    D_800F2E98 = scale;
}

void func_8000B300(Object *obj);
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

    func_8000DD78(obj);
    if (obj->behaviorId == 0x72) {
        func_80054530(&gObjectCurrDisplayList, &gObjectCurrMatrix, obj);
    } else if (obj->behaviorId == 0x4E) {
        func_80054530(&gObjectCurrDisplayList, &gObjectCurrMatrix, obj);
    } else if (obj->behaviorId == 0x81) {
        func_80054530(&gObjectCurrDisplayList, &gObjectCurrMatrix, &gObjectCurrVertexList, obj);
    } else if (obj->behaviorId == 7) {
        func_8000B300(obj);
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

#ifdef NON_MATCHING
/* JFG: advance the animation frame by speed * rate; past either end it wraps (looping attachment) or stops there. Returns whether it passed an end.
   NON_MATCHING: 7 words: the target shares one return block at the end (here it is duplicated). */
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
        return 1;
    }
    if (obj->animFrame < 0.0f) {
        result = 1;
        if (attachment->unk8) {
            while (obj->animFrame < 0.0f) {
                obj->animFrame += 1.0f;
            }
        } else {
            obj->animFrame = 0.0f;
        }
    }
    return result;
}
#else
INCLUDE_ASM("asm/nonmatchings/53C0", func_8001138C);
#endif

INCLUDE_ASM("asm/nonmatchings/53C0", func_8001149C);

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

INCLUDE_ASM("asm/nonmatchings/53C0", func_80011708);

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
