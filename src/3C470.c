#include "common.h"

/* Copies a block as halfwords (size in bytes, rounded up) (0x8003B870). */
void mem_copy_u16(u16 *src, u16 *dst, s32 size) {
    size = (size + 1) >> 1;
    while (size--) {
        *dst++ = *src++;
    }
}

#define COLOUR_TAG_GREEN 0x00FF00FF

void *mempool_alloc_safe(s32 size, u32 colourTag);
u32 *asset_table_load(u32 assetIndex);
extern s32 *D_800F6F10; // JFG: the model table (asset 0x26)
extern s32 *D_800F6F14; // JFG: the model cache, (id, model) pairs
extern s32 *D_800F6F18; // JFG: the free model cache slots
extern s32 D_800F6F1C;
extern s32 D_800F6F20; // JFG: the number of models in the table
extern s32 D_800F6F24;
extern u8 *D_800F6F28;
extern u8 *D_800F6F2C;
extern u8 *D_800F6F34;
extern s32 *D_800F6F38;
extern s32 *D_800F6F30;
extern s32 *D_800F6F3C;
extern s32 D_800F6F40;
extern s32 D_800F6F58;

/* JFG: allocate the model caches and buffers and count the model table's entries (DKR
 * allocate_object_model_pools). */
void allocate_object_model_pools(void) {
    u8 *buf;

    D_800F6F14 = mempool_alloc_safe(0x2A8, COLOUR_TAG_GREEN);
    D_800F6F18 = mempool_alloc_safe(0x190, COLOUR_TAG_GREEN);
    D_800F6F1C = 0;
    D_800F6F24 = 0;
    D_800F6F58 = (s32) mempool_alloc_safe(0x2000, COLOUR_TAG_GREEN);
    D_800F6F10 = (s32 *) asset_table_load(0x26);
    D_800F6F20 = 0;
    while (D_800F6F10[D_800F6F20] != -1) {
        D_800F6F20++;
    }
    D_800F6F20--;
    buf = mempool_alloc_safe(0xA0, COLOUR_TAG_GREEN);
    D_800F6F28 = buf;
    D_800F6F2C = buf + 0x80;
    D_800F6F38 = (s32 *) D_800F6F2C;
    D_800F6F34 = buf + 0x90;
    D_800F6F30 = mempool_alloc_safe(0x800, COLOUR_TAG_GREEN);
    D_800F6F3C = mempool_alloc_safe(0x100, COLOUR_TAG_GREEN);
    D_800F6F40 = 0;
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003B9E8);

/* JFG: set count matrices to the identity (0x8003C0C4). */
void mtxf_set_identity_array(s32 count, MtxF *mtx) {
    while (count > 0) {
        count--;
        (*mtx)[0][0] = 1.0f;
        (*mtx)[0][1] = 0.0f;
        (*mtx)[0][2] = 0.0f;
        (*mtx)[0][3] = 0.0f;
        (*mtx)[1][0] = 0.0f;
        (*mtx)[1][1] = 1.0f;
        (*mtx)[1][2] = 0.0f;
        (*mtx)[1][3] = 0.0f;
        (*mtx)[2][0] = 0.0f;
        (*mtx)[2][1] = 0.0f;
        (*mtx)[2][2] = 1.0f;
        (*mtx)[2][3] = 0.0f;
        (*mtx)[3][0] = 0.0f;
        (*mtx)[3][1] = 0.0f;
        (*mtx)[3][2] = 0.0f;
        (*mtx)[3][3] = 1.0f;
        mtx++;
    }
}

/* JFG: an asset with a table of 10-byte entries */
typedef struct Asset3C {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ s16 count;
    /* 0x14 */ u8 pad14[8];
    /* 0x1C */ u16 *entries;
} Asset3C;

/* JFG: an instance of it with its own copy of the table */
typedef struct Instance3C {
    /* 0x00 */ Asset3C *asset;
    /* 0x04 */ u16 *entries;
    /* 0x08 */ u8 pad8[2];
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s32 unkC;
} Instance3C;

void *mempool_alloc(s32 size, u32 colourTag);

/* JFG: make an instance of the asset (the table follows the header). */
Instance3C *func_8003C12C(Asset3C *asset) {
    u16 *entries;
    Instance3C *inst;

    inst = mempool_alloc(asset->count * 10 + sizeof(Instance3C), 0xFFFF);
    if (inst != NULL) {
        entries = (u16 *) (inst + 1);
        inst->asset = asset;
        inst->entries = entries;
        inst->unkA = 2;
        inst->unkB = 0;
        inst->unkC = 0;
        mem_copy_u16(asset->entries, entries, asset->count * 10);
    }
    return inst;
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C1B4);

/* JFG: a loaded model (only the fields used here) */
typedef struct ModelTexture3C {
    /* 0x00 */ void *texture;
    /* 0x04 */ s32 unk4;
} ModelTexture3C;

typedef struct Model3C7 {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numberOfTextures;
    /* 0x11 */ u8 pad11[0x18 - 0x11];
    /* 0x18 */ ModelTexture3C *textures;
    /* 0x1C */ u8 pad1C[0x28 - 0x1C];
    /* 0x28 */ void *unk28;
    /* 0x2C */ u8 pad2C[0x4C - 0x2C];
    /* 0x4C */ s16 references;
    /* 0x4E */ s8 numAssets;
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ s8 **assets; // released with func_8003D150
    /* 0x54 */ u8 pad54[4];
    /* 0x58 */ void *unk58;
    /* 0x5C */ void *unk5C;
    /* 0x60 */ void *unk60;
    /* 0x64 */ u8 unk64;
    /* 0x65 */ u8 pad65[0x74 - 0x65];
    /* 0x74 */ void *unk74;
    /* 0x78 */ void *unk78;
    /* 0x7C */ s32 numBuffers; // the last index of buffers
    /* 0x80 */ u8 pad80[4];
    /* 0x84 */ void **buffers;
} Model3C7;

/* JFG: an instance of a model */
typedef struct ModelInstance3C {
    /* 0x00 */ Model3C7 *model;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s8 animFlags;
    /* 0x09 */ s8 prevAnimFlags;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ void *unkC;
    /* 0x10 */ u8 pad10[0x24 - 0x10];
    /* 0x24 */ u16 animIndex;
    /* 0x26 */ u16 prevAnimIndex;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ f32 unk30;
    /* 0x34 */ f32 unk34;
    /* 0x38 */ f32 animLength;
    /* 0x3C */ f32 prevAnimLength;
    /* 0x40 */ f32 unk40;
    /* 0x44 */ f32 unk44;
    /* 0x48 */ u8 pad48[0x5C - 0x48];
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 unk5E;
    /* 0x60 */ u8 *anim;
    /* 0x64 */ u8 *prevAnim;
    /* 0x68 */ u8 pad68[0x80 - 0x68];
    /* 0x80 */ s8 *assets[2]; // released with func_8003D150
    /* 0x88 */ s16 assetIds[2];
} ModelInstance3C;

void free_model_data(Model3C7 *model);
void func_8003D150(s8 *asset);
void mempool_free(void *data);

/* JFG: free a model instance; free the model itself when it has no references left (DKR free_3d_model). */
void free_3d_model(ModelInstance3C *modInst) {
    s32 modelIndex;
    Model3C7 *model;
    s32 i;

    if (modInst != NULL) {
        model = modInst->model;
        if (modInst->unkC != NULL) {
            mempool_free(modInst->unkC);
        }
        if (model->unk64 != 0) {
            for (i = 0; i < 2; i++) {
                if (modInst->assets[i] != NULL) {
                    func_8003D150(modInst->assets[i]);
                }
            }
        }
        mempool_free(modInst);
        model->references--;
        if (model->references <= 0) {
            for (i = 0, modelIndex = -1; i < D_800F6F1C; i++) {
                if ((s32) model == D_800F6F14[(i << 1) + 1]) {
                    modelIndex = i;
                }
            }
            if (modelIndex != -1) {
                free_model_data(model);
                D_800F6F18[D_800F6F24] = modelIndex;
                D_800F6F24++;
                D_800F6F14[(modelIndex << 1) + 0] = -1;
                D_800F6F14[(modelIndex << 1) + 1] = -1;
            }
        }
    }
}

void tex_free(void *texture);

/* JFG: free a model's textures, buffers and the model itself (DKR free_model_data). */
void free_model_data(Model3C7 *mdl) {
    s32 numTextures;
    s32 i;
    s32 j;

    numTextures = mdl->numberOfTextures;
    if (numTextures > 0) {
        i = 0;
        j = 0;
        do {
            if (mdl->textures[j].texture != NULL) {
                tex_free(mdl->textures[j].texture);
                numTextures = mdl->numberOfTextures;
            }
            i++;
            j++;
        } while (i < numTextures);
    }
    if (mdl->unk60 != NULL) {
        mempool_free(mdl->unk60);
    }
    if (mdl->unk28 != NULL) {
        mempool_free(mdl->unk28);
    }
    if (mdl->unk74 != NULL) {
        mempool_free(mdl->unk74);
    }
    if (mdl->unk78 != NULL) {
        mempool_free(mdl->unk78);
    }
    if (mdl->numAssets != 0) {
        if (mdl->unk64 == 0 && mdl->assets != NULL) {
            i = 0;
            j = 0;
            do {
                func_8003D150(mdl->assets[j]);
                i++;
                j++;
            } while (i < mdl->numAssets);
            mempool_free(mdl->assets);
        }
        if (mdl->unk5C != NULL) {
            mempool_free(mdl->unk5C);
        }
        mempool_free(mdl->unk58);
    }
    if (mdl->buffers != NULL) {
        i = mdl->numBuffers + 1;
        while (i--) {
            mempool_free(mdl->buffers[i]);
        }
        mempool_free(mdl->buffers);
    }
    mempool_free(mdl);
}

s8 *asset_cache_get(s16 id);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);
void *func_8003CF24(s32 index);

/* JFG: load model `id`'s asset list (tables 0x28/0x29) into the model: its 0x58 block, the ids (0x5C) and, unless
 * 0x64 is set, the cached assets (0x50); returns 1 when something could not be had (all freed again). */
#ifdef NON_MATCHING
/* NON_MATCHING: work in progress */
s32 func_8003CB04(Model3C7 *model, s32 id) {
    s32 start;
    s32 end;
    s32 offset;
    s32 size;
    s32 i;
    s32 j;

    asset_load(0x28, (u32) D_800F6F2C, (id & ~3) * 2, 0x10);
    start = ((u16 *) D_800F6F2C)[id & 3] >> 1;
    end = ((u16 *) D_800F6F2C)[(id & 3) + 1] >> 1;
    model->numAssets = end - start;
    if (start == end) {
        return 0;
    }
    offset = start & 3;
    size = ((model->numAssets & ~3) + 4) * 2;
    if (offset) {
        size += 8;
    }
    asset_load(0x29, (u32) D_800F6F28, (start & ~3) * 2, size);
    if ((model->unk58 = func_8003CF24(id)) == NULL) {
        return 1;
    }
    if (!model->unk64) {
        model->assets = mempool_alloc(model->numAssets * 4, 0xFF0000FF);
    }
    model->unk5C = mempool_alloc(model->numAssets * 2, 0xFF0000FF);
    if (!model->unk64) {
        if (model->assets == NULL) {
            mempool_free(model->unk58);
            return 1;
        }
        i = 0;
        do {
            model->assets[i] = asset_cache_get(((s16 *) D_800F6F28)[offset + i]);
            if (model->assets[i] == NULL) {
                for (j = 0; j < i; j++) {
                    func_8003D150(model->assets[j]);
                }
                mempool_free(model->assets);
                mempool_free(model->unk58);
                mempool_free(model->unk5C);
                model->assets = NULL;
                return 1;
            }
            ((s16 *) model->unk5C)[i] = ((s16 *) D_800F6F28)[offset + i];
            i++;
        } while (i < model->numAssets);
        return 0;
    }
    for (i = 0; i < model->numAssets; i++) {
        ((s16 *) model->unk5C)[i] = ((s16 *) D_800F6F28)[offset + i];
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CB04);
#endif

#ifdef NON_MATCHING
/* NON_MATCHING: IDO copies the model pointer to v0 for the else arm (target keeps v1) */
/* JFG: reset a model instance's animation state and select its first animation (shared assets are fetched
 * through the cache when the model has unk64 set). */
void func_8003CDAC(ModelInstance3C *inst) {
    Model3C7 *model = inst->model;
    s8 *anim;

    inst->animIndex = 0;
    inst->unk5C = 0;
    inst->unk5E = 0;
    inst->animFlags = 0;
    inst->unk30 = 0.0f;
    inst->unk28 = 0.0f;
    inst->unk40 = 0.0f;
    inst->animLength = 0.0f;
    if (model->numAssets != 0) {
        if (model->unk64 != 0) {
            inst->assets[0] = asset_cache_get(*(s16 *) model->unk5C);
            inst->assets[1] = asset_cache_get(*(s16 *) inst->model->unk5C);
            model = inst->model;
            anim = inst->assets[0];
            inst->assetIds[0] = *(s16 *) model->unk5C;
            inst->assetIds[1] = *(s16 *) model->unk5C;
        } else {
            anim = model->assets[inst->animIndex];
        }
        inst->anim = (u8 *) anim + 8;
        inst->animFlags = anim[1] & 0xF0;
        inst->animLength = inst->anim[3];
        if (inst->animFlags == 0) {
            inst->animLength -= 1.0f;
        }
        inst->prevAnimFlags = inst->animFlags;
        inst->prevAnim = inst->anim;
        inst->prevAnimIndex = inst->animIndex;
        inst->unk2C = inst->unk28;
        inst->prevAnimLength = inst->animLength;
        inst->unk44 = inst->unk40;
        inst->unk34 = inst->unk30;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CDAC);
#endif

extern s32 *D_800F6F38; // JFG: a buffer for two entries of an offset table
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/* JFG: load an entry of asset 0x2D, found by its offset in table 0x2C. */
void *func_8003CF24(s32 index) {
    s32 start;
    s32 end;
    s32 *offsets;
    s32 size;
    void *data;

    asset_load(0x2C, (u32) D_800F6F38, (index & ~1) * 4, 0x10);
    offsets = (s32 *) ((u32) D_800F6F38 + (index & 1) * 4);
    start = offsets[0];
    end = offsets[1];
    size = end - start;
    data = mempool_alloc(size, 0xFF0000FF);
    if (data == NULL) {
        return NULL;
    }
    asset_load(0x2D, (u32) data, start, size);
    return data;
}

extern s32 *D_800F6F30; // JFG: a cache of (id, asset) pairs
extern s32 D_800F6F40;  // JFG: its length

/* JFG: get entry id of asset 0x2B (offsets in table 0x2A) through the D_800F6F30 cache; the first byte of an
 * asset counts its users. */
s8 *asset_cache_get(s16 id) {
    s32 i;
    s32 slot;
    s32 start;
    s32 size;
    s8 *asset;
    s32 *offsets;

    slot = -1;
    for (i = 0; i < D_800F6F40; i++) {
        if (id == D_800F6F30[(i << 1) + 0]) {
            asset = (s8 *) D_800F6F30[(i << 1) + 1];
            (*asset)++;
            return asset;
        }
        if (D_800F6F30[(i << 1) + 0] == -1) {
            slot = i;
        }
    }
    if (slot == -1) {
        slot = D_800F6F40;
        if (D_800F6F40 >= 256) {
            return NULL;
        }
        D_800F6F40++;
    }
    asset_load(0x2A, (u32) D_800F6F34, (id & ~1) * 4, 0x10);
    offsets = (s32 *) ((u32) D_800F6F34 + (id & 1) * 4);
    start = offsets[0];
    size = offsets[1] - start;
    asset = mempool_alloc(size, 0xFF00FF);
    if (asset == NULL) {
        return NULL;
    }
    asset_load(0x2B, (u32) asset, start, size);
    *asset = 1;
    D_800F6F30[(slot << 1) + 0] = id;
    D_800F6F30[(slot << 1) + 1] = (s32) asset;
    return asset;
}

/* DKR's asset cache pairs (as gTextureCache) */
#define ASSETCACHE_ID(x) ((x << 1) + 0)
#define ASSETCACHE_PTR(x) ((x << 1) + 1)

extern s32 *D_800F6F30; // JFG: a cache of (id, asset) pairs
extern s32 D_800F6F40;  // JFG: its length
void mempool_free(void *data);

/* JFG: release a cached asset (its count is the first byte); free it when unused. */
void func_8003D150(s8 *asset) {
    s32 i;
    s32 index;

    if (asset != NULL) {
        (*asset)--;
        if (*asset <= 0) {
            index = -1;
            for (i = 0; i < D_800F6F40; i++) {
                if ((s32) asset == D_800F6F30[ASSETCACHE_PTR(i)]) {
                    index = i;
                }
            }
            if (index != -1) {
                mempool_free(asset);
                D_800F6F30[ASSETCACHE_ID(index)] = -1;
                D_800F6F30[ASSETCACHE_PTR(index)] = -1;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003D21C);

extern s32 D_800A2EC0;

void func_8003DDBC(s32 arg0) {
    D_800A2EC0 = arg0;
}

extern s32 D_800A2EC0;

s32 func_8003DDC8(void) {
    return D_800A2EC0;
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003DDD8);

extern s8 D_800F6F48[3];
extern s16 D_800F6F4C[3];

void func_8003E35C(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        D_800F6F48[i] = -1;
        D_800F6F4C[i] = 1000;
    }
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003E398);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003E638);

extern s32 *D_800F6F3C; // JFG: the second cache of (id, asset) pairs
extern s32 D_800F6F44;

/* JFG: asset_cache_get for asset 0x44 (offsets in table 0x43), the second cache (32 entries, a short count). */
s16 *asset_cache2_get(s16 id) {
    s32 i;
    s32 slot;
    s32 start;
    s32 size;
    s16 *asset;
    s32 *offsets;

    slot = -1;
    for (i = 0; i < D_800F6F44; i++) {
        if (id == D_800F6F3C[(i << 1) + 0]) {
            asset = (s16 *) D_800F6F3C[(i << 1) + 1];
            (*asset)++;
            return asset;
        }
        if (D_800F6F3C[(i << 1) + 0] == -1) {
            slot = i;
        }
    }
    if (slot == -1) {
        slot = D_800F6F44;
        if (D_800F6F44 >= 32) {
            return NULL;
        }
        D_800F6F44++;
    }
    asset_load(0x43, (u32) D_800F6F34, (id & ~1) * 4, 0x10);
    offsets = (s32 *) ((u32) D_800F6F34 + (id & 1) * 4);
    start = offsets[0];
    size = offsets[1] - start;
    asset = mempool_alloc(size, 0xFF00FF);
    if (asset == NULL) {
        return NULL;
    }
    asset_load(0x44, (u32) asset, start, size);
    *asset = 1;
    D_800F6F3C[(slot << 1) + 0] = id;
    D_800F6F3C[(slot << 1) + 1] = (s32) asset;
    return asset;
}

extern s32 *D_800F6F3C; // JFG: the second cache of (id, asset) pairs
extern s32 D_800F6F44;

/* JFG: func_8003D150 for the second cache (the count is the first short). */
void func_8003F0E4(s16 *asset) {
    s32 i;
    s32 index;

    if (asset != NULL) {
        (*asset)--;
        if (*asset <= 0) {
            index = -1;
            for (i = 0; i < D_800F6F44; i++) {
                if ((s32) asset == D_800F6F3C[ASSETCACHE_PTR(i)]) {
                    index = i;
                }
            }
            if (index != -1) {
                mempool_free(asset);
                D_800F6F3C[ASSETCACHE_ID(index)] = -1;
                D_800F6F3C[ASSETCACHE_PTR(index)] = -1;
            }
        }
    }
}

extern s32 D_800F6F58;

s32 func_8003F1B0(s32 *size) {
    *size = 0x400;
    return D_800F6F58;
}

typedef struct Weight3C { s16 value; s16 value2; s32 id; } Weight3C;
typedef struct Bone3C { u8 pad0[0x16]; s16 scale; } Bone3C;
typedef struct BoneRef3C { Bone3C *bone; s32 pad4; } BoneRef3C;
typedef struct Model3C { u8 pad0[0x18]; BoneRef3C *bones; u8 pad1C[0x2C - 0x1C]; u8 count; } Model3C;
typedef struct Obj3C { Model3C *model; u8 pad4[0xB - 4]; s8 index; u8 padC[0x74 - 0xC]; Weight3C *weights; s16 *out[1]; } Obj3C;

void func_8003F1C8(Obj3C *obj, s32 id, s32 value) {
    Model3C *model = obj->model;
    Weight3C *w = obj->weights;
    s16 *out = obj->out[obj->index];
    s32 i;
    s32 k;
    Bone3C *bone;

    for (i = 0; i < model->count; i++, w++) {
        k = w->id & 0xFF;
        bone = model->bones[k].bone;
        if (k == id) {
            w->value = value;
        }
        *out++ = (w->value >> 8) * bone->scale;
        if (w->value2 >= 0) {
            *out++ = (w->value2 >> 8) * bone->scale;
        }
    }
}

/* JFG: an entry of the D_800A2EC8 texture table */
typedef struct TexEntry3C {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 address;
} TexEntry3C;

extern TexEntry3C *D_800A2EC8;
extern s16 D_800A2ECC;
s32 tex_find_id(void *texture);

/* JFG: free the textures of every cached model whose id is not in `keep` (ended by -1), noting each one's id
 * and address in a new D_800A2EC8 table (at most 125). */
void model_cache_textures_release(s16 *keep) {
    TexEntry3C *entry;
    Model3C7 *model;
    s32 i;
    s32 j;
    s32 found;

    D_800A2ECC = 0;
    entry = D_800A2EC8 = mempool_alloc_safe(1000, 0xFF00FF);
    for (i = 0; i < D_800F6F1C && D_800A2ECC < 125; i++) {
        if (D_800F6F14[(i << 1) + 0] != -1) {
            model = (Model3C7 *) D_800F6F14[(i << 1) + 1];
            found = FALSE;
            j = 0;
            while (keep[j] != -1 && !found) {
                if (keep[j] == D_800F6F14[(i << 1) + 0]) {
                    found = TRUE;
                }
                j++;
            }
            if (!found) {
                for (j = 0; j < model->numberOfTextures && D_800A2ECC < 125; j++) {
                    entry->address = (s32) model->textures[j].texture;
                    entry->id = tex_find_id(model->textures[j].texture);
                    tex_free(model->textures[j].texture);
                    D_800A2ECC++;
                    entry++;
                }
            }
        }
    }
}
void *load_texture_at(s32 textureID, s32 address);
void mempool_free(void *data);

/* JFG: free the D_800A2EC8 table, loading each entry's texture at its address first. */
void func_8003F468(void) {
    TexEntry3C *entry;
    s32 i;

    if (D_800A2EC8 != NULL) {
        entry = D_800A2EC8;
        for (i = 0; i < D_800A2ECC; i++, entry++) {
            if (entry->address != 0) {
                load_texture_at(entry->id, entry->address);
            }
        }
        mempool_free(D_800A2EC8);
        D_800A2ECC = 0;
    }
}
