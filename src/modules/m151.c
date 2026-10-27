/*
 * Runlink module 151: a spider enemy that goes after the objects on module
 * 139's list (eating them, with a spray of particles) and otherwise
 * wanders inside its zone; its spit projectile; and the spit's animated
 * texture, drawn by drops that run down an 8-bit buffer that is blurred
 * into the other of two buffers every frame and converted to IA16.
 */
#include "module.h"

/* Its home object's data (that object's unk68) */
typedef struct M151Home {
    /* 0x00 */ u8 pad0[0x7A];
    /* 0x7A */ s8 zone;
} M151Home;

/* The part of module 3's enemy record that this enemy uses */
typedef struct M151Enemy {
    /* 0x00 */ u32 pad0_0 : 18;
    /* 0x00 */ u32 wandering : 1; /* has a wander point */
    /* 0x00 */ u32 pad0_1 : 13;
    /* 0x04 */ u8 pad4[0x1A - 0x4];
    /* 0x1A */ u8 mission;
    /* 0x1B */ u8 state;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ Object *home;
    /* 0x28 */ u8 pad28[0x38 - 0x28];
    /* 0x38 */ f32 x;
    /* 0x3C */ f32 y;
    /* 0x40 */ f32 z;
    /* 0x44 */ u8 pad44[0x4B - 0x44];
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ u8 pad4C[0x51 - 0x4C];
    /* 0x51 */ u8 timer;
    /* 0x52 */ u8 pad52[0x64 - 0x52];
    /* 0x64 */ Object *target;
} M151Enemy;

/* What the spider's interactObj points at */
typedef struct M151Interact {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ f32 radius;
} M151Interact;

typedef struct M151SpitSetup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 size;
    /* 0x0B */ u8 time; /* in tenths of a second */
    /* 0x0C */ s16 rotation[2]; /* y, x (what func_8004944C reads of its Vec3s) */
    /* 0x10 */ f32 speed;
    /* 0x14 */ s32 unk14;
} M151SpitSetup;

typedef struct M151Spit {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s16 life;
    /* 0x06 */ u8 size;
} M151Spit;

/* A drop on the spit texture; positions in 1/256 texels */
typedef struct M151Drop {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 vx;
    /* 0x06 */ s16 vy;
    /* 0x08 */ u8 pad8[0x2];
    /* 0x0A */ s16 life;
} M151Drop;

/* The texture: its size, then IA16 texels at 0x20 */
typedef struct M151Texture {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0x20 - 0x2];
    /* 0x20 */ u16 texels[1];
} M151Texture;

extern u8 *D_80102D60[]; /* the two 8-bit work buffers of each texture */
extern u8 *D_80102D78[];

extern void func_800541B0(s32, s32, s32);
extern Object *func_m139_00184(s16 x, s16 z, s32 radius, s32 zone);
extern s32 func_m139_0038C(Object *obj);
extern s32 func_m003_0CAA4(f32, f32, s32);
extern s32 func_m003_04260(s32, s32, s32, s32, s32);
extern void func_m003_0512C(s32, f32, s32);
extern void func_m003_04398(s32, s32);
extern void func_m003_04D08(s32, s32, s32, s32);
extern s32 func_m003_04F88(s32, f32);
extern Object *func_m003_053C4(void);
extern f32 func_m003_03E68(f32 x1, f32 z1, f32 x2, f32 z2);
extern s32 func_m003_17910(f32 x, f32 z, s32 zone);
extern s32 func_m003_057CC(f32 x, f32 z);
extern s32 func_m003_1794C(f32 x1, f32 z1, f32 x2, f32 z2, s32 zone, s32, s32, s32);
extern void func_8001191C(Object *obj, Object *target);
extern void func_80011974(Object *obj);
extern void *func_m008_001CC(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, s32 alpha, s32, f32 scale);
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle); /* cosine */
extern s32 func_80049C84(f32 dx, f32 dz); /* angle of a vector */
extern s32 func_80005064(s32);

s32 D_m151_00FE0 = 151; /* the module's number */

#ifdef NON_MATCHING
static M151Drop sDrops[16];
static u8 sDropTimer;
static u8 sNextDrop;
static u8 sFlip; /* which buffer holds the current frame */
#endif

void func_m151_00000(Object *obj) {
    M151Enemy *e = obj->unk68;
    Object *homeObj = e->home;
    M151Home *home = homeObj->unk68;
    Object *food;

    food = func_m139_00184(obj->trans.x_position, obj->trans.z_position, 800, home->zone);
    func_800541B0(D_m151_00FE0, 2, 2);
    if (food != NULL && func_m003_04260(4, 0x14, 0, 0, 0) != 0) {
        e->x = food->trans.x_position;
        e->z = food->trans.z_position;
        e->target = food;
        func_8001191C(obj, food);
    }
    if (e->mission != 0x14) {
        if (func_m003_0CAA4(0.0f, 400.0f, 0x17F0) != 0) {
            if (e->mission != 5 && func_m003_04260(4, 5, 0, 0, 0) != 0) {
                func_m003_0512C(6, 1.0f, 5);
            }
        }
    }
}

#ifdef NON_MATCHING
/*
 * Picks a new wander point when the way ahead is blocked or the point is
 * reached. Equivalent; only the register of the dead copy that IDO makes for
 * `i--` differs (s0 in the original, v0 here), which moves two delay slots.
 */
void func_m151_00154(Object *obj) {
    M151Enemy *e;
    Object *homeObj;
    M151Home *home;
    Object *player;
    s32 pad1;
    s32 pad2;
    s32 blocked;
    s32 i;
    s32 k;
    s32 d;
    s32 angle;
    f32 r;
    f32 x;
    f32 z;

    e = obj->unk68;
    homeObj = e->home;
    home = homeObj->unk68;
    player = func_m003_053C4();
    blocked = FALSE;
    i = 6;
    do {
        k = i - 3;
        angle = (s16) (obj->trans.rotation.y_rotation + (k << 12));
        if (k > 0) {
            d = k;
        } else {
            d = -k;
        }
        r = (5.0f - d) * ((M151Interact *) obj->interactObj)->radius;
        x = obj->trans.x_position - func_80049C34(angle) * r;
        z = obj->trans.z_position - func_80049C30(angle) * r;
        if (func_m003_17910(x, z, home->zone) == 0 || func_m003_057CC(x, z) == 0) {
            blocked = TRUE;
            break;
        }
    } while (i--);
    if (func_m003_03E68(obj->trans.x_position, obj->trans.z_position, e->x, e->z) < 600.0f) {
        blocked = TRUE;
        e->wandering = 0;
    }
    if (blocked) {
        if (!e->wandering) {
            i = 4;
            do {
                if (i == 4 && func_m003_17910(player->trans.x_position, player->trans.z_position, home->zone) != 0) {
                    d = func_80049C84(obj->trans.x_position - player->trans.x_position,
                                      obj->trans.z_position - player->trans.z_position);
                    angle = func_80048E2C(-6000, 6000) + d;
                } else {
                    angle = func_80048E2C(-0x8000, 0x7FFF);
                }
                r = func_80048E2C(200, 400);
                x = obj->trans.x_position - func_80049C34(angle) * r;
                z = obj->trans.z_position - func_80049C30(angle) * r;
                if (func_m003_1794C(obj->trans.x_position, obj->trans.z_position, x, z, home->zone, 0, -1, 0xFF) == 0) {
                    e->x = x;
                    e->z = z;
                    e->wandering = 1;
                    return;
                }
            } while (i--);
            e->x = obj->trans.x_position;
            e->z = obj->trans.z_position;
        }
    } else {
        e->wandering = 0;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m151", func_m151_00154);
#endif

/* Goes to the food, eats it (with particles), then goes back to the player */
void func_m151_00438(Object *obj, f32 *x, f32 *y, f32 *z) {
    M151Enemy *e = obj->unk68;
    Object *player;
    s32 alpha;
    s32 pad;
    s32 vy;
    s32 vx;

    player = func_m003_053C4();
    *x = e->x;
    *y = obj->trans.y_position;
    *z = e->z;
    if (func_m139_0038C(e->target) == 0) {
        e->state = 5;
    }
    switch (e->state) {
        case 1:
            if (func_m003_03E68(obj->trans.x_position, obj->trans.z_position, e->x, e->z) < 600.0f) {
                func_m003_04D08(3, 0x36, 0, 0);
                e->state = 4;
                func_80003410(0x105, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            }
            e->timer = 1;
            break;
        case 4:
            if (e->target->behaviorId != 0x4D && func_80048E2C(1, 7) == 1) {
                if (obj->shading != NULL) {
                    alpha = obj->shading->unk0 * 255.0f;
                } else {
                    alpha = 255;
                }
                vx = func_80048E2C(-3, 3);
                vy = func_80048E2C(1, 4);
                func_m008_001CC(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, vx, vy,
                                func_80048E2C(-3, 3), alpha, 1, 1.0f);
            }
            if (func_m003_04F88(3, 1.0f) != 0) {
                if (--e->timer == 0) {
                    func_800079B8(e->target);
                    e->state = 5;
                }
            }
            break;
        case 5:
            e->target = player;
            e->state = 9;
            /* fallthrough */
        case 9:
            if (func_m003_04F88(3, 1.0f) != 0 || e->unk4B != 3) {
                e->unk4B = 0;
                func_80011974(obj);
                func_m003_04398(1, 0);
            }
            break;
    }
}

/* The spit's init */
void func_m151_006E8(Object *obj, M151SpitSetup *setup) {
    M151Spit *s = obj->unk68;

    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    obj->velocity.z = -setup->speed;
    func_8004944C((Vec3s *) setup->rotation, &obj->velocity);
    func_80079D1C(obj, (setup->size + 1) << 8, 1, 1, 0, 0, 0, 16.0f, 16.0f, 1.0f, 0.75f, 0x11);
    s->life = func_80005064(setup->time * 6);
    s->size = setup->size;
    s->unk0 = setup->unk14;
    *obj->unk7C &= ~2;
}

/* The spit's control: it goes when its time is up or it hits something */
void func_m151_007F0(Object *obj, s32 updateRate) {
    M151Spit *s = obj->unk68;
    u8 *flags = obj->unk7C;

    s->life -= updateRate;
    if (s->life < 0 || (*flags & 2)) {
        func_800079B8(obj);
    }
}

#ifdef NON_MATCHING
/* Starts a drop at (x, y) in a random direction (matches; kept with its caller) */
static void add_drop(s32 x, s32 y) {
    M151Drop *d = &sDrops[sNextDrop++];
    s16 angle;
    s16 speed;

    angle = func_80048E2C(0, 0x10000);
    speed = func_80048E2C(150, 255);
    d->x = x << 7;
    d->y = y << 7;
    d->vx = func_80049C34(angle) * speed;
    d->vy = func_80049C30(angle) * speed;
    d->life = (y << 7) / speed;
    if (sNextDrop >= 16) {
        sNextDrop = 0;
    }
}

/*
 * Advances texture `index` one frame (restarting it if reset). Equivalent;
 * the original sign-extends the texel value more often in the IA16
 * conversion (the 4x unrolled loops come out 0xC0 bytes longer), and some
 * registers differ.
 */
void func_m151_009BC(s32 index, M151Texture *tex, s32 reset) {
    s32 w;
    s32 i;
    s32 j;
    s32 off;
    s32 h;
    u8 *dst;
    u8 *s;
    u8 *o;
    u16 *out;
    u16 *rowOut;
    M151Drop *d;
    s16 c;
    u8 *src;
    s16 a;

    w = tex->width;
    h = tex->height;
    if (sFlip) {
        src = D_80102D60[index];
    } else {
        src = D_80102D78[index];
    }
    if (reset) {
        sNextDrop = 0;
        i = 8;
        while (i--) {
            add_drop(w, h);
        }
        sDropTimer = 0;
        sFlip = 0;
    }
    if (++sDropTimer >= 3) {
        add_drop(w, h);
        sDropTimer = 0;
    }
    d = sDrops;
    i = 16;
    while (i--) {
        if (d->life != 0) {
            src[(d->y >> 8) * w + (d->x >> 8)] = d->life * 8;
            d->x += d->vx;
            d->y += d->vy;
            d->life--;
        }
        d++;
    }
    sFlip ^= 1;
    if (sFlip) {
        dst = D_80102D60[index];
    } else {
        dst = D_80102D78[index];
    }
    s = src + w + 1;
    o = dst + w + 1;
    i = h - 2;
    while (i--) {
        j = w - 2;
        while (j--) {
            *o++ = (s[w] + s[0] * 4 + s[-1] + s[1] + s[-w]) >> 3;
            s++;
        }
        o += 2;
        s += 2;
    }
    s = dst;
    out = tex->texels;
    i = h;
    while (i--) {
        rowOut = out;
        if (i & 1) {
            j = w;
            while (j--) {
                c = *s++;
                a = c;
                if ((s16) (c >> 4) != 0) {
                    a = c * (s16) (c >> 4);
                }
                if (a >= 0x100) {
                    a = 0xFF;
                }
                *out++ = a | (c << 8);
            }
        } else {
            for (j = 0; j < w; j++) {
                if (j & 2) {
                    off = -2;
                } else {
                    off = 2;
                }
                c = *s++;
                a = c;
                if ((s16) (c >> 4) != 0) {
                    a = c * (s16) (c >> 4);
                }
                if (a >= 0x100) {
                    a = 0xFF;
                }
                rowOut[off + j] = a | (c << 8);
            }
            out = rowOut + w;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m151", func_m151_0084C);

INCLUDE_ASM("asm/nonmatchings/modules/m151", func_m151_009BC);

INCLUDE_ASM("asm/nonmatchings/modules/m151", bss);
#endif
