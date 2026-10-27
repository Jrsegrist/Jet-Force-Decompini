/*
 * Runlink module 140: target choice for an ant-like enemy. Of the live
 * objects in the active groups it picks the nearest one (in its home's zone
 * when the home asks for that), notices the player inside its view cone and
 * range when there is a clear line, and goes for the player once it has
 * noticed it and the player is clearly nearer.
 */
#include "module.h"

/* What the enemy's home object keeps (that object's unk68) */
typedef struct M140Home {
    /* 0x00 */ u8 pad0[0x1F];
    /* 0x1F */ u8 viewRange;  /* range = viewRange^2 << 12 (squared distance) */
    /* 0x20 */ u8 viewAngle;  /* half cone, in degrees */
    /* 0x21 */ u8 pad21[0x32 - 0x21];
    /* 0x32 */ u8 kind;       /* 0x11: targets must be in its zone */
    /* 0x33 */ u8 pad33[0x7A - 0x33];
    /* 0x7A */ s8 zone;
} M140Home;

/* The part of module 3's enemy record that this enemy uses */
typedef struct M140Enemy {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ u16 pad4_0 : 15;
    /* 0x04 */ u16 seen : 1; /* has noticed the player */
    /* 0x06 */ u8 pad6[0x24 - 0x6];
    /* 0x24 */ Object *home;
    /* 0x28 */ u8 pad28[0x64 - 0x28];
    /* 0x64 */ Object *target;
    /* 0x68 */ f32 x;
    /* 0x6C */ f32 y;
    /* 0x70 */ f32 z;
} M140Enemy;

/* A target candidate's data (its unk68) */
typedef struct M140Candidate {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ Object *next;
    /* 0x2C */ u8 pad2C[0xA6 - 0x2C];
    /* 0xA6 */ s16 health;
} M140Candidate;

/* An entry of the object group list (its unk68) */
typedef struct M140Group {
    /* 0x00 */ u8 pad0[0x2C];
    /* 0x2C */ u16 id;
    /* 0x2E */ u8 pad2E[0x58 - 0x2E];
    /* 0x58 */ Object *first;
} M140Group;

typedef struct M140Point {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} M140Point;

extern void func_800541B0(s32, s32, s32);
extern Object **func_80050F44(s32 *count);
extern s32 func_80050ED0(s32 id);
extern f32 func_80011CE0(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2); /* squared distance? */
extern s32 func_80049C84(f32 dx, f32 dz);
extern s32 func_8007B99C(Object *obj, Object *other, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
extern Object *func_m003_053C4(void);
extern s32 func_m003_17910(f32 x, f32 z, s32 zone);
extern void func_m003_0CD34(Object *obj);

s32 D_m140_003B0 = 140; /* the module's number */

void func_m140_00000(Object *obj) {
    M140Enemy *e;
    M140Home *home;
    Object **list;
    s32 count;
    Object *o;
    M140Candidate *c;
    Object *target;
    Object *player;
    f32 best;
    f32 playerDist;
    f32 dist;
    f32 x;
    f32 y;
    f32 z;
    s16 angle;

    e = obj->unk68;
    best = 5e9f;
    home = e->home->unk68;
    list = func_80050F44(&count);
    x = e->x;
    y = e->y;
    z = e->z;
    target = NULL;
    playerDist = -1.0f;
    player = func_m003_053C4();
    func_800541B0(D_m140_003B0, 10, 5);
    while (count--) {
        o = list[count];
        if (func_80050ED0(((M140Group *) o->unk68)->id) != 0) {
            for (o = ((M140Group *) o->unk68)->first; o != NULL; o = c->next) {
                c = o->unk68;
                if (o->unkA0 == 0 && c->health > 0) {
                    dist = func_80011CE0(o->trans.x_position, o->trans.y_position, o->trans.z_position, x, y, z);
                    if (dist < best && (home->kind != 0x11 || func_m003_17910(o->trans.x_position, o->trans.z_position, home->zone) != 0)) {
                        best = dist;
                        target = o;
                    }
                }
            }
        }
    }
    if (!e->seen) {
        angle = func_80049C84(x - player->trans.x_position, z - player->trans.z_position);
        if (angle < 0) {
            angle = -angle;
        }
        playerDist = func_80011CE0(x, y, z, player->trans.x_position, player->trans.y_position, player->trans.z_position);
        if (playerDist < (f32) ((home->viewRange << 12) * home->viewRange) && angle < home->viewAngle * 91) {
            if (func_8007B99C(obj, player, x, y, z, ((M140Point *) player->interactObj)->x,
                              ((M140Point *) player->interactObj)->y, ((M140Point *) player->interactObj)->z) == 0) {
                e->seen = 1;
            }
        }
    }
    if (e->seen) {
        if (playerDist == -1.0f) {
            playerDist = func_80011CE0(x, y, z, player->trans.x_position, player->trans.y_position, player->trans.z_position);
        }
        if (playerDist + playerDist < best) {
            target = player;
        }
    }
    if (target != NULL) {
        e->target = target;
    } else {
        e->target = player;
    }
    func_m003_0CD34(obj);
}
