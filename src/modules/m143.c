/*
 * Runlink module 143: an enemy kind that flies in a linked group.
 * Family: enemy behaviours (docs/runlink.md, "Module families").
 */
#include "module.h"
#include "gfx.h"

/* What each member's obj->unk68 points at */
typedef struct M143Member {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ Object *next;
    /* 0x2C */ u8 pad2C[0xA6 - 0x2C];
    /* 0xA6 */ s16 health;
} M143Member;

/* The group */
typedef struct M143Group {
    /* 0x00 */ u8 count;
    /* 0x01 */ u8 pad1[0x58 - 0x1];
    /* 0x58 */ Object *first;
    /* 0x5C */ u8 pad5C[0x84 - 0x5C];
    /* 0x84 */ s32 sound;
} M143Group;

/* A vertex of the ring */
typedef struct M143Vtx {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 r;
    /* 0x07 */ u8 g;
    /* 0x08 */ u8 b;
    /* 0x09 */ u8 a;
} M143Vtx;

/* What a ring's obj->unk68 points at */
typedef struct M143Ring {
    /* 0x000 */ u8 pad0[0x300];
    /* 0x300 */ M143Vtx verts[2 * 48]; /* two buffers of 48 */
    /* 0x6C0 */ u8 pad6C0[0x6EC - 0x6C0];
    /* 0x6EC */ s32 radius;
    /* 0x6F0 */ u8 pad6F0[0x6F8 - 0x6F0];
    /* 0x6F8 */ u8 buffer;
    /* 0x6F9 */ u8 hit;
} M143Ring;

extern void sound_stop(s32 handle);
extern Object *func_m003_053C4(void); /* the closest player */
extern f32 func_80097140(f32 x);     /* sqrtf */
extern void func_800013FC(s32 sound, s32 handle, s32 volume);

/* A follower's timer */
typedef struct M143Timer {
    /* 0x00 */ u16 time;
    /* 0x02 */ u16 done;
} M143Timer;

/* What a follower's obj->unk68 points at */
typedef struct M143Follower {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ u8 stop;
    /* 0x19 */ u8 pad19[0x38 - 0x19];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0x64 - 0x44];
    /* 0x64 */ Object *leader;
    /* 0x68 */ u8 pad68[0xA8 - 0x68];
    /* 0xA8 */ M143Timer timer;
} M143Follower;

/* What the leader's interactObj points at, as far as the followers use it */
typedef struct M143Interaction {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ f32 x;
    /* 0x1C */ f32 y;
    /* 0x20 */ f32 z;
} M143Interaction;

extern s32 D_800FE9F8;

extern s32 rl_m143_02F74; /* a sound playing */
extern s32 rl_m143_032C8;

/* A point of the trail the group follows, and the distance from the one before */
typedef struct M143TrailPoint {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
    /* 0xC */ u8 dist;
    /* 0xD */ u8 padD[3];
} M143TrailPoint;

/* .bss (still assembly): the trail, a ring of 41 points, and its newest point */
extern M143TrailPoint rl_m143_02FE0[41];
extern s32 rl_m143_03270;

/* The trail point about `dist` back from the newest one (or the oldest) (called from 0007C) */
static void func_m143_00000(s32 dist, f32 *x, f32 *y, f32 *z) {
    s32 i = rl_m143_03270;
    M143TrailPoint *p;

    while (dist > 0) {
        if (--i < 0) {
            i = 40;
        }
        if (i == rl_m143_03270) {
            break;
        }
        dist -= rl_m143_02FE0[i].dist;
    }
    p = &rl_m143_02FE0[i];
    *x = p->x;
    *y = p->y;
    *z = p->z;
}

extern void func_m003_0609C(s32 joint, f32 *x, f32 *y, f32 *z); /* a joint's position */

/*
 * .data: the picture (0x380 bytes) and the tables after it up to 0x2F78 stay assembly
 * (data.s, cut there by tools/runlink_split.py's DATA_SPLITS); the rest is the statics
 * of 0007C (0x2F78) and 02520 (0x2F7C, 0x2F80), which follow it in this order.
 */
INCLUDE_ASM("asm/nonmatchings/modules/m143", data);
extern u8 rl_m143_02BC0[]; /* (.data) a picture copied in once */

/*
 * Where a group member aims: -1/-2 are fixed points off the object, -3..-7
 * random joints (-3 never the same one twice running), -8 a point on the
 * trail, any other value that joint. (point is dead once the switch has
 * dispatched, so case -3 reuses it for the joint it picks.)
 */
void func_m143_0007C(Object *obj, s32 point, f32 *x, f32 *y, f32 *z) {
    static s32 sLastJoint = 0; /* .data 0x2F78 */

    switch (point) {
        case -1:
            *x = obj->trans.x_position + 332.0f;
            *y = obj->trans.y_position + 183.0f;
            *z = obj->trans.z_position - 388.0f;
            break;
        case -2:
            *x = obj->trans.x_position - 332.0f;
            *y = obj->trans.y_position + 183.0f;
            *z = obj->trans.z_position - 388.0f;
            break;
        case -3:
            do {
                point = func_80048E2C(9, 16);
            } while (point == sLastJoint);
            func_m003_0609C(sLastJoint = point, x, y, z);
            break;
        case -4:
            func_m003_0609C(func_80048E2C(11, 12), x, y, z);
            break;
        case -6:
            func_m003_0609C(func_80048E2C(13, 14), x, y, z);
            break;
        case -5:
            func_m003_0609C(func_80048E2C(9, 10), x, y, z);
            break;
        case -7:
            func_m003_0609C(func_80048E2C(15, 16), x, y, z);
            break;
        case -8:
            func_m143_00000(func_80005064(0x28), x, y, z);
            break;
        default:
            func_m003_0609C(point, x, y, z);
            break;
    }
}

extern void effect_slot_start(s32 arg0, void *tex);
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */
extern void func_80041CA0(Gfx **dl, Mtx **mtx, Object *obj, f32 scale, f32 arg4);
extern void func_80041DC0(Gfx **dl);
/* The start of a texture */
typedef struct M143Texture {
    /* 0x0 */ u8 width;
    /* 0x1 */ u8 height;
    /* 0x2 */ u8 pad2[4];
    /* 0x6 */ s16 flags;
} M143Texture;

extern M143Texture *rl_m143_032D0; /* the ring's texture (.bss) */

/*
 * Draws a ring: its three batches of vertices (from the current buffer) and
 * their triangles.
 */
void func_m143_00280(Gfx **dl, Mtx **mtx, Object *obj) {
    M143Ring *ring;
    Gfx *g;
    Mtx *m;
    s32 k;

    ring = obj->unk68;
    g = *dl;
    m = *mtx;
    effect_slot_start(2, rl_m143_032D0);
    func_80055C00(&g, rl_m143_032D0, 0x20E, 0);
    gDPSetPrimColor(g++, 0, 0, 255, 255, 0, 255);
    gDPSetEnvColor(g++, 229, 0, 0, 0);
    func_80041CA0(&g, &m, obj, 1.0f, 0.0f);
    gDPPipeSync(g++);
    k = ring->buffer * 48;
    gSPVertexJFG(g++, OS_K0_TO_PHYSICAL(&ring->verts[k]), 25, 0);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL(ring), 12, 1);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0xC0), 10, 1);
    gSPVertexJFG(g++, OS_K0_TO_PHYSICAL(&ring->verts[k + 22]), 25, 0);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0x160), 12, 1);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0x220), 10, 1);
    gSPVertexJFG(g++, OS_K0_TO_PHYSICAL(&ring->verts[k + 44]), 7, 0);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0x2C0), 4, 1);
    func_80041DC0(&g);
    gDPPipeSync(g++);
    gDPSetEnvColor(g++, 255, 255, 255, 255);
    gDPSetPrimColor(g++, 0, 0, 255, 255, 255, 255);
    *dl = g;
    *mtx = m;
}

#ifdef NON_MATCHING
/*
 * A ring's init: clears its vertices and lays out its 48 triangles round them,
 * mapping the texture. ROM length and frame; 42 words differ, all t-register
 * numbers from the texture-size set-up on (the ROM's free-list order is two
 * temporaries further on).
 */
void func_m143_00548(Object *obj, UNUSED LevelObjectEntryCommon *setup) {
    M143Ring *ring;
    M143Vtx *v;
    Triangle *t;
    s32 i;
    s32 k;
    s16 u;
    s16 w;
    s16 h;
    s16 tmp;

    ring = obj->unk68;
    v = ring->verts;
    i = 100;
    while (i--) {
        v->x = 0;
        v->y = 0;
        v->z = 0;
        v->r = 0xFF;
        v->g = 0xFF;
        v->b = 0xFF;
        v->a = 0xFF;
        v++;
    }
    rl_m143_032D0->flags &= ~0x100;
    ring->hit = 0;
    effect_slot_start(2, rl_m143_032D0);
    {
        s32 ws = (rl_m143_032D0->width - 1) << 5;

        h = (rl_m143_032D0->height - 1) << 5;
        u = 0;
        w = ws;
    }
    k = 0;
    t = (Triangle *) ring;
    for (i = 0; i < 48; i += 2) {
        t[0].vi0 = k % 24;
        t[1].vi0 = k % 24;
        t[0].vi1 = (k + 3) % 24;
        t[1].vi2 = (k + 3) % 24;
        t[0].vi2 = (k + 1) % 24;
        t[1].vi1 = (k + 2) % 24;
        t[0].uv[0][0] = u;
        t[0].uv[2][0] = u;
        t[1].uv[0][0] = u;
        t[0].uv[1][0] = w;
        t[1].uv[1][0] = w;
        t[1].uv[2][0] = w;
        t[0].uv[0][1] = 0;
        t[0].uv[1][1] = h;
        t[0].uv[2][1] = h;
        t[0].flags = 0x40;
        t[1].uv[0][1] = 0;
        t[1].uv[1][1] = 0;
        t[1].uv[2][1] = h;
        t[1].flags = 0x40;
        t += 2;
        tmp = u;
        u = w;
        w = tmp;
        k += 2;
        if (k >= 21) {
            k = 0;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00548);
#endif

/* A ring that grows outwards along the ground and hurts a player it passes */
void func_m143_008C0(Object *obj, s32 speed) {
    M143Ring *ring = obj->unk68;
    M143Vtx *v = &ring->verts[ring->buffer * 48];
    M143Vtx *w = v + 1;
    Object *player = func_m003_053C4();
    s32 angle;
    f32 dx;
    f32 dz;
    f32 d;
    s32 volume;

    for (angle = 0; angle != 0x10A9A; angle += 0xAAA) {
        w->x = func_80049C34((s16) angle) * (f32) (ring->radius + 10);
        w->z = func_80049C30((s16) angle) * (f32) (ring->radius + 10);
        v->x = func_80049C34((s16) angle) * (f32) (ring->radius - 10);
        v->z = func_80049C30((s16) angle) * (f32) (ring->radius - 10);
        w->y = 0;
        v->y = 50;
        w += 2;
        v += 2;
    }
    if (!ring->hit) {
        dx = player->trans.x_position - obj->trans.x_position;
        dz = player->trans.z_position - obj->trans.z_position;
        d = func_80097140(dx * dx + dz * dz) - (f32) ring->radius;
        if (d < 0.0f) {
            d = -d;
        }
        if (d < 30.0f && player->trans.y_position - obj->trans.y_position < 50.0f) {
            ((s16 *) player->interactObj)[3] -= 0x500;
            ring->hit = 1;
        }
    }
    ring->buffer ^= 1;
    ring->radius += speed * 5;
    if (ring->radius > 1300) {
        func_800079B8(obj);
    }
    if (ring->radius >= 434 && ring->radius < 473 && rl_m143_02F74 == 0) {
        func_80001248(0x20F, (s32) &rl_m143_02F74);
    }
    if (rl_m143_02F74 != 0) {
        volume = ring->radius * 80 / 1300 + 47;
        if (volume > 127) {
            volume = 127;
        }
        if (volume < 0) {
            volume = 47;
        }
        func_800013FC(0x20F, rl_m143_02F74, volume & 0xFF);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00C2C);

INCLUDE_ASM("asm/nonmatchings/modules/m143/ro", func_m143_01940);

/* Follows its leader's position until its time is up or it is told to stop */
void func_m143_02394(Object *obj) {
    M143Follower *f = obj->unk68;
    M143Timer *t = &f->timer;
    Object *leader = f->leader;

    if (rl_m143_032C8) {
        obj->interactObj->hitPoints = -0x400;
    }
    t->time += D_800FE9F8;
    if (t->time >= 0xB5) {
        t->done = 1;
    }
    if (f->stop || t->done) {
        obj->interactObj->hitPoints = -0x400;
        return;
    }
    f->x = ((M143Interaction *) leader->interactObj)->x;
    f->y = ((M143Interaction *) leader->interactObj)->y;
    f->z = ((M143Interaction *) leader->interactObj)->z;
}

/* Now and then one living member of the group, picked at random, makes its sound */
void func_m143_02450(M143Group *group) {
    Object *o;
    Object *pick;
    s32 r;
    s32 i;
    M143Member *m;

    if (group->sound == 0 && func_80048E2C(0, 100) >= 96) {
        o = group->first;
        pick = o;
        r = func_80048E2C(0, group->count);
        for (i = 0; o != NULL; i++) {
            if (i == r) {
                pick = o;
            }
            m = o->unk68;
            o = m->next;
        }
        if (pick != NULL && (m = pick->unk68, m->health > 0)) {
            func_80003410(0x1E3, pick->trans.x_position, pick->trans.y_position, pick->trans.z_position, 4,
                          &group->sound);
        }
    }
}

extern u8 *D_80102D60[4]; /* the effect slots' two buffers */
extern u8 *D_80102D78[4];


/*
 * An effect slot's handler (effect_slot_start in 69050.c calls it with the slot, the texture and an init flag):
 * a rising-heat blur. A random bottom row; each pixel becomes the average of itself and the three below it, into the other
 * buffer; everything dims by 3; the picture is copied in once on a flag; then the texture gets intensity/alpha
 * pairs, every other row shifted by two pixels. The countdown loops' dead copies go through `off` (their register
 * in the ROM is off's); off is set before every read of it in the shifted rows.
 */
void func_m143_02520(s32 slot, u8 *tex, s32 init) {
    static u8 sCurBuffer = 0;
    static s32 sCopyPicture = 1;
    s32 i;
    s32 j;
    s32 w;
    s32 h;
    u8 *cur;
    s16 c;
    s16 d;
    s32 off;
    u8 *p;
    u16 *out;
    u16 *row;
    u8 *buf;
    u8 *q;

    h = tex[1];
    w = tex[0];
    if (sCurBuffer) {
        buf = D_80102D60[slot];
    } else {
        buf = D_80102D78[slot];
    }
    p = buf + (h - 1) * w;
    i = w;
    while (off = i, i--, off) {
        *p++ = func_80048E2C(0, 255);
    }
    q = buf;
    sCurBuffer ^= 1;
    if (sCurBuffer) {
        buf = D_80102D60[slot];
    } else {
        buf = D_80102D78[slot];
    }
    cur = q;
    p = buf;
    i = h - 1;
    while (off = i, i--, off) {
        *p = (cur[w + 1] + ((cur + w)[w - 1] + (cur[0] + cur[w]))) >> 2;
        p++;
        cur++;
        j = w - 2;
        while (off = j, j--, off) {
            *p = (cur[w + 1] + (cur[w - 1] + (cur[0] + cur[w]))) >> 2;
            p++;
            cur++;
        }
        *p = (cur[1] + (cur[w - 1] + (cur[0] + cur[w]))) >> 2;
        p++;
        cur++;
    }
    p = buf;
    i = h * w;
    while (off = i, i--, off) {
        d = *p - 3;
        if (d < 0) {
            d = 0;
        }
        *p++ = d;
    }
    if (sCopyPicture) {
        cur = rl_m143_02BC0;
        p = buf;
        i = h * w;
        while (off = i, i--, off) {
            *p++ = *cur++;
        }
        sCopyPicture = 0;
    }
    out = (u16 *) (tex + 0x20);
    cur = buf;
    i = h;
    while (off = i, i--, off) {
        s16 v;

        row = out;
        if (i & 1) {
            j = w;
            while (off = j, j--, off) {
                v = *cur;
                c = v;
                d = v >> 4;
                if (d) {
                    c = v * d;
                }
                if (c >= 256) {
                    c = 255;
                }
                *out++ = c | (v << 8);
                cur++;
            }
        } else {
            for (j = 0; j < w; j++) {
                if (j & 2) {
                    off = -2;
                } else {
                    off = 2;
                }
                out = off + row + j;
                v = *cur;
                c = v;
                d = v >> 4;
                if (d) {
                    c = v * d;
                }
                if (c >= 256) {
                    c = 255;
                }
                *out = c | (v << 8);
                cur++;
            }
            out = row + w;
        }
    }
}

void func_m143_02B88(void) {
    if (rl_m143_02F74 != 0) {
        sound_stop(rl_m143_02F74);
    }
}
#pragma weak __rl_fini = func_m143_02B88

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m143", bss);
