#include "common.h"

/* Size: 0x24 bytes */
typedef struct FmvPlayer {
    /* 0x00 */ s8 unk0;
    /* 0x01 */ u8 pad1[0x14 - 1];
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} FmvPlayer;
extern FmvPlayer D_80104E58[2];
extern u32 *D_80104E50;
u32 *asset_table_load(u32 assetIndex);

void func_80083A90(void) {
    FmvPlayer *player;
    s32 i;
    D_80104E50 = asset_table_load(0x45);
    player = D_80104E58;
    i = 2;
    while (i--) {
        player->unk0 = -1;
        player->unk14 = 0;
        player->unk18 = 0;
        player->unk1C = 0;
        player->unk20 = 0;
        player++;
    }
}
