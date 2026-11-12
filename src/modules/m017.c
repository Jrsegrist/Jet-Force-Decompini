/*
 * Runlink module 17: a player character's behaviour (the control function
 * called every frame, the firing check, and the movement states it runs).
 * A sibling of modules 15 and 16.
 */
/* obj_anim_set returns a value here (module.h declares it void): that keeps IDO off v0 after the call */
#define obj_anim_set func_8001149C_void
#include "module.h"
#undef obj_anim_set
extern s32 obj_anim_set(Object *obj, s32 anim, f32 frame);

/* What racer->pose points at */
typedef struct RacerPose {
    /* 0x00 */ u8 pad0[5];
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
} RacerPose;

/* The flag word at 0x540 (as in 2BAA0.c) */
typedef struct RacerFlags {
    u32 unk31 : 1;
    u32 unk30 : 1;
    u32 weaponDir : 2;
    u32 unk27 : 1;
    u32 useOverride : 1;
    u32 unk25 : 1;
    u32 unk24 : 1;
    u32 unk8 : 16;
    u32 unk7 : 1;
    u32 unk6 : 1;
    u32 unk0 : 6;
} RacerFlags;

/* A player's state block (the object's unk68); see 2BAA0.c */
typedef struct Racer {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 unk2;
    /* 0x003 */ u8 unk3;
    /* 0x004 */ f32 unk4;
    /* 0x008 */ f32 unk8;
    /* 0x00C */ u8 padC[0x10 - 0xC];
    /* 0x010 */ f32 unk10;
    /* 0x014 */ u8 pad14[0x48 - 0x14];
    /* 0x048 */ f32 unk48;
    /* 0x04C */ u8 pad4C[0x50 - 0x4C];
    /* 0x050 */ f32 unk50;
    /* 0x054 */ f32 unk54;
    /* 0x058 */ f32 unk58;
    /* 0x05C */ f32 unk5C;
    /* 0x060 */ f32 unk60;
    /* 0x064 */ u8 pad64[0x74 - 0x64];
    /* 0x074 */ f32 unk74;
    /* 0x078 */ s32 unk78;
    /* 0x07C */ u8 pad7C[0xB8 - 0x7C];
    /* 0x0B8 */ u16 unkB8; /* the sound playing in unkBC */
    /* 0x0BA */ u8 padBA[0xBC - 0xBA];
    /* 0x0BC */ s32 unkBC; /* sound handle */
    /* 0x0C0 */ u8 padC0[0xC8 - 0xC0];
    /* 0x0C8 */ s32 unkC8; /* sound handle */
    /* 0x0CC */ s32 unkCC; /* sound handle */
    /* 0x0D0 */ u8 padD0[0xD4 - 0xD0];
    /* 0x0D4 */ s32 unkD4;
    /* 0x0D8 */ u8 padD8[0x10D - 0xD8];
    /* 0x10D */ u8 unk10D;
    /* 0x10E */ u8 pad10E[0x11C - 0x10E];
    /* 0x11C */ s16 unk11C;
    /* 0x11E */ s16 unk11E;
    /* 0x120 */ s16 unk120;
    /* 0x122 */ u8 pad122[0x128 - 0x122];
    /* 0x128 */ s16 unk128;
    /* 0x12A */ s16 unk12A;
    /* 0x12C */ s16 unk12C;
    /* 0x12E */ u8 pad12E[0x13E - 0x12E];
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 pad13F[0x14A - 0x13F];
    /* 0x14A */ s8 unk14A;
    /* 0x14B */ s8 unk14B;
    /* 0x14C */ u8 pad14C[0x185 - 0x14C];
    /* 0x185 */ s8 unk185;
    /* 0x186 */ s8 unk186;
    /* 0x187 */ u8 pad187[0x188 - 0x187];
    /* 0x188 */ u8 unk188;
    /* 0x189 */ u8 pad189[0x198 - 0x189];
    /* 0x198 */ u8 unk198;
    /* 0x199 */ u8 pad199[0x19E - 0x199];
    /* 0x19E */ u8 unk19E;
    /* 0x19F */ u8 unk19F;
    /* 0x1A0 */ u8 pad1A0[0x1F4 - 0x1A0];
    /* 0x1F4 */ s8 unk1F4;
    /* 0x1F5 */ s8 unk1F5;
    /* 0x1F6 */ s8 unk1F6;
    /* 0x1F7 */ u8 pad1F7[0x1F9 - 0x1F7];
    /* 0x1F9 */ u8 unk1F9;
    /* 0x1FA */ u8 unk1FA;
    /* 0x1FB */ u8 pad1FB[0x240 - 0x1FB];
    /* 0x240 */ s16 cmds[0x80]; /* the command list func_m017_00000 builds */
    /* 0x340 */ u8 pad340[0x34A - 0x340];
    /* 0x34A */ s16 unk34A;
    /* 0x34C */ u8 pad34C[0x360 - 0x34C];
    /* 0x360 */ RacerPose *pose;
    /* 0x364 */ u8 pad364[0x520 - 0x364];
    /* 0x520 */ u32 unk520;
    /* 0x524 */ u8 pad524[0x532 - 0x524];
    /* 0x532 */ u8 unk532;
    /* 0x533 */ u8 pad533[0x536 - 0x533];
    /* 0x536 */ s16 unk536;
    /* 0x538 */ u8 unk538;
    /* 0x539 */ u8 pad539[0x53D - 0x539];
    /* 0x53D */ s8 unk53D;
    /* 0x53E */ u8 pad53E[0x540 - 0x53E];
    /* 0x540 */ RacerFlags flags;
    /* 0x544 */ Vec3f overridePos;
    /* 0x550 */ u8 pad550[0x568 - 0x550];
    /* 0x568 */ u8 state;
    /* 0x569 */ u8 pad569;
    /* 0x56A */ u8 unk56A;
    /* 0x56B */ s8 unk56B;
    /* 0x56C */ u8 pad56C[0x576 - 0x56C];
    /* 0x576 */ s8 unk576;
    /* 0x577 */ s8 unk577;
    /* 0x578 */ u8 pad578[0x57A - 0x578];
    /* 0x57A */ s8 unk57A;
    /* 0x57B */ u8 pad57B;
    /* 0x57C */ f32 unk57C;
    /* 0x580 */ u8 pad580[0x585 - 0x580];
    /* 0x585 */ s8 unk585;
    /* 0x586 */ u8 pad586[0x58E - 0x586];
    /* 0x58E */ s8 unk58E;
    /* 0x58F */ u8 pad58F[0x594 - 0x58F];
    /* 0x594 */ f32 unk594;
    /* 0x598 */ u8 pad598[0x5BC - 0x598];
    /* 0x5BC */ void *unk5BC;
    /* 0x5C0 */ s32 unk5C0;
    /* 0x5C4 */ u8 pad5C4[0x5C8 - 0x5C4];
    /* 0x5C8 */ s16 unk5C8;
    /* 0x5CA */ u8 pad5CA[0x5D8 - 0x5CA];
    /* 0x5D8 */ s32 unk5D8; /* sound handle */
    /* 0x5DC */ u8 pad5DC[0x5EC - 0x5DC];
    /* 0x5EC */ u8 unk5EC;
    /* 0x5ED */ u8 unk5ED; /* that sound's volume */
    /* 0x5EE */ u8 unk5EE;
    /* 0x5EF */ u8 pad5EF[0x5F0 - 0x5EF];
} Racer;

typedef struct M017Model {
    /* 0x00 */ u8 pad0[0x2F];
    /* 0x2F */ u8 unk2F;
} M017Model;

typedef struct M017ModelInst {
    /* 0x000 */ M017Model *model;
    /* 0x004 */ u8 pad4[0xA - 0x4];
    /* 0x00A */ s8 unkA;
    /* 0x00B */ s8 unkB;
    /* 0x00C */ u8 padC[0x10 - 0xC];
    /* 0x010 */ void *unk10[2];
    /* 0x018 */ u8 pad18[0x70 - 0x18];
    /* 0x070 */ Vec3f *vertices;
} M017ModelInst;

#define MODEL_INST(obj, i) ((M017ModelInst *) (obj)->modelInstances[i])

/* A player's character record (func_80047C1C) */
typedef struct M017Char {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 unkA;
    /* 0x0C */ u8 padC[0x6F - 0xC];
    /* 0x6F */ u8 unk6F;
    /* 0x70 */ u8 weapon;
} M017Char;

/* A weapon's entry in gWeaponInfo */
typedef struct M017WeaponInfo {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ u8 pad24[0x30 - 0x24];
} M017WeaponInfo;

/* What obj+0x58 points at, as used here */
typedef struct M017Obj58 {
    /* 0x00 */ u8 pad0[0x75];
    /* 0x75 */ u8 unk75;
    /* 0x76 */ u8 pad76[0x80 - 0x76];
    /* 0x80 */ f32 unk80;
    /* 0x84 */ u8 *unk84;
} M017Obj58;
#define OBJ58(obj) (*(M017Obj58 **) ((u8 *) (obj) + 0x58))

/* The controller's button masks for the actions */
typedef struct M017Buttons {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ u32 unk4;
    /* 0x08 */ u32 unk8;
    /* 0x0C */ u32 unkC;
    /* 0x10 */ u32 unk10;
    /* 0x14 */ u32 unk14;
    /* 0x18 */ u32 unk18;
    /* 0x1C */ u32 unk1C;
    /* 0x20 */ u32 unk20;
} M017Buttons;

/* The level header (DKR's level_header) as used here */
typedef struct M017LevelHeader {
    /* 0x00 */ u8 pad0[0xE2];
    /* 0xE2 */ u8 unkE2;
} M017LevelHeader;

extern M017WeaponInfo gWeaponInfo[];
extern M017Buttons *D_800A18B0;
extern u32 D_800A197C[4]; /* the button for each weapon slot */
extern u8 D_800A4FC4;
extern u8 D_800A5070;
extern u8 D_800A5078;
extern s32 gPlayerButtonsHeld;
extern s32 gPlayerButtonsPressed;
extern s32 gPlayerStickX;
extern s32 gPlayerStickY;
extern s32 gPlayerInputLocked;
extern s16 *D_800F6DC0;

extern void func_80000510(s32 arg0);
extern void sound_stop(s32 handle);
extern void func_800013FC(s32 sound, s32 handle, u8 volume);
extern void func_800035E8(s32 handle, f32 x, f32 y, f32 z);
extern void func_80003614(s32 handle);
extern void obj_tex_animate(Object *obj, M017Model *model, M017ModelInst *inst, s32 updateRate);
extern s32 func_8000BC28(Object *obj, M017ModelInst *inst, M017Model *model, s32 weapon, s32 *arg4, f32 *x, f32 *y,
                         f32 *z);
extern u32 track_height_at(f32 x, f32 z, f32 *height, u32 mask);
extern void func_800221F0(Object *obj, M017ModelInst *inst, void *shading, void *arg3);
extern void chain_object_unlink_from_players(Object *obj);
extern void player_voice_play(Object *obj, s32 arg1);
extern void player_items_unlink(Racer *racer);
extern void player_charge_meter_update(Racer *racer, M017Char *character, s32 updateRate);
extern s32 func_80033F4C(s32 arg0);
extern s16 angle_lerp(s16 from, s16 to, f32 amount);
extern void func_80034018(Racer *racer);
extern void func_80034164(Object *obj, Racer *racer);
extern void player_loop_sounds_update(Object *obj, Racer *racer, s32 a, s32 b, s32 updateRate);
extern void player_platform_ride(Object *obj, Racer *racer, f32 updateRate);
extern void player_squash_precheck(Object *obj, Racer *racer, s32 updateRate);
extern void player_squash_update(Object *obj, Racer *racer, s32 updateRate);
extern void player_input_lock_set(s32 arg0);
extern void player_input_latch(s32 player);
extern void func_8003733C(s32 player, s32 a, s32 b, s32 c, s32 d);
extern s32 func_80037550(Object *obj, Racer *racer, f32 updateRate);
extern void func_80038324(Object *obj, Racer *racer, s32 weapon);
extern s32 weapon_select(Object *obj, Racer *racer, s32 weapon);
extern void weapon_select_prev(Object *obj, Racer *racer, s32 weapon);
extern void func_80039FCC(Object *obj, Racer *racer, s32 a, s32 b, f32 c, f32 d, f32 e, s32 updateRate);
extern void player_drop_held_object(Object *obj);
extern void func_8003D21C(M017ModelInst *inst, M017Model *model, Object *obj, f32 a, f32 b);
extern M017LevelHeader *func_8004470C(void);
extern u8 *get_settings(void);
extern M017Char *func_80047C1C(s32 player, s32 character);
extern void character_item_give(s32 player, s32 character, s32 arg2);
extern s32 character_has_item(s32 player, s32 character, s32 arg2);
extern void func_80049394(Vec3s *rotation, Vec3f *vec);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_80049C84(f32 x, f32 z);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_8006B090(Object *obj, s32 updateRate);
extern void multiplayer_score_kill(Object *obj, Object *obj2);
extern f32 func_80097140(f32 x); /* sqrtf */
extern void func_m107_00D08(void *arg0, f32 a, f32 b, s32 c);

/* The static functions, still assembly */
extern void rl_m017_01A30(Object *obj, Racer *racer, s32 updateRate);
extern void rl_m017_01F7C(Object *obj, Racer *racer, s32 updateRate);
extern void rl_m017_02564(M017Model *model, M017ModelInst *inst, Racer *racer, s32 updateRate);
extern void rl_m017_02A40(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m017_02A54(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m017_0313C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m017_03258(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m017_0430C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m017_04614(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m017_04B84(Object *obj, Racer *racer, s32 anim, f32 frame);
extern void rl_m017_04C50(Object *obj, Racer *racer, f32 speed, f32 updateRate);
extern void rl_m017_050BC(Object *obj, Racer *racer, f32 updateRate);
extern void rl_m017_056D4(Object *obj, Racer *racer, s32 updateRate);
extern void rl_m017_05930(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m017_05F08(Object *obj, Racer *racer, f32 speed, f32 angle, f32 updateRate);

/* .data: the particle emitters for each level kind */
extern u32 rl_m017_06450[];

/* The module's .bss */
static s32 D_m017_069B0;
static s32 D_m017_069B4;
static s32 D_m017_069B8;
static s32 D_m017_069BC;
static s32 D_m017_069C0;
static s16 *D_m017_069C4;

/* The control function: called every frame */
void func_m017_00000(Object *obj, s32 updateRate) {
    s32 deep;
    s32 result;
    f32 speed;
    M017WeaponInfo *info;
    ObjectInteraction *interact;
    Vec3f dir;
    s16 angle;
    Vec3s rot;
    u32 hit;
    f32 x;
    M017Model *model;
    M017ModelInst *inst;
    M017Char *character;
    u8 *flags;
    s32 cur;
    s32 oldWeapon;
    s32 i;
    u8 *weapons;
    Racer *racer;

    D_m017_069B8 = 0;
    D_m017_069BC = 0;
    racer = obj->unk68;
    obj->particleEmittersEnabled = 0;
    D_m017_069C4 = racer->cmds;
    character = func_80047C1C(racer->player, racer->character);
    if (racer->state != 3) {
        racer->unk56A = 0;
    }
    racer->unk14A = 0;
    racer->unk19E = 0;
    racer->unk11E = 0;
    racer->unk120 = 0;
    D_m017_069B0 = 0;
    D_m017_069B4 = 0;
    player_platform_ride(obj, racer, updateRate);
    player_squash_precheck(obj, racer, updateRate);
    D_m017_069C0 = racer->unk577;
    if (racer->unk56B != 0) {
        player_input_lock_set(1);
        racer->unk56B -= updateRate;
        if (racer->unk56B < 0) {
            racer->unk56B = 0;
        }
    }
    if (racer->unk577 != 0) {
        player_input_lock_set(1);
        racer->unk577 -= updateRate;
        if (racer->unk577 < 0) {
            racer->unk577 = 0;
        }
    }
    if (racer->unk1F9 != 0 || racer->unk1FA != 0 || racer->unk186 != 0 || racer->unk198 != 0 || racer->unk5EE != 0 ||
        racer->state == 10 || racer->unk5C0 != 0) {
        player_input_lock_set(1);
    }
    if (racer->unk19F != 0 || racer->unk10D != 0) {
        player_input_lock_set(1);
        if (obj->animationID != 0x16) {
            rl_m017_04B84(obj, racer, 0x16, 0);
        }
    }
    if (racer->unk1FA >= 2) {
        racer->unk1FA = 0;
        D_m017_069BC = 1;
    }
    if (racer->unk198 >= 2) {
        racer->unk198 = 0;
    }
    if (racer->unk10D == 4) {
        racer->unk10D = 0;
        racer->unk594 = 0.0f;
    }
    player_input_latch(racer->player);
    func_80034018(racer);
    if (obj->velocity.x < -20.0f) {
        obj->velocity.x = -20.0f;
    }
    if (obj->velocity.x > 20.0f) {
        obj->velocity.x = 20.0f;
    }
    if (obj->velocity.y < -20.0f) {
        obj->velocity.y = -20.0f;
    }
    if (obj->velocity.y > 15.0f) {
        obj->velocity.y = 15.0f;
    }
    if (obj->velocity.z < -20.0f) {
        obj->velocity.z = -20.0f;
    }
    if (obj->velocity.z > 20.0f) {
        obj->velocity.z = 20.0f;
    }
    if (racer->unk4 < -20.0f) {
        racer->unk4 = -20.0f;
    }
    if (racer->unk4 > 20.0f) {
        racer->unk4 = 20.0f;
    }
    if (racer->unk8 < -20.0f) {
        racer->unk8 = -20.0f;
    }
    if (racer->unk8 > 20.0f) {
        racer->unk8 = 20.0f;
    }
    func_80034164(obj, racer);
    rot.y_rotation = -racer->unk11C;
    rot.x_rotation = -obj->trans.rotation.x_rotation;
    rot.z_rotation = -obj->trans.rotation.z_rotation;
    dir.z = 0.0f;
    dir.x = 0.0f;
    dir.y = -1.0f;
    func_80049394(&rot, &dir);
    racer->unk54 = dir.x;
    racer->unk58 = dir.y;
    racer->unk50 = dir.z;
    speed = func_80097140(gPlayerStickX * gPlayerStickX + gPlayerStickY * gPlayerStickY);
    if (speed > 65.0f) {
        speed = 65.0f;
    }
    speed *= 0.0625f;
    if (speed != 0.0f) {
        angle = func_80049C84(gPlayerStickX, -gPlayerStickY) - *D_800F6DC0;
    } else {
        angle = racer->unk11C;
    }
    if ((racer->state == 0 || racer->state == 5 || racer->state == 11) && !(racer->pose->unk5 & racer->unk532) &&
        racer->unk10D == 0 && racer->unk19F == 0) {
        racer->unk185 += updateRate;
        if (racer->unk185 >= 16) {
            racer->unk57C = obj->trans.y_position;
            rl_m017_04B84(obj, racer, 0x10, 0);
            racer->unk56A = racer->state;
            racer->state = 3;
            racer->unk58E = 0;
        }
    } else if ((racer->state == 2 || racer->state == 1) && !(racer->pose->unk5 & racer->unk532)) {
        racer->unk185 += updateRate;
        if (racer->unk185 >= 16) {
            racer->unk57C = obj->trans.y_position;
            rl_m017_04B84(obj, racer, 0x10, 0);
            racer->unk56A = 0;
            racer->state = 3;
            racer->unk58E = 0;
        }
    } else {
        racer->unk185 = 0;
    }
    if (((racer->pose->unk5 & racer->unk532) && racer->state != 3) || racer->state == 4) {
        racer->unk57C = obj->trans.y_position;
    }
    info = &gWeaponInfo[character->weapon];
    if (racer->state != 5 && racer->state != 11 && racer->state != 10) {
        func_80039FCC(obj, racer, info->unkC, info->unk14, info->unk18, info->unk1C, info->unk20, updateRate);
    } else {
        racer->unk1F4 = 1;
    }
    if (racer->state != 10) {
        player_charge_meter_update(racer, character, updateRate);
    }
    deep = 0;
    hit = track_height_at(obj->trans.x_position, obj->trans.z_position, &racer->unk5C, 0x80002000);
    if ((hit & 0x80000000) && obj->trans.y_position < racer->unk5C) {
        racer->unk5EE = 1;
    }
    if ((hit & 0x2000) && obj->trans.y_position < racer->unk5C) {
        racer->unk2 = 1;
        racer->unk60 = racer->unk5C - obj->trans.y_position;
        if (racer->unk3 != 1 && racer->unk74 < 0.0f) {
            rl_m017_01A30(obj, racer, updateRate);
        }
        if ((racer->character == 6 && racer->unk60 > 12.5f) || (racer->character == 2 && racer->unk60 > 28.0f)) {
            deep = 1;
        }
    } else {
        racer->unk2 = 0;
        racer->unk60 = 0.0f;
    }
    if ((racer->state < 4 || racer->state == 11 || racer->state == 5) && deep != 0) {
        racer->state = 4;
        racer->unk12A = 0;
        racer->unk12C = 0;
        racer->unk128 = racer->unk11C;
        if (racer->character == 6) {
            rl_m017_04B84(obj, racer, 6, obj->unk28);
        } else {
            racer->unk53D = 0;
            racer->flags.unk31 = 0;
            racer->unk538 = 0;
            if (racer->unk4 < -0.85f || racer->unk4 > 0.85f || obj->velocity.y > 0.85f) {
                rl_m017_04B84(obj, racer, 0x14, obj->unk28);
            } else {
                rl_m017_04B84(obj, racer, 0x15, obj->unk28);
            }
        }
    }
    interact = obj->interactObj;
    if (interact->hitPoints < 0x100 && racer->state != 10) {
        if (D_800A5070 != 0) {
            interact->hitPoints = 0x2200;
        } else {
            if (racer->character == 6 && racer->state == 4) {
                rl_m017_04B84(obj, racer, 0x1C, 0);
            } else {
                rl_m017_04B84(obj, racer, 0x17, 0);
            }
            obj->interactObj->hitPoints = 0;
            if (D_800A4FC4 == 0) {
                func_80000510(0x38);
            } else {
                chain_object_unlink_from_players(obj);
            }
            racer->state = 10;
            gPlayerButtonsHeld = 0;
            gPlayerButtonsPressed = 0;
            racer->unk53D = 0;
            player_drop_held_object(obj);
            player_items_unlink(racer);
        }
    }
    if (racer->state != 0) {
        racer->unk13E = 0;
        if (racer->state != 1) {
            racer->unk576 = 0;
        }
    }
    switch (racer->state) {
        case 0:
            rl_m017_02A54(obj, racer, speed, angle, updateRate);
            break;
        case 1:
            rl_m017_0313C(obj, racer, speed, angle, updateRate);
            break;
        case 2:
            rl_m017_02A40(obj, racer, speed, angle, updateRate);
            break;
        case 3:
            rl_m017_03258(obj, racer, speed, angle, updateRate);
            break;
        case 5:
            rl_m017_0430C(obj, racer, speed, angle, updateRate);
            break;
        case 4:
            rl_m017_05930(obj, racer, speed, angle, updateRate);
            break;
        case 10:
            rl_m017_05F08(obj, racer, speed, angle, updateRate);
            break;
        case 11:
            rl_m017_04614(obj, racer, speed, angle, updateRate);
            break;
    }
    if (racer->unk1F6 != 0) {
        racer->unk1F6 -= updateRate;
        if (racer->unk1F6 < 0) {
            racer->unk1F6 = 0;
        }
    }
    if (D_800A5078 != 0) {
        character->unk6F = 15;
        character->unkA = 0x7FFF;
    }
    if ((character->unkA & 0x20) && character_has_item(racer->player, racer->character, 10) == 0) {
        character_item_give(racer->player, racer->character, 10);
    }
    racer->flags.weaponDir = 0;
    if (character->unk6F >= 2) {
        oldWeapon = character->weapon;
        cur = racer->unk585;
        if (racer->unk585 != character->weapon && gPlayerInputLocked == 0) {
            weapon_select(obj, racer, cur);
        } else if (D_800A18B0->unk4 & gPlayerButtonsPressed) {
            func_80038324(obj, racer, cur);
        } else if (D_800A18B0->unk8 & gPlayerButtonsPressed) {
            weapon_select_prev(obj, racer, cur);
        } else {
            i = 0;
            weapons = get_settings();
            for (; i < 4; i++) {
                if ((D_800A197C[i] & gPlayerButtonsPressed) && weapon_select(obj, racer, (weapons + 0x534)[i]) != 0) {
                    break;
                }
            }
        }
        if (oldWeapon != character->weapon) {
            racer->unk538 = 0;
            racer->unk53D = 0;
            racer->unk585 = character->weapon;
        }
    }
    rl_m017_04C50(obj, racer, speed, updateRate);
    rl_m017_050BC(obj, racer, updateRate);
    player_squash_update(obj, racer, updateRate);
    if ((racer->unk520 & 0x8000) && racer->state != 10 && racer->unk5C8 == 0) {
        func_80001248(0x214, (s32) &racer->unkD4);
        player_voice_play(obj, 3);
        interact = obj->interactObj;
        interact->hitPoints -= 0x900;
        interact = obj->interactObj;
        if (interact->hitPoints >= 0x100) {
            racer->state = 3;
            obj->velocity.y = 6.0f;
            racer->unk594 = 0.7f;
            racer->unk14B = 60;
            racer->unk57A = 0;
            racer->unk57C = obj->trans.y_position;
            racer->unk58E = 0;
            D_m017_069B0 |= 1;
            D_m017_069B4 |= 1;
            obj_anim_set(obj, 5, 0);
        } else {
            interact->hitPoints = 0;
            if (D_800A4FC4 != 0) {
                multiplayer_score_kill(obj, obj);
            }
        }
    } else if (racer->unk14B > 0) {
        racer->unk14B -= updateRate;
        obj->particleEmittersEnabled |= 0x80;
    }
    if (racer->state != 5 && racer->state != 11 && racer->state != 3 && racer->state != 8) {
        rl_m017_056D4(obj, racer, updateRate);
    }
    if (racer->unk5BC != NULL) {
        switch (racer->state) {
            case 2:
                func_m107_00D08(racer->unk5BC, 1.5f, 0.5f, 40);
                break;
            case 4:
                func_m107_00D08(racer->unk5BC, 1.5f, 0.5f, 120);
                break;
            case 1:
                func_m107_00D08(racer->unk5BC, 1.0f, 0.75f, 120);
                break;
            default:
                func_m107_00D08(racer->unk5BC, 1.0f, 1.0f, 120);
                break;
        }
    }
    if (racer->character == 6) {
        if (racer->state == 0) {
            racer->unk34A = angle_lerp(racer->unk34A, racer->unk120, 1.0f - func_80049BEC(0.96f, updateRate));
        } else {
            racer->unk34A = angle_lerp(racer->unk34A, 0, 1.0f - func_80049BEC(0.96f, updateRate));
        }
        if (racer->unk34A < 0) {
            *D_m017_069C4 = 0x5E;
            D_m017_069C4++;
            *D_m017_069C4 = -racer->unk34A;
            D_m017_069C4++;
        } else if (racer->unk34A > 0) {
            *D_m017_069C4 = 0x52;
            D_m017_069C4++;
            *D_m017_069C4 = -racer->unk34A;
            D_m017_069C4++;
        }
    }
    rl_m017_01F7C(obj, racer, updateRate);
    if (racer->character == 6) {
        *D_m017_069C4 = 0x3A;
        D_m017_069C4++;
        *D_m017_069C4 = -obj->trans.rotation.z_rotation;
        D_m017_069C4++;
    } else {
        *D_m017_069C4 = 0x5E;
        D_m017_069C4++;
        *D_m017_069C4 = -obj->trans.rotation.z_rotation;
        D_m017_069C4++;
    }
    if (racer->flags.unk6) {
        *D_m017_069C4 = 0x4030;
        D_m017_069C4++;
        *D_m017_069C4 = 0x20;
        D_m017_069C4++;
        *D_m017_069C4 = 0x4032;
        D_m017_069C4++;
        *D_m017_069C4 = 0x20;
        D_m017_069C4++;
        *D_m017_069C4 = 0x4034;
        D_m017_069C4++;
        *D_m017_069C4 = 0x20;
        D_m017_069C4++;
    }
    *D_m017_069C4 = 0x1000;
    D_m017_069C4++;
    if (racer->unk576 != 0 ||
        (racer->unk188 != 0 && (racer->unk532 & racer->pose->unk5) &&
         (obj->velocity.x < -1.0f || obj->velocity.x > 1.0f || obj->velocity.z < -1.0f || obj->velocity.z > 1.0f))) {
        obj->particleEmittersEnabled |= rl_m017_06450[func_8004470C()->unkE2];
        if (racer->unkC8 == 0) {
            func_80001248(0x1C, (s32) &racer->unkC8);
        }
    } else {
        obj->particleEmittersEnabled &= ~6;
        if (racer->unkC8 != 0) {
            sound_stop(racer->unkC8);
        }
    }
    if (racer->state == 10 || racer->unk10D != 0 || racer->unk19F != 0) {
        obj->state[0x9F - 0x88] = 3;
    } else {
        obj->state[0x9F - 0x88] = 0;
    }
    inst = MODEL_INST(obj, obj->modelIndex);
    if (inst != NULL && inst->unkA != 0) {
        model = inst->model;
        x = racer->unk48;
        func_8003D21C(inst, model, obj, x, 0.0f);
        racer->flags.useOverride = func_8000BC28(obj, inst, model, character->weapon, &racer->unk78,
                                                 &racer->overridePos.x, &racer->overridePos.y, &racer->overridePos.z);
        func_800221F0(obj, inst, obj->shading, inst->unk10[inst->unkB]);
        if (racer->character == 2) {
            obj_tex_animate(obj, model, inst, inst->unkA);
        } else {
            rl_m017_02564(model, inst, racer, inst->unkA);
        }
        inst->unkA = 0;
        if (model->unk2F >= 3) {
            x = inst->vertices[2].y + 4.0f - obj->trans.y_position;
            if (x >= 64.0f) {
                x = 63.9f;
            }
            ((s8 *) obj->shadow)[0xE] = x * 4.0f;
        }
    }
    result = func_80037550(obj, racer, updateRate);
    if (func_80033F4C(result) != 0) {
        if (racer->character == 6 && racer->state == 4) {
            if (obj->animationID == 0x1B) {
                obj_anim_set(obj, 0x1B, 0);
            } else {
                rl_m017_04B84(obj, racer, 0x1B, 0);
            }
        } else if (obj->animationID == 0xE) {
            obj_anim_set(obj, 0xE, 0);
        } else {
            rl_m017_04B84(obj, racer, 0xE, 0);
        }
        racer->unk577 = 40;
    }
    player_loop_sounds_update(obj, racer, 0xF000, 0xF00, updateRate);
    OBJ58(obj)->unk75 = 0;
    flags = OBJ58(obj)->unk84;
    if (flags != NULL) {
        *flags &= ~2;
    }
    if (racer->unk2 == 1 && racer->unk60 < 50.0f) {
        OBJ58(obj)->unk80 = racer->unk5C;
        OBJ58(obj)->unk75 = 1;
        if (flags != NULL &&
            (racer->unk4 < -1.0f || racer->unk4 > 1.0f || racer->unk10 < -1.0f || racer->unk10 > 1.0f)) {
            *flags |= 2;
        }
    }
    func_8006B090(obj, updateRate);
    func_8005F5AC(obj, updateRate);
    if (obj->animationID == 9 || obj->animationID == 0xC) {
        if (racer->unkBC == 0) {
            func_80003410(0x135, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 1,
                          &racer->unkBC);
            racer->unkB8 = 0x135;
        } else if (racer->unkB8 == 0x135) {
            func_800035E8(racer->unkBC, obj->trans.x_position, obj->trans.y_position, obj->trans.z_position);
        }
    } else if (racer->unkBC != 0 && racer->unkB8 == 0x135) {
        func_80003614(racer->unkBC);
    }
    if (racer->character == 6) {
        if ((racer->unk4 < -0.1f || racer->unk4 > 0.1f || racer->unk8 < -0.5f || racer->unk8 > 0.5f) &&
            racer->state != 3 &&
            (gPlayerStickX != 0 || gPlayerStickY != 0 ||
             ((D_800A18B0->unk20 | D_800A18B0->unk14 | D_800A18B0->unk18 | D_800A18B0->unk1C) & gPlayerButtonsHeld))) {
            if (racer->unk5D8 != 0 && racer->unk5EC == 1) {
                sound_stop(racer->unk5D8);
            }
            if (racer->unk5D8 == 0) {
                racer->unk5EC = 0;
                racer->unk5ED = 0x7F;
                func_80001248(0x1F8, (s32) &racer->unk5D8);
            }
            racer->unk5ED -= updateRate;
            if (racer->unk5ED < 0x30) {
                racer->unk5ED = 0x30;
            }
            func_800013FC(0x1F8, racer->unk5D8, racer->unk5ED);
        } else if (racer->unk5D8 != 0 && racer->unk5EC == 0) {
            sound_stop(racer->unk5D8);
            racer->unk5EC = 1;
            func_80001248(0x1F9, (s32) &racer->unk5D8);
        }
    }
    if (racer->unk14A != 0) {
        if (racer->unkCC == 0) {
            func_80001248(0x9C, (s32) &racer->unkCC);
        }
    } else if (racer->unkCC != 0) {
        sound_stop(racer->unkCC);
    }
    func_8003733C(racer->player, D_m017_069B0, D_m017_069B4, result, racer->unk536 / updateRate);
    racer->unk536 = 0;
}

/* Whether the character may fire now */
s32 func_m017_01990(Object *obj, Racer *racer) {
    s32 state;

    if (racer->unk576 == 0 && racer->unk13E == 0) {
        state = racer->state & ~0x20;
        if (state == 0 || state == 5 || state == 0xB || state == 0xA || (state == 3 && racer->unk14A != 0)) {
            return TRUE;
        }
        if (racer->character == 6 && state == 4) {
            return TRUE;
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_01A30);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_01C44);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_01F7C);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_02564);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_02848);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_02A40);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_02A54);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_0313C);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_03258);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_04178);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_04270);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_0430C);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_04614);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_04A44);

INCLUDE_ASM("asm/nonmatchings/modules/m017", func_m017_04B84);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_04C50);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_050BC);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_056D4);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_05930);

INCLUDE_ASM("asm/nonmatchings/modules/m017/ro", func_m017_05F08);

INCLUDE_ASM("asm/nonmatchings/modules/m017", data);

/* (.rodata: C, and the assembly functions' own from ro/) */
