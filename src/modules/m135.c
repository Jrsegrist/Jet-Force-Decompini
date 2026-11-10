/*
 * Runlink module 135: the object behaviours of the multiplayer menu room: the
 * room camera and the point it aims at, the second camera on the screen, and
 * the player selectors (their mode, sounds and joint angles).
 */
#include "module.h"

/* What the aim point keeps: the object nearest to it that it follows */
typedef struct M135AnimPath {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ f32 speed;
} M135AnimPath;

/* The aim point's behaviour data */
typedef struct M135Aim {
    /* 0x00 */ Object *target;
    /* 0x04 */ M135AnimPath *path;
} M135Aim;

/*
 * The aim point's level entry. The aim points are the room camera's targets:
 * the camera reads the rest (the animations of the object followed, the view
 * and the part of the screen the screen camera shows).
 */
typedef struct M135AimEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 animSelect;  /* when chosen */
    /* 0x0B */ u8 speedSelect;
    /* 0x0C */ u8 anim;
    /* 0x0D */ u8 speed; /* in 1/255ths */
    /* 0x0E */ u8 index; /* in the camera's targets */
    /* 0x0F */ u8 padF;
    /* 0x10 */ s16 yaw;
    /* 0x12 */ s16 pitch;
    /* 0x14 */ s16 swing;
    /* 0x16 */ s16 x; /* the part of the screen the screen camera shows */
    /* 0x18 */ s16 y;
    /* 0x1A */ s16 size;
    /* 0x1C */ u8 animConfirm;
    /* 0x1D */ u8 speedConfirm;
} M135AimEntry;

/* An object header, as the aim point reads it */
typedef struct M135Header {
    /* 0x00 */ u8 pad0[0x1B];
    /* 0x1B */ u8 kind;
} M135Header;

/* The camera's behaviour data */
typedef struct M135Camera {
    /* 0x00 */ s32 flags; /* 1: turning backwards, 2: a target chosen, 4: confirmed */
    /* 0x04 */ Object *targets[8];
    /* 0x24 */ f32 zoom; /* 0 (the room) .. 1 (the target) */
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 confirm;
    /* 0x30 */ s32 fogNear; /* the fog, << 20 */
    /* 0x34 */ s32 fogFar;
    /* 0x38 */ s32 fogR;
    /* 0x3C */ s32 fogG;
    /* 0x40 */ s32 fogB;
    /* 0x44 */ s32 sound; /* a playing sound's handle, or 0 */
    /* 0x48 */ s16 unk48;
    /* 0x4A */ s16 unk4A;
    /* 0x4C */ s16 yaw;
    /* 0x4E */ s16 pitch;
    /* 0x50 */ u8 unk50;
    /* 0x51 */ s8 target; /* the one looked at */
    /* 0x52 */ s8 lastTarget;
    /* 0x53 */ s8 delay;
    /* 0x54 */ s8 unk54;
    /* 0x55 */ s8 unk55;
    /* 0x56 */ u8 soundOn;
} M135Camera;

/* The camera's level entry */
typedef struct M135CamEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u16 targetDist;
    /* 0x0C */ u16 dist;
    /* 0x0E */ s16 targetHeight;
    /* 0x10 */ s16 height;
    /* 0x12 */ s16 dist2;
    /* 0x14 */ s16 height2;
    /* 0x16 */ s16 lookHeight;
    /* 0x18 */ u8 fogR;
    /* 0x19 */ u8 fogG;
    /* 0x1A */ u8 fogB;
    /* 0x1B */ u8 pad1B;
    /* 0x1C */ s16 fogNear;
    /* 0x1E */ s16 fogFar;
    /* 0x20 */ s16 unk20;
} M135CamEntry;

/* The level, as the camera reads it */
typedef struct M135Level {
    /* 0x00 */ u8 pad0[0x5A];
    /* 0x5A */ s16 fogNear;
    /* 0x5C */ s16 fogFar;
    /* 0x5E */ u8 pad5E[2];
    /* 0x60 */ u8 fogR;
    /* 0x61 */ u8 fogG;
    /* 0x62 */ u8 fogB;
    /* 0x63 */ u8 pad63[0xAC - 0x63];
    /* 0xAC */ s8 fov;
} M135Level;

/* What objects of behaviour 0x23 keep in unk68, as the camera sees it */
typedef struct M135Other {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ u8 pad2[2];
    /* 0x4 */ s16 unk4;
    /* 0x6 */ s16 unk6;
} M135Other;

extern M135Level *func_8004470C(void); /* DKR levelGetLevel */
extern void func_8001BC2C(s32 player, s16 *near, s16 *far, s16 *arg3, u8 *r, u8 *g, u8 *b, s8 *arg7); /* DKR get_fog_settings */
extern void func_8001BB30(s32 player, s16 near, s16 far, s16 arg3, s32 r, s32 g, s32 b, s32 arg7);
extern s32 func_80043558(s32 player); /* the stick's x */
extern s32 func_80043510(s32 player); /* the buttons pressed */
extern void set_active_camera(s32 camera);
extern ObjectTransform *func_80041DE0(void); /* the camera */
extern f32 func_8003F7C4(void);              /* the field of view */
extern void func_8003F7F4(f32 fov, s32 arg1); /* sets the field of view */

extern void *get_settings(void);
extern f32 func_80097140(f32 x);        /* sqrtf */
extern s32 func_80049C84(f32 a, f32 b); /* the angle of (a, b) */
extern s32 rl_m135_02A78[]; /* per target: the flag it needs, and one that hides it */
extern s32 rl_m135_02A84[];
extern M135AnimPath *func_80079AA4(Object *obj);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s16 angle_lerp(s16 from, s16 to, f32 t);
extern u16 *gVideoLastFramebuffer;
extern void func_800013FC(s32 sound, s32 handle, u8 volume);
extern void func_80001500(s32 handle, f32 pitch);

/* The screen camera's behaviour data */
typedef struct M135ScreenCam {
    /* 0x00 */ Object *camera; /* the room camera */
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 size;
    /* 0x0A */ s16 timer; /* until the next interference */
    /* 0x0C */ s16 noise;
    /* 0x0E */ u8 padE[2];
    /* 0x10 */ s32 sound;
} M135ScreenCam;

/* The texture the screen camera draws into */
typedef struct M135ScreenTex {
    /* 0x00 */ u8 width;
    /* 0x01 */ u8 height;
    /* 0x02 */ u8 pad2[0x20 - 0x2];
    /* 0x20 */ u16 pixels[1];
} M135ScreenTex;

typedef struct M135ScreenTexEntry {
    /* 0x0 */ M135ScreenTex *texture;
} M135ScreenTexEntry;

typedef struct M135ScreenModel {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M135ScreenTexEntry *textures;
} M135ScreenModel;

/* A player selector's behaviour data */
typedef struct M135Selector {
    /* 0x00 */ s32 sound;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 pad11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 mode;
    /* 0x14 */ s8 unk14;
    /* 0x15 */ u8 pad15;
    /* 0x16 */ s8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 soundOn;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 unk1A;
} M135Selector;

/* A player selector's level entry */
typedef struct M135SelectorEntry {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
} M135SelectorEntry;

/* The selectors' model and frame per selection (the module's .data, still assembly) */
typedef struct M135Frame {
    /* 0x0 */ u8 pad0[2];
    /* 0x2 */ u8 model;
    /* 0x3 */ u8 frame;
    /* 0x4 */ u8 pad4[4];
} M135Frame;
extern M135Frame rl_m135_029D0[];
extern s16 rl_m135_02A68[];

extern void sound_stop(s32 handle); /* stops a sound */

static s16 sJointAngles[16];

void func_m135_00000(Object *obj, LevelObjectEntryCommon *entry) {
    M135Aim *aim = obj->unk68;

    aim->target = NULL;
}

/*
 * The module's .rodata can only be C once 002BC matches (all of it is float literals of 00010, 002BC, 01A04 and
 * 021F8), so 00010 and 021F8 (which match with it in C) are C only under NON_MATCHING, and 01A04 reads its
 * float from the assembly .rodata in the matching build.
 */
#ifdef NON_MATCHING
#define SCREEN_TURN_RATE 0.98f
#else
extern f32 rl_m135_02AC4; /* 0.98f */
#define SCREEN_TURN_RATE rl_m135_02AC4
#endif

#ifdef NON_MATCHING
/* Matches (needs the module's .rodata in C: the 1e9 literal twice) */
/* The aim point: finds the nearest object of kind 8 and follows it */
void func_m135_00010(Object *obj, s32 updateRate) {
    M135Aim *aim;
    f32 best;
    Object **objs;
    s32 i;
    Object *o;
    f32 d;
    s32 first;
    s32 count;
    f32 dx;
    f32 dy;
    f32 dz;
    M135AimEntry *entry;
    M135AnimPath *path;

    aim = obj->unk68;
    entry = (M135AimEntry *) obj->level_entry;
    if (aim->target == NULL) {
        best = 1e9f;
        objs = func_80005E98(&first, &count);
        for (i = first; i < count; i++) {
            o = objs[i];
            if (((M135Header *) o->header)->kind == 8) {
                dx = obj->trans.x_position - o->trans.x_position;
                dy = obj->trans.y_position - o->trans.y_position;
                dz = obj->trans.z_position - o->trans.z_position;
                d = dx * dx + dy * dy + dz * dz;
                if (d < best) {
                    aim->target = o;
                    best = d;
                }
            }
        }
        if (aim->target != NULL) {
            path = func_80079AA4(aim->target);
            aim->path = path;
            obj_anim_set(aim->target, entry->anim, 0);
            path->speed = entry->speed / 255.0f;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_00010);
#endif

void func_m135_00298(Object *obj, LevelObjectEntryCommon *entry) {
    M135Camera *cam = obj->unk68;

    cam->unk4A = 0;
    cam->unk50 = 0;
    cam->unk54 = 40;
    cam->unk28 = 0.0f;
}

s32 func_m135_01754(Object *obj, s32 dir);

#ifdef NON_MATCHING
/* The fog call's arguments: the ROM copies the shifted near, R and far values back into v0/t3/v1 (31 words) */
/* The room camera's control: chooses and confirms the targets, and moves the camera, its fog and its view */
void func_m135_002BC(Object *obj, s32 updateRate) {
    f32 fov;
    f32 f;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 targetFov;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x0;
    f32 y0;
    f32 z0;
    f32 dist;
    s16 yaw1;
    s16 pitch3;
    s16 pitch1;
    s16 yaw2;
    s16 yaw3;
    s16 pitch2;
    s32 near;
    s32 far;
    s32 r;
    s32 g;
    s32 b;
    s32 cur;
    s32 goal;
    s32 x;
    s32 y;
    s32 z;
    s16 fogNear;
    s16 fogFar;
    u8 fogR;
    u8 fogG;
    u8 fogB;
    s32 first;
    s32 count;
    s32 j;
    s32 i;
    Object **objs;
    M135Other *other;
    Object *o;
    M135CamEntry *camEntry;
    UNUSED s32 pad8C;
    Object *target;
    UNUSED s32 pad84;
    M135Camera *cam;
    UNUSED s32 pad7C;
    M135AimEntry *aimEntry;
    M135Aim *aim;
    ObjectTransform *camT;
    M135Level *level;

    cam = obj->unk68;
    camEntry = (M135CamEntry *) obj->level_entry;
    level = func_8004470C();
    if (!cam->unk50) {
        objs = func_80005E98(&first, &count);
        for (j = first; j < count; j++) {
            o = objs[j];
            if (o->behaviorId == 0x47) {
                aimEntry = (M135AimEntry *) o->level_entry;
                aim = o->unk68;
                cam->targets[aimEntry->index] = o;
                if (cam->lastTarget < aimEntry->index) {
                    cam->lastTarget = aimEntry->index;
                }
            }
        }
        cam->unk50 = 1;
        func_m135_01754(obj, 0);
        func_8001BC2C(0, &fogNear, &fogFar, &cam->unk48, &fogR, &fogG, &fogB, &cam->unk55);
        cam->fogNear = fogNear << 20;
        cam->fogFar = fogFar << 20;
        cam->fogR = fogR << 20;
        cam->fogG = fogG << 20;
        cam->fogB = fogB << 20;
    }
    if (cam->delay > 0) {
        cam->delay -= updateRate;
    } else if (!(cam->flags & 2)) {
        if (func_80043558(0) < -32 || (func_80043510(0) & 0x20)) {
            if (func_m135_01754(obj, 1)) {
                cam->delay = 20;
                cam->unk54 = 40;
            }
        } else if (func_80043558(0) > 32 || (func_80043510(0) & 0x10)) {
            if (func_m135_01754(obj, 0)) {
                cam->delay = 20;
                cam->unk54 = 40;
            }
        } else if (func_80043558(0) >= -7 && func_80043558(0) < 8) {
            cam->delay = 0;
        }
    }
    if (cam->targets[cam->target] != NULL) {
        aim = cam->targets[cam->target]->unk68;
        aimEntry = (M135AimEntry *) cam->targets[cam->target]->level_entry;
        target = aim->target;
    } else {
        target = NULL;
    }
    if (aim->path != NULL) {
        if (func_80043510(0) & 0x8000) {
            if (cam->flags & 2) {
                if (!(cam->flags & 4)) {
                    cam->flags |= 4;
                    obj_anim_set(target, aimEntry->animConfirm, 0.0f);
                    aim->path->speed = aimEntry->speedConfirm / 255.0f;
                    objs = func_80005E98(&first, &count);
                    for (j = first; j < count; j++) {
                        o = objs[j];
                        if (o->behaviorId == 0x23) {
                            other = o->unk68;
                            if (other->unk0 == camEntry->unk20) {
                                other->unk4 = 0;
                                other->unk6 = 0;
                            }
                        }
                    }
                }
            } else {
                cam->flags |= 2;
                if (target != NULL && target->animationID != aimEntry->animSelect) {
                    obj_anim_set(target, aimEntry->animSelect, 0.0f);
                }
                aim->path->speed = aimEntry->speedSelect / 255.0f;
            }
        } else if (func_80043510(0) & 0x4000) {
            if (!(cam->flags & 4)) {
                cam->flags &= ~2;
                if (target != NULL && target->animationID != aimEntry->anim) {
                    obj_anim_set(target, aimEntry->anim, 0.0f);
                }
                aim->path->speed = aimEntry->speed / 255.0f;
            }
        }
    }
    if (cam->flags & 2) {
        for (i = 0; i < updateRate; i++) {
            cam->zoom += (1.0f - cam->zoom) * 0.05f;
        }
    } else {
        for (i = 0; i < updateRate; i++) {
            cam->zoom += (0.0f - cam->zoom) * 0.05f;
        }
    }
    if (cam->flags & 4) {
        for (i = 0; i < updateRate; i++) {
            cam->confirm += (1.0f - cam->confirm) * 0.05f;
        }
    }
    set_active_camera(0);
    camT = func_80041DE0();
    if (cam->unk54 > 0) {
        cam->unk54 -= updateRate;
        for (i = 0; i < updateRate; i++) {
            cam->unk28 += (0.0f - cam->unk28) * 0.05f;
        }
    } else {
        for (i = 0; i < updateRate; i++) {
            cam->unk28 += (1.0f - cam->unk28) * 0.01f;
        }
    }
    dist = camEntry->dist + (camEntry->dist2 - camEntry->dist) * cam->unk28;
    x0 = func_80049C34(cam->unk4A) * dist;
    y0 = camEntry->height + (camEntry->height2 - camEntry->height) * cam->unk28;
    z0 = func_80049C30(cam->unk4A) * dist;
    o = cam->targets[cam->target];
    aimEntry = (M135AimEntry *) o->level_entry;
    x1 = func_80049C34(cam->unk4A + (1.0f - cam->confirm) * aimEntry->swing) * camEntry->targetDist;
    y1 = camEntry->targetHeight;
    z1 = func_80049C30(cam->unk4A + (1.0f - cam->confirm) * aimEntry->swing) * camEntry->targetDist;
    camT->x_position = cam->zoom * (x1 - x0) + x0 + obj->trans.x_position;
    camT->y_position = cam->zoom * (y1 - y0) + y0 + obj->trans.y_position;
    camT->z_position = cam->zoom * (z1 - z0) + z0 + obj->trans.z_position;
    fov = func_8003F7C4();
    if (!(cam->flags & 2)) {
        r = level->fogR << 20;
        g = level->fogG << 20;
        b = level->fogB << 20;
        near = level->fogNear << 20;
        far = level->fogFar << 20;
        targetFov = level->fov;
    } else {
        r = camEntry->fogR << 20;
        targetFov = 45.0f;
        g = camEntry->fogG << 20;
        b = camEntry->fogB << 20;
        near = camEntry->fogNear << 20;
        far = camEntry->fogFar << 20;
    }
    for (i = 0; i < updateRate; i++) {
        cam->fogNear += (near - cam->fogNear) >> 5;
        cam->fogFar += (far - cam->fogFar) >> 5;
        cam->fogR += (r - cam->fogR) >> 5;
        cam->fogG += (g - cam->fogG) >> 5;
        fov += (targetFov - fov) * 0.125f;
        cam->fogB += (b - cam->fogB) >> 5;
    }
    g = cam->fogG;
    b = cam->fogB;
    func_8001BB30(0, x = cam->fogNear >> 20, z = cam->fogFar >> 20, cam->unk48, y = cam->fogR >> 20, g >> 20, b >> 20,
                  cam->unk55);
    func_8003F7F4(fov, 1);
    yaw1 = aimEntry->yaw;
    pitch1 = aimEntry->pitch;
    aim = o->unk68;
    o = aim->target;
    if (o == NULL) {
        o = cam->targets[cam->target];
    }
    dx = o->trans.x_position - camT->x_position;
    dy = o->trans.y_position - camT->y_position;
    dz = o->trans.z_position - camT->z_position;
    yaw1 -= func_80049C84(dx, dz);
    pitch1 -= func_80049C84(dy, func_80097140(dx * dx + dy * dy + dz * dz));
    dx = obj->trans.x_position - camT->x_position;
    dy = (obj->trans.y_position + camEntry->lookHeight) - camT->y_position;
    dz = obj->trans.z_position - camT->z_position;
    yaw2 = -func_80049C84(dx, dz);
    pitch2 = -func_80049C84(dy, func_80097140(dx * dx + dy * dy + dz * dz));
    yaw3 = angle_lerp(yaw2, yaw1, cam->zoom);
    pitch3 = angle_lerp(pitch2, pitch1, cam->zoom);
    camT->rotation.y_rotation = angle_lerp(yaw3, yaw2, cam->confirm);
    camT->rotation.x_rotation = pitch3;
    cur = cam->unk4A;
    goal = cam->yaw;
    switch (cam->flags & 1) {
        case 1:
            if (cur < goal) {
                goal -= 0x10000;
            }
            break;
        case 0:
            if (goal < cur) {
                goal += 0x10000;
            }
            break;
    }
    for (i = 0; i < updateRate; i++) {
        cur += (goal - cur) >> 5;
    }
    cam->unk4A = cur;
    i = goal - cur;
    if (i < 0) {
        i = -i;
    }
    if (i < 100) {
        if (cam->soundOn) {
            if (cam->sound != 0) {
                sound_stop(cam->sound);
            }
            func_80001248(0x21, 0);
            cam->soundOn = FALSE;
        }
    } else {
        cam->soundOn = TRUE;
        if (cam->sound == 0) {
            func_80001248(0x23, (s32) &cam->sound);
        }
        if (cam->sound != 0) {
            f = (f32) i / 5000.0f;
            if (f > 1.0f) {
                f = 1.0f;
            }
            f += 0.25f;
            func_80001500(cam->sound, f);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_002BC);
#endif

/* Turns the camera to the next target that can be shown (dir 0: forwards); TRUE if it changed */
s32 func_m135_01754(Object *obj, s32 dir) {
    s32 prev;
    f32 dx;
    f32 dy;
    f32 dz;
    M135Camera *cam = obj->unk68;
    Object *t;

    get_settings();
    prev = cam->target;
    if (cam->lastTarget >= 0) {
        if (dir == 0) {
        do {
            cam->target++;
            if (cam->lastTarget < cam->target) {
                cam->target = 0;
            }
        } while (cam->targets[cam->target] == NULL || !game_flag_get(rl_m135_02A78[cam->target]) ||
                 game_flag_get(rl_m135_02A84[cam->target]));
        } else {
            do {
                cam->target--;
                if (cam->target < 0) {
                    cam->target = cam->lastTarget;
                }
            } while (cam->targets[cam->target] == NULL || !game_flag_get(rl_m135_02A78[cam->target]) ||
                     game_flag_get(rl_m135_02A84[cam->target]));
        }
    } else {
        cam->target = 0;
    }
    t = cam->targets[cam->target];
    if (t != NULL) {
        dx = t->trans.x_position - obj->trans.x_position;
        dy = t->trans.y_position - obj->trans.y_position;
        dz = t->trans.z_position - obj->trans.z_position;
        cam->yaw = func_80049C84(dx, dz);
        cam->pitch = -func_80049C84(dy, func_80097140(dx * dx + dy * dy + dz * dz));
        if (prev != cam->target) {
            cam->flags &= ~1;
            cam->flags |= dir;
        }
    }
    return prev != cam->target;
}

void func_m135_019CC(Object *obj) {
    M135Camera *cam = obj->unk68;

    if (cam->sound != 0) {
        sound_stop(cam->sound);
    }
}

Object *func_m135_0205C(void);

/* The screen camera: copies the part of the frame its target names into its texture, with interference */
void func_m135_01A04(Object *obj, s32 updateRate) {
    M135ScreenCam *sc;
    M135AimEntry *src;
    M135ScreenTex *tex;
    s32 row;
    s32 col;
    s32 flip;
    f32 fx;
    f32 stepX;
    f32 stepY;
    s32 h;
    u16 *line;
    u16 *dst;
    s32 n;
    u16 c;
    M135Camera *cam;
    s32 vol;
    M135ScreenModel **inst;

    sc = obj->unk68;
    src = NULL;
    if (sc->camera == NULL) {
        sc->camera = func_m135_0205C();
        sc->timer = func_80048E2C(60, 90);
    }
    if (sc->camera != NULL) {
        cam = sc->camera->unk68;
        if (cam->unk54 <= 0) {
            obj->trans.rotation.y_rotation = angle_lerp(obj->trans.rotation.y_rotation, cam->yaw + 0x8000,
                                                        1.0f - func_80049BEC(SCREEN_TURN_RATE, updateRate));
        }
        src = (M135AimEntry *) cam->targets[cam->target]->level_entry;
    }
    flip = 0;
    if (src != NULL) {
        if (sc->size == 0) {
            sc->x = src->x;
            sc->y = src->y;
            sc->size = src->size;
        } else {
            if (sc->x < src->x) {
                sc->x += updateRate;
                if (src->x < sc->x) {
                    sc->x = src->x;
                }
            } else if (src->x < sc->x) {
                sc->x -= updateRate;
                if (sc->x < src->x) {
                    sc->x = src->x;
                }
            }
            if (sc->y < src->y) {
                sc->y += updateRate;
                if (src->y < sc->y) {
                    sc->y = src->y;
                }
            } else if (src->y < sc->y) {
                sc->y -= updateRate;
                if (sc->y < src->y) {
                    sc->y = src->y;
                }
            }
            if (sc->size < src->size) {
                sc->size += updateRate;
                if (src->size < sc->size) {
                    sc->size = src->size;
                }
            } else if (src->size < sc->size) {
                sc->size -= updateRate;
                if (sc->size < src->size) {
                    sc->size = src->size;
                }
            }
        }
        inst = obj->modelInstances[0];
        tex = (*inst)->textures[0].texture;
        h = (sc->size * 3) / 4;
        stepX = (f32) sc->size / (f32) tex->width;
        stepY = (f32) h / (f32) tex->height;
        for (row = tex->height - 1; row >= 0; row--) {
            line = gVideoLastFramebuffer + (s32) (row * stepY + sc->y) * 320 + sc->x;
            dst = tex->pixels + tex->width * ((tex->height - row) - 1);
            fx = 0.0f;
            for (col = 0; col < tex->width; col++) {
                dst[col ^ flip] = line[(s32) fx];
                fx += stepX;
            }
            flip ^= 2;
        }
        if (sc->noise > 0) {
            n = (tex->width * tex->height * sc->noise) / 100;
            while (n != 0) {
                row = func_80048E2C(0, tex->height - 1);
                col = func_80048E2C(0, tex->width - 1);
                dst = tex->pixels + tex->width * ((tex->height - row) - 1) + col;
                c = func_80048E2C(0, 0x80) * 2;
                *dst = c | (c << 5) | (c << 10) | 1;
                n--;
            }
            if (sc->noise < updateRate) {
                sc->timer = func_80048E2C(30, 90);
                sc->noise = 0;
            } else {
                sc->noise -= updateRate;
            }
            if (cam->flags & 2) {
                if (sc->sound != 0) {
                    sound_stop(sc->sound);
                }
            } else {
                if (sc->sound == 0) {
                    func_80001248(0xCF, (s32) &sc->sound);
                }
                if (sc->sound != 0) {
                    vol = sc->noise * 2;
                    if (vol >= 0x80) {
                        vol = 0x7F;
                    }
                    func_800013FC(0xCF, sc->sound, vol);
                }
            }
        } else {
            if (sc->sound != 0) {
                sound_stop(sc->sound);
            }
            if (sc->timer < updateRate) {
                sc->noise = func_80048E2C(20, 75);
            } else {
                sc->timer -= updateRate;
            }
        }
    }
}

void func_m135_02024(Object *obj) {
    M135ScreenCam *cam = obj->unk68;

    if (cam->sound != 0) {
        sound_stop(cam->sound);
    }
}

/* The first object with behaviour 0x46 (the menu camera), or NULL */
Object *func_m135_0205C(void) {
    s32 i;
    Object **objs;
    Object *obj;
    s32 first;
    s32 count;

    objs = func_80005E98(&first, &count);
    for (i = first; i < count; i++) {
        obj = objs[i];
        if (obj->behaviorId == 0x46) {
            return obj;
        }
    }
    return NULL;
}

void func_m135_02188(Object *obj, M135SelectorEntry *entry) {
    M135Selector *sel = obj->unk68;

    sel->unk4 = entry->unkA;
    sel->unk6 = entry->unkC;
    sel->unkC = entry->unk10;
    sel->unkA = entry->unk12;
    sel->unk8 = sel->unkA;
    sel->unk12 = entry->unkF;
    sel->unk10 = entry->unkE;
    obj->modelIndex = entry->unkE;
    obj->trans.rotation.y_rotation = sel->unk4;
}

#ifdef NON_MATCHING
static void func_m135_02834(Object *obj, s32 frame, s32 frames);

/* Matches (needs the module's .rodata in C: with the assembly 5000.0f its FPRs differ) */
/* A player selector's control: turns it, plays its turning sound and sets its model and frame */
void func_m135_021F8(Object *obj, s32 updateRate) {
    M135Selector *sel;
    s32 prev;
    s32 i;
    s32 update;
    f32 f;
    s32 v;

    sel = obj->unk68;
    prev = obj->trans.rotation.y_rotation;
    switch (sel->mode) {
        case 2:
        case 1:
            v = sel->unk8;
            if (sel->unk16 == -1 && v < sel->unkA) {
                v += 0x10000;
            } else if (sel->unk16 == 1 && sel->unkA < v) {
                v -= 0x10000;
            }
            for (i = 0; i < updateRate; i++) {
                obj->trans.rotation.y_rotation += (sel->unk6 - obj->trans.rotation.y_rotation) >> 3;
                v += (sel->unkA - v) >> 4;
            }
            sel->unk8 = v;
            break;
        case 0:
            for (i = 0; i < updateRate; i++) {
                obj->trans.rotation.y_rotation += (sel->unk4 - obj->trans.rotation.y_rotation) >> 4;
                sel->unk8 -= sel->unk8 >> 1;
            }
            break;
    }
    v = (obj->trans.rotation.y_rotation - prev) / updateRate;
    if (v < 0) {
        v = -v;
    }
    if (v < 100) {
        if (sel->soundOn) {
            if (sel->sound != 0) {
                sound_stop(sel->sound);
            }
            func_80001248(0x21, 0);
            sel->soundOn = FALSE;
        }
    } else {
        sel->soundOn = TRUE;
        if (sel->sound == 0) {
            func_80001248(0x23, (s32) &sel->sound);
        }
        if (sel->sound != 0) {
            f = (f32) v / 5000.0f;
            if (f > 1.0f) {
                f = 1.0f;
            }
            f += 0.25f;
            func_80001500(sel->sound, f);
        }
    }
    v = sel->unk8;
    update = FALSE;
    if (v < 0) {
        v += 0x8000;
    }
    prev = sel->unkE;
    if (prev < 0) {
        prev += 0x8000;
    }
    if ((sel->unk16 == 1 && !(sel->unkC < prev) && sel->unkC < v) ||
        (sel->unk16 == -1 && sel->unkC < prev && !(sel->unkC < v)) || sel->unk17) {
        update = TRUE;
    }
    if (sel->unk1A == 1) {
        update = TRUE;
        sel->unk1A = 2;
    }
    if (update) {
        if (sel->unk19) {
            v = rl_m135_02A68[sel->unk14 & 7];
            prev = 0;
        } else {
            prev = rl_m135_029D0[sel->unk14].model;
            v = rl_m135_029D0[sel->unk14].frame;
            if (sel->mode == 2) {
                v++;
            }
        }
        obj->modelIndex = (obj->modelIndex & 3) + prev;
        func_m135_02834(obj, v << 8, 14);
    }
    sel->unk17 = 0;
    sel->unkE = sel->unk8;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m135", func_m135_021F8);
#endif

void func_m135_0270C(Object *obj, s32 mode, s32 arg2, s32 turn, s32 arg4) {
    M135Selector *sel = obj->unk68;

    if (turn == 1) {
        sel->unk16 = -1;
        sel->unkA -= 0x8000;
    } else if (turn == 2) {
        sel->unk16 = 1;
        sel->unkA += 0x8000;
    }
    if (sel->mode == 1 && mode == 0) {
        if (sel->unk8 < 0) {
            sel->unk8 += 0x8000;
        }
    }
    if (mode == 2 && sel->mode == 1) {
        sel->unk17 = 1;
        func_80001248(0xFD, 0);
    } else if (mode == 1 && sel->mode == 2) {
        sel->unk17 = 1;
        func_80001248(0xFE, 0);
    }
    sel->unk14 = arg2;
    sel->mode = mode;
    sel->unk19 = arg4;
    if (sel->unk1A == 0) {
        sel->unk1A = 1;
    }
}

/* A texture of a model, as 02834 uses it */
typedef struct M135Texture {
    /* 0x00 */ u8 pad0[0x12];
    /* 0x12 */ u16 frames; /* times 256 */
    /* 0x14 */ u8 pad14[2];
    /* 0x16 */ s16 frameSize;
} M135Texture;

/* A model's texture table entry (8 bytes) */
typedef struct M135ModelTexture {
    /* 0x0 */ M135Texture *texture;
    /* 0x4 */ u8 pad4[4];
} M135ModelTexture;

typedef struct M135Model {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ M135ModelTexture *textures;
    /* 0x1C */ u8 pad1C[0x2C - 0x1C];
    /* 0x2C */ u8 batchCount;
} M135Model;

/* A batch's texture animation (8 bytes) */
typedef struct M135TexAnim {
    /* 0x0 */ s16 frame;
    /* 0x2 */ s16 frame2; /* < 0: none */
    /* 0x4 */ u32 flags;  /* the low byte: the texture; 0x10000: set by 02834 */
} M135TexAnim;

typedef struct M135Instance {
    /* 0x00 */ M135Model *model;
    /* 0x04 */ u8 pad4[0xB - 0x4];
    /* 0x0B */ s8 buffer;
    /* 0x0C */ u8 padC[0x74 - 0xC];
    /* 0x74 */ M135TexAnim *texAnims;
    /* 0x78 */ s16 *texOffsets[2];
} M135Instance;

/* Sets the frames of the object's animated textures `texture` frames long to `frame` (the second one to
   the next frame), and writes each batch's texture offsets (called from 021F8; global while 021F8 is asm) */
#ifdef NON_MATCHING
static
#endif
void func_m135_02834(Object *obj, s32 frame, s32 frames) {
    M135Instance *inst = obj->modelInstances[obj->modelIndex];
    M135Model *model = inst->model;
    M135TexAnim *a = inst->texAnims;
    M135Texture *tex;
    s16 *out;
    s32 i;
    s32 idx;

    frames <<= 8;
    if (a == NULL) {
        return;
    }
    out = inst->texOffsets[inst->buffer];
    for (i = 0; i < model->batchCount; i++) {
        idx = a->flags & 0xFF;
        tex = model->textures[idx].texture;
        if ((a->flags & 0x10000) && frames == tex->frames) {
            a->frame = frame;
            if (a->frame2 >= 0) {
                frame += 0x100;
                if (frame >= tex->frames) {
                    frame -= 0x100;
                }
                a->frame2 = frame;
            }
        }
        if (a->frame2 >= 0) {
            *out++ = (a->frame2 >> 8) * tex->frameSize;
        }
        *out++ = (a->frame >> 8) * tex->frameSize;
        a++;
    }
}

s16 *func_m135_02958(Object *obj) {
    M135Selector *sel = obj->unk68;

    sJointAngles[0] = 0x2008;
    sJointAngles[1] = sel->unk8;
    sJointAngles[2] = 0x2006;
    sJointAngles[3] = 0;
    sJointAngles[4] = 0x200A;
    sJointAngles[5] = 0;
    sJointAngles[6] = 0x1000;
    return sJointAngles;
}

void func_m135_02998(Object *obj) {
    M135Selector *sel = obj->unk68;

    if (sel->sound != 0) {
        sound_stop(sel->sound);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m135", data);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m135", rodata);
#endif
