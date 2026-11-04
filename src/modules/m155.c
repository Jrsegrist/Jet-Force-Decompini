/*
 * Runlink module 155: the enemy AI shared by the soldier enemies: where the
 * player was last spotted, paths between nodes, the enemies' states and
 * which way they face.
 */
#include "module.h"

/* A work buffer (size 0x10) */
typedef struct M155Buffer {
    /* 0x0 */ u8 pad0[8];
    /* 0x8 */ void *data;
    /* 0xC */ u8 padC[4];
} M155Buffer;

/* The current enemy's flags */
typedef struct M155Flags {
    /* 0x0 */ u32 unk0 : 8;
    /* 0x1 */ u32 unk1 : 8;
    u32 pad : 13;
    u32 alerted : 1;
    u32 pad2 : 2;
    /* 0x04 */ u8 pad4[0x14 - 0x4];
    /* 0x14 */ s16 nextX; /* the next point of its path */
    /* 0x16 */ s16 nextZ;
} M155Flags;

/* The current enemy's state */
typedef struct M155Enemy {
    /* 0x00 */ u8 pad0[0x1A];
    /* 0x1A */ u8 state;
    /* 0x1B */ u8 pad1B[0x38 - 0x1B];
    /* 0x38 */ f32 goalX;
    /* 0x3C */ f32 goalY;
    /* 0x40 */ f32 goalZ;
    /* 0x44 */ u8 pad44[0x64 - 0x44];
    /* 0x64 */ Object *target;
} M155Enemy;

extern void func_8004AA80(void *data); /* DKR mempool_free */
extern s32 func_80049C84(f32 a, f32 b); /* the angle of (a, b) */

/* .bss, still assembly */
extern s16 rl_m155_04F64; /* where the player was last spotted */
extern s16 rl_m155_04F66;
extern M155Enemy *rl_m155_04F70;
extern M155Flags *rl_m155_04F7C;
extern M155Buffer rl_m155_04F98[4];
#define sLastX rl_m155_04F64
#define sLastZ rl_m155_04F66
#define sEnemy rl_m155_04F70
#define sFlags rl_m155_04F7C
#define sBuffers rl_m155_04F98

/* still assembly */
extern s32 rl_m155_01584(Object *obj, s32 arg1);

/* The module's init: the player is where it was last spotted */
void func_m155_00000(void) {
    Object *player = func_8000E4D0(0);

    sLastX = player->trans.x_position;
    sLastZ = player->trans.z_position;
}
#pragma weak __rl_init = func_m155_00000

/* The module's fini: frees the work buffers */
void func_m155_00070(void) {
    s32 i;
    M155Buffer *buf;

    buf = sBuffers;
    i = 4;
    while (i--) {
        if (buf->data != NULL) {
            func_8004AA80(buf->data);
        }
        buf++;
    }
}
#pragma weak __rl_fini = func_m155_00070

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_000C8);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_001B4);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_00268);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_00644);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_00FC4);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01274);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01584);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_018C4);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01900);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_019A4);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01A00);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01BE0);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01D9C);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01F9C);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_02248);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_022C8);

/* A path through the level's regions (points 1.. are the crossings) */
typedef struct M155Path {
    /* 0x00 */ s16 x[20];
    /* 0x28 */ s16 z[20];
    /* 0x50 */ u8 from[20]; /* the regions each crossing is between */
    /* 0x64 */ u8 to[20];
    /* 0x78 */ u8 count;
} M155Path;

/* Where a segment leaves a region (module 3's) */
typedef struct M155Crossing {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 z;
    /* 0x04 */ u8 pad4[0xA - 0x4];
} M155Crossing;

extern u8 rl_m155_03CD0[0x1080]; /* the path search's work areas */
extern u8 rl_m155_04D60[0x200];
extern u8 *rl_m155_04D58;
extern s32 rl_m155_04D5C;
extern s32 rl_m155_04F60;
extern void *rl_m155_04F80; /* the enemy's region */
extern M155Path *rl_m155_022C8(s16 x, s16 z, s16 toX, s16 toZ);
extern void rl_m155_01D9C(s16 *x, s16 *z, u8 from, u8 to);
extern s32 func_m003_175E8(f32 x1, f32 z1, f32 x2, f32 z2, void *region, M155Crossing *out, s32 point, u8 index);

/* Finds the enemy's path from (x, z) to (toX, toZ) and sets the first point to head for */
void func_m155_02458(Object *obj, s16 x, s16 z, s16 toX, s16 toZ) {
    M155Path *path;
    s32 i;
    M155Crossing cross;
    s32 d;
    s16 *p;

    bzero(rl_m155_03CD0, sizeof(rl_m155_03CD0));
    bzero(rl_m155_04D60, sizeof(rl_m155_04D60));
    rl_m155_04D58 = rl_m155_03CD0;
    rl_m155_04D5C = 0x3F;
    rl_m155_04F60 = 0;
    path = rl_m155_022C8(x, z, toX, toZ);
    if (path == NULL) {
        sEnemy->goalX = x;
        sEnemy->goalY = obj->trans.y_position;
        sEnemy->goalZ = z;
        return;
    }
    rl_m155_04D5C--;
    if (path->count >= 2) {
        for (i = 2; i < path->count; i++) {
            if (func_m003_175E8(x, z, path->x[i], path->z[i], rl_m155_04F80, &cross, -1, 0xFF)) {
                /* (twice the product, as the ROM computes it) */
                d = (cross.x - path->x[i]) * (cross.x - path->z[i]) + (cross.x - path->x[i]) * (cross.x - path->z[i]);
                if ((u32) d > 16) {
                    break;
                }
            }
        }
    } else {
        i = 2;
    }
    p = &path->x[i];
    if (path->from[i - 1] != 0xFF) {
        rl_m155_01D9C(p - 1, p + 19, path->from[i - 1], path->to[i - 1]);
    }
    sFlags->nextX = p[-1];
    sFlags->nextZ = p[19];
}

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_026D8);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_02748);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_0289C);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_02AEC);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_02BB0);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_02FD0);

/* The way `obj` faces: at the enemy's target once it is alerted, else `yaw` */
s16 func_m155_03B94(Object *obj, s16 yaw) {
    Object *target;

    rl_m155_01584(obj, 0);
    if (sFlags->alerted || sFlags->unk1 != 0) {
        target = sEnemy->target;
        if (target != NULL && sEnemy->state != 11) {
            return -0x4000 - func_80049C84(target->trans.z_position - obj->trans.z_position,
                                           target->trans.x_position - obj->trans.x_position);
        }
    }
    return yaw;
}

/* Sets where the player was last spotted */
void func_m155_03C4C(s16 x, s16 z) {
    sLastX = x;
    sLastZ = z;
}

INCLUDE_ASM("asm/nonmatchings/modules/m155", data);

INCLUDE_ASM("asm/nonmatchings/modules/m155", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m155", bss);
