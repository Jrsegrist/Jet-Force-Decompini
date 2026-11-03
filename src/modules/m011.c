/*
 * Runlink module 11: a camera that follows curved paths. The level's path
 * points (objects with three links each) are collected into up to 8 lists of
 * 31 nodes; the module finds the node nearest the player, interpolates the
 * curve through the nodes and places the player and the camera along it.
 */
#include "module.h"

struct M011Node;

/* A node's place in one path: the path, and its neighbours there */
typedef struct M011Link {
    /* 0x0 */ u8 path; /* 0xFF: none */
    /* 0x1 */ u8 unk1;
    /* 0x2 */ u8 unk2;
    /* 0x3 */ u8 pad3;
    /* 0x4 */ struct M011Node *next;
    /* 0x8 */ struct M011Node *prev;
} M011Link;

/* A path node (a level object's behaviour data) */
typedef struct M011Node {
    /* 0x00 */ u8 path;
    /* 0x01 */ u8 index;
    /* 0x02 */ s16 radius;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
    /* 0x14 */ f32 camX;
    /* 0x18 */ f32 camY;
    /* 0x1C */ f32 camZ;
    /* 0x20 */ f32 angleX;
    /* 0x24 */ f32 angleY;
    /* 0x28 */ f32 angleZ;
    /* 0x2C */ M011Link links[3];
} M011Node;

/* A camera's place on a path */
typedef struct M011Cam {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ s16 path; /* 0xFF: none */
    /* 0x04 */ M011Node *node;
    /* 0x08 */ f32 t;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 radiusSq;
    /* 0x1C */ f32 weight;
} M011Cam;

/* What the camera follows */
typedef struct M011Target {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
} M011Target;

/* A point on a path */
typedef struct M011Pos {
    /* 0x00 */ M011Node *node;
    /* 0x04 */ f32 t;
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ f32 z;
    /* 0x14 */ f32 dist; /* squared, to the point searched for */
} M011Pos;

/* A span of a path being searched */
typedef struct M011Span {
    /* 0x00 */ M011Pos a;
    /* 0x18 */ M011Pos b;
    /* 0x30 */ f32 step;
    /* 0x34 */ s32 path;
} M011Span;

extern f32 func_80097140(f32 x);  /* sqrtf */
extern s32 func_80049C84(f32 a, f32 b); /* the angle of (a, b) */
extern void transform_offset_world_pos(M011Target *target, f32 *x, f32 *y, f32 *z);

extern void *func_8004A57C(s32 size, s32 colour); /* DKR mempool_alloc_safe */
extern void func_8004AA80(void *data);            /* DKR mempool_free */

/* interpolates a curve through four values */
extern f32 func_80049798(f32 a, f32 b, f32 c, f32 d, f32 t);

/* The node lists, 8 of 31 (bss, still assembly) */
extern void *rl_m011_01C70[8][31];
#define sNodeLists rl_m011_01C70

void func_m011_00020(void);
void func_m011_01510(M011Node *node, s32 path, f32 t, f32 *x, f32 *y, f32 *z);

void func_m011_00000(void) {
    func_m011_00020();
}

/* Clears the node lists */
void func_m011_00020(void) {
    s32 i;
    s32 j;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 31; j++) {
            sNodeLists[i][j] = NULL;
        }
    }
}

/* The first node of list `list`'s entry `index`, or NULL */
void *func_m011_0007C(s32 list, s32 index) {
    if (list < 0 || list >= 8) {
        return NULL;
    }
    if (index < 0 || index >= 31) {
        return NULL;
    }
    return sNodeLists[list][index];
}

#ifdef NON_MATCHING
static Object *func_m011_01BD0(s32 path, s32 *index, s32 count, Object **objs, M011Node **node);
#else
extern Object *rl_m011_01BD0(s32 path, s32 *index, s32 count, Object **objs, M011Node **node);
#define func_m011_01BD0 rl_m011_01BD0
#endif

/* Collects the nodes of path `path` from the level's objects and links them */
void func_m011_000D4(s32 path) {
    s32 i;
    s32 j;
    s32 index;
    s32 first;
    s32 count;
    s32 maxIndex;
    M011Node *n;
    Object **objs;
    M011Node **nodes;
    M011Node *node;

    objs = func_80005E98(&first, &count);
    nodes = func_8004A57C(0x400, 0x7F7F7FFF);
    i = 0;
    while (i < 256) {
        nodes[i++] = NULL;
    }
    maxIndex = -1;
    index = first;
    while (func_m011_01BD0(path, &index, count, objs, &node) != NULL) {
        if (node->index != 0xFF && nodes[node->index] == NULL) {
            u8 *entry = (u8 *) objs[index - 1]->level_entry;

            node->links[0].next = (M011Node *) (s32) entry[0x10];
            node->links[0].prev = (M011Node *) (s32) entry[0x11];
            node->links[1].next = (M011Node *) (s32) entry[0x13];
            node->links[1].prev = (M011Node *) (s32) entry[0x14];
            node->links[2].next = (M011Node *) (s32) entry[0x16];
            node->links[2].prev = (M011Node *) (s32) entry[0x17];
            nodes[node->index] = node;
            if (maxIndex < node->index) {
                maxIndex = node->index;
            }
        } else {
            node->links[0].path = 0xFF;
            node->links[1].path = 0xFF;
            node->links[2].path = 0xFF;
        }
    }
    for (i = 0; i < 31;) {
        sNodeLists[path][i++] = NULL;
    }
    if (maxIndex >= 0) {
        for (i = 0; i <= maxIndex; i++) {
            n = nodes[i];
            if (n != NULL) {
                for (j = 0; j < 3; j++) {
                    if (n->links[j].path != 0xFF) {
                        n->links[j].next = nodes[(s32) n->links[j].next];
                        if (n->links[j].next == NULL) {
                            n->links[j].next = n;
                        } else if (n->links[j].next->links[0].path != n->links[j].path &&
                                   n->links[j].next->links[1].path != n->links[j].path &&
                                   n->links[j].next->links[2].path != n->links[j].path) {
                            n->links[j].next = n;
                        }
                        n->links[j].prev = nodes[(s32) n->links[j].prev];
                        if (n->links[j].prev == NULL) {
                            n->links[j].prev = n;
                        } else if (n->links[j].prev->links[0].path != n->links[j].path &&
                                   n->links[j].prev->links[1].path != n->links[j].path &&
                                   n->links[j].prev->links[2].path != n->links[j].path) {
                            n->links[j].prev = n;
                        }
                        if (sNodeLists[path][n->links[j].path] == NULL) {
                            sNodeLists[path][n->links[j].path] = n;
                        }
                        if (n->links[j].unk1 != 0 && n->links[j].next != NULL) {
                            sNodeLists[path][n->links[j].path] = n->links[j].next;
                        }
                    }
                }
            }
        }
    }
    func_8004AA80(nodes);
}

/*
 * Finds the point of the paths nearest (x, y, z): the nearest node, then 16 steps along each
 * of its paths both ways. Returns the path, and the node and position along it.
 */
s32 func_m011_00454(s32 list, f32 x, f32 y, f32 z, M011Node **outNode, f32 *outT) {
    s32 i;
    s32 count;
    s32 k;
    s32 path;
    s32 p;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 bestDist;
    f32 d;
    M011Node *from;
    M011Node *to;
    M011Node *node;
    M011Node *nearest;
    Object **objs;
    Object *obj;

    path = 0xFF;
    if (func_m011_0007C(list, 0) == NULL) {
        *outNode = NULL;
        *outT = 0.0f;
    } else {
        nearest = NULL;
        bestDist = 0.0f;
        objs = func_80005E98(&i, &count);
        for (; i < count; i++) {
            obj = objs[i];
            if (obj->behaviorId == 0x12) {
                node = obj->unk68;
                dx = x - node->x;
                dy = y - node->y;
                dz = z - node->z;
                if (nearest == NULL || dx * dx + dy * dy + dz * dz < bestDist) {
                    nearest = node;
                    bestDist = dx * dx + dy * dy + dz * dz;
                }
            }
        }
        bestDist = 0.0f;
        *outT = -1.0f;
        for (k = 0; k < 6; k++) {
            p = nearest->links[k >> 1].path;
            if (k & 1) {
                from = nearest->links[k >> 1].next;
                to = nearest;
            } else {
                from = nearest;
                to = nearest->links[k >> 1].prev;
            }
            if (p != 0xFF && from != NULL && from != to) {
                for (i = 0; i < 16; i++) {
                    func_m011_01510(from, p, i / (f32) 16, &dx, &dy, &dz);
                    dx -= x;
                    dy -= y;
                    dz -= z;
                    d = dx * dx + dy * dy + dz * dz;
                    if (*outT < 0.0f || d < bestDist) {
                        *outNode = from;
                        bestDist = d;
                        path = p;
                        *outT = i / (f32) 16;
                    }
                }
            }
        }
    }
    return path;
}

/* The node after `node` on path `path`, or NULL */
static M011Node *func_m011_007B0(M011Node *node, s32 path) {
    if (path == node->links[0].path) {
        return node->links[0].next;
    }
    if (path == node->links[1].path) {
        return node->links[1].next;
    }
    if (path == node->links[2].path) {
        return node->links[2].next;
    }
    return NULL;
}

/* The node before `node` on path `path`, or NULL */
static M011Node *func_m011_0080C(M011Node *node, s32 path) {
    if (path == node->links[0].path) {
        return node->links[0].prev;
    }
    if (path == node->links[1].path) {
        return node->links[1].prev;
    }
    if (path == node->links[2].path) {
        return node->links[2].prev;
    }
    return NULL;
}

/* Link flag 1 of `node` on path `path` */
static u8 func_m011_00868(M011Node *node, s32 path) {
    if (path == node->links[0].path) {
        return node->links[0].unk1;
    }
    if (path == node->links[1].path) {
        return node->links[1].unk1;
    }
    if (path == node->links[2].path) {
        return node->links[2].unk1;
    }
    return 0;
}

/* Link flag 2 of `node` on path `path` */
static u8 func_m011_008C4(M011Node *node, s32 path) {
    if (path == node->links[0].path) {
        return node->links[0].unk2;
    }
    if (path == node->links[1].path) {
        return node->links[1].unk2;
    }
    if (path == node->links[2].path) {
        return node->links[2].unk2;
    }
    return 0;
}

/*
 * The node (of `node` and the one before it on path `path`) whose radius holds (x, y, z) and
 * that is nearest; *outT gets the distance as a fraction of the radius.
 */
static M011Node *func_m011_00920(M011Node *node, s32 path, f32 x, f32 y, f32 z, f32 *outT) {
    s32 pad[4];
    s32 i;
    M011Node *best;
    f32 bestDist;
    f32 radius;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 r;

    bestDist = -1.0f;
    i = 0;
    best = NULL;
    if (node != NULL) {
        do {
            if (node->radius != 0) {
                r = node->radius;
                dx = x - node->x;
                dy = y - node->y;
                dz = z - node->z;
                d = dx * dx + dy * dy + dz * dz;
                if (d < r * r && (best == NULL || d < bestDist)) {
                    best = node;
                    bestDist = d;
                    radius = r;
                }
            }
            node = func_m011_0080C(node, path);
            i++;
        } while (node != NULL && i != 2);
    }
    if (best != NULL) {
        *outT = func_80097140(bestDist) / radius;
    }
    return best;
}

/* Moves `step` along path `path`, going on to the next or previous node past either end */
static void func_m011_00A84(M011Node **nodeP, s32 path, f32 *t, f32 step) {
    M011Node *next;
    M011Node *node;
    M011Node *prev;

    *t += step;
    node = *nodeP;
    if (*t > 1.0f) {
        prev = func_m011_0080C(node, path);
        next = func_m011_0080C(prev, path);
        if (prev == next || func_m011_008C4(prev, path)) {
            *t = 1.0f;
        } else {
            *t -= 1.0f;
            *nodeP = prev;
        }
    } else if (*t < 0.0f) {
        next = func_m011_007B0(node, path);
        if (next == node || func_m011_00868(node, path)) {
            *t = 0.0f;
        } else {
            *t += 1.0f;
            *nodeP = next;
        }
    }
}

/* The squared distance from (x, y, z) to the point at `t` on path `path` after `node` (stored in out*) */
static f32 func_m011_00BB8(M011Node *node, s32 path, f32 t, f32 x, f32 y, f32 z, f32 *outX, f32 *outY,
                           f32 *outZ) {
    f32 dx;
    f32 dy;
    f32 dz;

    func_m011_01510(node, path, t, outX, outY, outZ);
    dx = *outX - x;
    dy = *outY - y;
    dz = *outZ - z;
    return dx * dx + dy * dy + dz * dz;
}

/* Narrows a span of a path down to the point nearest (x, y, z) by halving steps */
static void func_m011_00C34(M011Span *span, f32 x, f32 y, f32 z) {
    f32 t;
    f32 d;
    f32 px;
    f32 py;
    f32 pz;
    M011Node *node;

    span->a.dist = func_m011_00BB8(span->a.node, span->path, span->a.t, x, y, z, &span->a.x, &span->a.y,
                                   &span->a.z);
    span->b.dist = func_m011_00BB8(span->b.node, span->path, span->b.t, x, y, z, &span->b.x, &span->b.y,
                                   &span->b.z);
    while (span->step > 0.0025f) {
        span->step *= 0.5f;
        node = span->a.node;
        t = span->a.t;
        func_m011_00A84(&node, span->path, &t, span->step);
        d = func_m011_00BB8(node, span->path, t, x, y, z, &px, &py, &pz);
        if (span->a.dist < span->b.dist) {
            span->b.node = node;
            span->b.t = t;
            span->b.x = px;
            span->b.y = py;
            span->b.z = pz;
            span->b.dist = d;
        } else {
            span->a.node = node;
            span->a.t = t;
            span->a.x = px;
            span->a.y = py;
            span->a.z = pz;
            span->a.dist = d;
        }
    }
}

/*
 * Moves the path cameras to the points of the paths nearest (x, y, z): the camera's own path
 * and the other paths through the node it is at, nearest first.
 */
void func_m011_00E74(M011Cam *cam, M011Cam *cam2, f32 x, f32 y, f32 z) {
    s32 i;
    s32 j;
    s32 count;
    M011Node *node;
    M011Span *tmp;
    M011Span spans[3];
    M011Span *sorted[3];
    M011Pos *pos;
    f32 d0;
    f32 d1;

    spans[0].a.node = cam->node;
    spans[0].a.t = cam->t;
    func_m011_00A84(&spans[0].a.node, cam->path, &spans[0].a.t, -0.08f);
    spans[0].b.node = spans[0].a.node;
    spans[0].b.t = spans[0].a.t;
    func_m011_00A84(&spans[0].b.node, cam->path, &spans[0].b.t, 0.16f);
    spans[0].step = 0.16f;
    spans[0].path = cam->path;
    func_m011_00C34(&spans[0], x, y, z);
    sorted[0] = &spans[0];
    count = 1;
    node = func_m011_00920(cam->node, cam->path, cam->x, cam->y, cam->z, &cam->weight);
    if (node != NULL) {
        sorted[1] = &spans[1];
        sorted[2] = &spans[2];
        for (i = 0; i < 3; i++) {
            if (node->links[i].path != 0xFF && cam->path != node->links[i].path) {
                spans[count].a.node = node;
                spans[count].a.t = 0.0f;
                func_m011_00A84(&spans[count].a.node, node->links[i].path, &spans[count].a.t, -0.75f);
                spans[count].b.node = node;
                spans[count].b.t = 0.0f;
                func_m011_00A84(&spans[count].b.node, node->links[i].path, &spans[count].b.t, 0.75f);
                spans[count].step = 1.5f;
                spans[count].path = node->links[i].path;
                func_m011_00C34(&spans[count], x, y, z);
                count++;
            }
        }
        for (i = count - 1; i > 0; i--) {
            for (j = 0; j < i; j++) {
                d0 = sorted[j]->b.dist;
                if (sorted[j]->a.dist < d0) {
                    d0 = sorted[j]->a.dist;
                }
                d1 = sorted[j + 1]->b.dist;
                if (sorted[j + 1]->a.dist < d1) {
                    d1 = sorted[j + 1]->a.dist;
                }
                if (d1 < d0) {
                    tmp = sorted[j];
                    sorted[j] = sorted[j + 1];
                    sorted[j + 1] = tmp;
                }
            }
        }
    }
    cam2->path = 0xFF;
    for (i = 0; i < count && i < 2; i++) {
        pos = &sorted[i]->a;
        cam->path = ((M011Span *) pos)->path;
        if (sorted[i]->b.dist < sorted[i]->a.dist) {
            pos = &sorted[i]->b;
        }
        cam->node = pos->node;
        cam->t = pos->t;
        cam->x = pos->x;
        cam->y = pos->y;
        cam->z = pos->z;
        cam->radiusSq = pos->dist;
        cam = cam2;
    }
}

/* Stores four angles, unwrapped so that neighbours differ by less than half a turn */
static void func_m011_01288(f32 a, f32 b, f32 c, f32 d, f32 *out) {
    s32 i;
    s32 j;
    f32 diff;

    out[0] = a;
    out[1] = b;
    out[2] = c;
    out[3] = d;
    for (i = 0; i < 3; i++) {
        diff = out[i + 1] - out[i];
        if (diff >= 32768.0f) {
            for (j = i + 1; j < 4; j++) {
                out[j] -= 65536.0f;
            }
        } else if (diff < -32768.0f) {
            for (j = i + 1; j < 4; j++) {
                out[j] += 65536.0f;
            }
        }
    }
}

/* The four nodes around `node` on path `path`: the next one, the node, the previous two */
static s32 func_m011_01424(M011Node *node, s32 path, M011Node **next, M011Node **cur, M011Node **prev,
                           M011Node **prev2) {
    s32 i;

    *next = NULL;
    *cur = node;
    *prev = NULL;
    *prev2 = NULL;
    for (i = 0; i < 3; i++) {
        if (path == node->links[i].path) {
            *next = node->links[i].next;
            *prev = node->links[i].prev;
        }
    }
    for (i = 0; i < 3; i++) {
        if (path == (*prev)->links[i].path) {
            *prev2 = (*prev)->links[i].prev;
        }
    }
    if (*next != NULL && *cur != NULL && *prev != NULL && *prev2 != NULL) {
        return TRUE;
    }
    return FALSE;
}

/* The position at `t` on path `path` between `node` and the next node */
void func_m011_01510(M011Node *node, s32 path, f32 t, f32 *x, f32 *y, f32 *z) {
    M011Node *next;
    M011Node *cur;
    M011Node *prev;
    M011Node *prev2;

    if (func_m011_01424(node, path, &next, &cur, &prev, &prev2)) {
        *x = func_80049798(next->x, cur->x, prev->x, prev2->x, t);
        *y = func_80049798(next->y, cur->y, prev->y, prev2->y, t);
        *z = func_80049798(next->z, cur->z, prev->z, prev2->z, t);
    }
}

/* The camera position and angles from up to two path cameras, blended by the first one's weight */
void func_m011_015FC(M011Cam *cam, M011Cam *cam2, M011Target *target, f32 *x, f32 *y, f32 *z, s16 *rx, s16 *ry,
                     s16 *rz) {
    s32 i;
    s32 count;
    s32 angleCount;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;
    f32 radiusSq;
    f32 w2;
    f32 w1;
    f32 posX[2];
    f32 posY[2];
    f32 posZ[2];
    f32 angleX[2];
    f32 angleY[2];
    f32 angleZ[2];
    f32 vals[4];
    M011Node *next;
    M011Node *cur;
    M011Node *prev;
    M011Node *prev2;
    M011Node *other;
    M011Cam *c;

    i = 0;
    count = 0;
    angleCount = 0;
    c = cam;
    for (; i < 2; c = cam2, i++) {
        if (c->path != 0xFF && func_m011_01424(c->node, c->path, &next, &cur, &prev, &prev2)) {
            posX[count] = func_80049798(next->camX, cur->camX, prev->camX, prev2->camX, c->t);
            posY[count] = func_80049798(next->camY, cur->camY, prev->camY, prev2->camY, c->t);
            posZ[count] = func_80049798(next->camZ, cur->camZ, prev->camZ, prev2->camZ, c->t);
            dist = c->node->unk4;
            other = func_m011_0080C(c->node, c->path);
            if (other != NULL) {
                dist += (other->unk4 - dist) * c->t;
            }
            if (dist > 0.0f) {
                radiusSq = c->radiusSq;
                dx = target->x - c->x;
                dy = target->y - c->y;
                dz = target->z - c->z;
                if (dist * dist < radiusSq) {
                    radiusSq = dist / func_80097140(radiusSq);
                    dx *= radiusSq;
                    dy *= radiusSq;
                    dz *= radiusSq;
                }
                posX[count] += dx;
                posY[count] += dy;
                posZ[count] += dz;
            }
            count++;
            if (target->unk6 >= 2) {
                func_m011_01288(next->angleX, cur->angleX, prev->angleX, prev2->angleX, vals);
                angleX[angleCount] = func_80049798(vals[0], vals[1], vals[2], vals[3], c->t);
                func_m011_01288(next->angleY, cur->angleY, prev->angleY, prev2->angleY, vals);
                angleY[angleCount] = func_80049798(vals[0], vals[1], vals[2], vals[3], c->t);
                func_m011_01288(next->angleZ, cur->angleZ, prev->angleZ, prev2->angleZ, vals);
                angleZ[angleCount] = func_80049798(vals[0], vals[1], vals[2], vals[3], c->t);
                angleCount++;
            }
        }
    }
    if (count > 0) {
        if (count >= 2) {
            w2 = 1.0f - cam->weight;
            w1 = 1.0f / (1.0f + w2);
            posX[0] = (posX[0] + posX[1] * w2) * w1;
            posY[0] = (posY[0] + posY[1] * w2) * w1;
            posZ[0] = (posZ[0] + posZ[1] * w2) * w1;
        }
        *x = posX[0];
        *y = posY[0];
        *z = posZ[0];
        if (angleCount > 0) {
            if (angleCount >= 2) {
                angleX[0] = (angleX[0] + angleX[1] * w2) * w1;
                angleY[0] = (angleY[0] + angleY[1] * w2) * w1;
                angleZ[0] = (angleZ[0] + angleZ[1] * w2) * w1;
            }
            *rx = angleX[0];
            *ry = angleY[0];
            *rz = angleZ[0];
            return;
        }
        transform_offset_world_pos(target, &dx, &dy, &dz);
        dx = *x - dx;
        dy = *y - dy;
        dz = *z - dz;
        radiusSq = func_80097140(dx * dx + dz * dz);
        *rx = 0x8000 - func_80049C84(dx, dz);
        *ry = func_80049C84(dy, radiusSq);
        *rz = 0;
    }
}

#ifdef NON_MATCHING
/*
 * The next path node object (behaviour 0x12) of path `path` from objs[*index] on, or NULL.
 * NON_MATCHING: same code but for two registers: `out` and the node pointer are in v0/t1 where the
 * ROM has t1/v0 (the `out` copy and the empty `if (objs)` came from the permuter: 18 diffs -> 4).
 */
static Object *func_m011_01BD0(s32 path, s32 *index, s32 count, Object **objs, M011Node **node) {
    M011Node **out;
    Object *obj;
    Object *found;

    found = NULL;
    if (objs) {
    }
    out = node;
    while (*index < count) {
        obj = objs[*index];
        if (obj->behaviorId == 0x12 && path == ((M011Node *) obj->unk68)->path) {
            *out = obj->unk68;
            found = obj;
        }
        (*index)++;
        if (found != NULL) {
            break;
        }
    }
    return found;
}
#else
#undef func_m011_01BD0
INCLUDE_ASM("asm/nonmatchings/modules/m011", func_m011_01BD0);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m011", bss);
