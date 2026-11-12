/*
 * Runlink module 31: races. It keeps a ring of up to 256 checkpoint objects
 * (each with a plane and a radius), up to four racers' records (whether a
 * racer is computer-driven, its boosts and its pick-up), and the countdown;
 * the race logic and drawing are still in assembly.
 */
#include "module.h"

/* the .rodata strings the data refers to (first in .rodata) */
INCLUDE_ASM("asm/nonmatchings/modules/m031/ro", rodata);

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

/* A best time: the time and three 6-bit letters */
typedef struct M031Record {
    u32 time : 14;
    u32 c0 : 6;
    u32 c1 : 6;
    u32 c2 : 6;
} M031Record;

/* A racer's record (size 0x28) */
typedef struct M031Racer {
    /* 0x00 */ Object *next;     /* the checkpoint ahead */
    /* 0x04 */ Object *previous; /* the checkpoint behind */
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ f32 distance;     /* from the plane of the checkpoint behind */
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 computer;
    /* 0x16 */ u8 unk16;  /* the computer's driving: 0, 1 */
    /* 0x17 */ u8 unk17;  /* which corner of the checkpoint behind it to head for */
    /* 0x18 */ u8 position;
    /* 0x19 */ u8 boosts;
    /* 0x1A */ u8 pickup; /* 1: oil, 2: bomb */
    /* 0x1B */ s8 unk1B;  /* time to the next boost */
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ s8 place;          /* in the race's best times */
    /* 0x20 */ M031Record *record; /* its best time, or NULL */
    /* 0x24 */ s32 unk24;
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
    /* 0x13 */ u8 pad13;
    /* 0x14 */ u8 unk14; /* its place at the end of a race, 1 to 4 (01150) */
    /* 0x15 */ u8 pad15[0x1B - 0x15];
    /* 0x1B */ u8 pad1B_5 : 3;
    /* 0x1B */ u8 index : 5;
    /* 0x1C */ u8 pad1C[0x90 - 0x1C];
    /* 0x90 */ s32 time; /* the race time, in frames */
    /* 0x94 */ u8 pad94[0xA0 - 0x94];
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
static Object *sBss408[32]; /* the objects 0x274 */
static s32 sBss488[4];
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

void func_m031_00364(Object *obj, M031CheckpointSetup *setup) {
    f32 s;
    f32 co;
    M031Checkpoint *c = obj->unk68;
    f32 k;

    obj->trans.scale = setup->radius * 0.04f;
    obj->trans.rotation.y_rotation = setup->yaw << 10;
    obj->trans.rotation.x_rotation = 0;
    obj->trans.rotation.z_rotation = 0;
    c->number = setup->number;
    c->unk1 = setup->unk12;
    c->unk2 = setup->unk13;
    c->unk8 = setup->unkB * 0.01f;
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

    bestDist = 1.0e6f;
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
Object *func_m031_005AC(f32 x, f32 y, f32 z, s32 flat);
INCLUDE_ASM("asm/nonmatchings/modules/m031/ro", func_m031_005AC);
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
f32 func_m031_007B4(Object *checkpoint, f32 x, f32 y, f32 z, UNUSED s32 flat) {
    M031Checkpoint *c;
    f32 dist = 32768.0f;

    if (checkpoint != NULL) {
        c = checkpoint->unk68;
        dist = c->nx * x + c->ny * y + c->nz * z + c->d;
    }
    return dist;
}

extern Object **func_8000E464(s32 *count); /* the players */
extern s32 rl_m031_040EC; /* (.data) */
extern s32 rl_m031_040F0;
extern s32 rl_m031_040F4;
extern s32 rl_m031_040F8;

/* Starts the race: finds each racer's checkpoints and resets its record */
void func_m031_0080C(void) {
    s32 i;
    M031Racer *r;
    s32 count;
    Object **players;
    Object *checkpoint;
    Object *player;
    u8 *data;
    s32 side;

    func_m031_00230();
    players = func_8000E464(&count);
    r = sRacers;
    for (i = 0; i < count; i++, r++) {
        player = *players++;
        side = 0;
        if (player != NULL) {
            checkpoint = func_m031_005AC(player->trans.x_position, player->trans.y_position, player->trans.z_position, 1);
            side = func_m031_006F0(checkpoint, player->trans.x_position, player->trans.y_position, player->trans.z_position, 1);
            if (checkpoint != NULL) {
                data = checkpoint->unk68;
            }
        }
        switch (side) {
            default:
                r->next = NULL;
                r->previous = NULL;
                break;
            case -1:
                r->next = func_m031_0066C(*data);
                r->previous = checkpoint;
                break;
            case 1:
                r->next = checkpoint;
                r->previous = func_m031_006AC(*data);
                break;
        }
        r->unk8 = 0;
        r->unkC = 0;
        r->distance = func_m031_007B4(r->previous, player->trans.x_position, player->trans.y_position,
                                      player->trans.z_position, 1);
        r->unk14 = 0;
        r->computer = i >= count - D_800A4FD4;
        r->unk16 = i - 1;
        r->unk17 = i - 1;
        r->position = i;
        r->boosts = D_800A5038;
        r->pickup = 0;
        r->unk1B = 0;
        r->unk1C = 0;
        r->unk1E = 0;
        r->unk24 = 0;
    }
    sCountdown = 360;
    if (*(u8 *) &D_800A4FB4 != 0 && D_800A503C == 0) {
        sCountdown += 300;
    }
    rl_m031_040EC = 0;
    rl_m031_040F0 = -1;
    sBss488[0] = 0;
    sBss488[1] = 0;
    sBss488[2] = 0;
    sBss488[3] = 0;
    rl_m031_040F4 = 0;
    rl_m031_040F8 = 0;
}

extern void rl_m031_021F0(s32 updateRate);
static void func_m031_01150(s32 count, s32 updateRate);
extern void rl_m031_0242C(s32 arg0, s32 updateRate);

/*
 * The race's per-frame update: each racer passes its checkpoints (counting laps), then the racers' places, the
 * countdown's beeps and the computer racers
 */
void func_m031_00A3C(s32 updateRate) {
    s32 i;
    M031Racer *r;
    Object *player;
    s32 side;
    s32 count;
    s32 j;
    f32 d; /* the distance before, then how far it moved */
    s32 pos;
    Object **players;
    UNUSED s32 pad;
    M031Racer *o;
    s32 t;
    M031Checkpoint *data;

    players = func_8000E464(&count);
    i = 0;
    if (count <= 0) {
        return;
    }
    for (r = sRacers; i < count; i++, r++) {
        player = *players++;
        if (r->next != NULL && r->previous != NULL) {
            side = func_m031_006F0(r->next, player->trans.x_position, player->trans.y_position,
                                   player->trans.z_position, 1);
            if (func_m031_006F0(r->previous, player->trans.x_position, player->trans.y_position,
                                player->trans.z_position, 1) == 1) {
                data = r->previous->unk68;
                r->next = r->previous;
                r->previous = func_m031_006AC(data->number);
                r->unkC++;
                if (r->unkC >= rl_m031_040DC) {
                    if (r->unk14 == 0 && r->unk1E == 0) {
                        r->unk8++;
                    }
                    r->unkC -= rl_m031_040DC;
                    r->unk14 = 0;
                }
            } else if (side == -1) {
                data = r->next->unk68;
                r->previous = r->next;
                r->next = func_m031_0066C(data->number);
                r->unkC--;
                if (r->unkC < 0) {
                    r->unk14 = 1;
                    r->unkC += rl_m031_040DC;
                }
            }
            d = r->distance;
            r->distance = func_m031_007B4(r->previous, player->trans.x_position, player->trans.y_position,
                                          player->trans.z_position, 1);
            d = r->distance - d;
            if (sCountdown <= 0 && -0.075f < d && d < 0.075f) {
                r->unk24 += updateRate;
            } else {
                r->unk24 = 0;
            }
        }
        if (r->unk1B > 0) {
            r->unk1B -= updateRate;
        }
    }
    for (r = sRacers, i = 0; i < count; i++, r++) {
        pos = 0;
        j = 0;
        o = sRacers;
        for (; j < count; j++, o++) {
            if (j != i) {
                if (r->unk8 < o->unk8 || r->unk1E < o->unk1E) {
                    pos++;
                } else if (r->unk8 == o->unk8) {
                    if (r->unk14 != 0 && o->unk14 == 0) {
                        pos++;
                    } else if (r->unk14 == o->unk14) {
                        if (r->unkC < o->unkC) {
                            pos++;
                        } else if (r->unkC == o->unkC && o->distance < r->distance) {
                            pos++;
                        }
                    }
                }
            }
        }
        if (pos == r->position) {
            r->unk1C += updateRate;
        } else {
            r->unk1C = 0;
        }
        r->position = pos;
        if (r->computer) {
            data = r->next->unk68;
            if (!r->unk16 && data->unk2) {
                if (r->position == 0 && r->unk1C >= 360) {
                    r->unk17 = 1;
                } else if (r->position != 0 || r->unk1C < 360) {
                    r->unk17 = 0;
                }
            }
        }
    }
    if (sCountdown > 0) {
        t = sCountdown;
        sCountdown -= updateRate;
        if (t >= 300 && sCountdown < 300) {
            func_80001248(0x164, 0);
        } else if (t >= 240 && sCountdown < 240) {
            func_80001248(0x162, 0);
        } else if (t >= 180 && sCountdown < 180) {
            func_80001248(0x160, 0);
        } else if (t >= 120 && sCountdown < 120) {
            func_80001248(0xDA, 0);
        }
    }
    if (sCountdown < 120) {
        rl_m031_021F0(updateRate);
        func_m031_01150(count, updateRate);
    }
    if (*(u8 *) &D_800A4FB4 != 0 && sRacers[0].unk1E != 0) {
        rl_m031_0242C(0, updateRate);
    }
}

extern s8 D_800A5028;  /* the race's time limit, in minutes (0: none) */
extern s16 D_800A4FB8;
extern u8 D_800A4FBC;
extern void func_80001170(u8 sound);
extern void music_volume_fade_scaled(f32 volume, s32 arg1);
extern void music_volume_fade(f32 volume, s32 arg1);
extern void func_80000510(s32 song);
extern s32 func_80043510(s32 player); /* the buttons pressed */
extern void mark_write_eeprom_settings(void);
extern void func_8004665C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_800468C0(s32 arg0);
extern void func_80047608(s32 arg0, s32 arg1, s32 arg2);
extern void func_80048518(s32 arg0);

/*
 * The race's end: times the racers (to the limit) until every one has finished (laps or time), then the end
 * fade, and after a button the next screen
 */
static void func_m031_01150(s32 count, s32 updateRate) {
    s32 i;
    s32 limit;
    s32 done;
    M031Racer *r;

    switch (*(u8 *) &D_800A4FEC) {
        case 0:
            limit = D_800A5028 * 3600;
            gMenuTimer += updateRate;
            for (i = 0; i < count; i++) {
                if (sRacers[i].unk1E == 0) {
                    D_800FED50[i].time += updateRate;
                    if (D_800A5024 && (u32) D_800FED50[i].time > (u32) limit) {
                        D_800FED50[i].time = limit;
                    }
                }
            }
            i = 0;
            sBss0 = -1;
            r = sRacers;
            if (!D_800A5024) {
                for (; i < count; i++, r++) {
                    if (r->unk8 >= D_800A502C && r->unk1E == 0) {
                        r->unk1E = 4 - r->position;
                        if (!r->computer) {
                            r->computer = 1;
                            r->unk17 = func_80048E2C(0, 2);
                        }
                    }
                }
                r = sRacers;
            } else if (gMenuTimer >= limit) {
                gMenuTimer = limit;
                for (; i < count; i++, r++) {
                    r->unk1E = 4 - r->position;
                    if (!r->computer) {
                        r->computer = 1;
                        r->unk17 = func_80048E2C(0, 2);
                    }
                }
                r = sRacers;
            }
            done = TRUE;
            i = count;
            while (i--) {
                if (r->unk1E == 0 && !r->computer) {
                    done = FALSE;
                }
                r++;
            }
            if (done) {
                i = count;
                while (i--) {
                    D_800FED50[i].unk14 = 5 - sRacers[i].unk1E;
                }
                *(u8 *) &D_800A4FEC = 1;
                if (D_800A503C) {
                    func_80001170(0x4E);
                    music_volume_fade_scaled(0.34f, 0x20);
                } else if (!*(u8 *) &D_800A4FB4 || sRacers[0].unk1E == 4) {
                    func_80000510(0x57);
                } else {
                    func_80000510(0x58);
                }
            }
            break;
        case 1:
            if ((((u16 *) D_800FF1B8)[0] += updateRate) >= 256) {
                ((u16 *) D_800FF1B8)[0] = 255;
                *(u8 *) &D_800A4FEC = 2;
                if (D_800A503C) {
                    music_volume_fade_scaled(2.0f, 0x7F);
                }
            }
            break;
        case 2:
            for (i = 0; i < count; i++) {
                if (func_80043510(i) & 0x9000) {
                    if (rl_m031_040F4) {
                        mark_write_eeprom_settings();
                    }
                    if (*(u8 *) &D_800A4FB4) {
                        if (D_800A503C) {
                            func_8004665C(D_800A4FB8, D_800A4FC0, D_800A4FBC, 0x10, 1, 0);
                        } else if (sRacers[0].unk1E == 4 && !game_flag_get(0x25)) {
                            func_8004665C(0x18A, 3, 0, 0x10, 1, 0);
                        } else {
                            func_8004665C(0x11D, 3, 0, 0x10, 1, 0);
                        }
                        *(u8 *) &D_800A4FEC = 3;
                    } else {
                        music_volume_fade(0.5f, 0);
                        func_800468C0(1);
                        func_80047608(0x15, 0, 0);
                        *(u8 *) &D_800A4FEC = 3;
                    }
                    break;
                }
            }
            break;
    }
    if (*(u8 *) &D_800A4FEC) {
        func_80048518(10);
    }
}

extern u8 D_800A5040;
extern f32 func_80097140(f32 x); /* square root */
s32 func_m031_01EB0(void);

/*
 * A computer-driven racer's controller input: steers (*stickX, *stickY) at a corner of the checkpoint behind it
 * and sometimes presses boost (0x8000) or drops its pick-up (0x2000). FALSE if racer n isn't one.
 */
s32 func_m031_017A0(s32 n, s32 *stickX, s32 *stickY, s32 *arg3, s32 *buttons) {
    f32 dx;
    f32 dz;
    f32 speed;
    Object *prev;
    Object *player;
    f32 f;
    M031Checkpoint *cp;
    M031Racer *r;

    if (n >= 0 && n < 4) {
        r = &sRacers[n];
        if (r->computer) {
            if (!func_m031_01EB0()) {
                player = func_8000E4D0(n);
                if (r->unk24 >= 21 && D_800A5040 == 2 && r->unkC >= 15 && r->unkC < 17) {
                    r->previous = func_m031_006AC(0x27);
                    r->next = func_m031_0066C(0x28);
                    r->unkC = 0x27;
                    r->unk14 = 1;
                    r->unk24 = 0;
                }
                prev = r->previous;
                cp = prev->unk68;
                switch (r->unk17) {
                    case 0:
                        dx = cp->corners[0].x;
                        dz = cp->corners[0].z;
                        speed = 65.0f;
                        break;
                    case 2:
                        dx = cp->corners[1].x;
                        dz = cp->corners[1].z;
                        speed = 60.0f;
                        break;
                    case 3:
                        dx = cp->corners[3].x;
                        dz = cp->corners[3].z;
                        speed = 62.5f;
                        break;
                    default:
                        speed = 62.5f;
                        dx = cp->corners[2].x;
                        dz = cp->corners[2].z;
                        break;
                }
                dx = player->trans.x_position - dx;
                dz = dz - player->trans.z_position;
                f = func_80097140(dx * dx + dz * dz);
                if (f > 0.0f) {
                    f = (cp->unk8 * speed) / f;
                }
                *stickX = dx * f;
                *stickY = dz * f;
                *arg3 = 0;
                *buttons = 0;
                switch (r->unk16) {
                    case 0:
                        if (r->position != 0 && r->unk1B <= 0 && r->boosts > 0) {
                            r->unk1B = 120;
                            *buttons = 0x8000;
                        }
                        if (r->position != 3 && r->pickup != 0 && func_80048E2C(1, 1000) >= 0x3D0) {
                            *buttons |= 0x2000;
                            return TRUE;
                        }
                        break;
                    case 1:
                        if (r->position != 0 && r->unk1B <= 0 && r->boosts > 0 && func_80048E2C(1, 1000) >= 0x3DF) {
                            r->unk1B = 120;
                            *buttons = 0x8000;
                        }
                        if (r->position != 3 && r->pickup != 0 && func_80048E2C(1, 1000) >= 0x3E4) {
                            *buttons |= 0x2000;
                            return TRUE;
                        }
                        break;
                }
            } else {
                *stickX = 0;
                *stickY = 0;
                *arg3 = 0;
                *buttons = 0;
            }
            return TRUE;
        }
    }
    return FALSE;
}

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

extern s16 *rl_m031_03DF8[]; /* (.data) per race: the pick-ups that can appear */

/*
 * Every few seconds puts a random pick-up of the race's kinds at a random one of the objects 0x274 (never
 * the same one twice running) (called from 00A3C)
 */
void func_m031_021F0(s32 updateRate) {
    s32 r;
    M031Entry entry;
    s16 *ids;
    Object *at;
    s32 r2;
    Object *spawned;

    if (rl_m031_040E0 > 0 && D_800A5034 != 0) {
        rl_m031_040E8 -= updateRate;
        if (rl_m031_040E8 <= 0) {
            if (rl_m031_040E0 >= 2) {
                do {
                    r = func_80048E2C(0, rl_m031_040E0 - 1);
                } while (r == rl_m031_040E4);
            } else {
                r = 0;
            }
            r2 = func_80048E2C(0, 7);
            ids = rl_m031_03DF8[D_800A5034];
            at = sBss408[r];
            entry.objectID = ids[r2];
            entry.x = at->trans.x_position;
            entry.y = at->trans.y_position;
            entry.z = at->trans.z_position;
            entry.unkA = 10;
            spawned = func_80005F14(&entry, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
            rl_m031_040E4 = r;
            rl_m031_040E8 = func_80048E2C(0, 180) + 300;
        }
    }
}

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

/* The game's save data, as used here */
typedef struct M031Save {
    /* 0x00 */ M031Record records[5][5]; /* the best times of each race */
    /* 0x64 */ u8 pad64[0x6A - 0x64];
    /* 0x6A */ u8 unk6A; /* the race-reward flags given */
} M031Save;

extern M031Save D_800FF318;
extern void func_80047EFC(s32 flag, s32 set);
static M031Record *func_m031_038E0(u32 time, s8 *place);

/*
 * The finish of racer n: once, its time goes into the best times (with the reward flags for a
 * first place), then the result fades in by `step` and the four bars of sBss488 fill one by one
 */
void func_m031_0242C(s32 n, s32 step) {
    if (n >= 0 && n < D_800A4FD0) {
        if (rl_m031_040EC == 0) {
            sRacers[n].record = func_m031_038E0(D_800FED50[n].time * 100 / 60, &sRacers[n].place);
            if (sRacers[n].record != NULL) {
                if (D_800A5040 == 0) {
                    if (sRacers[n].unk1E == 4 && !(D_800FF318.unk6A & 1)) {
                        func_80047EFC(0x42, 1);
                        rl_m031_040F8 = 1;
                    }
                } else if (D_800A5040 == 1) {
                    if (sRacers[n].unk1E == 4 && !(D_800FF318.unk6A & 2)) {
                        func_80047EFC(0x43, 1);
                        rl_m031_040F8 = 1;
                    }
                } else if (D_800A5040 == 2) {
                    if (sRacers[n].unk1E == 4 && !(D_800FF318.unk6A & 4)) {
                        func_80047EFC(0x44, 1);
                        rl_m031_040F8 = 1;
                    }
                }
                if (!(D_800FF318.unk6A & 0x10) && D_800FF318.records[1][0].time < 3000 &&
                    D_800FF318.records[2][0].time < 4000) {
                    func_80047EFC(0x46, 1);
                    rl_m031_040F8 = 1;
                }
                rl_m031_040F4 = 1;
            }
        }
        if (rl_m031_040EC < 0x100) {
            rl_m031_040EC += step * 4;
            if (rl_m031_040EC >= 0x100) {
                rl_m031_040EC = 0x100;
                rl_m031_040F0 = 0;
            }
        } else if (rl_m031_040F0 >= 0 && rl_m031_040F0 < 4) {
            sBss488[rl_m031_040F0] += step * 8;
            if (sBss488[rl_m031_040F0] > 0x100) {
                sBss488[rl_m031_040F0++] = 0x100;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m031/ro", func_m031_026B8);

/* A coloured mark of the speed gauge (12 bytes; the colour's low byte is its alpha) */
typedef struct M031GaugeMark {
    /* 0x0 */ u8 pad0[0x8];
    /* 0x8 */ u32 colour;
} M031GaugeMark;

/* The racer state that 029FC reads (obj->unk68) */
typedef struct M031Speed {
    /* 0x0 */ u8 pad0[0x4];
    /* 0x4 */ f32 speed;
} M031Speed;

/* .data/.rodata (still assembly): the gauge's two rows of marks and the second row's alpha scale */
extern s32 rl_m031_04108[20];
extern M031GaugeMark rl_m031_04158[20];
extern M031GaugeMark rl_m031_04248[20];
extern void screen_line_add_raw(s32 x, s32 y, s32 value, s32 digits, s32 flag); /* draws a number */
extern void menu_fill_boxes(s32 arg0, s32 count, M031GaugeMark *marks, s32 arg3);

/* Draws the racer's speed gauge: the marks up to its speed lit, and the speed as a number */
#ifdef NON_MATCHING
/* Same code and length; only the loop counter and the scale pointer swap registers (a0/a2) */
void func_m031_029FC(s32 arg0, Object *obj) {
    M031Speed *s;
    s32 *scale;
    M031GaugeMark *lit;
    M031GaugeMark *dim;
    s32 i;
    s32 n;
    u32 base;

    if (obj != NULL && (s = obj->unk68) != NULL) {
        n = s->speed * 0.86f;
        if (n < 0) {
            n = -n;
        }
        scale = rl_m031_04108;
        lit = rl_m031_04248;
        dim = rl_m031_04158;
        for (i = 0; i < 20; i++) {
            base = lit->colour & ~0xFF;
            if (i < n) {
                lit->colour = base | 0xAA;
                dim->colour = ((*scale * 0xAA) >> 8) | base;
            } else {
                lit->colour = base | 0x55;
                dim->colour = ((*scale * 0x55) >> 8) | base;
            }
            scale++;
            lit++;
            dim++;
        }
        n = s->speed * 25.0f;
        if (n < 0) {
            n = -n;
        }
        screen_line_add_raw(0xA8, 0xCC, n, 3, n >= 501);
    }
    menu_fill_boxes(arg0, 20, rl_m031_04248, 0);
    menu_fill_boxes(arg0, 20, rl_m031_04158, 0);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m031/ro", func_m031_029FC);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m031/ro", func_m031_02B88);

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

/*
 * Puts a best time into the current race's table (shifting the slower ones down, with the default
 * letters) and gives its place; returns the table, or NULL when the time is not among the five best
 */
static M031Record *func_m031_038E0(u32 time, s8 *place) {
    M031Record *table;
    s32 i;
    s32 j;

    switch (D_800A5040) {
        case 0:
            table = D_800FF318.records[0];
            break;
        case 1:
            table = D_800FF318.records[1];
            break;
        case 2:
            table = D_800FF318.records[2];
            break;
        case 3:
            table = D_800FF318.records[3];
            break;
        default:
            table = D_800FF318.records[4];
            break;
    }
    i = 5;
    while (i > 0 && time < table[i - 1].time) {
        i--;
    }
    if (i != 5) {
        j = 4;
        while (j > i) {
            table[j].time = table[j - 1].time;
            table[j].c0 = table[j - 1].c0;
            table[j].c1 = table[j - 1].c1;
            table[j].c2 = table[j - 1].c2;
            j--;
        }
        table[i].time = time;
        if (D_800A4FC0 & 3) {
            table[i].c0 = 9;
            table[i].c1 = 20;
            table[i].c2 = 13;
        } else {
            table[i].c0 = 21;
            table[i].c1 = 4;
            table[i].c2 = 11;
        }
    } else {
        table = NULL;
    }
    *place = i;
    return table;
}

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

/* (.rodata: C, and the assembly functions' own from ro/) */
