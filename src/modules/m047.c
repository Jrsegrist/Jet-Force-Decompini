/*
 * Runlink module 47: the cutscene sequencer's helpers - scrolling textures,
 * colour fades, curves and paths, effects - and the subtitle boxes shown
 * during cutscenes (up to five, fading in while their timer runs and out after).
 */
#include "module.h"

/* An unreferenced debug string at the start of the module's .rodata */
static const char sDebugText[] = "x = %d, y = %d, width = %d, alpha = %d\n";

/* A subtitle (size 0xC) */
typedef struct M047Subtitle {
    /* 0x0 */ s16 timer;
    /* 0x2 */ u16 text; /* index into the game's texts */
    /* 0x4 */ u16 x;
    /* 0x6 */ u16 y;
    /* 0x8 */ u8 r;
    /* 0x9 */ u8 g;
    /* 0xA */ u8 b;
    /* 0xB */ u8 alpha;
} M047Subtitle;

/* A colour fade (size 0x10) */
typedef struct M047ColourFade {
    /* 0x0 */ s32 target; /* < 0: an object's light; else a colour block, address & 0x7FFFFFFF */
    /* 0x4 */ u8 r0;      /* the colour at the start */
    /* 0x5 */ u8 r1;      /* and at the end */
    /* 0x6 */ u8 g0;
    /* 0x7 */ u8 g1;
    /* 0x8 */ u8 b0;
    /* 0x9 */ u8 b1;
    /* 0xA */ u8 a0;
    /* 0xB */ u8 a1;
    /* 0xC */ s16 timer;
    /* 0xE */ s16 duration;
} M047ColourFade;

extern M047ColourFade D_80104708[12];
extern void light_colour_set(s32 light, u8 r, u8 g, u8 b); /* sets a light's colour */
extern void light_intensity_set(s32 light, u8 a);


extern void func_8006FC54(s32 font);
extern void func_8006FC8C(s32 r, s32 g, s32 b, s32 a);
extern void func_8006FC68(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8006FCA8(Gfx **dl, s32 x, s32 y, char *text, s32 flags);
extern s32 func_800706D4(char *text, s32 x, s32 font); /* DKR get_text_width */
extern void func_m157_00000(Gfx **dl, u8 shape, s32 x, s32 y, s32 width, s32 height, u32 colour);

extern char **gMenuText; /* the game's texts */

static M047Subtitle sSubtitles[5];

/* A scrolling texture (size 0x14) */
typedef struct M047Scroller {
    /* 0x00 */ u8 tex;  /* 0xFF: not in use */
    /* 0x01 */ u8 frac; /* the eighths of a texel left over: u in the low nibble, v in the high one */
    /* 0x02 */ s16 timer; /* frames left of the speed change, 0: none */
    /* 0x04 */ u32 du;  /* the speed in u and v (8 fractional bits) */
    /* 0x08 */ u32 ddu; /* and how it changes per frame */
    /* 0x0C */ u32 dv;
    /* 0x10 */ u32 ddv;
} M047Scroller;

extern M047Scroller D_80104608[8];
extern void tex_scroll_queue(s32 tex, s32 u, s32 v); /* scrolls a texture by (u, v) texels */

/* Scrolls the textures on by `updateRate` frames */
void func_m047_00000(s32 updateRate) {
    s32 i;
    s32 step;
    s32 u;
    s32 v;
    M047Scroller *s;

    s = D_80104608;
    i = 7;
    do {
        if (s->tex != 0xFF) {
            step = s->timer;
            if (step != 0) {
                if (updateRate < step) {
                    step = updateRate;
                }
                s->du += s->ddu * step;
                s->dv += s->ddv * step;
                s->timer -= step;
            }
            u = ((s32) (s->du * updateRate) >> 8) + (s->frac & 0xF);
            v = ((s32) (s->dv * updateRate) >> 8) + (s->frac >> 4);
            tex_scroll_queue(s->tex, u >> 3, v >> 3);
            s->frac = u & 7;
            s->frac |= (v & 7) << 4;
        }
        s++;
    } while (i--);
}

/* Moves the colour fades on by `updateRate` frames */
void func_m047_00130(s32 updateRate) {
    s32 i;
    s32 target;
    s32 r;
    s32 g;
    s32 b;
    s32 a;
    u32 frac;
    u8 *colour;
    M047ColourFade *fade;

    fade = D_80104708;
    i = 12;
    while (i--) {
        target = fade->target;
        if (target != 0) {
            r = fade->r1;
            g = fade->g1;
            b = fade->b1;
            a = fade->a1;
            if (updateRate >= fade->timer) {
                fade->target = 0;
            } else {
                fade->timer -= updateRate;
                frac = (fade->timer << 16) / fade->duration;
                r += (s32) ((fade->r0 - fade->r1) * frac) >> 16;
                g += (s32) ((fade->g0 - fade->g1) * frac) >> 16;
                b += (s32) ((fade->b0 - fade->b1) * frac) >> 16;
                a += (s32) ((fade->a0 - fade->a1) * frac) >> 16;
            }
            colour = (u8 *) (target | 0x80000000);
            if (target < 0) {
                light_colour_set(target, r, g, b);
                light_intensity_set(target, a);
            } else {
                colour[0x38] = r;
                colour[0x39] = g;
                colour[0x3A] = b;
                colour[0x3B] = a;
            }
        }
        fade++;
    }
}


/* A key of a path (size 0x2C): a transform, the spin to apply on the way to the next key, the speed */
typedef struct M047Key {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ s16 z_rotation;
    /* 0x06 */ u8 pad6;
    /* 0x07 */ u8 linear; /* interpolate straight to the next key, not along a curve */
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ s16 spin[3]; /* angle speeds (y, x, z) */
    /* 0x1E */ u8 pad1E[0x20 - 0x1E];
    /* 0x20 */ f32 speed;
    /* 0x24 */ struct M047Key *prev;
    /* 0x28 */ struct M047Key *next;
} M047Key;

/*
 * Evaluates channel `channel` of the spline through the previous key, the key, the next key and the one after
 * at `t` (0..1): 0-2 the position, 3-5 the rotation (angles unwrapped first), else the scale. Writes the
 * value and the slope.
 */
void func_m047_002C4(M047Key *key, f32 t, f32 *value, f32 *slope, s32 channel) {
    s32 i;
    s32 j;
    s32 isAngle;
    M047Key *next;
    f32 c0;
    M047Key *prev;
    M047Key *last;
    f32 d;
    f32 p[4];
    f32 c1;
    f32 c2;
    f32 c3;

    prev = key->prev;
    next = key->next;
    last = next->next;
    isAngle = FALSE;
    switch (channel) {
        case 0:
            p[0] = prev->x;
            p[1] = key->x;
            p[2] = next->x;
            p[3] = last->x;
            break;
        case 1:
            p[0] = prev->y;
            p[1] = key->y;
            p[2] = next->y;
            p[3] = last->y;
            break;
        case 2:
            p[0] = prev->z;
            p[1] = key->z;
            p[2] = next->z;
            p[3] = last->z;
            break;
        case 3:
            isAngle = TRUE;
            p[0] = prev->y_rotation;
            p[1] = key->y_rotation;
            p[2] = next->y_rotation;
            p[3] = last->y_rotation;
            break;
        case 4:
            isAngle = TRUE;
            p[0] = prev->x_rotation;
            p[1] = key->x_rotation;
            p[2] = next->x_rotation;
            p[3] = last->x_rotation;
            break;
        case 5:
            isAngle = TRUE;
            p[0] = prev->z_rotation;
            p[1] = key->z_rotation;
            p[2] = next->z_rotation;
            p[3] = last->z_rotation;
            break;
        default:
            p[0] = prev->scale;
            p[1] = key->scale;
            p[2] = next->scale;
            p[3] = last->scale;
            break;
    }
    if (isAngle) {
        for (i = 0; i < 3; i++) {
            d = p[i + 1] - p[i];
            if (d > 32768.0f) {
                for (j = i + 1; j < 4; j++) {
                    p[j] -= 65536.0f;
                }
            } else if (d < -32768.0f) {
                for (j = i + 1; j < 4; j++) {
                    p[j] += 65536.0f;
                }
            }
        }
    }
    if (key->linear) {
        if (value != NULL) {
            *value = p[1] + (p[2] - p[1]) * t;
        }
        if (slope != NULL) {
            *slope = p[2] - p[1];
        }
    } else {
        c0 = p[1];
        c1 = -0.56f * p[0] + p[2] * 0.56f;
        c2 = 1.12f * p[0] + -2.44f * p[1] + 1.88f * p[2] + p[3] * -0.56f;
        c3 = -0.56f * p[0] + 1.44f * p[1] + -1.44f * p[2] + p[3] * 0.56f;
        if (value != NULL) {
            *value = ((c3 * t + c2) * t + c1) * t + c0;
        }
        if (slope != NULL) {
            *slope = (c3 * 3.0f * t + 2.0f * c2) * t + c1;
        }
    }
}

/* A cutscene actor: its object, the path it follows and how */
typedef struct M047Actor {
    /* 0x00 */ u8 pad0;
    /* 0x01 */ u8 target;    /* rotation mode 3: the actor to face */
    /* 0x02 */ u8 pad2[0x8 - 0x2];
    /* 0x08 */ Object *obj;
    /* 0x0C */ f32 animSpeed;
    /* 0x10 */ f32 smoothing; /* 1: turn straight to the path's rotation */
    /* 0x14 */ u8 backwards; /* play the animation backwards */
    /* 0x15 */ u8 rotMode;   /* 1: spin, 2: face along the path, 3: face the target, else the keys' rotation */
    /* 0x16 */ u8 flags;     /* 2: the actor to follow, 4: not moved along the path */
    /* 0x17 */ u8 pad17;
    /* 0x18 */ M047Key *key; /* the key before the object */
    /* 0x1C */ f32 t;        /* how far to the next key (0..1) */
    /* 0x20 */ u8 pad20[0x24 - 0x20];
    /* 0x24 */ u8 alphaFrom;
    /* 0x25 */ u8 alphaTo;
    /* 0x26 */ u8 fadeLength;
    /* 0x27 */ u8 fadeTimer;
    /* 0x28 */ u8 pad28[0x2A - 0x28];
    /* 0x2A */ s16 timer;    /* frames left of the scale change */
    /* 0x2C */ f32 scale;
    /* 0x30 */ f32 scaleSpeed;
} M047Actor;

/* What obj->unk5C points at, as 0084C uses it */
typedef struct M047Unk5C {
    /* 0x000 */ u8 pad0[0x118];
    /* 0x118 */ f32 x;
    /* 0x11C */ f32 y;
    /* 0x120 */ f32 z;
    /* 0x124 */ s16 z_rotation;
    /* 0x126 */ s16 x_rotation;
    /* 0x128 */ s16 y_rotation;
    /* 0x12A */ u8 pad12A[0x12E - 0x12A];
    /* 0x12E */ s16 saved; /* the start transform is saved */
} M047Unk5C;

extern M047Actor **D_801045B0; /* the actors */
extern s32 D_800A74D4;
extern s16 func_80015510(f32 x, f32 y, f32 z); /* the level segment at a position */
extern s32 func_80049C84(f32 x, f32 z);         /* arctangent as a 16-bit angle */
extern f32 func_80097140(f32 x);                /* square root */
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern void func_8005F5AC(Object *obj, s32 updateRate);

static void func_m047_013D0(M047Actor *a, Object *obj, s32 updateRate);
static void func_m047_01560(M047Actor *a, Object *obj, s32 updateRate);

/* Turns angle `from` part `t` of the shorter way towards `to` (called from 0084C) */
static s16 func_m047_007DC(f32 t, s16 from, s16 to) {
    s32 d = (u16) (to - from);
    s32 back = (u16) (from - to);

    if (back < d) {
        d = -back;
    }
    return from + (s32) (d * t);
}

/* Moves the actor's object along its path by `time` seconds: position, speed, scale and rotation */
void func_m047_0084C(M047Actor *a, f32 time, s32 updateRate, s32 arg3) {
    f32 nt;
    s16 rotY;
    s16 rotX;
    s16 rotZ;
    f32 speed0;
    f32 speed1;
    f32 t2;
    f32 tx;
    f32 ty;
    f32 tz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 d;
    f32 total;
    f32 t;
    f32 dist;
    f32 avg;
    f32 remaining;
    f32 step;
    f32 half;
    f32 frames;
    s32 iter;
    M047Key *key;
    M047Unk5C *u;
    Object *obj;
    Object *other;

    if (a != NULL) {
        if (a->obj != NULL && !(time < 0.0f)) {
            obj = a->obj;
            frames = time * 60.0f;
            u = (M047Unk5C *) obj->unk5C;
            if (u != NULL && u->saved == 0) {
                u->x = obj->trans.x_position;
                u->y = obj->trans.y_position;
                u->z = obj->trans.z_position;
                u->y_rotation = obj->trans.rotation.y_rotation;
                u->x_rotation = obj->trans.rotation.x_rotation;
                u->z_rotation = obj->trans.rotation.z_rotation;
                u->saved = 1;
            }
            func_m047_013D0(a, obj, arg3);
            if ((a->flags & 4) || D_800A74D4 < 0) {
                func_m047_01560(a, obj, arg3);
                func_8005F5AC(obj, updateRate);
                return;
            }
            key = a->key;
            t = a->t;
            if (time == 0.0f) {
                func_m047_002C4(key, t, &obj->trans.x_position, NULL, 0);
                func_m047_002C4(key, t, &obj->trans.y_position, NULL, 1);
                func_m047_002C4(key, t, &obj->trans.z_position, NULL, 2);
                obj->segmentID = func_80015510(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
            } else if (key->speed != 0.0f || key->next->speed != 0.0f) {
                step = 0.01f;
                half = 0.005f;
                remaining = time;
                iter = 0;
                speed0 = key->speed + t * (key->next->speed - key->speed);
                obj->velocity.x = obj->trans.x_position;
                obj->velocity.y = obj->trans.y_position;
                obj->velocity.z = obj->trans.z_position;
                total = 0.0f;
                while (iter < 9 && (t != 1.0f || key->next != key->next->next)) {
                    if (t == 1.0f) {
                        key = key->next;
                        step = 0.01f;
                        half = 0.005f;
                        t = 0.0f;
                        iter = 0;
                    }
                    nt = t + step;
                    t2 = nt;
                    if (nt > 1.0f) {
                        speed1 = key->next->speed;
                        tx = key->next->x;
                        t2 = 1.0f;
                        ty = key->next->y;
                        tz = key->next->z;
                    } else {
                        speed1 = key->speed + t2 * (key->next->speed - key->speed);
                        func_m047_002C4(key, t2, &tx, NULL, 0);
                        func_m047_002C4(key, nt, &ty, NULL, 1);
                        func_m047_002C4(key, nt, &tz, NULL, 2);
                    }
                    dx = tx - obj->trans.x_position;
                    dy = ty - obj->trans.y_position;
                    dz = tz - obj->trans.z_position;
                    dist = func_80097140(dx * dx + dy * dy + dz * dz);
                    avg = (speed0 + speed1) * 0.5f;
                    if (avg > 0.0f) {
                        d = dist / avg;
                        remaining -= d;
                        if ((remaining > -0.00001f && remaining < 0.00001f) || iter == 8) {
                            iter = 9;
                            obj->trans.x_position = tx;
                            obj->trans.y_position = ty;
                            obj->trans.z_position = tz;
                            obj->segmentID = func_80015510(tx, ty, tz);
                            t = t2;
                            total += dist;
                        } else if (iter == 0 && remaining > 0.0f) {
                            obj->trans.x_position = tx;
                            obj->trans.y_position = ty;
                            obj->trans.z_position = tz;
                            obj->segmentID = func_80015510(tx, ty, tz);
                            t = t2;
                            speed0 = speed1;
                            total += dist;
                        } else {
                            if (remaining > 0.0f) {
                                step += half;
                            } else {
                                step -= half;
                            }
                            remaining += d;
                            half *= 0.5f;
                            iter++;
                        }
                    } else {
                        iter = 9;
                    }
                }
                obj->velocity.x = (obj->trans.x_position - obj->velocity.x) / frames;
                obj->velocity.y = (obj->trans.y_position - obj->velocity.y) / frames;
                obj->velocity.z = (obj->trans.z_position - obj->velocity.z) / frames;
            }
            a->t = t;
            a->key = key;
            if (a->timer != 0) {
                if (updateRate < a->timer) {
                    a->scale += a->scaleSpeed * frames;
                    a->timer -= updateRate;
                } else {
                    a->scale += a->scaleSpeed * a->timer;
                    a->timer = 0;
                }
            }
            func_m047_002C4(key, t, &dx, NULL, 6);
            obj->trans.scale = obj->header->scale * a->scale * dx;
            if (obj->shadow != NULL) {
                *obj->shadow = obj->header->shadowScale * dx;
            }
            rotY = obj->trans.rotation.y_rotation;
            rotX = obj->trans.rotation.x_rotation;
            rotZ = obj->trans.rotation.z_rotation;
            switch (a->rotMode) {
                case 1:
                    dx = (key->next->spin[0] - key->spin[0]) * t + key->spin[0];
                    dy = (key->next->spin[1] - key->spin[1]) * t + key->spin[1];
                    dz = (key->next->spin[2] - key->spin[2]) * t + key->spin[2];
                    rotY += (s16) (dx * frames);
                    rotX += (s16) (dy * frames);
                    rotZ += (s16) (dz * frames);
                    break;
                case 2:
                    func_m047_002C4(key, t, NULL, &dx, 0);
                    func_m047_002C4(key, t, NULL, &dy, 1);
                    func_m047_002C4(key, t, NULL, &dz, 2);
                    rotY = func_80049C84(dx, dz) - 0x8000;
                    rotX = func_80049C84(dy, func_80097140(dx * dx + dy * dy + dz * dz));
                    func_m047_002C4(key, t, &dz, NULL, 5);
                    rotZ = dz;
                    break;
                case 3:
                    if (D_801045B0[a->target] != NULL) {
                        other = D_801045B0[a->target]->obj;
                        if (other != NULL) {
                            dx = other->trans.x_position - obj->trans.x_position;
                            dy = other->trans.y_position - obj->trans.y_position;
                            dz = other->trans.z_position - obj->trans.z_position;
                        rotY = func_80049C84(dx, dz) - 0x8000;
                        rotX = func_80049C84(dy, func_80097140(dx * dx + dy * dy + dz * dz));
                            rotZ = 0;
                        }
                    }
                    break;
                default:
                    func_m047_002C4(key, t, &dx, NULL, 3);
                    func_m047_002C4(key, t, &dy, NULL, 4);
                    func_m047_002C4(key, t, &dz, NULL, 5);
                    rotY = dx;
                    rotX = dy;
                    rotZ = dz;
                    break;
            }
            if (a->smoothing != 1.0f) {
                t = 1.0f - func_80049BEC(1.0f - a->smoothing, updateRate);
                obj->trans.rotation.y_rotation = func_m047_007DC(t, obj->trans.rotation.y_rotation, rotY);
                obj->trans.rotation.x_rotation = func_m047_007DC(t, obj->trans.rotation.x_rotation, rotX);
                obj->trans.rotation.z_rotation = func_m047_007DC(t, obj->trans.rotation.z_rotation, rotZ);
            } else {
                obj->trans.rotation.y_rotation = rotY;
                obj->trans.rotation.x_rotation = rotX;
                obj->trans.rotation.z_rotation = rotZ;
            }
            func_m047_01560(a, obj, updateRate);
            func_8005F5AC(obj, updateRate);
        }
    }
}


/* The parts of an object header 013D0 reads */
typedef struct M047Header {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ u16 flags; /* 4: always drawn translucent */
    /* 0x16 */ u8 pad16[0x1E - 0x16];
    /* 0x1E */ s8 noAnim;
} M047Header;

extern Object *D_801045B8;

/* Animates the actor's object, fades its opacity from alphaFrom to alphaTo over fadeLength, and makes it the
   one to follow if flagged (called from 0084C) */
static void func_m047_013D0(M047Actor *a, Object *obj, s32 updateRate) {
    if (!((M047Header *) obj->header)->noAnim) {
        if (a->backwards) {
            func_8001138C(obj, -a->animSpeed, updateRate);
        } else {
            func_8001138C(obj, a->animSpeed, updateRate);
        }
    }
    if (a->fadeTimer < a->fadeLength) {
        a->fadeTimer += updateRate;
        if (a->fadeTimer >= a->fadeLength) {
            a->fadeTimer = a->fadeLength;
        }
        obj->opacity = a->alphaFrom + (a->alphaTo - a->alphaFrom) * a->fadeTimer / a->fadeLength;
        if (obj->opacity < 255) {
            obj->trans.flags |= 4;
        } else if (!(((M047Header *) obj->header)->flags & 4)) {
            obj->trans.flags &= ~4;
        }
    }
    if (a->flags & 2) {
        D_801045B8 = obj;
    }
}

/* The model of a model instance, as 01560 uses it */
typedef struct M047ActorModel {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 texAnims; /* animated textures */
    /* 0x12 */ u8 pad12[0x4E - 0x12];
    /* 0x4E */ s8 animated;
} M047ActorModel;

/* A model instance, as 01560 uses it */
typedef struct M047ActorInst {
    /* 0x00 */ M047ActorModel *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 updateRate; /* set while the object is drawn this frame */
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ void *vertices[2];
} M047ActorInst;

/* What obj->unk60 points at for objects 0x33 */
typedef struct M047Attached {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ Object *obj;
} M047Attached;

extern s32 sprite_anim_advance(void *sprite, s32 *flags, s32 frame, f32 *frameOut, s32 updateRate);
extern void func_8003D21C(M047ActorInst *inst, M047ActorModel *model, Object *obj, f32 scale, f32 arg4);
extern void func_800221F0(Object *obj, M047ActorInst *inst, ObjectShading *shading, void *vertices);
extern void func_8000BC28(Object *obj, M047ActorInst *inst, M047ActorModel *model, s32 arg3, void *arg4, void *arg5,
                          void *arg6, void *arg7);
extern void obj_tex_animate(Object *obj, M047ActorModel *model, M047ActorInst *inst, s32 updateRate);

/* Animates the actor's sprite or model for this frame (called from 0084C) */
static void func_m047_01560(M047Actor *a, Object *obj, s32 updateRate) {
    UNUSED s32 pad0;
    s32 flags;
    UNUSED s32 pad;
    s32 frame;
    M047ActorModel *model;
    M047ActorInst *inst;

    if (((M047Header *) obj->header)->noAnim == 1) {
        void *sprite = obj->modelInstances[obj->modelIndex];

        if (sprite != NULL) {
            frame = a->animSpeed * 60.0f;
            flags = 9;
            if (a->backwards) {
                flags = 11;
            }
            sprite_anim_advance(sprite, &flags, frame, &obj->unk28, updateRate);
        }
    } else if (((M047Header *) obj->header)->noAnim == 0) {
        inst = obj->modelInstances[obj->modelIndex];
        if (inst->updateRate) {
            model = inst->model;
            if (model->animated) {
                func_8003D21C(inst, model, obj, 1.0f, 0.0f);
                func_800221F0(obj, inst, obj->shading, inst->vertices[inst->buffer]);
            } else {
                if (model->texAnims) {
                    inst->buffer = 1 - inst->buffer;
                }
                func_800221F0(obj, inst, obj->shading, inst->vertices[inst->buffer]);
            }
            if (obj->behaviorId == 0x33 && *(M047Attached **) &obj->pad60 != NULL) {
                u8 *v = obj->unk68;

                func_8000BC28(obj, inst, model, *(s32 *) &obj->state[4], v, v + 0x40, v + 0x44, v + 0x48);
                if ((*(M047Attached **) &obj->pad60)->obj != NULL) {
                    M047ActorInst *other =
                        (*(M047Attached **) &obj->pad60)->obj->modelInstances[*(s32 *) &obj->state[4]];

                    if (other != NULL) {
                        other->updateRate = updateRate;
                    }
                }
            }
            if (model->texAnims > 0) {
                obj_tex_animate(obj, model, inst, inst->updateRate);
            }
            inst->updateRate = 0;
        }
    }
}

/*
 * Starts changing a colour to (r, g, b, a) over `time`: an object's light (isLight) or a colour block
 * (target + 0x38).
 */
void func_m047_01798(u8 *target, s32 r, s32 g, s32 b, s32 a, f32 time, s32 isLight) {
    s32 i;
    M047ColourFade *fade;

    if (target != NULL) {
        fade = D_80104708;
        i = 12;
        while (i--) {
            if (fade->target == 0) {
                if (isLight) {
                    fade->target = (s32) target;
                    fade->r0 = target[0x40];
                    fade->g0 = target[0x41];
                    fade->b0 = target[0x42];
                    fade->a0 = target[0x43];
                } else {
                    fade->target = (s32) target & 0x7FFFFFFF;
                    fade->r0 = target[0x38];
                    fade->g0 = target[0x39];
                    fade->b0 = target[0x3A];
                    fade->a0 = target[0x3B];
                }
                fade->r1 = r;
                fade->g1 = g;
                fade->b1 = b;
                fade->a1 = a;
                fade->timer = time * 0.59999996f; /* 0x3F199999 */
                fade->duration = fade->timer;
                return;
            }
            fade++;
        }
    }
}

/* The level entry of the shot spawned (object 0x50, size 0x24) */
typedef struct M047ShotEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ s8 unk13;
    /* 0x14 */ f32 speed;
    /* 0x18 */ Object *owner;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} M047ShotEntry;

/* The level entry of the flash spawned with it (object 0xAC, size 0xC) */
typedef struct M047FlashEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 joint;
    /* 0x0B */ u8 unkB;
} M047FlashEntry;

typedef struct M047ModelHeader {
    /* 0x00 */ u8 pad0[0x2D];
    /* 0x2D */ u8 jointCount;
} M047ModelHeader;

typedef struct M047Model {
    /* 0x00 */ M047ModelHeader *header;
    /* 0x04 */ u8 pad4[0x68 - 0x4];
    /* 0x68 */ Vec3f *joints;
} M047Model;

typedef struct M047Shooter {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ Vec3f muzzle;
} M047Shooter;

extern Object *func_80005F14(void *entry, s32 arg1); /* DKR spawn_object */

/* Fires a shot from joint `joint` of the object's model (or from the object) */
void func_m047_018AC(Object *obj, s32 joint, s32 arg2, s32 speed, s32 arg4, s32 arg5, s32 arg6) {
    M047ShotEntry shot;
    M047FlashEntry flash;
    M047Model *model;
    Object *o;
    UNUSED s32 pad[2];

    model = ((M047Model **) obj->modelInstances)[obj->modelIndex];
    if (model == NULL) {
        return;
    }
    shot.objectID = 0x50;
    shot.size = 0x24;
    shot.unk3 = 0;
    if (obj->behaviorId == 0x33) {
        M047Shooter *s = obj->unk68;
        shot.x = s->muzzle.x;
        shot.y = s->muzzle.y;
        shot.z = s->muzzle.z;
    } else if (joint < model->header->jointCount) {
        shot.x = model->joints[joint].x;
        shot.y = model->joints[joint].y;
        shot.z = model->joints[joint].z;
    } else {
        shot.x = obj->trans.x_position;
        shot.y = obj->trans.y_position;
        shot.z = obj->trans.z_position;
    }
    shot.unk10 = 0;
    shot.unk11 = 0;
    shot.speed = speed;
    shot.owner = obj;
    shot.unk1C = 0;
    shot.unk20 = 0;
    shot.unk12 = 0;
    shot.unkA = arg2;
    shot.unkB = arg6;
    shot.unkC = arg4;
    shot.unkE = arg5;
    shot.unk13 = -1;
    o = func_80005F14(&shot, 1);
    if (o != NULL) {
        o->level_entry = NULL;
    }
    flash.objectID = 0xAC;
    flash.size = 0xC;
    flash.x = shot.x;
    flash.y = shot.y;
    flash.z = shot.z;
    flash.joint = joint;
    if (obj->behaviorId != 0x33 || arg2 == 1) {
        flash.unkB = 0;
    } else {
        flash.unkB = 2;
    }
    o = func_80005F14(&flash, 1);
    if (o != NULL) {
        o->level_entry = NULL;
        OBJ_STATE(o, Object *)[0] = obj;
        OBJ_STATE(o, s32)[1] = 1;
    }
}

extern s16 rl_m047_025A0[6][4]; /* .data: the rotation of the effect, per kind of object */

extern void mtx_model_push(Gfx **dl, s32 arg1, MtxF *mtx);
extern void mtx_model_rotate_translate_push(Gfx **dl, s32 arg1, void *arg2, MtxF *mtx);
extern void func_80048F90(MtxF *out, ObjectTransform *trans); /* the matrix of a transform */

/* Pushes the matrices for the effect drawn on the object, turned the way its kind needs */
void func_m047_01BAC(Gfx **dl, s32 arg1, Object *obj) {
    ObjectTransform trans;
    MtxF m;
    s16 *angles;
    u8 *data;

    data = obj->unk68;
    switch (obj->objectID) {
        case 0x2A3:
            angles = rl_m047_025A0[5];
            break;
        case 0x2A2:
            angles = rl_m047_025A0[4];
            break;
        case 0x2A1:
            angles = rl_m047_025A0[3];
            break;
        case 0xC4:
        case 0x11A:
        case 0x1B6:
        case 0x1F6:
        case 0x2BA:
            angles = rl_m047_025A0[2];
            break;
        case 0xC6:
        case 0x119:
        case 0x1B5:
        case 0x1F4:
        case 0x204:
        case 0x2B9:
            angles = rl_m047_025A0[1];
            break;
        default:
            angles = rl_m047_025A0[0];
            break;
    }
    mtx_model_push(dl, arg1, (MtxF *) data);
    trans.rotation.y_rotation = angles[0];
    trans.rotation.x_rotation = angles[1];
    trans.rotation.z_rotation = angles[2];
    trans.x_position = 0.0f;
    trans.y_position = 0.0f;
    trans.z_position = 0.0f;
    trans.scale = 1.0f;
    func_80048F90(&m, &trans);
    mtx_model_rotate_translate_push(dl, arg1, data + 0x40, &m);
}

extern void func_m106_01C04(f32 x, f32 y, f32 z, s16 yRotation, s16 xRotation, f32 speed, f32 size);
extern void func_m008_001CC(f32 x, f32 y, f32 z, f32 vx, f32 vy, f32 vz, s32 alpha, s32 kind, f32 scale);

/*
 * Spawns `count` particles at actor `index`'s object, aimed (yRot, xRot) give or take (yRange, xRange) at
 * speed `speed` give or take `speedRange`: kind 0xFF one sort (bigger with game flag 0x59 clear), else
 * the other sort, kinds 0-4.
 */
void func_m047_01D38(s32 index, s32 count, s32 kind, s32 yRot, s32 xRot, s32 yRange, s32 xRange, s32 speed,
                     s32 speedRange) {
    s32 alpha;
    Vec3s angles;
    f32 scale;
    Vec3f vel;
    Object *obj;
    M047Actor *actor;

    actor = D_801045B0[index];
    scale = 1.0f;
    if (actor != NULL) {
        obj = actor->obj;
        if (obj != NULL) {
            if (kind == 0xFF) {
                if (game_flag_get(0x59)) {
                    scale = 5.0f;
                } else {
                    scale = 40.0f;
                }
                while (count--) {
                    s32 y = func_80048E2C(-yRange, yRange);
                    s32 x = func_80048E2C(-xRange, xRange);

                    func_m106_01C04(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, y + yRot,
                                    x + xRot, (func_80048E2C(-speedRange, speedRange) + speed) * -0.01f, scale);
                }
            } else {
                switch (kind) {
                    case 0:
                        kind = 0;
                        break;
                    case 1:
                        kind = 1;
                        break;
                    case 2:
                        kind = 1;
                        scale = 0.5f;
                        break;
                    case 3:
                        kind = 5;
                        scale = 0.75f;
                        break;
                    case 4:
                        kind = 4;
                        break;
                    default:
                        kind = 2;
                        break;
                }
                if (obj->shading != NULL) {
                    alpha = obj->shading->unk0 * 255.0f;
                } else {
                    alpha = 255;
                }
                while (count--) {
                    angles.y_rotation = func_80048E2C(-yRange, yRange) + yRot;
                    angles.x_rotation = func_80048E2C(-xRange, xRange) + xRot;
                    vel.z = (func_80048E2C(-speedRange, speedRange) + speed) * -0.01f;
                    func_8004944C(&angles, &vel); /* (vel.x and vel.y unset, as in the original) */
                    func_m008_001CC(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, vel.x, vel.y,
                                    vel.z, alpha, kind, scale);
                }
            }
        }
    }
}

/* Clears the subtitles */
void func_m047_0206C(void) {
    s32 i;
    M047Subtitle *c;

    for (i = 0, c = sSubtitles; i < 5; i++, c++) {
        c->alpha = 0;
        c->timer = 0;
    }
}

/* Shows text `text` at (x, y) in colour (r, g, b) for `timer` frames, in the first free subtitle */
void func_m047_020AC(s32 text, s32 timer, s32 x, s32 y, s32 r, s32 g, s32 b) {
    s32 i;
    M047Subtitle *c;

    for (i = 0, c = sSubtitles; i < 5; i++, c++) {
        if (c->alpha == 0 && c->timer <= 0) {
            c->text = text;
            c->timer = timer;
            c->x = x;
            c->y = y;
            c->r = r;
            c->g = g;
            c->b = b;
            c->alpha = 0;
            return;
        }
    }
}

/* Counts the subtitles' timers down, fading them in while they run and out after */
void func_m047_0226C(s32 updateRate) {
    s32 i;
    s32 alpha;
    M047Subtitle *c;

    c = sSubtitles;
    for (i = 0; i < 5; i++) {
        if (c->timer > 0 || c->alpha > 0) {
            if (c->timer > 0) {
                c->timer -= updateRate;
                alpha = c->alpha + updateRate * 8;
                if (alpha >= 256) {
                    c->alpha = 255;
                } else {
                    c->alpha = alpha;
                }
            } else if (c->alpha > 0) {
                alpha = c->alpha - updateRate * 8;
                if (alpha < 0) {
                    c->alpha = 0;
                } else {
                    c->alpha = alpha;
                }
            }
        }
        c++;
    }
}

/* Draws the subtitles: a shaded box, the text's shadow and the text */
void func_m047_023FC(Gfx **dl) {
    s32 i;
    s32 width;
    s32 x;
    s32 y;
    M047Subtitle *c;

    c = sSubtitles;
    func_8006FC54(2);
    func_8006FC8C(0, 0, 0, 0);
    for (i = 0; i < 5; i++, c++) {
        if (c->alpha > 0) {
            width = func_800706D4(gMenuText[c->text], 2, 1) + 40;
            x = c->x - (width >> 1);
            y = c->y - 12;
            func_8006FC68(0, 0, 0, 255, c->alpha);
            func_m157_00000(dl, 0, x, y, width, 24, c->alpha >> 1);
            func_8006FCA8(dl, c->x + 1, c->y + 1, gMenuText[c->text], 12);
            func_8006FC68(c->r, c->g, c->b, 255, c->alpha);
            func_8006FCA8(dl, c->x, c->y, gMenuText[c->text], 12);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m047", data);

