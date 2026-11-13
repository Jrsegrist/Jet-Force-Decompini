/*
 * Runlink module 155: the enemy AI shared by the soldier enemies: where the
 * player was last spotted, paths between nodes, the enemies' states and
 * which way they face.
 */
#include "module.h"

/* A work buffer: the path nodes kept for one target (size 0x10) */
typedef struct M155Buffer {
    /* 0x0 */ Object *obj; /* the target, or NULL: free */
    /* 0x4 */ s16 x;       /* where it was when they were built */
    /* 0x6 */ s16 z;
    /* 0x8 */ void *data;  /* the nodes */
    /* 0xC */ u8 unkC;
    /* 0xD */ u8 unkD;
    /* 0xE */ u8 count;    /* of nodes */
    /* 0xF */ u8 padF;
} M155Buffer;

/* The current enemy's flags (its orders, allocated by 01584; size 0x20) */
typedef struct M155Flags {
    /* 0x0 */ u32 unk0 : 8;
    /* 0x1 */ u32 unk1 : 8; /* a delay: set by the troop's control when its side goes */
    /* 0x2 */ u32 unk2 : 8; /* frames it has been held up (02FD0) */
    u32 pad : 5;
    u32 alerted : 1;
    u32 go : 1; /* it needs a new path */
    u32 pad2 : 1;
    /* 0x04 */ s32 unk4;     /* the path node it last reached (as node->unkC) */
    /* 0x08 */ s16 unk8;     /* points of its path, (x, z) each */
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 nextX; /* the next point of its path */
    /* 0x16 */ s16 nextZ;
    /* 0x18 */ u32 unk18; /* the distance (squared) to its next point when last checked */
} M155Flags;

/* The current enemy's state (its object's unk68) */
typedef struct M155Enemy {
    /* 0x00 */ u32 pad0 : 5;
    /* 0x00 */ u32 unk0_4000000 : 1;
    /* 0x00 */ u32 pad0b : 14;
    /* 0x00 */ u32 unk0_800 : 1;
    /* 0x00 */ u32 pad0c : 7;
    /* 0x00 */ u32 leader : 1;
    /* 0x00 */ u32 pad0d : 3;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u8 pad6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 pad8[0x15 - 0x8];
    /* 0x15 */ u8 unk15; /* bit 0: its side of the troop */
    /* 0x16 */ u8 pad16[0x18 - 0x16];
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 pad19;
    /* 0x1A */ u8 state;
    /* 0x1B */ u8 pad1B[0x24 - 0x1B];
    /* 0x24 */ Object *home; /* its troop's home */
    /* 0x28 */ Object *next; /* the troop's next member */
    /* 0x2C */ u8 pad2C[0x38 - 0x2C];
    /* 0x38 */ f32 goalX;
    /* 0x3C */ f32 goalY;
    /* 0x40 */ f32 goalZ;
    /* 0x44 */ u8 pad44[0x64 - 0x44];
    /* 0x64 */ Object *target;
    /* 0x68 */ u8 pad68[0xB0 - 0x68];
    /* 0xB0 */ M155Flags *flags;
} M155Enemy;

extern void func_8004AA80(void *data); /* DKR mempool_free */
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
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

/* A path node */
typedef struct M155Node {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 z;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 stamp; /* the search that last visited it, or 0 */
} M155Node;

extern s32 rl_m155_03C70;       /* .data (still assembly): how many nodes there are */
extern M155Node *rl_m155_04D50; /* .bss (still assembly): the nodes */

/*
 * Adds a path node: a new one while there are fewer than 64, else it replaces the one with the
 * largest unk8. (As in the original, if every unk8 is 0 the node written is never set.) Called from
 * 00268 and 00644.
 */
static void func_m155_000C8(s16 x, s16 z, s16 arg2, s16 arg3, s32 arg4, u32 arg5) {
    M155Node *node;
    M155Node *n;
    u32 most;
    s32 i;

    i = 64;
    n = rl_m155_04D50;
    most = 0;
    if (rl_m155_03C70 < 64) {
        node = &rl_m155_04D50[rl_m155_03C70++];
    } else {
        while (i--) {
            if (most < n->unk8) {
                most = n->unk8;
                node = n;
            }
            n++;
        }
    }
    node->x = x;
    node->z = z;
    node->unk4 = arg2;
    node->unk6 = arg3;
    node->unk8 = arg5;
    node->unkC = arg4;
    node->stamp = 0;
}

/* A region of the level, as 001B4 uses it (module 3's) */
typedef struct M155Region {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ struct M155Region *next; /* the regions inside it */
    /* 0x14 */ u32 flags; /* 0x10: a hole in the one it is in */
} M155Region;

extern s32 func_m003_16F44(f32 x, f32 z, M155Region *region);

/* Whether (x, z) is in the region and in none of its holes */
static s32 func_m155_001B4(f32 x, f32 z, M155Region *region) {
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

/* A region's crossing point (module 3's) */
typedef struct M155Point {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 z;
} M155Point;

/* A region as 00FC4 walks them (module 3's) */
typedef struct M155Area {
    /* 0x00 */ M155Point *points; /* its crossing points */
    /* 0x04 */ u32 count;         /* of them */
    /* 0x08 */ u8 pad8[0x10 - 0x8];
    /* 0x10 */ struct M155Area *next;
    /* 0x14 */ u32 flags; /* 1: in use, 4: skip, 8: has its own nodes */
    /* 0x18 */ u8 pad18[0x20 - 0x18];
    /* 0x20 */ Object *obj;
    /* 0x24 */ u8 pad24[4];
    /* 0x28 */ u8 unk28; /* what module 3's 04260 gets when the enemy is inside */
    /* 0x29 */ u8 unk29;
    /* 0x2A */ u8 unk2A;
} M155Area;

extern M155Area *rl_m155_04F80; /* .bss (still assembly): the enemy's region */

/* What an object's unk5C points at, as 00268 uses it: its extent */
typedef struct M155Extent {
    /* 0x000 */ u8 pad0[0x100];
    /* 0x100 */ f32 x1;
    /* 0x104 */ u8 pad104[4];
    /* 0x108 */ f32 z1;
    /* 0x10C */ f32 x2;
    /* 0x110 */ u8 pad110[4];
    /* 0x114 */ f32 z2;
} M155Extent;

/* The target's object data, as 00268 uses it */
typedef struct M155Interaction {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 radius;
} M155Interaction;

extern f32 func_80097140(f32 x);        /* sqrtf */
extern s32 func_80049D80(s32 a, s32 b); /* the difference of two angles */
extern Object *rl_m155_04F6C;           /* .bss (still assembly): the enemy's target */
extern s32 rl_m155_04F8C;
extern s16 rl_m155_04F90;

/*
 * Adds a path node beside object `obj` (from (x, z)): at the point of its edge facing (x, z), pushed out
 * by the target's radius and 10, if that point is in the enemy's region; its other end goes to one side
 */
static void func_m155_00268(f32 x, f32 z, Object *obj, s32 arg3) {
    M155Extent *e;
    f32 cx;
    f32 cz;
    UNUSED f32 dx;
    UNUSED f32 dz;
    s16 angle;
    s16 px;
    s16 pz;
    s16 a;
    f32 len;
    f32 r;
    f32 d;
    s16 qx;
    s16 qz;
    f32 fx;
    f32 fz;
    s32 i;

    e = (M155Extent *) obj->unk5C;
    cx = (e->x2 + e->x1) * 0.5f;
    cz = (e->z2 + e->z1) * 0.5f;
    len = func_80097140((e->z1 - e->z2) * (e->z1 - e->z2) + (e->x1 - e->x2) * (e->x1 - e->x2));
    r = ((M155Interaction *) rl_m155_04F6C->interactObj)->radius + 10.0f;
    angle = func_80049C84(x - cx, z - cz) - 0x8000;
    d = len * 0.5f + r;
    px = func_80049C34(angle) * d + cx;
    pz = func_80049C30(angle) * d + cz;
    fx = px;
    if (func_m155_001B4(fx, pz, (M155Region *) rl_m155_04F80)) {
        fz = pz;
        i = 2;
        goto loop; /* FAKE (found by the permuter): keeps the loop's first test, as in the ROM */
    loop:
        while (i--) {
            a = (i ? -0x4000 : 0x4000) + angle;
            qx = func_80049C34(a) * len + fx;
            qz = func_80049C30(a) * len + fz;
            if (rl_m155_04F8C != 0) {
                a = func_80049D80(rl_m155_04F90, func_80049C84(rl_m155_04F6C->trans.z_position - fz,
                                                               rl_m155_04F6C->trans.x_position - fx) - 0x8000);
                if ((a >= 0 ? a : -a) < 0x2000) {
                    continue;
                }
            }
            func_m155_000C8(px, pz, qx, qz, (arg3 << 16) + i, (fx - x) * (fx - x) + (fz - z) * (fz - z));
            return;
        }
    }
}

extern s32 func_m003_056D4(M155Area *area);
extern s32 func_m003_03E90(s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 x4, s32 y4, s16 *x, s16 *y);
extern s32 func_m003_175E8(f32 x1, f32 z1, f32 x2, f32 z2, void *region, void *out, s32 point, u8 index);

/*
 * Adds path nodes around the corners of region `area` that (x, z) sees from outside: each such corner gets
 * a node pushed out by the target's radius and 10, joined to a point further along if the way there is
 * clear and in the enemy's region
 */
static void func_m155_00644(s16 x, s16 z, M155Area *area) {
    s32 n;
    M155Point *p;
    M155Point *q;
    M155Point *near;
    M155Point *far;
    s16 a1;
    s16 a2;
    u32 d1;
    u32 d2;
    s32 ok;
    u32 dist;
    s32 flip;
    s16 bx;
    s16 bz;
    s16 cx;
    s16 cz;
    s16 e;
    s16 b;
    UNUSED s16 h;
    f32 r;
    f32 s;
    s32 id;
    f32 len;

    id = func_m003_056D4(area);
    n = area->count;
    p = area->points;
    while (n--) {
        if (n != 0) {
            q = p + 1;
        } else {
            q = area->points;
        }
        a1 = func_80049C84(z - p->z, x - p->x) - 0x8000;
        a2 = func_80049C84(z - q->z, x - q->x) - 0x8000;
        d1 = (p->x - x) * (p->x - x) + (p->z - z) * (p->z - z);
        d2 = (q->x - x) * (q->x - x) + (q->z - z) * (q->z - z);
        if ((s16) func_80049D80(a1, a2) < -0x200) {
            if (d1 < d2) {
                dist = d1;
                near = p;
                far = q;
                flip = FALSE;
            } else {
                dist = d2;
                near = q;
                far = p;
                a1 = a2;
                flip = TRUE;
            }
            e = func_80049C84(near->z - far->z, near->x - far->x) - 0x8000;
            b = func_80049D80(e, a1) >> 1;
            r = ((M155Interaction *) rl_m155_04F6C->interactObj)->radius + 10.0f;
            s = r / func_80049C34(b);
            if (flip) {
                b += e - 0x8000;
            } else {
                b += e;
            }
            bx = func_80049C30(b) * (s + r) + near->x;
            bz = func_80049C34(b) * (s + r) + near->z;
            if (func_m155_001B4(bx, bz, (M155Region *) rl_m155_04F80)) {
                bx = func_80049C30(b) * s + near->x;
                ok = TRUE;
                bz = func_80049C34(b) * s + near->z;
                b = a1 - func_80049D80(a1, b);
                cx = func_80049C30(b) * s + near->x;
                cz = func_80049C34(b) * s + near->z;
                if (func_m003_175E8(bx, bz, cx, cz, rl_m155_04F80, NULL, -1, 0xFF) ||
                    !func_m155_001B4(cx, cz, (M155Region *) rl_m155_04F80)) {
                    ok = FALSE;
                } else {
                    if (1) { /* FAKE (found by the permuter): splits the else-if as the original */
                    }
                    if (rl_m155_04F8C != 0) {
                        a1 = func_80049D80(rl_m155_04F90, func_80049C84(rl_m155_04F6C->trans.z_position - bz,
                                                                        rl_m155_04F6C->trans.x_position - bx) - 0x8000);
                        if ((a1 >= 0 ? a1 : -a1) < 0x2000) {
                            ok = FALSE;
                        }
                    }
                }
                if (ok) {
                    func_m155_000C8(bx, bz, cx, cz, (id << 16) + n, dist);
                } else {
                    ok = TRUE;
                    if (flip) {
                        b = -0x8000 - e;
                    } else {
                        b = -e;
                    }
                    cx = func_80049C34(b) * r + near->x;
                    cz = func_80049C30(b) * r + near->z;
                    cx = bx + (cx - bx) * 2;
                    cz = bz + (cz - bz) * 2;
                    func_m003_03E90(x, z, near->x + (near->x - x) * 4, near->z + (near->z - z) * 4, bx, bz, cx, cz,
                                    &cx, &cz);
                    b = func_80049C84(cx - bx, cz - bz);
                    len = func_80097140((cx - bx) * (cx - bx) + (cz - bz) * (cz - bz));
                    cx = func_80049C34(b) * (len + r) + bx;
                    cz = func_80049C30(b) * (len + r) + bz;
                    if (func_m003_175E8(bx, bz, cx, cz, rl_m155_04F80, NULL, -1, 0xFF) ||
                        !func_m155_001B4(cx, cz, (M155Region *) rl_m155_04F80)) {
                        ok = FALSE;
                    } else if (rl_m155_04F8C != 0) {
                        a1 = func_80049D80(rl_m155_04F90, func_80049C84(rl_m155_04F6C->trans.z_position - bz,
                                                                        rl_m155_04F6C->trans.x_position - bx) - 0x8000);
                        if ((a1 >= 0 ? a1 : -a1) < 0x2000) {
                            ok = FALSE;
                        }
                    }
                    if (ok) {
                        func_m155_000C8(bx, bz, cx, cz, (id << 16) + n, dist);
                    }
                }
            }
        }
        p++;
    }
}

/* A troop home's data and a member's, as far as 00FC4, 01274 and 02BB0 use them (module 3's) */
typedef struct M155Home {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ u16 flags; /* 0x100: which side moves next */
    /* 0x2A */ u8 pad2A[2];
    /* 0x2C */ u16 kind;
    /* 0x2E */ u8 pad2E[0x37 - 0x2E];
    /* 0x37 */ u8 timer;
    /* 0x38 */ u8 order[2]; /* for each side */
    /* 0x3A */ u8 pad3A[0x58 - 0x3A];
    /* 0x58 */ Object *first;
    /* 0x5C */ u8 pad5C[0x64 - 0x5C];
    /* 0x64 */ u8 unk64;
    /* 0x65 */ u8 pad65[0x7A - 0x65];
    /* 0x7A */ s8 region;
} M155Home;
typedef struct M155MemberPath {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s32 unk4;
} M155MemberPath;
typedef struct M155Member {
    /* 0x00 */ u8 pad0[0x15];
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 pad16[0x28 - 0x16];
    /* 0x28 */ Object *next;
    /* 0x2C */ u8 pad2C[0x64 - 0x2C];
    /* 0x64 */ s32 unk64;
    /* 0x68 */ u8 pad68[0xB0 - 0x68];
    /* 0xB0 */ M155MemberPath *path;
} M155Member;

extern M155Home *rl_m155_04F78; /* .bss (still assembly): the current enemy's troop */
extern M155Area *rl_m155_04F80;
extern M155Area *func_m003_0569C(s32 id);
extern s32 func_m003_056D4(M155Area *area);
extern void *func_80050F44(s32 *count);

/*
 * Builds the path nodes from (x, z): the enemy's region and those inside it, then marks the nodes that
 * the members of troop `troop` have reached (called from 01274)
 */
void func_m155_00FC4(s16 x, s16 z, s32 troop) {
    M155Area *a;
    s32 count;
    Object **homes;
    M155Home *home;
    Object *h;
    Object *o;
    M155Member *m;
    M155Node *node;
    s32 n;

    rl_m155_03C70 = 0;
    a = rl_m155_04F80 = func_m003_0569C(rl_m155_04F78->region);
    func_m155_00644(x, z, a);
    for (a = a->next; a != NULL; a = a->next) {
        if ((a->flags & 1) && !(a->flags & 4)) {
            if (a->obj == NULL) {
                func_m155_00644(x, z, a);
            } else if (a->obj->behaviorId != 0x5B || a->obj->modelIndex < 2) {
                if (a->flags & 8) {
                    func_m155_00268(x, z, a->obj, func_m003_056D4(a));
                } else {
                    func_m155_00644(x, z, a);
                }
            }
        }
    }
    homes = func_80050F44(&count);
    while (count--) {
        h = homes[count];
        home = h->unk68;
        for (o = home->first; o != NULL; o = m->next) {
            m = o->unk68;
            if (!o->freed && troop == m->unk64) {
                n = rl_m155_03C70;
                node = rl_m155_04D50;
                while (n-- > 0) {
                    if (m->path != NULL && m->path->unk4 == node->unkC) {
                        node->stamp = (home->unk64 << 16) + m->unk15 + 1;
                        break;
                    }
                    node++;
                }
            }
        }
    }
}

/* What module 3 gives for a troop's first member */
typedef struct M155TroopInfo {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ u32 pad24 : 22;
    /* 0x24 */ u32 unk24_200 : 1;
    /* 0x24 */ u32 pad24c : 4;
    /* 0x24 */ u32 unk24_10 : 1;
    /* 0x24 */ u32 pad24b : 4;
    /* 0x28 */ u8 pad28[0x2E - 0x28];
    /* 0x2E */ u8 interval;
} M155TroopInfo;

extern M155TroopInfo *func_m003_0D2C0(Object *obj);

extern s32 rl_m155_04F8C;
extern M155Node *rl_m155_04D50;

#ifdef NON_MATCHING
/*
 * Picks the path nodes for heading at `target`: none when it is a troop leader of kind 0x18, else the
 * nodes of the work buffer kept for it (built from scratch in a free buffer the first time).
 * ROM length; 43 words: the ROM materialises &rl_m155_04D50 in v0 at each store (the int casts give the
 * materialised address, in a t-register), keeps 4 in a1 for the order tests, and loads z earlier.
 */
static void func_m155_01274(Object *target) {
    s32 n;
    M155Buffer *buf;
    s32 d;

    if (target->behaviorId == 0x18 && ((M155Enemy *) target->unk68)->leader) {
        *(M155Node **) (u32) &rl_m155_04D50 = NULL;
        rl_m155_03C70 = 0;
        return;
    }
    buf = sBuffers;
    n = 4;
    while (d = n, n--, d) {
        if (target == buf->obj) {
            *(M155Node **) (u32) &rl_m155_04D50 = buf->data;
            buf->x = target->trans.x_position;
            buf->z = target->trans.z_position;
            if (rl_m155_04F78->order[0] == 4 || rl_m155_04F78->order[1] == 4) {
                rl_m155_04F8C = 1;
            } else {
                rl_m155_04F8C = 0;
            }
            buf->unkD = rl_m155_04F8C;
            func_m155_00FC4(target->trans.x_position, target->trans.z_position, (s32) target);
            buf->count = rl_m155_03C70;
            return;
        }
        buf++;
    }
    buf = sBuffers;
    n = 4;
    while (d = n, n--, d) {
        if (buf->obj == NULL) {
            buf->obj = target;
            buf->unkC = 30;
            if (buf->data == NULL) {
                buf->data = func_8004A57C(0x500, -1);
            }
            if ((*(M155Node **) (u32) &rl_m155_04D50 = buf->data) == NULL) {
                return;
            }
            buf->x = target->trans.x_position;
            buf->z = target->trans.z_position;
            if (rl_m155_04F78->order[0] == 4 || rl_m155_04F78->order[1] == 4) {
                rl_m155_04F8C = 1;
            } else {
                rl_m155_04F8C = 0;
            }
            buf->unkD = rl_m155_04F8C;
            func_m155_00FC4(target->trans.x_position, target->trans.z_position, (s32) target);
            buf->count = rl_m155_03C70;
            return;
        }
        buf++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01274);
extern void rl_m155_01274(Object *target);
#define func_m155_01274 rl_m155_01274
#endif

extern Object *rl_m155_04F74;        /* the current enemy's troop home */
extern s32 rl_m155_04F94;
extern M155TroopInfo *rl_m155_04FD8; /* what module 3 gives for the current enemy */
extern s32 func_m003_056F4(s32 x, s32 z);
extern s32 rl_m155_04F88;
extern s16 rl_m155_04F68;
extern s16 rl_m155_04F6A;
extern M155Node *rl_m155_04F84;

/*
 * Makes obj the current enemy: its state, troop, region and flags (allocated the first time); with
 * `full`, also the path nodes towards its target and the node it last reached. FALSE if module 3 has
 * nothing for it
 */
static s32 func_m155_01584(Object *obj, s32 full) {
    s32 n = rl_m155_03C70;
    M155Node *node = rl_m155_04D50;

    rl_m155_04F78 = (rl_m155_04F74 = (sEnemy = (rl_m155_04F6C = obj)->unk68)->home)->unk68;
    rl_m155_04F88 = (rl_m155_04F78->unk64 << 16) + sEnemy->unk15 + 1;
    rl_m155_04F80 = func_m003_0569C(rl_m155_04F78->region);
    if ((rl_m155_04FD8 = func_m003_0D2C0(obj)) == NULL) {
        return FALSE;
    }
    if (sEnemy->target != NULL) {
        rl_m155_04F90 = func_80049C84(rl_m155_04F6C->trans.z_position - sEnemy->target->trans.z_position,
                                      rl_m155_04F6C->trans.x_position - sEnemy->target->trans.x_position) -
                        0x8000;
    }
    rl_m155_04F94 = 0;
    rl_m155_04F84 = NULL;
    if ((sFlags = sEnemy->flags) == NULL) {
        sFlags = func_8004A57C(0x20, 0xFFFF);
        sEnemy->flags = sFlags;
        sFlags->unk8 = sFlags->nextX = sFlags->unk10 = sFlags->unkC = rl_m155_04F6C->trans.x_position;
        sFlags->unkA = sFlags->nextZ = sFlags->unk12 = sFlags->unkE = rl_m155_04F6C->trans.z_position;
        sFlags->go = 1;
        sFlags->alerted = 0;
        sFlags->unk1 = 0;
        sFlags->unk18 = -1;
        sFlags->unk4 = 0;
    }
    if (!full) {
        return TRUE;
    }
    if (sEnemy->target == NULL) {
        rl_m155_04F84 = NULL;
        return TRUE;
    }
    func_m155_01274(sEnemy->target);
    if (func_m003_056F4(sLastX, sLastZ)) {
        rl_m155_04F68 = sLastX;
        rl_m155_04F6A = sLastZ;
    }
    while (n-- > 0) {
        if (node->unkC == sFlags->unk4) {
            rl_m155_04F84 = node;
            break;
        }
        node++;
    }
    if (rl_m155_04F84 == NULL) {
        sFlags->go = 1;
    }
    return TRUE;
}

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

#ifdef NON_MATCHING
/*
 * 122 diffs: the ROM narrows `from` into its home (sb, then lbu) as if the parameter were assigned
 * somewhere (`from += 0;` before the first call reproduces it, leaving 59: every temp one higher);
 * DKR's music_sequence_start (func_8000160C, also still asm) has the same sb.
 */
/*
 * Moves (x, z) away from crossing `from` of region `to`, along the line halfway between the two edges
 * that meet there, by the target's radius and 15 more
 */
static void func_m155_01D9C(s16 *x, s16 *z, u8 from, u8 to) {
    M155Area *zone;
    M155Point *cur;
    s32 i;
    M155Point *next;
    s16 a;
    s16 angle;
    M155Point *prev;

    zone = func_m003_0569C(to);
    cur = &zone->points[from];
    if (from != 0) {
        i = from - 1;
    } else {
        i = zone->count - 1;
    }
    prev = &zone->points[i];
    if (from >= zone->count) { /* (the original's test: from + 1 is never compared) */
        i = 0;
    } else {
        i = from + 1;
    }
    next = &zone->points[i];
    a = func_80049C84(cur->z - prev->z, cur->x - prev->x) - 0x8000;
    angle = (func_80049D80(a, (s16) (func_80049C84(cur->z - next->z, cur->x - next->x) - 0x8000)) >> 1) + a;
    *x -= func_80049C30(angle) * (((M155Interaction *) rl_m155_04F6C->interactObj)->radius + 15.0f);
    *z -= func_80049C34(angle) * (((M155Interaction *) rl_m155_04F6C->interactObj)->radius + 15.0f);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_01D9C);
#define func_m155_01D9C rl_m155_01D9C
#endif

/* Where a segment leaves a region, and where it can go on from (module 3's) */
typedef struct M155Exit {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 z;
    /* 0x4 */ u8 point;  /* the crossing it is */
    /* 0x5 */ u8 other1; /* the region's other ways on */
    /* 0x6 */ u8 other2;
    /* 0x7 */ u8 region; /* the region it leads into */
    /* 0x8 */ u8 unk8;
    /* 0x9 */ u8 pad9;
} M155Exit;

extern s32 func_m003_175E8(f32 x1, f32 z1, f32 x2, f32 z2, void *region, void *out, s32 point, u8 index);
extern M155Entry *rl_m155_019A4(M155Entry *e);
extern void rl_m155_01BE0(M155Entry *e, s16 x, s16 z, u8 from, u8 to);

/*
 * Takes the search one step from e: straight to the goal if nothing is in the way, else to where it
 * leaves the region, with copies of the entry for the region's other ways on; FALSE when the ring is
 * full or the path is too long
 */
s32 func_m155_01F9C(M155Entry *e) {
    s16 x;
    s16 z;
    M155Area *zone;
    M155Exit exit;
    M155Entry *copy;
    u8 n = e->count;
    UNUSED s32 pad[2];

    x = e->x[n];
    z = e->z[n];
    e->state &= 1;
    if (rl_m155_04F60 != 0 || n >= 20) {
        return FALSE;
    }
    if (!func_m003_175E8(x, z, rl_m155_04D54, rl_m155_04D56, rl_m155_04F80, &exit, e->from[n], e->to[n]) ||
        (exit.x == rl_m155_04D54 && exit.z == rl_m155_04D56)) {
        rl_m155_01BE0(e, rl_m155_04D54, rl_m155_04D56, 0xFF, 0);
        return TRUE;
    }
    zone = func_m003_0569C(exit.region);
    if (x != exit.x || z != exit.z) {
        rl_m155_01BE0(e, exit.x, exit.z, exit.point, exit.region);
        if (exit.unk8) {
            e->state |= 2;
        }
    }
    if (exit.point != exit.other1 && rl_m155_04F60 == 0) {
        copy = rl_m155_019A4(e);
        if (copy != NULL) {
            rl_m155_01BE0(copy, zone->points[exit.other1].x, zone->points[exit.other1].z, exit.other1,
                          exit.region);
            copy->state |= 2;
        }
    }
    if (exit.point != exit.other2 && rl_m155_04F60 == 0) {
        copy = rl_m155_019A4(e);
        if (copy != NULL) {
            rl_m155_01BE0(copy, zone->points[exit.other2].x, zone->points[exit.other2].z, exit.other2,
                          exit.region);
            copy->state |= 2;
        }
    }
    return TRUE;
}

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

extern M155Path *rl_m155_022C8(s16 x, s16 z, s16 toX, s16 toZ);
extern void rl_m155_01D9C(s16 *x, s16 *z, u8 from, u8 to);

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
        func_m155_01D9C(p - 1, p + 19, path->from[i - 1], path->to[i - 1]);
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

extern s16 rl_m155_04F68;        /* .bss (still assembly): the enemy's home point */
extern s16 rl_m155_04F6A;
extern M155Node *rl_m155_04F84;  /* .bss (still assembly): the node it is heading for, or NULL */
extern s32 func_80049D80(s32 a, s32 b); /* the difference of two angles */

#ifdef NON_MATCHING
/* 7 diffs: bestDist and limit take s6/s7 the other way round */
/*
 * The node nearest the target, short of the current node's unk8, that 018C4 accepts; it is
 * marked and its point (or (x, z)) goes to outX/outZ. (As in the original, with no current node
 * the unk8 limit is never set, and the angle test always passes.)
 */
M155Node *func_m155_0289C(s16 x, s16 z, s16 *outX, s16 *outZ) {
    UNUSED s32 pad[2];
    M155Node *best = NULL;
    M155Node *node = rl_m155_04D50;
    u32 unset;
    s32 tx;
    s32 tz;
    s32 n = rl_m155_03C70;
    u32 limit;
    u32 bestDist = -1;
    u32 dx;
    u32 dz;
    u32 dist;
    s16 a;
    s16 d;

    tx = rl_m155_04F6C->trans.x_position;
    tz = rl_m155_04F6C->trans.z_position;
    if (rl_m155_04F84 != NULL) {
        limit = rl_m155_04F84->unk8;
    } else {
        limit = unset;
    }
    while (n-- > 0) {
        if (node != rl_m155_04F84 && node->unk8 < limit) {
            dx = tx - node->x;
            dz = tz - node->z;
            dist = dx * dx + dz * dz;
            if (dist < bestDist && rl_m155_018C4(node)) {
                a = func_80049C84(tz - rl_m155_04F6A, tx - rl_m155_04F68);
                d = func_80049D80(a, (s16) func_80049C84(node->z - rl_m155_04F6A, node->x - rl_m155_04F68));
                if (d < 20000 || d > -20000) {
                    limit = node->unk8;
                    bestDist = dist;
                    best = node;
                }
            }
        }
        node++;
    }
    rl_m155_026D8(best, x, z, outX, outZ);
    return best;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m155", func_m155_0289C);
#endif

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

/* A troop's control: every member is set up in turn; then the troop's orders for its two sides */
void func_m155_02BB0(Object *obj) {
    M155Home *troop = obj->unk68;
    s32 ready = TRUE;
    M155TroopInfo *info = NULL;
    s32 side;
    Object *o;
    M155Enemy *m;
    s32 found;
    s32 none;
    u8 *a;
    u8 *b;

    if (troop->timer != 0) {
        troop->timer--;
        return;
    }
    o = troop->first;
    if (o != NULL) {
        info = func_m003_0D2C0(o);
    }
    while (o != NULL) {
        func_m155_01584(o, 0);
        if (!sFlags->alerted && sEnemy->state == 0) {
            ready = FALSE;
            break;
        }
        o = sEnemy->next;
    }
    if (sEnemy->leader) {
        if (ready) {
            sEnemy->flags->go = 1;
            sEnemy->flags->alerted = 0;
        }
        if (sEnemy->target == NULL) {
            goto stop;
        }
        if (sEnemy->target->behaviorId == 1) {
            if (!(sEnemy->unk4 & 1)) {
                goto stop;
            }
            troop->order[0] = 5;
        } else {
            troop->order[0] = 4;
        }
        return;
    }
    if (info->unk24_10) {
        if (ready) {
            sEnemy->flags->go = 1;
            sEnemy->flags->alerted = 0;
        }
        if (sEnemy->target == NULL || !(sEnemy->unk4 & 1)) {
            goto stop;
        }
        troop->order[0] = 5;
        return;
    }
    if (troop->kind == 0x8D) {
        found = FALSE;
        none = TRUE;
        for (o = troop->first; o != NULL; o = m->next) {
            m = o->unk68;
            if (m->target != NULL) {
                none = FALSE;
                if (m->target->behaviorId == 0x18) {
                    found = TRUE;
                    if (m->flags->alerted) {
                        m->flags->go = 1;
                        m->flags->alerted = 0;
                    }
                }
            }
        }
        if (found) {
            troop->order[0] = 5;
            troop->order[1] = 5;
            return;
        }
        if (none) {
            goto stop;
        }
    }
    if (ready) {
        troop->flags ^= 0x100;
        if (troop->flags & 0x100) {
            a = &troop->order[0];
            b = &troop->order[1];
            side = 0;
        } else {
            b = &troop->order[0];
            a = &troop->order[1];
            side = 1;
        }
        *a = 3;
        *b = 2;
        for (o = troop->first; o != NULL; o = m->next) {
            m = o->unk68;
            if (side == (m->unk15 & 1)) {
                m->flags->go = 1;
                m->flags->unk1 = func_80005064(func_80048E2C(10, 25));
            }
            m->flags->alerted = 0;
        }
        if (info != NULL) {
            troop->timer = info->interval;
        } else {
            troop->timer = 50;
        }
    }
    return;
stop:
    sEnemy->flags->go = 0;
    sEnemy->flags->alerted = 1;
    troop->order[0] = 8;
    troop->order[1] = 8;
}

INCLUDE_ASM("asm/nonmatchings/modules/m155/ro", func_m155_02FD0);

/* The way `obj` faces: at the enemy's target once it is alerted, else `yaw` */
s16 func_m155_03B94(Object *obj, s16 yaw) {
    Object *target;

    func_m155_01584(obj, 0);
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

INCLUDE_ASM("asm/nonmatchings/modules/m155/ro", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m155", bss);
