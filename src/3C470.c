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

/* JFG: a model texture entry (DKR TextureInfo) */
typedef struct ModelTextureB9 {
    /* 0x00 */ void *texture;
    /* 0x04 */ s16 pad4;
    /* 0x06 */ s16 id;
} ModelTextureB9;

/* JFG: a model batch (only the texture index is read here) */
typedef struct ModelBatchB9 {
    /* 0x00 */ u8 textureIndex;
    /* 0x01 */ u8 pad1[0xF];
} ModelBatchB9;

/* JFG: a loaded object model (DKR ObjectModel; only the fields used here) */
typedef struct ModelB9 {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numberOfTextures;
    /* 0x11 */ u8 animateTextures;
    /* 0x12 */ u8 pad12[0x16 - 0x12];
    /* 0x16 */ s16 numberOfBatches;
    /* 0x18 */ ModelTextureB9 *textures;
    /* 0x1C */ u8 *vertices;
    /* 0x20 */ u8 *triangles;
    /* 0x24 */ ModelBatchB9 *batches;
    /* 0x28 */ void *texAnims;
    /* 0x2C */ u8 numTexAnims;
    /* 0x2D */ u8 pad2D[3];
    /* 0x30 */ u8 *unk30;
    /* 0x34 */ u8 *unk34;
    /* 0x38 */ u8 *unk38;
    /* 0x3C */ u8 pad3C[0x4C - 0x3C];
    /* 0x4C */ s16 references;
    /* 0x4E */ s8 numAssets;
    /* 0x4F */ u8 pad4F;
    /* 0x50 */ void *assets;
    /* 0x54 */ u8 *unk54;
    /* 0x58 */ void *unk58;
    /* 0x5C */ u8 pad5C[4];
    /* 0x60 */ void *unk60;
    /* 0x64 */ u8 pad64[0x68 - 0x64];
    /* 0x68 */ u8 *unk68;
    /* 0x6C */ u8 *unk6C;
    /* 0x70 */ u8 *unk70;
    /* 0x74 */ Gfx *dlist;
    /* 0x78 */ Gfx *dlist2;
    /* 0x7C */ s32 numBuffers;
    /* 0x80 */ u8 *bufferEnds;
    /* 0x84 */ Gfx **buffers;
} ModelB9;

extern u8 D_800A5088;
void *mempool_alloc(s32 size, u32 colourTag);
s32 gzip_size_uncompressed(s32 assetIndex, s32 assetOffset);
u8 *gzip_inflate(u8 *compressedInput, u8 *decompressedOutput);
void *load_texture(s32 id);
void tex_free(void *texture);
struct Instance3C;
struct Asset3C;
struct InstanceC1;
struct ModelC1;
struct Model3C7;
struct ModelInstance3C;
struct DrawModel3C;
struct Instance3C *asset_instance_new(struct Asset3C *asset);
struct InstanceC1 *model_instance_init(struct ModelC1 *model, s32 count);
void free_model_data(struct Model3C7 *mdl);
s32 func_8003CB04(struct Model3C7 *model, s32 id);
void model_instance_anim_reset(struct ModelInstance3C *inst);
s32 model_batches_build_dlist(Gfx **out, struct DrawModel3C *model, s32 flags, s32 clearFlags, s32 minId, s32 maxId,
                  s32 noTextures);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/* DKR object_model_init: get model `modelID` from the cache, else load it (JFG: bit 0x8000 asks for an asset
 * instance, display lists built per id range, more failure cleanup), and make an instance of it. */
void *object_model_init(s32 modelID, s32 flags) {
    s32 i;
    s32 j;
    s32 cacheIndex;
    s32 isAsset;
    ModelB9 *objMdl;
    s32 temp_s0;
    s32 sp48;
    u32 compressedData;
    void *instance;
    s8 var_a2;
    s8 cacheChanged;
    UNUSED s32 pad[2];
    s32 modelSize;
    s32 k;
    s32 start;
    s32 end;

    isAsset = modelID & 0x8000;
    modelID ^= isAsset;
    if (modelID >= D_800F6F20) {
        modelID = 0;
    }
    if (D_800A5088 != 0) {
        if (modelID == 0x1BD) {
            modelID = 0x383;
        }
        if (modelID == 0x1BE) {
            modelID = 0x384;
        }
    }
    for (i = 0; i < D_800F6F1C; i++) {
        if (modelID == D_800F6F14[(i << 1) + 0]) {
            objMdl = (ModelB9 *) D_800F6F14[(i << 1) + 1];
            if (isAsset) {
                instance = asset_instance_new((struct Asset3C *) objMdl);
            } else {
                instance = model_instance_init((struct ModelC1 *) objMdl, flags & 3);
            }
            if (instance != NULL) {
                objMdl->references++;
                if (!isAsset) {
                    model_instance_anim_reset((struct ModelInstance3C *) instance);
                }
            }
            return instance;
        }
    }
    var_a2 = FALSE;
    cacheChanged = FALSE;
    if (D_800F6F24 > 0) {
        D_800F6F24--;
        var_a2 = TRUE;
        cacheIndex = D_800F6F18[D_800F6F24];
    } else {
        cacheIndex = D_800F6F1C;
        cacheChanged = TRUE;
        D_800F6F1C++;
    }
    temp_s0 = D_800F6F10[modelID];
    sp48 = D_800F6F10[modelID + 1] - temp_s0;
    modelSize = gzip_size_uncompressed(0x27, temp_s0) + 0x80;
    objMdl = (ModelB9 *) mempool_alloc(modelSize, 0xFF0000FF);
    if (objMdl == NULL) {
        if (var_a2) {
            D_800F6F24++;
        }
        if (cacheChanged) {
            D_800F6F1C--;
        }
        return NULL;
    }
    compressedData = (u32) ((u8 *) objMdl + modelSize) - sp48;
    asset_load(0x27, compressedData, temp_s0, sp48);
    gzip_inflate((u8 *) compressedData, (u8 *) objMdl);
    if (objMdl->numBuffers != 0) {
        objMdl->buffers = mempool_alloc((objMdl->numBuffers * 4) + 4, 0xFF0000FF);
        if (objMdl->buffers == NULL) {
            if (var_a2) {
                D_800F6F24++;
            }
            if (cacheChanged) {
                D_800F6F1C--;
            }
            return NULL;
        }
    } else {
        objMdl->buffers = NULL;
    }
    objMdl->textures = (ModelTextureB9 *) ((s32) objMdl->textures + (u8 *) objMdl);
    objMdl->vertices = (u8 *) ((s32) objMdl->vertices + (u8 *) objMdl);
    objMdl->triangles = (u8 *) ((s32) objMdl->triangles + (u8 *) objMdl);
    objMdl->batches = (ModelBatchB9 *) ((s32) objMdl->batches + (u8 *) objMdl);
    objMdl->bufferEnds = (u8 *) ((s32) objMdl->bufferEnds + (u8 *) objMdl);
    objMdl->unk30 = (u8 *) ((s32) objMdl->unk30 + (u8 *) objMdl);
    objMdl->unk34 = (u8 *) ((s32) objMdl->unk34 + (u8 *) objMdl);
    objMdl->unk38 = (u8 *) ((s32) objMdl->unk38 + (u8 *) objMdl);
    objMdl->unk6C = (u8 *) ((s32) objMdl->unk6C + (u8 *) objMdl);
    if (objMdl->unk70 != NULL) {
        objMdl->unk70 = (u8 *) ((s32) objMdl->unk70 + (u8 *) objMdl);
    }
    objMdl->unk58 = NULL;
    objMdl->unk68 = (u8 *) ((s32) objMdl->unk68 + (u8 *) objMdl);
    if (objMdl->unk54 != NULL) {
        objMdl->unk54 = (u8 *) ((s32) objMdl->unk54 + (u8 *) objMdl);
    }
    objMdl->references = 1;
    objMdl->numAssets = 0;
    objMdl->unk60 = NULL;
    objMdl->assets = NULL;
    objMdl->texAnims = NULL;
    objMdl->dlist = NULL;
    objMdl->dlist2 = NULL;
    for (i = 0; i < objMdl->numberOfTextures; i++) {
        objMdl->textures[i].texture = load_texture(objMdl->textures[i].id);
        if (objMdl->textures[i].texture == NULL) {
            for (j = 0; j < i; j++) {
                tex_free(objMdl->textures[j].texture);
                objMdl->textures[j].texture = NULL;
            }
            for (; j < objMdl->numberOfTextures; j++) {
                objMdl->textures[j].texture = NULL;
            }
            goto block_30;
        }
    }
    for (i = 0; i < objMdl->numberOfBatches; i++) {
        if (objMdl->batches[i].textureIndex != 0xFF && objMdl->batches[i].textureIndex >= objMdl->numberOfTextures) {
            goto block_30;
        }
    }
    if (func_8003CB04((struct Model3C7 *) objMdl, modelID) == 0) {
        if (objMdl->animateTextures != 0) {
            objMdl->texAnims = mempool_alloc(objMdl->numberOfTextures * 8, 0xFF0000FF);
            if (objMdl->texAnims == NULL) {
                goto block_30;
            }
        }
        if (objMdl->numBuffers != 0) {
            start = 0;
            for (k = 0; k < objMdl->numBuffers; k++) {
                end = objMdl->bufferEnds[k] - 1;
                model_batches_build_dlist(&objMdl->buffers[k], (struct DrawModel3C *) objMdl, 0, 0, start, end, 0);
                start = end + 1;
            }
            model_batches_build_dlist(&objMdl->buffers[k], (struct DrawModel3C *) objMdl, 0, 0, start, 0xFF, 0);
        } else {
            objMdl->numTexAnims = model_batches_build_dlist(&objMdl->dlist, (struct DrawModel3C *) objMdl, 0, 0, 0, 0xFF, 0);
            if (objMdl->dlist == NULL) {
                goto block_30;
            }
            model_batches_build_dlist(&objMdl->dlist2, (struct DrawModel3C *) objMdl, 4, 0, 0, 0xFF, 0);
            if (objMdl->dlist2 == NULL) {
                goto block_30;
            }
        }
        if (isAsset) {
            instance = asset_instance_new((struct Asset3C *) objMdl);
        } else {
            instance = model_instance_init((struct ModelC1 *) objMdl, flags & 3);
        }
        if (instance != NULL) {
            if (!isAsset) {
                model_instance_anim_reset((struct ModelInstance3C *) instance);
            }
            D_800F6F14[(cacheIndex << 1) + 0] = modelID;
            D_800F6F14[(cacheIndex << 1) + 1] = (s32) objMdl;
            if (D_800F6F1C < 85) {
                return instance;
            }
        }
    }
block_30:
    if (cacheChanged) {
        D_800F6F1C--;
    }
    if (var_a2) {
        D_800F6F24++;
    }
    free_model_data((struct Model3C7 *) objMdl);
    return NULL;
}

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
Instance3C *asset_instance_new(Asset3C *asset) {
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

/* JFG: a model vertex (DKR Vertex) */
typedef struct VertexC1 {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} VertexC1;

typedef struct Vec3fC1 {
    f32 x;
    f32 y;
    f32 z;
} Vec3fC1;

/* JFG: an 8-byte model entry the instance gets a copy of */
typedef struct ModelEntryC1 {
    /* 0x00 */ s16 a;
    /* 0x02 */ s16 b;
    /* 0x04 */ s32 c;
} ModelEntryC1;

/* JFG: an 8-byte entry of the instance's two tables of 10, cleared */
typedef struct InstSlotC1 {
    /* 0x00 */ s16 a;
    /* 0x02 */ s16 b;
    /* 0x04 */ s16 c;
    /* 0x06 */ s16 d;
} InstSlotC1;

typedef struct ModelIndexC1 {
    /* 0x00 */ u16 vertex;
    /* 0x02 */ u16 pad2;
} ModelIndexC1;

/* JFG: the model an instance is made from (only the fields used here) */
typedef struct ModelC1 {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 numEntries;
    /* 0x11 */ u8 hasEntries;
    /* 0x12 */ s16 numVertices;
    /* 0x14 */ u8 pad14[0x1C - 0x14];
    /* 0x1C */ VertexC1 *vertices;
    /* 0x20 */ u8 pad20[0x28 - 0x20];
    /* 0x28 */ ModelEntryC1 *entries;
    /* 0x2C */ u8 pad2C;
    /* 0x2D */ u8 numPoints;
    /* 0x2E */ u8 numPoints2;
    /* 0x2F */ u8 numPoints3;
    /* 0x30 */ ModelIndexC1 *pointVertices;
    /* 0x34 */ u8 pad34[0x3C - 0x34];
    /* 0x3C */ u16 unk3C[6];
    /* 0x48 */ u8 pad48[0x4E - 0x48];
    /* 0x4E */ s8 animated;
    /* 0x4F */ s8 numMatrices;
} ModelC1;

/* JFG: a model instance (0x98 bytes, followed by its buffers) */
typedef struct InstanceC1 {
    /* 0x00 */ ModelC1 *model;
    /* 0x04 */ VertexC1 *vertices;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 pad9;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ MtxF *matrices[2];
    /* 0x18 */ VertexC1 *vertexBuffers[2];
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s16 unk24;
    /* 0x26 */ u8 pad26[2];
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x38 - 0x2C];
    /* 0x38 */ f32 unk38;
    /* 0x3C */ u8 pad3C[0x68 - 0x3C];
    /* 0x68 */ Vec3fC1 *points;
    /* 0x6C */ Vec3fC1 *points2;
    /* 0x70 */ Vec3fC1 *points3;
    /* 0x74 */ ModelEntryC1 *entries;
    /* 0x78 */ InstSlotC1 *slots[2];
    /* 0x80 */ u8 pad80[0x8C - 0x80];
    /* 0x8C */ u16 unk8C[6];
} InstanceC1;

/* JFG: make an instance of a model with `count` vertex buffers: its matrices (two sets when the model is
 * animated), the vertex copies, three point tables and two tables of 10 slots (DKR model_instance_init). */
InstanceC1 *model_instance_init(ModelC1 *model, s32 count) {
    s32 j;
    Vec3fC1 *point;
    VertexC1 *src;
    s32 mtxSize;
    s32 i; // DKR's `temp`: the vertex buffer size, the slot table address and the loop counters
    s32 size2;
    s32 size3;
    s32 size1;
    s32 entriesSize;
    UNUSED s32 pad;
    s32 *clear;
    VertexC1 *dst;
    VertexC1 *v;
    UNUSED s32 pad2;
    InstanceC1 *inst;
    u32 k;
    UNUSED s32 pad3;

    if (model->animated != 0) {
        mtxSize = model->numMatrices << 6;
    } else {
        mtxSize = 0;
    }
    i = model->numVertices * sizeof(VertexC1);
    if (i & 7) {
        i = (i - (i & 7)) + 8;
    }
    size2 = model->numPoints2 * sizeof(Vec3fC1);
    if (size2 & 3) {
        size2 = (size2 - (size2 & 3)) + 4;
    }
    size3 = model->numPoints3 * sizeof(Vec3fC1);
    if (size3 & 3) {
        size3 = (size3 - (size3 & 3)) + 4;
    }
    size1 = model->numPoints * sizeof(Vec3fC1);
    if (size1 & 3) {
        size1 = (size1 - (size1 & 3)) + 4;
    }
    entriesSize = 0;
    if (model->hasEntries != 0) {
        entriesSize = (model->numEntries * sizeof(ModelEntryC1)) + 0xA8;
    }
    inst = mempool_alloc((mtxSize * 2) + (i * count) + size2 + size3 + size1 + entriesSize + sizeof(InstanceC1),
                         0xFFFF);
    if (inst != NULL) {
        if (model->numMatrices != 0 && mtxSize != 0) {
            inst->matrices[0] = (MtxF *) ((u8 *) inst + sizeof(InstanceC1));
            inst->matrices[1] = (MtxF *) ((u8 *) inst->matrices[0] + mtxSize);
            mtxf_set_identity_array(model->numMatrices, inst->matrices[0]);
            mtxf_set_identity_array(model->numMatrices, inst->matrices[1]);
        } else {
            inst->matrices[0] = NULL;
            inst->matrices[1] = NULL;
        }
        if (count > 0) {
            inst->vertexBuffers[0] = (VertexC1 *) ((u8 *) inst + (mtxSize * 2) + sizeof(InstanceC1));
        } else {
            inst->vertexBuffers[0] = model->vertices;
        }
        if (count >= 2) {
            inst->vertexBuffers[1] = (VertexC1 *) ((u8 *) inst->vertexBuffers[0] + i);
        } else {
            inst->vertexBuffers[1] = inst->vertexBuffers[0];
        }
        if (model->numPoints2 != 0) {
            inst->points2 = (Vec3fC1 *) ((u8 *) inst + (mtxSize * 2) + (i * count) + sizeof(InstanceC1));
        } else {
            inst->points2 = NULL;
        }
        if (model->numPoints3 != 0) {
            inst->points3 =
                (Vec3fC1 *) ((u8 *) inst + (mtxSize * 2) + (i * count) + size2 + sizeof(InstanceC1));
        } else {
            inst->points3 = NULL;
        }
        if (model->numPoints != 0) {
            inst->points =
                (Vec3fC1 *) ((u8 *) inst + (mtxSize * 2) + (i * count) + size2 + size3 + sizeof(InstanceC1));
        } else {
            inst->points = NULL;
        }
        if (model->hasEntries != 0) {
            inst->entries = (ModelEntryC1 *) ((u8 *) inst + (mtxSize * 2) + (i * count) + size2 + size3 +
                                              size1 + sizeof(InstanceC1));
            i = (s32) (inst->entries + model->numEntries);
            if ((u32) i & 7) {
                i = (i - ((u32) i & 7)) + 8;
            }
            inst->slots[0] = (InstSlotC1 *) i;
            inst->slots[1] = (InstSlotC1 *) (i + 0x50);
            for (i = 0; i < 2; i++) {
                j = 0;
                while (TRUE) {
                    inst->slots[i][j].a = 0;
                    inst->slots[i][j].b = 0;
                    inst->slots[i][j].c = 0;
                    inst->slots[i][j].d = 0;
                    j++;
                    if (j != 10) {
                        continue;
                    }
                    break;
                }
            }
        } else {
            inst->entries = NULL;
            inst->slots[0] = NULL;
            inst->slots[1] = NULL;
        }
        for (i = 0; i < count; i++) {
            src = model->vertices;
            dst = inst->vertexBuffers[i];
            for (j = 0; j < model->numVertices; j++) {
                dst->x = src->x;
                dst->y = src->y;
                dst->z = src->z;
                dst->r = src->r;
                dst->g = src->g;
                dst->b = src->b;
                dst->a = src->a;
                src++;
                dst++;
            }
        }
        k = 2;
        if (model->animated == 0) {
            point = inst->points;
            for (i = 0; i < model->numPoints; i++) {
                v = &model->vertices[model->pointVertices[i].vertex];
                point->x = v->x;
                point->y = v->y;
                point->z = v->z;
                point++;
            }
        } else {
            do {
                if (k == 2) {
                    i = model->numPoints;
                    point = inst->points;
                } else if (k == 1) {
                    i = model->numPoints2;
                    point = inst->points2;
                } else {
                    i = model->numPoints3;
                    point = inst->points3;
                }
                while (i--) {
                    point->x = 0.0f;
                    point->y = 0.0f;
                    point->z = 0.0f;
                    point++;
                }
            } while (k--);
        }
        if (inst->entries != NULL) {
            for (i = 0; i < model->numEntries; i++) {
                inst->entries[i].a = model->entries[i].a;
                inst->entries[i].b = model->entries[i].b;
                inst->entries[i].c = model->entries[i].c;
            }
        }
        clear = &inst->unk20;
        i = 17;
        while (i--) {
            *clear++ = 0;
        }
        inst->model = model;
        inst->unk8 = 0;
        inst->unkB = 0;
        inst->unkC = 0;
        inst->unkA = 2;
        inst->unk24 = 0;
        inst->unk38 = 0.0f;
        inst->unk28 = 0.0f;
        inst->vertices = inst->vertexBuffers[0];
        mem_copy_u16(model->unk3C, inst->unk8C, 12);
    }
    return inst;
}

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
    /* 0x50 */ s8 **assets; // released with asset_cache_release
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
    /* 0x80 */ s8 *assets[2]; // released with asset_cache_release
    /* 0x88 */ s16 assetIds[2];
} ModelInstance3C;

void free_model_data(Model3C7 *model);
void asset_cache_release(s8 *asset);
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
                    asset_cache_release(modInst->assets[i]);
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
                asset_cache_release(mdl->assets[j]);
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
void *model_asset_block_load(s32 index);

#ifdef NON_MATCHING
/* NON_MATCHING: saved registers: the target keeps &D_800F6F2C in $s0 and id in $s1 (mine the other way round)
   and reloads/re-spills the offset around each call */
/* JFG: load model `id`'s asset list (tables 0x28/0x29) into the model: its 0x58 block, the ids (0x5C) and, unless
 * 0x64 is set, the cached assets (0x50); returns 1 when something could not be had (all freed again). */
s32 func_8003CB04(Model3C7 *model, s32 id) {
    s32 offset;
    s32 start;
    s32 end;
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
    if ((model->unk58 = model_asset_block_load(id)) == NULL) {
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
            model->assets[i] = asset_cache_get(((s16 *) D_800F6F28)[offset]);
            if (model->assets[i] == NULL) {
                for (j = 0; j < i; j++) {
                    asset_cache_release(model->assets[j]);
                }
                mempool_free(model->assets);
                mempool_free(model->unk58);
                mempool_free(model->unk5C);
                model->assets = NULL;
                return 1;
            }
            ((s16 *) model->unk5C)[i] = ((s16 *) D_800F6F28)[offset];
            offset++;
            i++;
        } while (i < model->numAssets);
        return 0;
    }
    for (i = 0; i < model->numAssets; i++) {
        ((s16 *) model->unk5C)[i] = ((s16 *) D_800F6F28)[offset];
        offset++;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/3C470", func_8003CB04);
#endif

/* JFG: reset a model instance's animation state and select its first animation (shared assets are fetched
 * through the cache when the model has unk64 set). */
void model_instance_anim_reset(ModelInstance3C *inst) {
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
            inst->assets[0] = asset_cache_get(*(s16 *) inst->model->unk5C);
            inst->assets[1] = asset_cache_get(*(s16 *) inst->model->unk5C);
            anim = inst->assets[0];
            inst->assetIds[0] = *(s16 *) inst->model->unk5C;
            inst->assetIds[1] = *(s16 *) inst->model->unk5C;
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

extern s32 *D_800F6F38; // JFG: a buffer for two entries of an offset table
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

/* JFG: load an entry of asset 0x2D, found by its offset in table 0x2C. */
void *model_asset_block_load(s32 index) {
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
void asset_cache_release(s8 *asset) {
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

/* JFG: a texture header (only the fields used here; DKR TextureHeader) */
typedef struct BatchTexture3C {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 flags;
    /* 0x08 */ u8 pad8[0x12 - 0x8];
    /* 0x12 */ u16 numOfTextures;
    /* 0x14 */ u8 pad14[4];
    /* 0x18 */ u8 unk18;
} BatchTexture3C;

/* JFG: a model batch (0x10 bytes; DKR TriangleBatchInfo with JFG's slot ids and vertex splits) */
typedef struct DrawBatch3C {
    /* 0x00 */ u8 textureIndex;
    /* 0x01 */ s8 ids[3];
    /* 0x04 */ s8 splits[2];
    /* 0x06 */ s16 verticesOffset;
    /* 0x08 */ s16 facesOffset;
    /* 0x0A */ u8 padA;
    /* 0x0B */ u8 texOffset;
    /* 0x0C */ u32 flags;
} DrawBatch3C;

/* JFG: a texture animation entry the draw records for each textured batch */
typedef struct DrawTexAnim3C {
    /* 0x00 */ s16 offset;
    /* 0x02 */ s16 frame;
    /* 0x04 */ s32 flags;
} DrawTexAnim3C;

typedef struct DrawTexture3C {
    /* 0x00 */ BatchTexture3C *texture;
    /* 0x04 */ s32 unk4;
} DrawTexture3C;

typedef struct DrawModel3C {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 animateTextures;
    /* 0x12 */ u8 pad12[0x16 - 0x12];
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ DrawTexture3C *textures;
    /* 0x1C */ u8 pad1C[4];
    /* 0x20 */ u8 *triangles;
    /* 0x24 */ DrawBatch3C *batches;
    /* 0x28 */ DrawTexAnim3C *texAnims;
    /* 0x2C */ u8 pad2C[0x4E - 0x2C];
    /* 0x4E */ s8 useSlots;
} DrawModel3C;

#define G_TRIN 5
/* Draws numTris triangles at ptr (DKR) */
#define gSPPolygon(dl, ptr, numTris, texEnabled)                                                                   \
    {                                                                                                              \
        Gfx *_g = (Gfx *) (dl);                                                                                    \
        _g->words.w0 =                                                                                             \
            _SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | _SHIFTL(G_TRIN, 24, 8) | _SHIFTL(((numTris) * 16), 0, 16); \
        _g->words.w1 = (unsigned int) (ptr);                                                                       \
    }

struct BatchModel3E;
extern u8 D_800A4E70;
void func_80055684(u8 arg0);
void batch_slots_reset(void);
void rendermode_reset(Gfx **dList);
void func_80055C00(Gfx **dList, void *tex, s32 flags, s32 texOffset); // DKR: material_set
void func_8003E398(Gfx **dList, s8 *ids, s8 *slots, struct BatchModel3E *model, s32 batchIndex, s32 base);

/* JFG: build a display list for the model's batches whose first id is in [minId, maxId] into the D_800F6F58
 * buffer, then copy it to a new block (*out); returns the number of texture animation entries written. */
s32 model_batches_build_dlist(Gfx **out, DrawModel3C *model, s32 flags, s32 clearFlags, s32 minId, s32 maxId, s32 noTextures) {
    s32 i;
    DrawBatch3C *batch;
    BatchTexture3C *tex;
    s32 texOffset;
    s32 vertOffset;
    s32 numTris;
    s32 numVerts;
    s32 batchFlags;
    s32 j;
    s32 start;
    s32 prevOffset;
    s32 end;
    s32 hasTexture;
    u32 n;
    s32 count;
    s8 slots[3];
    s32 texIndex;
    BatchTexture3C *prevTex;
    Gfx *src;
    u8 *triangles;
    DrawTexAnim3C *anim;
    Gfx *dl;

    batch = model->batches;
    func_80055684(1);
    if (flags & 4) {
        D_800A4E70 = 1;
    }
    clearFlags = ~clearFlags;
    dl = (Gfx *) D_800F6F58;
    prevTex = (BatchTexture3C *) -1;
    prevOffset = -1;
    if (minId == 0 && noTextures == 0) {
        batch_slots_reset();
        rendermode_reset(&dl);
    } else if (noTextures != 0) {
        batch_slots_reset();
        rendermode_reset(NULL);
        gDPPipeSync(dl++);
        gSPSetGeometryMode(dl++, 0x10001);
    }
    anim = model->texAnims;
    count = 0;
    i = 0;
    for (; i < model->numBatches; i++, batch++) {
        batchFlags = batch->flags;
        if (batch->ids[0] >= minId && maxId >= batch->ids[0] && !(batch->flags & 0x400)) {
            vertOffset = batch->verticesOffset;
            numVerts = batch[1].verticesOffset - vertOffset;
            numTris = batch[1].facesOffset - batch->facesOffset;
            triangles = &model->triangles[batch->facesOffset * 16];
            texIndex = batch->textureIndex;
            if (texIndex == 0xFF || noTextures != 0) {
                texOffset = 0;
                tex = NULL;
                hasTexture = FALSE;
            } else {
                texOffset = batch->texOffset << 14;
                tex = model->textures[texIndex].texture;
                hasTexture = TRUE;
            }
            batchFlags = (batchFlags | flags | D_800A2EC0) & clearFlags;
            if (model->animateTextures != 0 && tex != NULL && (tex != prevTex || texOffset != prevOffset)) {
                anim->offset = texOffset;
                anim->frame = -1;
                anim->flags = (batch->flags & ~0xFF) | texIndex;
                if (tex->flags & 0x40) {
                    anim->frame = texOffset + 0x100;
                    if (anim->frame >= tex->numOfTextures) {
                        if (tex->unk18) {
                            anim->frame = 0;
                        } else {
                            anim->frame -= 0x100;
                        }
                    }
                }
                anim++;
                count++;
            }
            prevTex = tex;
            prevOffset = texOffset;
            func_80055C00(&dl, tex, batchFlags, texOffset);
            if (model->useSlots == 0) {
                gDma1p(dl++, G_VTX, (vertOffset * 10u) & 0x0FFFFFFF, (numVerts << 3) + (numVerts << 1) + 8,
                       (numVerts << 3) | (((vertOffset * 10u) & 0x0FFFFFFF) & 6));
            } else {
                func_8003E398(&dl, batch->ids, slots, (struct BatchModel3E *) model, i, 0);
                j = 0;
                start = 0;
                if (numVerts > 0) {
                    do {
                        gDma1p(dl++, 0xBC, slots[j] << 6, 0xA, 0);
                        if (j < 2) {
                            end = batch->splits[j];
                        } else {
                            end = numVerts;
                        }
                        gDma1p(dl++, G_VTX, ((vertOffset + start) * 10u) & 0x0FFFFFFF,
                               (((end - start) << 3) + ((end - start) << 1) + 8) | (start << 9),
                               ((end - start) << 3) | ((((vertOffset + start) * 10u) & 0x0FFFFFFF) & 6));
                        start = end;
                        j++;
                    } while (end < numVerts);
                }
            }
            gSPPolygon(dl++, (u32) triangles & 0x0FFFFFFF, numTris, hasTexture);
        }
    }
    gDPPipeSync(dl++);
    gSPEndDisplayList(dl++);
    n = dl - (Gfx *) D_800F6F58;
    dl = *out = mempool_alloc(n * 8, 0xFF0000FF);
    if (dl != NULL) {
        src = (Gfx *) D_800F6F58;
        i = 0;
        if (n != 0) {
            do {
                dl->words.w0 = src->words.w0;
                dl->words.w1 = src->words.w1;
                src++;
                dl++;
                i++;
            } while (i < n);
        }
    }
    func_80055684(0);
    D_800A4E70 = 0;
    return count;
}

extern s8 D_800F6F48[3];
extern s16 D_800F6F4C[3];

void batch_slots_reset(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        D_800F6F48[i] = -1;
        D_800F6F4C[i] = 1000;
    }
}

/* JFG: a 0x10 byte batch of a model, with the three slot ids it uses */
typedef struct Batch3E {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ s8 ids[3];
    /* 0x04 */ u8 pad4[0xC];
} Batch3E;

typedef struct BatchModel3E {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 numBatches;
    /* 0x18 */ u8 pad18[0x24 - 0x18];
    /* 0x24 */ Batch3E *batches;
} BatchModel3E;

#ifdef NON_MATCHING
/* NON_MATCHING: same code, other registers: the target keeps each slot id in $a1 (copied from $v0) and the
   loop counters/pointers in $v1/$t0/$t1, mine shifts them all by one */
/* JFG: the three cached slots (D_800F6F48: id, D_800F6F4C: batches until it is used again): count each down and
 * re-measure; then give each of the three ids a slot (one it has, a free one or the one needed latest that none
 * of them uses), loading a newly given one with a command (base + id * 0x40). */
void func_8003E398(Gfx **dList, s8 *ids, s8 *slots, BatchModel3E *model, s32 batchIndex, s32 base) {
    s32 i;
    s32 j;
    s32 best;
    s32 bestCount;
    s8 *slot;
    s8 *id;
    Gfx *gfx;

    for (i = 0; i < 3; i++) {
        if (D_800F6F48[i] != -1) {
            D_800F6F4C[i]--;
            if (D_800F6F4C[i] <= 0) {
                D_800F6F4C[i] = 1;
                for (j = batchIndex + 1; j < model->numBatches; j++) {
                    if (model->batches[j].ids[0] == D_800F6F48[i] || model->batches[j].ids[1] == D_800F6F48[i] ||
                        model->batches[j].ids[2] == D_800F6F48[i]) {
                        j = model->numBatches;
                    } else {
                        D_800F6F4C[i]++;
                    }
                }
            }
        }
    }
    slot = slots;
    id = ids;
    for (j = 0; j < 3; j++, slot++, id++) {
        *slot = -1;
        if (*id == -1) {
            continue;
        }
        for (i = 0; i < 3; i++) {
            if (D_800F6F48[i] == *id) {
                *slot = i + 1;
            }
        }
        if (*slot != -1) {
            continue;
        }
        best = -1;
        for (i = 0; i < 3 && best == -1; i++) {
            if (D_800F6F48[i] == -1) {
                best = i;
            }
        }
        if (best == -1) {
            bestCount = 0;
            for (i = 0; i < 3; i++) {
                if (D_800F6F48[i] != ids[0] && D_800F6F48[i] != ids[1] && D_800F6F48[i] != ids[2] &&
                    bestCount < D_800F6F4C[i]) {
                    bestCount = D_800F6F4C[i];
                    best = i;
                }
            }
        }
        D_800F6F48[best] = *id;
        D_800F6F4C[best] = 0;
        *slot = best + 1;
        gfx = (*dList)++;
        gfx->words.w0 = (((*slot | 0x80) & 0xFF) << 16) | 0x1000040;
        gfx->words.w1 = ((*id << 6) + base) & 0x0FFFFFFF;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/3C470", func_8003E398);
#endif

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

/* JFG: asset_cache_release for the second cache (the count is the first short). */
void asset_cache2_release(s16 *asset) {
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
