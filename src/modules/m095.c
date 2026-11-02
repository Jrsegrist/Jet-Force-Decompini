/*
 * Runlink module 95: a flame weapon. Init clears a user's weapon data and
 * keeps it in a list of up to four; the fire, update, limit, hit and draw
 * functions are still in assembly; the fini function removes the light of
 * each player user and frees it.
 */
#include "module.h"

/* A user's weapon data (size 0x624) */
typedef struct M095Data {
    /* 0x000 */ Object *owner;
} M095Data;

/* The owner's light (in the player's data) */
typedef struct M095Light {
    /* 0x0 */ u8 pad0[0x4];
    /* 0x4 */ s32 light;
} M095Light;

/* What a player object keeps in unk68 */
typedef struct M095Player {
    /* 0x000 */ u8 pad0[0x564];
    /* 0x564 */ M095Light *flameLight;
} M095Player;

extern void func_80098360(void *data, s32 size); /* clears */
extern void func_8002179C(s32 light);            /* DKR light_remove */
extern void func_8004AA80(void *data);           /* DKR mempool_free */

static M095Data *sUsers[4];
static s32 sUserCount;

void func_m095_00000(Object *owner, M095Data *data) {
    func_80098360(data, 0x624);
    data->owner = owner;
    sUsers[sUserCount++] = data;
}

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_0005C);

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_0059C);

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_00D38);

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_0110C);

INCLUDE_ASM("asm/nonmatchings/modules/m095", func_m095_0148C);

void func_m095_0175C(void) {
    s32 i;
    Object *owner;
    M095Data *data;
    M095Player *player;

    i = sUserCount;
    while (i--) {
        data = sUsers[i];
        owner = data->owner;
        if (owner->behaviorId == 1) {
            player = owner->unk68;
            if (player->flameLight->light != 0) {
                func_8002179C(player->flameLight->light);
            }
            func_8004AA80(player->flameLight);
            player->flameLight = NULL;
        }
    }
}
#pragma weak __rl_fini = func_m095_0175C

INCLUDE_ASM("asm/nonmatchings/modules/m095", data);

INCLUDE_ASM("asm/nonmatchings/modules/m095", rodata);
