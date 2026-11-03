/*
 * Runlink module 135: the object behaviours of the multiplayer menu room: the
 * room camera and the point it aims at, the second camera on the screen, and
 * the player selectors (their mode, sounds and joint angles).
 */
#include "module.h"

/* The aim point's behaviour data */
typedef struct M135Aim {
    /* 0x00 */ s32 state;
} M135Aim;

/* The camera's behaviour data */
typedef struct M135Camera {
    /* 0x00 */ s32 flags;
    /* 0x04 */ Object *targets[9];
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x44 - 0x2C];
    /* 0x44 */ s32 sound; /* a playing sound's handle, or 0 */
    /* 0x48 */ u8 pad48[2];
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ s16 yaw;
    /* 0x4E */ s16 pitch;
    /* 0x50 */ u8 unk50;
    /* 0x51 */ s8 target; /* the one looked at */
    /* 0x52 */ s8 lastTarget;
    /* 0x53 */ u8 pad53;
    /* 0x54 */ u8 unk54;
} M135Camera;

extern void *get_settings(void);
extern f32 func_80097140(f32 x);        /* sqrtf */
extern s32 func_80049C84(f32 a, f32 b); /* the angle of (a, b) */
extern s32 rl_m135_02A78[]; /* per target: the flag it needs, and one that hides it */
extern s32 rl_m135_02A84[];

/* The screen camera's behaviour data */
typedef struct M135ScreenCam {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ s32 sound;
} M135ScreenCam;

/* A player selector's behaviour data */
typedef struct M135Selector {
    /* 0x00 */ s32 sound;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 pad11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 mode;
    /* 0x14 */ s8 unk14;
    /* 0x15 */ u8 pad15;
    /* 0x16 */ s8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 pad18;
    /* 0x19 */ s8 unk19;
    /* 0x1A */ u8 unk1A;
} M135Selector;

/* A player selector's level entry */
typedef struct M135SelectorEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
} M135SelectorEntry;

extern void sound_stop(s32 handle); /* stops a sound */

static s16 sJointAngles[16];

void func_m135_00000(Object *obj, LevelObjectEntryCommon *entry) {
    M135Aim *aim = obj->unk68;

    aim->state = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_00010);

void func_m135_00298(Object *obj, LevelObjectEntryCommon *entry) {
    M135Camera *cam = obj->unk68;

    cam->unk4A = 0;
    cam->unk50 = 0;
    cam->unk54 = 40;
    cam->unk28 = 0.0f;
}

INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_002BC);

/* Turns the camera to the next target that can be shown (dir 0: forwards); TRUE if it changed */
s32 func_m135_01754(Object *obj, s32 dir) {
    s32 prev;
    f32 dx;
    f32 dy;
    f32 dz;
    M135Camera *cam = obj->unk68;
    Object *t;

    get_settings();
    prev = cam->target;
    if (cam->lastTarget >= 0) {
        if (dir == 0) {
        do {
            cam->target++;
            if (cam->lastTarget < cam->target) {
                cam->target = 0;
            }
        } while (cam->targets[cam->target] == NULL || !game_flag_get(rl_m135_02A78[cam->target]) ||
                 game_flag_get(rl_m135_02A84[cam->target]));
        } else {
            do {
                cam->target--;
                if (cam->target < 0) {
                    cam->target = cam->lastTarget;
                }
            } while (cam->targets[cam->target] == NULL || !game_flag_get(rl_m135_02A78[cam->target]) ||
                     game_flag_get(rl_m135_02A84[cam->target]));
        }
    } else {
        cam->target = 0;
    }
    t = cam->targets[cam->target];
    if (t != NULL) {
        dx = t->trans.x_position - obj->trans.x_position;
        dy = t->trans.y_position - obj->trans.y_position;
        dz = t->trans.z_position - obj->trans.z_position;
        cam->yaw = func_80049C84(dx, dz);
        cam->pitch = -func_80049C84(dy, func_80097140(dx * dx + dy * dy + dz * dz));
        if (prev != cam->target) {
            cam->flags &= ~1;
            cam->flags |= dir;
        }
    }
    return prev != cam->target;
}

void func_m135_019CC(Object *obj) {
    M135Camera *cam = obj->unk68;

    if (cam->sound != 0) {
        sound_stop(cam->sound);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_01A04);

void func_m135_02024(Object *obj) {
    M135ScreenCam *cam = obj->unk68;

    if (cam->sound != 0) {
        sound_stop(cam->sound);
    }
}

/* The first object with behaviour 0x46 (the menu camera), or NULL */
Object *func_m135_0205C(void) {
    s32 i;
    Object **objs;
    Object *obj;
    s32 first;
    s32 count;

    objs = func_80005E98(&first, &count);
    for (i = first; i < count; i++) {
        obj = objs[i];
        if (obj->behaviorId == 0x46) {
            return obj;
        }
    }
    return NULL;
}

void func_m135_02188(Object *obj, M135SelectorEntry *entry) {
    M135Selector *sel = obj->unk68;

    sel->unk4 = entry->unkA;
    sel->unk6 = entry->unkC;
    sel->unkC = entry->unk10;
    sel->unkA = entry->unk12;
    sel->unk8 = sel->unkA;
    sel->unk12 = entry->unkF;
    sel->unk10 = entry->unkE;
    obj->modelIndex = entry->unkE;
    obj->trans.rotation.y_rotation = sel->unk4;
}

INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_021F8);

void func_m135_0270C(Object *obj, s32 mode, s32 arg2, s32 turn, s32 arg4) {
    M135Selector *sel = obj->unk68;

    if (turn == 1) {
        sel->unk16 = -1;
        sel->unkA -= 0x8000;
    } else if (turn == 2) {
        sel->unk16 = 1;
        sel->unkA += 0x8000;
    }
    if (sel->mode == 1 && mode == 0) {
        if (sel->unk8 < 0) {
            sel->unk8 += 0x8000;
        }
    }
    if (mode == 2 && sel->mode == 1) {
        sel->unk17 = 1;
        func_80001248(0xFD, 0);
    } else if (mode == 1 && sel->mode == 2) {
        sel->unk17 = 1;
        func_80001248(0xFE, 0);
    }
    sel->unk14 = arg2;
    sel->mode = mode;
    sel->unk19 = arg4;
    if (sel->unk1A == 0) {
        sel->unk1A = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_02834);

s16 *func_m135_02958(Object *obj) {
    M135Selector *sel = obj->unk68;

    sJointAngles[0] = 0x2008;
    sJointAngles[1] = sel->unk8;
    sJointAngles[2] = 0x2006;
    sJointAngles[3] = 0;
    sJointAngles[4] = 0x200A;
    sJointAngles[5] = 0;
    sJointAngles[6] = 0x1000;
    return sJointAngles;
}

void func_m135_02998(Object *obj) {
    M135Selector *sel = obj->unk68;

    if (sel->sound != 0) {
        sound_stop(sel->sound);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m135", data);

INCLUDE_ASM("asm/nonmatchings/modules/m135", rodata);
