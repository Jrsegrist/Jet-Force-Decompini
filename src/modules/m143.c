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
static void func_m143_0007C(Object *obj, s32 point, f32 *x, f32 *y, f32 *z) {
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

/*
 * A ring's init: clears its vertices and lays out its 48 triangles round them,
 * mapping the texture.
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
        s16 ws = (rl_m143_032D0->width - 1) << 5;

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

/* The flag word of a hit zone */
typedef struct M143ZoneFlags {
    u32 unk31 : 1;
    u32 shotAway : 1;
    u32 active : 1; /* can be hit */
    u32 unk0 : 29;
} M143ZoneFlags;

/* A hit zone of the boss's model (size 0x34) */
typedef struct M143HitZone {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ Vec3f pos;
    /* 0x18 */ u8 pad18[0x2C - 0x18];
    /* 0x2C */ s16 damage;
    /* 0x2E */ u8 pad2E;
    /* 0x2F */ u8 unk2F;
    /* 0x30 */ M143ZoneFlags flags;
} M143HitZone;

/* What obj->interactObj points at, for this boss */
typedef struct M143BossInteraction {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0x32 - 0x8];
    /* 0x32 */ s16 zoneCount;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ M143HitZone *zones;
} M143BossInteraction;

#define INTERACT(obj) ((M143BossInteraction *) (obj)->interactObj)

/* The boss's own state */
typedef struct M143BossState {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 phase; /* low nibble: the phase; the high nibble counts up */
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3; /* a countdown */
    /* 0x04 */ u8 pad4[0x8 - 0x4];
    /* 0x08 */ Object *parts[8];
    /* 0x28 */ s8 from[8]; /* the points (func_m143_0007C) each part runs between */
    /* 0x30 */ s8 to[8];
} M143BossState;

/* What the enemy record's owner keeps in its unk68 */
typedef struct M143OwnerData {
    /* 0x00 */ u8 pad0[0x2A];
    /* 0x2A */ s16 hitPoints;
    /* 0x2C */ u8 pad2C[0x30 - 0x2C];
    /* 0x30 */ u8 unk30;
} M143OwnerData;

/* The enemy record (obj->unk68) */
typedef struct M143Enemy {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ s16 unk10;
    /* 0x12 */ u8 pad12[0x1A - 0x12];
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 pad1B[0x24 - 0x1B];
    /* 0x24 */ Object *owner;
    /* 0x28 */ u8 pad28[0x4A - 0x28];
    /* 0x4A */ u8 unk4A;
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ u8 pad4C[0x5A - 0x4C];
    /* 0x5A */ u16 parts; /* the zones shot away */
    /* 0x5C */ u8 pad5C[0x64 - 0x5C];
    /* 0x64 */ Object *target;
    /* 0x68 */ u8 pad68[0xA0 - 0x68];
    /* 0xA0 */ u16 unkA0;
    /* 0xA2 */ u8 padA2[2];
    /* 0xA4 */ u16 unkA4;
    /* 0xA6 */ u8 padA6[2];
    /* 0xA8 */ M143BossState *state;
} M143Enemy;

/* A troop record of module 3, as used here */
typedef struct M143Troop {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 count;
    /* 0x02 */ u8 pad2[0x1A - 0x2];
    /* 0x1A */ u16 kind2 : 4;
    /* 0x1A */ u16 unk1A_0 : 12;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ u16 kind;
    /* 0x2E */ u8 pad2E[0x30 - 0x2E];
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 pad31[0x63 - 0x31];
    /* 0x63 */ u8 arrived;
    /* 0x64 */ u8 pad64[0x91 - 0x64];
    /* 0x91 */ u8 unk91;
} M143Troop;

/* A part's state (in the object at 0x88) */
typedef struct M143PartState {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
} M143PartState;

/* What a part's link object keeps in its unk68 */
typedef struct M143LinkData {
    /* 0x000 */ u8 pad0[0xA78];
    /* 0xA78 */ s16 unkA78;
    /* 0xA7A */ s16 unkA7A;
} M143LinkData;

/* What the first of the list at obj->header + 0xAC points at, as used here */
typedef struct M143ModelHeader {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ M143Texture **textures;
} M143ModelHeader;

/* What a player's interactObj points at, as used here */
typedef struct M143PlayerInteraction {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} M143PlayerInteraction;

extern void func_8000C9B4(u8, u8, u8, u8);
extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern void func_m087_011B0(s32 id, s16 value);
extern void func_m087_011DC(Object *obj);
extern void func_m003_04398(s32, s32);
extern void *troop_list_get(s32 *count);
extern void spawn_object_from_list(s16 arg0, s16 x, s16 y, s16 z, s32 arg4, s32 arg5);
extern void chain_object_unlink_from_players(Object *obj);
extern s32 func_80049C84(f32 dx, f32 dz); /* the angle of a vector */
extern s32 func_m003_0C324();
extern s32 func_m003_04260(s32, s32, u8, u8, u8);
extern s32 func_m003_0512C(s32 kind, f32 speed, s32 anim);

/* .bss (still assembly) */
extern s32 rl_m143_03274;
extern M143BossState rl_m143_03278; /* the boss's state */
extern M143Troop *rl_m143_032B0;
extern M143Troop *rl_m143_032B4;
extern Object *rl_m143_032B8;
extern Object *rl_m143_032BC;
extern Object *rl_m143_032C0;
extern Object *rl_m143_032C4;
extern s32 rl_m143_032CC; /* which side */
/* .data (still assembly) */
extern u8 rl_m143_02F70[];       /* how many parts, per unk2 */
extern s8 (*rl_m143_02F64[])[2]; /* their points, per unk2 */

/*
 * The boss's update: sets up its state, its two troops and its two objects 0x5B the first time; then its
 * parts' positions between their points, the troops following its zones, the zones shot away for its
 * phase, the trail, and its next attack through module 3
 */
void func_m143_00C2C(Object *obj) {
    M143Enemy *e;
    Object *ownerObj;
    M143OwnerData *owner;
    Object *player;
    M143BossState *state;
    Object **list;
    Object **objs;
    M143HitZone *zone;
    s32 n;
    s32 m;
    s8 (*pts)[2];
    s32 first;
    s32 count;
    Object *part;
    Object *link;
    M143LinkData *linkData;
    f32 p1x;
    f32 p1y;
    f32 p1z;
    f32 p2x;
    f32 p2y;
    f32 p2z;
    f32 lx;
    f32 ly;
    f32 lz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 h;
    f32 d;
    s32 action;

    e = obj->unk68;
    ownerObj = e->owner;
    owner = ownerObj->unk68;
    player = func_m003_053C4();
    state = e->state;
    if (state == NULL) {
        objs = func_80005E98(&first, &count);
        list = troop_list_get(&n);
        state = &rl_m143_03278;
        e->state = state;
        state->phase = 1;
        state->unk2 = 0;
        rl_m143_032C8 = 0;
        rl_m143_032D0 = *(**(M143ModelHeader ***) ((u8 *) obj->header + 0xAC))->textures;
        while (n--) {
            if (rl_m143_032B4 != NULL) {
                rl_m143_032B0 = list[n]->unk68;
                if (rl_m143_032B0->kind != 0x144) {
                    rl_m143_032B8 = list[n];
                    rl_m143_032B0->kind2 = 3;
                    rl_m143_032B0->count = 5;
                    rl_m143_032B0->unk30 = 7;
                    break;
                }
            } else {
                rl_m143_032B4 = list[n]->unk68;
                if (rl_m143_032B4->kind != 0x144) {
                    rl_m143_032BC = list[n];
                    rl_m143_032B4->kind2 = 3;
                    rl_m143_032B4->count = 5;
                    rl_m143_032B4->unk30 = 7;
                }
            }
        }
        for (n = first; n < count; n++) {
            Object *o = objs[n];

            if (o->behaviorId == 0x5B) {
                if (rl_m143_032C0 != NULL) {
                    rl_m143_032C4 = o;
                    break;
                }
                rl_m143_032C0 = o;
            }
        }
        func_m087_011B0(0x2CC, 0x7FFF);
        rl_m143_03274 = 0;
        n = INTERACT(obj)->zoneCount;
        while (n--) {
            zone = &INTERACT(obj)->zones[n];
            zone->unk2F = 0;
            zone->flags.active = 1;
        }
    }
    if ((state->phase & 0xF) < 3) {
        INTERACT(obj)->hitPoints = owner->hitPoints;
    }
    if (!rl_m143_03274) {
        func_m087_011DC(rl_m143_032C0);
        func_m087_011DC(rl_m143_032C4);
    }
    if (state->unk3 != 0 && --state->unk3 == 0) {
        obj->particleEmittersEnabled &= ~0x30;
    }
    if (state->parts[0] != NULL) {
        for (n = 0; n < 8; n++) {
            part = state->parts[n];
            link = *(Object **) part->unk68;
            linkData = link->unk68;
            part->trans.x_position = 0.0f;
            part->trans.y_position = -1000.0f;
            part->trans.z_position = 0.0f;
            OBJ_STATE(part, M143PartState)->unk4 = 0;
            part->trans.flags |= 0x400;
            linkData->unkA78 = 0;
            linkData->unkA7A = 0;
        }
    }
    if (state->unk0) {
        n = rl_m143_02F70[state->unk2];
        m = n >> 1; /* half of them (m is the count again below) */
        pts = rl_m143_02F64[state->unk2];
        if (rl_m143_032C0->modelIndex) {
            n -= m;
        }
        if (rl_m143_032C4->modelIndex) {
            n -= m;
            pts += m;
        }
        m = n;
        while (n--) {
            state->from[n] = (*pts)[0];
            state->to[n] = (*pts)[1];
            pts++;
        }
        n = m;
        while (n--) {
            func_m143_0007C(obj, state->from[n], &p1x, &p1y, &p1z);
            func_m143_0007C(obj, state->to[n], &p2x, &p2y, &p2z);
            if (state->to[n] == -8) {
                lx = p2x;
                ly = p2y;
                lz = p2z;
            }
            part = state->parts[n];
            link = *(Object **) part->unk68;
            link->trans.x_position = p1x;
            linkData = link->unk68;
            link->trans.y_position = p1y;
            link->trans.z_position = p1z;
            dx = p2x - p1x;
            dy = p2y - p1y;
            dz = p2z - p1z;
            h = dx * dx + dz * dz;
            d = func_80097140(dy * dy + h);
            h = func_80097140(h);
            link->trans.rotation.y_rotation = 0x4000 - func_80049C84(dz, dx);
            link->trans.rotation.x_rotation = func_80049C84(h, dy) + 0x4000;
            linkData->unkA7A = linkData->unkA78 = d;
            link->segmentID = player->segmentID;
            OBJ_STATE(part, M143PartState)->unk4 = (s32) d;
            part->trans.flags &= ~0x400;
        }
        if (state->unk2 == 2) {
            state->unk0 = 0;
            state->phase += 0x10;
            spawn_object_from_list(0x10, lx, ly, lz, 100, 0);
        }
    }
    if (rl_m143_032B0->unk30 != 7 && rl_m143_032B0->unk30 != 10) {
        rl_m143_032B8->trans.x_position = INTERACT(obj)->zones[21].pos.x;
        rl_m143_032B8->trans.y_position = INTERACT(obj)->zones[21].pos.y;
        rl_m143_032B8->trans.z_position = INTERACT(obj)->zones[21].pos.z;
    }
    if (rl_m143_032B4->unk30 != 7 && rl_m143_032B4->unk30 != 10) {
        rl_m143_032BC->trans.x_position = INTERACT(obj)->zones[22].pos.x;
        rl_m143_032BC->trans.y_position = INTERACT(obj)->zones[22].pos.y;
        rl_m143_032BC->trans.z_position = INTERACT(obj)->zones[22].pos.z;
    }
    rl_m143_032B0->unk91 = 3;
    rl_m143_032B4->unk91 = 3;
    e->target = player;
    e->unk10 = func_80049C84(obj->trans.x_position - player->trans.x_position,
                             obj->trans.z_position - player->trans.z_position) -
               obj->trans.rotation.y_rotation;
    switch (state->phase & 0xF) {
        case 1:
            if (!(e->parts & 2) && INTERACT(obj)->zones[21].damage > 0x700) {
                e->unkA0 |= 2;
                INTERACT(obj)->zones[21].flags.shotAway = 1;
                chain_object_unlink_from_players(obj);
                if (rl_m143_032B0->unk30 == 1) {
                    rl_m143_032B0->count = rl_m143_032B0->arrived;
                    rl_m143_032B0->unk30 = 3;
                    rl_m143_032B0->kind2 = 4;
                } else {
                    rl_m143_032B0->unk30 = 10;
                }
            }
            if (!(e->parts & 4) && INTERACT(obj)->zones[22].damage > 0x700) {
                e->unkA0 |= 4;
                INTERACT(obj)->zones[22].flags.shotAway = 1;
                chain_object_unlink_from_players(obj);
                if (rl_m143_032B4->unk30 == 1) {
                    rl_m143_032B4->count = rl_m143_032B4->arrived;
                    rl_m143_032B4->unk30 = 3;
                    rl_m143_032B4->kind2 = 4;
                } else {
                    rl_m143_032B4->unk30 = 10;
                }
            }
            break;
        case 2:
            if (!(e->parts & 8) && INTERACT(obj)->zones[20].damage > 0x700) {
                e->unkA0 |= 8;
                INTERACT(obj)->zones[20].flags.shotAway = 1;
                chain_object_unlink_from_players(obj);
            }
            if (!(e->parts & 0x10) && INTERACT(obj)->zones[18].damage > 0x700) {
                e->unkA0 |= 0x10;
                INTERACT(obj)->zones[18].flags.shotAway = 1;
                chain_object_unlink_from_players(obj);
            }
            break;
        case 3:
            break;
    }
    if ((obj->particleEmittersEnabled & 1) && (e->parts & 0x10)) {
        obj->particleEmittersEnabled &= ~1;
    }
    if ((obj->particleEmittersEnabled & 2) && (e->parts & 8)) {
        obj->particleEmittersEnabled &= ~2;
    }
    if ((obj->particleEmittersEnabled & 4) && (e->parts & 2)) {
        obj->particleEmittersEnabled &= ~4;
    }
    if ((obj->particleEmittersEnabled & 8) && (e->parts & 4)) {
        obj->particleEmittersEnabled &= ~8;
    }
    rl_m143_02FE0[rl_m143_03270].x = ((M143PlayerInteraction *) player->interactObj)->x;
    rl_m143_02FE0[rl_m143_03270].y = ((M143PlayerInteraction *) player->interactObj)->y;
    rl_m143_02FE0[rl_m143_03270].z = ((M143PlayerInteraction *) player->interactObj)->z;
    rl_m143_02FE0[rl_m143_03270].dist = D_800FE9F8;
    if (++rl_m143_03270 >= 41) {
        rl_m143_03270 = 0;
    }
    if (owner->unk30 == 3 && e->unk4A == 0 && e->unk1A != 5 && func_m003_0C324(obj) &&
        func_m003_04260(0x10, 5, 0, 0, 0)) {
        if (state->phase == 4 || (rl_m143_032C0->modelIndex && rl_m143_032C4->modelIndex && state->phase == 3)) {
            state->phase += 0x10;
        }
        switch (state->phase) {
            case 1:
                action = 5;
                break;
            case 2:
                action = 6;
                break;
            case 3:
                action = 7;
                break;
            default:
                action = 0x22;
                break;
        }
        func_m003_0512C(6, 1.0f, action);
    }
}

/* A shot's object entry */
typedef struct M143ShotEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s16 vx;
    /* 0x0E */ s16 vy;
    /* 0x10 */ s16 vz;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ Object *owner;
} M143ShotEntry;

/* A short object entry */
typedef struct M143Entry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 padA;
} M143Entry;

/* A part's object entry */
typedef struct M143PartEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ Object *owner;
} M143PartEntry;

/* What the boss does when module 3 tells it an event */
void func_m143_01940(Object *obj, s32 event) {
    M143Enemy *e;
    M143BossState *state;
    Object *player;
    M143ShotEntry shot;
    f32 px;
    f32 py;
    f32 pz;
    Object *spawned;
    M143HitZone *zone;
    UNUSED s32 padA;
    M143Entry entry;
    UNUSED s32 padB;
    f32 x;
    f32 y;
    f32 z;
    M143PartEntry part;
    s32 i;

    e = obj->unk68;
    state = e->state;
    player = func_m003_053C4();
    switch (event) {
        case 3:
            if (rl_m143_032CC) {
                if (e->parts & 2) {
                    rl_m143_032CC = 0;
                }
            } else if (e->parts & 4) {
                rl_m143_032CC = 1;
            }
            if (rl_m143_032CC) {
                obj->particleEmittersEnabled |= 4;
                INTERACT(obj)->zones[21].flags.active = 0;
                INTERACT(obj)->zones[21].unk2F = 100;
                e->unkA4 |= 2;
            } else {
                obj->particleEmittersEnabled |= 8;
                INTERACT(obj)->zones[22].flags.active = 0;
                INTERACT(obj)->zones[22].unk2F = 100;
                e->unkA4 |= 4;
            }
            func_8000C9B4(0xFF, 0xFF, 0xFF, 1);
            rl_m143_032C8 = 1;
            break;
        case 4:
            rl_m143_032C8 = 0;
            if (rl_m143_032CC) {
                if (rl_m143_032B0->unk30 != 10) {
                    rl_m143_032B0->unk30 = 0;
                    rl_m143_032B0->arrived = 0;
                    rl_m143_032B0->unk0 = 0;
                    rl_m143_032B0->unk91 = 3;
                }
            } else {
                if (rl_m143_032B4->unk30 != 10) {
                    rl_m143_032B4->unk30 = 0;
                    rl_m143_032B4->arrived = 0;
                    rl_m143_032B4->unk0 = 0;
                    rl_m143_032B4->unk91 = 3;
                }
            }
            break;
        case 5:
            obj->particleEmittersEnabled = 0;
            INTERACT(obj)->zones[21].flags.active = 1;
            INTERACT(obj)->zones[21].unk2F = 0;
            INTERACT(obj)->zones[22].flags.active = 1;
            INTERACT(obj)->zones[22].unk2F = 0;
            state->phase += 0x10;
            break;
        case 1:
            if (rl_m143_032CC) {
                if (e->parts & 8) {
                    rl_m143_032CC = 0;
                }
            } else if (e->parts & 0x10) {
                rl_m143_032CC = 1;
            }
            if (rl_m143_032CC) {
                obj->particleEmittersEnabled |= 2;
                INTERACT(obj)->zones[20].flags.active = 0;
                INTERACT(obj)->zones[20].unk2F = 100;
                e->unkA4 |= 0x20;
            } else {
                obj->particleEmittersEnabled |= 1;
                INTERACT(obj)->zones[18].flags.active = 0;
                INTERACT(obj)->zones[18].unk2F = 100;
                e->unkA4 |= 0x10;
            }
            func_8000C9B4(0xFF, 0xFF, 0xFF, 1);
            break;
        case 2:
            state->phase += 0x10;
            obj->particleEmittersEnabled = 0;
            if (rl_m143_032CC) {
                if (e->parts & 8) {
                    break;
                }
                INTERACT(obj)->zones[20].flags.active = 1;
                INTERACT(obj)->zones[20].unk2F = 0;
                px = INTERACT(obj)->zones[20].pos.x;
                py = INTERACT(obj)->zones[20].pos.y;
                pz = INTERACT(obj)->zones[20].pos.z;
            } else {
                if (e->parts & 0x10) {
                    break;
                }
                INTERACT(obj)->zones[18].flags.active = 1;
                INTERACT(obj)->zones[18].unk2F = 0;
                px = INTERACT(obj)->zones[18].pos.x;
                py = INTERACT(obj)->zones[18].pos.y;
                pz = INTERACT(obj)->zones[18].pos.z;
            }
            shot.objectID = 0x2D4;
            shot.size = 0x18;
            shot.unk3 = 0;
            shot.x = px;
            shot.y = py;
            shot.z = pz;
            shot.vx = (player->trans.x_position - px) / 10.0f;
            shot.vy = (player->trans.y_position - py) / 10.0f;
            shot.vz = (player->trans.z_position - pz) / 10.0f;
            shot.unkA = 0;
            shot.unkB = 100;
            shot.owner = obj;
            func_80003410(0x95, shot.x, shot.y, shot.z, 4, NULL);
            spawned = func_80005F14(&shot, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
            break;
        case 6:
            if (INTERACT(obj)->zones[15].flags.active) {
                i = INTERACT(obj)->zoneCount;
                while (i--) {
                    zone = &INTERACT(obj)->zones[i];
                    if (!zone->flags.shotAway) {
                        zone->unk2F = 100;
                        zone->flags.active = 0;
                    }
                }
                e->unkA4 = 0xFFFF;
                func_8000C9B4(0xFF, 0xFF, 0xFF, 11);
            }
            func_m087_011B0(0x2CC, 1);
            rl_m143_03274 = 1;
            if (!rl_m143_032C0->modelIndex || !rl_m143_032C4->modelIndex) {
                state->unk0 = 1;
                state->unk2 = 0;
            } else {
                func_m003_04398(1, 0);
                e->unk4B = 0;
                state->phase += 0x10;
            }
            break;
        case 7:
            state->unk2 = 1;
            break;
        case 8:
            state->unk2 = 2;
            break;
        case 10:
            entry.objectID = 0x2CA;
            entry.size = 10;
            func_m003_0609C(14, &x, &y, &z);
            entry.x = x;
            entry.y = obj->trans.y_position;
            entry.z = z;
            spawned = func_80005F14(&entry, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
            func_m003_0609C(12, &x, &y, &z);
            entry.x = x;
            entry.y = obj->trans.y_position;
            entry.z = z;
            spawned = func_80005F14(&entry, 1);
            if (spawned != NULL) {
                spawned->level_entry = NULL;
            }
            func_80001248(0x1CC, 0);
            obj->particleEmittersEnabled |= 0x30;
            state->unk3 = 2;
            state->phase &= 0xF;
            rl_m143_032CC ^= 1;
            switch (state->phase) {
                case 1:
                    if ((e->parts & 6) == 6) {
                        state->phase = 2;
                    }
                    break;
                case 2:
                    if ((e->parts & 0x18) == 0x18) {
                        state->phase = 3;
                        part.objectID = 0xDB;
                        part.size = 0x18;
                        part.x = 0;
                        part.y = 0;
                        part.z = 0;
                        part.unkA = 0;
                        part.unkC = 0;
                        part.unkE = 0;
                        part.unk10 = 0x28;
                        part.unk11 = 2;
                        part.owner = obj;
                        i = 8;
                        while (i--) {
                            state->parts[i] = func_80005F14(&part, 1);
                            OBJ_STATE(state->parts[i], M143PartState)->unk4 = 100;
                        }
                    }
                    break;
                case 3:
                    if (rl_m143_032C0->modelIndex && rl_m143_032C4->modelIndex) {
                        state->phase = 4;
                    }
                    break;
            }
            break;
    }
}

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
