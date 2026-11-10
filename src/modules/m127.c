/*
 * Runlink module 127: a terminal object that the player can use, with a
 * camera that swings round to look at it.
 */
#include "module.h"

/* What init gets */
typedef struct M127Setup {
    /* 0x00 */ LevelObjectEntryCommon common;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 yaw; /* in 1/256ths of a turn */
} M127Setup;

/* Kept in obj->unk68 */
typedef struct M127State {
    /* 0x00 */ u8 mode;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ Object *unk8; /* the screen */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s32 unk1C;
} M127State;

/* An unused debug string, first in the module's .rodata */
static const char sDebugText[] = "Range %.1f, Dy %.1f, Angle %04x\n";

#ifdef NON_MATCHING
/* The camera that looks at the terminal (the module's .bss) */
static f32 sTargetX;
static f32 sTargetY;
static f32 sTargetZ;
static f32 sPosX;
static f32 sPosY;
static f32 sPosZ;
static f32 sDist; /* of the game camera from the target, at the start */
static f32 sHeight;
static s16 sAngle;
static s16 sCamYaw; /* the game camera's rotation at the start */
static s16 sCamPitch;
static s16 sCamRoll;
static f32 sCurDist;
static f32 sCurHeight;
static s16 sCurAngle;
static f32 sTargetDist;
static f32 sTargetHeight;
static s16 sTargetAngle;
static f32 sRate;
static f32 sRate2;
static s32 sActive;
#else
/* The .bss is still assembly: each variable by its module offset */
extern f32 rl_m127_011D0, rl_m127_011D4, rl_m127_011D8, rl_m127_011DC, rl_m127_011E0, rl_m127_011E4;
extern f32 rl_m127_011E8, rl_m127_011EC;
extern s16 rl_m127_011F0, rl_m127_011F2, rl_m127_011F4, rl_m127_011F6, rl_m127_01200, rl_m127_0120C;
extern f32 rl_m127_011F8, rl_m127_011FC, rl_m127_01204, rl_m127_01208, rl_m127_01210, rl_m127_01214;
extern s32 rl_m127_01218;
#define sTargetX rl_m127_011D0
#define sTargetY rl_m127_011D4
#define sTargetZ rl_m127_011D8
#define sPosX rl_m127_011DC
#define sPosY rl_m127_011E0
#define sPosZ rl_m127_011E4
#define sDist rl_m127_011E8
#define sHeight rl_m127_011EC
#define sAngle rl_m127_011F0
#define sCamYaw rl_m127_011F2
#define sCamPitch rl_m127_011F4
#define sCamRoll rl_m127_011F6
#define sCurDist rl_m127_011F8
#define sCurHeight rl_m127_011FC
#define sCurAngle rl_m127_01200
#define sTargetDist rl_m127_01204
#define sTargetHeight rl_m127_01208
#define sTargetAngle rl_m127_0120C
#define sRate rl_m127_01210
#define sRate2 rl_m127_01214
#define sActive rl_m127_01218
#endif

extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s16 angle_lerp(s16 from, s16 to, f32 t); /* moves an angle towards another */

extern Object *func_80041DE0(void); /* the game camera */
extern void func_800496B0(s32 arg0, Object *obj, Vec3f *in, Vec3f *out); /* rotates by the object's transform */
extern f32 func_80097140(f32 x);             /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z);      /* atan2 */


/* The point the player must be near to use the terminal, and how near (the second argument is unused) */
static void func_m127_00000(Object *obj, UNUSED void *arg1, f32 *x, f32 *y, f32 *z, f32 *range) {
    *x = obj->trans.x_position;
    *y = obj->trans.y_position;
    *z = obj->trans.z_position;
    *range = 10000.0f;
}

void func_m127_0003C(Object *obj, M127Setup *setup) {
    M127State *s = obj->unk68;

    obj->trans.rotation.y_rotation = setup->yaw << 8;
    s->mode = 0;
    s->unk1 = setup->unkA;
    s->unk2 = 0;
    s->unk8 = 0;
    s->unkC = 0;
    s->unk10 = 0;
    s->unk1C = 0;
    s->unk18 = 0.0f;
    s->unk14 = 0.0f;
    obj_anim_set(obj, 0, 0.0f);
    obj_anim_set(obj, 0, 0.0f);
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 4.0f, 4.0f, -1.0f, 1.0f, 1);
}

void func_m127_00B14(Object *obj);

/* The screen object's state (OBJ_STATE) */
typedef struct M127ScreenState {
    /* 0x00 */ s32 unk0; /* a phase, in 1/256ths */
    /* 0x04 */ s32 unk4; /* the glow, 0-255 */
} M127ScreenState;

/* The player's racer data, reduced */
typedef struct M127Racer {
    /* 0x000 */ u8 pad0[0x1F9];
    /* 0x1F9 */ u8 unk1F9;
} M127Racer;

extern s32 func_80043510(s32 player); /* the buttons pressed */
extern void func_80048518(s32 arg0);
extern void func_800099F4(void);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void sound_stop(s32 sound);
extern void func_m007_00148(s32 hint, s32 arg1);
extern void func_m007_00D6C(void);
extern s32 func_m007_00D80(void);
#ifdef NON_MATCHING
static void func_m127_00FA0(Object *obj, s32 scroll);
#define rl_m127_00FA0 func_m127_00FA0
#else
extern void rl_m127_00FA0(Object *obj, s32 scroll); /* still assembly */
#endif

/* The terminal's control: the player walks up and presses the button, the camera swings round */
void func_m127_00104(Object *obj, s32 updateRate) {
    s32 first;
    s32 count;
    Object *player;
    M127State *s;
    Object *screen;
    f32 x;
    f32 y;
    f32 z;
    s32 move;
    f32 rate;
    f32 range;
    f32 pos[3];
    s32 i;
    UNUSED s32 pad;
    Object **objs;
    M127Racer *racer;

    player = func_8000E4D0(0);
    if (player == NULL) {
        func_800079B8(obj);
        return;
    }
    s = obj->unk68;
    racer = player->unk68;
    obj->particleEmittersEnabled = 0;
    switch (s->mode) {
        case 0:
            if (func_80043510(0) & 0x8000) {
                s16 angle;

                func_m127_00000(obj, racer, &x, &y, &z, &range);
                x -= player->trans.x_position;
                y -= player->trans.y_position;
                z -= player->trans.z_position;
                angle = player->trans.rotation.y_rotation - obj->trans.rotation.y_rotation - 0x8000;
                if (x * x + z * z < range) {
                    if (y >= -8.0f && y <= 8.0f) {
                        if (angle >= -0x1555 && angle < 0x1556) {
                            s->mode = 1;
                            func_m127_00B14(obj);
                            func_80048518(10);
                            racer->unk1F9 = 2;
                            obj_anim_set(obj, 0, 0.0f);
                        }
                    }
                }
            }
            break;
        case 1:
            func_80048518(10);
            func_8001138C(obj, 0.0084f, updateRate);
            if (obj->unk28 == 1.0f) {
                s->mode = 2;
                s->unk2 = 120;
                s->unk4 = 0.001f;
                obj_anim_set(obj, 1, 0.0f);
            }
            break;
        case 2:
            func_80048518(10);
            func_8001138C(obj, s->unk4, updateRate);
            s->unk4 += 0.0002f * updateRate;
            if (s->unk4 > 0.015f) {
                s->unk4 = 0.015f;
            }
            if (s->unk4 > 0.008f) {
                obj->particleEmittersEnabled = 1;
                if (s->unk1C == 0) {
                    func_80001248(0x1BE, (s32) &s->unk1C);
                }
            }
            s->unk2 -= updateRate;
            if (s->unk2 <= 0) {
                s->mode = 3;
                s->unk2 = 0x80;
                s->unkC = 0;
                s->unk10 = 0xFF;
                if (s->unk1C != 0) {
                    sound_stop(s->unk1C);
                }
                func_80001248(0x1BF, 0);
            }
            break;
        case 3:
            func_80048518(10);
            func_8001138C(obj, s->unk4, updateRate);
            s->unk4 -= 0.0002f * updateRate;
            if (s->unk4 < 0.005f) {
                s->unk4 = 0.005f;
            }
            if (s->unk4 > 0.008f) {
                obj->particleEmittersEnabled = 1;
            }
            s->unk2 -= updateRate;
            if (s->unk2 <= 0) {
                s->mode = 4;
                func_m007_00148(s->unk1, 0);
            }
            break;
        case 4:
            func_80048518(10);
            func_8001138C(obj, s->unk4, updateRate);
            if (func_80043510(0) & 0x1000) {
                func_m007_00D6C();
            }
            if (func_m007_00D80() == 0) {
                s->mode = 5;
                obj_anim_set(obj, 2, 1.0f);
                sTargetDist = sDist;
                sTargetHeight = sHeight;
                sTargetAngle = sAngle;
                sRate = 0.95f;
                sRate2 = 0.95f;
                sActive = FALSE;
                s->unk18 = 0.0f;
                s->unk14 = 0.0f;
                func_80001248(0x1BF, 0);
            }
            break;
        case 5:
            func_80048518(10);
            func_8001138C(obj, -0.0084f, updateRate);
            if (obj->unk28 == 0.0f) {
                s->mode = 0;
                racer->unk1F9 = 0;
            }
            break;
    }
    func_8005F5AC(obj, updateRate);
    screen = s->unk8;
    if (screen != NULL) {
        if (s->mode == 3 || s->mode == 4) {
            screen->trans.rotation.y_rotation = obj->trans.rotation.y_rotation;
            screen->trans.rotation.x_rotation = -0x800;
            screen->trans.rotation.z_rotation = 0;
            screen->particleEmittersEnabled = 0;
            if (s->unk10 > 0) {
                i = updateRate * 2;
                while (i--) {
                    if (s->unkC < 0xC000) {
                        s->unkC += 0x100;
                        screen->particleEmittersEnabled = 1;
                    } else if (s->unk10 > 0) {
                        s->unkC += s->unk10;
                        s->unk10 -= 2;
                    }
                }
                pos[0] = func_80049C34(s->unkC) * -85.0f;
                pos[1] = func_80049C30((s->unkC * 3) >> 2) * 55.0f + 100.0f;
                pos[2] = func_80049C30(s->unkC) * 85.0f + -35.0f;
                rate = 0.0f;
                screen->trans.rotation.y_rotation = screen->trans.rotation.y_rotation - s->unkC + (s->unk10 << 6);
                screen->trans.rotation.z_rotation -= s->unk10 * 16;
            } else {
                func_8001138C(screen, -0.0167f, updateRate);
                pos[0] = 0.0f;
                pos[1] = func_80049C34(OBJ_STATE(screen, M127ScreenState)->unk0) * 8.0f + 100.0f;
                pos[2] = 50.0f;
                rate = 0.95f;
            }
            screen->trans.flags &= ~0x400;
            OBJ_STATE(screen, M127ScreenState)->unk0 += updateRate << 8;
            OBJ_STATE(screen, M127ScreenState)->unk4 += updateRate * 4;
            if (OBJ_STATE(screen, M127ScreenState)->unk4 > 0xFF) {
                OBJ_STATE(screen, M127ScreenState)->unk4 = 0xFF;
            }
            func_8005F5AC(screen, updateRate);
            move = TRUE;
        } else if (OBJ_STATE(screen, M127ScreenState)->unk4 != 0) {
            i = updateRate;
            while (i--) {
                s->unk14 += 0.05f;
                s->unk18 += s->unk14;
                screen->trans.rotation.x_rotation += 0x40;
                if (screen->trans.rotation.x_rotation > 0x4000) {
                    screen->trans.rotation.x_rotation = 0x4000;
                }
                if (s->unk18 > 5.0f) {
                    s->unk18 = 5.0f;
                }
            }
            pos[2] = -s->unk18 * updateRate;
            func_8004944C(&screen->trans.rotation, (Vec3f *) pos);
            func_800099F4();
            func_80009A04(screen, pos[0], pos[1], pos[2]);
            OBJ_STATE(screen, M127ScreenState)->unk4 -= updateRate * 2;
            if (OBJ_STATE(screen, M127ScreenState)->unk4 <= 0) {
                OBJ_STATE(screen, M127ScreenState)->unk4 = 0;
                screen->trans.flags |= 0x400;
            }
            screen->particleEmittersEnabled = 1;
            func_8005F5AC(screen, updateRate);
            move = FALSE;
            func_8001138C(screen, 0.0167f, updateRate);
        } else {
            OBJ_STATE(screen, M127ScreenState)->unk0 = 0;
            OBJ_STATE(screen, M127ScreenState)->unk4 = 0;
            move = FALSE;
            screen->unk28 = 1.0f;
        }
        if (move) {
            func_800496B0(1, obj, (Vec3f *) pos, (Vec3f *) pos);
            x = pos[0] + obj->trans.x_position;
            y = pos[1] + obj->trans.y_position;
            z = pos[2] + obj->trans.z_position;
            rate = 1.0f - func_80049BEC(rate, updateRate);
            func_800099F4();
            func_80009A04(screen, (x - screen->trans.x_position) * rate, (y - screen->trans.y_position) * rate,
                          (z - screen->trans.z_position) * rate);
        }
    } else {
        objs = func_80005E98(&first, &count);
        while (first < count) {
            screen = objs[first++];
            if (screen->behaviorId == 0x7E) {
                s->unk8 = screen;
                OBJ_STATE(screen, M127ScreenState)->unk4 = 0;
                break;
            }
        }
    }
    rl_m127_00FA0(obj, updateRate);
}

/* Starts the camera: from where the game camera is, towards a view of the terminal */
void func_m127_00B14(Object *obj) {
    f32 dx;
    f32 dy;
    f32 dz;
    f32 v[3];
    Object *cam;

    cam = func_80041DE0();
    v[1] = 70.0f;
    v[0] = 0.0f;
    v[2] = -40.0f;
    func_800496B0(1, obj, (Vec3f *) v, (Vec3f *) v);
    sPosX = v[0] + obj->trans.x_position;
    sPosY = v[1] + obj->trans.y_position;
    sPosZ = v[2] + obj->trans.z_position;
    v[0] = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
    func_800496B0(1, obj, (Vec3f *) v, (Vec3f *) v);
    sTargetX = v[0] + obj->trans.x_position;
    sTargetY = v[1] + obj->trans.y_position;
    sTargetZ = v[2] + obj->trans.z_position;
    dx = cam->trans.x_position - sTargetX;
    dy = cam->trans.y_position - sTargetY;
    dz = cam->trans.z_position - sTargetZ;
    sDist = func_80097140(dx * dx + dz * dz);
    sHeight = dy;
    sAngle = 0x8000 - func_80049C84(dx, dz);
    sCamYaw = cam->trans.rotation.y_rotation;
    sCamPitch = cam->trans.rotation.x_rotation;
    sCamRoll = cam->trans.rotation.z_rotation;
    sCurDist = sDist;
    sCurHeight = sHeight;
    sCurAngle = sAngle;
    sTargetDist = 200.0f;
    sTargetHeight = 100.0f;
    sTargetAngle = 0x2800 - obj->trans.rotation.y_rotation;
    sRate = 0.975f;
    sRate2 = 0.9f;
    sActive = TRUE;
}

/* Moves the camera round the terminal and turns it to look at it */
void func_m127_00D44(Object *cam, s32 updateRate) {
    s16 roll;
    s16 pitch;
    s16 yaw;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 t;

    t = 1.0f - func_80049BEC(sRate, updateRate);
    sCurDist += (sTargetDist - sCurDist) * t;
    sCurHeight += (sTargetHeight - sCurHeight) * t;
    sCurAngle = angle_lerp(sCurAngle, sTargetAngle, t);
    func_80049C34(sCurAngle);
    func_80049C30(sCurAngle);
    cam->trans.x_position = func_80049C34(sCurAngle) * sCurDist + sTargetX;
    cam->trans.y_position = sTargetY + sCurHeight;
    cam->trans.z_position = sTargetZ - func_80049C30(sCurAngle) * sCurDist;
    if (sActive) {
        dx = cam->trans.x_position - sPosX;
        dy = cam->trans.y_position - sPosY;
        dz = cam->trans.z_position - sPosZ;
        yaw = 0x8000 - func_80049C84(dx, dz);
        pitch = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
        roll = 0;
    } else {
        yaw = sCamYaw;
        pitch = sCamPitch;
        roll = sCamRoll;
    }
    t = 1.0f - func_80049BEC(sRate2, updateRate);
    cam->trans.rotation.y_rotation = angle_lerp(cam->trans.rotation.y_rotation, yaw, t);
    cam->trans.rotation.x_rotation = angle_lerp(cam->trans.rotation.x_rotation, pitch, t);
    cam->trans.rotation.z_rotation = angle_lerp(cam->trans.rotation.z_rotation, roll, t);
}

/* The parts of the terminal's model that its screen animation uses */
typedef struct M127Batch {
    /* 0x0 */ u8 kind; /* 7: the screen */
    /* 0x1 */ u8 pad1[7];
    /* 0x8 */ s16 firstTri;
    /* 0xA */ u8 padA[6];
} M127Batch;
typedef struct M127Tri {
    /* 0x0 */ u8 pad0[4];
    /* 0x4 */ s16 uv[3][2];
} M127Tri;
typedef struct M127Texture {
    /* 0x0 */ u8 pad0;
    /* 0x1 */ u8 height; /* in 32-texel units */
} M127Texture;
typedef struct M127TexInfo {
    /* 0x00 */ u8 pad0[0x38];
    /* 0x38 */ M127Texture *texture;
} M127TexInfo;
typedef struct M127Model {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 batchCount;
    /* 0x18 */ M127TexInfo *texInfo;
    /* 0x1C */ u8 pad1C[4];
    /* 0x20 */ M127Tri *tris;
    /* 0x24 */ M127Batch *batches;
    /* 0x28 */ u8 pad28[4];
    /* 0x2C */ u8 attachCount;
} M127Model;
typedef struct M127Attach {
    /* 0x0 */ s16 flags;
    /* 0x2 */ u8 pad2[2];
    /* 0x4 */ s32 kind; /* the low byte; 7: the screen */
} M127Attach;
typedef struct M127Instance {
    /* 0x00 */ M127Model *model;
    /* 0x04 */ u8 pad4[0x74 - 0x4];
    /* 0x74 */ M127Attach *attach;
} M127Instance;

/*
 * Scrolls the screen's texture by `scroll` while the terminal is idle, or puts it back at the top.
 * `minV` is never set before its first use in the second branch (the original's slip; nothing reads
 * the result, but IDO gives it a stack home that the ROM loads and stores).
 */
#ifdef NON_MATCHING
static void func_m127_00FA0(Object *obj, s32 scroll) {
    M127State *s;
    M127Instance *inst;
    M127Model *model;
    s32 mask;
    M127Batch *batch;
    s32 i;
    s32 j;
    M127Tri *tri;
    s32 minU;
    s32 minV;
    s32 v0;
    s32 v1;
    s32 v2;
    M127Attach *a;

    s = obj->unk68;
    inst = *(M127Instance **) obj->modelInstances;
    model = inst->model;
    i = model->batchCount;
    mask = (model->texInfo->texture->height << 5) - 1;
    batch = model->batches;
    while (i--) {
        if (batch->kind == 7) {
            j = batch[1].firstTri - batch->firstTri;
            tri = &model->tris[batch->firstTri];
            while (j--) {
                if (s->mode == 0) {
                    v0 = tri->uv[0][1];
                    minU = tri->uv[1][1] - v0;
                    tri->uv[0][1] = (v0 + scroll * 16) & mask;
                    minV = tri->uv[2][1] - v0;
                    tri->uv[1][1] = tri->uv[0][1] + minU;
                    tri->uv[2][1] = tri->uv[0][1] + minV;
                } else {
                    v0 = tri->uv[0][1];
                    v1 = tri->uv[1][1];
                    minU = v0;
                    if (v1 < v0) {
                        minU = v1;
                    }
                    v2 = tri->uv[2][1];
                    if (v2 < minV) {
                        minV = v2;
                    }
                    tri->uv[0][1] = v0 - minU;
                    tri->uv[1][1] = v1 - minU;
                    tri->uv[2][1] = v2 - minU;
                }
                tri++;
            }
        }
        batch++;
    }
    i = model->attachCount;
    a = inst->attach;
    while (i--) {
        if ((a->kind & 0xFF) == 7) {
            if (s->mode == 0) {
                a->flags = 0;
            } else {
                a->flags = 0x100;
            }
        }
        a++;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m127", func_m127_00FA0);
#endif

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m127", bss);
#endif

