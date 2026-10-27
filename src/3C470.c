#include "common.h"

/* Copies a block as halfwords (size in bytes, rounded up) (0x8003B870). */
void mem_copy_u16(u16 *src, u16 *dst, s32 size) {
    size = (size + 1) >> 1;
    while (size--) {
        *dst++ = *src++;
    }
}

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003B8AC);

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

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C7C4);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003C92C);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CB04);

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CDAC);

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

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CFCC);

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

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003EF60);

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

INCLUDE_ASM("asm/nonmatchings/3C470", func_8003F284);

/* JFG: an entry of the D_800A2EC8 texture table */
typedef struct TexEntry3C {
    /* 0x00 */ s32 id;
    /* 0x04 */ s32 address;
} TexEntry3C;

extern TexEntry3C *D_800A2EC8;
extern s16 D_800A2ECC;
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
