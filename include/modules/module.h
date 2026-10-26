#ifndef MODULES_MODULE_H
#define MODULES_MODULE_H

/*
 * What runlink module C shares: the module ABI and the main-segment
 * functions that many modules call. See docs/runlink.md.
 *
 * Module ABI:
 * - A module is one object. Its exported functions and variables (those
 *   that have a runlink name) are global and named func_mNNN_XXXXX /
 *   D_mNNN_XXXXX by offset; everything else is static.
 * - The loader calls the init function after linking and the fini function
 *   before unloading, if the module has them; mark them with
 *       #pragma weak __rl_init = func
 *       #pragma weak __rl_fini = func
 * - Object behaviours export an init function `void (Object *, setup)`, called
 *   once with the object's level entry, and a control function
 *   `void (Object *, s32 updateRate)`, called every frame.
 *
 * Main-segment functions are declared by address name (func_XXXXXXXX), or by
 * the DKR name that undefined_syms.us.txt gives them; the comments give the
 * DKR equivalent where there is one. The argument types come from the
 * modules' calls.
 */

#include "common.h"
#include "object.h"

/* Objects (DKR objects.c) */
extern void func_800079B8(Object *obj);                      /* DKR free_object */
extern void obj_spawn_particle(Object *obj, s32 updateRate); /* DKR */
/* advances the object's animation */
extern void func_8001138C(Object *obj, f32 speed, f32 updateRate);
/* sets the object's animation and its position in it */
extern void func_8001149C(Object *obj, s32 anim, f32 frame);
/* sets the object's hit box / collision */
extern void func_80079D1C(Object *obj, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, f32 arg7,
                          f32 arg8, f32 arg9, f32 arg10, s32 arg11);

/* the player's object; callers pass one or two arguments (no prototype) */
extern Object *func_8000E4D0();

/* Game flags (switches, triggers) */
extern s32 func_80047ED0(s32 flag);             /* reads */
extern void func_80047EFC(s32 flag, s32 value); /* writes */

/* Module 6 (the HUD): queues a message for the player (whose object's unk68 is passed) */
extern void func_m006_011E0(void *player, s32 message);

/* Module 10 (sparks): adds one spark */
typedef struct SparkParams {
    /* 0x00 */ s16 y_rotation;
    /* 0x02 */ s16 x_rotation;
    /* 0x04 */ f32 speed;
    /* 0x08 */ f32 x_position;
    /* 0x0C */ f32 y_position;
    /* 0x10 */ f32 z_position;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ s32 unk1C; /* colours? */
    /* 0x20 */ s32 unk20;
} SparkParams;
extern void func_m010_0000C(SparkParams *spark);

/*
 * Animation sequences that triggers and switches start and stop. Some
 * callers pass (anim, updateRate, obj, 1), others only the sequence, so the
 * original had no prototype in scope for them.
 */
extern void func_800767EC();
extern void func_8007688C();
extern void func_80076928();
extern void func_800772C4();
extern void func_80079BA8();
extern void func_80079BB8();

/* Maths (DKR math_util) */
extern void func_8004944C(Vec3s *rotation, Vec3f *vec); /* DKR vec3f_rotate_py */
extern s32 func_80048E2C(s32 min, s32 max); /* DKR get_random_number */

/* Sound (DKR audio_spatial.c) */
extern void func_80003410(s32 soundId, f32 x, f32 y, f32 z, s32 arg4, s32 *soundMask); /* DKR play_sound_at_position */

#endif
