/*
 * Runlink module 43: collectables described by a table (keyed by object ID):
 * each spins, can drive particle emitters when the player is near, and when
 * the player touches it sets a flag or gives an item, queues a message and
 * plays a sound, then is freed or switches to its second model. A
 * collectable whose conditions (game flags or the player's items) say it
 * was already taken goes away when it is first run.
 */
#include "module.h"

/* Size 0x20 */
typedef struct M043Collectable {
    /* 0x00 */ s16 objectID; /* -1 ends the table */
    /* 0x02 */ s16 keep;     /* nonzero: switch to model 1 rather than free */
    /* 0x04 */ u8 kind[2];   /* 0: game flag, 1, 2: player items, 3: ?, else none */
    /* 0x06 */ u8 id[2];
    /* 0x08 */ s16 message;  /* -1: none */
    /* 0x0A */ s16 sound;    /* -1: none; 0x1000 set: a sound effect */
    /* 0x0C */ f32 radiusSq;
    /* 0x10 */ f32 minY;
    /* 0x14 */ f32 maxY;
    /* 0x18 */ f32 emitRadiusSq;
    /* 0x1C */ u16 emitters; /* 0: none */
    /* 0x1E */ s16 spin;
} M043Collectable;

/* The player's object data (its unk68) */
typedef struct M043PlayerData {
    /* 0x0 */ s8 player;
    /* 0x1 */ s8 unk1;
} M043PlayerData;

extern s32 character_has_item(s32 player, s32 arg1, s32 id);
extern s32 character_has_weapon(s32 player, s32 arg1, s32 id);
extern s32 func_80047DD8(s32 id);
extern void character_item_give(s32 player, s32 arg1, s32 id);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_80001170(u8 sound);

#ifdef NON_MATCHING
static M043Collectable D_m043_00400[] = {
    { 0x18A, 0, { 3, 3 }, { 0x10, 0x11 }, -1, -1, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x220, 0, { 0, 4 }, { 0x20, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x221, 0, { 0, 4 }, { 0x21, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x222, 0, { 0, 4 }, { 0x22, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x225, 0, { 0, 4 }, { 0x23, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x228, 0, { 0, 4 }, { 0x24, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x229, 0, { 0, 4 }, { 0x25, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x22A, 0, { 0, 4 }, { 0x26, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x22B, 0, { 0, 4 }, { 0x27, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x22C, 0, { 0, 4 }, { 0x28, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x22D, 0, { 0, 4 }, { 0x29, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x22E, 0, { 0, 4 }, { 0x2A, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x22F, 0, { 0, 4 }, { 0x2B, 0 }, 0x98, 0x11F5, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
    { 0x265, 0, { 0, 4 }, { 0x2C, 0 }, 0x12D, -1, 1024.0f, -45.0f, 45.0f, 102400.0f, 1, 0x100 },
    { 0x266, 0, { 0, 4 }, { 0x2D, 0 }, 0x12D, -1, 1024.0f, -45.0f, 45.0f, 102400.0f, 1, 0x100 },
    { 0x267, 0, { 0, 4 }, { 0x2E, 0 }, 0x12D, -1, 1024.0f, -45.0f, 45.0f, 102400.0f, 1, 0x100 },
    { 0x27E, 1, { 3, 3 }, { 0x14, 0x15 }, -1, 0x11EE, 1024.0f, -45.0f, 45.0f, 62500.0f, 0, 0 },
    { 0x2C1, 0, { 3, 4 }, { 0x16, 0 }, 0x12E, 0x11EF, 1024.0f, -45.0f, 45.0f, 62500.0f, 0, 0x100 },
    { 0x2ED, 0, { 0, 4 }, { 0x41, 0 }, 0x188, 0xF, 1024.0f, -45.0f, 45.0f, 62500.0f, 0, 0x100 },
    { 0x2DB, 0, { 3, 2 }, { 0x17, 5 }, -1, 0xF, 1024.0f, -45.0f, 45.0f, 90000.0f, 1, 0x100 },
    { 0x2DC, 0, { 3, 2 }, { 0x18, 5 }, -1, 0xF, 1024.0f, -45.0f, 45.0f, 90000.0f, 1, 0x100 },
    { 0x2DD, 0, { 3, 2 }, { 0x19, 5 }, -1, 0xF, 1024.0f, -45.0f, 45.0f, 90000.0f, 1, 0x100 },
    { -1, 0, { 0xFF, 0xFF }, { 0, 0 }, -1, -1, 1024.0f, -45.0f, 45.0f, 40000.0f, 0, 0 },
};
#endif

void func_m043_00000(Object *obj, LevelObjectEntryCommon *setup) {
    obj->modelIndex = 0;
    *OBJ_STATE(obj, s32) = 1; /* check the conditions on the first run */
}

#ifdef NON_MATCHING
/*
 * Equivalent, and the frame matches; the original keeps the switch values
 * in v1 (here v0) and spills the second flag of the 0x265 sum to the stack
 * rather than keeping it in s1.
 */
void func_m043_00014(Object *obj, s32 updateRate) {
    M043Collectable *e;
    s32 got[2];
    M043PlayerData *pd;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 i;
    s32 sound;
    s32 a;
    Object *player;
    s32 b;

    if (obj->modelIndex != 0) {
        return;
    }
    player = func_8000E4D0(0);
    if (player == NULL) {
        func_800079B8(obj);
        return;
    }
    pd = player->unk68;
    for (e = D_m043_00400; e->objectID != -1 && e->objectID != obj->objectID; e++) {
    }
    if (*OBJ_STATE(obj, s32) != 0) {
        i = 2;
        while (i--) {
            switch (e->kind[i]) {
                case 0:
                    got[i] = game_flag_get(e->id[i]);
                    break;
                case 1:
                    got[i] = character_has_item(pd->player, pd->unk1, e->id[i]);
                    break;
                case 2:
                    got[i] = character_has_weapon(pd->player, pd->unk1, e->id[i]);
                    break;
                case 3:
                    got[i] = func_80047DD8(e->id[i]);
                    break;
                default:
                    got[i] = 0;
                    break;
            }
        }
        if (got[0] || got[1]) {
            if (e->keep) {
                obj->modelIndex = 1;
            } else {
                func_800079B8(obj);
            }
        }
        *OBJ_STATE(obj, s32) = 0;
    }
    obj->trans.rotation.y_rotation += e->spin * updateRate;
    dx = obj->trans.x_position - player->trans.x_position;
    dy = obj->trans.y_position - player->trans.y_position;
    dz = obj->trans.z_position - player->trans.z_position;
    if (e->emitters != 0) {
        if (dx * dx + dz * dz < e->emitRadiusSq) {
            obj->particleEmittersEnabled = e->emitters;
        } else {
            obj->particleEmittersEnabled = 0;
        }
        func_8005F5AC(obj, updateRate);
    }
    if (dx * dx + dz * dz < e->radiusSq && e->minY < dy && dy < e->maxY) {
        switch (e->kind[0]) {
            case 0:
                func_80047EFC(e->id[0], 1);
                break;
            case 1:
            case 3:
                character_item_give(pd->player, pd->unk1, e->id[0]);
                break;
        }
        if (e->message != -1) {
            func_m006_011E0(pd, e->message);
        }
        sound = e->sound;
        if (sound != -1) {
            if (sound & 0x1000) {
                func_80001248((u16) (sound & 0xEFFF), 0);
            } else {
                func_80001170(sound);
            }
        } else if (e->objectID >= 0x265 && e->objectID < 0x268) {
            a = game_flag_get(0x2D);
            b = game_flag_get(0x2C);
            func_80001170(game_flag_get(0x2E) + b + a + 0x46);
        }
        if (e->keep) {
            obj->modelIndex = 1;
        } else {
            func_800079B8(obj);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m043", func_m043_00014);

INCLUDE_ASM("asm/nonmatchings/modules/m043", data);
#endif
