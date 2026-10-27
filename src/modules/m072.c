/*
 * Runlink module 72: ammunition pickups. Each kind tops up the ammunition
 * (or the capacity) of a weapon that the player already has, then respawns
 * after a delay or is freed. One kind (0xE7, a capacity upgrade) is taken
 * only once: it sets a game flag, and is freed at once if that flag is set.
 */
#include "module.h"

typedef struct M072Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s8 respawn; /* in seconds; 0: no respawn */
    /* 0x0B */ u8 weapon;
    /* 0x0C */ u8 y_rotation;
    /* 0x0D */ u8 padD;
    /* 0x0E */ u16 flag;
} M072Setup;

/* The pickup's data (its object's unk68) */
typedef struct M072Pickup {
    /* 0x0 */ s16 objectID;
    /* 0x2 */ u8 respawn;
    /* 0x3 */ u8 weapon;
    /* 0x4 */ s16 timer; /* until it comes back */
    /* 0x6 */ s16 fade;  /* 0: hidden; counts up to 0xFF */
    /* 0x8 */ u16 flag;
} M072Pickup;

/* The player's object data (its unk68) */
typedef struct M072PlayerData {
    /* 0x0 */ s8 player;
    /* 0x1 */ s8 unk1;
} M072PlayerData;

/* What func_80047C1C returns: the player's weapons */
typedef struct M072Weapons {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 owned; /* bit per weapon */
    /* 0x0C */ u8 padC[0x14 - 0xC];
    /* 0x14 */ u16 ammo[16];
    /* 0x34 */ u16 maxAmmo[16];
} M072Weapons;

/* What func_8003B500 returns: a weapon's properties */
typedef struct M072WeaponInfo {
    /* 0x00 */ u8 pad0[0x26];
    /* 0x26 */ s16 capacityPickup;
    /* 0x28 */ u8 pad28[0x2A - 0x28];
    /* 0x2A */ s16 ammoPickup;
    /* 0x2C */ s16 bigAmmoPickup;
    /* 0x2E */ s16 maxCapacity;
} M072WeaponInfo;

extern void func_800541B0(s32, s32, s32);
extern s32 func_8007E150(f32, f32, f32, f32, s32, Object **);
extern M072Weapons *func_80047C1C(s32 player, s32 arg1);
extern M072WeaponInfo *func_8003B500(s32 weapon);
extern void func_80001248(s32 sound, s32 arg1); /* DKR amSndPlay */

s32 D_m072_00480 = 72; /* the module's number */

void func_m072_00000(Object *obj, M072Setup *setup) {
    M072Pickup *p = obj->unk68;

    p->objectID = setup->common.objectID;
    p->respawn = setup->respawn;
    p->weapon = setup->weapon;
    p->flag = setup->flag;
    p->timer = 0;
    p->fade = 0xFF;
    obj->trans.rotation.y_rotation = setup->y_rotation << 8;
    func_8001149C(obj, 0, 0.0f);
}

void func_m072_00064(Object *obj, s32 updateRate) {
    M072Pickup *p;
    s32 n;
    s32 i;
    M072PlayerData *pd;
    M072Weapons *w;
    Object *list[8];
    M072WeaponInfo *info;
    s32 weapon;
    s32 message;
    s32 take;

    func_800541B0(D_m072_00480, 2, 2);
    p = obj->unk68;
    if (p->objectID == 0xE7 && func_80047ED0(p->flag + 0x80)) {
        func_800079B8(obj);
        return;
    }
    if (p->objectID == 0x17E || p->objectID == 0xE7) {
        obj->trans.rotation.y_rotation += updateRate << 7;
    }
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
        for (i = 0; i < n;) {
            pd = list[i]->unk68;
            w = func_80047C1C(pd->player, pd->unk1);
            weapon = p->weapon;
            info = func_8003B500(weapon);
            if ((w->owned & (1 << p->weapon)) &&
                (p->objectID != 0xE7 || !(info->maxCapacity < w->maxAmmo[weapon] + info->capacityPickup))) {
                take = 1;
            } else {
                take = 0;
            }
            if (take) {
                message = weapon;
                i = n;
                switch (p->objectID) {
                    case 0xCF:
                        message = weapon + 0x48;
                        w->ammo[weapon] += info->ammoPickup;
                        if (w->maxAmmo[weapon] < w->ammo[weapon]) {
                            w->ammo[weapon] = w->maxAmmo[weapon];
                        }
                        func_80001248(0x1E9, 0);
                        break;
                    case 0x17E:
                        message = weapon + 0x57;
                        w->ammo[weapon] += info->bigAmmoPickup;
                        if (w->maxAmmo[weapon] < w->ammo[weapon]) {
                            w->ammo[weapon] = w->maxAmmo[weapon];
                        }
                        func_80001248(0x1E9, 0);
                        break;
                    case 0xE7:
                        w->maxAmmo[weapon] += info->capacityPickup;
                        message = weapon + 0x75;
                        func_80047EFC(p->flag + 0x80, 1);
                        func_80001248(0x1EA, 0);
                        break;
                    case 0xE8:
                        w->ammo[weapon] = w->maxAmmo[weapon];
                        message = weapon + 0x66;
                        func_80001248(0x1E9, 0);
                        break;
                }
                func_m006_011E0(pd, message);
                if (p->respawn != 0) {
                    p->timer = p->respawn * 60;
                    p->fade = 0;
                } else {
                    func_800079B8(obj);
                }
            } else {
                i++;
            }
        }
    }
}

/* The pickup's opacity for the given player: dimmed when the player cannot take it */
u8 func_m072_003BC(Object *obj, s32 player) {
    M072Pickup *p = obj->unk68;
    M072Weapons *w;
    register Object *o;
    register M072PlayerData *pd;
    register M072WeaponInfo *info;

    o = func_8000E4D0(player);
    if (o != NULL) {
        pd = o->unk68;
        w = func_80047C1C(player, pd->unk1);
        info = func_8003B500(p->weapon);
        if (!(w->owned & (1 << p->weapon)) ||
            (p->objectID == 0xE7 && info->maxCapacity < w->maxAmmo[p->weapon] + info->capacityPickup)) {
            return p->fade >> 2;
        }
    }
    return p->fade;
}
