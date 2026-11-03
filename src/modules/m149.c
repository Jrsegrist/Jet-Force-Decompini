/*
 * Runlink module 149: a boss enemy for the enemy AI (module 3) made of hit
 * zones: shooting zones away moves it through its phases (the zone
 * flags in the enemy record's parts word), it picks its attacks through
 * module 3, flies between random points of its arena, and fires homing
 * bolts and lobbed bombs (each with its own init and control function).
 */
#include "module.h"

/* The flag word of a hit zone */
typedef struct M149ZoneFlags {
    u32 unk31 : 1;
    u32 shotAway : 1;
    u32 active : 1; /* can be hit */
    u32 unk0 : 29;
} M149ZoneFlags;

/* A hit zone of the boss's model (size 0x34) */
typedef struct M149HitZone {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ s16 damage;
    /* 0x2E */ u8 pad2E;
    /* 0x2F */ u8 unk2F;
    /* 0x30 */ M149ZoneFlags flags;
} M149HitZone;

/* What obj->interactObj points at, for this boss */
typedef struct M149Interaction {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0x24 - 0x8];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ u8 pad28[0x32 - 0x28];
    /* 0x32 */ s16 zoneCount;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ M149HitZone *zones;
} M149Interaction;

#define INTERACT(obj) ((M149Interaction *) (obj)->interactObj)

/* The boss's own state (one, shared by its objects) */
typedef struct M149State {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 phase; /* 0x80: busy */
    /* 0x02 */ u8 count;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ f32 destX;
    /* 0x08 */ f32 destY;
    /* 0x0C */ f32 destZ;
    /* 0x10 */ f32 startX;
    /* 0x14 */ f32 startY;
    /* 0x18 */ f32 startZ;
    /* 0x1C */ s32 t; /* along the way, 0..0x8000 */
    /* 0x20 */ u8 pad20[2];
    /* 0x22 */ s16 speed;
    /* 0x24 */ u8 pad24[0x2C - 0x24];
} M149State;

/* What the enemy record's owner keeps in its unk68 */
typedef struct M149OwnerData {
    /* 0x00 */ u8 pad0[0x2A];
    /* 0x2A */ s16 hitPoints;
    /* 0x2C */ u8 pad2C[0x30 - 0x2C];
    /* 0x30 */ u8 unk30;
} M149OwnerData;

/* The enemy record (obj->unk68) */
typedef struct M149Enemy {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 unk2_7 : 1;
    /* 0x02 */ u8 unk2_6 : 1;
    /* 0x02 */ u8 unk2_5 : 1;
    /* 0x02 */ u8 unk2_4 : 1; /* (0x10) */
    /* 0x02 */ u8 unk2_0 : 4;
    /* 0x03 */ u8 pad3[0xA - 0x3];
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 padB;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 pad12[0x1A - 0x12];
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 pad1B[0x24 - 0x1B];
    /* 0x24 */ Object *owner;
    /* 0x28 */ u8 pad28[0x4A - 0x28];
    /* 0x4A */ u8 unk4A;
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ u8 pad4C[0x5A - 0x4C];
    /* 0x5A */ u16 parts; /* the zones shot away */
    /* 0x5C */ u8 pad5C[0x64 - 0x5C];
    /* 0x64 */ Object *target;
    /* 0x68 */ u8 pad68[0xA0 - 0x68];
    /* 0xA0 */ u16 unkA0;
    /* 0xA2 */ u8 padA2[2];
    /* 0xA4 */ u16 unkA4;
    /* 0xA6 */ u8 padA6[2];
    /* 0xA8 */ M149State *state;
} M149Enemy;

/* A short object entry for spawn_object */
typedef struct M149Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 padA;
} M149Entry;

/* The bomb's explosion entry */
typedef struct M149BlastEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ s16 pad2;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 padE;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ s32 unk14;
} M149BlastEntry;

/* A homing bolt's data (obj->unk68) */
typedef struct M149Bolt {
    /* 0x00 */ Object *owner;
    /* 0x04 */ u8 pad4[0x10 - 0x4];
    /* 0x10 */ s16 age;
    /* 0x12 */ s16 targetX;
    /* 0x14 */ s16 targetY;
    /* 0x16 */ s16 targetZ;
    /* 0x18 */ Vec3f prev;
    /* 0x24 */ f32 turnX;
    /* 0x28 */ f32 turnZ;
    /* 0x2C */ f32 dist;
    /* 0x30 */ f32 height;
} M149Bolt;

/* A bolt's level entry */
typedef struct M149BoltSetup {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Object *spawner;
} M149BoltSetup;

/* A bomb's data (obj->unk68) */
typedef struct M149Bomb {
    /* 0x00 */ u16 landed;
    /* 0x02 */ u16 timer;
} M149Bomb;

extern Object *func_m003_053C4(void); /* the closest player */
extern void func_m003_04EC4();
extern s32 func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_179B0(s32, s32);
extern s32 func_80049C84(f32 dx, f32 dz); /* the angle of a vector */
extern s32 func_80049D80(s32 from, s32 to); /* the difference between two angles */
extern void chain_object_unlink_from_players(Object *obj);
extern void func_8000C9B4(s32, s32, s32, s32);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern f32 func_80097140(f32 x); /* square root */
extern f32 point_distance(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
extern s16 func_80015510(f32 x, f32 y, f32 z); /* the level segment at a position */
extern void func_80029BA4(f32 x, f32 y, f32 z, f32 arg3, f32 arg4, f32 arg5, s32 arg6, Object *owner);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz);

static s16 sSpeed = 0x140; /* the boss's speed along its way */

static M149State sState;
static s32 sBoltSide;     /* which side the bolts fall on */
static Object *sArena;    /* the object with ID 0x61: the arena's centre */
static s32 sBoltCount;
static s32 sSwoopSide;

void func_m149_00000(Object *obj) {
    M149Enemy *e;
    Object *ownerObj;
    M149OwnerData *owner;
    Object *player;
    s32 cur;
    M149State *state;
    M149HitZone *zone;
    s32 first;
    s32 count;
    Object **objs;
    s32 n;
    s32 action;
    s32 phase;

    e = obj->unk68;
    ownerObj = e->owner;
    owner = ownerObj->unk68;
    player = func_m003_053C4();
    state = e->state;
    if (state == NULL) {
        objs = func_80005E98(&first, &count);
        sSwoopSide = 0;
        sBoltCount = 0;
        state = &sState;
        e->state = state;
        state->phase = 0x81;
        state->count = 0;
        state->speed = 0x140;
        n = INTERACT(obj)->zoneCount;
        while (n--) {
            zone = &INTERACT(obj)->zones[n];
            zone->unk2F = 0;
            zone->flags.active = 1;
        }
        for (n = first; n < count; n++) {
            if (objs[n]->behaviorId == 0x61) {
                sArena = objs[n];
                break;
            }
        }
    }
    e->target = player;
    e->unk10 = func_80049C84(obj->trans.x_position - player->trans.x_position,
                             obj->trans.z_position - player->trans.z_position) -
               obj->trans.rotation.y_rotation;
    e->unkC = e->unk10;
    obj->trans.rotation.y_rotation = -0x8000;
    INTERACT(obj)->hitPoints = owner->hitPoints;
    switch (state->phase & 7) {
        case 1:
            if (!(e->parts & 0x20) && INTERACT(obj)->zones[5].damage > 0xA00) {
                e->unkA0 |= 0x20;
                INTERACT(obj)->zones[5].flags.shotAway = 1;
                chain_object_unlink_from_players(obj);
            }
            if (!(e->parts & 0x10) && INTERACT(obj)->zones[4].damage > 0xA00) {
                e->unkA0 |= 0x10;
                INTERACT(obj)->zones[4].flags.shotAway = 1;
                chain_object_unlink_from_players(obj);
            }
            break;
        case 2:
            if (!(e->parts & 4) && INTERACT(obj)->zones[9].damage > 0x1800) {
                chain_object_unlink_from_players(obj);
                INTERACT(obj)->zones[9].flags.shotAway = 1;
                INTERACT(obj)->zones[9].unk2F = 0;
                e->unkA0 |= 4;
                e->unk4B = 0;
                e->unk2_4 = 1;
                func_m003_04EC4(0, 6);
            }
            break;
        case 3:
            if (!(e->parts & 0x80) && INTERACT(obj)->zones[3].damage > 0x400) {
                e->unkA0 |= 0x80;
                INTERACT(obj)->zones[3].flags.shotAway = 1;
            }
            if (!(e->parts & 0x100) && INTERACT(obj)->zones[1].damage > 0x400) {
                e->unkA0 |= 0x100;
                INTERACT(obj)->zones[1].flags.shotAway = 1;
            }
            if (!(e->parts & 0x200) && INTERACT(obj)->zones[2].damage > 0x400) {
                e->unkA0 |= 0x200;
                INTERACT(obj)->zones[2].flags.shotAway = 1;
            }
            if (!(e->parts & 0x400) && INTERACT(obj)->zones[0].damage > 0x400) {
                e->unkA0 |= 0x400;
                INTERACT(obj)->zones[0].flags.shotAway = 1;
            }
            break;
        case 4:
            if (!(e->parts & 0x40) && INTERACT(obj)->zones[7].damage > 0x700) {
                e->unkA0 |= 0x40;
                INTERACT(obj)->zones[7].flags.shotAway = 1;
                state->phase = 5;
                func_m003_179B0(0x172, func_80005064(30));
            }
            break;
    }
    if ((e->parts & 0x80) && (e->parts & 0x100) && (e->parts & 0x200) && (e->parts & 0x400)) {
        if (INTERACT(obj)->zones[6].flags.active) {
            INTERACT(obj)->zones[6].flags.active = 0;
            INTERACT(obj)->zones[6].unk2F = 100;
            INTERACT(obj)->zones[7].flags.active = 0;
            INTERACT(obj)->zones[7].unk2F = 100;
            e->unkA4 |= 2;
            func_8000C9B4(0xFF, 0xFF, 0xFF, 1);
        }
    }
    if ((obj->particleEmittersEnabled & 1) && (e->parts & 0x10)) {
        obj->particleEmittersEnabled &= ~1;
    }
    if ((obj->particleEmittersEnabled & 2) && (e->parts & 0x20)) {
        obj->particleEmittersEnabled &= ~2;
    }
    if (owner->unk30 == 3 && e->unk4A == 0 && e->unk1A != 5 && state->phase != 4 &&
        func_m003_04260(0x10, 5, 0, 0, 0)) {
        cur = state->phase;
        phase = cur;
        action = 6;
        if (phase & 0x80) {
            action = 7;
        } else {
            state->unk0 = 0;
            switch (cur) {
                case 1:
                    if ((e->parts & 0x10) && (e->parts & 0x20)) {
                        phase = state->phase = 2;
                    }
                    break;
                case 2:
                    if (e->parts & 4) {
                        phase = state->phase = 3;
                    }
                    break;
                case 3:
                    if ((e->parts & 0x400) && (e->parts & 0x200) && (e->parts & 0x100) && (e->parts & 0x80)) {
                        phase = state->phase = 4;
                    }
                    break;
            }
            switch (phase) {
                case 1:
                    if (state->count < 0 || state->count >= 6) {
                        state->count = 0;
                    }
                    switch (state->count) {
                        case 2:
                            state->unk0 = 1;
                        case 0:
                            if (!(e->parts & 0x20)) {
                                action = 2;
                            } else {
                                action = 1;
                            }
                            break;
                        case 3:
                            state->unk0 = 1;
                        case 1:
                            if (!(e->parts & 0x10)) {
                                action = 1;
                            } else {
                                action = 2;
                            }
                            break;
                        case 5:
                            state->unk0 = 1;
                        case 4:
                            if (e->parts & 0x10) {
                                action = 2;
                            } else if (e->parts & 0x20) {
                                action = 1;
                            } else {
                                action = 3;
                            }
                            break;
                    }
                    break;
                case 2:
                case 3:
                    if (sBoltSide != 0) {
                        if (!(e->parts & 0x80) || !(e->parts & 0x200)) {
                            action = 5;
                        } else {
                            action = 4;
                        }
                    } else if (!(e->parts & 0x100) || !(e->parts & 0x400)) {
                        action = 4;
                    } else {
                        action = 5;
                    }
                    break;
                case 5:
                    action = 9;
                    break;
            }
        }
        func_m003_04EC4(6, action, phase);
        e->unkA = 5;
    }
}

/* What the boss does when module 3 tells it an event */
void func_m149_009A4(Object *obj, s32 event) {
    M149Enemy *e;
    M149State *state;
    Object *player;
    M149Entry entry;
    s32 lo;
    s32 hi;
    s32 d;
    Object *spawned;

    e = obj->unk68;
    state = e->state;
    player = func_m003_053C4();
    switch (event) {
        case 3:
            event = 5;
            while (event--) {
                entry.objectID = 0x30A;
                entry.size = 10;
                entry.unk3 = 0;
                lo = player->trans.x_position - 60.0f;
                hi = player->trans.x_position + 60.0f;
                d = sArena->trans.x_position - 205.0f - lo;
                if (d > 0) {
                    lo += d;
                    hi += d;
                }
                d = hi - (sArena->trans.x_position + 205.0f);
                if (d > 0) {
                    lo -= d;
                    hi -= d;
                }
                entry.x = func_80048E2C(lo, hi) + sArena->trans.x_position;
                entry.y = func_80048E2C(250, 290) + sArena->trans.y_position;
                entry.z = func_80048E2C(-120, 45) + sArena->trans.z_position;
                spawned = func_80005F14(&entry, 1);
                if (spawned != NULL) {
                    spawned->level_entry = NULL;
                }
            }
            sBoltSide ^= 1;
            switch (state->phase) {
                case 1:
                    if ((e->parts & 0x10) && (e->parts & 0x20)) {
                        state->phase = 2;
                    }
                    break;
                case 2:
                    if (e->parts & 4) {
                        state->phase = 3;
                    }
                    break;
                case 3:
                    if ((e->parts & 0x400) && (e->parts & 0x200) && (e->parts & 0x100) && (e->parts & 0x80)) {
                        state->phase = 4;
                    }
                    break;
                case 4:
                    break;
            }
            break;
        case 4:
            state->phase &= ~0x80;
            break;
        case 5:
            obj->particleEmittersEnabled |= 1;
            INTERACT(obj)->zones[4].flags.active = 0;
            INTERACT(obj)->zones[4].unk2F = 100;
            e->unkA4 |= 0x40;
            func_8000C9B4(0xFF, 0xFF, 0xFF, 1);
            break;
        case 6:
            INTERACT(obj)->zones[4].flags.active = 1;
            INTERACT(obj)->zones[4].unk2F = 0;
            state->phase |= 0x80;
            state->count++;
            break;
        case 7:
            obj->particleEmittersEnabled |= 2;
            INTERACT(obj)->zones[5].flags.active = 0;
            INTERACT(obj)->zones[5].unk2F = 100;
            e->unkA4 |= 0x80;
            func_8000C9B4(0xFF, 0xFF, 0xFF, 1);
            break;
        case 8:
            INTERACT(obj)->zones[5].flags.active = 1;
            INTERACT(obj)->zones[5].unk2F = 0;
            state->phase |= 0x80;
            state->count++;
            break;
        case 9:
            obj->particleEmittersEnabled |= 3;
            INTERACT(obj)->zones[5].flags.active = 0;
            INTERACT(obj)->zones[5].unk2F = 100;
            INTERACT(obj)->zones[4].flags.active = 0;
            INTERACT(obj)->zones[4].unk2F = 100;
            e->unkA4 |= 0xC0;
            func_8000C9B4(0xFF, 0xFF, 0xFF, 2);
            break;
        case 10:
            INTERACT(obj)->zones[5].flags.active = 1;
            INTERACT(obj)->zones[5].unk2F = 0;
            INTERACT(obj)->zones[4].flags.active = 1;
            INTERACT(obj)->zones[4].unk2F = 0;
            state->phase |= 0x80;
            if (state->count < 5) {
                state->count++;
            }
            break;
        case 11:
            if (state->phase == 2) {
                INTERACT(obj)->zones[9].flags.active = 0;
                INTERACT(obj)->zones[9].unk2F = 100;
                e->unkA4 |= 0x100;
                func_8000C9B4(0xFF, 0xFF, 0xFF, 1);
            } else if (INTERACT(obj)->zones[0].flags.active) {
                INTERACT(obj)->zones[0].flags.active = 0;
                INTERACT(obj)->zones[0].unk2F = 100;
                INTERACT(obj)->zones[1].flags.active = 0;
                INTERACT(obj)->zones[1].unk2F = 100;
                e->unkA4 |= 0xC;
                func_8000C9B4(0xFF, 0xFF, 0xFF, 2);
            }
            break;
        case 13:
            if (state->phase == 2) {
                INTERACT(obj)->zones[9].flags.active = 0;
                INTERACT(obj)->zones[9].unk2F = 100;
                e->unkA4 |= 0x100;
                func_8000C9B4(0xFF, 0xFF, 0xFF, 1);
            } else if (INTERACT(obj)->zones[2].flags.active) {
                INTERACT(obj)->zones[2].flags.active = 0;
                INTERACT(obj)->zones[2].unk2F = 100;
                INTERACT(obj)->zones[3].flags.active = 0;
                INTERACT(obj)->zones[3].unk2F = 100;
                e->unkA4 |= 0x30;
                func_8000C9B4(0xFF, 0xFF, 0xFF, 2);
            }
            break;
        case 12:
            if (state->phase == 2) {
                INTERACT(obj)->zones[9].flags.active = 1;
                INTERACT(obj)->zones[9].unk2F = 0;
            }
            state->phase |= 0x80;
            break;
        case 14:
            if (state->phase == 2) {
                INTERACT(obj)->zones[9].flags.active = 1;
                INTERACT(obj)->zones[9].unk2F = 0;
            }
            state->phase |= 0x80;
            break;
        case 16:
            break;
    }
}

/* A homing bolt: aims at a point near the player and starts sideways */
void func_m149_0121C(Object *obj, M149BoltSetup *setup) {
    M149Bolt *bolt;
    Object *player;
    f32 dx;
    f32 dz;

    bolt = obj->unk68;
    player = func_8000E4D0(0);
    setup->spawner->particleEmittersEnabled = 0;
    bolt->owner = setup->spawner;
    bolt->targetX = func_80048E2C(-40, 40) + player->trans.x_position;
    bolt->targetY = player->trans.y_position;
    bolt->targetZ = player->trans.z_position;
    bolt->prev.x = obj->trans.x_position;
    bolt->prev.y = obj->trans.y_position;
    bolt->prev.z = obj->trans.z_position;
    bolt->age = 0;
    dx = obj->trans.x_position - bolt->targetX;
    dz = obj->trans.z_position - bolt->targetZ;
    bolt->dist = func_80097140(dx * dx + dz * dz);
    bolt->height = obj->trans.y_position - bolt->targetY;
    sBoltCount++;
    if (sBoltCount == 1 || sBoltCount == 2 || sBoltCount == 7 || sBoltCount == 8) {
        obj->velocity.x = 0.0f;
    } else if (sBoltCount & 1) {
        obj->velocity.x = -8.0f;
    } else {
        obj->velocity.x = 8.0f;
    }
    obj->velocity.y = 0.0f;
    obj->velocity.z = 8.0f;
    func_80079D1C(obj, 10, 1, 1, 0, 0, 0, 16.0f, 16.0f, 1.0f, 0.75f, 0x11);
}

/* A homing bolt: turns towards its target point and drops onto it */
void func_m149_01450(Object *obj, s32 updateRate) {
    M149Bolt *bolt;
    f32 dz;
    f32 dx;
    f32 dist;
    HitResult hit;
    f32 f;
    f32 dy;
    f32 dx2;
    f32 f2;
    f32 turnZ;
    s16 newAngle;
    s16 angle;
    s16 diff;
    s32 hitWall;
    f32 rate;

    rate = updateRate;
    bolt = obj->unk68;
    bolt->age++;
    func_8001CE10(1, &bolt->prev, (Vec3f *) &obj->trans.x_position, &obj->header->unk50->radius, 0xCE000000, 0, 0);
    hitWall = func_80016208(obj, &bolt->prev, (Vec3f *) &obj->trans.x_position, obj->header->unk50->radius, obj->header->unk50->unk10, &hit);
    bolt->prev.x = obj->trans.x_position;
    bolt->prev.y = obj->trans.y_position;
    bolt->prev.z = obj->trans.z_position;
    if (hitWall) {
        func_800079B8(obj);
        return;
    }
    func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
    dx = bolt->targetX - obj->trans.x_position;
    dy = bolt->targetY - obj->trans.y_position;
    dz = bolt->targetZ - obj->trans.z_position;
    dist = point_distance(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, bolt->targetX,
                          bolt->targetY, bolt->targetZ);
    if (dist != 0.0f) {
        dx /= dist;
        dy /= dist;
        dz /= dist;
    }
    if (bolt->age < 10 || obj->velocity.x * dx + dy * obj->velocity.y + dz * obj->velocity.z >= 0.0f) {
        f = 200.0f / dist;
        if (f < 0.3f) {
            f = 0.3f;
        }
        if (f > 1.0f) {
            f = 1.0f;
        }
        dx = dx * 7.5f;
        dz = dz * 7.5f;
        f2 = (dx - obj->velocity.x) * f;
        turnZ = (dz - obj->velocity.z) * f;
        bolt->turnX += (f2 - bolt->turnX) * 0.11f;
        bolt->turnZ += (turnZ - bolt->turnZ) * 0.11f;
        angle = func_80049C84(obj->velocity.x, obj->velocity.z);
        dx = obj->velocity.x + bolt->turnX;
        dz = obj->velocity.z + bolt->turnZ;
        newAngle = func_80049C84(dx, dz);
        turnZ = func_80097140(dx * dx + dz * dz);
        diff = func_80049D80(angle, newAngle);
        if (diff > 3000) {
            diff = 3000;
        } else if (diff < -3000) {
            diff = -3000;
        }
        obj->velocity.x = func_80049C34((s16) (angle + diff)) * turnZ;
        obj->velocity.z = func_80049C30((s16) (angle + diff)) * turnZ;
    } else {
        func_800079B8(obj);
        return;
    }
    dx2 = obj->trans.x_position - bolt->targetX;
    dy = obj->trans.z_position - bolt->targetZ;
    f = func_80097140(dx2 * dx2 + dy * dy);
    obj->velocity.y = (bolt->targetY + bolt->height * (f / bolt->dist) - obj->trans.y_position) / rate;
    obj->particleEmittersEnabled = 1;
    obj_spawn_particle(obj, updateRate);
}

/* A bomb: starts at rest with a random fuse */
void func_m149_01868(Object *obj, UNUSED LevelObjectEntryCommon *setup) {
    M149Bomb *bomb;
    UNUSED s32 pad40;

    bomb = obj->unk68;
    bomb->landed = 0;
    bomb->timer = func_80048E2C(150, 250);
    obj->trans.scale = func_80049C34(func_80097140(bomb->timer * 1600.0f) * 600.0f) * 0.03f + 0.2f;
    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    obj->velocity.z = 0.0f;
    obj->trans.rotation.y_rotation = func_80048E2C(-0x8000, 0x7FFF);
    obj->trans.rotation.x_rotation = 0;
    obj->trans.rotation.z_rotation = 0;
    obj->level_entry = NULL;
    func_80079D1C(obj, 1, 1, 2, 0, 0, 0, INTERACT(obj)->unk24, INTERACT(obj)->unk24, -1.0f, 1.0f, 1);
    func_8001149C(obj, 1, 0.0f);
}

/* A bomb: falls until it lands, then pulses until its fuse runs out and explodes */
void func_m149_019C8(Object *obj, s32 updateRate) {
    M149Bomb *bomb;
    Object *spawned;
    HitResult hit;
    s32 i;
    Vec3f prev;
    M149BlastEntry entry;

    bomb = obj->unk68;
    if (bomb->landed == 0) {
        prev.x = obj->trans.x_position;
        prev.y = obj->trans.y_position;
        prev.z = obj->trans.z_position;
        i = updateRate;
        while (i--) {
            obj->velocity.y -= 0.14f;
            obj->trans.x_position += obj->velocity.x;
            obj->trans.y_position += obj->velocity.y;
            obj->trans.z_position += obj->velocity.z;
        }
        obj->segmentID = func_80015510(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        if (func_80018808(obj, &prev, (Vec3f *) &obj->trans.x_position, &hit, 0x2000, 0)) {
            func_80009A04(obj, hit.pos.x - obj->trans.x_position, hit.pos.y - obj->trans.y_position,
                          hit.pos.z - obj->trans.z_position);
            bomb->landed = 1;
            func_80003410(func_80048E2C(0xAA, 0xAD), obj->trans.x_position, obj->trans.y_position,
                          obj->trans.z_position, 4, NULL);
        }
    } else if (updateRate >= bomb->timer) {
        entry.objectID = 0x13C;
        entry.x = obj->trans.x_position;
        entry.y = obj->trans.y_position;
        entry.z = obj->trans.z_position;
        entry.yRotation = obj->trans.rotation.y_rotation;
        entry.xRotation = obj->trans.rotation.x_rotation;
        entry.unk10 = 2;
        entry.unk12 = 0;
        entry.unk13 = 1;
        entry.unk11 = 100;
        entry.unk14 = 0;
        spawned = func_80005F14(&entry, 1);
        if (spawned != NULL) {
            spawned->level_entry = NULL;
        }
        func_80029BA4(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 160.0f, 80.0f, 0.5f, 100,
                      NULL);
        func_80003410(0x22, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
        func_800079B8(obj);
    } else {
        bomb->timer -= updateRate;
        obj->trans.scale = func_80049C34(func_80097140(bomb->timer * 1600.0f) * 600.0f) * 0.03f + 0.2f;
        func_8001138C(obj, 0.0167f, updateRate);
    }
}

/* Moves the boss: from its start to a destination (random, or set by the phase) along a cosine curve */
void func_m149_01D20(Object *obj, f32 updateRate) {
    M149Enemy *e;
    M149State *state;
    f32 f;
    f32 x;
    f32 y;
    f32 z;
    s32 r;

    e = obj->unk68;
    state = e->state;
    if (state != NULL && (e->parts & 4)) {
        if (state->phase == 5) {
            obj->velocity.x *= 0.4f;
            obj->velocity.z *= 0.4f;
            obj->velocity.y *= 0.4f;
            func_80009A04(obj, obj->velocity.x * updateRate, obj->velocity.y * updateRate,
                          obj->velocity.z * updateRate);
        } else if (state->t == 0) {
            if (state->speed < 0x400) {
                state->speed += 2;
            }
            r = func_80048E2C(-50, 50);
            sSpeed = func_80048E2C(-20, 20) + state->speed + r;
            if (state->phase == 4) {
                if (-142.009552 < e->target->trans.x_position && e->target->trans.x_position < 130.524169 &&
                    (sSwoopSide != 0 || func_80048E2C(1, 4) == 1)) {
                    state->destX = e->target->trans.x_position;
                    state->destY = 165.84201f;
                    state->destZ = 1306.6984f;
                    func_m003_04EC4(6, 8);
                    sSpeed = 0x18D;
                    sSwoopSide ^= 1;
                } else {
                    state->destX = func_80048E2C(-764, 626);
                    state->destY = func_80048E2C(79, 375);
                    state->destZ = func_80048E2C(-239, 1150);
                }
            } else if (func_80048E2C(1, 6) == 1) {
                state->destX = -66.0501f;
                state->destY = 296.058f;
                state->destZ = 806.8733f;
            } else {
                state->destX = func_80048E2C(-764, 626);
                state->destY = func_80048E2C(79, 375);
                state->destZ = func_80048E2C(-239, 1150);
            }
            state->startX = obj->trans.x_position;
            state->startY = obj->trans.y_position;
            state->startZ = obj->trans.z_position;
            state->t = sSpeed * updateRate;
        } else {
            f = (func_80049C30(state->t + 0x8000) + 1.0f) * 0.5f;
            x = state->startX + (state->destX - state->startX) * f;
            y = state->startY + (state->destY - state->startY) * f;
            z = state->startZ + (state->destZ - state->startZ) * f;
            obj->velocity.x = (x - obj->trans.x_position) / updateRate;
            obj->velocity.y = (y - obj->trans.y_position) / updateRate;
            obj->velocity.z = (z - obj->trans.z_position) / updateRate;
            state->t += sSpeed * updateRate;
            if (state->t > 0x8000) {
                state->t = 0;
            }
            func_80009A04(obj, obj->velocity.x * updateRate, obj->velocity.y * updateRate,
                          obj->velocity.z * updateRate);
        }
    }
}
