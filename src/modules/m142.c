/*
 * Runlink module 142: turns a shape's outline (a closed list of 2D points,
 * plus the flagged shapes linked after it) into a 2D line-partition tree:
 * every edge becomes a segment, segments are split recursively by the line
 * of the edge that best balances the two sides, and the finished tree (inner
 * nodes and leaves of segments) is copied into one compact allocation hung
 * on the shape. The table of part names is not used by the code.
 */
#include "module.h"

/* A point of a shape's outline */
typedef struct M142Point {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
} M142Point;

/* An edge segment (size 0x14) */
typedef struct M142Seg {
    /* 0x00 */ f32 x1;
    /* 0x04 */ f32 y1;
    /* 0x08 */ f32 x2;
    /* 0x0C */ f32 y2;
    /* 0x10 */ u8 edge; /* the index of the outline edge it came from */
    /* 0x11 */ u8 kind; /* from module 3, per shape */
} M142Seg;

/* A node while building (size 0x14) */
typedef struct M142Node {
    /* 0x00 */ struct M142Node *front;
    /* 0x04 */ struct M142Node *back; /* NULL: a leaf */
    /* 0x08 */ f32 slope;              /* the splitting line y = slope * x + intercept */
    /* 0x0C */ f32 intercept;
    /* 0x10 */ u16 first;              /* its segments in sSegs */
    /* 0x12 */ u16 count;
} M142Node;

/* A candidate splitting line (size 0x14) */
typedef struct M142Split {
    /* 0x00 */ f32 slope;
    /* 0x04 */ f32 intercept;
    /* 0x08 */ s32 seg;     /* the segment it runs along */
    /* 0x0C */ s16 behind;  /* segments on each side, and cut by it */
    /* 0x0E */ s16 front;
    /* 0x10 */ u16 cut;
} M142Split;

/* A node of the finished tree (size 0x14) */
typedef struct M142TreeNode {
    /* 0x00 */ union {
        struct M142TreeNode *front;
        struct {
            s16 first; /* in the shape's segs */
            u16 count;
        } leaf;
    } u;
    /* 0x04 */ struct M142TreeNode *back;
    /* 0x08 */ f32 slope;
    /* 0x0C */ f32 intercept;
    /* 0x10 */ u8 isLeaf;
} M142TreeNode;

/* The shape the tree is built for */
typedef struct M142Shape {
    /* 0x00 */ M142Point *points;
    /* 0x04 */ s32 pointCount;
    /* 0x08 */ u8 pad8[0x10 - 0x8];
    /* 0x10 */ struct M142Shape *next;
    /* 0x14 */ u32 flags;
    /* 0x18 */ M142TreeNode *tree;
    /* 0x1C */ M142Seg *segs; /* the leaves' segments, after the nodes */
} M142Shape;

#define MAX_SEGS 0xC00
#define MAX_NODES 0x200
#define MAX_SPLITS 0x200

extern void runlink_unload_timer_set(s32, s32, s32);
extern void *func_8004A57C(s32 size, s32 colour);                /* DKR mempool_alloc_safe */
extern void *func_8004A828(s32 size, void *address, s32 colour); /* DKR mempool_alloc_fixed */
extern void func_8004AA80(void *data);                           /* DKR mempool_free */
extern s32 mempool_free_timer_get(void);
extern void func_8004AA08(s32 state);
extern void func_80098360(void *p, s32 size); /* clears memory */
extern s32 func_80049C84(f32 dx, f32 dz);     /* the angle of a vector */
extern s32 func_80049D80(s32 from, s32 to);   /* the signed difference of two angles */
extern s32 func_m003_056D4(M142Shape *shape);

/*
 * The .rodata (the part-name strings, then two error messages no code uses)
 * stays in asm; the table points into it.
 */
extern char rl_m142_014A0[];

static char *sPartNames[] = {
    rl_m142_014A0 + 0x00, /* RIGHTLEG */
    rl_m142_014A0 + 0x0C, /* LEFTLEG */
    rl_m142_014A0 + 0x14, /* BODY */
    rl_m142_014A0 + 0x1C, /* TORSO */
    rl_m142_014A0 + 0x24, /* LEFTARM */
    rl_m142_014A0 + 0x2C, /* RIGHTARM */
    rl_m142_014A0 + 0x38, /* HEAD */
    rl_m142_014A0 + 0x40, /* GUN_TL */
    rl_m142_014A0 + 0x48, /* GUN_TR */
    rl_m142_014A0 + 0x50, /* GUN_BL */
    rl_m142_014A0 + 0x58, /* GUN_BR */
    rl_m142_014A0 + 0x60, /* FLESH */
    rl_m142_014A0 + 0x68, /* OTHER */
    rl_m142_014A0 + 0x70, /* HEAD */
    rl_m142_014A0 + 0x78, /* MG_AMMO */
    rl_m142_014A0 + 0x80, /* FT_AMMO */
};

s32 D_m142_01490 = 142; /* the module's number */

static M142Seg *sSegs;
static M142Node *sNodes;
static M142Split *sSplits;
static s32 sSegCount;
static s32 sNodeCount;
static s32 sSplitCount;
static s32 sBest;   /* the chosen split, then the segment it runs along */
static s32 sLineX1; /* that segment, in whole units */
static s32 sLineY1;
static s32 sLineX2;
static s32 sLineY2;
static f32 sSlope; /* the current splitting line */
static f32 sIntercept;

/* 1 if the node's segments already form a convex chain (no further split) */
static s32 func_m142_00000(M142Node *node) {
    M142Seg *seg;
    M142Seg *next;
    s32 n;
    s16 angle;
    s16 nextAngle;
    s16 gapAngle;

    seg = &sSegs[node->first];
    n = node->count;
    if (n == 1) {
        return 1;
    }
    while (n--) {
        angle = func_80049C84(seg->y2 - seg->y1, seg->x2 - seg->x1);
        if (n != 0) {
            next = seg + 1;
        } else {
            next = &sSegs[node->first];
        }
        nextAngle = func_80049C84(next->y2 - next->y1, next->x2 - next->x1);
        if (seg->x2 != next->x1 || seg->y2 != next->y1) {
            gapAngle = func_80049C84(next->y1 - seg->y2, next->x1 - seg->x2);
            if (func_80049D80(angle, gapAngle) < 0 || func_80049D80(gapAngle, nextAngle) < 0) {
                return 0;
            }
        } else if (func_80049D80(angle, nextAngle) < 0) {
            return 0;
        }
        seg = next;
    }
    return 1;
}

/* Adds a segment (none if it has no length) */
static void func_m142_001BC(f32 x1, f32 y1, f32 x2, f32 y2, u8 edge, u8 kind) {
    if (x1 == x2 && y1 == y2) {
        return;
    }
    if (sSegCount >= MAX_SEGS) {
        /* (an error message in .rodata) */
        for (;;) {}
    }
    sSegs[sSegCount].x1 = x1;
    sSegs[sSegCount].y1 = y1;
    sSegs[sSegCount].x2 = x2;
    sSegs[sSegCount].y2 = y2;
    sSegs[sSegCount].edge = edge;
    sSegs[sSegCount].kind = kind;
    sSegCount++;
}

static s32 func_m142_002C0(s32 a, s32 b) {
    return (a > b) ? a : b;
}

static s32 func_m142_002DC(s32 x) {
    return (x >= 0) ? x : -x;
}

/* 1 if the point lies on the splitting segment's line (within rounding) */
static s32 func_m142_002F4(s32 x, s32 y) {
    s32 limit;
    s32 dx;
    s32 dist;
    s32 dy;

    dist = func_m142_002DC((sLineY2 - sLineY1) * (x - sLineX1) - (y - sLineY1) * (sLineX2 - sLineX1));
    dx = func_m142_002DC(sLineX2 - sLineX1);
    dy = func_m142_002DC(sLineY2 - sLineY1);
    limit = func_m142_002C0(dx, dy);
    return !(dist >= limit);
}

/* Where a segment's line crosses the splitting line */
static void func_m142_003B0(f32 x1, f32 y1, f32 x2, f32 y2, f32 *outX, f32 *outY) {
    f32 slope;
    f32 intercept;

    if (x1 == x2) {
        *outX = 0.0f;
        *outY = 0.0f;
        return;
    }
    slope = (y2 - y1) / (x2 - x1);
    intercept = y1 - slope * x1;
    *outX = (sIntercept - intercept) / (slope - sSlope);
    *outY = *outX * slope + intercept;
}

/* Chooses the node's splitting line: the line of the segment that best balances the sides */
static void func_m142_00444(M142Node *node) {
    M142Seg *seg;
    M142Seg *other;
    s32 i;
    s32 j;
    s32 behind;
    s32 front;
    s32 cut;
    s32 side1;
    s32 side2;
    s32 diff;
    s32 best;

    seg = &sSegs[node->first];
    i = node->count;
    best = 0x7FFFFFFF;
    sSplitCount = 0;
    while (i--) {
        if (seg->x1 == seg->x2) {
            seg++;
            continue;
        }
        {
            sSlope = (seg->y2 - seg->y1) / (seg->x2 - seg->x1);
            sIntercept = seg->y1 - sSlope * seg->x1;
            j = node->count;
            other = &sSegs[node->first];
            behind = 0;
            front = 0;
            cut = 0;
            while (j--) {
                if (i != j) {
                    side1 = other->y1 < sSlope * other->x1 + sIntercept;
                    side2 = other->y2 < sSlope * other->x2 + sIntercept;
                    if (seg->x1 == other->x2 && seg->y1 == other->y2) {
                        side2 = side1;
                    }
                    if (seg->x2 == other->x1 && seg->y2 == other->y1) {
                        side1 = side2;
                    }
                    if (side1 != side2) {
                        cut++;
                    } else if (side1) {
                        front++;
                    } else {
                        behind++;
                    }
                }
                other++;
            }
            diff = front - behind;
            if (diff < 0) {
                diff = -diff;
            }
            if (diff < best) {
                sSplitCount = 0;
                best = diff;
            }
            if (diff <= best) {
                sSplits[sSplitCount].slope = sSlope;
                sSplits[sSplitCount].intercept = sIntercept;
                sSplits[sSplitCount].seg = seg - sSegs;
                sSplits[sSplitCount].behind = behind;
                sSplits[sSplitCount].front = front;
                sSplits[sSplitCount].cut = cut;
                if (sSplitCount < 0x20) {
                    sSplitCount++;
                }
            }
        }
        seg++;
    }
    best = 0x0FFFFFFF;
    i = sSplitCount;
    while (i--) {
        if (sSplits[i].cut < best) {
            sBest = i;
        }
    }
    sSlope = sSplits[sBest].slope;
    node->slope = sSlope;
    sIntercept = sSplits[sBest].intercept;
    node->intercept = sIntercept;
    sBest = sSplits[sBest].seg;
    sLineX1 = sSegs[sBest].x1;
    sLineY1 = sSegs[sBest].y1;
    sLineX2 = sSegs[sBest].x2;
    sLineY2 = sSegs[sBest].y2;
}

/* Fills `out` with the parts of the node's segments on one side of the splitting line */
static void func_m142_00930(M142Node *node, M142Node *out, s32 side) {
    M142Seg *seg;
    s32 n;
    s32 side1;
    s32 side2;
    f32 x;
    f32 y;
    s16 angle;

    seg = &sSegs[node->first];
    n = node->count;
    out->first = sSegCount;
    while (n--) {
        side1 = seg->y1 < sSlope * seg->x1 + sIntercept;
        side2 = seg->y2 < sSlope * seg->x2 + sIntercept;
        if (func_m142_002F4(seg->x1, seg->y1)) {
            side1 = side2;
        }
        if (func_m142_002F4(seg->x2, seg->y2)) {
            side2 = side1;
        }
        if (sBest == seg - sSegs) {
            angle = -func_80049C84(seg->y2 - seg->y1, seg->x2 - seg->x1);
            x = func_80049C34(angle) * 100.0f + seg->x1;
            y = func_80049C30(angle) * 100.0f + seg->y1;
            if (side == (y < sSlope * x + sIntercept)) {
                func_m142_001BC(seg->x1, seg->y1, seg->x2, seg->y2, seg->edge, seg->kind);
            }
        } else if (side1 != side2) {
            func_m142_003B0(seg->x1, seg->y1, seg->x2, seg->y2, &x, &y);
            if (side1 == side) {
                func_m142_001BC(seg->x1, seg->y1, x, y, seg->edge, seg->kind);
            } else {
                func_m142_001BC(x, y, seg->x2, seg->y2, seg->edge, seg->kind);
            }
        } else if (side1 == side) {
            func_m142_001BC(seg->x1, seg->y1, seg->x2, seg->y2, seg->edge, seg->kind);
        }
        seg++;
    }
    out->count = sSegCount - out->first;
}

/* Splits the node until its segments are convex; `prev` is the node it was split from */
static void func_m142_00CFC(M142Node *prev, M142Node *node) {
    if (func_m142_00000(node)) {
        return;
    }
    func_m142_00444(node);
    if (prev != NULL && prev->slope == node->slope && prev->intercept == node->intercept) {
        return;
    }
    node->front = &sNodes[sNodeCount];
    sNodeCount++;
    func_m142_00930(node, node->front, 1);
    node->back = &sNodes[sNodeCount];
    sNodeCount++;
    func_m142_00930(node, node->back, 0);
    func_m142_00CFC(node, node->front);
    func_m142_00CFC(node, node->back);
}

/* Builds the shape's tree: from its outline, and if `all`, from the flagged shapes after it */
void func_m142_00E40(M142Shape *shape, s32 all) {
    M142Shape *sub;
    M142Point *p;
    M142Point *next;
    M142Point *first;
    M142Point *last;
    s32 n;
    M142Node *root;
    s32 kind;
    M142Node *src;
    M142TreeNode *out;
    M142Seg *dst;
    M142Seg *seg;
    s32 i;
    s32 j;
    s32 state;

    n = shape->pointCount;
    p = shape->points;
    last = p + n - 1;
    sub = shape;
    first = p;
    kind = func_m003_056D4(sub);
    runlink_unload_timer_set(D_m142_01490, 2, 2);
    sSegs = func_8004A57C(MAX_SEGS * sizeof(M142Seg), -1);
    sNodes = func_8004A57C(MAX_NODES * sizeof(M142Node), -1);
    sSplits = func_8004A57C(MAX_SPLITS * sizeof(M142Split), -1);
    func_80098360(sSegs, MAX_SEGS * sizeof(M142Seg));
    func_80098360(sNodes, MAX_NODES * sizeof(M142Node));
    func_80098360(sSplits, MAX_SPLITS * sizeof(M142Split));
    sSegCount = 0;
    sNodeCount = 1;
    root = sNodes;
    while (n-- > 0) {
        if (p < last) {
            next = p + 1;
        } else {
            next = first;
        }
        func_m142_001BC(p->x, p->y, next->x, next->y, p - first, kind);
        p = next;
    }
    root->count = sSegCount;
    if (all != 0) {
        for (sub = sub->next; sub != NULL; sub = sub->next) {
            if ((sub->flags & 1) && !(shape->flags & 0x12)) {
                kind = func_m003_056D4(sub);
                n = sub->pointCount;
                p = sub->points;
                last = p + n - 1;
                first = p;
                while (n-- > 0) {
                    if (p < last) {
                        next = p + 1;
                    } else {
                        next = first;
                    }
                    func_m142_001BC(p->x, p->y, next->x, next->y, p - first, kind);
                    p = next;
                }
                root->count = sSegCount;
            }
        }
    }
    func_m142_00CFC(NULL, root);
    shape->tree = func_8004A57C(sNodeCount * sizeof(M142TreeNode) + sSegCount * sizeof(M142Seg), -1);
    shape->segs = (M142Seg *) ((u8 *) shape->tree + sNodeCount * sizeof(M142TreeNode));
    src = sNodes;
    out = shape->tree;
    dst = shape->segs;
    i = sNodeCount;
    while (i--) {
        if (src->back != NULL) {
            out->u.front = (src->front - sNodes) + shape->tree;
            out->back = (src->back - sNodes) + shape->tree;
            out->slope = src->slope;
            out->intercept = src->intercept;
            out->isLeaf = 0;
        } else {
            out->u.leaf.first = dst - shape->segs;
            out->u.leaf.count = src->count;
            out->isLeaf = 1;
            seg = &sSegs[src->first];
            j = src->count;
            while (j--) {
                dst->x1 = seg->x1;
                dst->y1 = seg->y1;
                dst->x2 = seg->x2;
                dst->y2 = seg->y2;
                dst->edge = seg->edge;
                dst->kind = seg->kind;
                dst++;
                seg++;
            }
        }
        src++;
        out++;
    }
    i = dst - shape->segs;
    state = mempool_free_timer_get();
    func_8004AA08(0);
    func_8004AA80(shape->tree);
    func_8004AA08(state);
    func_8004A828(sNodeCount * sizeof(M142TreeNode) + i * sizeof(M142Seg), shape->tree, -1);
    state = mempool_free_timer_get();
    func_8004AA08(0);
    func_8004AA80(sSegs);
    func_8004AA80(sNodes);
    func_8004AA80(sSplits);
    func_8004AA08(state);
}

void func_m142_01428(M142Shape *shape) {
    func_m142_00E40(shape, 1);
}

INCLUDE_ASM("asm/nonmatchings/modules/m142", rodata);
