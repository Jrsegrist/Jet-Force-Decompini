/*
 * Runlink module 106: a hovering tank enemy (its effects are drawn far to
 * near; its control pulls some objects towards it) and the gems it drops: a gem object that
 * spins, fades out over its last 64 frames and sparkles at random, and the
 * function that spawns one with a given position, direction and speed.
 */
#include "module.h"

/* The tank's data (obj->unk68) */
typedef struct M106Tank {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ Vec3f effectPos;
    /* 0x10 */ Vec3f effectDir;
    /* 0x1C */ void *effect; /* a module 81 effect */
    /* 0x20 */ f32 unk20;
    /* 0x24 */ Object *child;
    /* 0x28 */ s16 unk28;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ f32 unk30;
    /* 0x34 */ u8 unk34;
    /* 0x35 */ s8 unk35;
    /* 0x36 */ u8 unk36;
    /* 0x37 */ u8 unk37;
    /* 0x38 */ union {
        s16 word;
        struct {
            u8 unk38;
            u8 flareAlpha;
        } b;
    } u38;
    /* 0x3A */ u8 unk3A; /* random */
    /* 0x3B */ u8 unk3B;
    /* 0x3C */ struct M106Flare *flare1;
    /* 0x40 */ struct M106Flare *flare2;
    /* 0x44 */ s32 sound1;
    /* 0x48 */ s32 sound2;
    /* 0x4C */ s32 sound3;
} M106Tank;

/* A gem's state, kept in the object */
typedef struct M106Gem {
    /* 0x00 */ s32 timer;
    /* 0x04 */ s32 alpha;
} M106Gem;

/* The level entry a gem is spawned from */
typedef struct M106GemEntry {
    LevelObjectEntryCommon common;
    /* 0x0A */ u16 size;
} M106GemEntry;

extern void effect_texture_pair_free(void *flare);
extern void func_m081_0034C(void *effect);
extern void func_80003614(s32 sound); /* stops a sound */
extern void sprite_anim_advance(void *model, s32 *flags, s32 arg2, f32 *frame, s32 updateRate);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern Object *func_80005F14(void *entry, s32 arg1); /* spawns an object from a level entry */

/* What the tank's init gets */
typedef struct M106Setup {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 scale; /* in 20ths */
} M106Setup;

extern void *effect_mesh_create(s16 x, s16 y, s16 z, s32 arg3, s32 arg4, f32 arg5, f32 arg6, s32 arg7, s32 arg8,
                                s32 arg9); /* a flare (freed with effect_texture_pair_free) */

void func_m106_00000(Object *obj, M106Setup *setup) {
    f32 zero = 0.0f;
    M106Tank *t;
    s16 x;

    obj->velocity.x = zero;
    obj->velocity.y = zero;
    obj->velocity.z = zero;
    t = obj->unk68;
    obj->trans.scale = setup->scale * 0.05f;
    if (game_flag_get(0x59)) {
        t->unk0 = 5;
    } else {
        t->unk0 = 0;
    }
    t->unk1 = 0;
    t->unk2 = 0;
    t->effect = NULL;
    t->unk20 = zero;
    t->child = NULL;
    t->unk28 = 0;
    t->unk2A = 0;
    t->unk35 = 0;
    t->unk2C = zero;
    t->unk30 = zero;
    t->unk34 = 1;
    t->unk36 = 0;
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 32.0f, 32.0f, -1.0f, 1.0f, 1);
    t->u38.word = 0;
    t->unk3A = func_80048E2C(-0x80, 0x7F);
    x = zero;
    t->flare1 = effect_mesh_create(x, x, obj->trans.scale * -68.0f, 0, -0x4000, 18.0f, 75.0f, 0, 0x11, 0x12);
    t->unk3B = func_80048E2C(-0x80, 0x7F);
    t->flare2 = effect_mesh_create(x, x, obj->trans.scale * 70.0f, 0, -0x4000, 24.0f, 100.0f, 0, 0x11, 0x12);
    t->sound1 = 0;
    t->sound2 = 0;
    t->sound3 = 0;
}

/* A flare (what effect_mesh_create makes) */
typedef struct M106Flare {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ s16 size;
} M106Flare;

/* The level entry the tank's child object is spawned from */
typedef struct M106ChildEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
} M106ChildEntry;

/* The level entry of the object spawned where the tank lands */
typedef struct M106ImpactEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
} M106ImpactEntry;

extern s32 D_800A32CC;
extern void *func_m081_00080(s32 arg0, s32 arg1, s32 arg2, f32 arg3, f32 arg4); /* creates a module 81 effect */
extern void func_m081_003D8(void *effect, f32 value);
extern void func_m081_00464(void *effect, s32 updateRate);
extern void func_m023_0065C(void);
extern s32 obj_dist_racer(f32 x, f32 y, f32 z, f32 radius, s32 is2dCheck, Object **list);
extern s32 func_8001B078(f32 x, f32 z, s32 flags, f32 ***heights); /* floor heights at x, z */
extern f32 func_80049BEC(f32 base, s32 exponent);
extern s32 func_80049D80(s32 from, s32 to); /* the difference between two angles */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern f32 get_distance_to_active_camera(f32 x, f32 y, f32 z);
extern void cam_shake_start(s32 index, f32 phase1, f32 phase2, f32 phase3, s32 strength);
extern void func_800496B0(s32 arg0, Object *obj, Vec3f *in, Vec3f *out); /* rotates by the object's transform */
extern f32 func_80097140(f32 x); /* square root */
extern void music_stop(void);
extern void func_80000510(s32 arg0);
extern s32 func_800468CC(void);
extern s32 menu_mode_get(void);
extern void level_goto(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void music_volume_fade(f32 time, s32 volume);
extern void func_800116BC(Object *obj, f32 arg1);
extern s32 func_8004966C(s16 angle); /* sine */
extern void effect_mesh_set_uvs(M106Flare *flare, s16 angle);
extern void effect_mesh_update(M106Flare *flare, s16 size, s32 alpha);
extern void audspat_point_set_pitch(s32 sound, u8 pitch);

/*
 * The tank's control: turns and hovers it, lands it with a camera shake, pulls objects
 * of behaviour 0x86 in front of it towards its effect, fades its flares and plays its sounds.
 * (The literal spellings matter: IDO shares a constant only between uses spelt the same, so
 * `+ 10`, `0.5` and `0` keep the ROM's register allocation.)
 */
void func_m106_00290(Object *obj, s32 updateRate) {
    UNUSED s32 pad;
    s32 i;
    s32 count;
    M106Tank *t;
    f32 rate;
    Vec3f from;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 speed;
    f32 dot;
    f32 target;
    f32 radius;
    f32 shake;
    HitResult hit;
    u8 *grab;
    Object *other;
    Object *child;
    Object *list[8];
    Object **objs;
    s32 k;
    f32 **heights;
    M106ChildEntry entry;
    M106ImpactEntry impact;
    s32 a;
    s32 size;
    M106Flare *flare;

    t = obj->unk68;
    if (t->unk0 == 5) {
        func_800079B8(obj);
        D_800A32CC = 0;
        return;
    }
    if (t->unk2 < 6000) {
        if (updateRate < t->unk2) {
            t->unk2 = t->unk2 - updateRate;
        } else {
            t->unk2 = 0;
        }
    }
    if (t->effect == NULL) {
        t->effect = func_m081_00080(0x80, 2, 4, 45.0f, 320.0f);
    }
    if (t->child == NULL) {
        entry.objectID = 0x1DB;
        entry.size = 10;
        entry.unk3 = 0;
        entry.x = 0;
        entry.y = 0;
        entry.z = 0;
        if ((t->child = func_80005F14(&entry, 1)) != NULL) {
            t->child->level_entry = NULL;
        }
    }
    from.x = obj->trans.x_position;
    from.y = obj->trans.y_position;
    from.z = obj->trans.z_position;
    if (obj_dist_racer(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 450.0f, 1, list) != 0 && t->unk0 < 3) {
        t->unk1 = 2;
        if (t->unk0 != 2) {
            t->unk0 = 2;
            t->unk37 = 0;
        }
    } else {
        t->unk1 = 0;
    }
    switch (t->unk0) {
        case 0:
            t->unk0 = 1;
            break;
        case 1:
            if (t->unk35 < 32) {
                t->unk35++;
            }
            obj->trans.rotation.y_rotation += t->unk35 * updateRate;
            break;
        case 2:
            if (t->unk35 > 0) {
                t->unk35--;
            }
            if (t->unk34 != 0) {
                t->unk0 = 1;
            } else if (func_8001B078(obj->trans.x_position, obj->trans.z_position, 0x800, &heights)) {
                shake = obj->trans.y_position - **heights;
                if (t->unk37 != 0) {
                    if (shake < 480.0f) {
                        t->unk37 = 0;
                    }
                } else if (shake > 480.0f) {
                    t->unk37 = 1;
                }
            }
            if (t->unk37 != 0) {
                t->unk1 = 0;
            }
            obj->trans.rotation.y_rotation += t->unk35 * updateRate;
            break;
        case 3:
        case 4:
            if (t->unk35 > 0) {
                t->unk35--;
            }
            obj->trans.rotation.y_rotation += t->unk35 * updateRate;
            break;
    }
    rate = updateRate;
    if (t->unk1 & 1) {
        if (t->unk2C < 24.0f) {
            t->unk2C += 0.1f * rate;
            if (t->unk2C > 24.0f) {
                t->unk2C = 24.0f;
            }
        }
    } else {
        t->unk2C *= func_80049BEC(0.975f, updateRate);
        if (t->unk2C > -0.1f && t->unk2C < 0.1f) {
            t->unk2C = 0.0f;
        }
    }
    obj->velocity.x = -(func_80049C34(obj->trans.rotation.y_rotation) * t->unk2C);
    obj->velocity.z = -(func_80049C30(obj->trans.rotation.y_rotation) * t->unk2C);
    if (t->unk1 & 2) {
        if (t->unk30 < 0.5f) {
            t->unk30 += 0.01f * rate;
            if (t->unk30 > 0.5f) {
                t->unk30 = 0.5f;
            }
        }
        t->unk34 = 0;
    } else {
        t->unk30 *= func_80049BEC(0.825f, updateRate);
        if (t->unk30 > -0.01f && t->unk30 < 0.01f) {
            t->unk30 = 0.0f;
            if (t->unk1 & 4) {
                obj->velocity.y *= func_80049BEC(0.95f, updateRate);
                if (obj->velocity.y > -0.1f && obj->velocity.y < 0.1f) {
                    obj->velocity.y = 0.0f;
                }
            } else if (t->unk34 == 0) {
                if (obj->velocity.y > 0.0f) {
                    obj->velocity.y *= func_80049BEC(0.95f, updateRate);
                }
                obj->velocity.y -= 0.35f;
                if (obj->velocity.y < -10.0f) {
                    obj->velocity.y = -10.0f;
                }
            }
        }
    }
    if (t->unk36 != 0) {
        t->unk36 -= updateRate;
        if (t->unk36 <= 0) {
            t->unk36 = 0;
            t->unk1 &= ~3;
        }
    }
    if (t->unk30 != 0) {
        if (obj->velocity.y < 0.0f) {
            obj->velocity.y *= func_80049BEC(0.95f, updateRate);
        }
        obj->velocity.y += t->unk30;
        if (obj->velocity.y > 10.0f) {
            obj->velocity.y = 10.0f;
        }
    }
    a = t->unk2C * 170.66667f;
    if (a < (s32) (t->unk30 * 6400.0f)) {
        a = -(s32) (t->unk30 * 6400.0f);
    }
    i = updateRate;
    while (i--) {
        obj->trans.rotation.x_rotation += func_80049D80(obj->trans.rotation.x_rotation, -a) >> 4;
    }
    if (t->unk34 == 0) {
        func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
        radius = 3.0f;
        func_8001CE10(1, &from, (Vec3f *) &obj->trans.x_position, &radius, 0, 0, 0);
        if (func_80016208(obj, &from, (Vec3f *) &obj->trans.x_position, radius, 32, &hit) && (((u8 *) &hit.flags)[1] & 2)) {
            d = get_distance_to_active_camera(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
            if (d < 2500.0f) {
                shake = -(obj->velocity.y * 2.4f);
                if (d > 500.0f) {
                    shake *= (2500.0f - d) / 2000.0f;
                }
                cam_shake_start(0, 0.0f, 0.0f, 2.0f, shake);
            }
            func_80003410(0x1D4, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 4, NULL);
            impact.common.objectID = 0x13B;
            impact.common.x = obj->trans.x_position;
            impact.common.y = obj->trans.y_position + 4.0f;
            impact.common.z = obj->trans.z_position;
            impact.unkA = 0;
            impact.unkC = -0x4000;
            impact.unkE = 0x2D;
            impact.unk10 = 1.0f;
            if ((other = func_80005F14(&impact, 1)) != NULL) {
                other->level_entry = NULL;
            }
            t->unk34 = 1;
            obj->velocity.y = 0.0f;
        }
    }
    t->effectPos.x = 0.0f;
    t->effectPos.y = obj->trans.scale * 20.0f;
    t->effectPos.z = obj->trans.scale * -88.0f;
    func_800496B0(1, obj, &t->effectPos, &t->effectPos);
    t->effectPos.x += obj->trans.x_position;
    t->effectPos.y += obj->trans.y_position;
    t->effectPos.z += obj->trans.z_position;
    t->effectDir.x = 0.0f;
    t->effectDir.y = 0.0f;
    t->effectDir.z = -1.0f;
    func_800496B0(1, obj, &t->effectDir, &t->effectDir);
    objs = func_80005E98(&i, &count);
    target = 0;
    if (t->unk0 == 1) {
        while (i < count) {
            other = objs[i++];
            if (other->behaviorId == 0x86) {
                dx = other->trans.x_position - t->effectPos.x;
                dy = other->trans.y_position - t->effectPos.y;
                dz = other->trans.z_position - t->effectPos.z;
                d = dx * dx + dy * dy + dz * dz;
                if (d < 64.0f) {
                    func_800079B8(other);
                    func_80001248(0x16E, 0);
                    t->unk2 += 600;
                    if (t->unk2 > 6000) {
                        t->unk2 = 6000;
                    }
                } else if (d < 102400.0f) {
                    d = func_80097140(d);
                    speed = d * -0.025f + 10;
                    if (d > 0.0f) {
                        dx /= d;
                        dy /= d;
                        dz /= d;
                    }
                    grab = other->unk7C;
                    dot = t->effectDir.x * dx + t->effectDir.y * dy + t->effectDir.z * dz;
                    if (grab != NULL) {
                        if (d < 48.0f) {
                            target = 1.0f;
                            d = func_80049BEC(0.6667f, updateRate);
                            dx = -dx * speed;
                            dy = -dy * speed;
                            *grab &= ~2;
                            dz = -dz * speed;
                            d = 1.0f - d;
                            other->velocity.x += (dx - other->velocity.x) * d;
                            other->velocity.y += (dy - other->velocity.y) * d;
                            other->velocity.z += (dz - other->velocity.z) * d;
                            *(Object **) (grab + 0x20) = obj;
                        } else if (dot > 0.7071f) {
                            speed *= dot;
                            target = 1.0f;
                            d = func_80049BEC(0.875f, updateRate);
                            dx = -dx * speed;
                            dy = -dy * speed;
                            *grab &= ~2;
                            dz = -dz * speed;
                            d = 1.0f - d;
                            other->velocity.x += (dx - other->velocity.x) * d;
                            other->velocity.y += (dy - other->velocity.y) * d;
                            other->velocity.z += (dz - other->velocity.z) * d;
                            *(Object **) (grab + 0x20) = obj;
                        }
                    }
                }
            }
        }
    }
    if (t->effect != NULL) {
        t->unk20 += (target - t->unk20) * (1.0f - func_80049BEC(0.9f, updateRate));
        func_m081_003D8(t->effect, t->unk20);
        func_m081_00464(t->effect, updateRate);
    }
    if (t->unk2A == 0) {
        if (t->unk28 < t->unk2) {
            t->unk28 += updateRate * 4;
            if (t->unk28 > t->unk2) {
                t->unk28 = t->unk2;
            }
        } else {
            t->unk28 -= updateRate * 4;
            if (t->unk28 < t->unk2) {
                t->unk28 = t->unk2;
            }
        }
        D_800A32CC = t->unk28 * 0x1800 / 6000 + 0x100;
        if (t->unk28 == 6000) {
            t->unk2A = 60;
            music_stop();
            func_80000510(0x2B);
        }
    } else if (t->unk0 != 4) {
        if (updateRate < t->unk2A) {
            t->unk2A -= updateRate;
            func_m023_0065C();
        } else {
            t->unk2A = 1;
            t->unk28 -= updateRate * 12;
            if (t->unk28 < 0) {
                t->unk0 = 4;
                t->unk28 = 0;
                level_goto(0x165, func_800468CC(), 0, menu_mode_get(), 0, 0);
                music_volume_fade(0.5, 0);
            }
            D_800A32CC = t->unk28 * 0x1300 / 6000 + 0x600;
        }
    }
    child = t->child;
    if (child != NULL) {
        child->trans.rotation.y_rotation = obj->trans.rotation.y_rotation;
        child->trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
        child->trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
        child->trans.x_position = 0.0f;
        child->trans.y_position = obj->trans.scale * 130.0f;
        child->trans.z_position = 0.0f;
        func_800496B0(1, obj, (Vec3f *) &child->trans.x_position, (Vec3f *) &child->trans.x_position);
        child->trans.x_position += obj->trans.x_position;
        child->trans.y_position += obj->trans.y_position;
        child->trans.z_position += obj->trans.z_position;
        child->trans.scale = t->unk28 / 6000.0f * obj->trans.scale + 0.001f;
        func_800116BC(child, 1.0f);
    }
    if (t->unk37 != 0) {
        count = 0x80;
    } else if (t->unk1 == 0) {
        count = 0;
    } else {
        count = 0xFF;
    }
    k = updateRate;
    while (k--) {
        i = (count - t->u38.word) >> 4;
        if (i == 0) {
            if (count < t->u38.word) {
                i = -1;
            } else if (t->u38.word < count) {
                i = 1;
            }
        }
        t->u38.word += i;
    }
    if (t->u38.word != 0) {
        if ((flare = t->flare1) != NULL) {
            t->unk3A += updateRate * 16;
            size = ((func_8004966C(t->unk3A << 8) + 0x18000) * ((flare->size * t->u38.word) >> 8)) >> 16;
            effect_mesh_set_uvs(flare, t->unk3A << 8);
            effect_mesh_update(flare, size, t->u38.word);
        }
        if ((flare = t->flare2) != NULL) {
            t->unk3B += updateRate * 16;
            size = ((func_8004966C(t->unk3B << 8) + 0x18000) * ((flare->size * t->u38.word) >> 8)) >> 16;
            effect_mesh_set_uvs(flare, t->unk3B << 8);
            effect_mesh_update(flare, size, t->u38.word);
        }
    }
    if (t->unk28 != 0) {
        if (t->sound1 == 0) {
            func_80003410(0x1D6, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &t->sound1);
        }
        audspat_point_set_pitch(t->sound1, t->unk28 * 75 / 6000 + 75);
    } else if (t->sound1 != 0) {
        func_80003614(t->sound1);
    }
    if (t->unk34 != 0 && t->unk35 != 0) {
        if (t->sound2 == 0) {
            func_80003410(0x1D3, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &t->sound2);
        }
        audspat_point_set_pitch(t->sound2, t->unk35 + 70);
    } else if (t->sound2 != 0) {
        func_80003614(t->sound2);
    }
    if (t->u38.word != 0) {
        if (t->sound3 == 0) {
            func_80003410(0x1D5, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1, &t->sound3);
        }
        audspat_point_set_pitch(t->sound3, t->u38.word * 50 / 255 + 75);
    } else if (t->sound3 != 0) {
        func_80003614(t->sound3);
    }
}

extern void func_80049DC0(ObjectTransform *trans, MtxF *out); /* the transform's matrix */
extern void func_80048B60(MtxF *mtx, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz); /* DKR mathMtxXFMF */
extern void mtx_model_push(Gfx **dl, Mtx **mtx, MtxF *transform);
extern void mtx_pop(Gfx **dl);
extern void func_80069BE8(Gfx **dl, void *flare, s32 arg2, s32 alpha);
extern void func_m081_006F8(Gfx **dl, Mtx **mtx, void *effect, s16 *rotation, Vec3f *pos, MtxF *transform);

/* Draws the tank's two flares and its effect, the farthest from the camera first */
void func_m106_01694(Gfx **dl, Mtx **mtx, Object *obj) {
    M106Tank *t = obj->unk68;
    s32 n;
    s32 i;
    s32 j;
    s32 kind[3];
    f32 x;
    f32 y;
    f32 z;
    s32 k;
    MtxF m;
    s16 rot[3];
    f32 dist[3];
    ObjectTransform trans;
    f32 tmp;

    trans.rotation.y_rotation = obj->trans.rotation.y_rotation;
    trans.rotation.x_rotation = obj->trans.rotation.x_rotation;
    trans.rotation.z_rotation = obj->trans.rotation.z_rotation;
    trans.scale = 1.0f;
    trans.x_position = obj->trans.x_position;
    trans.y_position = obj->trans.y_position;
    trans.z_position = obj->trans.z_position;
    func_80049DC0(&trans, &m);
    n = 0;
    if (t->flare1 != NULL) {
        func_80048B60(&m, 0.0f, 0.0f, obj->trans.scale * -68.0f, &x, &y, &z);
        dist[0] = get_distance_to_active_camera(x, y, z);
        kind[0] = 0;
        n = 1;
    }
    if (t->flare2 != NULL) {
        func_80048B60(&m, 0.0f, 0.0f, obj->trans.scale * 70.0f, &x, &y, &z);
        dist[n] = get_distance_to_active_camera(x, y, z);
        kind[n] = 1;
        n++;
    }
    if (t->effect != NULL) {
        dist[n] = get_distance_to_active_camera(t->effectPos.x, t->effectPos.y, t->effectPos.z);
        kind[n] = 2;
        n++;
    }
    for (i = n - 1; i > 0; i--) {
        for (j = 0; j < i; j++) {
            if (dist[j] < dist[j + 1]) {
                tmp = dist[j];
                dist[j] = dist[j + 1];
                dist[j + 1] = tmp;
                k = kind[j];
                kind[j] = kind[j + 1];
                kind[j + 1] = k;
            }
        }
    }
    for (i = 0; i < n;) {
        switch (kind[i++]) {
            case 0:
                mtx_model_push(dl, mtx, &m);
                func_80069BE8(dl, t->flare1, 6, t->u38.b.flareAlpha);
                mtx_pop(dl);
                break;
            case 1:
                mtx_model_push(dl, mtx, &m);
                func_80069BE8(dl, t->flare2, 6, t->u38.b.flareAlpha);
                mtx_pop(dl);
                break;
            case 2:
                rot[0] = 0;
                rot[1] = 0x4000;
                rot[2] = 0;
                func_m081_006F8(dl, mtx, t->effect, rot, &t->effectPos, &m);
                break;
        }
    }
}

void func_m106_01A30(Object *obj) {
    M106Tank *tank = obj->unk68;

    if (tank->flare1 != NULL) {
        effect_texture_pair_free(tank->flare1);
    }
    if (tank->flare2 != NULL) {
        effect_texture_pair_free(tank->flare2);
    }
    if (tank->effect != NULL) {
        func_m081_0034C(tank->effect);
    }
    if (tank->child != NULL) {
        func_800079B8(tank->child);
    }
    if (tank->sound1 != 0) {
        func_80003614(tank->sound1);
    }
    if (tank->sound2 != 0) {
        func_80003614(tank->sound2);
    }
    if (tank->sound3 != 0) {
        func_80003614(tank->sound3);
    }
}

void func_m106_01AFC(Object *obj, M106GemEntry *setup) {
    OBJ_STATE(obj, M106Gem)->timer = setup->size * 6;
    OBJ_STATE(obj, M106Gem)->alpha = 0xFF;
}

void func_m106_01B1C(Object *obj, s32 updateRate) {
    s32 flags;

    obj->interactObj->hitPoints = 0x1000;
    if (updateRate >= OBJ_STATE(obj, M106Gem)->timer) {
        func_800079B8(obj);
        return;
    }
    OBJ_STATE(obj, M106Gem)->timer -= updateRate;
    if (OBJ_STATE(obj, M106Gem)->timer < 64) {
        OBJ_STATE(obj, M106Gem)->alpha = OBJ_STATE(obj, M106Gem)->timer * 4;
    }
    flags = 9;
    sprite_anim_advance(obj->modelInstances[obj->modelIndex], &flags, 12, &obj->unk28, updateRate);
    if (func_80048E2C(0, 100) > 90) {
        obj->particleEmittersEnabled |= 1;
    } else {
        obj->particleEmittersEnabled &= ~1;
    }
    func_8005F5AC(obj, updateRate);
}

void func_m106_01C04(f32 x, f32 y, f32 z, s16 yRotation, s16 xRotation, f32 speed, f32 size) {
    Vec3s rot;
    M106GemEntry entry;
    Object *obj;

    entry.common.objectID = 0x311;
    entry.common.x = x;
    entry.common.y = y;
    entry.common.z = z;
    entry.size = (s32) (size * 10.0f);
    obj = func_80005F14(&entry, 1);
    if (obj != NULL) {
        obj->level_entry = NULL;
        rot.z_rotation = 0;
        rot.y_rotation = yRotation;
        rot.x_rotation = xRotation;
        obj->velocity.z = -speed;
        func_8004944C(&rot, &obj->velocity);
        if (obj->unk7C != NULL) {
            *obj->unk7C &= ~2;
        }
    }
}
