/*
 * Runlink module 80: a small flapping object drawn from its own six vertices
 * and eight textured triangles (two vertex buffers, both set up from the
 * same corner positions). Init copies the triangles' vertex indices once and
 * masks their texture coordinates to the size of the chosen texture; control
 * (002A0, with its helper 01324) moves and animates it.
 */
#include "module.h"
#include "gfx.h"

/* What init gets */
typedef struct M080Setup {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ u8 textureIndex;
    /* 0x0D */ u8 scale; /* in hundredths */
} M080Setup;

/* The first bytes of a texture */
typedef struct M080Texture {
    /* 0x0 */ u8 width;
    /* 0x1 */ u8 height;
} M080Texture;

/* What obj->unk68 points at */
typedef struct M080State {
    /* 0x000 */ Triangle tris[8];
    /* 0x080 */ Vertex verts[2][6];
    /* 0x0F8 */ M080Texture *texture;
    /* 0x0FC */ u8 unkFC;
    /* 0x0FD */ u8 unkFD;
    /* 0x0FE */ u8 unkFE;
    /* 0x100 */ Object *target;
    /* 0x104 */ u16 unk104;
    /* 0x106 */ s16 unk106;
    /* 0x108 */ f32 unk108;
    /* 0x10C */ f32 unk10C;
    /* 0x110 */ s32 unk110;
    /* 0x114 */ s32 unk114;
} M080State;

/* .data (asm): the triangles, then the six corner positions */
extern Triangle rl_m080_01490[8];
extern s16 rl_m080_01510[6][3];
/* .rodata (asm): 0.01f, the scale unit */
extern f32 rl_m080_01540[];

#ifdef NON_MATCHING
/* 36 diffs: pos/dest pointer registers in the corner loop, and the scale multiply's load order (0.01f read from the asm .rodata) */
void func_m080_00000(Object *obj, M080Setup *setup, s32 reinit) {
    M080State *s = obj->unk68;
    s32 i;
    s32 uMask;
    s32 vMask;
    f32 scale;
    s16 *pos;

    if (!reinit) {
        obj->velocity.y = 0.0f;
        s->unkFE = 0;
        s->target = NULL;
        s->unk104 = 0;
        s->unkFD = 0;
        s->unk106 = 0x180;
        s->unk108 = 0.0f;
        for (i = 0; i < 8; i++) {
            s->tris[i].flags = rl_m080_01490[i].flags;
            s->tris[i].vi0 = rl_m080_01490[i].vi0;
            s->tris[i].vi1 = rl_m080_01490[i].vi1;
            s->tris[i].vi2 = rl_m080_01490[i].vi2;
        }
        pos = rl_m080_01510[0];
        i = 0;
        while (TRUE) {
            s->verts[0][i].x = pos[0];
            s->verts[0][i].y = pos[1];
            s->verts[0][i].z = pos[2];
            s->verts[0][i].r = 0xFF;
            s->verts[0][i].g = 0xFF;
            s->verts[0][i].b = 0xFF;
            s->verts[0][i].a = 0xFF;
            s->verts[1][i].x = pos[0];
            s->verts[1][i].y = pos[1];
            s->verts[1][i].z = pos[2];
            s->verts[1][i].r = 0xFF;
            s->verts[1][i].g = 0xFF;
            s->verts[1][i].b = 0xFF;
            s->verts[1][i].a = 0xFF;
            pos += 3;
            if (++i == 6) {
                break;
            }
        }
        s->unkFC = 1;
    }
    uMask = 0;
    scale = setup->scale;
    obj->trans.scale = scale * rl_m080_01540[0];
    if (setup->textureIndex < obj->header->numberOfModelIds) {
        s->texture = obj->modelInstances[setup->textureIndex];
    } else {
        s->texture = obj->modelInstances[0];
    }
    if (s->texture != NULL) {
        uMask = (s->texture->width - 1) << 5;
        vMask = (s->texture->height - 1) << 5;
    } else {
        vMask = 0;
    }
    for (i = 0; i < 8; i++) {
        s->tris[i].uv[0][0] = rl_m080_01490[i].uv[0][0] & uMask;
        s->tris[i].uv[0][1] = rl_m080_01490[i].uv[0][1] & vMask;
        s->tris[i].uv[1][0] = rl_m080_01490[i].uv[1][0] & uMask;
        s->tris[i].uv[1][1] = rl_m080_01490[i].uv[1][1] & vMask;
        s->tris[i].uv[2][0] = rl_m080_01490[i].uv[2][0] & uMask;
        s->tris[i].uv[2][1] = rl_m080_01490[i].uv[2][1] & vMask;
    }
    s->unk10C = 1.0f;
    s->unk110 = 0x100;
    s->unk114 = 0x100;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m080/ro", func_m080_00000);
#endif

#ifdef NON_MATCHING
/* 613 diffs: the frame is 8 bytes bigger (two extra temp words below the declared block), so every sp offset moves */
/* What a target's unk4C points at, as used here */
typedef struct M080Pos {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} M080Pos;

/* The level entry, as control reads it */
typedef struct M080Entry {
    /* 0x00 */ u8 pad0[4];
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u16 range;
    /* 0x0C */ u8 kind; /* 2: big */
} M080Entry;

#define TARGET(s) ((M080Pos *) (s)->target->interactObj)

extern s32 obj_dist_racer(f32 x, f32 y, f32 z, f32 range, s32 arg4, Object **found); /* finds objects near a point */
extern void func_800099F4();
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern f32 func_80097140(f32 x); /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z); /* arctangent as a 16-bit angle */
Object *func_m080_01324(f32 x, f32 y, f32 z, f32 range, s32 flat);

/* Flies about its home, to targets nearby, and flaps */
void func_m080_002A0(Object *obj, s32 updateRate) {
    f32 dx;
    f32 dy;
    f32 dz;
    s16 d;
    f32 range;
    s16 n;
    s32 b;
    f32 speed;
    f32 maxSpeed;
    f32 amp;
    f32 lim;
    s16 turn;
    s32 a;
    UNUSED s32 pad60;
    s32 big;
    Object *found[8];
    Vertex *v;
    M080State *s;
    M080Entry *setup;

    setup = (M080Entry *) obj->level_entry;
    s = obj->unk68;
    big = setup->kind == 2;
    turn = s->unk106;
    if (turn < 0) {
        turn = -turn;
    }
    turn *= updateRate;
    speed = 0.1f;
    if (big) {
        speed *= 1.6f;
    }
    maxSpeed = 3.0f;
    if (big) {
        maxSpeed *= 1.6f;
    }
    switch (s->unkFD) {
        case 0:
            s->unk108 = 0.0f;
            range = setup->range;
            if ((n = obj_dist_racer(setup->x, 0.0f, setup->z, range, 1, found)) > 0) {
                s->unk104 = 0xF0;
                s->unkFD = 3;
                s->target = found[0];
            } else if ((s->target = func_m080_01324(setup->x, 0.0f, setup->z, range, 1)) != NULL) {
                s->unk104 = 0xF0;
                s->unkFD = 3;
            }
            obj->unk28 += updateRate * 0.004f;
            if (obj->unk28 >= 1.0f) {
                obj->unk28 -= 1.0f;
            }
            if (big) {
                obj->unk28 = 0.48f;
            }
            break;
        case 1:
        case 2:
            if (s->unk106 < 0x480) {
                s->unk106 += updateRate << 4;
                if (s->unk106 > 0x480) {
                    s->unk106 = 0x480;
                }
            }
            dx = obj->trans.x_position - setup->x;
            dz = obj->trans.z_position - setup->z;
            d = func_80049C84(dx, dz) - obj->trans.rotation.y_rotation;
            if (d < 0) {
                if (-turn < d) {
                    obj->trans.rotation.y_rotation += d;
                } else {
                    obj->trans.rotation.y_rotation -= turn;
                }
            } else if (d > 0) {
                if (d < turn) {
                    obj->trans.rotation.y_rotation += d;
                } else {
                    obj->trans.rotation.y_rotation += turn;
                }
            }
            dy = setup->y - obj->trans.y_position;
            if (dy < 0.0f) {
                obj->velocity.y -= speed * updateRate;
                if (obj->velocity.y < dy) {
                    obj->velocity.y = dy;
                }
            } else if (dy > 0.0f) {
                obj->velocity.y += speed * updateRate;
                if (dy < obj->velocity.y) {
                    obj->velocity.y = dy;
                }
            } else {
                obj->velocity.y = 0.0f;
            }
            if (s->unkFD == 1) {
                range = setup->range;
                if ((n = obj_dist_racer(setup->x, 0.0f, setup->z, range, 1, found)) > 0) {
                    s->unk104 = 0x78;
                    s->unkFD = 3;
                    s->target = found[0];
                } else if ((s->target = func_m080_01324(setup->x, 0.0f, setup->z, range, 1)) != NULL) {
                    s->unk104 = 0xF0;
                    s->unkFD = 3;
                }
            }
            if (s->unkFD != 3) {
                dy = obj->trans.y_position - setup->y;
                if (func_80097140(dx * dx + dy * dy + dz * dz) < 4.0f) {
                    func_800099F4();
                    func_80009A04(obj, -dx, -dy, -dz);
                    obj->velocity.y = 0.0f;
                    s->unkFD = 0;
                    s->unk108 = 0.0f;
                }
            }
            obj->unk28 += 0.064f * updateRate;
            if (obj->unk28 >= 1.0f) {
                obj->unk28 -= 1.0f;
            }
            break;
        case 3:
            if (s->unk106 < 0x480) {
                s->unk106 += updateRate << 4;
                if (s->unk106 > 0x480) {
                    s->unk106 = 0x480;
                }
            }
            dx = TARGET(s)->x - setup->x;
            dz = TARGET(s)->z - setup->z;
            if ((f32) (setup->range * 2) * (f32) (setup->range * 2) < dx * dx + dz * dz) {
                s->target = NULL;
                range = setup->range;
                if ((n = obj_dist_racer(setup->x, 0.0f, setup->z, range, 1, found)) > 0) {
                    s->unk104 = 0xF0;
                    s->target = found[0];
                } else if ((s->target = func_m080_01324(setup->x, 0.0f, setup->z, range, 1)) != NULL) {
                    s->unk104 = 0xF0;
                }
            }
            if (s->target != NULL) {
                dx = obj->trans.x_position - TARGET(s)->x;
                dz = obj->trans.z_position - TARGET(s)->z;
                d = func_80049C84(dx, dz) - obj->trans.rotation.y_rotation;
                if (d < 0) {
                    if (-turn < d) {
                        obj->trans.rotation.y_rotation += d;
                    } else {
                        obj->trans.rotation.y_rotation -= turn;
                    }
                } else if (d > 0) {
                    if (d < turn) {
                        obj->trans.rotation.y_rotation += d;
                    } else {
                        obj->trans.rotation.y_rotation += turn;
                    }
                }
                dy = TARGET(s)->y - obj->trans.y_position;
                if (dy < 0.0f) {
                    obj->velocity.y -= speed * updateRate;
                    if (obj->velocity.y < dy) {
                        obj->velocity.y = dy;
                    }
                } else if (dy > 0.0f) {
                    obj->velocity.y += speed * updateRate;
                    if (dy < obj->velocity.y) {
                        obj->velocity.y = dy;
                    }
                }
                if (dx * dx + dz * dz < 256.0f) {
                    s->unkFD = 4;
                }
            } else {
                s->unkFD = 1;
            }
            obj->unk28 += 0.064f * updateRate;
            if (obj->unk28 >= 1.0f) {
                obj->unk28 -= 1.0f;
            }
            break;
        case 4:
            if (s->unk106 > 0x180) {
                s->unk106 -= updateRate << 4;
                if (s->unk106 < 0x180) {
                    s->unk106 = 0x180;
                }
            }
            dx = obj->trans.x_position - setup->x;
            dz = obj->trans.z_position - setup->z;
            if ((f32) (setup->range * 2) * (f32) (setup->range * 2) < dx * dx + dz * dz) {
                s->unkFD = 2;
            } else {
                if (updateRate < s->unk104) {
                    s->unk104 -= updateRate;
                } else {
                    s->unk104 = 0;
                    n = obj_dist_racer(obj->trans.x_position, 0.0f, obj->trans.z_position, 150.0f, 1, found);
                    if (n == 0) {
                        s->target = NULL;
                        s->unkFD = 1;
                    } else {
                        if (n >= 2) {
                            n = func_80048E2C(1, n);
                        }
                        s->unk104 = 0xF0;
                        s->target = found[n - 1];
                    }
                }
                if (s->unkFD == 4) {
                    dx = obj->trans.x_position - TARGET(s)->x;
                    dz = obj->trans.z_position - TARGET(s)->z;
                    if (22500.0f < dx * dx + dz * dz) {
                        s->unkFD = 3;
                    } else {
                        if (func_80048E2C(0, 100) >= 99) {
                            s->unk106 = -s->unk106;
                        }
                        obj->trans.rotation.y_rotation += updateRate * s->unk106;
                        if (TARGET(s)->y + 40.0f < obj->trans.y_position) {
                            s->unkFE = 0;
                        } else if (obj->trans.y_position < TARGET(s)->y) {
                            s->unkFE = 1;
                        }
                        if (s->unkFE) {
                            obj->velocity.y += speed * updateRate;
                        } else {
                            obj->velocity.y -= speed * updateRate;
                        }
                        obj->trans.rotation.y_rotation += s->unk106;
                    }
                }
            }
            obj->unk28 += 0.064f * updateRate;
            if (obj->unk28 >= 1.0f) {
                obj->unk28 -= 1.0f;
            }
            break;
        case 5:
            obj->particleEmittersEnabled = 1;
            func_8005F5AC(obj, updateRate);
            s->unk10C += 0.2f * updateRate;
            s->unk110 -= updateRate * 24;
            if (s->unk110 < 0x48) {
                s->unk110 = 0x48;
                s->unk114 -= updateRate << 5;
                if (s->unk114 <= 0) {
                    s->unk114 = 0;
                    func_800079B8(obj);
                }
            }
            break;
    }
    if (s->unkFD != 0 && s->unk108 < maxSpeed) {
        s->unk108 += 0.25f * updateRate;
        if (maxSpeed < s->unk108) {
            s->unk108 = maxSpeed;
        }
    }
    if (s->unk108 != 0.0f && s->unkFD != 5) {
        if (s->unkFD != 4) {
            lim = 1.2f;
        } else {
            lim = 1.6f;
        }
        if (big) {
            lim *= 1.6f;
        }
        if (obj->velocity.y < -lim) {
            obj->velocity.y = -lim;
        }
        if (lim < obj->velocity.y) {
            obj->velocity.y = lim;
        }
        if (s->unk108 != 0.0f) {
            obj->velocity.x = func_80049C34(obj->trans.rotation.y_rotation) * -s->unk108;
            obj->velocity.z = func_80049C30(obj->trans.rotation.y_rotation) * -s->unk108;
            func_80009A04(obj, obj->velocity.x, obj->velocity.y, obj->velocity.z);
        }
    }
    s->unkFC = 1 - s->unkFC;
    v = s->verts[s->unkFC];
    if (!big) {
        amp = 32768.0f;
    } else {
        amp = 65536.0f;
    }
    a = func_80049C34(obj->unk28 * amp) * 64.0f;
    b = func_80049C30(obj->unk28 * amp) * 64.0f;
    if (a < 0) {
        a = -a;
    }
    n = b;
    if (!big && b < 0) {
        n = -b;
    }
    v[0].x = -a;
    v[0].y = n;
    v[1].x = -a;
    v[1].y = n;
    v[4].x = a;
    v[4].y = n;
    v[5].x = a;
    v[5].y = n;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m080/ro", func_m080_002A0);
#endif

extern f32 func_80097140(f32 x); /* sqrtf */

/* The first object of behaviour 0x32 within `range` of the point (ignoring height if `flat`), or NULL */
Object *func_m080_01324(f32 x, f32 y, f32 z, f32 range, s32 flat) {
    s32 i;
    s32 count;
    Object **objs;
    Object *obj;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;

    objs = func_80005E98(&i, &count);
    while (i < count) {
        obj = objs[i++];
        if (obj->behaviorId == 0x32) {
            dx = obj->trans.x_position - x;
            dz = obj->trans.z_position - z;
            if (!flat) {
                dy = obj->trans.y_position - y;
                dist = func_80097140(dx * dx + dy * dy + dz * dz);
            } else {
                dist = func_80097140(dx * dx + dz * dz);
            }
            if (dist < range) {
                return obj;
            }
        }
    }
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/modules/m080", data);

/* (.rodata: C, and the assembly functions' own from ro/) */
