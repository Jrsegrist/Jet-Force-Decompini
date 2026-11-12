/*
 * Runlink module 16: a player character's behaviour (the control function
 * called every frame, the firing check, and the movement states it runs).
 * A sibling of module 15.
 */
#include "module.h"

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

/* The flag word, also read and written as halves and bytes */
typedef union RacerFlagWord {
    RacerFlags bits;
    u16 half[2];
    u8 byte[4];
} RacerFlagWord;

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
    /* 0x014 */ f32 unk14;
    /* 0x018 */ u8 pad18[0x3C - 0x18];
    /* 0x03C */ f32 unk3C;
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
    /* 0x074 */ f32 unk74;
    /* 0x078 */ s32 unk78;
    /* 0x07C */ u8 pad7C[0xC8 - 0x7C];
    /* 0x0C8 */ s32 unkC8; /* sound handle */
    /* 0x0CC */ s32 unkCC; /* sound handle */
    /* 0x0D0 */ u8 padD0[0x10D - 0xD0];
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
    /* 0x14B */ u8 pad14B[0x16C - 0x14B];
    /* 0x16C */ Object *unk16C;
    /* 0x170 */ u8 pad170[0x185 - 0x170];
    /* 0x185 */ s8 unk185;
    /* 0x186 */ s8 unk186;
    /* 0x187 */ u8 pad187[0x188 - 0x187];
    /* 0x188 */ u8 unk188;
    /* 0x189 */ s8 unk189;
    /* 0x18A */ u8 pad18A[0x18D - 0x18A];
    /* 0x18D */ u8 unk18D;
    /* 0x18E */ u8 pad18E[0x194 - 0x18E];
    /* 0x194 */ u8 unk194;
    /* 0x195 */ u8 pad195[0x198 - 0x195];
    /* 0x198 */ u8 unk198;
    /* 0x199 */ u8 pad199[0x19E - 0x199];
    /* 0x19E */ u8 unk19E;
    /* 0x19F */ u8 unk19F;
    /* 0x1A0 */ u8 pad1A0[0x1F4 - 0x1A0];
    /* 0x1F4 */ s8 unk1F4;
    /* 0x1F5 */ s8 unk1F5;
    /* 0x1F6 */ s8 unk1F6;
    /* 0x1F7 */ s8 unk1F7;
    /* 0x1F8 */ s8 unk1F8;
    /* 0x1F9 */ u8 unk1F9;
    /* 0x1FA */ u8 unk1FA;
    /* 0x1FB */ u8 pad1FB[0x240 - 0x1FB];
    /* 0x240 */ s16 cmds[0x80]; /* the command list func_m016_00000 builds */
    /* 0x340 */ u8 pad340[0x360 - 0x340];
    /* 0x360 */ RacerPose *pose;
    /* 0x364 */ u8 pad364[0x510 - 0x364];
    /* 0x510 */ Object *unk510;
    /* 0x514 */ u8 pad514[0x532 - 0x514];
    /* 0x532 */ u8 unk532;
    /* 0x533 */ u8 unk533;
    /* 0x534 */ u8 pad534[0x536 - 0x534];
    /* 0x536 */ s16 unk536;
    /* 0x538 */ u8 unk538;
    /* 0x539 */ u8 pad539[0x53C - 0x539];
    /* 0x53C */ u8 unk53C;
    /* 0x53D */ s8 unk53D;
    /* 0x53E */ u8 pad53E[0x540 - 0x53E];
    /* 0x540 */ RacerFlagWord flags;
    /* 0x544 */ Vec3f overridePos;
    /* 0x550 */ u8 pad550[0x568 - 0x550];
    /* 0x568 */ u8 state;
    /* 0x569 */ u8 unk569;
    /* 0x56A */ u8 unk56A;
    /* 0x56B */ s8 unk56B;
    /* 0x56C */ u8 pad56C[0x576 - 0x56C];
    /* 0x576 */ s8 unk576;
    /* 0x577 */ s8 unk577;
    /* 0x578 */ u8 pad578[0x57C - 0x578];
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
    /* 0x5C4 */ u8 pad5C4[0x5EE - 0x5C4];
    /* 0x5EE */ u8 unk5EE;
    /* 0x5EF */ u8 pad5EF[0x5F0 - 0x5EF];
} Racer;

typedef struct M016Model {
    /* 0x000 */ u8 pad0[0x11 - 0x0];
    /* 0x011 */ u8 unk11;
} M016Model;

typedef struct M016ModelInst {
    /* 0x000 */ M016Model *model;
    /* 0x004 */ u8 pad4[0xA - 0x4];
    /* 0x00A */ s8 unkA;
    /* 0x00B */ s8 unkB;
    /* 0x00C */ u8 padC[0x10 - 0xC];
    /* 0x010 */ void *unk10[2];
    /* 0x018 */ u8 pad18[0x70 - 0x18];
    /* 0x070 */ Vec3f *vertices;
} M016ModelInst;

#define MODEL_INST(obj, i) ((M016ModelInst *) (obj)->modelInstances[i])

/* A body part entry */
typedef struct M016Part {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ u32 flags;
} M016Part;

/* A list of body parts with its settings */
typedef struct M016PartSet {
    /* 0x00 */ M016Part **parts;
    /* 0x04 */ u32 unk4;
    /* 0x08 */ s32 count;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
} M016PartSet;

/* The animation table: per animation, its variants and a pose index */
typedef struct M016Anim {
    u8 normal;
    u8 alt1;
    u8 alt2;
    u8 pose;
    u8 alt1Pose;
} M016Anim;

/* The table passed to func_80035E14 / func_800364D4 */
typedef struct M016Steps {
    /* 0x00 */ f32 steps[5];
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
} M016Steps;

/* A damage/score event with its debug name */
typedef struct M016Event {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ char name[26];
} M016Event;

typedef struct M016Rate {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
} M016Rate;

/* A player's character record (func_80047C1C) */
typedef struct M016Char {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 unkA;
    /* 0x0C */ u8 padC[0x6F - 0xC];
    /* 0x6F */ u8 unk6F;
    /* 0x70 */ u8 weapon;
} M016Char;

/* The level header (DKR's level_header) as used here */
typedef struct M016LevelHeader {
    /* 0x00 */ u8 pad0[0xE2];
    /* 0xE2 */ u8 unkE2;
} M016LevelHeader;

/* A weapon's entry in gWeaponInfo */
typedef struct M016WeaponInfo {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ u8 pad24[0x30 - 0x24];
} M016WeaponInfo;

/* What obj+0x58 points at, as used here */
typedef struct M016Obj58 {
    /* 0x00 */ u8 pad0[0x75];
    /* 0x75 */ u8 unk75;
    /* 0x76 */ u8 pad76[0x80 - 0x76];
    /* 0x80 */ f32 unk80;
    /* 0x84 */ u8 *unk84;
} M016Obj58;
#define OBJ58(obj) (*(M016Obj58 **) ((u8 *) (obj) + 0x58))

/* The controller's button masks for the actions */
typedef struct M016Buttons {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ u32 unk4;
    /* 0x08 */ u32 unk8;
} M016Buttons;

extern u16 D_800FD7A8[];
extern M016WeaponInfo gWeaponInfo[];
extern u32 D_800A197C[4]; /* the button for each weapon slot */
extern M016Buttons *D_800A18B0;
extern u8 D_800A4FC4;
extern u8 D_800A5070;
extern u8 D_800A5078;
extern u8 D_800A5084;
extern s32 gPlayerButtonsHeld;
extern s32 gPlayerButtonsPressed;
extern s32 gPlayerStickX;
extern s32 gPlayerStickY;
extern s32 gPlayerInputLocked;
extern s16 *D_800F6DC0;
extern M016Char *func_80047C1C(s32 player, s32 character);
extern void player_platform_ride(Object *obj, Racer *racer, f32 updateRate);
extern void racer_angle_sync(Object *obj, Racer *racer, f32 updateRate);
extern void player_squash_precheck(Object *obj, Racer *racer, s32 updateRate);
extern void player_input_lock_set(s32 arg0);
extern void player_input_latch(s32 player);
extern void func_80034018(Racer *racer);
extern void func_80034164(Object *obj, Racer *racer);
extern void func_80049394(Vec3s *rotation, Vec3f *vec);
extern f32 func_80097140(f32 x); /* sqrtf */
extern s32 func_80049C84(f32 x, f32 z);
extern void func_80039FCC(Object *obj, Racer *racer, s32 a, s32 b, f32 c, f32 d, f32 e, s32 updateRate);
extern void player_charge_meter_update(Racer *racer, s32 arg1, s32 updateRate);
extern u32 track_height_at(f32 x, f32 z, f32 *height, u32 mask);
extern void func_80000510(s32 arg0);
extern void chain_object_unlink_from_players(Object *obj);
extern void player_drop_held_object(Object *obj);
extern void player_items_unlink(Racer *racer);
extern void character_item_give(s32 player, s32 character, s32 arg2);
extern s32 character_has_item(s32 player, s32 character, s32 arg2);
extern s32 weapon_select(Object *obj, Racer *racer, s32 weapon);
extern void func_80038324(Object *obj, Racer *racer, s32 weapon);
extern void weapon_select_prev(Object *obj, Racer *racer, s32 weapon);
extern u8 *get_settings(void);
extern void player_squash_update(Object *obj, Racer *racer, s32 updateRate);
extern void func_80035E14(Object *obj, Racer *racer, f32 speed, f32 angle, f32 updateRate, void *table);
extern void func_800364D4(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate, void *table);
extern void func_m107_00D08(void *arg0, f32 a, f32 b, s32 c);
extern M016LevelHeader *func_8004470C(void);
extern void func_80001248(s32 sound, s32 handle);
extern void sound_stop(s32 handle);
extern void func_8003D21C(M016ModelInst *inst, M016Model *model, Object *obj, f32 a, f32 b);
extern s32 func_8000BC28(Object *obj, M016ModelInst *inst, M016Model *model, s32 weapon, s32 *arg4, f32 *x, f32 *y,
                         f32 *z);
extern void func_800221F0(Object *obj, M016ModelInst *inst, void *shading, void *arg3);
extern void obj_tex_animate(Object *obj, M016Model *model, M016ModelInst *inst, s32 updateRate);
extern s32 func_80037550(Object *obj, Racer *racer, f32 updateRate);
extern s32 func_80033F4C(s32 arg0);
extern void func_80038630(Object *obj, Racer *racer);
extern void player_loop_sounds_update(Object *obj, Racer *racer, s32 a, s32 b, s32 updateRate);
extern void func_8006B090(Object *obj, s32 updateRate);
extern void func_8005F5AC(Object *obj, s32 updateRate);
extern void func_8003733C(s32 player, s32 a, s32 b, s32 c, s32 d);

/* The module's functions still in assembly (statics, called by their rl_ names) */
extern void rl_m016_01B48(Object *obj, Racer *racer, s32 updateRate);
extern void rl_m016_02220(Object *obj, Racer *racer, s32 updateRate);
extern void rl_m016_02708(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_02EB4(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_0321C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_03464(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_03F30(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_04440(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_04840(Object *obj, Racer *racer, s32 updateRate);
extern void rl_m016_04F78(Object *obj, Racer *racer, s32 anim, f32 frame);
extern void rl_m016_05120(Object *obj, Racer *racer, f32 speed, f32 updateRate);
extern void rl_m016_05BB8(Object *obj, Racer *racer, f32 updateRate);
extern void rl_m016_06290(Object *obj, Racer *racer, s32 updateRate);
extern void rl_m016_0650C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_06AEC(Object *obj, Racer *racer, f32 speed, f32 angle, f32 updateRate);
extern void rl_m016_06E24(Object *obj, Racer *racer, f32 updateRate);
extern void rl_m016_070A4(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
extern void rl_m016_07344(Object *obj, Racer *racer, f32 updateRate);
extern void rl_m016_073A8(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);

/* Body part entries (0x14 each), grouped by the lists below */
static M016Part D_m016_075F0[18] = {
    { 0.0f, 15.0f, 0.0f, 15.0f, 0x2010000 },
    { 0.0f, 30.0f, 0.0f, 15.0f, 0x8010000 },
    { 0.0f, 45.0f, 0.0f, 15.0f, 0x8010000 },
    { 0.0f, 61.0f, 0.0f, 3.0f, 0x4010000 },
    { 0.0f, 34.0f, 0.0f, 8.0f, 0x8010000 },
    { 0.0f, 34.0f, -32.0f, 8.0f, 0x8010000 },
    { 35.0f, 30.0f, 0.0f, 8.0f, 0x8010000 },
    { 0.0f, 15.0f, 15.0f, 15.0f, 0x2010000 },
    { 0.0f, 15.0f, -15.0f, 15.0f, 0x2010000 },
    { 0.0f, 15.0f, 0.0f, 3.0f, 0x4010000 },
    { 0.0f, 15.0f, 0.0f, 8.0f, 0x8010000 },
    { 0.0f, 26.0f, 0.0f, 8.0f, 0x8010000 },
    { 0.0f, 26.0f, -32.0f, 8.0f, 0x8010000 },
    { 0.0f, 15.0f, 11.0f, 15.0f, 0x2010000 },
    { 0.0f, 30.0f, 11.0f, 15.0f, 0x8010000 },
    { 0.0f, 45.0f, 11.0f, 15.0f, 0x8010000 },
    { 0.0f, 61.0f, -7.0f, 3.0f, 0x4010000 },
    { 0.0f, 34.0f, 11.0f, 8.0f, 0x8010000 },
};

static M016Part *D_m016_07758[35] = {
    &D_m016_075F0[0], &D_m016_075F0[1], &D_m016_075F0[2], &D_m016_075F0[3], &D_m016_075F0[4],
    &D_m016_075F0[0], &D_m016_075F0[7], &D_m016_075F0[8], &D_m016_075F0[9], &D_m016_075F0[10],
    &D_m016_075F0[0], &D_m016_075F0[1], &D_m016_075F0[1], &D_m016_075F0[9], &D_m016_075F0[11],
    &D_m016_075F0[0], &D_m016_075F0[1], &D_m016_075F0[2], &D_m016_075F0[3], &D_m016_075F0[5],
    &D_m016_075F0[0], &D_m016_075F0[1], &D_m016_075F0[1], &D_m016_075F0[9], &D_m016_075F0[12],
    &D_m016_075F0[13], &D_m016_075F0[14], &D_m016_075F0[15], &D_m016_075F0[16], &D_m016_075F0[17],
    &D_m016_075F0[0], &D_m016_075F0[1], &D_m016_075F0[2], &D_m016_075F0[3], &D_m016_075F0[6],
};

static M016PartSet D_m016_077E4[7] = {
    { &D_m016_07758[0], 0x18010816, 5, 60.0f, 15.0f },
    { &D_m016_07758[5], 0x18070810, 5, 30.0f, 60.0f },
    { &D_m016_07758[10], 0x1C010816, 5, 45.0f, 60.0f },
    { &D_m016_07758[15], 0x8010816, 5, 60.0f, 45.0f },
    { &D_m016_07758[20], 0xC010816, 5, 45.0f, 45.0f },
    { &D_m016_07758[25], 0x10010816, 1, 60.0f, 26.0f },
    { &D_m016_07758[30], 0x8010816, 5, 60.0f, 45.0f },
};

static M016Anim D_m016_07870[52] = {
    { 0, 28, 36, 0, 3 },   { 1, 29, 35, 0, 3 },   { 2, 30, 34, 0, 3 },   { 3, 26, 3, 0, 3 },
    { 44, 44, 4, 1, 1 },   { 5, 5, 5, 0, 0 },     { 6, 6, 6, 0, 0 },     { 7, 7, 7, 0, 0 },
    { 8, 8, 8, 0, 3 },     { 9, 31, 47, 0, 3 },   { 10, 32, 48, 0, 3 },  { 11, 11, 11, 2, 4 },
    { 12, 12, 12, 1, 1 },  { 13, 13, 13, 2, 4 },  { 14, 33, 14, 2, 4 },  { 15, 15, 15, 0, 3 },
    { 16, 27, 45, 0, 3 },  { 17, 27, 20, 0, 3 },  { 18, 27, 18, 0, 3 },  { 16, 27, 19, 0, 3 },
    { 17, 27, 20, 0, 3 },  { 21, 21, 21, 6, 6 },  { 22, 22, 22, 1, 4 },  { 23, 23, 23, 1, 1 },
    { 24, 24, 24, 0, 0 },  { 25, 25, 25, 0, 0 },  { 3, 26, 3, 0, 3 },    { 16, 27, 16, 0, 3 },
    { 0, 28, 36, 0, 3 },   { 1, 29, 35, 0, 3 },   { 2, 30, 34, 0, 3 },   { 9, 31, 47, 0, 3 },
    { 10, 32, 48, 0, 3 },  { 14, 33, 14, 2, 4 },  { 2, 30, 34, 0, 3 },   { 1, 29, 35, 0, 3 },
    { 0, 28, 36, 0, 3 },   { 37, 37, 37, 0, 3 },  { 38, 38, 38, 0, 3 },  { 39, 39, 39, 0, 3 },
    { 40, 40, 40, 5, 5 },  { 41, 41, 41, 5, 5 },  { 42, 42, 42, 5, 5 },  { 43, 43, 43, 0, 0 },
    { 44, 44, 4, 1, 1 },   { 16, 27, 45, 0, 3 },  { 46, 46, 46, 3, 3 },  { 9, 31, 47, 0, 3 },
    { 10, 32, 48, 0, 3 },  { 49, 49, 49, 2, 4 },  { 50, 50, 50, 1, 1 },  { 51, 51, 51, 0, 3 },
};

/* animation speeds */
static f32 D_m016_07974[52] = {
    0.015f, 0.009f, 0.0075f, 0.014f, 0.0175f, 0.01f,
    0.02f, 0.0175f, 0.0334f, 0.0115f, 0.0115f, 0.0175f,
    0.028f, 0.02f, 0.005f, 0.025f, 0.005f, 0.005f,
    0.005f, 0.005f, 0.005f, 0.017f, 0.005f, 0.01f,
    0.01f, 0.0334f, 0.014f, 0.005f, 0.015f, 0.01f,
    0.0075f, 0.0115f, 0.0115f, 0.06f, 0.0075f, 0.01f,
    0.015f, 0.006f, 0.004f, 0.01f, 0.014f, 0.025f,
    0.02f, 0.003f, 0.0175f, 0.005f, 0.017f, 0.0115f,
    0.0115f, 0.0175f, 0.028f, 0.01f,
};

static M016Steps D_m016_07A44 = { { 1.75f, 2.5f, 4.0f, 20.0f, 58.0f }, 5, 5 };

static u32 D_m016_07A60[3] = { 2, 4, 2 };
static s32 D_m016_07A6C[3] = { 8, 0x10, 0x40 };

/* Damage/score events with their debug names */
static M016Event D_m016_07A78[13] = {
    { 500, -250, -250, "Player hit (soft)" },
    { 1000, -500, -500, "Player hit (normal)" },
    { 2000, -1000, -1000, "Player hit (hard)" },
    { 400, 0, -32718, "Player Fired" },
    { 800, 0, -32668, "Player Threw Grenade" },
    { 0, 250, -32718, "Enemy hit (soft)" },
    { 0, 500, -32668, "Enemy hit (normal)" },
    { 0, 1000, -32568, "Enemy hit (hard)" },
    { 0, 1500, -32368, "Killed Enemy" },
    { -4000, 0, 2000, "Player Saved a Bear" },
    { 2000, -1000, -1000, "Player hit a Bear" },
    { 4000, -2000, -2000, "Player Killed a Bear" },
    { 1000, 0, -500, "Enemy killed a Bear" },
};

static M016Rate D_m016_07C18[19] = {
    { 0.004f, 0.8f, 0, 0 },  { 0.007f, 0.85f, 0, 0 }, { 0.01f, 0.85f, 0, 0 },  { 0.004f, 0.5f, 0, 0 },
    { 0.01f, 0.5f, 0, 0 },   { 0.01f, 0.5f, 0, 0 },   { 0.01f, 0.5f, 0, 0 },   { 0.01f, 0.5f, 0, 0 },
    { 0.01f, 0.5f, 0, 0 },   { 0.01f, 0.5f, 0, 0 },   { 0.004f, 0.5f, 0, 0 },  { 0.004f, 0.8f, 0, 0 },
    { 0.004f, 0.8f, 0, 0 },  { 0.004f, 0.84f, 0, 0 }, { 0.004f, 0.8f, 0, 0 },  { 0.004f, 0.8f, 0, 0 },
    { 0.004f, 0.8f, 0, 0 },  { 0.004f, 0.8f, 0, 0 },  { 0.004f, 0.8f, 0, 0 },
};

static s32 D_m016_07D48 = 1;

/* The module's .bss */
static s32 D_m016_080D0;
static s32 D_m016_080D4;
static s16 *D_m016_080D8; /* the next free word of racer->cmds */
static s16 D_m016_080DC;
static s16 D_m016_080DE;
static s32 sAnimChanges; /* 0x080E0 */
static f32 D_m016_080E4;

#ifdef NON_MATCHING
/* 10 instructions off: the state switch before the joint loop keeps its value in a3, the ROM in a1 (so the float arguments' lui go into the other delay slots) */
/* The control function: called every frame */
void func_m016_00000(Object *obj, s32 updateRate) {
    s32 deep;
    Racer *racer;
    f32 speed;
    M016WeaponInfo *info;
    ObjectInteraction *interact;
    Vec3f dir;
    s16 angle;
    Vec3s rot;
    M016Model *model;
    f32 x;
    u8 *flags;
    M016ModelInst *inst;
    M016Char *character;
    s32 result;
    s32 cur;
    s32 oldWeapon;
    s32 i;
    u8 *weapons;

    sAnimChanges = 0;
    racer = obj->unk68;
    obj->particleEmittersEnabled = 0;
    D_m016_080D8 = racer->cmds;
    D_m016_080DE = 0;
    character = func_80047C1C(racer->player, racer->character);
    if (racer->state != 3) {
        racer->unk56A = 0;
    }
    racer->unk19E = 0;
    racer->unk11E = 0;
    racer->unk120 = 0;
    D_m016_080D0 = 0;
    D_m016_080D4 = 0;
    if (racer->state == 6 || racer->state == 8 || racer->state == 7 || racer->state == 12) {
        racer->unk510 = racer->unk16C;
    } else {
        racer->unk16C = NULL;
    }
    player_platform_ride(obj, racer, updateRate);
    racer_angle_sync(obj, racer, updateRate);
    player_squash_precheck(obj, racer, updateRate);
    D_m016_080E4 = 0.0625f;
    if (D_800A4FC4 != 0) {
        if (racer->unk189 == 2 || racer->unk189 == 7) {
            D_m016_080E4 = 0.075f;
        } else if (racer->unk189 >= 4) {
            D_m016_080E4 = 0.05f;
        }
    }
    if (*(u16 *) ((u32) D_800FD7A8 + 4) != 0xDB4F) {
        D_m016_080E4 = 0.016666668f;
    }
    D_m016_080DC = racer->unk577;
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
    if (racer->unk10D != 0 || racer->unk19F != 0) {
        player_input_lock_set(1);
        if (obj->animationID != 0x27) {
            rl_m016_04F78(obj, racer, 0x27, 0);
        }
    }
    if (racer->unk1FA >= 2) {
        racer->unk1FA = 0;
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
    if (obj->velocity.x < -50.0f) {
        obj->velocity.x = -50.0f;
    }
    if (obj->velocity.x > 50.0f) {
        obj->velocity.x = 50.0f;
    }
    if (obj->velocity.y < -50.0f) {
        obj->velocity.y = -50.0f;
    }
    if (obj->velocity.y > 50.0f) {
        obj->velocity.y = 50.0f;
    }
    if (obj->velocity.z < -50.0f) {
        obj->velocity.z = -50.0f;
    }
    if (obj->velocity.z > 50.0f) {
        obj->velocity.z = 50.0f;
    }
    if (racer->unk4 < -50.0f) {
        racer->unk4 = -50.0f;
    }
    if (racer->unk4 > 50.0f) {
        racer->unk4 = 50.0f;
    }
    if (racer->unk8 < -50.0f) {
        racer->unk8 = -50.0f;
    }
    if (racer->unk8 > 50.0f) {
        racer->unk8 = 50.0f;
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
    speed *= D_m016_080E4;
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
            rl_m016_04F78(obj, racer, 0x18, 0);
            racer->unk56A = racer->state;
            racer->state = 3;
            racer->unk58E = 0;
        }
    } else if ((racer->state == 2 || racer->state == 1) && !(racer->pose->unk5 & racer->unk532) &&
               !(racer->pose->unk6 & racer->unk532) && !(racer->pose->unk7 & racer->unk532)) {
        racer->unk185 += updateRate;
        if (racer->unk185 >= 16) {
            racer->unk57C = obj->trans.y_position;
            rl_m016_04F78(obj, racer, 0x18, 0);
            racer->unk56A = racer->state;
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
        if (racer->unk1F4 != 0) {
            racer->unk1F8 -= updateRate;
            if (racer->unk1F8 < 0) {
                racer->unk1F8 = 0;
            }
        }
        racer->unk1F4 = 1;
    }
    if (racer->state != 10) {
        player_charge_meter_update(racer, 0, updateRate);
    }
    deep = 0;
    {
    u32 hit;

    hit = track_height_at(obj->trans.x_position, obj->trans.z_position, &racer->unk5C, 0x8000A000);
    if ((hit & 0x80000000) && obj->trans.y_position < racer->unk5C) {
        racer->unk5EE = 1;
    }
    if ((hit & 0x2000) && obj->trans.y_position < racer->unk5C) {
        racer->unk2 = 1;
        racer->unk18D = 0;
        racer->unk60 = racer->unk5C - obj->trans.y_position;
        if (racer->unk3 != 1 && racer->unk74 < 0.0f) {
            rl_m016_01B48(obj, racer, updateRate);
        }
        if (racer->unk60 > 48.5f) {
            deep = 1;
        }
    } else if ((hit & 0x8000) && obj->trans.y_position < racer->unk5C &&
               racer->unk5C < obj->trans.y_position + 50.0f) {
        racer->unk2 = 0;
        racer->unk18D += updateRate;
        if (racer->unk18D > 128) {
            racer->unk18D = 128;
        }
        racer->unk60 = racer->unk5C - obj->trans.y_position;
    } else {
        racer->unk2 = 0;
        racer->unk60 = 0.0f;
        if (racer->unk18D > updateRate) {
            racer->unk18D -= updateRate;
        } else {
            racer->unk18D = 0;
        }
    }
    }
    if ((racer->state < 4 || racer->state == 11 || racer->state == 5) && deep != 0) {
        racer->state = 4;
        racer->unk12A = 0;
        racer->unk12C = 0;
        racer->unk128 = racer->unk11C;
        racer->flags.bits.unk31 = 0;
        racer->unk53D = 0;
        racer->unk538 = 0;
        if (racer->unk4 < -0.85f || 0.85f < racer->unk4 || 0.85f < obj->velocity.y) {
            rl_m016_04F78(obj, racer, 0x25, 0);
        } else {
            rl_m016_04F78(obj, racer, 0x26, 0);
        }
    }
    interact = obj->interactObj;
    if (interact->hitPoints < 0x100 && racer->state != 10) {
        if (D_800A5070 != 0) {
            interact->hitPoints = 0x2200;
        } else {
            interact->hitPoints = 0;
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
            racer->unk194 = 0;
            func_80034018(racer);
            rl_m016_04F78(obj, racer, 0x17, 0);
        }
    }
    if (racer->state != 0 && racer->state != 2) {
        racer->unk13E = 0;
        if (racer->state != 1) {
            racer->unk576 = 0;
        }
    }
    switch (racer->state) {
        case 0:
            rl_m016_02708(obj, racer, speed, angle, updateRate);
            break;
        case 1:
            rl_m016_02EB4(obj, racer, speed, angle, updateRate);
            break;
        case 2:
            rl_m016_0321C(obj, racer, speed, angle, updateRate);
            break;
        case 3:
            rl_m016_03464(obj, racer, speed, angle, updateRate);
            break;
        case 5:
            rl_m016_03F30(obj, racer, speed, angle, updateRate);
            break;
        case 6:
            rl_m016_06E24(obj, racer, updateRate);
            break;
        case 8:
            rl_m016_070A4(obj, racer, speed, angle, updateRate);
            break;
        case 7:
            rl_m016_07344(obj, racer, updateRate);
            break;
        case 4:
            rl_m016_0650C(obj, racer, speed, angle, updateRate);
            break;
        case 10:
            rl_m016_06AEC(obj, racer, speed, angle, updateRate);
            break;
        case 11:
            rl_m016_04440(obj, racer, speed, angle, updateRate);
            break;
        case 12:
            rl_m016_073A8(obj, racer, speed, angle, updateRate);
            break;
    }
    if (racer->state != 3 && racer->unk14A != 0) {
        if (racer->state != 10 && racer->state != 4) {
            racer->unk594 = 0.0f;
        }
        racer->unk14A = 0;
    }
    if (racer->state == 4) {
        racer->unk53C = 1;
    } else if (racer->state != 3) {
        racer->unk53C = 0;
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
    racer->flags.bits.weaponDir = 0;
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
    rl_m016_05120(obj, racer, speed, updateRate);
    rl_m016_05BB8(obj, racer, updateRate);
    player_squash_update(obj, racer, updateRate);
    if (racer->unk189 < 4) {
        if (racer->state == 0 || racer->state == 5 || racer->state == 11) {
            func_80035E14(obj, racer, speed, angle, updateRate, &D_m016_07A44);
        } else if (racer->state == 3) {
            func_800364D4(obj, racer, speed, angle, updateRate, &D_m016_07A44);
        }
    }
    if (racer->state != 5 && racer->state != 11 && racer->state != 3 && racer->state != 8) {
        rl_m016_06290(obj, racer, updateRate);
    }
    if (racer->unk5BC != NULL) {
        i = racer->state;
        switch (i) {
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
    rl_m016_02220(obj, racer, updateRate);
    if (racer->flags.bits.unk6) {
        *D_m016_080D8 = 0x4024;
        D_m016_080D8++;
        *D_m016_080D8 = 0x20;
        D_m016_080D8++;
        *D_m016_080D8 = 0x4026;
        D_m016_080D8++;
        *D_m016_080D8 = 0x20;
        D_m016_080D8++;
        *D_m016_080D8 = 0x4028;
        D_m016_080D8++;
        *D_m016_080D8 = 0x20;
        D_m016_080D8++;
    }
    rl_m016_04840(obj, racer, updateRate);
    if (D_800A5084 != 0 && racer->unk189 != 2) {
        u32 joints[6] = { 0, 6, 9, 10, 14, 18 };
        f32 amounts[6] = { 0.8f, 1.55f, 1.55f, 1.55f, 1.55f, 1.55f };
        s32 k;

        k = 5;
        do {
            if (k) {}
            *D_m016_080D8 = (joints[k] * 6) | 0x4000; D_m016_080D8++;
            *D_m016_080D8 = (u16) (amounts[k] * 32768.0f); D_m016_080D8++;
            *D_m016_080D8 = (joints[k] * 6 + 2) | 0x4000; D_m016_080D8++;
            *D_m016_080D8 = (u16) (amounts[k] * 32768.0f); D_m016_080D8++;
            *D_m016_080D8 = (joints[k] * 6 + 4) | 0x4000; D_m016_080D8++;
            *D_m016_080D8 = (u16) (amounts[k] * 32768.0f); D_m016_080D8++;
        } while (k--);
    }
    *D_m016_080D8 = 0x1000;
    D_m016_080D8++;
    if (racer->unk576 != 0 ||
        (racer->unk188 != 0 && (racer->unk532 & racer->pose->unk5) &&
         (obj->velocity.x < -1.0f || obj->velocity.x > 1.0f || obj->velocity.z < -1.0f || obj->velocity.z > 1.0f))) {
        obj->particleEmittersEnabled |= D_m016_07A60[func_8004470C()->unkE2];
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
        racer->flags.bits.useOverride =
            func_8000BC28(obj, inst, model, character->weapon, &racer->unk78, &racer->overridePos.x,
                          &racer->overridePos.y, &racer->overridePos.z);
        func_800221F0(obj, inst, obj->shading, inst->unk10[inst->unkB]);
        if (model->unk11 > 0) {
            obj_tex_animate(obj, model, inst, inst->unkA);
        }
        inst->unkA = 0;
        x = inst->vertices[9].y + 4.0f - obj->trans.y_position;
        if (x >= 64.0f) {
            x = 63.9f;
        }
        ((s8 *) obj->shadow)[0xE] = x * 4.0f;
    }
    result = func_80037550(obj, racer, updateRate);
    switch (func_80033F4C(result)) {
        case 1:
            if (obj->animationID == 0x15) {
                obj_anim_set(obj, 0x15, 0);
            } else {
                rl_m016_04F78(obj, racer, 0x15, 0);
            }
            racer->unk577 = 40;
            break;
        case 2:
            if (obj->animationID == 0x2E) {
                obj_anim_set(obj, 0x2E, 0);
            } else {
                rl_m016_04F78(obj, racer, 0x2E, 0);
            }
            racer->unk577 = 40;
            break;
    }
    if (D_m016_080DE != 0) {
        func_80038630(obj, racer);
    }
    player_loop_sounds_update(obj, racer, 0x100, 0x80, updateRate);
    OBJ58(obj)->unk80 = racer->unk5C;
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
    if (racer->unk14A != 0) {
        if (racer->unkCC == 0) {
            func_80001248(0x9C, (s32) &racer->unkCC);
        }
    } else if (racer->unkCC != 0) {
        sound_stop(racer->unkCC);
    }
    func_8003733C(racer->player, D_m016_080D0, D_m016_080D4, result, racer->unk536 / updateRate);
    racer->unk536 = 0;
}
#else
/* func_m016_00000's local arrays (their initialisers are in .data) */
static u32 D_m016_07D4C[6] = { 0, 6, 9, 10, 14, 18 };
static f32 D_m016_07D64[6] = { 0.8f, 1.55f, 1.55f, 1.55f, 1.55f, 1.55f };

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_00000);
#endif

/* Tables the assembly functions 04934 and 0650C read (after 00000's local arrays in .data) */
static f32 D_m016_07D7C[12] = { 0.0f, 2.0f, 5.0f, 9.0f, 11.0f, 15.0f, 19.0f, 23.0f, 27.0f, 30.0f, 32.0f, 32.0f };
static f32 D_m016_07DAC[12] = { 0.0f, 1.5f, 5.0f, 10.0f, 15.0f, 20.0f, 25.0f, 30.0f, 35.0f, 40.0f, 44.0f, 44.0f };
static f32 D_m016_07DDC[8] = { 0.005f, 0.025f, 0.03f, 0.035f, 0.04f, 0.045f, 0.05f, 0.05f };
static f32 D_m016_07DFC[8] = { 0.01f, 0.015f, 0.02f, 0.025f, 0.035f, 0.045f, 0.045f, 0.045f };

/* Whether the character may fire now */
s32 func_m016_01A60(Object *obj, Racer *racer) {
    s32 anim;
    s32 state;
    s16 *model;

    if (racer->unk576 == 0 && racer->unk13E == 0) {
        anim = obj->animationID;
        if (anim != 8) {
            state = racer->state & ~0x20;
            if (state == 0 || state == 5 || state == 0xB || state == 0xA || (state == 3 && racer->unk14A != 0)) {
                return TRUE;
            }
            model = obj->modelInstances[0];
            if ((anim == 0x15 || anim == 0x2E) && racer->unk53D != 0) {
                return TRUE;
            }
            if ((anim == 0xE || anim == 0x21) && model[0x5E / 2] == 0) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_01B48);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_01D58);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_02220);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_02708);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_02EB4);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_0321C);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_03464);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_03DB0);

/* Two empty exported functions */
void func_m016_03F20(void) {
}

void func_m016_03F28(void) {
}

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_03F30);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_04440);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04840);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_04934);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_04E08);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04F78);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_05120);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_05BB8);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_06290);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_0650C);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_06AEC);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_06E24);

INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_070A4);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_07344);

INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_073A8);
