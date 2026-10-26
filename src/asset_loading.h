#ifndef _ASSET_LOADING_H_
#define _ASSET_LOADING_H_

/* From the Diddy Kong Racing decompilation (CC0 1.0), src/asset_loading.h. */

#include "common.h"

void pi_init(void);
u32 *asset_table_load(u32 assetIndex);
u8 *asset_table_load_zipped(u32 assetIndex, s32 extraMemory);
s32 asset_load(u32 assetIndex, u32 address, s32 assetOffset, s32 size);
u8 *asset_rom_offset(u32 assetIndex, u32 assetOffset);
s32 asset_table_size(u32 assetIndex);
void dmacopy(u32 romOffset, u32 ramAddress, s32 numBytes);

#endif
