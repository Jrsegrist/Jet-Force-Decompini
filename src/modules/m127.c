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
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ s32 unk1C;
} M127State;

/* The module's .data (floats, a debug string and the control's jump table), still assembly */
extern f32 rl_m127_01150[];

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
#endif

extern Object *func_80041DE0(void); /* the game camera */
extern void func_800496B0(s32 arg0, Object *obj, Vec3f *in, Vec3f *out); /* rotates by the object's transform */
extern f32 func_80097140(f32 x);             /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z);      /* atan2 */

INCLUDE_ASM("asm/nonmatchings/modules/m127", func_m127_00000);

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
    func_8001149C(obj, 0, 0.0f);
    func_8001149C(obj, 0, 0.0f);
    func_80079D1C(obj, -1, 0, 8, 0, 0, 0, 4.0f, 4.0f, -1.0f, 1.0f, 1);
}

INCLUDE_ASM("asm/nonmatchings/modules/m127", func_m127_00104);

/*
 * NON_MATCHING: the camera init. Everything matches except the two rates at
 * the end: the ROM loads each from .data with its own lui (literals of a
 * section that assembly still holds, with the control's jump table); one
 * extern array gives a shared base register. The .bss statics are C only in
 * this branch (IDO drops statics no C uses).
 */
#ifdef NON_MATCHING
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
    sRate = rl_m127_01150[27];
    sRate2 = rl_m127_01150[28];
    sActive = TRUE;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m127", func_m127_00B14);
#endif

INCLUDE_ASM("asm/nonmatchings/modules/m127", func_m127_00D44);

INCLUDE_ASM("asm/nonmatchings/modules/m127", func_m127_00FA0);

INCLUDE_ASM("asm/nonmatchings/modules/m127", data);

#ifndef NON_MATCHING
INCLUDE_ASM("asm/nonmatchings/modules/m127", bss);
#endif

