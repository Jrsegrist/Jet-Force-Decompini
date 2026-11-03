/*
 * Runlink module 31: races. It keeps a ring of up to 256 checkpoint objects
 * (each with a plane and a radius), up to four racers' records (whether a
 * racer is computer-driven, its boosts and its pick-up), and the countdown;
 * the race logic and drawing are still in assembly.
 */
#include "module.h"

/* A checkpoint object's data (obj->unk68) */
typedef struct M031Checkpoint {
    /* 0x00 */ u8 number;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 pad3[0x8 - 0x3];
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 radiusSq;
    /* 0x10 */ f32 nx; /* the plane through the checkpoint */
    /* 0x14 */ f32 ny;
    /* 0x18 */ f32 nz;
    /* 0x1C */ f32 d;
    /* 0x20 */ struct {
        f32 x;
        f32 z;
    } corners[4];
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
    /* 0x13F */ u8 pad13F[0x354 - 0x13F];
    /* 0x354 */ f32 pushX; /* pushes from bumping into other racers */
    /* 0x358 */ f32 pushY;
    /* 0x35C */ f32 pushZ;
    /* 0x360 */ u8 pad360[0x576 - 0x360];
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
static Object *sBss408[(0x498 - 0x408) / 4]; /* the objects 0x274 */
static M031Racer sRacers[4];

/* Whether racer n is computer-driven */
s32 func_m031_00000(s32 n) {
    if (n >= 0 && n < 4 && sRacers[n].computer) {
        return TRUE;
    }
    return FALSE;
}

extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 alpha);
extern void func_8006FCA8(s32 arg0, s32 x, s32 y, s32 arg3, s32 arg4);

/* Draws text that glows: glow 0x80..0xFF fades a white outline in and out */
void func_m031_00044(s32 arg0, s32 x, s32 y, s32 arg3, s32 arg4, s32 r, s32 g, s32 b, s32 a, s32 glow) {
    s32 k;

    if (glow != 0) {
        if (glow >= 0x100) {
            func_8006FC68(r, g, b, 0xFF, a);
        } else if (glow < 0x80) {
            k = glow * 2;
            func_8006FC68(r, g, b, 0xFF, (a * k) >> 8);
        } else {
            if (glow < 0xC0) {
                k = glow - 0x80;
                k <<= 2;
            } else {
                k = 0x100 - glow;
                k <<= 2;
            }
            glow = (a * k) >> 8;
            func_8006FC68(0xFF, 0xFF, 0xFF, 0xFF, glow);
            func_8006FCA8(arg0, x - 1, y, arg3, arg4);
            func_8006FCA8(arg0, x, y + 1, arg3, arg4);
            func_8006FCA8(arg0, x + 1, y, arg3, arg4);
            func_8006FCA8(arg0, x, y - 1, arg3, arg4);
            func_8006FC68((((0xFF - r) * k) >> 8) + r, (((0xFF - g) * k) >> 8) + g, (((0xFF - b) * k) >> 8) + b,
                          0xFF, glow);
        }
        func_8006FCA8(arg0, x, y, arg3, arg4);
    }
}

extern s32 rl_m031_040DC; /* checkpoints found */
extern s32 rl_m031_040E0; /* objects 0x274 found */
extern s32 rl_m031_040E4;
extern s32 rl_m031_040E8;

/* Collects the checkpoints (by their number) and the objects 0x274 from the object list */
void func_m031_00230(void) {
    Object **objs;
    s32 first;
    s32 count;
    s32 i;
    Object *obj;

    i = 256;
    while (i--) {
        sCheckpoints[i] = NULL;
    }
    rl_m031_040DC = 0;
    rl_m031_040E0 = 0;
    rl_m031_040E4 = -1;
    rl_m031_040E8 = 0;
    objs = func_80005E98(&first, &count);
    objs = first + objs;
    i = count - first;
    while (i--) {
        obj = *objs++;
        if (obj->behaviorId == 0x79) {
            u8 *data = obj->unk68;
            sCheckpoints[*data] = obj;
            rl_m031_040DC++;
        } else if (obj->objectID == 0x274) {
            sBss408[rl_m031_040E0] = obj;
            rl_m031_040E0++;
        }
    }
}

/* What a checkpoint's init gets */
typedef struct M031CheckpointSetup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 number;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 yaw;
    /* 0x0D */ u8 radius;
    /* 0x0E */ s8 corners[4]; /* along the checkpoint, in 8ths */
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
} M031CheckpointSetup;

/* The module's .rodata (still assembly) */
extern f32 rl_m031_04458;
extern f32 rl_m031_0445C;
extern f32 rl_m031_04460; /* farther than any checkpoint (squared) */

void func_m031_00364(Object *obj, M031CheckpointSetup *setup) {
    f32 s;
    f32 co;
    M031Checkpoint *c = obj->unk68;
    f32 k;

    obj->trans.scale = setup->radius * rl_m031_04458;
    obj->trans.rotation.y_rotation = setup->yaw << 10;
    obj->trans.rotation.x_rotation = 0;
    obj->trans.rotation.z_rotation = 0;
    c->number = setup->number;
    c->unk1 = setup->unk12;
    c->unk2 = setup->unk13;
    c->unk8 = setup->unkB * rl_m031_0445C;
    c->radiusSq = setup->radius * 4;
    c->radiusSq = c->radiusSq * c->radiusSq;
    s = func_80049C34(obj->trans.rotation.y_rotation);
    co = func_80049C30(obj->trans.rotation.y_rotation);
    c->nx = func_80049C34(obj->trans.rotation.y_rotation);
    c->ny = 0.0f;
    c->nz = func_80049C30(obj->trans.rotation.y_rotation);
    c->d = -(c->nx * obj->trans.x_position + c->nz * obj->trans.z_position);
    k = setup->corners[0] * 8;
    c->corners[0].x = obj->trans.x_position + k * co;
    c->corners[0].z = obj->trans.z_position - k * s;
    k = setup->corners[1] * 8;
    c->corners[1].x = obj->trans.x_position + k * co;
    c->corners[1].z = obj->trans.z_position - k * s;
    k = setup->corners[2] * 8;
    c->corners[2].x = obj->trans.x_position + k * co;
    c->corners[2].z = obj->trans.z_position - k * s;
    k = setup->corners[3] * 8;
    c->corners[3].x = obj->trans.x_position + k * co;
    c->corners[3].z = obj->trans.z_position - k * s;
}

#ifdef NON_MATCHING
/*
 * The nearest checkpoint to (x, y, z) (flat: by x and z only; otherwise the height difference is
 * added, unsquared). NON_MATCHING: the sum and the distance swap f16 and f18.
 */
Object *func_m031_005AC(f32 x, f32 y, f32 z, s32 flat) {
    Object *best;
    f32 bestDist;
    Object **p;
    s32 i;
    Object *obj;
    f32 dx;
    f32 dz;
    f32 sq;
    f32 dist;

    bestDist = rl_m031_04460;
    p = sCheckpoints;
    best = NULL;
    i = 256;
    while (i--) {
        obj = *p++;
        if (obj != NULL) {
            dx = obj->trans.x_position - x;
            dz = obj->trans.z_position - z;
            sq = dx * dx + dz * dz;
            dist = sq;
            if (!flat) {
                f32 dy = obj->trans.y_position - y;
                dist = sq + dy;
            }
            if (best == NULL || dist < bestDist) {
                best = obj;
                bestDist = dist;
            }
        }
    }
    return best;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_005AC);
#endif

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

/* A level object entry built on the stack (the racer's pick-up shot) */
typedef struct M031Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ Object *owner;
} M031Entry;

extern s32 func_8001B078(f32 x, f32 z, s32 arg2, f32 ***heights);
extern Object *func_80005F14(void *entry, s32 arg1);

#ifdef NON_MATCHING
/* Racer n fires its pick-up (an oil slick or a bomb) */
/* (same code, but the frame is 8 bytes bigger: every local sits 8 higher) */
s32 func_m031_01C68(Object *obj, s32 n) {
    Object *spawned;
    M031Entry entry;
    s32 count;
    f32 **heights;
    s32 i;
    M031Racer *racer;

    if (n >= 0 && n < 4) {
        racer = &sRacers[n];
        if (racer->pickup != 0) {
            entry.x = obj->trans.x_position;
            entry.y = obj->trans.y_position;
            entry.z = obj->trans.z_position;
            count = func_8001B078(obj->trans.x_position, obj->trans.z_position, 0, &heights);
            if (count != 0) {
                i = count;
                while (i--) {
                    if (*heights[i] < obj->trans.y_position) {
                        entry.y = *heights[i];
                        break;
                    }
                }
            }
            entry.owner = obj;
            if (racer->pickup == 2) {
                entry.objectID = 0x279;
                entry.unkA = 0x1E0;
                if (D_800A503C != 0) {
                    func_80001248(0x215, 0);
                } else {
                    func_80003410(0x215, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4,
                                  NULL);
                }
            } else {
                if (D_800A503C == 0) {
                    entry.objectID = 0x278;
                    func_80003410(0x216, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4,
                                  NULL);
                } else {
                    entry.objectID = 0x1FB;
                    func_80001248(0x216, 0);
                }
                entry.unkA = 0x258;
            }
            spawned = func_80005F14(&entry, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
            racer->pickup = 0;
            return TRUE;
        }
    }
    return FALSE;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m031", func_m031_01C68);
#endif

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

/* A sphere of an object's hit shape (size 0x34) */
typedef struct M031Sphere {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ u8 pad18[0x24 - 0x18];
    /* 0x24 */ f32 radius;
    /* 0x28 */ u8 pad28[0x34 - 0x28];
} M031Sphere;

/* What obj->interactObj points at here: the hit shape */
typedef struct M031HitShape {
    /* 0x00 */ u8 pad0[0x32];
    /* 0x32 */ s16 count;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ M031Sphere *spheres;
} M031HitShape;

extern f32 func_80097140(f32 x); /* square root */

/* Pushes two racers apart where their hit shapes overlap, with a rumble when it is a hard bump */
void func_m031_03AF8(Object *a, Object *b) {
    M031HitShape *shapeA = (M031HitShape *) a->interactObj;
    M031Player *pa = a->unk68;
    s32 i = shapeA->count;
    M031HitShape *shapeB = (M031HitShape *) b->interactObj;
    M031Player *pb = b->unk68;
    M031Sphere *sa = shapeA->spheres;
    s32 bumped = FALSE;
    M031Sphere *sb;
    s32 j;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 r;
    f32 d;
    f32 depth;
    f32 f;

    while (i--) {
        j = shapeB->count;
        sb = shapeB->spheres;
        while (j--) {
            dx = (sa->pos.x + pa->pushX) - (sb->pos.x + pb->pushX);
            dy = sa->pos.y - sb->pos.y;
            dz = (sa->pos.z + pa->pushZ) - (sb->pos.z + pb->pushZ);
            r = sa->radius + sb->radius;
            d = func_80097140(dx * dx + dz * dz);
            sb++;
            if (-r < dy && dy < r && d < r) {
                if (d != 0.0f) {
                    depth = r - d;
                    f = depth / (d + d);
                    dx *= f;
                    dz *= f;
                } else {
                    dx = 0.0f;
                    dz = d * -0.5f;
                    depth = r - d;
                }
                pa->pushX += dx;
                pa->pushZ += dz;
                pb->pushX -= dx;
                pb->pushZ -= dz;
                if (depth > 5.0f) {
                    bumped = TRUE;
                }
            }
        }
        sa++;
    }
    if (bumped) {
        if (!func_m031_00000(pa->player)) {
            rumble_raise(pa->player, 50, 0.2f);
        }
        if (!func_m031_00000(pb->player)) {
            rumble_raise(pa->player, 50, 0.2f);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m031", data);

INCLUDE_ASM("asm/nonmatchings/modules/m031", rodata);
