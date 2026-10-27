/*
 * Runlink module 152: a many-headed creature made of five chains of eight
 * spheres. Each chain either sways around the object or reaches out to the
 * player (blending between the two by distance); the module keeps the chain
 * points, updates the creature's hit spheres from them and renders them back
 * to front.
 */
#include "module.h"

/* One chain: eight points (place and size) and its sway state */
typedef struct M152Chain {
    /* 0x00 */ f32 points[8][4]; /* x, y, z, size */
    /* 0x80 */ s32 unk80;
    /* 0x84 */ s16 phase;
    /* 0x86 */ s16 phaseSpeed;
    /* 0x88 */ s16 yaw;
    /* 0x8A */ s16 unk8A;
    /* 0x8C */ s16 unk8C;
} M152Chain;

/* One point to render, sorted by depth */
typedef struct M152DepthEntry {
    /* 0x0 */ f32 depth;
    /* 0x4 */ u8 chain;
    /* 0x5 */ u8 point;
} M152DepthEntry;

/* What the parent object's unk68 points at */
typedef struct M152ParentData {
    /* 0x00 */ u8 pad0[2];
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3[0x1B - 0x3];
    /* 0x1B */ s8 yaw;
} M152ParentData;

/* The creature object's unk68 */
typedef struct M152Hydra {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ Object *parent;
    /* 0x28 */ u8 pad28[0xA4 - 0x28];
    /* 0xA4 */ u16 chainMask;
    /* 0xA8 */ M152Chain *chains;
} M152Hydra;

/* One hit sphere */
typedef struct M152HitSphere {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 prevX;
    /* 0x1C */ f32 prevY;
    /* 0x20 */ f32 prevZ;
    /* 0x24 */ f32 radius;
    /* 0x28 */ f32 radius2;
    /* 0x2C */ u8 pad2C[0x34 - 0x2C];
} M152HitSphere;

typedef struct M152Interaction {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ M152HitSphere *spheres;
} M152Interaction;

extern s32 D_800FE9F8; /* frames this update */
extern s16 func_80049C84(f32 x, f32 z); /* angle of a vector */
extern f32 func_80097140(f32 x);        /* square root */
extern s16 func_80049D80(s16 from, s16 to); /* difference between two angles */
extern f32 get_distance_to_camera(f32 x, f32 y, f32 z); /* depth of a point from the camera */
extern void func_80041450(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 x, s32 y, s32 z, s32 arg7, f32 size,
                          f32 arg9, s32 arg10, s32 arg11);

f32 func_m152_00E34(s16 from, s16 to, u16 t);

/* All of the module's .bss: one struct (IDO addresses it from one base) */
static struct {
    M152Chain chains[5];
    M152DepthEntry depthList[40];
    s32 turnTime;
    u16 turnRate;
    s16 rippleAngle;
    s16 turnAngle;
    s16 turnFrom;
    s16 turnTo;
    s16 unused41E;
    s16 reachMode; /* 1: waving, 2: blending, 3: reaching */
    s16 reachBlend;
    s16 reachBlendSpeed;
} B;
static f32 sHydraScale;
static f32 sPlayerDistance;
static Object *sPlayer;
#define sChains B.chains
#define sDepthList B.depthList
#define sTurnTime B.turnTime
#define sTurnRate B.turnRate
#define sRippleAngle B.rippleAngle
#define sTurnAngle B.turnAngle
#define sTurnFrom B.turnFrom
#define sTurnTo B.turnTo
#define sReachMode B.reachMode
#define sReachBlend B.reachBlend
#define sReachBlendSpeed B.reachBlendSpeed

/*
 * NON_MATCHING: func_m152_004AC and func_m152_00B8C are not matched yet (see the notes at them).
 * func_m152_00000 matches, but it uses a .rodata float, and .rodata must come all from C or all
 * from assembly; so the default build takes it, the two unmatched functions and the data from
 * assembly.
 */
#ifdef NON_MATCHING
void func_m152_00000(Object *obj, M152Chain *chain, s32 index, f32 *outX, f32 *outY, f32 *outZ) {
    f32 radius;
    s16 pitch;
    s32 t;
    f32 cosPitch;

    t = sTurnTime - index * 1800;
    if (t < 0) {
        t = 0;
    }
    if (t > 0xFFFF) {
        t = 0xFFFF;
    }
    chain->yaw = func_m152_00E34(sTurnFrom, sTurnTo, t);
    chain->yaw += (chain - sChains) * 0x3333;
    pitch = func_80049C34(chain->phase + index * -0x1147) * 2000.0f + 5900.0f;
    radius = (f32) (index * 90) * sHydraScale;
    cosPitch = func_80049C30(pitch);
    *outX = func_80049C34(chain->yaw) * cosPitch * radius + obj->trans.x_position;
    *outY = func_80049C34(pitch) * radius + obj->trans.y_position;
    cosPitch = func_80049C30(pitch);
    *outZ = func_80049C30(chain->yaw) * cosPitch * radius + obj->trans.z_position;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m152", func_m152_00000);
#endif

void func_m152_00228(Object *obj, M152Chain *chain, s32 index, f32 *outX, f32 *outY, f32 *outZ) {
    M152Hydra *hydra;
    M152ParentData *data;
    s16 yaw;
    f32 dx;
    s16 angleY;
    s16 angleX;
    s16 unusedAngle; /* never used, but takes stack space */
    f32 dy;
    f32 dz;
    f32 dist;
    f32 offset;

    hydra = obj->unk68;
    data = hydra->parent->unk68;
    yaw = data->yaw << 8;
    dx = func_80049C34(yaw + 0x4000) * (f32) 0 + (sPlayer->trans.x_position - obj->trans.x_position);
    dy = sPlayer->trans.y_position - obj->trans.y_position;
    dz = func_80049C30(yaw + 0x4000) * (f32) 0 + (sPlayer->trans.z_position - obj->trans.z_position);
    angleY = func_80049C84(dx, dz);
    dist = dx * dx + dz * dz;
    angleX = func_80049C84(dy, func_80097140(dist));
    dist = func_80097140(dy * dy + dist);
    if (720.0f * sHydraScale < dist) {
        dist = 720.0f * sHydraScale;
    }
    dx = func_80049C30(angleX) * func_80049C34(angleY) * dist;
    dy = func_80049C34(angleX) * dist;
    dz = func_80049C30(angleX) * func_80049C30(angleY) * dist;
    offset = (chain - sChains) - 2;
    dx += func_80049C34(yaw) * offset * 56.0f;
    dz += func_80049C30(yaw) * offset * 56.0f;
    *outX = obj->trans.x_position + dx * (index / (f32) 8);
    *outY = obj->trans.y_position + dy * (index / (f32) 8);
    *outZ = obj->trans.z_position + dz * (index / (f32) 8);
}

#ifdef NON_MATCHING
/* Differs only in the three j loops of the switch: the original tests n != 0 before each
   (beq n, zero) although n is the constant 8; IDO folds that test here. */
void func_m152_004AC(Object *obj) {
    M152Hydra *hydra;
    M152HitSphere *sphere;
    M152Chain *chains;
    s16 angle;
    u32 n;
    s32 i;
    M152Chain *chain;
    f32 *p;
    f32 x;
    f32 y;
    f32 z;
    f32 reachX;
    f32 reachY;
    f32 reachZ;
    f32 blend;
    s32 j;
    s32 t;

    hydra = obj->unk68;
    sphere = ((M152Interaction *) obj->interactObj)->spheres;
    chains = hydra->chains;
    if (chains == NULL) {
        hydra->chains = sChains;
        sHydraScale = ((M152ParentData *) hydra->parent->unk68)->size / 142.0f;
        sPlayer = func_8000E4D0(0);
        angle = func_80049C84(sPlayer->trans.x_position - obj->trans.x_position,
                              sPlayer->trans.z_position - obj->trans.z_position);
        sRippleAngle = 0;
        sTurnTime = 0;
        sTurnRate = 375;
        sTurnAngle = angle;
        sTurnFrom = angle;
        sTurnTo = angle;
        sReachMode = 1;
        sReachBlend = 0;
        sReachBlendSpeed = 0;
        i = 5;
        chain = sChains;
        while (i--) {
            chain->unk80 = 0;
            chain->phase = func_80048E2C(0, 0xFFFF);
            chain->phaseSpeed = func_80048E2C(275, 500);
            chain->yaw = angle;
            chain->unk8A = angle;
            chain->unk8C = angle;
            chain++;
        }
    }
    sPlayerDistance = func_80097140((sPlayer->trans.x_position - obj->trans.x_position) *
                                        (sPlayer->trans.x_position - obj->trans.x_position) +
                                    (sPlayer->trans.z_position - obj->trans.z_position) *
                                        (sPlayer->trans.z_position - obj->trans.z_position));
    switch (sReachMode) {
        case 1:
            if (sPlayerDistance < 1500.0f * sHydraScale) {
                sReachMode = 2;
                sReachBlend = 0;
                sReachBlendSpeed = 2;
            }
            break;
        case 3:
            if (2000.0f * sHydraScale < sPlayerDistance) {
                sReachMode = 2;
                sReachBlend = 0x80;
                sReachBlendSpeed = -1;
            }
            break;
    }
    sRippleAngle -= D_800FE9F8 * 522;
    t = (sTurnTime < 0xFFFF) ? sTurnTime : 0xFFFF;
    sTurnAngle = func_m152_00E34(sTurnFrom, sTurnTo, t);
    sTurnTime += sTurnRate * D_800FE9F8;
    if (sTurnTime + sTurnRate * D_800FE9F8 > 100000) {
        sTurnFrom = sTurnAngle;
        sTurnTo = func_80048E2C(0, 0xFFFF);
        sTurnTime = 0;
    }
    sReachBlend += sReachBlendSpeed * D_800FE9F8;
    if (sReachBlend >= 0x80) {
        sReachMode = 3;
    } else if (sReachBlend <= 0) {
        sReachMode = 1;
    }
    n = 8;
    i = 5;
    chain = sChains;
    while (i--) {
        chain->phase += chain->phaseSpeed * D_800FE9F8;
        p = chain->points[0];
        switch (sReachMode) {
            case 1:
                j = n;
                while (j--) {
                    func_m152_00000(obj, chain, n - j, &x, &y, &z);
                    p[0] = x;
                    p[1] = y;
                    p[2] = z;
                    p += 4;
                }
                break;
            case 2:
                j = n;
                while (j--) {
                    blend = (func_80049C30((sReachBlend << 8) + 0x8000) + 1.0f) * 0.5f;
                    func_m152_00000(obj, chain, n - j, &x, &y, &z);
                    func_m152_00228(obj, chain, n - j, &reachX, &reachY, &reachZ);
                    p[0] = (reachX - x) * blend + x;
                    p[1] = (reachY - y) * blend + y;
                    p[2] = (reachZ - z) * blend + z;
                    p += 4;
                }
                break;
            case 3:
                j = n;
                while (j--) {
                    func_m152_00228(obj, chain, n - j, &x, &y, &z);
                    p[0] = x;
                    p[1] = y;
                    p[2] = z;
                    p += 4;
                }
                break;
        }
        j = 8;
        while (j--) {
            p = chain->points[j];
            p[3] = (func_80049C34(sRippleAngle - j * 0x2225 + 8) / 2.33f + 3.19f) * sHydraScale;
            sphere->prevX = sphere->x;
            sphere->prevY = sphere->y;
            sphere->prevZ = sphere->z;
            sphere->x = p[0];
            sphere->y = p[1];
            sphere->z = p[2];
            sphere->radius = p[3] * 25.0f;
            sphere->radius2 = p[3] * 25.0f;
            sphere++;
        }
        chain++;
    }
}

/* Differs only in the render loop: the original keeps the chain array's address in s6 (set before
   the loop, then base + chain * 0x90 + point * 0x10) and the hoisted arguments in s3-s5 in a
   different order; IDO rebuilds the address in the loop here. */
void func_m152_00B8C(Object *obj, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, u8 arg6, s32 inverted) {
    M152Hydra *hydra;
    s32 count;
    M152DepthEntry *e;
    s32 i;
    s32 j;
    s32 n;
    s32 swapped;
    s32 unused[4]; /* never used, but takes stack space */
    M152Chain *chains;
    M152Chain *chain;
    f32 *p;
    f32 depth;
    u8 b;

    hydra = obj->unk68;
    count = 0;
    e = sDepthList;
    chains = hydra->chains;
    if (chains != NULL) {
        for (i = 0, chain = sChains, n = 8; i < 5; i++, chain++) {
            p = chain->points[0];
            for (j = 0; j < n; j++, p += 4) {
                if ((hydra->chainMask & (1 << i)) ? inverted : !inverted) {
                    e->depth = -get_distance_to_camera(p[0], p[1], p[2]);
                    e->chain = i;
                    e->point = j;
                    e++;
                    count++;
                }
            }
        }
        if (count != 0) {
            do {
                swapped = 0;
                e = sDepthList;
                j = count - 1;
                while (j-- > 0) {
                    if (e[0].depth < e[1].depth) {
                        depth = e[0].depth;
                        e[0].depth = e[1].depth;
                        e[1].depth = depth;
                        b = e[0].chain;
                        e[0].chain = e[1].chain;
                        e[1].chain = b;
                        b = e[0].point;
                        e[0].point = e[1].point;
                        e[1].point = b;
                        swapped = 1;
                    }
                    e++;
                }
            } while (swapped);
            e = sDepthList;
            j = count;
            while (j--) {
                p = sChains[e->chain].points[e->point];
                func_80041450(arg1, arg2, arg3, arg4, p[0], p[1], p[2], 0, p[3], (f32) 0, arg5, arg6);
                e++;
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m152", func_m152_004AC);

INCLUDE_ASM("asm/nonmatchings/modules/m152", func_m152_00B8C);
#endif

f32 func_m152_00E34(s16 from, s16 to, u16 t) {
    s16 diff;

    diff = func_80049D80(from, to);
    return (func_80049C34((t >> 1) + 0xC000) + 1.0f) * 0.5f * diff + from;
}

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m152", data);
#endif
