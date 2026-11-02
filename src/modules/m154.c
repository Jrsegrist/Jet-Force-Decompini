/*
 * Runlink module 154: the enemies' way-point graph. Init builds the graph from
 * the level's point (0x61) and link (0x60) entries and numbers its connected
 * parts; the rest finds the nearest or farthest point, picks the next point at
 * random among the open links, runs a best-first search to a target point
 * (24 search entries) and a fall-back point away from the player, and runs a
 * point's command.
 */
#include "module.h"

/* A way point (0x34 bytes, the array at D_800FEA9C) */
typedef struct M154Node {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ u8 pad0C[0x10 - 0xC];
    /* 0x10 */ s32 unk10;
    /* 0x14 */ u32 cost : 16;   /* search cost from the start point */
    /* 0x16 */ u32 id : 8;
    /* 0x17 */ u32 unk17 : 2;
    /* 0x17 */ u32 flags : 6;
    /* 0x18 */ u8 cmd;          /* command run on arrival */
    /* 0x19 */ u8 cmdArg[4];
    /* 0x1D */ u8 links[6];     /* linked points (0 = none) */
    /* 0x23 */ u8 linkInfo[6];  /* weight in the low 5 bits, 0x20 a flag */
    /* 0x29 */ u8 linkCost[6];
    /* 0x2F */ u8 blockedBy;    /* group of the enemy standing there */
    /* 0x30 */ u8 part;         /* connected part of the graph */
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 unk32;
    /* 0x33 */ u8 unk33;
} M154Node;

/* A search entry: a path from the start point */
typedef struct M154Entry {
    /* 0x00 */ f32 score;
    /* 0x04 */ u16 cost;
    /* 0x06 */ u8 path[8];
    /* 0x0E */ u8 depth;
    /* 0x0F */ u8 state; /* 0 free, 1 open, 2 done, 3 dropped */
} M154Entry;

/* The enemy record (an enemy object's unk68) */
typedef struct M154Enemy {
    /* 0x00 */ u8 count;
    /* 0x01 */ u8 pad1[0x4 - 0x1];
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 pad5[0x7 - 0x5];
    /* 0x07 */ u8 unk7;
    /* 0x08 */ u8 pad8[0x28 - 0x8];
    /* 0x28 */ u16 flags;
    /* 0x2A */ u8 pad2A[0x2C - 0x2A];
    /* 0x2C */ u16 type;
    /* 0x2E */ u8 pad2E[0x30 - 0x2E];
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 pad31;
    /* 0x32 */ u8 kind;
    /* 0x33 */ u8 unk33;
    /* 0x34 */ u8 pad34[0x3C - 0x34];
    /* 0x3C */ s16 nodes; /* current point in the low byte, the next one in the high byte */
    /* 0x3E */ s16 unk3E;
    /* 0x40 */ s16 targetX;
    /* 0x42 */ s16 targetY;
    /* 0x44 */ s16 targetZ;
    /* 0x46 */ u8 pad46[0x48 - 0x46];
    /* 0x48 */ f32 x;
    /* 0x4C */ f32 y;
    /* 0x50 */ f32 z;
    /* 0x54 */ u8 pad54[0x58 - 0x54];
    /* 0x58 */ Object *leader;
    /* 0x5C */ u8 pad5C[0x64 - 0x5C];
    /* 0x64 */ u8 group;
    /* 0x65 */ u8 pad65[0x74 - 0x65];
    /* 0x74 */ u8 unk74;
} M154Enemy;

/* A group member's record (for the command that frees them) */
typedef struct M154Member {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ Object *next;
    /* 0x2C */ u8 pad2C[0x4A - 0x2C];
    /* 0x4A */ u8 unk4A;
} M154Member;

/* An entry of D_800A38E0 (0x80 bytes) */
typedef struct M154Kind {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ u32 pad24_0 : 22;
    /* 0x24 */ u32 canOpen : 1;
    /* 0x24 */ u32 pad24_1 : 9;
    /* 0x28 */ u8 pad28[0x80 - 0x28];
} M154Kind;

/* The level entries this module reads (point 0x61, link 0x60) */
typedef struct M154Setup {
    /* 0x00 */ s16 type;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA; /* point: command; link: first point */
    /* 0x0B */ u8 unkB; /* link: second point */
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF; /* point: its number; link: direction flags (0x40, 0x80) */
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 pad13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ s8 unk15;
} M154Setup;

extern Object **func_80050F44(s32 *count);
extern f32 point_distance(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
extern f32 point_distance_squared(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2);
extern f32 func_m003_03E20(f32 x, f32 z, f32 targetX, f32 targetZ);
extern s32 func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_09A14(Object *obj);
extern Object *func_m003_053C4(void);
extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern s32 troop_slot_get(s32 type);

extern u8 D_800FE9EE;
extern s32 D_800FEA98;
extern M154Node *D_800FEA9C;
extern M154Kind *D_800A38E0;

static M154Entry sEntries[24];
static M154Entry *sCursor;
static f32 sTargetX;
static f32 sTargetZ;
static u8 sTargetNode;
static u32 sBestCost;
static M154Enemy *sEnemy;
static M154Kind *sKind;
static Object *sPlayer;

void func_m154_00000(void) {
    Object **list;
    s32 count;
    M154Node *node;
    s32 i;
    Object *obj;
    M154Enemy *e;

    list = func_80050F44(&count);
    node = D_800FEA9C;
    i = D_800FEA98;
    while (i-- > 0) {
        (node++)->blockedBy = 0;
    }
    while (count--) {
        obj = list[count];
        e = obj->unk68;
        if (e->unk30 < 6 && (e->kind == 2 || e->kind == 12)) {
            if (e->nodes & 0xFF) {
                D_800FEA9C[e->nodes & 0xFF].blockedBy = e->group;
            }
            if ((e->nodes & 0xFFFF) >> 8) {
                D_800FEA9C[(e->nodes & 0xFFFF) >> 8].blockedBy = e->group;
            }
        }
    }
}

u8 func_m154_00130(f32 x, f32 y, f32 z, s32 part) {
    M154Node *node;
    s32 i;
    f32 best;
    f32 dist;
    u8 bestIndex;

    best = 5e9f;
    bestIndex = 0;
    i = D_800FEA98 - 1;
    node = &D_800FEA9C[1];
    if (D_800FEA98 == 0) {
        return 0;
    }
    while (i--) {
        if (part == 0 || part == node->part) {
            dist = point_distance_squared(x, y, z, node->x, node->y, node->z);
            if (dist < best) {
                best = dist;
                bestIndex = node - D_800FEA9C;
            }
        }
        node++;
    }
    return bestIndex;
}

u8 func_m154_002A8(f32 x, f32 y, f32 z, s32 part) {
    M154Node *node;
    s32 i;
    f32 best;
    f32 dist;
    u8 bestIndex;

    i = D_800FEA98 - 1;
    node = &D_800FEA9C[1];
    best = 0.0f;
    bestIndex = 0;
    while (i--) {
        if (part == 0 || part == node->part) {
            dist = point_distance_squared(x, y, z, node->x, node->y, node->z);
            if (best < dist) {
                best = dist;
                bestIndex = node - D_800FEA9C;
            }
        }
        node++;
    }
    return bestIndex;
}

/* The open entry with the lowest score */
static M154Entry *func_m154_00410(void) {
    M154Entry *e;
    M154Entry *best;
    f32 bestScore;
    s32 i;

    bestScore = 5e9f;
    best = NULL;
    for (i = 0; i < 24; i++) {
        e = &sEntries[i];
        if (e->state == 0) {
            break;
        }
        if (e->state == 1 && e->cost < sBestCost && e->depth < 7 && e->score < bestScore) {
            bestScore = e->score;
            best = e;
        }
    }
    return best;
}

/* Expands an entry: adds an entry for each link that gets a point more cheaply; 1 if out of entries */
static s32 func_m154_004B0(M154Entry *entry) {
    M154Node *node;
    M154Node *next;
    s32 i;
    s32 n;
    s32 link;

    node = &D_800FEA9C[entry->path[entry->depth]];
    entry->state = 2;
    i = 0;
    while (i < 6 && (link = node->links[i]) != 0) {
        next = &D_800FEA9C[link];
        if (next->blockedBy != 0 && next->blockedBy != sEnemy->group) {
            i++;
            continue;
        }
        if (node->linkCost[i] + entry->cost < next->cost) {
            next->cost = node->linkCost[i] + entry->cost;
            n = 0;
            do {
                sCursor++;
                if (sCursor > &sEntries[23]) {
                    sCursor = sEntries;
                }
                if (n++ >= 24) {
                    return 1;
                }
            } while (sCursor->state != 0 && sCursor->state != 3);
            for (n = 0; n <= entry->depth; n++) {
                sCursor->path[n] = entry->path[n];
            }
            sCursor->path[n] = node->links[i];
            sCursor->cost = node->linkCost[i] + entry->cost;
            sCursor->depth = entry->depth + 1;
            sCursor->state = 1;
            sCursor->score = func_m003_03E20(next->x, next->z, sTargetX, sTargetZ);
            if (sTargetNode == sCursor->path[sCursor->depth]) {
                sBestCost = sCursor->cost;
            }
        }
        i++;
    }
    return 0;
}

/* The search result: 0x100 | the first step when the target was reached, else the best step */
static s32 func_m154_00720(void) {
    M154Entry *e;
    M154Entry *best;
    s32 i;
    f32 bestScore;

    bestScore = 5e9f;
    e = sEntries;
    i = 24;
    while (i--) {
        if (sTargetNode == e->path[e->depth] && e->state != 3) {
            return (u16) (e->path[1] | 0x100);
        }
        e++;
    }
    best = NULL;
    for (i = 0; i < 24; i++) {
        e = &sEntries[i];
        if (e->state == 0) {
            break;
        }
        if (e->state == 2 && e->score < bestScore) {
            bestScore = e->score;
            best = e;
        }
    }
    if (best->path[1] != 0) {
        return best->path[1];
    }
    return best->path[0];
}

static void func_m154_00810(u8 start) {
    s32 *p;
    s32 i;
    M154Node *node;

    p = (s32 *) sEntries;
    i = 96;
    while (i--) {
        *p++ = 0;
    }
    sCursor = sEntries;
    node = D_800FEA9C;
    i = D_800FEA98;
    while (i--) {
        node->cost = 0xFFFF;
        node++;
    }
    sBestCost = -1;
    D_800FEA9C[start].cost = 0;
    sCursor->path[0] = start;
    sCursor->state = 1;
    sCursor->score = func_m003_03E20(D_800FEA9C[start].x, D_800FEA9C[start].z, sTargetX, sTargetZ);
}

/* Searches from `from` to `to` */
static u16 func_m154_00920(u8 from, u8 to) {
    s32 e;
    u16 result;

    if (from == to) {
        return from;
    }
    sTargetX = D_800FEA9C[to].x;
    sTargetZ = D_800FEA9C[to].z;
    sTargetNode = to;
    func_m154_00810(from);
    e = func_m154_004B0(sCursor);
    while (TRUE) {
        if (e == 0) {
            e = (s32) func_m154_00410();
        }
        if (e == 0) {
            result = func_m154_00720();
            break;
        }
        e = func_m154_004B0((M154Entry *) e);
        if (e == 1) {
            result = func_m154_00720();
            break;
        }
    }
    return result;
}

/* Picks a point away from the player and near the leader, and returns the step towards it */
u8 func_m154_00A10(u8 from) {
    s32 i;
    u16 points[4];
    f32 dists[4];
    M154Node *node;
    Object *leader;
    f32 dist;
    s32 j;
    u16 k;
    s32 swapped;
    f32 worst;

    leader = sEnemy->leader;
    node = D_800FEA9C + D_800FEA98 - 1;
    i = 4;
    while (i--) {
        points[i] = 0;
        dists[i] = 1e9f;
    }
    i = D_800FEA98;
    while (i-- > 0) {
        if (node->id != sEnemy->unk74 && node->id != 0 && (node->flags & 2)) {
            if (point_distance(sPlayer->trans.x_position, sPlayer->trans.y_position, sPlayer->trans.z_position,
                               node->x, node->y, node->z) > 700.0f) {
                dist = point_distance(leader->trans.x_position, leader->trans.y_position, leader->trans.z_position,
                                      node->x, node->y, node->z);
                worst = 0.0f;
                j = 4;
                while (j--) {
                    if (worst < dists[j]) {
                        worst = dists[j];
                        k = j;
                    }
                }
                if (dist < dists[k]) {
                    points[k] = i;
                    dists[k] = dist;
                }
            }
        }
        node--;
    }
    do {
        swapped = FALSE;
        i = 3;
        while (i--) {
            if (dists[i] < dists[i + 1]) {
                worst = points[i];
                points[i] = points[i + 1];
                points[i + 1] = worst;
                swapped = TRUE;
                worst = dists[i];
                dists[i] = dists[i + 1];
                dists[i + 1] = worst;
            }
        }
    } while (swapped);
    i = 4;
    while (i--) {
        if (points[i] == 0) {
            break;
        }
        k = func_m154_00920(from, points[i]);
        if (k & 0x100) {
            return k;
        }
    }
    return func_m154_00920(from, points[3]);
}

/* 0 if the enemy can take the link to `n`; -1 back to `prev`, -2 none, -3 closed, else the group there */
static s32 func_m154_00D68(u8 n, u8 prev) {
    if (n == prev) {
        return -1;
    }
    if (!n) {
        return -2;
    }
    if ((D_800FEA9C[n].flags & 4) && !sKind->canOpen) {
        return -3;
    }
    return D_800FEA9C[n].blockedBy;
}

/* The next point to go to from `cur` (having come from `prev`) */
u8 func_m154_00DFC(u8 mode, u8 cur, u8 prev) {
    s32 total;
    s32 open;
    s32 i;
    u8 pick;
    u8 blocked;
    s32 r;
    u8 part;
    M154Node *node;

    total = 0;
    open = 0;
    blocked = 0;
    i = 6;
    node = &D_800FEA9C[cur];
    if (sPlayer == NULL) {
        return 0;
    }
    sEnemy->flags &= ~0x400;
    if (cur) {
        part = D_800FEA9C[cur].part;
    } else {
        part = 0;
    }
    if (sEnemy->kind == 0x13) {
        pick = func_m154_00920(cur, func_m154_002A8(sPlayer->trans.x_position, sPlayer->trans.y_position,
                                                    sPlayer->trans.z_position, part));
        if (pick == cur) {
            sEnemy->flags |= 0x400;
        }
        return pick;
    }
    if (mode == 3) {
        pick = func_m154_00920(cur, func_m154_00130(sPlayer->trans.x_position, sPlayer->trans.y_position,
                                                    sPlayer->trans.z_position, part));
        if (pick == cur) {
            sEnemy->flags |= 0x400;
        }
        return pick;
    }
    while (i--) {
        r = func_m154_00D68(node->links[i], prev);
        if (r == 0) {
            open++;
            total += node->linkInfo[i] & 0x1F;
        } else if (r >= 2) {
            blocked = 1;
        }
    }
    if (open == 0) {
        if (blocked) {
            return cur;
        }
        return prev ? prev : cur;
    }
    pick = func_80048E2C(1, total);
    total = 0;
    i = 6;
    while (i--) {
        if (func_m154_00D68(node->links[i], prev) == 0) {
            total += node->linkInfo[i] & 0x1F;
            if (total >= pick) {
                if (node->linkInfo[i] & 0x20) {
                    sEnemy->flags |= 0x20;
                } else {
                    sEnemy->flags &= ~0x20;
                }
                return node->links[i];
            }
        }
    }
    return prev;
}

/* Runs the point's command */
void func_m154_01110(s16 *arg0, M154Node *node) {
    Object *obj;
    M154Member *m;
    s32 n;
    Object *next;

    obj = sEnemy->leader;
    m = obj->unk68;
    switch (node->cmd) {
        case 2:
            sEnemy->unk4 = sEnemy->unk7 = D_800FE9EE = node->cmdArg[0];
            sEnemy->nodes = 0;
            sEnemy->unk3E = 0;
            break;
        case 3:
        case 9:
            func_m003_04260(2, 0x13, node->cmdArg[0], node->cmdArg[1], node->cmdArg[2]);
            break;
        case 4:
            if (func_m003_04260(2, 2, node->cmdArg[0], node->cmdArg[1], node->cmdArg[2]) != 0) {
                m->unk4A = node->cmdArg[3];
            }
            break;
        case 7:
            n = sEnemy->count;
            while (n--) {
                next = m->next;
                func_m003_09A14(obj);
                obj = next;
                if (next == NULL) {
                    break;
                }
                m = next->unk68;
            }
            break;
        case 8:
            sEnemy->unk33 = 0x22;
            *arg0 = node->cmdArg[1] << 8;
            sEnemy->unk3E = 0;
            break;
    }
}

/* Gives the enemy a point if it has none; 0 if there is none */
s32 func_m154_01278(void) {
    M154Node *node;

    if (!(sEnemy->nodes & 0xFF)) {
        sEnemy->nodes = func_m154_00130(sEnemy->x, sEnemy->y, sEnemy->z, 0);
        node = &D_800FEA9C[sEnemy->nodes & 0xFF];
        sEnemy->targetX = node->x;
        sEnemy->targetY = node->y;
        sEnemy->targetZ = node->z;
        if (sEnemy->nodes == 0) {
            D_800FE9EE = 3;
            return 0;
        }
    }
    return 1;
}

/* Builds the way-point graph from the level entries */
#ifdef NON_MATCHING
/*
 * t-register choice in the link and part-numbering loops (183 diffs; reusing `n` as the part-loop counter
 * and `off2` as the found flag gets 160)
 */
void func_m154_013A8(M154Setup *setup, s32 size) {
    M154Setup *s;
    M154Setup *l;
    s32 off;
    s32 off2;
    M154Node *node;
    M154Node *other;
    s32 n;
    s32 to;
    s32 i;
    s32 j;
    s32 part;
    s32 found;

    D_800FEA98 = 1;
    for (off = 0, s = setup; off < size; off += s->size, s = (M154Setup *) ((u8 *) s + s->size)) {
        if (s->type == 0x61) {
            D_800FEA98++;
        }
    }
    if (D_800FEA98 >= 2) {
        D_800FEA9C = func_8004A57C(D_800FEA98 * sizeof(M154Node), 0xFFFF00FF);
    } else {
        D_800FEA98 = 0;
        D_800FEA9C = NULL;
    }
    if (D_800FEA98 == 0) {
        return;
    }
    for (off = 0, s = setup; off < size; off += s->size, s = (M154Setup *) ((u8 *) s + s->size)) {
        if (s->type == 0x61) {
            node = &D_800FEA9C[s->unkF];
            node->x = s->x;
            node->y = s->y;
            node->z = s->z;
            node->id = s->unkF;
            node->unk17 = s->unk15;
            node->flags = s->unk10;
            node->cmd = s->unkA;
            node->cmdArg[0] = s->unkB;
            node->cmdArg[1] = s->unkC;
            node->cmdArg[2] = s->unkD;
            node->part = 0;
            node->cmdArg[3] = s->unkE;
            node->unk10 = 0;
            node->unk31 = s->unk14;
            node->unk32 = s->unk11;
            node->unk33 = s->unk12;
            n = 0;
            for (off2 = 0, l = setup; off2 < size; off2 += l->size, l = (M154Setup *) ((u8 *) l + l->size)) {
                if (l->type == 0x60 && (l->unkA == node->id || l->unkB == node->id)) {
                    if (l->unkA == node->id) {
                        if (l->unkF & 0x40) {
                            to = l->unkB;
                        } else {
                            to = -1;
                        }
                    } else {
                        to = -1;
                        if (l->unkF & 0x80) {
                            to = l->unkA;
                        }
                    }
                    if (to != -1 && n < 6) {
                        node->links[n] = to;
                        node->linkInfo[n] = (l->unkC & 0x1F) | (l->unkF & ~0x1F);
                        node->linkCost[n] = l->unkD;
                        n++;
                    }
                }
            }
            while (n < 6) {
                node->links[n] = 0;
                node->linkInfo[n] = 0;
                node->linkCost[n] = 0;
                n++;
            }
        }
    }
    part = 1;
    while (TRUE) {
        found = FALSE;
        node = &D_800FEA9C[1];
        i = D_800FEA98 - 1;
        while (i--) {
            if (node->part == 0) {
                node->part = part;
                found = TRUE;
                break;
            }
            node++;
        }
        if (!found) {
            return;
        }
        do {
            found = FALSE;
            node = &D_800FEA9C[1];
            i = D_800FEA98 - 1;
            while (i--) {
                if (part == node->part) {
                    for (j = 0; j < 6; j++) {
                        if (node->links[j] == 0) {
                            break;
                        }
                        other = &D_800FEA9C[node->links[j]];
                        if (other->part == 0) {
                            other->part = part;
                            found = TRUE;
                        }
                    }
                }
                node++;
            }
        } while (found == TRUE);
        part++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m154", func_m154_013A8);
#endif

/* Sets the enemy the functions above work for */
void func_m154_017E8(Object *obj) {
    s32 index;

    sEnemy = obj->unk68;
    index = troop_slot_get(sEnemy->type);
    sPlayer = func_m003_053C4();
    if (index != -1) {
        sKind = &D_800A38E0[index];
    } else {
        sKind = NULL;
    }
}
