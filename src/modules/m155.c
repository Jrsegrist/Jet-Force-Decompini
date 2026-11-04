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
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 pad8[0x14 - 0x8];
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

/* A region of the level, as 001B4 uses it (module 3's) */
typedef struct M155Region {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ struct M155Region *next; /* the regions inside it */
    /* 0x14 */ u32 flags; /* 0x10: a hole in the one it is in */
} M155Region;

extern s32 func_m003_16F44(f32 x, f32 z, M155Region *region);

/* Whether (x, z) is in the region and in none of its holes */
s32 func_m155_001B4(f32 x, f32 z, M155Region *region) {
    M155Region *r = region;

    if (!func_m003_16F44(x, z, r)) {
        return FALSE;
    }
    for (r = r->next; r != NULL; r = r->next) {
        if ((r->flags & 0x10) && func_m003_16F44(x, z, r)) {
            return FALSE;
        }
    }
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_00268);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_00644);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_00FC4);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01274);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01584);

/* A path node, as 018C4 and 026D8 use it */
typedef struct M155Node {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 z;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 stamp; /* the search that last visited it, or 0 */
} M155Node;

extern s32 rl_m155_04F88; /* the current search's stamp (.bss, still assembly) */

/* 2 if the current search has visited the node, 1 if no search has, else 0 */
s32 func_m155_018C4(M155Node *node) {
    if (rl_m155_04F88 == node->stamp) {
        return 2;
    }
    if (node->stamp == 0) {
        return 1;
    }
    return 0;
}

/* An entry of the path search's ring (a path, then the search's bookkeeping); size 0x84 */
typedef struct M155Entry {
    /* 0x00 */ s16 x[20];
    /* 0x28 */ s16 z[20];
    /* 0x50 */ u8 from[20];
    /* 0x64 */ u8 to[20];
    /* 0x78 */ u16 count : 8;
    /* 0x79 */ u16 search : 6; /* the search it belongs to */
    /* 0x79 */ u16 state : 2;  /* bit 0: in use; 3: done */
    /* 0x7A */ u8 pad7A[0x7C - 0x7A];
    /* 0x7C */ f32 length; /* of the path so far */
    /* 0x80 */ u32 cost;
} M155Entry;

extern u8 rl_m155_03CD0[0x1080]; /* the path search's work areas: the ring of 32 entries */
extern u8 *rl_m155_04D58;        /* the ring's next entry */
extern s32 rl_m155_04D5C;        /* the current search */
extern s32 rl_m155_04F60;        /* set when the ring is full */

/* Takes the ring's next entry for the current search, moving on past the entries the search still uses;
   NULL (and the full flag set) if there is none */
M155Entry *func_m155_01900(void) {
    M155Entry *entry;
    M155Entry *e = (M155Entry *) rl_m155_04D58;

    entry = e;
    do {
        e = (M155Entry *) (rl_m155_04D58 = (u8 *) (e + 1));
        if (!(e < (M155Entry *) &rl_m155_03CD0[0x1080])) {
            rl_m155_04D58 = rl_m155_03CD0;
            e = (M155Entry *) rl_m155_03CD0;
        }
        if (entry == e) {
            rl_m155_04F60 = 1;
            return NULL;
        }
    } while (e->search == rl_m155_04D5C && !(e->state & 1));
    entry->search = rl_m155_04D5C;
    return entry;
}

extern s32 *rl_m155_01900(void); /* 01900 by its offset, as 33 words */

#ifdef NON_MATCHING
/* 7 diffs: the copy's destination and counter take a1/v1 the other way round */
/* Copies an entry (33 words) into the next free one of the ring; that one, or NULL */
s32 *func_m155_019A4(s32 *src) {
    s32 *entry;
    s32 *dst;
    s32 *s;
    s32 n;

    entry = rl_m155_01900();
    if (entry == NULL) {
        return NULL;
    }
    dst = entry;
    s = src;
    n = 33;
    while (n--) {
        *dst++ = *s++;
    }
    return entry;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_019A4);
#endif

/* A point a search has reached, and the shortest path length to it so far (8 bytes) */
typedef struct M155Visit {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 z;
    /* 0x4 */ u32 length; /* 0: a free slot */
} M155Visit;

extern M155Visit rl_m155_04D60[64]; /* .bss (still assembly): the points the search has reached */
extern s32 rl_m155_01A00(s16 x, s16 z, f32 length);
extern f32 func_80097140(f32 x); /* sqrtf */
extern s16 rl_m155_04D54;           /* .bss (still assembly): where the search is going */
extern s16 rl_m155_04D56;

/* Notes that the search reached (x, z) after `length`: FALSE if it got there no longer before (or there is
   no room) */
s32 func_m155_01A00(s16 x, s16 z, f32 length) {
    M155Visit *v = rl_m155_04D60;
    s32 n = 64;

    while (n--) {
        if (v->length != 0 && x == v->x && z == v->z) {
            if (length < v->length) {
                v->length = length;
                return TRUE;
            }
            return FALSE;
        }
        v++;
    }
    v = rl_m155_04D60;
    n = 64;
    while (n--) {
        if (v->length == 0) {
            v->x = x;
            v->z = z;
            v->length = length;
            return TRUE;
        }
        v++;
    }
    return FALSE;
}

/* Adds the point (x, z) (crossing from region `from` to `to`) to the entry's path; it stays in use if that
   is the shortest way there so far, and its cost becomes the distance (squared) still to go */
void func_m155_01BE0(M155Entry *e, s16 x, s16 z, u8 from, u8 to) {
    s16 dx = x - e->x[e->count];
    s16 dz = z - e->z[e->count];

    e->count++;
    e->x[e->count] = x;
    e->z[e->count] = z;
    e->from[e->count] = from;
    e->to[e->count] = to;
    e->length += func_80097140(dx * dx + dz * dz);
    e->state = 1;
    if (e->count >= 2 && !rl_m155_01A00(x, z, e->length)) {
        e->state = 0;
        return;
    }
    if (x == rl_m155_04D54 && z == rl_m155_04D56) {
        e->cost = 0;
        return;
    }
    dx = x - rl_m155_04D54;
    dz = z - rl_m155_04D56;
    e->cost = dx * dx + dz * dz;
}

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01D9C);

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01F9C);

/* The current search's entry to go on from: one with no cost yet, else the cheapest done one; or NULL */
M155Entry *func_m155_02248(void) {
    M155Entry *e = (M155Entry *) rl_m155_03CD0;
    u32 best = -1;
    M155Entry *found = NULL;
    s32 n = 32;

    while (n--) {
        if (rl_m155_04D5C == e->search) {
            if (e->cost == 0 || (e->state == 3 && e->cost < best)) {
                best = e->cost;
                found = e;
            }
        }
        e++;
    }
    return found;
}

extern f32 func_80097140(f32 x); /* sqrtf */
extern M155Entry *rl_m155_02248(void);
extern s32 rl_m155_01F9C(M155Entry *e); /* still assembly: takes the search one step from e */
extern s16 rl_m155_04D54;               /* .bss (still assembly): where the search is going */
extern s16 rl_m155_04D56;

/* Searches a path from (x, z) to (toX, toZ): the entry it ends with, or NULL */
M155Entry *func_m155_022C8(s16 x, s16 z, s16 toX, s16 toZ) {
    M155Entry *e = (M155Entry *) rl_m155_01900();
    s32 dx;
    s32 dz;

    if (e == NULL) {
        return rl_m155_02248();
    }
    e->x[0] = x;
    e->z[0] = z;
    e->from[0] = 0xFF;
    e->to[0] = 0;
    dx = toX - x;
    dz = toZ - z;
    e->cost = func_80097140(dx * dx + dz * dz);
    e->state = 3;
    rl_m155_04D54 = toX;
    rl_m155_04D56 = toZ;
    while (rl_m155_01F9C(e)) {
        e = rl_m155_02248();
        if (e == NULL) {
            return NULL;
        }
        if (e->cost == 0) {
            break;
        }
    }
    return e;
}

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

/* Marks the node visited and gives its point, or (x, z) when there is no node */
void func_m155_026D8(M155Node *node, s16 x, s16 z, s16 *outX, s16 *outZ) {
    if (node != NULL) {
        node->stamp = rl_m155_04F88;
        sFlags->unk4 = node->unkC;
        *outX = node->x;
        *outZ = node->z;
    } else {
        *outX = x;
        *outZ = z;
    }
}

extern s32 rl_m155_03C70;        /* .data (still assembly): how many nodes there are */
extern M155Node *rl_m155_04D50;  /* .bss (still assembly): the nodes */
extern Object *rl_m155_04F6C;    /* .bss (still assembly): the enemy's target */
extern s32 rl_m155_018C4(M155Node *node);
extern void rl_m155_026D8(M155Node *node, s16 x, s16 z, s16 *outX, s16 *outZ);

/* Heads for the node nearest the target that 018C4 accepts (or (x, z) when there is none) */
void func_m155_02748(s16 x, s16 z, s16 *outX, s16 *outZ) {
    M155Node *node = rl_m155_04D50;
    M155Node *best = NULL;
    u32 bestDist = -1;
    s32 n = rl_m155_03C70;
    s16 tx = rl_m155_04F6C->trans.x_position;
    s16 tz = rl_m155_04F6C->trans.z_position;
    s32 dx;
    s32 dz;
    u32 dist;

    while (n-- > 0) {
        dx = tx - node->x;
        dz = tz - node->z;
        dist = dx * dx + dz * dz;
        if (dist < bestDist && rl_m155_018C4(node)) {
            bestDist = dist;
            best = node;
        }
        node++;
    }
    rl_m155_026D8(best, x, z, outX, outZ);
}

INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_0289C);

/* The node with the highest unk8 that 018C4 accepts; its point (or (x, z)) goes to outX/outZ */
M155Node *func_m155_02AEC(s16 x, s16 z, s16 *outX, s16 *outZ) {
    M155Node *node = rl_m155_04D50;
    M155Node *best = NULL;
    u32 bestValue = 0;
    s32 n = rl_m155_03C70;

    while (n-- > 0) {
        if (bestValue < node->unk8 && rl_m155_018C4(node)) {
            bestValue = node->unk8;
            best = node;
        }
        node++;
    }
    rl_m155_026D8(best, x, z, outX, outZ);
    return best;
}

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
