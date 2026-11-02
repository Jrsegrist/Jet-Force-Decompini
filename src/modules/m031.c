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
extern s8 D_800A4FD0;
extern s8 D_800A5024;
extern s8 D_800A502C;
extern s8 D_800A5034;
extern s8 D_800A5038;
extern s8 D_800A5044;
extern s8 D_800A5048;
extern s8 D_800A504C;
extern s8 D_800A5050;

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

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_01ED8);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_02064);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_021F0);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_02388);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_0242C);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_026B8);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_029FC);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_02B88);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_0361C);

INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_036E8);

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
