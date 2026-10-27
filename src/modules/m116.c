/*
 * Runlink module 116: two race-course behaviours. A pickup that spins, fades
 * in, is collected by a player (reporting it to module 31) and respawns or
 * disappears; and an obstacle that reports the players it touches to module 31
 * and goes away (with a sound) once touched, or after a while.
 */
#include "module.h"

typedef struct M116Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 respawn; /* seconds; 0: respawns */
} M116Setup;

typedef struct M116State {
    /* 0x0 */ s32 timer;
    /* 0x4 */ s32 alpha; /* above 0xFF: not yet shown */
} M116State;

typedef struct M116ObstacleSetup {
    LevelObjectEntryCommon common;
    /* 0x0A */ s16 lifetime;
    /* 0x0C */ s32 owner;
} M116ObstacleSetup;

typedef struct M116Obstacle {
    /* 0x0 */ s32 timer;
    /* 0x4 */ s32 owner; /* the object it ignores while it touches it */
} M116Obstacle;

extern s8 D_800A5034; /* which pickups the race has */
extern s8 D_800A503C;
extern void func_80001248(s32 sound, s32 arg1);
extern s32 func_8007E150(f32 x, f32 y, f32 z, f32 radius, s32 arg4, void *list);
extern void func_8005F5AC(Object *obj, s32 arg1);
extern void func_m031_01B7C(s32 player);
extern void func_m031_01BC0(s32 player);
extern void func_m031_01BF0(s32 player);
extern void func_m031_01ED8(s32 obj);
extern void func_m031_02064(s32 obj);

void func_m116_00000(Object *obj, M116Setup *setup) {
    if (setup->respawn == 0) {
        OBJ_STATE(obj, M116State)->timer = 0;
        OBJ_STATE(obj, M116State)->alpha = 0x1FF;
    } else {
        OBJ_STATE(obj, M116State)->timer = setup->respawn * 60;
        OBJ_STATE(obj, M116State)->alpha = 0xFF;
    }
}

void func_m116_00038(Object *obj, s32 updateRate) {
    s32 pad[2];
    s32 touched;
    s8 *player;
    Object *hit;

    if (OBJ_STATE(obj, M116State)->alpha >= 0x100) {
        if (obj->objectID == 0x275) {
            if (!(D_800A5034 & 2)) {
                func_800079B8(obj);
                return;
            }
        } else if (obj->objectID == 0x276) {
            if (!(D_800A5034 & 4)) {
                func_800079B8(obj);
                return;
            }
        } else if (!(D_800A5034 & 1)) {
            func_800079B8(obj);
            return;
        }
        OBJ_STATE(obj, M116State)->alpha = 0xFF;
    }
    if (OBJ_STATE(obj, M116State)->alpha != 0) {
        OBJ_STATE(obj, M116State)->alpha += updateRate * 8;
        if (OBJ_STATE(obj, M116State)->alpha >= 0x100) {
            OBJ_STATE(obj, M116State)->alpha = 0xFF;
        }
    }
    if (D_800A503C == 0) {
        obj->trans.rotation.y_rotation += updateRate << 9;
    } else {
        obj->trans.rotation.x_rotation += updateRate << 9;
    }
    touched = func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 64.0f, 1, &hit);
    if (OBJ_STATE(obj, M116State)->alpha == 0) {
        OBJ_STATE(obj, M116State)->timer += updateRate;
        if (touched == 0 && OBJ_STATE(obj, M116State)->timer >= 360) {
            OBJ_STATE(obj, M116State)->timer = 0;
            OBJ_STATE(obj, M116State)->alpha = 1;
        }
    } else if (touched != 0) {
        player = hit->unk68;
        switch (obj->objectID) {
            case 0x275:
                func_m031_01BC0(*player);
                break;
            case 0x276:
                func_m031_01BF0(*player);
                break;
            default:
                func_m031_01B7C(*player);
                break;
        }
        if (D_800A503C != 0) {
            func_80001248(0x193, 0);
        } else {
            func_80003410(0x193, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        }
        if (OBJ_STATE(obj, M116State)->timer == 0) {
            OBJ_STATE(obj, M116State)->alpha = 0;
        } else {
            func_800079B8(obj);
        }
    } else if (OBJ_STATE(obj, M116State)->timer != 0) {
        OBJ_STATE(obj, M116State)->timer -= updateRate;
        if (OBJ_STATE(obj, M116State)->timer <= 0) {
            func_800079B8(obj);
            return;
        }
        if (OBJ_STATE(obj, M116State)->timer < 64) {
            OBJ_STATE(obj, M116State)->alpha = OBJ_STATE(obj, M116State)->timer * 4;
        }
    }
}

void func_m116_002B0(Object *obj, M116ObstacleSetup *setup) {
    M116Obstacle *o = obj->unk68;

    o->timer = setup->lifetime;
    o->owner = setup->owner;
    OBJ_STATE(obj, M116State)->alpha = 0xFF;
    if (D_800A503C == 0) {
        obj->trans.scale *= 0.67f;
    }
}

#ifdef NON_MATCHING
/* Differs only in register allocation: s0/s1 (counter, list pointer) and s4/s5 (obj, found) are swapped */
void func_m116_002FC(Object *obj, s32 updateRate) {
    M116Obstacle *o;
    s32 found;
    s32 list[5];
    s32 i;
    s32 hits;

    hits = func_8007E150(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 64.0f, 0, list);
    o = obj->unk68;
    found = 0;
    if (hits != 0) {
        i = hits;
        hits = 0;
        while (i--) {
            if (o->owner == list[i]) {
                found = 1;
            } else {
                hits++;
                if (obj->objectID == 0x278) {
                    func_m031_01ED8(list[i]);
                } else {
                    func_m031_02064(list[i]);
                }
            }
        }
    }
    if (!found) {
        o->owner = 0;
    }
    if (hits == 0) {
        o->timer -= updateRate;
        if (o->timer <= 0) {
            func_800079B8(obj);
            return;
        }
        if (o->timer < 64) {
            OBJ_STATE(obj, M116State)->alpha = o->timer * 4;
        }
    } else {
        switch (obj->objectID) {
            case 0x278:
                func_80003410(0x21D, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                break;
            case 0x1FB:
                func_80001248(0x21D, 0);
                break;
            case 0x279:
                obj->particleEmittersEnabled = 1;
                func_8005F5AC(obj, 2);
                if (D_800A503C != 0) {
                    func_80001248(0x22, 0);
                } else {
                    func_80003410(0x22, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
                }
                break;
        }
        func_800079B8(obj);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m116", func_m116_002FC);
#endif
