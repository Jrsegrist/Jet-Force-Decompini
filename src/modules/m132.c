/*
 * Runlink module 132: a weapon pickup. It spins, fades in, and gives the
 * player who touches it the weapon (or that weapon's ammunition), then
 * either respawns after a delay or is freed.
 */
#include "module.h"

typedef struct M132Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s8 model;
    /* 0x0B */ s8 respawn; /* in seconds; 0: no respawn */
} M132Setup;

/* Kept in the object at 0x88 */
typedef struct M132Pickup {
    /* 0x0 */ s16 weapon;
    /* 0x2 */ s16 respawn; /* frames */
    /* 0x4 */ s16 timer;   /* until it comes back */
    /* 0x6 */ s16 fade;    /* 0: hidden; counts up to 0xFF */
} M132Pickup;

/* The player's object data (its unk68) */
typedef struct M132PlayerData {
    /* 0x0 */ s8 unk0;
    /* 0x1 */ s8 unk1;
} M132PlayerData;

/* What func_80047C1C returns: the player's weapons */
typedef struct M132Weapons {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 owned; /* bit per weapon */
    /* 0x0C */ u8 padC[0x14 - 0xC];
    /* 0x14 */ u16 ammo[16];
    /* 0x34 */ u16 maxAmmo[16];
    /* 0x54 */ u8 unk54[0x6F - 0x54];
    /* 0x6F */ u8 count;
} M132Weapons;

/* What func_8003B500 returns: a weapon's properties */
typedef struct M132WeaponInfo {
    /* 0x00 */ u8 pad0[3];
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 pad4[0x24 - 0x4];
    /* 0x24 */ s16 maxAmmo;
    /* 0x26 */ u8 pad26[2];
    /* 0x28 */ s16 ammo;
    /* 0x2A */ s16 ammoPickup;
} M132WeaponInfo;

extern s16 D_800A0CB0[15]; /* the weapon pickups' object IDs */

extern void func_800541B0(s32, s32, s32);
extern s32 func_8007E150(f32, f32, f32, f32, s32, Object **);
extern M132Weapons *func_80047C1C(s32, s32);
extern M132WeaponInfo *func_8003B500(s32 weapon);
extern void func_80001248(s32 sound, s32 arg1); /* DKR amSndPlay */

s32 D_m132_00310 = 132; /* the module's number */

void func_m132_00000(Object *obj, M132Setup *setup) {
    M132Pickup *p = OBJ_STATE(obj, M132Pickup);
    s32 i;

    i = obj->header->numberOfModelIds - 1;
    if (setup->model < i) {
        i = setup->model;
    }
    if (i < 0) {
        i = 0;
    }
    obj->modelIndex = i;
    if (setup->common.objectID != 0x88) {
        for (i = 0; i < 15 && setup->common.objectID != D_800A0CB0[i]; i++) {
        }
    }
    p->weapon = i;
    if (setup->common.objectID == 0x88) {
        if (p->weapon >= 8) {
            obj->trans.scale = 1.5f;
        } else {
            obj->trans.scale = 0.2f;
        }
    }
    p->respawn = setup->respawn * 60;
    p->timer = 0;
    p->fade = 0xFF;
}

void func_m132_000E4(Object *obj, s32 updateRate) {
    M132Pickup *p = OBJ_STATE(obj, M132Pickup);
    s32 message;
    Object *list[8];
    M132PlayerData *pd;
    M132Weapons *w;
    M132WeaponInfo *info;
    s32 n;

    func_800541B0(D_m132_00310, 2, 2);
    obj->trans.rotation.y_rotation += updateRate << 7;
    n = func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 40.0f, 0, list);
    if (p->fade == 0) {
        p->timer -= updateRate;
        if (p->timer <= 0) {
            p->timer = 0;
            if (n == 0) {
                p->fade = updateRate;
            }
        }
    } else {
        p->fade += updateRate * 4;
        if (p->fade >= 0x100) {
            p->fade = 0xFF;
        }
        if (n != 0) {
            pd = list[0]->unk68;
            w = func_80047C1C(pd->unk0, pd->unk1);
            info = func_8003B500(p->weapon);
            if (!(w->owned & (1 << p->weapon))) {
                w->owned |= 1 << p->weapon;
                w->count++;
                w->maxAmmo[p->weapon] = info->maxAmmo;
                w->ammo[p->weapon] = info->ammo;
                w->unk54[p->weapon] = info->unk3;
                message = p->weapon + 0x39;
            } else {
                w->ammo[p->weapon] += info->ammoPickup;
                if (w->maxAmmo[p->weapon] < w->ammo[p->weapon]) {
                    w->ammo[p->weapon] = w->maxAmmo[p->weapon];
                }
                message = p->weapon + 0x48;
            }
            if (p->respawn != 0) {
                p->timer = p->respawn;
                p->fade = 0;
            } else {
                func_800079B8(obj);
            }
            func_80001248(0x8A, 0);
            func_m006_011E0(pd, message);
        }
    }
}
