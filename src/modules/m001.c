/*
 * Runlink module 1: a pool of 32 small particles (drops) that fall, land and
 * spread out as a fading splash, each drawn as two textured quads.
 */
#include "module.h"
#include "gfx.h"

typedef struct M001Particle {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 unk4;
    /* 0x06 */ u8 state; /* 0: free, 1: falling, 2: splashing */
    /* 0x07 */ u8 alpha;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 floorY;
    /* 0x1C */ f32 vx;
    /* 0x20 */ f32 vy;
    /* 0x24 */ f32 vz;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ s32 buffer; /* which of the two vertex sets is current */
    /* 0x30 */ Vertex verts[2][4];
} M001Particle;

/* What func_8004470C returns */
typedef struct M001Env {
    /* 0x00 */ u8 pad0[0xF3];
    /* 0xF3 */ u8 r;
    /* 0xF4 */ u8 g;
    /* 0xF5 */ u8 b;
} M001Env;

extern void *func_80056798(s32, s32);
extern void func_80056C14(void *);
extern u8 *func_80055694(s32);
extern void func_80055A00(void *);
extern void runlink_unload_timer_set(s32, s32, s32);
extern M001Env *func_8004470C(void);
extern ObjectTransform *func_80041DE0(void); /* the camera */
extern void func_8005600C(s32 r, s32 g, s32 b, s32 r2, s32 g2, s32 b2);
extern void func_80056064(void);
extern void func_80041174(Gfx **dl, s32 arg1, s32 arg2, M001Particle *p, void *model, s32 arg5, s32 arg6);
extern void func_80055C00(Gfx **gfx, void *tex, s32 flags, s32 offset); /* sets a texture */

void func_m001_002A0(M001Particle *p, s32 updateRate);

static Triangle sSplashTris[2] = {
    { 0x40, 0, 1, 3, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
    { 0x40, 0, 3, 2, { { 0, 0 }, { 0, 0 }, { 0, 0 } } },
};
static void *D_m001_00C50 = NULL; /* loaded with func_80056798 */
static void *D_m001_00C54 = NULL; /* loaded with func_80056798 */
static u8 *sSplashTexture = NULL;   /* texture, loaded with func_80055694 */
static s32 sDropsInUse = 0;      /* particles in use */
static M001Particle sDropPool[32];
static s32 sDropsLoaded; /* Loads the drop resources and clears the pool of 32 drops (module m001, 0x00000). */
/* set once the resources are loaded */

static void drop_pool_init(void) {
    s32 i;
    s32 j;
    s32 k;
    M001Particle *p;
    s32 w;
    s32 h;

    D_m001_00C50 = func_80056798(0x48, 0);
    D_m001_00C54 = func_80056798(0x45, 0);
    sSplashTexture = func_80055694(0x1AC);
    for (i = 0, p = sDropPool; i < 32; i++, p++) {
        p->state = 0;
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 4; k++) {
                p->verts[j][k].r = 0xFF;
                p->verts[j][k].g = 0xFF;
                p->verts[j][k].b = 0xFF;
                p->verts[j][k].a = 0xFF;
            }
        }
    }
    sDropsInUse = 0;
    if (sSplashTexture != NULL) {
        w = (sSplashTexture[0] - 1) << 5;
        h = (sSplashTexture[1] - 1) << 5;
        sSplashTris[0].uv[1][0] = w;
        sSplashTris[0].uv[2][0] = w;
        sSplashTris[0].uv[2][1] = h;
        sSplashTris[1].uv[1][0] = w;
        sSplashTris[1].uv[1][1] = h;
        sSplashTris[1].uv[2][1] = h;
    }
    sDropsLoaded = 1;
}

void func_m001_00120(void) {
    if (D_m001_00C50 != NULL) {
        func_80056C14(D_m001_00C50);
    }
    if (D_m001_00C54 != NULL) {
        func_80056C14(D_m001_00C54);
    }
    if (sSplashTexture != NULL) {
        func_80055A00(sSplashTexture);
    }
    sDropsLoaded = 0;
}

M001Particle *func_m001_00188(f32 scale, f32 x, f32 y, f32 z, f32 floorY, f32 vx, f32 vy, f32 vz) {
    M001Particle *p;
    s32 i;

    if (!sDropsLoaded) {
        drop_pool_init();
    }
    if (sDropsInUse >= 32) {
        return NULL;
    }
    for (i = 0, p = sDropPool; i < 32; i++, p++) {
        if (p->state == 0) {
            break;
        }
    }
    if (i < 32) {
        p->unk4 = 0;
        p->state = 1;
        p->alpha = 0x80;
        p->scale = scale;
        p->x = x;
        p->y = y;
        p->z = z;
        p->floorY = floorY;
        p->vx = vx;
        p->vy = vy;
        p->vz = vz;
        p->buffer = 1;
        p->unk28 = 0.0f;
        sDropsInUse++;
    }
    return p;
}

void func_m001_002A0(M001Particle *p, s32 updateRate) {
    f32 size;
    s16 y;
    Vertex *v;

    runlink_unload_timer_set(1, 50, 10);
    if (p->state == 1) {
        while (updateRate--) {
            p->x += p->vx;
            p->y += p->vy;
            p->z += p->vz;
            p->vy -= 0.3f;
            if (p->y < p->floorY) {
                p->y = p->floorY;
                p->state = 2;
                break;
            }
        }
    }
    if (p->state == 2) {
        while (updateRate--) {
            p->unk28 += 0.5f;
            p->alpha -= 2;
            if (p->alpha == 0) {
                p->state = 0;
                sDropsInUse--;
                break;
            }
        }
        if (p->state != 0) {
            p->buffer = 1 - p->buffer;
            v = p->verts[p->buffer];
            size = p->scale * (24.0f - 0.1875f * p->alpha);
            y = p->floorY;
            v->x = p->x - size;
            v->y = y;
            v->z = p->z + size;
            v++;
            v->x = p->x + size;
            v->y = y;
            v->z = p->z + size;
            v++;
            v->x = p->x - size;
            v->y = y;
            v->z = p->z - size;
            v++;
            v->x = p->x + size;
            v->y = y;
            v->z = p->z - size;
        }
    }
}

void func_m001_00600(s32 updateRate) {
    M001Particle *p;
    s32 i;

    if (sDropsInUse != 0) {
        p = sDropPool;
        for (i = 0; i < 32; i++, p++) {
            if (p->state != 0) {
                func_m001_002A0(p, updateRate);
            }
        }
    }
}

/* Draws one particle (module m001, 0x00678). */
static void drop_render(M001Particle *p, Gfx **dl, s32 arg2, s32 arg3) {
    f32 scale;
    M001Env *env;

    if ((p->state == 1 && D_m001_00C50 != NULL) || p->state == 2) {
        env = func_8004470C();
        scale = p->scale;
        if (p->state != 2) {
            func_8005600C(255, 255, 255, env->r, env->g, env->b);
            p->scale = scale;
            func_80041174(dl, arg2, arg3, p, D_m001_00C50, 6, 255);
            func_80056064();
        } else {
            func_80055C00(dl, sSplashTexture, 14, 0);
            if (p->alpha < 32) {
                gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, p->alpha * 8);
            }
            gSPVertexJFG((*dl)++, OS_K0_TO_PHYSICAL(p->verts[p->buffer]), 5, 0);
            gSPPolygon((*dl)++, OS_K0_TO_PHYSICAL(sSplashTris), 2, 1);
            if (D_m001_00C54 != NULL && p->unk28 < *(u8 *) D_m001_00C54) {
                p->scale = scale * 0.6f;
                gDPSetPrimColor((*dl)++, 0, 0, env->r, env->g, env->b, 160);
                gDPSetEnvColor((*dl)++, 255, 255, 255, 0);
                func_80041174(dl, arg2, arg3, p, D_m001_00C54, 14, 0);
            }
        }
        p->scale = scale;
        gDPPipeSync((*dl)++);
        gDPSetEnvColor((*dl)++, 255, 255, 255, 255);
        gDPSetPrimColor((*dl)++, 0, 0, 255, 255, 255, 255);
    }
}

/* Draws the particles in use, nearest to the camera first */
void func_m001_00978(Gfx **dl, s32 arg1, s32 arg2) {
    s32 i;
    s32 sorted;
    f32 unused; /* keeps the frame layout */
    s32 count;
    s32 order[32];
    s32 ti;
    ObjectTransform *cam;
    M001Particle *p;
    f32 dist[32];
    f32 dx;
    f32 dy;
    f32 dz;

    if (sDropsLoaded != 0 && sDropsInUse != 0) {
        cam = func_80041DE0();
        for (i = 0, count = 0, p = sDropPool; i < 32; i++, p++) {
            if (p->state != 0) {
                dx = p->x - cam->x_position;
                dy = p->y - cam->y_position;
                dz = p->z - cam->z_position;
                order[count] = i;
                dist[count] = dx * dx + dy * dy + dz * dz;
                count++;
            }
        }
        if (count >= 2) {
            do {
                sorted = 1;
                for (i = 0; i < count - 1; i++) {
                    if (dist[i + 1] < dist[i]) {
                        dx = dist[i];
                        dist[i] = dist[i + 1];
                        dist[i + 1] = dx;
                        ti = order[i];
                        order[i] = order[i + 1];
                        order[i + 1] = ti;
                        sorted = 0;
                    }
                }
            } while (!sorted);
        }
        for (i = 0; i < count; i++) {
            drop_render(&sDropPool[order[i]], dl, arg1, arg2);
        }
    }
}

/* The module's shutdown function (runlink fini) */
void func_m001_00C0C(void) {
    func_m001_00120();
}
#pragma weak __rl_fini = func_m001_00C0C

