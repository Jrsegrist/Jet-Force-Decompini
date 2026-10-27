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

/* Collision: sweeps a sphere from `from` to `to`, and finds what it hits */
extern void func_8001CE10(s32 arg0, Vec3f *from, Vec3f *to, f32 *radius, s32 arg4, s32 arg5, s32 arg6);
extern s32 func_80016208(void *obj, Vec3f *from, Vec3f *to, f32 radius, s32 arg4, void *hit);

/* the player's object; callers pass one or two arguments (no prototype) */
extern Object *func_8000E4D0();

/* Game flags (switches, triggers) */
extern s32 game_flag_get(s32 flag);             /* reads */
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
    /* 0x1A */ u8 unk1A;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ s32 unk1C; /* colours? */
    /* 0x20 */ s32 unk20;
} SparkParams;
extern void *func_m010_0000C(SparkParams *spark); /* returns the spark (callers ignore it) */

/*
 * Main-segment calls that triggers and switches use to start and stop
 * scripted animations. Some callers pass (anim, updateRate, obj, 1), others
 * only the first argument, so the original had no prototype in scope for them.
 */
extern void func_800767EC();
extern void func_8007688C();
extern void func_80076928();
extern void func_800772C4();
extern void func_80079BA8();
extern void func_80079BB8();

/*
 * The description of an object light, for func_800210A4 (which adds one to
 * the object, like DKR's light_add_from_object_header; its return value is
 * the light handle that light_colour_set/light_intensity_set take). Size 0x18.
 */
typedef struct LightParams {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 flags;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u16 radius;
    /* 0x0E */ s16 radius2;
    /* 0x10 */ u8 r;
    /* 0x11 */ u8 g;
    /* 0x12 */ u8 b;
    /* 0x13 */ u8 a;
    /* 0x14 */ s16 unk14;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
} LightParams;

/* What func_80018808 fills in */
typedef struct HitResult {
    /* 0x00 */ Object *obj; /* the object hit, or NULL for the level */
    /* 0x04 */ Vec3f pos;
    /* 0x10 */ Vec3f normal;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ s32 flags;
} HitResult;
/* Sweeps the object from `from` to `to` against objects and the level */
extern s32 func_80018808(Object *obj, Vec3f *from, Vec3f *to, HitResult *hit, s32 arg4, s32 arg5);

/* The object list: returns it, with the range of slots in use (DKR objGetObjList) */
extern Object **func_80005E98(s32 *first, s32 *count);

/* Maths (DKR math_util) */
extern void func_8004944C(Vec3s *rotation, Vec3f *vec); /* DKR vec3f_rotate_py */
extern s32 func_80048E2C(s32 min, s32 max); /* DKR get_random_number */
extern f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */
extern f32 func_80049C30(s32 angle); /* cosine of a 16-bit angle */

/* Sound (DKR audio_spatial.c) */
extern void func_80003410(u16 soundId, f32 x, f32 y, f32 z, s32 arg4, s32 *soundMask); /* DKR play_sound_at_position */
extern void func_80001248(s32 sound, s32 arg1); /* DKR amSndPlay */

/*
 * Called first thing by most control functions with the module's own number
 * (the s32 at the start of its .data) and (2, 2).
 */
extern void runlink_unload_timer_set(s32 module, s32 arg1, s32 arg2);

/* Scales an amount or a timer (m100, m138, m153 use it) */
extern s32 func_80005064(s32 amount);

#endif
