/*
 * Asset table access and ROM DMA.
 *
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/asset_loading.c.
 * piRomLoad and romCopy differ in Jet Force Gemini and are still
 * assembly.
 */
#include "asset_loading.h"
#include "gzip.h"
#include "memory.h"

/************ .bss ************/

extern OSMesgQueue gDmaMesgQueue;
extern OSMesg gDmaMesg;
extern OSMesgQueue gPIMesgQueue;
extern OSMesg gPIMesgBuf[16];
extern u32 *gAssetsLookupTable;

/*******************************/

// The asset table (u32 count, count + 1 offsets) sits just before the assets.
extern u8 __ASSETS_LUT_START[], __ASSETS_LUT_END[];

/**
 * Set up the peripheral interface message queues and scheduling.
 * This will send messages when DMA reads are finished.
 * After, allocate space and load the asset table into RAM.
 * JFG: no DMA mutex, and the table is not locked in memory.
 * Official Name: piInit
 */
void pi_init(void) {
    u32 assetTableSize;
    osCreateMesgQueue(&gPIMesgQueue, gPIMesgBuf, ARRAY_COUNT(gPIMesgBuf));
    osCreateMesgQueue(&gDmaMesgQueue, &gDmaMesg, 1);
    osCreatePiManager((OSPri) 150, &gPIMesgQueue, gPIMesgBuf, ARRAY_COUNT(gPIMesgBuf));

    assetTableSize = __ASSETS_LUT_END - __ASSETS_LUT_START;
    gAssetsLookupTable = (u32 *) mempool_alloc_safe(assetTableSize, COLOUR_TAG_GREY);
    dmacopy((u32) __ASSETS_LUT_START, (u32) gAssetsLookupTable, (s32) assetTableSize);
}

INCLUDE_ASM("asm/nonmatchings/asset_loading", func_8004D37C);

/**
 * Loads a gzip compressed asset from the ROM file.
 * Returns a pointer to the decompressed data.
 * Official name: piRomLoadCompressed
 */
u8 *asset_table_load_zipped(u32 assetIndex, s32 extraMemory) {
    s32 size;
    s32 start;
    s32 totalSpace;
    u8 *gzipHeaderRamPos;
    u8 *out;

    if (gAssetsLookupTable[0] < assetIndex) {
        return NULL;
    }
    assetIndex++;
    out = (u8 *) (assetIndex + gAssetsLookupTable);
    start = ((s32 *) out)[0];
    size = ((s32 *) out)[1] - start;
    gzipHeaderRamPos = (u8 *) mempool_alloc_safe(8, COLOUR_TAG_WHITE);

    dmacopy((u32) (start + __ASSETS_LUT_END), (u32) gzipHeaderRamPos, 8);

    totalSpace = byteswap32(gzipHeaderRamPos) + extraMemory;
    mempool_free(gzipHeaderRamPos);
    out = (u8 *) mempool_alloc_safe(totalSpace + extraMemory, COLOUR_TAG_GREY);
    if (out == NULL) {
        return NULL;
    }
    gzipHeaderRamPos = (out + totalSpace) - size;
    if (1) {} // Fakematch
    dmacopy((u32) (start + __ASSETS_LUT_END), (u32) gzipHeaderRamPos, size);
    gzip_inflate(gzipHeaderRamPos, out);

    return out;
}

/**
 * Loads part of an asset section to a specific memory address.
 * Returns the size argument.
 * Official name: piRomLoadSection
 */
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size) {
    u32 *index;
    s32 start;

    if (size == 0 || gAssetsLookupTable[0] < assetIndex) {
        return NULL;
    }

    assetIndex++;
    index = assetIndex + gAssetsLookupTable;
    start = *index + assetOffset;

    dmacopy((u32) (start + __ASSETS_LUT_END), address, size);

    return size;
}

/**
 * Returns a rom offset of an asset given its asset section and a local offset.
 * Official name: piRomGetSectionPtr
 */
u8 *asset_rom_offset(u32 assetIndex, u32 assetOffset) {
    u32 *index;
    u32 start;

    if (gAssetsLookupTable[0] < assetIndex) {
        return NULL;
    }

    assetIndex++;
    index = assetIndex + gAssetsLookupTable;
    start = *index + assetOffset;

    return start + __ASSETS_LUT_END;
}

/**
 * Returns the size of an asset section.
 * Official name: piRomGetFileSize
 */
s32 asset_table_size(u32 assetIndex) {
    u32 *index;

    if (gAssetsLookupTable[0] < assetIndex) {
        return NULL;
    }

    assetIndex++;
    index = assetIndex + gAssetsLookupTable;

    return *(index + 1) - *index;
}

INCLUDE_ASM("asm/nonmatchings/asset_loading", func_8004D640);
