/*
 * Runlink module 8: an effect library. It keeps a pool of 64 drops that fly,
 * bounce off the level (with a sound) and leave a mark, and a pool of 5
 * sprites drawn at hit points; other modules add to the pools, and the
 * module updates and draws them.
 * Family: effect libraries (docs/runlink.md, "Module families").
 */
#include "module.h"
#include "gfx.h"

/* A drop (size 0x54) */
typedef struct M008Drop {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 prevX;
    /* 0x10 */ f32 prevY;
    /* 0x14 */ f32 prevZ;
    /* 0x18 */ f32 vx;
    /* 0x1C */ f32 vy;
    /* 0x20 */ f32 vz;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 scale;
    /* 0x2C */ f32 nx; /* the surface it hit */
    /* 0x30 */ f32 ny;
    /* 0x34 */ f32 nz;
    /* 0x38 */ f32 unk38;
    /* 0x3C */ s16 timer;
    /* 0x3E */ s16 corners[2][3]; /* the mark's two corners, turned to the surface */
    /* 0x4A */ u8 state; /* 0: free, 1: falling, 2: landed, 3: fading */
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ s16 unk4C;
    /* 0x4E */ s16 unk4E;
    /* 0x50 */ u8 unk50;
    /* 0x51 */ u8 unk51;
    /* 0x52 */ u8 mark; /* landed on the level: draw a mark */
    /* 0x53 */ u8 pad53;
} M008Drop;

/* What func_80018808 fills in, as this module reads it (one word more than HitResult) */
typedef struct M008Contact {
    /* 0x00 */ Object *obj;
    /* 0x04 */ Vec3f pos;
    /* 0x10 */ Vec3f normal;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ u8 pad20[4];
    /* 0x24 */ u32 flags;
    /* 0x28 */ u32 kind;
} M008Contact;

/* The level model, as far as this module uses it */
typedef struct M008Level {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ s16 minY;
} M008Level;

/* A sprite at a hit point (size 0x18) */
typedef struct M008Hit {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 state; /* 0: free */
    /* 0x17 */ u8 pad17;
} M008Hit;

extern void *func_80055694(s32 id); /* loads a texture */
extern void func_80055A00(void *tex);
extern void *func_80056798(s32 id, s32 arg1);
extern void func_80056C14(void *sprite);
extern u8 D_800A5080;

/* The module's data and bss, still assembly */
extern s32 rl_m008_016C0;     /* set once the textures are loaded */
extern s32 rl_m008_017A0;     /* drops in use */
extern s32 rl_m008_017A4;     /* hit sprites in use */
extern void *rl_m008_017A8[5]; /* textures */
extern void *rl_m008_017BC;   /* sprite */
extern M008Drop rl_m008_017C0[64];
extern M008Hit rl_m008_02CC0[5];
extern s32 rl_m008_02D38;

#define sLoaded rl_m008_016C0
#define sDropCount rl_m008_017A0
#define sHitCount rl_m008_017A4
#define sTextures rl_m008_017A8
#define sSprite rl_m008_017BC
#define sDrops rl_m008_017C0
#define sHits rl_m008_02CC0
#define sUnk2D38 rl_m008_02D38

/*
 * The 64-drop clearing loops (00000 and 000C4): `while (TRUE) { ...; if (i < 64) continue; break; }`
 * keeps the ROM's plain loop on the counter (a `for` is unrolled four times), the store after
 * `p++` gives its negative offset, and `i = 0; p = sDrops;` on one line its register order.
 */
/* Loads the textures and clears the pools (module m008, 0x00000). */
static void fxdrops_pools_init(void) {
    s32 i;
    M008Drop *p;

    sTextures[0] = func_80055694(0x170);
    sTextures[1] = func_80055694(0x171);
    sTextures[2] = func_80055694(0x172);
    sTextures[3] = func_80055694(0x173);
    sTextures[4] = func_80055694(0x174);
    sSprite = func_80056798(0x43, 1);
    i = 0; p = sDrops;
    while (TRUE) {
        i++;
        p++;
        p[-1].state = 0;
        if (i < 64) {
            continue;
        }
        break;
    }
    sHits[0].state = 0;
    sLoaded = 1;
    sDropCount = 0;
    sHitCount = 0;
    sUnk2D38 = 0;
}

void func_m008_000C4(void) {
    s32 i;
    M008Drop *p;

    if (sLoaded) {
        if (sTextures[0] != NULL) {
            func_80055A00(sTextures[0]);
        }
        if (sTextures[1] != NULL) {
            func_80055A00(sTextures[1]);
        }
        if (sTextures[2] != NULL) {
            func_80055A00(sTextures[2]);
        }
        if (sTextures[3] != NULL) {
            func_80055A00(sTextures[3]);
        }
        if (sTextures[4] != NULL) {
            func_80055A00(sTextures[4]);
        }
        if (sSprite != NULL) {
            func_80056C14(sSprite);
        }
        i = 0; p = sDrops;
        while (TRUE) {
            i++;
            p++;
            p[-1].state = 0;
            if (i < 64) {
                continue;
            }
            break;
        }
        sHits[0].state = 0;
    }
    sLoaded = 0;
    sDropCount = 0;
    sHitCount = 0;
}

void func_m008_001CC(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, s32 angle, s32 colour, f32 scale) {
    M008Drop *p;
    s32 i;

    if (!sLoaded) {
        fxdrops_pools_init();
    }
    i = 0;
    if (sDropCount < 64) {
        for (p = sDrops; i < 64; i++, p++) {
            if (p->state == 0) {
                break;
            }
        }
        if (i < 64) {
            p->vx = vx;
            p->vy = vy;
            p->vz = vz;
            p->scale = scale;
            p->state = 1;
            p->timer = 300;
            p->prevX = x;
            p->prevY = y;
            p->unk24 = 0.0f;
            p->prevZ = z;
            p->x = x;
            p->y = y;
            p->z = z;
            p->unk4B = func_80048E2C(0, 1);
            p->unk4E = p->unk4C = angle << 5;
            if (D_800A5080) {
                p->unk50 = func_80048E2C(0, 7);
            } else {
                p->unk50 = colour;
            }
            p->unk51 = func_80048E2C(0, 2);
            sDropCount++;
        }
    }
}

void func_m008_00334(f32 x, f32 y, f32 z, s32 arg3, s32 arg4) {
    M008Hit *h;
    s32 i;

    if (!sLoaded) {
        fxdrops_pools_init();
    }
    i = 0;
    if (sHitCount < 5) {
        for (h = sHits; i < 5; i++, h++) {
            if (h->state == 0) {
                break;
            }
        }
        if (i < 5) {
            h->x = x;
            h->y = y;
            h->z = z;
            h->unk14 = arg3;
            h->unk15 = arg4;
            h->state = 1;
            h->unkC = 0.0f;
            sHitCount++;
        }
    }
}

extern s32 func_80049C84(f32 x, f32 z); /* the angle of a direction */
extern f32 func_80097140(f32 x);        /* sqrtf */
extern M008Level *func_8001B650(void);  /* the level model */
extern s32 sprite_anim_advance(void *sprite, s32 *flags, u32 speed, f32 *frame, u32 updateRate);

/*
 * Updates the pools: drops fall (and vanish below the level), test every other frame (odd and even drops in
 * turn) for a hit on the level, then stick there with a sound, a random size and, on flat ground, a mark
 * turned to the surface; the mark animates, then fades out on its timer. The hit sprites animate and free
 * themselves. (The mark's corners are (10, -10) and (10, 10) turned by a random roll, then to the surface.)
 */
void func_m008_0040C(s32 updateRate) {
    UNUSED s32 pad[2];
    f32 rate;
    f32 floor;
    M008Drop *p;
    s32 i;
    s16 pitch;
    s16 yaw;
    s16 roll;
    f32 cosR;
    f32 sinR;
    f32 cosP;
    f32 sinP;
    f32 cosY;
    f32 sinY;
    M008Contact hit;
    s32 flags;
    f32 x;
    f32 y;
    UNUSED f32 pad2;
    f32 b;
    M008Hit *h;

    rate = updateRate;
    flags = 1;
    floor = func_8001B650()->minY - 1000.0f;
    for (i = 0, p = sDrops; i < 64; i++, p++) {
        switch (p->state) {
            case 1:
                p->y += p->vy * rate;
                if (p->y < floor) {
                    p->state = 0;
                    sDropCount--;
                    break;
                }
                p->x += p->vx * rate;
                p->z += p->vz * rate;
                p->vy -= 0.1f * updateRate;
                if (((i & 1) && sUnk2D38) || (!(i & 1) && !sUnk2D38)) {
                    if (func_80018808(NULL, (Vec3f *) &p->prevX, (Vec3f *) &p->x, (HitResult *) &hit, 0x2000, 0)) {
                        p->nx = hit.normal.x;
                        p->ny = hit.normal.y;
                        p->nz = hit.normal.z;
                        p->unk38 = hit.unk1C;
                        p->x = hit.pos.x;
                        p->y = hit.pos.y;
                        p->z = hit.pos.z;
                        p->unk4E = (0xFF - (((hit.flags >> 22) & 7) << 5)) << 5;
                        p->scale *= 2.0f + (func_80048E2C(0, 255) - 128) * 0.0078f;
                        func_80003410(func_80048E2C(0, 3) + 0xAA, p->x, p->y, p->z, 4, NULL);
                        if (hit.obj == NULL && hit.kind != 3) {
                            pitch = func_80049C84(hit.normal.y, func_80097140(hit.normal.z * hit.normal.z +
                                                                               hit.normal.x * hit.normal.x)) - 0x4000;
                            yaw = func_80049C84(-hit.normal.x, -hit.normal.z);
                            roll = func_80048E2C(-0x8000, 0x7FFF);
                            cosR = func_80049C30(roll);
                            sinR = func_80049C34(roll);
                            cosP = func_80049C30(-pitch);
                            sinP = func_80049C34(-pitch);
                            cosY = func_80049C30(yaw);
                            sinY = func_80049C34(yaw);
                            x = (-10.0f * cosR + 10.0f * sinR) * p->scale;
                            y = (10.0f * cosR - -10.0f * sinR) * p->scale;
                            p->corners[0][1] = y * sinP;
                            b = y * cosP;
                            p->corners[0][0] = x * cosY + b * sinY;
                            p->corners[0][2] = b * cosY - x * sinY;
                            x = (10.0f * cosR + 10.0f * sinR) * p->scale;
                            y = (10.0f * cosR - 10.0f * sinR) * p->scale;
                            p->corners[1][1] = y * sinP;
                            b = y * cosP;
                            p->corners[1][0] = x * cosY + b * sinY;
                            p->corners[1][2] = b * cosY - x * sinY;
                            p->mark = 1;
                        } else {
                            p->mark = 0;
                        }
                        p->state = 2;
                    } else {
                        p->prevX = p->x;
                        p->prevY = p->y;
                        p->prevZ = p->z;
                    }
                }
                break;
            case 2:
                p->unk4C += (p->unk4E - p->unk4C) >> 1;
                if (sprite_anim_advance(sSprite, &flags, 12, &p->unk24, updateRate)) {
                    p->state = 3;
                }
                break;
            case 3:
                p->timer -= updateRate;
                if (p->timer < 0) {
                    p->state = 0;
                    sDropCount--;
                }
                break;
        }
    }
    sUnk2D38 ^= 1;
    flags = 1;
    for (i = 0, h = sHits; i < 5; i++, h++) {
        if (!h->state == FALSE) {
            if (sprite_anim_advance(sSprite, &flags, 15, &h->unkC, updateRate)) {
                h->state = 0;
                sHitCount--;
            }
        }
    }
}

extern Triangle rl_m008_01660[]; /* .data: a drop's streak (two triangles; [2]: the other one) */
extern Triangle rl_m008_016A0[]; /* .data: a mark's two triangles */
extern s32 rl_m008_016C4[];      /* .data: the drops' tints, three per colour */
extern s32 rl_m008_01724[];      /* .data: the hit sprites' tints, three per colour */
extern f32 get_distance_to_camera(f32 x, f32 y, f32 z);
/* (40240.c's definition takes s16 positions and roll: the ROM passes them unconverted) */
extern void render_sprite_billboard_at(Gfx **dl, Mtx **mtx, Vertex **verts, void *sprite, s32 x, s32 y, s32 z, s32 roll,
                                       f32 scale, f32 frame, s32 flags, u8 alpha);
extern void draw_tint_pair_set(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2);
extern void draw_tint_pair_off(void);
extern void func_8000A300(Gfx **dl, Vertex **vtx, void *tex, Triangle *tris, Vec3f *p1, Vec3f *p2, f32 width,
                          s32 primColour, s32 envColour, s32 flags); /* draws a streak from p2 to p1 */
extern void func_80055C00(Gfx **dl, void *tex, s32 flags, s32 offset); /* sets a texture */

#ifdef NON_MATCHING
/*
 * First C body (726/746 words, frame 0x138/0x140): the ROM also keeps 2.0f in $f30, gives p/0xFF the other
 * two of s0/s1, re-reads the FPU control word for every vertex conversion and stores one vertex's colours
 * in reverse order without reloading vtx.
 */
/*
 * Draws the pools: each drop's mark (two triangles turned to the surface, lifted towards the camera, fading on
 * its timer), each falling drop as a streak along its velocity, each landed drop and each hit as a tinted sprite.
 */
void func_m008_00AB4(Gfx **dlp, Mtx **mtxp, Vertex **vtxp) {
    f32 dist;
    f32 offset;
    s32 alpha;
    s32 i;
    s32 b;
    u32 colour1;
    u32 colour2;
    f32 x;
    f32 y;
    f32 z;
    f32 vx;
    Vec3f from;
    f32 vy;
    f32 vz;
    f32 speed;
    f32 k;
    M008Drop *p;
    M008Hit *h;
    s32 *tint;
    Triangle *tris;
    Vertex *v;
    u32 prim;
    u32 env;
    Vertex *vtx;
    s16 unused[8];
    Gfx *dl;
    Mtx *mtx;

    dl = *dlp;
    mtx = *mtxp;
    vtx = *vtxp;
    unused[0] = 0;
    for (i = 0, p = sDrops; i < 64; i++, p++) {
        if (p->state != 0) {
            b = p->unk4C;
            colour1 = (((b * 255) >> 5) & 0xFF00) | ((((b * 255) >> 5) & 0xFF00) << 8) |
                      ((((b * 255) >> 5) & 0xFF00) << 16) | 0xFF;
            tint = &rl_m008_016C4[p->unk50 * 3];
            colour2 = (((b * tint[0]) << 11) & 0xFF000000) | (((b * tint[1]) << 3) & 0xFF0000) |
                      (((tint[2] * b) >> 5) & 0xFF00) | 0xFF;
        }
        if ((p->state == 2 || p->state == 3) && p->mark) {
            if (p->timer < 120) {
                alpha = (p->timer * 255) / 120;
            } else {
                alpha = 255;
            }
            if (sTextures[p->unk51 + 2] != NULL) {
                prim = colour1;
                env = colour2;
                prim &= ~0xFF;
                env &= ~0xFF;
                func_80055C00(&dl, sTextures[p->unk51 + 2], 0x203, 0);
                gDPPipeSync(dl++);
                gDPSetColor(dl++, G_SETPRIMCOLOR, prim | alpha);
                gDPSetColor(dl++, G_SETENVCOLOR, env | alpha);
                gSPVertexJFG(dl++, OS_K0_TO_PHYSICAL(vtx), 5, 0);
                gSPPolygon(dl++, OS_K0_TO_PHYSICAL(rl_m008_016A0), 2, 1);
                dist = -get_distance_to_camera(p->x, p->y, p->z);
                offset = dist;
                if (dist < 0.0f) {
                    offset = -dist;
                }
                offset -= 250.0f;
                if (offset < 0.0f) {
                    offset = 0.0f;
                }
                if (1024.0f < offset) {
                    offset = 1024.0f;
                }
                offset = 2.0f + offset * 0.01f;
                x = p->nx * offset + p->x;
                y = p->ny * offset + p->y;
                z = p->nz * offset + p->z;
                v = vtx;
                v->x = p->corners[0][0] + x;
                v->y = p->corners[0][1] + y;
                v->z = p->corners[0][2] + z;
                v++;
                v->x = p->corners[1][0] + x;
                v->y = p->corners[1][1] + y;
                v->z = p->corners[1][2] + z;
                v++;
                v->x = x - p->corners[1][0];
                v->y = y - p->corners[1][1];
                v->z = z - p->corners[1][2];
                v++;
                v->x = x - p->corners[0][0];
                v->y = y - p->corners[0][1];
                v->z = z - p->corners[0][2];
                vtx->r = 255;
                vtx->g = 255;
                vtx->b = 255;
                vtx->a = 255;
                vtx++;
                vtx->r = 255;
                vtx->g = 255;
                vtx->b = 255;
                vtx->a = 255;
                vtx++;
                vtx->r = 255;
                vtx->g = 255;
                vtx->b = 255;
                vtx->a = 255;
                vtx++;
                vtx->r = 255;
                vtx->g = 255;
                vtx->b = 255;
                vtx->a = 255;
                vtx++;
            }
        }
        switch (p->state) {
            case 1:
                vx = p->vx;
                vy = p->vy;
                vz = p->vz;
                speed = func_80097140(vx * vx + vy * vy + vz * vz);
                if (speed == 0.0f) {
                    k = 0.0f;
                } else {
                    k = 40.0f / speed;
                }
                k *= p->scale;
                vx *= k;
                from.x = p->x - vx;
                vy *= k;
                from.y = p->y - vy;
                vz *= k;
                from.z = p->z - vz;
                if (p->unk4B == 0) {
                    tris = rl_m008_01660;
                } else {
                    tris = &rl_m008_01660[2];
                }
                func_8000A300(&dl, &vtx, sTextures[p->unk4B], tris, (Vec3f *) p, &from, p->scale * 8.0f, colour1,
                              colour2, 0x200);
                break;
            case 2:
                tint = &rl_m008_016C4[p->unk50 * 3];
                draw_tint_pair_set(((b * 255) >> 13) & 0xFF, ((b * 255) >> 13) & 0xFF, ((b * 255) >> 13) & 0xFF,
                                   ((tint[0] * b) >> 13) & 0xFF, ((tint[1] * b) >> 13) & 0xFF,
                                   ((tint[2] * b) >> 13) & 0xFF);
                render_sprite_billboard_at(&dl, &mtx, &vtx, sSprite, p->x, p->y, p->z, 0, p->scale * 1.8f, p->unk24,
                                           14, 255);
                draw_tint_pair_off();
                break;
        }
    }
    for (i = 0, h = sHits; i < 5; i++, h++) {
        if (h->state) {
            b = h->unk14;
            tint = &rl_m008_01724[h->unk15 * 3];
            draw_tint_pair_set(((b * 255) >> 8) & 0xFF, ((b * 255) >> 8) & 0xFF, ((b * 255) >> 8) & 0xFF,
                               ((tint[0] * b) >> 8) & 0xFF, ((tint[1] * b) >> 8) & 0xFF, ((tint[2] * b) >> 8) & 0xFF);
            render_sprite_billboard_at(&dl, &mtx, &vtx, sSprite, h->x, h->y, h->z, 0, 4.0f, h->unkC, 14, 255);
        }
    }
    draw_tint_pair_off();
    gDPSetPrimColor(dl++, 0, 0, 255, 255, 255, 255);
    *dlp = dl;
    *mtxp = mtx;
    *vtxp = vtx;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m008/ro", func_m008_00AB4);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m008", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m008", bss);
