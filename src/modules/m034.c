/*
 * Runlink module 34: allocates the object pools and loads the object asset
 * tables, decodes the cheat codes, then runs module 33 (the explosion
 * set-up) and unloads it. This is DKR's allocate_object_pools and
 * decrypt_magic_codes (DKR decomp, src/objects.c, CC0) with JFG's pools.
 */
#include "module.h"

extern void *D_800F2CB0;    /* DKR gObjectMemoryPool */
extern void *gParticlePtrList;
extern void *gRacers;
extern void *D_800F2D18;
extern s16 *D_800F2CF8;     /* DKR gAssetsLvlObjTranslationTable */
extern s32 D_800F2CFC;      /* DKR gAssetsLvlObjTranslationTableLength */
extern s32 D_800F2C68;      /* the spawn heap's size */
extern void *D_800F2C64;    /* DKR gSpawnObjectHeap */
extern s32 *D_800F2C74;     /* DKR gAssetsObjectHeadersTable */
extern s32 D_800F2C78;      /* DKR gAssetsObjectHeadersTableLength */
extern void *D_800F2C98;    /* DKR gLoadedObjectHeaders */
extern s16 *D_800F2C9C;     /* DKR gObjectHeaderReferences */
extern s32 *gAssetsMiscSection;
extern s32 *gAssetsMiscTable;
extern s32 gAssetsMiscTableLength;
extern void *gObjPtrList;
extern void *D_800F2CB4;
extern f32 D_800F2C88;      /* DKR gObjectUpdateRateF */

extern void func_800224D0(s32, s32, s32, s32, s32);  /* DKR set_world_shading */
extern void *func_8004A458(s32 size, s32 count);     /* DKR mempool_new_sub */
extern void *func_8004A57C(s32 size, u32 colour);    /* DKR mempool_alloc_safe */
extern void *func_8004D37C(s32 asset);               /* DKR asset_table_load */
extern s32 func_8004D5F8(s32 asset);                 /* DKR asset_table_size */
extern void func_80079BD0(void);
extern void clear_object_pointers(void);
extern void func_m033_00000(void);
extern void func_8005342C(s32 module);               /* unloads a runlink module */

static void decrypt_magic_codes(s32 *data, s32 length);

/* DKR's allocate_object_pools */
void func_m034_00000(void) {
    s32 i;

    func_800224D0(0xAA, 0x55, 0, -0x2000, 0);
    D_800F2CB0 = func_8004A458(0x19000, 0x200);
    gParticlePtrList = func_8004A57C(0x320, 0xFFFF);
    func_80079BD0();
    gRacers = func_8004A57C(0x10, 0xFFFF);
    D_800F2D18 = func_8004A57C(0x40, 0xFFFF);
    D_800F2CF8 = func_8004D37C(0x30);
    D_800F2CFC = (func_8004D5F8(0x30) >> 1) - 1;
    while (D_800F2CF8[D_800F2CFC] == 0) {
        D_800F2CFC--;
    }
    D_800F2C64 = func_8004A57C(D_800F2C68 = 0x2000, 0xFFFF);
    D_800F2C74 = func_8004D37C(0x2E);
    D_800F2C78 = 0;
    while (-1 != D_800F2C74[D_800F2C78]) {
        D_800F2C78++;
    }
    D_800F2C78--;
    D_800F2C98 = func_8004A57C(D_800F2C78 * 4, -1);
    D_800F2C9C = func_8004A57C(D_800F2C78 * 2, -1);
    for (i = 0; i < D_800F2C78; i++) {
        D_800F2C9C[i] = 0;
    }
    gAssetsMiscSection = func_8004D37C(0x18);
    gAssetsMiscTable = func_8004D37C(0x19);
    gAssetsMiscTableLength = 0;
    while (-1 != gAssetsMiscTable[gAssetsMiscTableLength]) {
        gAssetsMiscTableLength++;
    }
    decrypt_magic_codes(&gAssetsMiscSection[gAssetsMiscTable[5]], (gAssetsMiscTable[6] - gAssetsMiscTable[5]) * 4);
    gObjPtrList = func_8004A57C(0x800, 0xFFFF);
    D_800F2CB4 = func_8004A57C(0x100, 0xFFFF);
    D_800F2C88 = 2.0f;
    clear_object_pointers();
    func_m033_00000();
    func_8005342C(33);
}

/* DKR's decrypt_magic_codes */
static void decrypt_magic_codes(s32 *data, s32 length) {
    s32 i;
    s32 j;
    u8 *ptr = (u8 *) data;
    u8 temp[4];

    for (i = 0; i < (length >> 2); i++) {
        // Swap bits according to the following pattern:
        // AABBCCDD EEFFGGHH IIJJKKLL MMNNOOPP -> AAEEIIMM BBFFJJNN CCGGKKOO DDHHLLPP
        temp[0] = ((ptr[0] & 0xC0)) | ((ptr[1] & 0xC0) >> 2) | ((ptr[2] & 0xC0) >> 4) | ((ptr[3] & 0xC0) >> 6);
        temp[1] = ((ptr[0] & 0x30) << 2) | ((ptr[1] & 0x30)) | ((ptr[2] & 0x30) >> 2) | ((ptr[3] & 0x30) >> 4);
        temp[2] = ((ptr[0] & 0x0C) << 4) | ((ptr[1] & 0x0C) << 2) | ((ptr[2] & 0x0C)) | ((ptr[3] & 0x0C) >> 2);
        temp[3] = ((ptr[0] & 0x03) << 6) | ((ptr[1] & 0x03) << 4) | ((ptr[2] & 0x03) << 2) | ((ptr[3] & 0x03));

        // Swap the odd and even bits
        for (j = 0; j < 4; j++) {
            *ptr++ = (((temp[j] & 0xAA) >> 1) | ((temp[j] & 0x55) << 1));
        }
    }
}
