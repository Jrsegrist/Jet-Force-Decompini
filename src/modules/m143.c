/*
 * Runlink module 143: an enemy kind that flies in a linked group.
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

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00000);

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_0007C);

extern void func_8006F998(s32 arg0, void *tex);
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

#ifdef NON_MATCHING
/*
 * Draws a ring: its three batches of vertices and their triangles. Same
 * instructions; the ROM adds each batch's offset to the buffer start without
 * the 0x300, and the scheduling and t-registers differ.
 */
void func_m143_00280(Gfx **dl, Mtx **mtx, Object *obj) {
    M143Ring *ring;
    Gfx *g;
    Mtx *m;
    M143Vtx *v;

    g = *dl;
    ring = obj->unk68;
    m = *mtx;
    func_8006F998(2, rl_m143_032D0);
    func_80055C00(&g, rl_m143_032D0, 0x20E, 0);
    gDPSetPrimColor(g++, 0, 0, 255, 255, 0, 255);
    gDPSetEnvColor(g++, 229, 0, 0, 0);
    func_80041CA0(&g, &m, obj, 1.0f, 0.0f);
    gDPPipeSync(g++);
    v = &ring->verts[ring->buffer * 48];
    gSPVertexJFG(g++, OS_K0_TO_PHYSICAL(v), 25, 0);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL(ring), 12, 1);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0xC0), 10, 1);
    gSPVertexJFG(g++, OS_K0_TO_PHYSICAL(v + 22), 25, 0);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0x160), 12, 1);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0x220), 10, 1);
    gSPVertexJFG(g++, OS_K0_TO_PHYSICAL(v + 44), 7, 0);
    gSPPolygon(g++, OS_K0_TO_PHYSICAL((u8 *) ring + 0x2C0), 4, 1);
    func_80041DC0(&g);
    gDPPipeSync(g++);
    gDPSetEnvColor(g++, 255, 255, 255, 255);
    gDPSetPrimColor(g++, 0, 0, 255, 255, 255, 255);
    *dl = g;
    *mtx = m;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_00280);
#endif

#ifdef NON_MATCHING
/*
 * A ring's init: clears its vertices and lays out its 48 triangles round them,
 * mapping the texture. The ROM unrolls the triangle loop twice and orders the
 * stores differently.
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
    func_8006F998(2, rl_m143_032D0);
    h = (rl_m143_032D0->height - 1) << 5;
    u = 0;
    w = (rl_m143_032D0->width - 1) << 5;
    k = 0;
    t = (Triangle *) ring;
    for (i = 0; i < 48; i += 2) {
        t[0].flags = 0x40;
        t[0].vi0 = k % 24;
        t[0].vi1 = (k + 3) % 24;
        t[0].vi2 = (k + 1) % 24;
        t[0].uv[0][0] = u;
        t[0].uv[0][1] = 0;
        t[0].uv[1][0] = w;
        t[0].uv[1][1] = h;
        t[0].uv[2][0] = u;
        t[0].uv[2][1] = h;
        t[1].flags = 0x40;
        t[1].vi0 = k % 24;
        t[1].vi1 = (k + 2) % 24;
        t[1].vi2 = (k + 3) % 24;
        t[1].uv[0][0] = u;
        t[1].uv[0][1] = 0;
        t[1].uv[1][0] = w;
        t[1].uv[1][1] = 0;
        t[1].uv[2][0] = w;
        t[1].uv[2][1] = h;
        t += 2;
        k += 2;
        if (k >= 21) {
            k = 0;
        }
        tmp = u;
        u = w;
        w = tmp;
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

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_01940);

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

INCLUDE_ASM("asm/nonmatchings/modules/m143", func_m143_02520);

void func_m143_02B88(void) {
    if (rl_m143_02F74 != 0) {
        sound_stop(rl_m143_02F74);
    }
}
#pragma weak __rl_fini = func_m143_02B88

INCLUDE_ASM("asm/nonmatchings/modules/m143", data);

INCLUDE_ASM("asm/nonmatchings/modules/m143", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m143", bss);
