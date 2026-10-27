/*
 * Runlink module 139: a list of up to 32 objects that enemy squads go
 * after (dead or flagged objects of kind 0x18, objects of kind 0x40 in
 * state 0 or 2, and every object of kind 0x4D), rebuilt from the active
 * objects, with queries for the nearest one, whether one is in a zone and
 * whether an object is on the list.
 */
#include "module.h"

#define M139_MAX 32

/* An object of kind 0x18's data (its unk68) */
typedef struct M139Kind18 {
    /* 0x00 */ u8 pad0[0x2];
    /* 0x02 */ u8 flags;
    /* 0x03 */ u8 pad3[0xA6 - 0x3];
    /* 0xA6 */ s16 health;
} M139Kind18;

/* An object of kind 0x40's data (its unk68) */
typedef struct M139Kind40 {
    /* 0x00 */ u8 pad0[0x17];
    /* 0x17 */ u8 state;
} M139Kind40;

extern s32 D_800FE9B0;

extern void func_800541B0(s32, s32, s32);
extern Object **func_80005E98(s32 *start, s32 *end);
extern f32 func_m003_03E68(f32 x1, f32 z1, f32 x2, f32 z2);
extern s32 func_m003_17910(f32 x, f32 z, s32 zone);

s32 D_m139_003E0 = 139; /* the module's number */

static Object *sList[M139_MAX];
static s32 sCount;

void func_m139_00030(void);

/* The module's init (runlink init) */
void func_m139_00000(void) {
    func_m139_00030();
    D_800FE9B0 |= 4;
}
#pragma weak __rl_init = func_m139_00000

void func_m139_00030(void) {
    s32 start;
    s32 end;
    s32 i;
    Object *o;
    Object **objs;
    Object **dst;
    s32 add;
    M139Kind18 *k18;
    M139Kind40 *k40;

    objs = func_80005E98(&start, &end);
    func_800541B0(D_m139_003E0, 2, 2);
    dst = sList;
    sCount = 0;
    for (i = start; i < end; i++) {
        o = objs[i];
        if (o->unkA0 == 0) {
            add = FALSE;
            switch (o->behaviorId) {
                case 0x18:
                    k18 = o->unk68;
                    if ((k18->flags & 1) || k18->health <= 0) {
                        add = TRUE;
                    }
                    break;
                case 0x40:
                    k40 = o->unk68;
                    if (k40->state == 0 || k40->state == 2) {
                        add = TRUE;
                    }
                    break;
                case 0x4D:
                    add = TRUE;
                    break;
            }
            if (add) {
                *dst++ = o;
                if (++sCount >= M139_MAX) {
                    break;
                }
            }
        }
    }
}

/* The listed object nearest to (x, z) within radius (and in zone, if not 0) */
Object *func_m139_00184(s16 x, s16 z, s32 radius, s32 zone) {
    s32 best;
    Object *found;
    s32 i;
    s32 dist;
    Object *o;

    radius *= radius;
    best = 0x7FFFFFFF;
    found = NULL;
    for (i = 0; i < sCount; i++) {
        o = sList[i];
        dist = func_m003_03E68(x, z, o->trans.x_position, o->trans.z_position);
        if (dist < radius) {
            if (zone != 0 && func_m003_17910(o->trans.x_position, o->trans.z_position, zone) == 0) {
                dist = 0x7FFFFFFF;
            }
            if (dist < best) {
                best = dist;
                found = o;
            }
        }
    }
    return found;
}

/* Whether a listed object is in the zone */
s32 func_m139_002F8(s32 zone) {
    s32 i;

    for (i = 0; i < sCount; i++) {
        if (func_m003_17910(sList[i]->trans.x_position, sList[i]->trans.z_position, zone) != 0) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Whether obj is on the list */
s32 func_m139_0038C(Object *obj) {
    s32 n = sCount;
    Object **p = sList;

    while (n--) {
        if (obj == *p++) {
            return TRUE;
        }
    }
    return FALSE;
}
