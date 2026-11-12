/*
 * Runlink module 28: a beam that follows a track. Its object spawns the beam
 * object (the module's other behaviour) and every frame hands it its own
 * position, facing and strength, or turns it off while hidden.
 */
#include "module.h"

typedef struct M028Setup {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 kind; /* 10: spawned by another object, with the defaults */
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 yRotation;
    /* 0x0C */ s16 xRotation;
    /* 0x0E */ s16 strength;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 pad11[0x14 - 0x11];
    /* 0x14 */ Object *owner;
} M028Setup;

/* The level entry the beam object is spawned from. Size 0x2C */
typedef struct M028BeamEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 padA[0x10 - 0xA];
    /* 0x10 */ Object *owner;
    /* 0x14 */ s16 shape[3][3];
    /* 0x26 */ s16 strength;
    /* 0x28 */ u8 unk28;
    /* 0x29 */ u8 unk29;
    /* 0x2A */ u8 unk2A;
} M028BeamEntry;

/* A triangle of the beam's model (gfx.h's Triangle) */
typedef struct M028Tri {
    /* 0x0 */ u8 flags;
    /* 0x1 */ u8 vi0;
    /* 0x2 */ u8 vi1;
    /* 0x3 */ u8 vi2;
    /* 0x4 */ s16 uv[3][2];
} M028Tri;

/* A vertex of the beam's model (gfx.h's Vertex) */
typedef struct M028Vertex {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ u8 r;
    /* 0x7 */ u8 g;
    /* 0x8 */ u8 b;
    /* 0x9 */ u8 a;
} M028Vertex;

/* The beam's state (its object's unk68) */
typedef struct M028BeamState {
    /* 0x000 */ M028Tri tris[96];
    /* 0x600 */ u8 verts[0xA38 - 0x600]; /* the model's vertices (M028Vertex; init's colour loop runs past the end) */
    /* 0xA38 */ Object *owner;
    /* 0xA3C */ s32 unkA3C[3];
    /* 0xA48 */ Vec3f end; /* where the beam ends */
    /* 0xA54 */ f32 unkA54;
    /* 0xA58 */ f32 unkA58;
    /* 0xA5C */ f32 unkA5C;
    /* 0xA60 */ f32 unkA60;
    /* 0xA64 */ s16 shape[3][3]; /* the entry's shape, by column */
    /* 0xA76 */ s16 unkA76;
    /* 0xA78 */ s16 strength;
    /* 0xA7A */ s16 strength2;
    /* 0xA7C */ u8 padA7C;
    /* 0xA7D */ u8 on;
    /* 0xA7E */ u8 unkA7E;
    /* 0xA7F */ u8 unkA7F;
    /* 0xA80 */ u8 padA80;
    /* 0xA81 */ u8 unkA81;
    /* 0xA82 */ u8 unkA82;
    /* 0xA83 */ u8 unkA83;
} M028BeamState;

/* The state in the object */
typedef struct M028State {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 strength;
} M028State;

extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */
extern void func_80003614(s32 handle);               /* stops a positioned sound */

void func_m028_0031C(Object *obj, s32 arg1);

extern s32 rl_m028_01BD4; /* the beam's sound */

extern void func_80079D1C(Object *obj, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, f32 arg7,
                          f32 arg8, f32 arg9, f32 arg10, s32 arg11);
extern void light_colour_cycle_set(s32 handle, s32 arg1);
extern void func_800212D0(s32 handle);
extern u8 rl_m028_01BB0[]; /* .data (still assembly) */
#define rl_m028_01BD8 ((s16 *) (rl_m028_01BB0 + 0x28)) /* per kind */

/* The beam object's init: places it on its owner, copies its shape and builds its model's triangles */
#ifdef NON_MATCHING
/* 44 diffs: the shape loop's counter and pointer registers swapped, the colour loop's vertices one apart,
   and the stores after it scheduled differently */
void func_m028_00000(Object *obj, M028BeamEntry *entry) {
    Object *owner = entry->owner;
    M028BeamState *st = obj->unk68;
    M028Tri *t;
    M028Vertex *vtx;
    Object *child;
    f32 f;
    s32 i;
    s32 n;
    s32 j;
    s32 k;
    s32 v;
    s32 uv;
    s32 prev;

    if (owner != NULL) {
        obj->trans.x_position = owner->trans.x_position;
        obj->trans.y_position = owner->trans.y_position;
        obj->trans.z_position = owner->trans.z_position;
        obj->trans.rotation.y_rotation = owner->trans.rotation.y_rotation;
        obj->trans.rotation.x_rotation = owner->trans.rotation.x_rotation;
        obj->trans.rotation.z_rotation = owner->trans.rotation.z_rotation;
    }
    obj->velocity.x = 0.0f;
    obj->velocity.y = 0.0f;
    obj->velocity.z = 0.0f;
    for (i = 0; i < 3; i++) {
        st->shape[0][i] = entry->shape[0][i];
        st->shape[1][i] = entry->shape[1][i];
        st->shape[2][i] = entry->shape[2][i];
        st->unkA3C[i] = 0;
    }
    f = 0.1f;
    st->strength = 0;
    st->owner = owner;
    st->padA7C = entry->unk28;
    st->unkA7E = 0;
    st->unkA7F = 0;
    st->unkA58 = f;
    st->strength2 = entry->strength;
    st->unkA81 = 0xFF;
    st->on = 0;
    st->unkA60 = f;
    st->unkA5C = 0.05f;
    st->unkA82 = entry->unk29;
    st->unkA83 = entry->unk2A;
    vtx = (M028Vertex *) st->verts;
    for (i = 0; i < 108; i++) {
        vtx[i + 1].r = 0xFF;
        vtx[i + 1].g = 0xFF;
        vtx[i + 1].b = 0xFF;
        vtx[i + 1].a = 0xFF;
    }
    t = st->tris;
    for (n = 0; n < 3; n++) {
        for (j = 0; j < 2; j++) {
            prev = 0;
            v = 0;
            for (k = 1; k < 9; k++) {
                uv = ((k * 64) / 8 - 1) << 5;
                t[0].flags = 0x40;
                t[0].vi0 = v;
                t[0].vi1 = v + 1;
                t[0].vi2 = v + 2;
                t[0].uv[0][0] = 0x1E0;
                t[0].uv[0][1] = prev;
                t[0].uv[1][0] = 0;
                t[0].uv[1][1] = prev;
                t[0].uv[2][0] = 0x1E0;
                t[0].uv[2][1] = uv;
                t[1].flags = 0x40;
                t[1].vi0 = v + 1;
                t[1].vi1 = v + 3;
                t[1].vi2 = v + 2;
                t[1].uv[0][0] = 0;
                t[1].uv[0][1] = prev;
                t[1].uv[1][0] = 0;
                t[1].uv[1][1] = uv;
                t[1].uv[2][0] = 0x1E0;
                t[1].uv[2][1] = uv;
                t += 2;
                prev = uv;
                v += 2;
            }
        }
    }
    func_80079D1C(obj, st->padA7C, 1, 4, 0, 0, 0, 20.0f, 20.0f, -1.0f, 1.0f, 0x11);
    child = **(Object ***) obj->pad78;
    if (child != NULL && child != NULL) {
        child->animationID = 0;
    }
    if (*obj->light != 0) {
        light_colour_cycle_set(*obj->light, rl_m028_01BD8[st->unkA83]);
        func_800212D0(*obj->light);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m028/ro", func_m028_00000);
#endif

#ifdef NON_MATCHING
/* 436 diffs: register allocation (the glow loop's counter is set before the call and spilled) and the stack temporaries below the declared block */
/* What the object's pad78 handle points at, as far as the beam uses it */
typedef struct M028Glow {
    /* 0x00 */ u8 pad0[0x3B];
    /* 0x3B */ u8 level;
} M028Glow;

#define M028_GLOW(obj) (**(M028Glow ***) (obj)->pad78)
#define M028_RADII ((f32 *) rl_m028_01BB0) /* the rings' widths along the beam */

extern void func_800212E4(s32 handle);
extern void light_intensity_set(s32 handle, u8 alpha, s32 level);
extern void func_80049DC0(Object *obj, f32 *mtx);
extern void func_80048B60(f32 *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern f32 func_80097140(f32 x); /* sqrtf */

/* The beam object's control: grows or fades its strength, flickers, and rebuilds its model out to what it hits */
void func_m028_0031C(Object *obj, s32 updateRate) {
    s32 i;
    HitResult hit;
    M028BeamState *st;
    s32 c;
    s32 j;
    s32 n;
    s32 level;
    s32 d;
    M028Glow *glow;
    s32 hitRes;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 pos[3];
    f32 mtx[16];
    f32 max;
    f32 min;
    f32 scale;
    f32 r;
    f32 h;
    s32 rot;
    f32 offsets[18];
    s16 step;
    s16 a;
    s16 z;
    s16 zStep;
    M028Tri *t;
    M028Vertex *v;
    M028Vertex *v2;
    M028Glow *child;

    st = obj->unk68;
    if (st->on) {
        if (!st->unkA7E) {
            child = M028_GLOW(obj);
            if (child != NULL && child != NULL) {
                child->level = 0x80;
            }
            st->unkA81 = 0xFF;
        }
        for (i = 0; i < updateRate; i++) {
            st->unkA58 *= 1.1f;
            st->strength += (st->strength2 - st->strength) / 4;
        }
        max = 3.0f;
        if (st->unkA58 > max) {
            st->unkA58 = max;
        }
        st->unkA7E = 1;
        min = 0.1f;
    } else if (st->unkA82 != 1 && (st->strength > 0 || 0.1f < st->unkA58)) {
        min = 0.1f;
        for (i = 0; i < updateRate; i++) {
            st->strength = st->strength * 9 / 10;
            st->unkA58 *= 0.95f;
        }
        if (st->unkA58 < min) {
            st->unkA58 = min;
        }
        if (st->strength < 10) {
            st->strength = 0;
            if (min == st->unkA58) {
                st->end.x = obj->trans.x_position;
                st->end.y = obj->trans.y_position;
                st->end.z = obj->trans.z_position;
                return;
            }
        }
        max = 3.0f;
        st->unkA7E = 0;
    } else {
        st->unkA7E = 0;
        if (*obj->light != 0) {
            func_800212D0(*obj->light);
        }
        child = M028_GLOW(obj);
        if (child != NULL && child != NULL) {
            child->level = 0;
        }
        st->end.x = obj->trans.x_position;
        st->end.y = obj->trans.y_position;
        st->end.z = obj->trans.z_position;
        return;
    }
    if (st->unkA7F == 0 && st->strength2 - 10 < st->strength && func_80048E2C(0, 2) == 0) {
        st->unkA54 = (f32) func_80048E2C(0, 1000) / 1000.0f + 0.8f;
        st->unkA76 = func_80048E2C(0, 0xFFFF);
        st->unkA7F = func_80048E2C(4, 6);
    } else {
        st->unkA76 += updateRate << 8;
        if (st->unkA7F < updateRate) {
            st->unkA7F = 0;
        } else {
            st->unkA7F -= updateRate;
        }
    }
    if (max == st->unkA58) {
        st->unkA5C += st->unkA60 * updateRate;
        if (st->unkA5C > 1.5f) {
            st->unkA5C = max - st->unkA5C;
            st->unkA60 = -st->unkA60;
        } else if (st->unkA5C < 0.05f) {
            st->unkA5C = min - st->unkA5C;
            st->unkA60 = -st->unkA60;
        }
    } else {
        st->unkA5C *= 0.9f;
        if (st->unkA5C < 0.05f) {
            st->unkA5C = 0.05f;
        }
    }
    if (st->unkA81 > 100) {
        st->unkA81 -= updateRate * 2;
    }
    glow = M028_GLOW(obj);
    if (glow != NULL && glow != NULL && glow->level != 0) {
        if (glow->level > 100) {
            obj->particleEmittersEnabled = 2;
            func_8005F5AC(obj, 8);
        }
        for (i = 0; i < updateRate; i++) {
            glow->level = glow->level * 9 / 10;
        }
        if (glow->level < 10) {
            glow->level = 0;
        }
    }
    if (st->strength != 0) {
        if (*obj->light != 0) {
            func_800212E4(*obj->light);
        }
        level = st->strength * 255 / st->strength2;
        if (*obj->light != 0) {
            light_intensity_set(*obj->light, level, level);
            ((f32 *) *obj->light)[4] = -(st->strength * 3) / 4;
        }
        d = updateRate << 5;
        t = st->tris;
        for (j = 0; j < 96; j++) {
            t->uv[0][1] -= d;
            t->uv[1][1] -= d;
            t->uv[2][1] -= d;
            t++;
            if (t->uv[0][1] < -0x800) {
                t->uv[0][1] += 0x1000;
                t->uv[1][1] += 0x1000;
                t->uv[2][1] += 0x1000;
            }
        }
        func_80049DC0(obj, mtx);
        func_80048B60(mtx, 0.0f, 0.0f, -st->strength, &st->end.x, &st->end.y, &st->end.z);
        hitRes = func_80018808(obj, (Vec3f *) &obj->trans.x_position, &st->end, &hit, 0, 0);
        if (hitRes) {
            st->end.x = hit.pos.x;
            st->end.y = hit.pos.y;
            st->end.z = hit.pos.z;
            dx = st->end.x - obj->trans.x_position;
            dy = st->end.y - obj->trans.y_position;
            dz = st->end.z - obj->trans.z_position;
            h = func_80097140(dx * dx + dy * dy + dz * dz);
        } else {
            h = st->strength;
        }
        zStep = -h / (f32) 8;
        if (st->unkA82 == 0) {
            step = 0xE00;
        } else {
            step = 0x1000;
        }
        n = 0;
        for (c = 0; c < 3; c++) {
            v = (M028Vertex *) st->verts + c * 36;
            v2 = (M028Vertex *) st->verts + c * 36 + 18;
            rot = st->shape[0][c];
            st->shape[0][c] = rot + updateRate * st->shape[1][c];
            a = 0;
            z = 0;
            scale = (f32) st->strength / (f32) st->strength2;
            for (i = 0; i < 9; i++) {
                f32 w;
                f32 ox;
                f32 oy;

                r =func_80049C34(a) * 20.0f * scale;
                a += step;
                pos[0] = func_80049C34(rot) * r;
                pos[1] = func_80049C30(rot) * r;
                pos[2] = 0.0f;
                if (c == 0) {
                    offsets[n++] = pos[0];
                    offsets[n++] = pos[1];
                    pos[0] = 0.0f;
                    pos[1] = 0.0f;
                }
                w = M028_RADII[i] * scale;
                ox = offsets[i * 2];
                oy = offsets[i * 2 + 1];
                h = 18.0f * w;
                v[0].x = pos[0] - h + ox;
                v[0].y = pos[1] + oy;
                v[0].z = z;
                v[1].x = pos[0] + h + ox;
                v[1].y = pos[1] + oy;
                v[1].z = z;
                v2[0].x = pos[0] + ox;
                v2[0].y = pos[1] - h + oy;
                v2[0].z = z;
                v2[1].x = pos[0] + ox;
                v2[1].y = pos[1] + h + oy;
                v2[1].z = z;
                v += 2;
                v2 += 2;
                z += zStep;
                rot += st->shape[2][c];
            }
        }
        if (hitRes) {
            pos[0] = obj->trans.x_position;
            pos[1] = obj->trans.y_position;
            pos[2] = obj->trans.z_position;
            obj->trans.x_position = st->end.x;
            obj->trans.y_position = st->end.y;
            obj->particleEmittersEnabled = 3;
            obj->trans.z_position = st->end.z;
            func_8005F5AC(obj, updateRate);
            obj->trans.x_position = pos[0];
            obj->trans.y_position = pos[1];
            obj->trans.z_position = pos[2];
        }
        if (obj->interactObj != NULL) {
            obj->interactObj->hitPoints = st->padA7C * updateRate;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m028/ro", func_m028_0031C);
#endif

void func_m028_01624(Object *obj, M028Setup *setup) {
    M028BeamEntry entry;
    Object **beam = obj->unk68;

    entry.objectID = 0xB6;
    entry.size = sizeof(M028BeamEntry);
    entry.x = setup->x;
    entry.y = setup->y;
    entry.z = setup->z;
    if (setup->objectID == 0x1BD) {
        entry.unk2A = 1;
    } else {
        entry.unk2A = 0;
    }
    if (setup->kind == 10) {
        entry.unk28 = 1;
        entry.strength = 250;
        entry.owner = obj;
    } else {
        entry.unk28 = setup->unk10;
        entry.strength = setup->strength;
        entry.owner = setup->owner;
    }
    entry.shape[0][0] = 0;
    entry.shape[1][0] = 0x800;
    entry.shape[2][0] = 0x2000;
    entry.shape[0][1] = 0x4000;
    entry.shape[1][1] = -0x1200;
    entry.shape[2][1] = 0x2000;
    entry.shape[0][2] = 0;
    entry.shape[1][2] = 0x800;
    entry.shape[2][2] = -0x2000;
    entry.unk29 = 1;
    *beam = func_80005F14(&entry, 1);
    (*beam)->level_entry = NULL;
    obj->trans.rotation.y_rotation = setup->yRotation;
    obj->trans.rotation.x_rotation = setup->xRotation;
    (*beam)->trans.rotation.y_rotation = setup->yRotation;
    (*beam)->trans.rotation.x_rotation = setup->xRotation;
    OBJ_STATE(obj, M028State)->strength = entry.strength;
    func_m028_0031C(*beam, 2);
}

void func_m028_01798(Object *obj, s32 updateRate) {
    Object **beams = obj->unk68;
    Object *beam = *beams;

    if (obj->trans.flags & 0x400) {
        ((M028BeamState *) beam->unk68)->strength = 0;
        ((M028BeamState *) beam->unk68)->on = 0;
    } else {
        ((M028BeamState *) beam->unk68)->strength = OBJ_STATE(obj, M028State)->strength;
        ((M028BeamState *) beam->unk68)->strength2 = OBJ_STATE(obj, M028State)->strength;
        ((M028BeamState *) beam->unk68)->on = 1;
    }
    beam->trans.x_position = obj->trans.x_position;
    beam->trans.y_position = obj->trans.y_position;
    beam->trans.z_position = obj->trans.z_position;
    beam->trans.rotation.y_rotation = obj->trans.rotation.y_rotation;
    beam->trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
    beam->trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
}

/* A camera, as far as 01840 uses it (0x4C bytes) */
typedef struct M028Camera {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x4C - 0x18];
} M028Camera;

extern s32 level_id(void);
extern s32 func_80048548(void);       /* the number of cameras */
extern M028Camera *func_80041E0C(void); /* the cameras */
extern void func_800035E8(s32 sound, f32 x, f32 y, f32 z); /* moves a positioned sound */

/* Plays the beams' sound at the lit beam nearest to a camera (not in level 0xD4) */
void func_m028_01840(void) {
    s32 i;
    s32 j;
    s32 first;
    s32 count;
    s32 n;
    Object **objs;
    Object *o;
    Object *nearest;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 best;
    M028BeamState *st;
    UNUSED s32 pad;
    M028Camera *cams;

    best = 1e9f;
    nearest = NULL;
    if (level_id() != 0xD4) {
        n = func_80048548();
        cams = func_80041E0C();
        objs = func_80005E98(&first, &count);
        for (j = 0; j < n; j++) {
            for (i = first; i < count; i++) {
                o = objs[i];
                if (o->behaviorId == 0x98 && (st = o->unk68)->strength > 0) {
                    dx = cams[j].x - o->trans.x_position;
                    dy = cams[j].y - o->trans.y_position;
                    dz = cams[j].z - o->trans.z_position;
                    if (dx * dx + dy * dy + dz * dz < best) {
                        nearest = o;
                        best = dx * dx + dy * dy + dz * dz;
                    }
                }
            }
        }
        if (nearest != NULL) {
            if (rl_m028_01BD4 == 0) {
                func_80003410(0x9B, nearest->trans.x_position, nearest->trans.y_position,
                              nearest->trans.z_position, 1, &rl_m028_01BD4);
            } else {
                func_800035E8(rl_m028_01BD4, nearest->trans.x_position, nearest->trans.y_position,
                              nearest->trans.z_position);
            }
        } else if (rl_m028_01BD4 != 0) {
            func_80003614(rl_m028_01BD4);
        }
    }
}

void func_m028_01B80(void) {
    if (rl_m028_01BD4 != 0) {
        func_80003614(rl_m028_01BD4);
    }
}
#pragma weak __rl_fini = func_m028_01B80

INCLUDE_ASM("asm/nonmatchings/modules/m028", data);

/* (.rodata: C, and the assembly functions' own from ro/) */
