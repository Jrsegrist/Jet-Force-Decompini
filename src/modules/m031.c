/*
 * Runlink module 31: races. It keeps a ring of up to 256 checkpoint objects
 * (each with a plane and a radius), up to four racers' records (whether a
 * racer is computer-driven, its boosts and its pick-up), and the countdown;
 * the race logic and drawing are still in assembly.
 */
#include "module.h"

/* A checkpoint object's data (obj->unk68) */
typedef struct M031Checkpoint {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 radiusSq;
    /* 0x10 */ f32 nx; /* the plane through the checkpoint */
    /* 0x14 */ f32 ny;
    /* 0x18 */ f32 nz;
    /* 0x1C */ f32 d;
} M031Checkpoint;

/* A racer's record (size 0x28) */
typedef struct M031Racer {
    /* 0x00 */ u8 pad0[0x15];
    /* 0x15 */ u8 computer;
    /* 0x16 */ u8 pad16[0x19 - 0x16];
    /* 0x19 */ u8 boosts;
    /* 0x1A */ u8 pickup; /* 1: oil, 2: bomb */
    /* 0x1B */ u8 pad1B[0x28 - 0x1B];
} M031Racer;

extern s8 D_800A4FB4;
extern u8 D_800A4FD0;
extern s8 D_800A5024;
extern s8 D_800A502C;
extern s8 D_800A5034;
extern s8 D_800A5038;
extern s8 D_800A5044;
extern s8 D_800A5048;
extern s8 D_800A504C;
extern s8 D_800A5050;

/* A player's state block (obj->unk68), as used here */
typedef struct M031Player {
    /* 0x000 */ s8 player;
    /* 0x001 */ u8 pad1[0x11C - 0x1];
    /* 0x11C */ s16 unk11C;
    /* 0x11E */ u8 pad11E[0x13C - 0x11E];
    /* 0x13C */ s16 unk13C;
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 pad13F[0x576 - 0x13F];
    /* 0x576 */ s8 unk576;
    /* 0x577 */ u8 pad577[0x5E0 - 0x577];
    /* 0x5E0 */ s32 sound1;
    /* 0x5E4 */ u8 pad5E4[0x5E8 - 0x5E4];
    /* 0x5E8 */ s32 sound2;
} M031Player;

/* A player slot of D_800FED50 (0xA0 bytes) */
typedef struct M031Slot {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 pad13[0x1B - 0x13];
    /* 0x1B */ u8 pad1B_5 : 3;
    /* 0x1B */ u8 index : 5;
    /* 0x1C */ u8 pad1C[0xA0 - 0x1C];
} M031Slot;

extern M031Slot D_800FED50[4];
extern s16 D_800FF1B8[4];
extern u8 D_800A4FC0;
extern u8 D_800A4FD4;
extern s8 D_800A4FEC;
extern s8 D_800A503C;
extern s32 gMenuTimer;
extern void func_80047A20(void);
extern void func_80003614(s32 handle);
extern s32 func_80055694(s32 id);
extern void func_80055A00(s32 handle);
extern void rumble_raise(s32 player, s32 arg1, f32 arg2);

/* The module's .data (still assembly) */
extern s16 rl_m031_040FC[]; /* a range of sounds per level */
#define sHitSounds rl_m031_040FC
extern s32 rl_m031_03F78, rl_m031_03F90, rl_m031_03FB4, rl_m031_03FCC, rl_m031_03FE4, rl_m031_03FFC, rl_m031_04014,
    rl_m031_0402C, rl_m031_04044; /* the race's sounds */

/* The module's .bss (the assembly functions reach it by offset) */
static s32 sBss0;
static s32 sCountdown;
static Object *sCheckpoints[256];
static u8 sBss408[0x498 - 0x408];
static M031Racer sRacers[4];

/* Whether racer n is computer-driven */
s32 func_m031_00000(s32 n) {
    if (n >= 0 && n < 4 && sRacers[n].computer) {
        return TRUE;
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_00044);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_00230);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_00364);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_005AC);

/* The checkpoint before `index` in the ring */
Object *func_m031_0066C(s32 index) {
    s32 i = index;
    Object *checkpoint;

    do {
        i--;
        if (i < 0) {
            i = 255;
        }
        checkpoint = sCheckpoints[i];
    } while (i != index && checkpoint == NULL);
    return checkpoint;
}

/* The checkpoint after `index` in the ring */
Object *func_m031_006AC(s32 index) {
    s32 i = index;
    Object *checkpoint;

    do {
        i++;
        if (i >= 256) {
            i = 0;
        }
        checkpoint = sCheckpoints[i];
    } while (i != index && checkpoint == NULL);
    return checkpoint;
}

/* Which side of a checkpoint a point is on, if within its radius (else 0) */
s32 func_m031_006F0(Object *checkpoint, f32 x, f32 y, f32 z, s32 flat) {
    M031Checkpoint *c;
    f32 dx;
    f32 dz;
    f32 dy;
    f32 distSq;
    f32 dist;

    if (checkpoint != NULL) {
        dx = checkpoint->trans.x_position - x;
        dz = checkpoint->trans.z_position - z;
        c = checkpoint->unk68;
        distSq = dx * dx + dz * dz;
        if (!flat) {
            dy = checkpoint->trans.y_position - y;
            distSq = dy * dy;
        }
        if (distSq < c->radiusSq) {
            dist = c->nx * x + c->ny * y + c->nz * z + c->d;
            if (dist < 0.0f) {
                return 1;
            }
            return -1;
        }
    }
    return 0;
}

/* The distance of a point from a checkpoint's plane */
f32 func_m031_007B4(Object *checkpoint, f32 x, f32 y, f32 z) {
    M031Checkpoint *c;
    f32 dist = 32768.0f;

    if (checkpoint != NULL) {
        c = checkpoint->unk68;
        dist = c->nx * x + c->ny * y + c->nz * z + c->d;
    }
    return dist;
}

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_0080C);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_00A3C);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_01150);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_017A0);

void func_m031_01B7C(s32 n) {
    M031Racer *racer;

    if (n >= 0 && n < 4) {
        racer = &sRacers[n];
        if (racer->boosts < 100) {
            racer->boosts++;
        }
    }
}

void func_m031_01BC0(s32 n) {
    if (n >= 0 && n < 4) {
        sRacers[n].pickup = 1;
    }
}

void func_m031_01BF0(s32 n) {
    if (n >= 0 && n < 4) {
        sRacers[n].pickup = 2;
    }
}

/* Uses one of racer n's boosts, if it has one */
s32 func_m031_01C20(s32 n) {
    M031Racer *racer;

    if (n >= 0 && n < 4) {
        racer = &sRacers[n];
        if (racer->boosts > 0) {
            racer->boosts--;
            return TRUE;
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_01C68);

/* Whether the countdown is over */
s32 func_m031_01EB0(void) {
    if (sCountdown >= 120) {
        return TRUE;
    }
    return FALSE;
}

/* A racer drives into an oil slick */
void func_m031_01ED8(Object *obj) {
    M031Player *p = obj->unk68;

    if (p != NULL) {
        p->unk13C = p->unk11C;
        p->unk576 = 30;
        p->unk13E = 2;
        if (D_800A503C != 0) {
            func_80001248(0x21C, 0);
        } else {
            if (p->sound1 != 0) {
                func_80003614(p->sound1);
            }
            if (p->sound2 != 0) {
                func_80003614(p->sound2);
            }
            func_80003410(0x21C, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, &p->sound1);
            if (p->player == 0 && D_800A4FD0 - D_800A4FD4 == 1) {
                func_80003410(func_80048E2C(sHitSounds[D_800A4FC0 * 2], sHitSounds[D_800A4FC0 * 2 + 1]),
                              obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, &p->sound2);
            } else {
                func_80003410(func_80048E2C(0xA0, 0xA9), obj->trans.x_position, obj->trans.y_position,
                              obj->trans.z_position, 4, &p->sound2);
            }
        }
        if (!func_m031_00000(p->player)) {
            rumble_raise(p->player, 40, 0.2f);
        }
    }
}

/* A racer hits a bomb */
void func_m031_02064(Object *obj) {
    M031Player *p = obj->unk68;

    if (p != NULL) {
        p->unk13C = p->unk11C;
        p->unk576 = 60;
        p->unk13E = 2;
        if (D_800A503C != 0) {
            func_80001248(0x21C, 0);
        } else {
            if (p->sound1 != 0) {
                func_80003614(p->sound1);
            }
            if (p->sound2 != 0) {
                func_80003614(p->sound2);
            }
            func_80003410(0x21C, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, &p->sound1);
            if (p->player == 0 && D_800A4FD0 - D_800A4FD4 == 1) {
                func_80003410(func_80048E2C(sHitSounds[D_800A4FC0 * 2], sHitSounds[D_800A4FC0 * 2 + 1]),
                              obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, &p->sound2);
            } else {
                func_80003410(func_80048E2C(0xA0, 0xA9), obj->trans.x_position, obj->trans.y_position,
                              obj->trans.z_position, 4, &p->sound2);
            }
        }
        if (!func_m031_00000(p->player)) {
            rumble_raise(p->player, 75, 0.2f);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_021F0);

/* Loads the race's sounds */
void func_m031_02388(void) {
    rl_m031_03F78 = func_80055694(0x266);
    rl_m031_03F90 = func_80055694(0x269);
    rl_m031_03FB4 = func_80055694(0x267);
    rl_m031_03FCC = func_80055694(0x268);
    rl_m031_03FE4 = func_80055694(0x26A);
    rl_m031_03FFC = func_80055694(0x26B);
    rl_m031_04014 = func_80055694(0x26C);
    rl_m031_0402C = func_80055694(0x26D);
    rl_m031_04044 = func_80055694(0x26E);
}

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_0242C);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_026B8);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_029FC);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_02B88);

/* Frees the race's sounds */
void func_m031_0361C(void) {
    func_80055A00(rl_m031_03FCC);
    rl_m031_03FCC = 0;
    func_80055A00(rl_m031_03FB4);
    rl_m031_03FB4 = 0;
    func_80055A00(rl_m031_03F78);
    rl_m031_03F78 = 0;
    func_80055A00(rl_m031_03F90);
    rl_m031_03F90 = 0;
    func_80055A00(rl_m031_03FE4);
    rl_m031_03FE4 = 0;
    func_80055A00(rl_m031_03FFC);
    rl_m031_03FFC = 0;
    func_80055A00(rl_m031_04014);
    rl_m031_04014 = 0;
    func_80055A00(rl_m031_0402C);
    rl_m031_0402C = 0;
    func_80055A00(rl_m031_04044);
    rl_m031_04044 = 0;
}

/* At the race's start */
void func_m031_036E8(s32 arg0) {
    s32 i;
    s32 index;

    func_80047A20();
    index = D_800A4FC0;
    for (i = 0; i < 4; i++) {
        D_800FF1B8[i] = 0;
        D_800FED50[i].unk12 = 3;
        D_800FED50[i].index = index;
        index = (index + 1) & 3;
    }
    D_800A5044 = D_800A5024;
    D_800A5048 = D_800A502C;
    D_800A504C = D_800A5034;
    D_800A5050 = D_800A5038;
    D_800A5024 = 0;
    D_800A502C = 3;
    D_800A5034 = 7;
    D_800A5038 = 3;
    D_800A503C = arg0;
    gMenuTimer = 0;
    D_800A4FEC = 0;
    D_800A4FD4 = 3;
    D_800A4FD0 = 4;
    D_800A4FB4 = 1;
}

/* At the race's end */
void func_m031_03888(void) {
    D_800A5024 = D_800A5044;
    D_800A502C = D_800A5048;
    D_800A5034 = D_800A504C;
    D_800A5038 = D_800A5050;
    D_800A4FD0 = 1;
    D_800A4FB4 = 0;
}

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_038E0);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_03AF8);

INCLUDE_ASM("asm/nonmatchings/modules/m031", data);

INCLUDE_ASM("asm/nonmatchings/modules/m031", rodata);
