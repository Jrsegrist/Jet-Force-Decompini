/*
 * Runlink module 22: the control plate (a level object that hands the player
 * control of a helper character) and the helper's task: its states, its goals,
 * the pause screen with its letterbox, and the plate's camera.
 */
#include "module.h"

/* The plate's behaviour data */
typedef struct M022Plate {
    /* 0x00 */ f32 radius;
    /* 0x04 */ f32 yMin;
    /* 0x08 */ f32 yMax;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 sound; /* a positioned sound's handle, or 0 */
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 pad18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ s8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ s32 unk1C;
} M022Plate;

/* The plate's level entry */
typedef struct M022PlateEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 radius;
    /* 0x0B */ s8 yMin;
    /* 0x0C */ s8 yMax;
    /* 0x0D */ u8 unkD;
} M022PlateEntry;

/* The object header as the plate reads it */
typedef struct M022Header {
    /* 0x00 */ u8 pad0[0xAC];
    /* 0x0AC */ s32 *unkAC;
} M022Header;

/* The helper (the player-like object the task runs on) */
typedef struct M022Helper {
    /* 0x000 */ u8 pad0[0x10D];
    /* 0x10D */ u8 state;
} M022Helper;

extern void func_80003614(s32 handle);             /* stops a positioned sound */
extern ObjectTransform *func_80041DE0(void);       /* the camera */
extern void func_80045B98(s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 speed); /* letterbox */
extern void set_game_paused(s32 paused);
extern f32 func_80097140(f32 x);                   /* sqrtf */
extern s32 func_80049C84(f32 a, f32 b);            /* the angle of (a, b) */

extern s32 D_800A320C;
extern s32 D_800A3210[3];
extern u8 D_800A3208;
extern s16 D_801046EE;
extern s16 D_801046F4;

/* .data, still assembly */
extern Object *rl_m022_03418; /* the helper's plate object */
extern u8 rl_m022_03424;
extern s16 rl_m022_03428;
extern s16 rl_m022_0342C;
extern s16 rl_m022_03430;
extern s8 rl_m022_03434;
extern u8 rl_m022_03438;
extern u8 rl_m022_0343C;
extern u8 rl_m022_03440;

/* .bss, still assembly: the plate camera */
extern f32 rl_m022_035B0;
extern f32 rl_m022_035B4;
extern f32 rl_m022_035B8;
extern f32 rl_m022_035BC;
extern f32 rl_m022_035C0;
extern s16 rl_m022_035C4;
extern s16 rl_m022_035C6;
extern s16 rl_m022_035C8;
#define sPlateCamX rl_m022_035B0
#define sPlateCamY rl_m022_035B4
#define sPlateCamZ rl_m022_035B8
#define sPlateCamDist rl_m022_035BC /* the distance to the camera, flat */
#define sPlateCamDy rl_m022_035C0
#define sPlateCamYaw rl_m022_035C4
#define sPlateCamUnk16 rl_m022_035C6
#define sPlateCamUnk18 rl_m022_035C8

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_00000);

void func_m022_00154(Object *obj, M022PlateEntry *entry) {
    M022Plate *plate = obj->unk68;

    plate->radius = entry->radius;
    plate->yMin = obj->trans.y_position + entry->yMin;
    plate->yMax = obj->trans.y_position + entry->yMax;
    plate->unkC = 0;
    plate->sound = 0;
    plate->unk14 = 0;
    plate->unk15 = entry->unkD;
    plate->unk16 = 0;
    plate->unk17 = 0;
    plate->unk19 = 0;
    plate->unk1A = -1;
    plate->unk1B = 1;
    plate->unk1C = 0;
    D_800A320C = ((M022Header *) obj->header)->unkAC[0];
    D_800A3210[0] = ((M022Header *) obj->header)->unkAC[1];
    D_800A3210[1] = ((M022Header *) obj->header)->unkAC[2];
    D_800A3210[2] = ((M022Header *) obj->header)->unkAC[3];
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    OBJ_STATE(obj, s32)[0] = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_002A4);

void func_m022_00EA0(Object *obj) {
    M022Plate *plate;

    if (rl_m022_03418 != NULL) {
        func_800079B8(rl_m022_03418);
        rl_m022_03418 = NULL;
    }
    D_800A320C = 0;
    D_800A3210[0] = 0;
    D_800A3210[1] = 0;
    D_800A3210[2] = 0;
    plate = obj->unk68;
    if (plate->sound != 0) {
        func_80003614(plate->sound);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_00F18);

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_0114C);

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_018AC);

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_020EC);

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_022C4);

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_02310);

void func_m022_023FC(M022Helper *helper) {
    helper->state = 3;
    rl_m022_03424 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_02410);

/* A task goal: a count to reach, or flags to have */
typedef struct M022Goal {
    /* 0x0 */ u8 unk0;
    /* 0x1 */ u8 kind; /* 0, 1: the counters; else the flags */
    /* 0x2 */ u16 value;
} M022Goal;

/* The task's goals: two lists */
typedef struct M022Goals {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 count1;
    /* 0x03 */ u8 count2;
    /* 0x04 */ M022Goal list1[3];
    /* 0x10 */ M022Goal list2[1];
} M022Goals;

extern u8 D_800FD790;  /* counters the goals count */
extern u8 D_800FD791;
extern u16 D_800FD792; /* flags */
extern M022Goals *rl_m022_03410;
extern u8 rl_m022_03420; /* a bit per goal reached */

/* Counts a thing done (kind 0 or 1) or sets flag `bit`, then sees which goals are reached */
void func_m022_02430(s32 bit, s32 kind) {
    M022Goals *g;
    M022Goal *e;
    s32 i;
    s32 mask;

    switch (kind) {
        case 0:
            D_800FD790++;
            break;
        case 1:
            D_800FD791++;
            break;
    }
    if (bit >= 0) {
        D_800FD792 |= 1 << bit;
    }
    rl_m022_03420 = 0;
    mask = 1;
    g = rl_m022_03410;
    bit = g->count1; /* the goals left in the list */
    e = g->list1;
    i = 2;
    while (i--) {
        while (bit--) {
            switch (e->kind) {
                case 0:
                    if (D_800FD790 >= e->value) {
                        rl_m022_03420 |= mask;
                    }
                    break;
                case 1:
                    if (D_800FD791 >= e->value) {
                        rl_m022_03420 |= mask;
                    }
                    break;
                default:
                    if ((D_800FD792 & e->value) == e->value) {
                        rl_m022_03420 |= mask;
                    }
                    break;
            }
            e++;
            mask <<= 1;
        }
        bit = g->count2;
        e = g->list2;
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_025B8);

/* Pauses the task: brings the letterbox in */
void func_m022_02990(void) {
    rl_m022_03434 = 2;
    rl_m022_03424 = 0;
    rl_m022_03430 = 0;
    rl_m022_0342C = 1;
    rl_m022_03438 = 2;
    rl_m022_0343C = 3;
    rl_m022_03440 = 0;
    func_80045B98(0, 0, 320, 240, 24, 30, 168, 138, 32);
    set_game_paused(1);
}

extern s32 func_80045DC0(void);
extern s32 func_80043510(s32 player); /* the buttons pressed */
extern void func_80001248(s32 sound, s32 arg1);
extern void transition_begin(void *transition);
extern u8 D_800A4FC8; /* a second player plays */
extern u8 rl_m022_033FC[];

/* The task's pause screen: A/start leave it, B and Z move between its pages */
void func_m022_02A34(void) {
    Object *player;
    s32 buttons;
    u8 *data;

    if (func_80045DC0()) {
        return;
    }
    rl_m022_03430 = 1;
    buttons = func_80043510(0);
    if (D_800A4FC8) {
        buttons |= func_80043510(1);
    }
    if (*(s8 *) &rl_m022_03438 == 2) {
        if (buttons & 0x9000) {
            func_80001248(0x16B, 0);
            transition_begin(rl_m022_033FC);
            func_80045B98(24, 30, 168, 138, 0, 0, 320, 240, -1);
            rl_m022_03430 = 0;
            rl_m022_0342C = 0;
            set_game_paused(0);
        } else if (buttons & 0x4000) {
            func_80001248(0x16B, 0);
            rl_m022_03438 = 5;
            rl_m022_0343C = 2;
            rl_m022_03440 = 0;
        } else if (buttons & 0x2000) {
            func_80001248(0x16C, 0);
            if (++*(u8 *) &rl_m022_03434 >= 3) {
                *(u8 *) &rl_m022_03434 = 0;
            }
        }
    } else if (buttons & 0x9000) {
        func_80001248(0x16B, 0);
        player = func_8000E4D0(0);
        if (player != NULL && (data = player->unk68) != NULL) {
            data[0x10D] = 5;
        }
        set_game_paused(0);
    } else if (buttons & 0x4000) {
        func_80001248(0x16B, 0);
        rl_m022_03438 = 2;
        rl_m022_0343C = 3;
        rl_m022_03440 = 0;
    }
}

/* Places the plate camera above the plate, turned towards the camera (useAngle) or along `yaw` */
void func_m022_02C0C(Object *obj, s32 yaw, s32 useAngle) {
    f32 dx;
    f32 dz;
    ObjectTransform *cam;

    cam = func_80041DE0();
    sPlateCamX = obj->trans.x_position;
    sPlateCamY = obj->trans.y_position + 50.0f;
    sPlateCamZ = obj->trans.z_position;
    dx = cam->x_position - obj->trans.x_position;
    dz = cam->z_position - obj->trans.z_position;
    sPlateCamDist = func_80097140(dx * dx + dz * dz);
    sPlateCamDy = cam->y_position - sPlateCamY;
    if (useAngle) {
        sPlateCamYaw = 0x8000 - func_80049C84(dx, dz);
    } else {
        sPlateCamYaw = yaw;
    }
    sPlateCamUnk16 = 0;
    rl_m022_03428 = 300;
    sPlateCamUnk18 = useAngle;
}

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_02D14);

INCLUDE_ASM("asm/nonmatchings/modules/m022", func_m022_02F38);

/* Ends the task: frees the plate object and takes the letterbox out */
void func_m022_03150(void) {
    D_800A3208 = 0;
    D_801046EE = 0;
    D_801046F4 = 0;
    rl_m022_03430 = 0;
    rl_m022_0342C = 0;
    if (rl_m022_03418 != NULL) {
        func_800079B8(rl_m022_03418);
    }
    func_80045B98(24, 30, 168, 138, 0, 0, 320, 240, -1);
}

INCLUDE_ASM("asm/nonmatchings/modules/m022", data);

INCLUDE_ASM("asm/nonmatchings/modules/m022", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m022", bss);
