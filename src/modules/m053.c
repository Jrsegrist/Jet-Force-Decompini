/*
 * Runlink module 53: a vehicle the players ride (its init, control and camera).
 * Family: other object behaviours (docs/runlink.md, "Module families").
 */
#include "module.h"

/* The module's .bss (still assembly) */
extern f32 rl_m053_03828[4];
extern f32 rl_m053_03838[4];
extern f32 rl_m053_03848[4];

/* Resets player `player`'s camera values */
void func_m053_00000(s32 player) {
    rl_m053_03828[player] = 0.15416667f;
    rl_m053_03838[player] = 0.0f;
    rl_m053_03848[player] = 0.0f;
}

/* (defined below and called from 00038) */
struct M053Steer;
struct M053Slow;
struct M053Turn;
struct M053HitState;
struct M053Water;
struct M053Cam;
struct M053Anim;
struct M053State;
extern s16 *rl_m053_03820; /* .bss (still assembly): the camera commands */
extern u8 D_800A4FC4;      /* a multiplayer game */
extern u8 D_800A4FB4;
static void func_m053_014F4(s16 *angles, struct M053Steer *s, f32 speed, s32 angle, f32 updateRateF);
static void func_m053_01850(Object *obj, struct M053Slow *s, f32 speed, s32 angle, f32 updateRateF);
static f32 func_m053_019B0(s16 *yaw, struct M053Turn *s, f32 speed, s16 target, f32 updateRateF);
static void func_m053_021E4(Object *obj, struct M053HitState *s, s32 updateRate);
static void func_m053_0230C(Object *obj, struct M053Water *w, s32 updateRate);
static void func_m053_023D0(s16 *angles, struct M053Cam *s, s32 updateRate);
static void func_m053_02708(void *arg0, void *arg1, void *arg2, s32 arg3);
static void func_m053_0271C(Object *obj, struct M053Anim *s, f32 speed, f32 updateRateF);
static void func_m053_028CC(Object *obj, struct M053State *state, s32 updateRate);
extern s32 func_m031_00000(s32 n); /* whether racer n is computer-driven */
extern void rumble_raise(s32 player, s32 strength, f32 time);
extern s16 angle_lerp(s16 from, s16 to, f32 t);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern f32 func_80097140(f32 x);         /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z); /* the angle of a direction */

/* What racer->pose points at (as m016's RacerPose) */
typedef struct M053Pose {
    /* 0x00 */ u8 pad0[5];
    /* 0x05 */ u8 unk5;
} M053Pose;

/* The flag word at 0x540 (as m016's RacerFlags) */
typedef struct M053Flags {
    u32 unk31 : 1;
    u32 unk30 : 1;
    u32 weaponDir : 2;
    u32 unk27 : 1;
    u32 useOverride : 1;
    u32 rest : 26;
} M053Flags;

/* The vehicle's state (the object's unk68: a player's state block, as m016's Racer), as far as 00038, 00EA0
   and 01BB4 use it */
typedef struct M053Vehicle {
    /* 0x000 */ s8 player;
    /* 0x001 */ u8 pad1[0x4 - 0x1];
    /* 0x004 */ f32 unk4; /* forward speed */
    /* 0x008 */ f32 unk8; /* sideways speed */
    /* 0x00C */ f32 unkC; /* speed limit */
    /* 0x010 */ f32 unk10;
    /* 0x014 */ f32 unk14;
    /* 0x018 */ u8 pad18[0x3C - 0x18];
    /* 0x03C */ f32 unk3C; /* the position before the move */
    /* 0x040 */ f32 unk40;
    /* 0x044 */ f32 unk44;
    /* 0x048 */ f32 unk48;
    /* 0x04C */ u8 pad4C[0x50 - 0x4C];
    /* 0x050 */ f32 unk50;
    /* 0x054 */ f32 unk54;
    /* 0x058 */ f32 unk58;
    /* 0x05C */ f32 unk5C;
    /* 0x060 */ f32 unk60;
    /* 0x064 */ u8 pad64[0x6C - 0x64];
    /* 0x06C */ f32 unk6C;
    /* 0x070 */ f32 unk70;
    /* 0x074 */ u8 pad74[0xBC - 0x74];
    /* 0x0BC */ s32 unkBC; /* sound handle */
    /* 0x0C0 */ u8 padC0[0xCC - 0xC0];
    /* 0x0CC */ s32 unkCC; /* sound handle */
    /* 0x0D0 */ u8 padD0[0x10D - 0xD0];
    /* 0x10D */ u8 unk10D;
    /* 0x10E */ u8 pad10E[0x11C - 0x10E];
    /* 0x11C */ s16 unk11C; /* yaw */
    /* 0x11E */ s16 unk11E;
    /* 0x120 */ s16 unk120;
    /* 0x122 */ u8 pad122[0x128 - 0x122];
    /* 0x128 */ s16 unk128;
    /* 0x12A */ u8 pad12A[0x13C - 0x12A];
    /* 0x13C */ s16 unk13C;
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 unk13F;
    /* 0x140 */ u8 pad140[0x149 - 0x140];
    /* 0x149 */ s8 unk149;
    /* 0x14A */ s8 unk14A;
    /* 0x14B */ s8 unk14B;
    /* 0x14C */ u8 pad14C[0x185 - 0x14C];
    /* 0x185 */ s8 unk185;
    /* 0x186 */ u8 pad186[0x188 - 0x186];
    /* 0x188 */ u8 unk188;
    /* 0x189 */ u8 pad189[0x198 - 0x189];
    /* 0x198 */ u8 unk198;
    /* 0x199 */ u8 pad199[0x1C6 - 0x199];
    /* 0x1C6 */ s16 unk1C6;
    /* 0x1C8 */ s16 unk1C8;
    /* 0x1CA */ s16 unk1CA;
    /* 0x1CC */ s16 unk1CC;
    /* 0x1CE */ s16 unk1CE;
    /* 0x1D0 */ s16 unk1D0;
    /* 0x1D2 */ u8 pad1D2[0x1DC - 0x1D2];
    /* 0x1DC */ s16 unk1DC;
    /* 0x1DE */ s16 unk1DE;
    /* 0x1E0 */ s16 unk1E0;
    /* 0x1E2 */ s16 unk1E2;
    /* 0x1E4 */ f32 unk1E4;
    /* 0x1E8 */ f32 unk1E8;
    /* 0x1EC */ u8 pad1EC[0x1F4 - 0x1EC];
    /* 0x1F4 */ s8 unk1F4;
    /* 0x1F5 */ u8 pad1F5[0x1F9 - 0x1F5];
    /* 0x1F9 */ u8 unk1F9;
    /* 0x1FA */ u8 unk1FA;
    /* 0x1FB */ u8 pad1FB[0x240 - 0x1FB];
    /* 0x240 */ s16 cmds[0x80]; /* the camera command list */
    /* 0x340 */ u8 pad340[0x354 - 0x340];
    /* 0x354 */ f32 unk354;
    /* 0x358 */ f32 unk358;
    /* 0x35C */ f32 unk35C;
    /* 0x360 */ M053Pose *pose;
    /* 0x364 */ u8 pad364[0x532 - 0x364];
    /* 0x532 */ u8 unk532;
    /* 0x533 */ u8 pad533[0x540 - 0x533];
    /* 0x540 */ M053Flags flags;
    /* 0x544 */ u8 pad544[0x568 - 0x544];
    /* 0x568 */ u8 state;
    /* 0x569 */ u8 pad569[0x56A - 0x569];
    /* 0x56A */ u8 unk56A;
    /* 0x56B */ u8 pad56B[0x576 - 0x56B];
    /* 0x576 */ s8 unk576;
    /* 0x577 */ u8 pad577[0x57C - 0x577];
    /* 0x57C */ f32 unk57C;
    /* 0x580 */ u8 pad580[0x58C - 0x580];
    /* 0x58C */ s16 unk58C; /* the boost's timer */
    /* 0x58E */ u8 pad58E[0x594 - 0x58E];
    /* 0x594 */ f32 unk594;
    /* 0x598 */ u8 pad598[0x5BC - 0x598];
    /* 0x5BC */ void *unk5BC;
    /* 0x5C0 */ u8 pad5C0[0x5E0 - 0x5C0];
    /* 0x5E0 */ s32 sounds[3]; /* positioned sound handles */
} M053Vehicle;

/* A model instance (obj->modelInstances[i]), as used here */
typedef struct M053ModelInst {
    /* 0x000 */ void *model;
    /* 0x004 */ u8 pad4[0xA - 0x4];
    /* 0x00A */ s8 unkA;
    /* 0x00B */ s8 unkB;
    /* 0x00C */ u8 padC[0x10 - 0xC];
    /* 0x010 */ void *unk10[2];
} M053ModelInst;

/* The controller's button masks for the actions */
typedef struct M053Buttons {
    /* 0x00 */ u32 unk0;
} M053Buttons;

extern u8 D_800A4FD8;
extern s8 D_800A503C;
extern M053Buttons *D_800A18B0;
extern s32 gPlayerButtonsHeld;
extern s32 gPlayerButtonsPressed;
extern s32 gPlayerStickX;
extern s32 gPlayerStickY;
extern ObjectTransform *D_800F6DC0; /* the camera's transform */
extern s16 rl_m053_03824; /* .bss: the camera's wanted angles */
extern s16 rl_m053_03826;
extern void player_input_lock_set(s32 arg0);
extern void player_input_latch(s32 player);
extern void func_80034018(M053Vehicle *s);
extern void func_80034164(Object *obj, M053Vehicle *s);
extern void func_80049394(Vec3s *rotation, Vec3f *vec);
extern s32 func_m031_017A0(s32 n, s32 *stickX, s32 *stickY, s32 *arg3, s32 *buttons);
extern s32 func_m031_01C68(Object *obj, s32 n);
extern void func_80039FCC(Object *obj, M053Vehicle *s, f32 a, f32 b, f32 c, f32 d, f32 e, s32 updateRate);
extern void player_charge_meter_update(M053Vehicle *s, s32 arg1, s32 updateRate);
extern s32 track_height_at(f32 x, f32 z, f32 *height, s32 mask);
extern void player_platform_ride(Object *obj, M053Vehicle *s, f32 updateRate);
extern void sound_pitch_set(s32 handle, f32 pitch);
extern void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, void **handlePtr);
extern void audspat_point_set_position(s32 handle, f32 x, f32 y, f32 z);
extern void audspat_point_set_pitch(s32 handle, u8 pitch);
extern void func_m107_00D08(void *arg0, f32 a, f32 b, s32 c);
extern void func_8003D21C(M053ModelInst *inst, void *model, Object *obj, f32 a, f32 b);
extern void func_800221F0(Object *obj, M053ModelInst *inst, void *shading, void *arg3);
extern void player_loop_sounds_update(Object *obj, M053Vehicle *s, s32 a, s32 b, s32 updateRate);
static void func_m053_00EA0(Object *obj, M053Vehicle *s, f32 speed, s16 angle, f32 updateRateF);
static void func_m053_01BB4(Object *obj, M053Vehicle *s, f32 updateRate);

/*
 * The vehicle's control function, called every frame (a shorter form of m016's player control function): input
 * and speed limits, the stick's speed and angle, the boost (unk58C), the state's movement (00EA0, 014F4, 01850),
 * the engine sound, the camera commands and the model's tilt while it is drawn.
 */
void func_m053_00038(Object *obj, s32 updateRate) {
    M053Vehicle *s;
    s32 mp; /* 017A0's result in a multiplayer game */
    f32 speed;
    f32 k;
    UNUSED s32 pad74;
    UNUSED s32 pad70;
    Vec3f dir;
    f32 rate;
    f32 target;
    s16 angle;
    Vec3s rot;
    UNUSED s32 pad50;
    void *model;
    M053ModelInst *inst;
    f32 x;
    f32 vol;
    f32 y;

    s = obj->unk68;
    obj->particleEmittersEnabled = 0;
    rl_m053_03820 = s->cmds;
    if (s->state != 3) {
        s->unk56A = 0;
    }
    if (s->unk149 > 0) {
        player_input_lock_set(2);
    }
    if (s->unk10D != 0 || s->unk1F9 != 0 || s->unk1FA != 0 || s->unk198 != 0 || s->unk188 != 0) {
        player_input_lock_set(1);
    }
    if (s->unk1FA >= 2) {
        s->unk1FA = 0;
    }
    if (s->unk198 >= 2) {
        s->unk198 = 0;
    }
    player_input_latch(s->player);
    func_80034018(s);
    if (obj->velocity.x < -25.0f) {
        obj->velocity.x = -25.0f;
    }
    if (obj->velocity.x > 25.0f) {
        obj->velocity.x = 25.0f;
    }
    if (obj->velocity.y < -20.0f) {
        obj->velocity.y = -20.0f;
    }
    if (obj->velocity.y > 10.0f) {
        obj->velocity.y = 10.0f;
    }
    if (obj->velocity.z < -25.0f) {
        obj->velocity.z = -25.0f;
    }
    if (obj->velocity.z > 25.0f) {
        obj->velocity.z = 25.0f;
    }
    if (s->unk4 < -25.0f) {
        s->unk4 = -25.0f;
    }
    if (s->unk4 > 25.0f) {
        s->unk4 = 25.0f;
    }
    if (s->unk8 < -25.0f) {
        s->unk8 = -25.0f;
    }
    if (s->unk8 > 25.0f) {
        s->unk8 = 25.0f;
    }
    func_80034164(obj, s);
    rot.y_rotation = -s->unk11C;
    rot.x_rotation = -obj->trans.rotation.x_rotation;
    rot.z_rotation = -obj->trans.rotation.z_rotation;
    dir.z = 0.0f;
    dir.x = 0.0f;
    dir.y = -1.0f;
    func_80049394(&rot, &dir);
    s->unk54 = dir.x;
    s->unk58 = dir.y;
    s->unk50 = dir.z;
    rl_m053_03824 = 0;
    rl_m053_03826 = 0;
    if ((D_800A4FC4 != 0 && D_800A4FD8 == 4) || D_800A4FB4 != 0) {
        mp = func_m031_017A0(s->player, &gPlayerStickX, &gPlayerStickY, &gPlayerButtonsHeld, &gPlayerButtonsPressed);
    } else {
        mp = 0;
    }
    speed = func_80097140(gPlayerStickX * gPlayerStickX + gPlayerStickY * gPlayerStickY);
    if (speed > 65.0f) {
        speed = 65.0f;
    }
    if (speed != 0.0f) {
        angle = func_80049C84(gPlayerStickX, -gPlayerStickY) - D_800F6DC0->rotation.y_rotation;
    } else {
        angle = s->unk11C;
    }
    if (mp != 0 && speed != 0.0f) {
        angle += D_800F6DC0->rotation.y_rotation;
    }
    s->unk58C -= updateRate;
    if (s->unk58C > 300) {
        s->unk14A = 3;
        rate = 0.0f;
        /* (int literals: as 60.0f this shares its text with 80039FCC's argument below, and IDO loads it twice) */
        if (speed < 60) {
            speed = 60;
        }
        if ((D_800A4FC4 != 0 || D_800A4FB4 != 0) && D_800A503C == 0) {
            target = 0.38461667f;
        } else {
            target = 0.23083334f;
        }
        if ((D_800A4FC4 == 0 && D_800A4FB4 == 0) || !func_m031_00000(s->player)) {
            rumble_raise(s->player, 40, 0.2f);
        }
    } else {
        s->unk14A = 0;
        if ((D_800A4FC4 != 0 || D_800A4FB4 != 0) && D_800A503C == 0) {
            target = 0.30768335f;
        } else {
            target = 0.15416667f;
        }
        rate = 0.965f;
    }
    rl_m053_03828[s->player] += (target - rl_m053_03828[s->player]) * (1.0f - func_80049BEC(rate, updateRate));
    if (s->unk58C < 0) {
        s->unk58C = 0;
    }
    speed *= rl_m053_03828[s->player];
    if ((s->state == 0 || s->state == 5 || s->state == 11) && !(s->pose->unk5 & s->unk532) && s->unk188 == 0) {
        s->unk185 += updateRate;
        if (s->unk185 >= 16) {
            s->unk57C = obj->trans.y_position;
            s->unk56A = s->state;
            s->state = 3;
        }
    } else {
        s->unk185 = 0;
    }
    if (s->pose->unk5 & s->unk532) {
        s->unk57C = obj->trans.y_position;
    }
    if (s->state != 5 && s->state != 11) {
        func_80039FCC(obj, s, 1000.0f, 10.0f, 60.0f, 15.0f, 65.0f, updateRate);
    } else {
        s->unk1F4 = 1;
    }
    if (s->state != 10) {
        player_charge_meter_update(s, 0, updateRate);
    }
    if ((track_height_at(obj->trans.x_position, obj->trans.z_position, &s->unk5C, 0x2000) & 0x2000) &&
        obj->trans.y_position - 16.0f < s->unk5C) {
        s->unk60 = s->unk5C - (obj->trans.y_position - 16.0f);
    } else {
        s->unk60 = 0.0f;
    }
    if (obj->interactObj->hitPoints < 0x100) {
        obj->interactObj->hitPoints = 0x100;
    }
    speed = func_m053_019B0((s16 *) obj, (struct M053Turn *) s, speed, angle, updateRate);
    switch (s->state) {
        case 0:
        case 3:
            func_m053_00EA0(obj, s, speed, angle, updateRate);
            break;
        case 5:
        case 11:
            func_m053_014F4((s16 *) obj, (struct M053Steer *) s, speed, angle, updateRate);
            break;
        case 10:
            func_m053_01850(obj, (struct M053Slow *) s, speed, angle, updateRate);
            break;
    }
    func_m053_0271C(obj, (struct M053Anim *) s, speed, updateRate);
    player_platform_ride(obj, s, updateRate);
    func_m053_01BB4(obj, s, updateRate);
    if (D_800A503C != 0) {
        if (s->unkCC == 0) {
            func_80001248(0x18E, (s32) &s->unkCC);
        }
        if (s->unkCC != 0) {
            vol = 0.3f - s->unk4 * 0.035f;
            if (vol < 0.3f) {
                vol = 0.3f;
            }
            if (0.8f < vol) {
                vol = 0.8f;
            }
            sound_pitch_set(s->unkCC, vol);
        }
    } else {
        if (s->unkBC == 0) {
            audspat_play_sound_at_position(0x18E, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1,
                                           (void **) &s->unkBC);
        }
        if (s->unkBC != 0) {
            vol = 0.3f - s->unk4 * 0.035f;
            if (vol < 0.3f) {
                vol = 0.3f;
            }
            if (0.8f < vol) {
                vol = 0.8f;
            }
            audspat_point_set_position(s->unkBC, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
            audspat_point_set_pitch(s->unkBC, (s32) (vol * 100.0f));
        }
    }
    if (s->state != 5 && s->state != 11) {
        func_m053_023D0((s16 *) obj, (struct M053Cam *) s, updateRate);
    }
    if (s->unk5BC != NULL) {
        func_m107_00D08(s->unk5BC, 1.5f, 0.5f, 40);
    }
    func_m053_021E4(obj, (struct M053HitState *) s, updateRate);
    k = 1.0f - func_80049BEC(0.9f, updateRate);
    s->unk11E = angle_lerp(s->unk11E, rl_m053_03824, k);
    s->unk120 = angle_lerp(s->unk120, rl_m053_03826, k);
    *rl_m053_03820 = 0x22;
    rl_m053_03820++;
    *rl_m053_03820 = -(s->unk120 >> 1);
    rl_m053_03820++;
    *rl_m053_03820 = 0x1E;
    rl_m053_03820++;
    *rl_m053_03820 = s->unk11E * 2;
    rl_m053_03820++;
    *rl_m053_03820 = 0x10;
    rl_m053_03820++;
    *rl_m053_03820 = s->unk14 * 23119.207f;
    rl_m053_03820++;
    *rl_m053_03820 = 0xA;
    rl_m053_03820++;
    *rl_m053_03820 = s->unk14 * -23119.207f;
    rl_m053_03820++;
    *rl_m053_03820 = 0x1000;
    rl_m053_03820++;
    inst = (M053ModelInst *) obj->modelInstances[obj->modelIndex];
    if (inst != NULL && inst->unkA != 0) {
        model = inst->model;
        x = s->unk48;
        y = obj->trans.y_position;
        obj->trans.y_position = s->unk14 * 16.0f;
        if (obj->trans.y_position > 20.0f) {
            obj->trans.y_position = 20.0f;
        }
        obj->trans.y_position += y;
        obj->trans.rotation.x_rotation += s->unk11E;
        obj->trans.rotation.z_rotation += s->unk120;
        func_8003D21C(inst, model, obj, x, 0.0f);
        s->flags.useOverride = 0;
        func_800221F0(obj, inst, obj->shading, inst->unk10[inst->unkB]);
        func_m053_02708(model, inst, s, inst->unkA);
        inst->unkA = 0;
        obj->trans.y_position = y;
        obj->trans.rotation.x_rotation -= s->unk11E;
        obj->trans.rotation.z_rotation -= s->unk120;
    }
    if ((D_800A4FC4 != 0 && D_800A4FD8 == 4) || D_800A4FB4 != 0) {
        if (D_800A18B0->unk0 & gPlayerButtonsPressed) {
            func_m031_01C68(obj, s->player);
        }
    } else {
        func_m053_028CC(obj, (struct M053State *) s, updateRate);
    }
    player_loop_sounds_update(obj, s, 0, 3, updateRate);
    func_m053_0230C(obj, (struct M053Water *) s, updateRate);
    if (s->unk14B > 0) {
        s->unk14B -= updateRate;
        obj->particleEmittersEnabled |= 0x80;
    }
    obj_spawn_particle(obj, updateRate);
    if (s->sounds[0] != 0) {
        audspat_point_set_position(s->sounds[0], obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    }
    if (s->sounds[1] != 0) {
        audspat_point_set_position(s->sounds[1], obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    }
    if (s->sounds[2] != 0) {
        audspat_point_set_position(s->sounds[2], obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
    }
}

extern s32 func_m031_01C20(s32 n); /* uses one of racer n's boosts */

/*
 * The vehicle on the move (states 0 and 3): the button that starts aiming (state 11), the boost button, steering
 * towards the stick's angle (or the turn's) and the forward speed towards the stick's, with the camera's tilt
 * angles from the acceleration and the turn. Called from 00038.
 */
static void func_m053_00EA0(Object *obj, M053Vehicle *s, f32 speed, s16 angle, f32 updateRateF) {
    UNUSED f32 pad3C;
    UNUSED s16 pad3A;
    s16 d;
    f32 accel;
    f32 rate; /* a rate, then the blend factor from it */

    s->unk594 = 0.4f;
    if ((D_800A4FC4 == 0 || D_800A4FD8 != 4) && D_800A4FB4 == 0 && (gPlayerButtonsHeld & 0x10)) {
        s->unk1E2 = 0;
        s->unk1E0 = 0;
        s->state = 11;
        s->unk1CE = 0;
        s->unk1D0 = 0;
        s->unk1E4 = 0.0f;
        s->unk1E8 = 0.0f;
        s->unk1C6 = obj->trans.rotation.y_rotation;
        s->unk1C8 = obj->trans.rotation.x_rotation;
        s->unk1CA = s->unk1C6;
        s->unk1CC = s->unk1C8;
        s->unk1DC = s->unk1CE;
        s->unk1DE = s->unk1D0;
    } else {
        if (gPlayerButtonsPressed & 0x8000) {
            if ((D_800A4FC4 != 0 && D_800A4FD8 == 4) || D_800A4FB4 != 0) {
                if (func_m031_01C20(s->player)) {
                    s->unk58C = 420;
                    if (D_800A503C != 0) {
                        func_80001248(0x18F, 0);
                    } else {
                        audspat_play_sound_at_position(0x18F, obj->trans.x_position, obj->trans.y_position,
                                                       obj->trans.z_position, 4, NULL);
                    }
                }
            } else if (s->unk58C == 0) {
                s->unk58C = 420;
                func_80001248(0x18F, 0);
            }
        }
        if ((D_800A4FC4 == 0 || D_800A4FD8 != 4) && D_800A4FB4 == 0 && (D_800A18B0->unk0 & gPlayerButtonsHeld)) {
            s->unk1F4 = 15;
        }
        if (s->unk13E >= 2) {
            angle = s->unk13C;
        }
        speed = -speed;
        if (s->unk13E >= 2 && s->unk576 != 0) {
            d = (s32) updateRateF * 2188;
            if (s->unk13F < 8) {
                s->unk11C -= d;
            } else {
                s->unk11C += d;
            }
            s->unk576 -= (s32) updateRateF;
            if (s->unk576 < 0) {
                s->unk576 = 0;
            }
        } else {
            if (s->unk13E) {
                rate = 0.925f;
            } else {
                rate = 0.94f;
            }
            d = angle_lerp(s->unk11C, angle, 1.0f - rate) - s->unk11C;
            rate = 1.0f - func_80049BEC(rate, updateRateF);
            if (s->unk13E == 0) {
                if (d < -0x280) {
                    if (s->unk13F > 0) {
                        s->unk13F--;
                    }
                } else if (d > 0x280) {
                    if (s->unk13F < 16) {
                        s->unk13F++;
                    }
                } else {
                    s->unk13F = 8;
                }
            }
            s->unk11C = angle_lerp(s->unk11C, angle, rate);
        }
        if (speed > 0.0f) {
            rate = 0.0f;
        } else {
            rate = 1.0f - func_80049BEC(speed * 0.0075f + 0.99f, updateRateF);
        }
        s->unk128 = angle_lerp(s->unk128, s->unk11C, rate);
        speed *= func_80049C30((s16) (s->unk11C - s->unk128));
        if (speed > 0.0f) {
            speed = 0.0f;
        }
        if (gPlayerButtonsHeld & 0x4000) {
            rate = 0.9625f;
            speed = 0.0f;
        } else if (speed < s->unk4) {
            rate = 0.9725f;
        } else {
            rate = 0.985f;
        }
        accel = (s->unk4 - speed) * (1.0f - rate);
        s->unk4 += (speed - s->unk4) * (1.0f - func_80049BEC(rate, updateRateF));
        if (-0.05f < s->unk4 && -0.05f < speed) {
            s->unk4 = 0.0f;
        }
        rl_m053_03824 = accel * 8192.0f;
        if (rl_m053_03824 < -0xC00) {
            rl_m053_03824 = -0xC00;
        }
        if (rl_m053_03824 > 0xE00) {
            rl_m053_03824 = 0xE00;
        }
        rl_m053_03826 = d * 6.0f;
        if (rl_m053_03826 < -0x1100) {
            rl_m053_03826 = -0x1100;
        }
        if (rl_m053_03826 > 0x1100) {
            rl_m053_03826 = 0x1100;
        }
    }
}

/* The part of the vehicle's state that 014F4 uses */
typedef struct M053Steer {
    /* 0x000 */ u8 pad0[0x4];
    /* 0x004 */ f32 unk4;
    /* 0x008 */ u8 pad8[0x104 - 0x8];
    /* 0x104 */ s16 unk104;
    /* 0x106 */ u8 pad106[0x1C6 - 0x106];
    /* 0x1C6 */ s16 unk1C6;
    /* 0x1C8 */ s16 unk1C8;
    /* 0x1CA */ s16 unk1CA;
    /* 0x1CC */ s16 unk1CC;
    /* 0x1CE */ s16 unk1CE;
    /* 0x1D0 */ s16 unk1D0;
    /* 0x1D2 */ u8 pad1D2[0x1DC - 0x1D2];
    /* 0x1DC */ s16 unk1DC;
    /* 0x1DE */ s16 unk1DE;
    /* 0x1E0 */ s16 unk1E0;
    /* 0x1E2 */ s16 unk1E2;
    /* 0x1E4 */ u8 pad1E4[0x1F6 - 0x1E4];
    /* 0x1F6 */ s8 unk1F6;
    /* 0x1F7 */ s8 unk1F7;
    /* 0x1F8 */ u8 pad1F8[0x568 - 0x1F8];
    /* 0x568 */ u8 unk568;
} M053Steer;

extern s32 gPlayerButtonsHeld;
extern s32 player_stick_aim_update(s16 *angles, M053Steer *s, s16 arg2, s16 arg3, s32 arg4, s32 arg5, s16 *yaw,
                                   s16 *pitch, f32 updateRate);
extern s16 angle_lerp(s16 from, s16 to, f32 t);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_8004966C(s16 angle); /* sine */
extern s16 *rl_m053_03820;          /* .bss (still assembly): the camera commands */

/* Steering the vehicle and its camera while the button is held (called from 00038) */
static void func_m053_014F4(s16 *angles, M053Steer *s, UNUSED f32 speed, UNUSED s32 angle, f32 updateRateF) {
    s16 yaw;
    s16 pitch;
    f32 k;

    if (!(gPlayerButtonsHeld & 0x10)) {
        s->unk568 = 0;
        s->unk104 = 0x8000 - angles[0];
        return;
    }
    s->unk4 *= func_80049BEC(0.95f, (s32) updateRateF);
    if (-0.1f < s->unk4 && s->unk4 < 0.1f) {
        s->unk4 = 0.0f;
    }
    player_stick_aim_update(angles, s, 0xE38, 0xE38, -0x2AAA, 0x2AAA, &yaw, &pitch, updateRateF);
    k = 1.0f - func_80049BEC(0.8333f, (s32) updateRateF);
    s->unk1E0 = angle_lerp(s->unk1E0, pitch, k);
    pitch += s->unk1E2;
    s->unk1DC = angle_lerp(s->unk1DC, yaw, k);
    s->unk1DE = angle_lerp(s->unk1DE, pitch, k);
    s->unk1CE = angle_lerp(s->unk1CE, yaw, k);
    s->unk1D0 = angle_lerp(s->unk1D0, pitch, k);
    /* ((&s->f)[0]: the field read first, as the ROM does) */
    s->unk1C6 = angles[0] + (&s->unk1CE)[0];
    s->unk1C8 = angles[1] + (&s->unk1D0)[0];
    s->unk1CA = s->unk1C6;
    s->unk1CC = s->unk1C8;
    yaw = s->unk1CE;
    pitch = s->unk1D0;
    if (s->unk1F6) {
        pitch += (s->unk1F7 * func_8004966C(s->unk1F6 << 12)) >> 8;
    }
    *rl_m053_03820 = 0x24;
    *++rl_m053_03820 = s->unk1DE;
    *++rl_m053_03820 = 0x26;
    *++rl_m053_03820 = s->unk1DC;
    *++rl_m053_03820 = 0x18;
    *++rl_m053_03820 = pitch;
    *++rl_m053_03820 = 0x1A;
    *++rl_m053_03820 = yaw;
    *++rl_m053_03820 = 0x12;
    *++rl_m053_03820 = pitch;
    *++rl_m053_03820 = 0x14;
    *++rl_m053_03820 = yaw;
    rl_m053_03820++;
}

/* The part of the vehicle's state that 01850 uses */
typedef struct M053Slow {
    /* 0x000 */ u8 pad0[0x2];
    /* 0x002 */ u8 unk2;
    /* 0x003 */ u8 pad3[0x4 - 0x3];
    /* 0x004 */ f32 unk4;
    /* 0x008 */ u8 pad8[0x10 - 0x8];
    /* 0x010 */ f32 unk10;
    /* 0x014 */ u8 pad14[0x60 - 0x14];
    /* 0x060 */ f32 unk60;
    /* 0x064 */ u8 pad64[0x193 - 0x64];
    /* 0x193 */ u8 unk193;
    /* 0x194 */ u8 pad194[0x19C - 0x194];
    /* 0x19C */ s8 unk19C;
    /* 0x19D */ u8 pad19D[0x541 - 0x19D];
    /* 0x541 */ u8 unk541;
    /* 0x542 */ u8 pad542[0x594 - 0x542];
    /* 0x594 */ f32 unk594;
} M053Slow;

extern f32 func_80049BEC(f32 rate, s32 updateRate);

/* Slows the vehicle down by the frame's rate (called from 00038) */
static void func_m053_01850(Object *obj, M053Slow *s, UNUSED f32 speed, UNUSED s32 angle, f32 updateRateF) {
    s32 updateRate;
    f32 f;

    obj->interactObj->hitPoints = 0;
    updateRate = updateRateF;
    f = func_80049BEC(0.85f, updateRate);
    s->unk4 *= f;
    s->unk541 &= ~0xF;
    s->unk10 *= f;
    if (s->unk2 == 0) {
        s->unk594 *= f;
    } else {
        s->unk594 = s->unk60 * 0.03125f;
        if (s->unk594 > 1.5f) {
            s->unk594 = 1.5f;
        }
        obj->velocity.y *= func_80049BEC(0.95f, updateRate);
    }
    if (0.98f < obj->unk28) {
        if (s->unk19C + updateRate >= 0x80) {
            obj->trans.flags |= 0x400;
            s->unk193 = 1;
        } else {
            s->unk19C += updateRate;
        }
    }
}

/* The part of the vehicle's state that 019B0 uses */
typedef struct M053Turn {
    /* 0x000 */ s8 player;
    /* 0x001 */ u8 pad1[0x13C - 0x1];
    /* 0x13C */ s16 turnYaw;  /* the yaw when the turn started */
    /* 0x13E */ u8 turning;   /* 0: no, 1: turning, 2: turning hard */
    /* 0x13F */ u8 unk13F;
    /* 0x140 */ u8 pad140[0x568 - 0x140];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 pad569[0x576 - 0x569];
    /* 0x576 */ s8 turnTimer;
} M053Turn;

extern u8 D_800A4FC4; /* a multiplayer game */
extern u8 D_800A4FB4;
extern s32 func_m031_00000(s32 n); /* whether racer n is computer-driven */
extern void rumble_raise(s32 player, s32 strength, f32 time);

/*
 * Turning round: starts a turn when the wanted yaw is about half a turn from the vehicle's, rumbles
 * while it turns and ends it near the target; returns the speed (0 while turning). Called from 00038.
 */
static f32 func_m053_019B0(s16 *yaw, M053Turn *s, f32 speed, s16 target, UNUSED f32 updateRateF) {
    s16 d;

    if (s->unk568 != 0 && s->unk568 != 3) {
        s->turning = 0;
        s->unk13F = 8;
        s->turnTimer = 0;
    } else if (s->turning == 0) {
        d = *yaw - target;
        if (d < -0x7000 || d > 0x7000) {
            s->turnYaw = target;
            if (s->unk13F == 0 || s->unk13F == 16) {
                s->turnTimer = 60;
                s->turning = 2;
            } else {
                s->turnTimer = 0;
                s->turning = 1;
            }
        }
    } else {
        if ((!D_800A4FC4 && !D_800A4FB4) || !func_m031_00000(s->player)) {
            rumble_raise(s->player, 40, 0.2f);
        }
        if (s->turnTimer == 0) {
            if (s->turning >= 2) {
                d = s->turnYaw - *yaw;
                if (d >= -0x38D && d < 0x38E) {
                    s->turning = 0;
                }
            } else {
                d = s->turnYaw - target;
                if (d >= -0x6FFF && d < 0x7000) {
                    s->turning = 0;
                }
                d = target - *yaw;
                if (d >= -0x38D && d < 0x38E) {
                    s->turning = 0;
                }
            }
        }
        if (s->turning == 0) {
            s->unk13F = 8;
            s->turnTimer = 0;
        } else {
            speed = 0.0f;
        }
    }
    return speed;
}

extern f32 D_800F6DC4; /* gravity */
extern s32 gPlayerInputLocked;
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern s32 func_80034BB0(Object *obj, M053Vehicle *s, f32 updateRate);

/*
 * Moves the vehicle (a form of m016's move): gravity, the slope, the speeds turned into a velocity, the move and
 * its collision, then the speed limit eased towards 0.5 on the ground (rumbling when fast) or 25 in the air.
 * Called from 00038.
 */
static void func_m053_01BB4(Object *obj, M053Vehicle *s, f32 updateRate) {
    s32 hit;
    f32 dx;
    f32 dz;
    f32 x; /* the turned velocity's sideways part */
    UNUSED s32 pad;
    Vec3f vec;
    f32 t; /* also 1 / updateRate */
    s16 angle;
    Vec3s rot;

    obj->velocity.y -= (1.0f - s->unk594) * (D_800F6DC4 * updateRate);
    if (obj->velocity.y > 2.5f) {
        obj->velocity.y += (2.5f - obj->velocity.y) * (1.0f - func_80049BEC(0.9f, updateRate));
    }
    if (s->unk50 < 0.0f) {
        t = -s->unk50 - 0.3f;
        if (t < 0.0f) {
            t = 0.0f;
        }
        if (t > 0.1f) {
            t = 0.1f;
        }
        t *= 35.0f;
    } else {
        t = 0.0f;
    }
    s->unk8 = 0.0f;
    s->unk4 += D_800F6DC4 * (s->unk50 / (4.0f - t)) * updateRate;
    obj->trans.rotation.y_rotation = s->unk11C;
    angle = s->unk128;
    obj->velocity.x = func_80049C34(angle) * s->unk4;
    obj->velocity.z = func_80049C30(angle) * s->unk4;
    angle = s->unk128;
    if (angle) {}
    obj->velocity.x += s->unk10 * func_80049C30(angle);
    obj->velocity.z -= s->unk10 * func_80049C34(angle);
    dx = obj->velocity.x;
    dz = obj->velocity.z;
    if (s->unk149 != 0) {
        dx += s->unk6C;
        dz += s->unk70;
    }
    if (gPlayerInputLocked == 1) {
        dx *= 0.65f;
        dz *= 0.65f;
        if (s->unk4 < -0.5f || s->unk4 > 0.5f) {
            s->unk4 *= 0.65f;
        } else {
            s->unk4 = 0.0f;
        }
        if (s->unk8 < -0.5f || s->unk8 > 0.5f) {
            s->unk8 *= 0.65f;
        } else {
            s->unk8 = 0.0f;
        }
    }
    func_80009A04(obj, dx * updateRate + s->unk354, obj->velocity.y * updateRate, dz * updateRate + s->unk35C);
    t = 1.0f / updateRate;
    dx = (obj->trans.x_position - s->unk3C) * t;
    dz = (obj->trans.z_position - s->unk44) * t;
    hit = func_80034BB0(obj, s, updateRate);
    obj->velocity.y = (obj->trans.y_position - s->unk40) * t;
    angle = s->unk128;
    rot.y_rotation = -angle;
    rot.x_rotation = -obj->trans.rotation.x_rotation;
    rot.z_rotation = -obj->trans.rotation.z_rotation;
    vec.x = dx; vec.y = 0.0f; vec.z = dz; func_80049394(&rot, &vec);
    x = vec.x;
    if (s->unk149 != 0) {
        s->unk149 -= (s32) updateRate;
        if (s->unk149 < 0) {
            s->unk149 = 0;
        }
    } else {
        t = s->unk4 - vec.z;
        if (t > 0.5f) {
            s->unk4 -= t - 0.5f;
        }
        if (t < -0.5f) {
            s->unk4 -= t + 0.5f;
        }
        t = s->unk8 - x;
        if (t > 0.5f) {
            s->unk8 -= t - 0.5f;
        }
        if (t < -0.5f) {
            s->unk8 -= t + 0.5f;
        }
    }
    if (hit & 3) {
        /* (.5f: a literal text of its own; written 0.5f it shares the slope limits' register-allocated 0.5f) */
        s->unkC += (.5f - s->unkC) * (1.0f - func_80049BEC(0.95f, updateRate));
        if (s->unk4 < -s->unkC) {
            s->unk4 = -s->unkC;
        }
        if (s->unk4 > s->unkC) {
            s->unk4 = s->unkC;
        }
        if (s->unk8 < -s->unkC) {
            s->unk8 = -s->unkC;
        }
        if (s->unk8 > s->unkC) {
            s->unk8 = s->unkC;
        }
        if ((D_800A4FC4 == 0 && D_800A4FB4 == 0) || !func_m031_00000(s->player)) {
            if (s->unk4 < -5.0f || s->unk4 > 5.0f) {
                rumble_raise(s->player, 50, 0.2f);
            }
        }
    } else {
        s->unkC += (25.0f - s->unkC) * (1.0f - func_80049BEC(0.875f, updateRate));
    }
}

/* A hit record (0x34 bytes) of the vehicle's interaction */
typedef struct M053Hit {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags;
    /* 0x0C */ u8 padC[0x34 - 0xC];
} M053Hit;

/* The vehicle's interaction (obj->interactObj), as far as 021E4 uses it */
typedef struct M053Interact {
    /* 0x00 */ u8 pad0[0x6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0x32 - 0x8];
    /* 0x32 */ s16 hitCount;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ M053Hit *hits;
} M053Interact;

/* The part of the vehicle's state that 021E4 uses */
typedef struct M053HitState {
    /* 0x000 */ s8 player;
    /* 0x001 */ u8 pad1[0x148 - 0x1];
    /* 0x148 */ s8 flash;
    /* 0x149 */ u8 pad149[0x190 - 0x149];
    /* 0x190 */ u8 hitTimer; /* 0x80: just hit */
} M053HitState;

extern void player_voice_play(Object *obj, u8 index);
extern void menu_draw_layout_box(s32 arg0, s32 index, s32 colour);

/* The vehicle being hit: the voice, a hit point less and the player's screen flashing red (called from
   00038) */
static void func_m053_021E4(Object *obj, M053HitState *s, s32 updateRate) {
    M053Hit *hit;
    s16 n;
    s32 wasHit;

    n = ((M053Interact *) obj->interactObj)->hitCount;
    hit = ((M053Interact *) obj->interactObj)->hits;
    wasHit = FALSE;
    while (n--) {
        if (hit->flags & 0x40) {
            wasHit = TRUE;
        }
        hit++;
    }
    if (s->hitTimer == 0x80) {
        s->hitTimer = 60;
        wasHit = TRUE;
        ((M053Interact *) obj->interactObj)->hitPoints -= 0x100;
    } else {
        s->hitTimer &= 0x7F;
        if (updateRate < s->hitTimer) {
            s->hitTimer -= updateRate;
        } else {
            s->hitTimer = 0;
        }
    }
    if (wasHit) {
        player_voice_play(obj, 3);
        s->flash = 8;
    } else {
        s->flash -= updateRate;
        if (s->flash < 0) {
            s->flash = 0;
        }
    }
    if (s->flash != 0) {
        menu_draw_layout_box(0, s->player, 0xFF0000FF);
    }
}

/* The object's ripple data (69050.c's RippleInit), as far as 0230C uses it */
typedef struct M053Ripple {
    /* 0x00 */ u8 pad0[0x75];
    /* 0x75 */ u8 unk75;
    /* 0x76 */ u8 pad76[0x80 - 0x76];
    /* 0x80 */ f32 levelY; /* the level height under it */
    /* 0x84 */ u8 *child;  /* its child effect (its first byte holds flags) */
} M053Ripple;

/* An object with ripple data at 0x58 */
typedef struct M053RippleObj {
    /* 0x00 */ u8 pad0[0x58];
    /* 0x58 */ M053Ripple *ripple;
} M053RippleObj;

/* The vehicle's state, as far as 0230C uses it */
typedef struct M053Water {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ f32 unk4;
    /* 0x08 */ u8 pad8[0x5C - 0x8];
    /* 0x5C */ f32 waterY;
    /* 0x60 */ f32 depth;
} M053Water;

extern void ripple_step_frame(Object *obj, s32 updateRate); /* moves a ripple */

/* Updates the vehicle's ripple: on the water while it is in it, its child shown outside unk4's limits
   (called from 00038) */
static void func_m053_0230C(Object *obj, M053Water *w, s32 updateRate) {
    M053Ripple *r = ((M053RippleObj *) obj)->ripple;
    u8 *child = r->child;

    r->unk75 = 0;
    if (child != NULL) {
        *child &= ~2;
    }
    if (w->depth > 0.0f) {
        ((M053RippleObj *) obj)->ripple->levelY = w->waterY;
        ((M053RippleObj *) obj)->ripple->unk75 = 1;
        if (child != NULL && (w->unk4 < -1.2f || 1.2f < w->unk4)) {
            *child |= 2;
        }
    }
    ripple_step_frame(obj, updateRate);
}

/* The part of the vehicle's state that 023D0 uses: the camera's offsets */
typedef struct M053Cam {
    /* 0x000 */ u8 pad0[0x1C6];
    /* 0x1C6 */ s16 unk1C6;
    /* 0x1C8 */ s16 unk1C8;
    /* 0x1CA */ u8 pad1CA[0x1CE - 0x1CA];
    /* 0x1CE */ s16 unk1CE;
    /* 0x1D0 */ s16 unk1D0;
    /* 0x1D2 */ u8 pad1D2[0x1D8 - 0x1D2];
    /* 0x1D8 */ s16 unk1D8;
    /* 0x1DA */ s16 unk1DA;
    /* 0x1DC */ s16 unk1DC;
    /* 0x1DE */ s16 unk1DE;
    /* 0x1E0 */ u8 pad1E0[0x1F4 - 0x1E0];
    /* 0x1F4 */ s8 unk1F4;
    /* 0x1F5 */ s8 unk1F5;
    /* 0x1F6 */ s8 unk1F6;
    /* 0x1F7 */ s8 unk1F7;
} M053Cam;

extern s32 func_8004966C(s16 angle); /* sine */
extern s16 *rl_m053_03820;          /* .bss (still assembly): the camera commands */

/* Eases the camera's offsets towards the vehicle's angles and writes them as camera commands (called from
   00038) */
static void func_m053_023D0(s16 *angles, M053Cam *s, s32 updateRate) {
    s16 i;
    s16 a;
    s16 b;

    a = s->unk1C8 - angles[1];
    if (a < -0x1800) {
        a = -0x1800;
    }
    if (a > 0x3000) {
        a = 0x3000;
    }
    b = s->unk1C6 - angles[0];
    if (b < -0x1000) {
        b = -0x1000;
    }
    if (b > 0x1000) {
        b = 0x1000;
    }
    for (i = 0; i < updateRate; i++) {
        s->unk1D0 += (a - s->unk1D0) >> 2;
        s->unk1CE += (b - s->unk1CE) >> 2;
    }
    if (s->unk1F5 > 0) {
        a = s->unk1DA;
        b = s->unk1D8;
        s->unk1F5 -= updateRate;
    } else {
        a = 0;
        b = 0;
    }
    /* (one line: IDO schedules by line) */
    if (a < -14563.553f) { a = -0x38E3; } if (14563.553f < a) {
        a = 0x38E3;
    }
    if (b < -3640.8882f) {
        b = -0xE38;
    }
    if (3640.8882f < b) {
        b = 0xE38;
    }
    for (i = 0; i < updateRate; i++) {
        s->unk1DE += (a - s->unk1DE) >> 4;
        s->unk1DC += (b - s->unk1DC) >> 4;
    }
    if (s->unk1F4) {
        a = s->unk1D0;
        if (s->unk1F6) {
            a += (s->unk1F7 * func_8004966C(s->unk1F6 << 12)) >> 8;
        }
        *rl_m053_03820 = 0x24;
        *++rl_m053_03820 = s->unk1DE;
        *++rl_m053_03820 = 0x26;
        *++rl_m053_03820 = s->unk1DC;
        *++rl_m053_03820 = 0x18;
        *++rl_m053_03820 = a;
        *++rl_m053_03820 = 0x1A;
        *++rl_m053_03820 = b;
        *++rl_m053_03820 = 0x12;
        *++rl_m053_03820 = a;
        *++rl_m053_03820 = 0x14;
        *++rl_m053_03820 = b;
        rl_m053_03820++;
    }
}

/* Does nothing (called from 00038) */
static void func_m053_02708(UNUSED void *arg0, UNUSED void *arg1, UNUSED void *arg2, UNUSED s32 arg3) {
}

/* The part of the vehicle's state that 0271C uses */
typedef struct M053Anim {
    /* 0x000 */ u8 pad0[0x4];
    /* 0x004 */ f32 unk4;
    /* 0x008 */ u8 pad8[0x14 - 0x8];
    /* 0x014 */ f32 unk14;
    /* 0x018 */ u8 pad18[0x58C - 0x18];
    /* 0x58C */ s16 unk58C;
} M053Anim;


/* Plays the vehicle's two animations and eases unk14 towards 0, 1 or 2 (called from 00038) */
static void func_m053_0271C(Object *obj, M053Anim *s, UNUSED f32 speed, f32 updateRateF) {
    f32 k;

    k = 1.0f - func_80049BEC(0.975f, updateRateF);
    switch (obj->animationID) {
        case 0:
            obj_anim_frame_advance(obj, 0.005f, updateRateF);
            if (s->unk4 < -1.2f || 1.2f < s->unk4) {
                obj_anim_set(obj, 1, obj->unk28);
            }
            s->unk14 += (0.0f - s->unk14) * k;
            break;
        case 1:
            obj_anim_frame_advance(obj, 0.01f, updateRateF);
            if (-0.8f < s->unk4 && s->unk4 < 0.8f) {
                obj_anim_set(obj, 0, obj->unk28);
            }
            if (s->unk58C > 300) {
                s->unk14 += (2.0f - s->unk14) * k;
            } else {
                s->unk14 += (1.0f - s->unk14) * k;
            }
            break;
    }
}

/* The part of the vehicle's state that 028CC clears */
typedef struct M053State {
    /* 0x000 */ u8 pad0[0x538];
    /* 0x538 */ u8 unk538;
} M053State;

/* Clears the state's byte at 0x538 (called from 00038) */
static void func_m053_028CC(UNUSED Object *obj, M053State *state, UNUSED s32 updateRate) {
    state->unk538 = 0;
}

extern s32 func_8001B078(f32 x, f32 z, s32 arg2, f32 ***heights); /* the floor heights at (x, z) */

/*
 * Moves `to` out of the way of what lies between `from` and it: a sweep from `from` (repeated through the
 * vehicles' own objects, behaviours 0x30 and 0x31, once) stops it at the first hit; then its height is put
 * on a floor just below or above it, and, if nothing was hit, kept no lower than it was. (Called from 02B48.)
 * The last loop over the floors does nothing: its test has an empty body, as in the ROM.
 */
static void func_m053_028DC(f32 *from, Vec3f *to, UNUSED f32 height) {
    s32 mode;
    HitResult hit;
    UNUSED s32 pad;
    s32 done;
    s32 n;
    s32 i;
    f32 y0;
    Vec3f pos;
    f32 **heights;

    pos.x = from[0];
    pos.y = from[1];
    pos.z = from[2];
    y0 = to->y;
    mode = 1;
    done = 0;
    do {
        if (!func_80018808(NULL, &pos, to, &hit, 0, 0)) {
            done = 1;
        } else if (mode != 2 && hit.obj != NULL && (hit.obj->behaviorId == 0x30 || hit.obj->behaviorId == 0x31)) {
            mode = 2;
            pos.x = hit.pos.x;
            pos.y = hit.pos.y;
            pos.z = hit.pos.z;
            to->y = hit.pos.y;
        } else {
            mode = 3;
            to->x = hit.pos.x;
            done = 1;
            to->y = hit.pos.y;
            to->z = hit.pos.z;
        }
    } while (!done);
    n = func_8001B078(to->x, to->z, 0xC00, &heights);
    while (n--) {
        f32 lim = *heights[n] + 1.0f;
        f32 h = *heights[n];

        if (h < to->y && to->y < lim) {
            to->y = h;
        }
    }
    n = func_8001B078(to->x, to->z, 0xC00, &heights);
    while (n--) {
        f32 h = *heights[n];
        f32 lim = *heights[n] - 1.0f;

        if (lim < to->y && to->y < h) {
            to->y = h;
        }
    }
    if (mode == 1) {
        n = func_8001B078(to->x, to->z, 0xC00, &heights);
        if (n != 0) {
            done = 0;
            for (i = 0; i < n && !done; i++) {
                if (*heights[i] < from[1]) {
                }
            }
        }
        if (to->y < y0) {
            to->y = y0;
        }
    }
}

/* The part of the vehicle's state that the camera (02B48) uses */
typedef struct M053CamState {
    /* 0x000 */ s8 player;
    /* 0x001 */ u8 pad1[0x104 - 0x1];
    /* 0x104 */ s16 yaw; /* the camera's angle round the vehicle */
    /* 0x106 */ u8 pad106[0x10A - 0x106];
    /* 0x10A */ s16 unk10A;
    /* 0x10C */ u8 pad10C[0x128 - 0x10C];
    /* 0x128 */ s16 unk128;
    /* 0x12A */ u8 pad12A[0x574 - 0x12A];
    /* 0x574 */ u8 smooth; /* ease the camera in */
} M053CamState;

/* The camera 02B48 moves: a transform and its last position */
typedef struct M053Camera {
    /* 0x00 */ ObjectTransform trans;
    /* 0x18 */ Vec3f prev;
} M053Camera;

extern void func_8003F7F4(f32 fov, s32 arg1);      /* sets the field of view */
extern f32 func_80097140(f32 x);                   /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z);            /* the angle of a direction */
extern void func_800496B0(s32 arg0, Vec3s *rotation, Vec3f *in, Vec3f *out); /* DKR vec3f_rotate */
extern ObjectTransform *D_800F6DC0;                /* the camera's transform */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */

/*
 * The vehicle's camera: it keeps between two distances behind the vehicle (easing towards the nearer one, and
 * swinging round behind it when it turns away), looks at a point above it, and pushes the vehicle away when it
 * comes too close.
 */
void func_m053_02B48(M053Camera *cam, Object *veh, f32 updateRate) {
    M053CamState *state;
    s16 yaw;
    s16 pitch;
    s16 roll;
    s16 target; /* the camera's angle off the vehicle's back, then the yaw to swing to */
    f32 dx;
    f32 dy;
    f32 dz;
    f32 r; /* the offset to the nearer of the two distances, then the distance kept */
    f32 d1;
    f32 d2;
    f32 d3;
    f32 dist;
    f32 blend;
    f32 blend2;
    Vec3f tmp;
    Vec3f pos;
    f32 s; /* |125 - dist|, then the squared distance to the vehicle */

    state = veh->unk68;
    state->unk10A = 0;
    blend = 1.0f;
    blend2 = 1.0f;
    func_8003F7F4(60.0f, 0);
    dx = cam->trans.x_position - veh->trans.x_position;
    dz = cam->trans.z_position - veh->trans.z_position;
    dist = func_80097140(dx * dx + dz * dz);
    if (dist > 225.0f || dist < 105.0f) {
        if (state->smooth) {
            blend = 1.0f - func_80049BEC(0.975f, updateRate);
        }
        rl_m053_03848[state->player] = rl_m053_03848[state->player] + (1.0f - rl_m053_03848[state->player]) * blend;
    } else {
        if (state->smooth) {
            blend = 1.0f - func_80049BEC(0.9f, updateRate);
        }
        rl_m053_03848[state->player] = rl_m053_03848[state->player] + (0.0f - rl_m053_03848[state->player]) * blend;
        if (rl_m053_03848[state->player] < 0.025f) {
            rl_m053_03848[state->player] = 0.0f;
        }
    }
    target = func_80049C84(dx, dz) - veh->trans.rotation.y_rotation - 0x8000;
    if (dist > 225.0f && (target > 0x4000 || target < -0x4000)) {
        if (state->smooth) {
            blend2 = 1.0f - func_80049BEC(0.975f, updateRate);
        }
        rl_m053_03838[state->player] = rl_m053_03838[state->player] + (1.0f - rl_m053_03838[state->player]) * blend2;
    } else {
        if (state->smooth) {
            blend2 = 1.0f - func_80049BEC(0.8334f, updateRate);
        }
        rl_m053_03838[state->player] = rl_m053_03838[state->player] + (0.0f - rl_m053_03838[state->player]) * blend2;
        if (rl_m053_03838[state->player] < 0.025f) {
            rl_m053_03838[state->player] = 0.0f;
        }
    }
    if (0.0f < rl_m053_03838[state->player]) {
        target = 0x8000 - state->unk128;
        if (state->smooth) {
            blend = 1.0f - func_80049BEC(1.0f - rl_m053_03838[state->player] * 0.125f, updateRate);
        }
        state->yaw = angle_lerp(state->yaw, target, blend);
    } else if (state->smooth) {
        state->yaw = 0x8000 - func_80049C84(dx, dz);
    }
    r = 205.0f - dist;
    d1 = r;
    if (205.0f < dist) {
        d1 = -d1;
    }
    d2 = 125.0f - dist;
    s = d2;
    if (125.0f < dist) {
        s = -d2;
    }
    if (s < d1) {
        r = d2;
    }
    r = rl_m053_03848[state->player] * r + dist;
    pos.x = func_80049C34(state->yaw) * r + veh->trans.x_position;
    pos.y = veh->trans.y_position + 60.0f;
    pos.z = veh->trans.z_position - func_80049C30(state->yaw) * r;
    func_m053_028DC(&veh->trans.x_position, &pos, 60.0f);
    if (state->smooth) {
        blend = 1.0f - func_80049BEC(0.875f, updateRate);
    }
    cam->trans.x_position = (pos.x - D_800F6DC0->x_position) * blend + D_800F6DC0->x_position;
    cam->trans.y_position = (pos.y - D_800F6DC0->y_position) * blend + D_800F6DC0->y_position;
    cam->trans.z_position = (pos.z - D_800F6DC0->z_position) * blend + D_800F6DC0->z_position;
    cam->prev.x = cam->trans.x_position;
    cam->prev.y = cam->trans.y_position;
    cam->prev.z = cam->trans.z_position;
    tmp.x = 0.0f;
    tmp.y = 40.0f;
    tmp.z = 0.0f;
    func_800496B0(1, &veh->trans.rotation, &tmp, &tmp);
    dx = cam->trans.x_position - (tmp.x + veh->trans.x_position);
    dy = cam->trans.y_position - (tmp.y + veh->trans.y_position);
    dz = cam->trans.z_position - (tmp.z + veh->trans.z_position);
    yaw = 0x8000 - func_80049C84(dx, dz);
    roll = 0;
    pitch = func_80049C84(dy, func_80097140(dx * dx + dz * dz));
    if (state->smooth) {
        blend2 = 1.0f - func_80049BEC(0.125f, updateRate);
    }
    cam->trans.rotation.y_rotation = angle_lerp(cam->trans.rotation.y_rotation, yaw, blend2);
    cam->trans.rotation.x_rotation = angle_lerp(cam->trans.rotation.x_rotation, pitch, blend2);
    cam->trans.rotation.z_rotation = angle_lerp(cam->trans.rotation.z_rotation, roll, blend2);
    dx = veh->trans.x_position - cam->trans.x_position;
    dz = veh->trans.z_position - cam->trans.z_position;
    s = dx * dx + dz * dz;
    func_80097140(s); /* (its result is not used) */
    if (s < 1024.0f) {
        if (s != 0.0f) {
            d3 = 32.0f / func_80097140(s) - 1.0f;
            dx *= d3;
            dz *= d3;
        }
        func_80009A04(veh, dx, 0.0f, dz);
    }
}

INCLUDE_ASM("asm/nonmatchings/modules/m053", data);

/* (.rodata: C, and the assembly functions' own from ro/) */

INCLUDE_ASM("asm/nonmatchings/modules/m053", bss);
