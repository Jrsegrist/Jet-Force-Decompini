/*
 * Runlink module 146: a dog-like enemy with a small state machine. It
 * wanders around its zone (circling, sitting, scratching), runs off when its
 * way is blocked, goes after food from module 139's list, chases the
 * player when it sees it or a kind-5 object comes near, and returns. A
 * state can only be entered from the states its mask allows.
 */
#include "module.h"

/* Its home object's data (that object's unk68) */
typedef struct M146Home {
    /* 0x00 */ u8 pad0[0x7A];
    /* 0x7A */ s8 zone;
} M146Home;

/* The part of module 3's enemy record that this enemy uses */
typedef struct M146Enemy {
    /* 0x00 */ u32 pad0_0 : 2;
    /* 0x00 */ u32 spotted : 1;
    /* 0x00 */ u32 pad0_1 : 11;
    /* 0x00 */ u32 sitting : 1;
    /* 0x00 */ u32 pad0_2 : 17;
    /* 0x04 */ u32 pad4_0 : 17;
    /* 0x04 */ u32 turning : 1;
    /* 0x04 */ u32 pad4_1 : 14;
    /* 0x08 */ u8 pad8[0x10 - 0x8];
    /* 0x10 */ s16 turn;
    /* 0x12 */ u8 pad12[0x1A - 0x12];
    /* 0x1A */ u8 mission;
    /* 0x1B */ u8 pad1B[0x20 - 0x1B];
    /* 0x20 */ f32 speed;
    /* 0x24 */ Object *home;
    /* 0x28 */ u8 pad28[0x38 - 0x28];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0x50 - 0x44];
    /* 0x50 */ u8 timer;
    /* 0x51 */ u8 dir;
    /* 0x52 */ u8 radius;
    /* 0x53 */ s8 angle;
    /* 0x54 */ u8 pad54[0x64 - 0x54];
    /* 0x64 */ Object *target;
} M146Enemy;

/* What the enemy's interactObj points at */
typedef struct M146Interact {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 radius;
} M146Interact;

extern s32 func_m003_04D08(s32, s32, s32, s32);
extern s32 func_m003_04F88(s32, f32);
extern Object *func_m003_053C4(void);
extern s32 func_m003_05450(Object *obj);
extern void func_m003_06754(Object *obj, f32 *out);
extern f32 func_m003_03E68(f32 x1, f32 z1, f32 x2, f32 z2);
extern s32 func_m003_17910(f32 x, f32 z, s32 zone);
extern s32 func_m003_1794C(f32 x1, f32 z1, f32 x2, f32 z2, s32 zone, s32, s32, s32);
extern Object *func_m139_00184(s16 x, s16 z, s32 radius, s32 zone);
extern void func_8001191C(Object *obj, Object *target);
extern void func_80011974(Object *obj);
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle); /* cosine */
extern s32 func_80049C84(f32 dx, f32 dz); /* angle of a vector */

/*
 * NON_MATCHING: everything here matches except func_m146_00350 (8
 * instructions: in the `mission != 0` branch the store of sState and the
 * address of sNextState swap places around the branch delay slot, and k/d
 * get v0/v1 the other way round in the blocked-path loop). Since the
 * module's .rodata (jump tables and floats of 00000, 00350 and 00B34)
 * cannot be split between C and assembly, the matching build keeps the
 * whole module in assembly (the #else at the end).
 */
#ifdef NON_MATCHING
/* For each state, the states (bit n = state n) it can be entered from */
static s32 sStateMasks[18] = {
    0x1001, 0x1FFF, 0x1BE3, 0x1003, 0x1009, 0x1003, 0x1021, 0x107B, 0x1081,
    0x11FB, 0x1003, 0x1401, 0x1401, 0x11BFF, 0x33FFF, 0x37FFF, 0x1BE7, 0x13FFF,
};

/* The state that each event asks for (0: none) */
static u8 sEventStates[40] = {
    0, 0, 0, 0, 0, 0, 0, 0x0D, 0, 0, 0, 0x0E, 0x0F, 0, 0, 0x10, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static s32 sState;
static s32 sNextState; /* the state to go back to after running off */

/* Enters `state` if the current state allows it; returns whether it did */
s32 func_m146_00000(Object *obj, s32 state) {
    M146Enemy *e = obj->unk68;
    s32 mask = sStateMasks[state];
    Object *homeObj = e->home;
    M146Home *home = homeObj->unk68;
    s32 anim = 0;
    s32 i;
    s16 angle;
    f32 r;
    f32 x;
    f32 z;
    f32 floor;

    func_m003_05450(obj);
    if ((1 << sState) & mask) {
    sState = state;
    e->sitting = 0;
    switch (sState) {
        case 1:
            e->timer = func_80048E2C(10, 50);
            e->radius = func_80048E2C(100, 200);
            e->angle = func_80048E2C(-50, 50);
            anim = 0xB;
            break;
        case 2:
            anim = 0xB;
            i = 9;
            do {
                angle = func_80048E2C(-0x8000, 0x7FFF);
                r = func_80048E2C(100, 250);
                x = obj->trans.x_position - func_80049C34(angle) * r;
                z = obj->trans.z_position - func_80049C30(angle) * r;
                if (func_m003_17910(obj->trans.x_position, obj->trans.z_position, home->zone) != 0
                        ? func_m003_1794C(obj->trans.x_position, obj->trans.z_position, x, z, home->zone, 0, -1, 0xFF) == 0
                        : func_m003_17910(x, z, home->zone)) {
                    e->x = x;
                    e->z = z;
                    break;
                }
            } while (i--);
            break;
        case 3:
            e->timer = func_80048E2C(200, 250);
            e->dir = func_80048E2C(0, 1);
            anim = 0xE;
            break;
        case 4:
            e->timer = func_80048E2C(50, 70);
            anim = 0x36;
            break;
        case 5:
            e->timer = func_80048E2C(2, 5);
            anim = 0x26;
            break;
        case 6:
        case 7:
            anim = 0xD;
            break;
        case 8:
            e->timer = func_80048E2C(3, 5);
            anim = 0x36;
            break;
        case 9:
            anim = 0xE;
            e->sitting = 1;
            break;
        case 10:
            func_m003_06754(obj, &floor);
            if (floor <= 0.0f) {
                e->timer = func_80048E2C(20, 40);
                anim = 0x2A;
                break;
            }
            return func_m146_00000(obj, 1);
        case 11:
            e->timer = 40;
            anim = 0x29;
            break;
        case 17:
            func_m003_04D08(0xB, 0x15, 0, 0);
            e->timer = func_80048E2C(150, 250);
            return TRUE;
    }
    if (anim != 0) {
        func_m003_04D08(3, anim, 0, 0);
    }
    return TRUE;
    }
    return FALSE;
}

/* The mover: picks the state, then where to go in it */
void func_m146_00350(Object *obj) {
    M146Enemy *e;
    Object *homeObj;
    M146Home *home;
    Object *player;
    f32 x;
    f32 z;
    s32 i;
    s32 k;
    s32 blocked;
    s32 d;
    f32 r;
    s32 angle;
    f32 nearest;
    s32 start;
    s32 end;
    s32 still;
    Object **objs;
    Object *o;
    f32 floor;

    e = obj->unk68;
    homeObj = e->home;
    home = homeObj->unk68;
    func_m003_053C4();
    still = FALSE;
    objs = func_80005E98(&start, &end);
    func_m003_05450(obj);
    player = func_m003_053C4();
    x = player->trans.x_position;
    z = player->trans.z_position;
    nearest = 5e9f;
    for (i = start; i < end; i++) {
        o = objs[i];
        if (o->behaviorId == 5) {
            r = func_m003_03E68(o->trans.x_position, o->trans.z_position, x, z);
            if (r < nearest) {
                nearest = r;
            }
        }
    }
    if (e->mission != 0) {
        sState = 12;
    } else {
        blocked = FALSE;
        if (func_m003_17910(obj->trans.x_position, obj->trans.z_position, home->zone) == 0) {
            blocked = TRUE;
        }
        if (!blocked) {
            i = 6;
            do {
                k = i - 3;
                angle = (s16) (obj->trans.rotation.y_rotation + (k << 12));
                if (k > 0) {
                    d = k;
                } else {
                    d = -k;
                }
                r = (3.0f - d) * ((M146Interact *) obj->interactObj)->radius;
                x = obj->trans.x_position - func_80049C34(angle) * r;
                z = obj->trans.z_position - func_80049C30(angle) * r;
                if (func_m003_17910(x, z, home->zone) == 0) {
                    blocked = TRUE;
                    break;
                }
            } while (i--);
        }
        if (blocked && (end = sState, func_m146_00000(obj, 2) != 0)) {
            sNextState = end;
        } else {
            o = func_m139_00184(obj->trans.x_position, obj->trans.z_position, 2000, home->zone);
            if (o != NULL && func_m146_00000(obj, 7) != 0) {
                e->target = o;
                func_8001191C(obj, o);
            } else if (nearest < 5000.0f && func_m146_00000(obj, 9) != 0) {
            } else {
            }
        }
    }
    switch (sState) {
        case 0:
            func_m146_00000(obj, 1);
            break;
        case 1:
            angle = (s16) (obj->trans.rotation.y_rotation + (e->angle << 5) - 0x8000);
            e->x = func_80049C34(angle) * e->radius + obj->trans.x_position;
            e->z = func_80049C30(angle) * e->radius + obj->trans.z_position;
            func_m003_04F88(3, e->speed);
            if (--e->timer == 0) {
                d = func_80048E2C(0, 100);
                if (d > 96) {
                    func_m146_00000(obj, 3);
                } else if (d > 92) {
                    func_m146_00000(obj, 10);
                } else {
                    func_m146_00000(obj, 1);
                }
            }
            break;
        case 2:
            func_m003_04F88(3, e->speed);
            if (func_m003_03E68(obj->trans.x_position, obj->trans.z_position, e->x, e->z) < 600.0f) {
                sState = 0;
                func_m146_00000(obj, sNextState);
                sNextState = 1;
            }
            break;
        case 3:
            still = TRUE;
            func_m003_04F88(3, 1.5f);
            if (--e->timer == 0) {
                func_m146_00000(obj, 4);
            }
            break;
        case 4:
            still = TRUE;
            func_m003_04F88(3, 1.0f);
            if (--e->timer == 0) {
                func_m146_00000(obj, 1);
            }
            break;
        case 5:
            still = TRUE;
            if (func_m003_04F88(3, 1.0f) != 0) {
                if (--e->timer == 0) {
                    func_m146_00000(obj, 6);
                }
            }
            break;
        case 6:
            e->x = player->trans.x_position;
            e->z = player->trans.z_position;
            func_m003_04F88(3, e->speed);
            if (func_m003_03E68(obj->trans.x_position, obj->trans.z_position, e->x, e->z) < 2000.0f) {
                func_m146_00000(obj, 1);
            }
            break;
        case 12:
            if (e->mission == 0) {
                func_m146_00000(obj, 1);
            }
            break;
        case 7:
            e->x = e->target->trans.x_position;
            e->z = e->target->trans.z_position;
            func_m003_04F88(3, e->speed);
            if (func_m003_03E68(obj->trans.x_position, obj->trans.z_position, e->x, e->z) < 2000.0f) {
                func_m146_00000(obj, 8);
            }
            break;
        case 8:
            still = TRUE;
            if (func_m003_04F88(3, 1.0f) != 0) {
                if (--e->timer == 0) {
                    func_m146_00000(obj, 1);
                    func_800079B8(e->target);
                }
            }
            break;
        case 9:
            e->x = player->trans.x_position;
            e->z = player->trans.z_position;
            func_m003_04F88(3, e->speed * 0.1f);
            if (50000.0f < nearest) {
                func_m146_00000(obj, 1);
            }
            break;
        case 10:
            still = TRUE;
            func_m003_04F88(3, e->speed);
            func_m003_06754(obj, &floor);
            if (0.0f < floor) {
                func_m146_00000(obj, 11);
            }
            break;
        case 11:
            func_m003_04F88(3, 1.0f);
            if (--e->timer == 0) {
                func_m146_00000(obj, 1);
            }
            break;
    }
    if (still) {
        e->x = obj->trans.x_position;
        e->z = obj->trans.z_position;
        obj->velocity.x = 0.0f;
        obj->velocity.z = 0.0f;
    }
}

/* The facer: the angle the enemy turns to */
s16 func_m146_00B34(Object *obj, f32 x, f32 z) {
    M146Enemy *e = obj->unk68;
    s16 turn;

    switch (sState) {
        case 3:
            if (e->dir != 0) {
                turn = 0x7724;
            } else {
                turn = -0x7724;
            }
            e->turn = turn;
            e->turning = 1;
            return obj->trans.rotation.y_rotation + turn;
        case 5:
            return func_80049C84(obj->trans.x_position - e->target->trans.x_position,
                                 obj->trans.z_position - e->target->trans.z_position);
        case 8:
        case 10:
        case 11:
        case 17:
            return obj->trans.rotation.y_rotation;
        default:
            return func_80049C84(x - obj->trans.x_position, z - obj->trans.z_position) - 0x8000;
    }
}

/* Called when the enemy spots the player */
void func_m146_00C34(Object *obj) {
    M146Enemy *e = obj->unk68;

    e->spotted = 0;
    func_m146_00000(obj, 5);
}

/* Called when the food it went for has gone */
void func_m146_00C68(Object *obj) {
    M146Enemy *e = obj->unk68;

    func_m003_05450(obj);
    if (sState == 7 || sState == 8) {
        func_80011974(obj);
        e->target = func_m003_053C4();
        func_m146_00000(obj, 1);
    }
}

/* Whether the current state allows the state that `event` asks for */
s32 func_m146_00CD8(s32 event) {
    u8 state = sEventStates[event];
    s32 mask = sStateMasks[state];

    if (state == 0) {
        return FALSE;
    }
    return (1 << sState) & mask;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m146", func_m146_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m146", func_m146_00350);

INCLUDE_ASM("asm/nonmatchings/modules/m146", func_m146_00B34);

INCLUDE_ASM("asm/nonmatchings/modules/m146", func_m146_00C34);

INCLUDE_ASM("asm/nonmatchings/modules/m146", func_m146_00C68);

INCLUDE_ASM("asm/nonmatchings/modules/m146", func_m146_00CD8);

INCLUDE_ASM("asm/nonmatchings/modules/m146", data);

INCLUDE_ASM("asm/nonmatchings/modules/m146", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m146", bss);
#endif
