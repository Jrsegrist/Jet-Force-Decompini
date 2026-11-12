/*
 * Runlink module 16: a player character's behaviour (the control function
 * called every frame, the firing check, and the movement states it runs).
 * A sibling of module 15.
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
    /* 0x0D0 */ u8 padD0[0xD4 - 0xD0];
    /* 0x0D4 */ s32 unkD4;
    /* 0x0D8 */ u8 padD8[0xE8 - 0xD8];
    /* 0x0E8 */ s32 unkE8;
    /* 0x0EC */ s32 unkEC;
    /* 0x0F0 */ u8 padF0[0x104 - 0xF0];
    /* 0x104 */ s16 unk104;
    /* 0x106 */ u8 pad106[0x10A - 0x106];
    /* 0x10A */ s16 unk10A;
    /* 0x10C */ u8 pad10C[0x10D - 0x10C];
    /* 0x10D */ u8 unk10D;
    /* 0x10E */ u8 pad10E[0x11C - 0x10E];
    /* 0x11C */ s16 unk11C;
    /* 0x11E */ s16 unk11E;
    /* 0x120 */ s16 unk120;
    /* 0x122 */ u8 pad122[0x128 - 0x122];
    /* 0x128 */ s16 unk128;
    /* 0x12A */ s16 unk12A;
    /* 0x12C */ s16 unk12C;
    /* 0x12E */ s8 unk12E;
    /* 0x12F */ s8 unk12F;
    /* 0x130 */ u8 pad130[0x138 - 0x130];
    /* 0x138 */ f32 unk138;
    /* 0x13C */ s16 unk13C;
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 pad13F[0x148 - 0x13F];
    /* 0x148 */ s8 flash;
    /* 0x149 */ s8 unk149;
    /* 0x14A */ s8 unk14A;
    /* 0x14B */ s8 unk14B;
    /* 0x14C */ u8 pad14C[0x16C - 0x14C];
    /* 0x16C */ Object *unk16C;
    /* 0x170 */ f32 unk170;
    /* 0x174 */ f32 unk174;
    /* 0x178 */ f32 unk178;
    /* 0x17C */ s16 unk17C;
    /* 0x17E */ u8 pad17E[0x184 - 0x17E];
    /* 0x184 */ s8 unk184;
    /* 0x185 */ s8 unk185;
    /* 0x186 */ s8 unk186;
    /* 0x187 */ u8 pad187[0x188 - 0x187];
    /* 0x188 */ u8 unk188;
    /* 0x189 */ s8 unk189;
    /* 0x18A */ u8 unk18A;
    /* 0x18B */ s8 unk18B;
    /* 0x18C */ u8 pad18C[0x18D - 0x18C];
    /* 0x18D */ u8 unk18D;
    /* 0x18E */ u8 pad18E[0x190 - 0x18E];
    /* 0x190 */ u8 unk190;
    /* 0x191 */ u8 pad191[0x194 - 0x191];
    /* 0x194 */ u8 unk194;
    /* 0x195 */ u8 pad195[0x198 - 0x195];
    /* 0x198 */ u8 unk198;
    /* 0x199 */ u8 unk199;
    /* 0x19A */ u8 pad19A[0x19C - 0x19A];
    /* 0x19C */ u8 unk19C;
    /* 0x19D */ u8 pad19D[0x19E - 0x19D];
    /* 0x19E */ u8 unk19E;
    /* 0x19F */ u8 unk19F;
    /* 0x1A0 */ u8 pad1A0[0x1C6 - 0x1A0];
    /* 0x1C6 */ s16 unk1C6;
    /* 0x1C8 */ s16 unk1C8;
    /* 0x1CA */ s16 unk1CA;
    /* 0x1CC */ s16 unk1CC;
    /* 0x1CE */ s16 unk1CE;
    /* 0x1D0 */ s16 unk1D0;
    /* 0x1D2 */ u8 pad1D2[0x1D8 - 0x1D2];
    /* 0x1D8 */ s16 unk1D8;
    /* 0x1DA */ s16 unk1DA;
    /* 0x1DC */ s16 unk1DC;
    /* 0x1DE */ s16 unk1DE;
    /* 0x1E0 */ s16 unk1E0;
    /* 0x1E2 */ s16 unk1E2;
    /* 0x1E4 */ f32 unk1E4;
    /* 0x1E8 */ f32 unk1E8;
    /* 0x1EC */ s32 unk1EC;
    /* 0x1F0 */ u8 pad1F0[0x1F4 - 0x1F0];
    /* 0x1F4 */ s8 unk1F4;
    /* 0x1F5 */ s8 unk1F5;
    /* 0x1F6 */ s8 unk1F6;
    /* 0x1F7 */ s8 unk1F7;
    /* 0x1F8 */ s8 unk1F8;
    /* 0x1F9 */ u8 unk1F9;
    /* 0x1FA */ u8 unk1FA;
    /* 0x1FB */ u8 unk1FB;
    /* 0x1FC */ u8 pad1FC[0x240 - 0x1FC];
    /* 0x240 */ s16 cmds[0x80]; /* the command list func_m016_00000 builds */
    /* 0x340 */ s8 roll;
    /* 0x341 */ u8 rollShift;
    /* 0x342 */ s8 shake;
    /* 0x343 */ u8 shakeShift;
    /* 0x344 */ u8 pad344[0x354 - 0x344];
    /* 0x354 */ f32 unk354;
    /* 0x358 */ f32 unk358;
    /* 0x35C */ f32 unk35C;
    /* 0x360 */ RacerPose *pose;
    /* 0x364 */ u8 pad364[0x4AC - 0x364];
    /* 0x4AC */ f32 unk4AC;
    /* 0x4B0 */ u8 pad4B0[0x510 - 0x4B0];
    /* 0x510 */ Object *unk510;
    /* 0x514 */ u8 pad514[0x520 - 0x514];
    /* 0x520 */ u32 unk520;
    /* 0x524 */ u8 pad524[0x532 - 0x524];
    /* 0x532 */ u8 unk532;
    /* 0x533 */ u8 unk533;
    /* 0x534 */ u8 unk534;
    /* 0x535 */ u8 pad535[0x536 - 0x535];
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
    /* 0x56C */ u8 unk56C;
    /* 0x56D */ u8 pad56D[0x56F - 0x56D];
    /* 0x56F */ s8 unk56F;
    /* 0x570 */ u8 pad570[0x576 - 0x570];
    /* 0x576 */ s8 unk576;
    /* 0x577 */ s8 unk577;
    /* 0x578 */ s8 unk578;
    /* 0x579 */ s8 unk579;
    /* 0x57A */ s8 unk57A;
    /* 0x57B */ u8 unk57B;
    /* 0x57C */ f32 unk57C;
    /* 0x580 */ s32 unk580;
    /* 0x584 */ s8 roll2;
    /* 0x585 */ s8 unk585;
    /* 0x586 */ u8 pad586[0x58C - 0x586];
    /* 0x58C */ s16 unk58C;
    /* 0x58E */ s8 unk58E;
    /* 0x58F */ u8 pad58F[0x591 - 0x58F];
    /* 0x591 */ s8 unk591;
    /* 0x592 */ s16 unk592;
    /* 0x594 */ f32 unk594;
    /* 0x598 */ u8 pad598[0x5BC - 0x598];
    /* 0x5BC */ void *unk5BC;
    /* 0x5C0 */ s32 unk5C0;
    /* 0x5C4 */ f32 unk5C4;
    /* 0x5C8 */ s16 unk5C8;
    /* 0x5CA */ s16 unk5CA;
    /* 0x5CC */ void *held;
    /* 0x5D0 */ u8 pad5D0[0x5EE - 0x5D0];
    /* 0x5EE */ u8 unk5EE;
    /* 0x5EF */ u8 pad5EF[0x5F0 - 0x5EF];
} Racer;

typedef struct M016Model {
    /* 0x000 */ u8 pad0[0x11 - 0x0];
    /* 0x011 */ u8 unk11;
    /* 0x012 */ u8 pad12[0x2F - 0x12];
    /* 0x02F */ u8 unk2F;
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
    /* 0x0C */ u32 unkC;
    /* 0x10 */ u32 unk10;
    /* 0x14 */ u32 unk14;
    /* 0x18 */ u32 unk18;
    /* 0x1C */ u32 unk1C;
    /* 0x20 */ u32 unk20;
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
extern s32 weapon_usable_by_character(Racer *racer);
extern void racer_pose_begin(Object *obj, Racer *racer, void *table, u8 pose);
extern s32 func_80049D80(s32 a, s32 b);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern f32 table_lerp_diff(f32 from, f32 to, void *table, f32 updateRate);
extern s16 angle_lerp(s16 from, s16 to, f32 amount);
extern s32 func_8004966C(s16 angle);
extern Object *func_80005F14(void *entry, s32 arg1); /* spawn_object */
extern void func_800013FC(s32 sound, s32 handle, u8 volume);
extern s32 collision_is_clear_above(f32 x, f32 y, f32 z, f32 arg3, f32 arg4);
extern void player_turn_limit_update(Object *obj, Racer *racer, f32 *speed, s16 angle, s32 arg4, s32 arg5, f32 arg6,
                                     f32 arg7);
extern void player_quick_turn(Object *obj, Racer *racer, f32 *speed, s16 *angle, s32 arg4, s32 arg5, void *arg6,
                              f32 arg7, f32 updateRate);
extern f32 D_800F6DC4; /* gravity */
extern u8 func_8005AAB8(s32 player);
extern s32 func_8003AABC(Object *obj, Racer *racer, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s16 *yaw, s16 *pitch,
                         f32 updateRate);
extern void cam_fov_zoom_set(s32 player, s32 arg1);
extern u8 D_800A4FD0;
extern void func_80048518(s32 arg0);
extern void func_80033920(Object *obj, Racer *racer);
extern s32 camera_follow_get(void);
extern void *func_8004271C(void);
extern void player_voice_play(Object *obj, s32 arg1);
extern void multiplayer_score_kill(Object *obj, Object *obj2);
extern void glitch_effect_skip_redraw(void);
extern void menu_draw_layout_box(s32 arg0, s32 player, u32 colour);
extern void *func_m001_00188(f32 size, f32 x, f32 y, f32 z, f32 level, f32 vx, f32 vy, f32 vz);
extern void func_m022_02F38(Object *obj, Racer *racer, s32 updateRate);
extern void func_m129_00CF8(Object *obj, Racer *racer, s32 updateRate);
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz);
extern s32 func_80034BB0(Object *obj, Racer *racer, f32 updateRate);
extern void anim_sound_events_play(Object *obj, f32 frame, f32 speed);

/* The two angles func_8004944C reads */
typedef struct M016Angles {
    s16 y_rotation;
    s16 x_rotation;
} M016Angles;

/* An entry of the interaction's hit list */
typedef struct M016Hit {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ u16 flags;
    /* 0x0C */ u8 padC[0x34 - 0xC];
} M016Hit;

/* What obj->interactObj points at, as this module uses it */
typedef struct M016Interact {
    /* 0x00 */ u8 pad0[6];
    /* 0x06 */ s16 hitPoints;
    /* 0x08 */ u8 pad8[0x32 - 0x8];
    /* 0x32 */ s16 numHits;
    /* 0x34 */ u8 pad34[0x40 - 0x34];
    /* 0x40 */ M016Hit *hits;
} M016Interact;

#define INTERACT(obj) ((M016Interact *) (obj)->interactObj)

/* A level object entry built on the stack */
typedef struct M016SpawnEntry {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 angle;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
} M016SpawnEntry;

/* The module's static functions */
#ifdef NON_MATCHING
static s32 func_m016_04E08(Racer *racer);
static void func_m016_04F78(Object *obj, Racer *racer, s32 anim, f32 frame);
static void func_m016_04840(Object *obj, Racer *racer, s32 updateRate);
static void func_m016_07344(Object *obj, Racer *racer, f32 updateRate);
static void func_m016_01B48(Object *obj, Racer *racer, s32 updateRate);
static void func_m016_06290(Object *obj, Racer *racer, s32 updateRate);
static void func_m016_070A4(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_073A8(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_0321C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_02EB4(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_04440(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_03F30(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_04934(Object *obj, Racer *racer, s32 mode, f32 updateRate);
static void func_m016_06AEC(Object *obj, Racer *racer, f32 speed, f32 angle, f32 updateRate);
static void func_m016_02708(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_0650C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
static void func_m016_01D58(Object *obj, Racer *racer, f32 frame, f32 speed);
static void func_m016_02220(Object *obj, Racer *racer, s32 updateRate);
static void func_m016_05BB8(Object *obj, Racer *racer, f32 updateRate);
static void func_m016_05120(Object *obj, Racer *racer, f32 speed, f32 updateRate);
#endif
#ifdef NON_MATCHING
static void func_m016_03DB0(s16 a, s16 b, s16 c, s16 d);
static void func_m016_06E24(Object *obj, Racer *racer, f32 updateRate);
static void func_m016_03464(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
#else
/* Still in assembly: called by their rl_ names */
extern void rl_m016_03DB0(s16 a, s16 b, s16 c, s16 d);
extern void rl_m016_06E24(Object *obj, Racer *racer, f32 updateRate);
extern void rl_m016_03464(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate);
#define func_m016_03DB0 rl_m016_03DB0
#define func_m016_06E24 rl_m016_06E24
#define func_m016_03464 rl_m016_03464
#endif

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

/* The control function: called every frame */
#ifdef NON_MATCHING
/* 4 instructions off (m015's 00000 has the same 4): j, the state switch's variable and the joint-loop counter, gets a1 in the loop and its copy a0, the ROM the other way round (a switch variable separate from the counter puts the switch in v0/a2/a3 instead of the ROM's a1). Its statics below are C under NON_MATCHING only because IDO drops a static with no C caller; all of them match once this does. */
void func_m016_00000(Object *obj, s32 updateRate) {
    s32 deep;
    Racer *racer;
    f32 speed;
    M016WeaponInfo *info;
    s32 j;
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
            func_m016_04F78(obj, racer, 0x27, 0);
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
            func_m016_04F78(obj, racer, 0x18, 0);
            racer->unk56A = racer->state;
            racer->state = 3;
            racer->unk58E = 0;
        }
    } else if ((racer->state == 2 || racer->state == 1) && !(racer->pose->unk5 & racer->unk532) &&
               !(racer->pose->unk6 & racer->unk532) && !(racer->pose->unk7 & racer->unk532)) {
        racer->unk185 += updateRate;
        if (racer->unk185 >= 16) {
            racer->unk57C = obj->trans.y_position;
            func_m016_04F78(obj, racer, 0x18, 0);
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
            func_m016_01B48(obj, racer, updateRate);
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
            func_m016_04F78(obj, racer, 0x25, 0);
        } else {
            func_m016_04F78(obj, racer, 0x26, 0);
        }
    }
    if (obj->interactObj->hitPoints < 0x100 && racer->state != 10) {
        if (D_800A5070 != 0) {
            obj->interactObj->hitPoints = 0x2200;
        } else {
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
            racer->unk194 = 0;
            func_80034018(racer);
            func_m016_04F78(obj, racer, 0x17, 0);
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
            func_m016_02708(obj, racer, speed, angle, updateRate);
            break;
        case 1:
            func_m016_02EB4(obj, racer, speed, angle, updateRate);
            break;
        case 2:
            func_m016_0321C(obj, racer, speed, angle, updateRate);
            break;
        case 3:
            func_m016_03464(obj, racer, speed, angle, updateRate);
            break;
        case 5:
            func_m016_03F30(obj, racer, speed, angle, updateRate);
            break;
        case 6:
            func_m016_06E24(obj, racer, updateRate);
            break;
        case 8:
            func_m016_070A4(obj, racer, speed, angle, updateRate);
            break;
        case 7:
            func_m016_07344(obj, racer, updateRate);
            break;
        case 4:
            func_m016_0650C(obj, racer, speed, angle, updateRate);
            break;
        case 10:
            func_m016_06AEC(obj, racer, speed, angle, updateRate);
            break;
        case 11:
            func_m016_04440(obj, racer, speed, angle, updateRate);
            break;
        case 12:
            func_m016_073A8(obj, racer, speed, angle, updateRate);
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
    func_m016_05120(obj, racer, speed, updateRate);
    func_m016_05BB8(obj, racer, updateRate);
    player_squash_update(obj, racer, updateRate);
    if (racer->unk189 < 4) {
        if (racer->state == 0 || racer->state == 5 || racer->state == 11) {
            func_80035E14(obj, racer, speed, angle, updateRate, &D_m016_07A44);
        } else if (racer->state == 3) {
            func_800364D4(obj, racer, speed, angle, updateRate, &D_m016_07A44);
        }
    }
    if (racer->state != 5 && racer->state != 11 && racer->state != 3 && racer->state != 8) {
        func_m016_06290(obj, racer, updateRate);
    }
    if (racer->unk5BC != NULL) {
        switch (j = racer->state) {
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
    func_m016_02220(obj, racer, updateRate);
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
    func_m016_04840(obj, racer, updateRate);
    if (D_800A5084 != 0 && racer->unk189 != 2) {
        u32 joints[6] = { 0, 6, 9, 10, 14, 18 };
        f32 amounts[6] = { 0.8f, 1.55f, 1.55f, 1.55f, 1.55f, 1.55f };
        UNUSED s32 pad;

        j = 5;
        do {
            if (j) {}
            *D_m016_080D8 = (joints[j] * 6) | 0x4000; D_m016_080D8++;
            *D_m016_080D8 = (u16) (amounts[j] * 32768.0f); D_m016_080D8++;
            *D_m016_080D8 = (joints[j] * 6 + 2) | 0x4000; D_m016_080D8++;
            *D_m016_080D8 = (u16) (amounts[j] * 32768.0f); D_m016_080D8++;
            *D_m016_080D8 = (joints[j] * 6 + 4) | 0x4000; D_m016_080D8++;
            *D_m016_080D8 = (u16) (amounts[j] * 32768.0f); D_m016_080D8++;
        } while (j--);
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
                func_m016_04F78(obj, racer, 0x15, 0);
            }
            racer->unk577 = 40;
            break;
        case 2:
            if (obj->animationID == 0x2E) {
                obj_anim_set(obj, 0x2E, 0);
            } else {
                func_m016_04F78(obj, racer, 0x2E, 0);
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

/* Entering water: the splash sound, and a splash object (or particles) */
#ifdef NON_MATCHING
static void func_m016_01B48(Object *obj, Racer *racer, s32 updateRate) {
    s32 volume;
    u32 savedEmitters;
    M016SpawnEntry entry;
    f32 size;
    s16 rotX;
    s16 rotZ;
    Object *spawned;

    volume = -racer->unk74 * 15.0f;
    if (volume > 0x7F) {
        volume = 0x7F;
    }
    func_80001248(0xD, (s32) &racer->unkD4);
    func_800013FC(0xD, racer->unkD4, volume);
    if (racer->unk74 < -5.0f) {
        entry.objectID = 0xC0;
        entry.size = 0x10;
        entry.x = obj->trans.x_position;
        entry.y = racer->unk5C;
        entry.z = obj->trans.z_position;
        entry.angle = 0x100;
        size = -(racer->unk74 + 5.0f);
        if (size > 10.0f) {
            size = 10.0f;
        }
        entry.unkC = size;
        spawned = func_80005F14(&entry, 1);
        if (spawned != NULL) {
            spawned->level_entry = NULL;
        }
    } else {
        savedEmitters = obj->particleEmittersEnabled;
        rotX = obj->trans.rotation.x_rotation;
        rotZ = obj->trans.rotation.z_rotation;
        obj->trans.y_position += racer->unk60;
        obj->trans.rotation.x_rotation = 0;
        obj->trans.rotation.z_rotation = 0;
        obj->particleEmittersEnabled = 0x40;
        func_8005F5AC(obj, updateRate);
        obj->trans.rotation.z_rotation = rotZ;
        obj->trans.rotation.x_rotation = rotX;
        obj->trans.y_position -= racer->unk60;
        obj->particleEmittersEnabled = savedEmitters;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_01B48);
#endif

/* Footstep splashes when a foot comes down in shallow water */
#ifdef NON_MATCHING
static void func_m016_01D58(Object *obj, Racer *racer, f32 frame, f32 speed) {
    s32 n;
    s32 count[2];
    f32 from;
    f32 to;
    f32 sine;
    f32 cosine;
    f32 x;
    f32 z;
    f32 side[2];
    f32 fwd[2];
    f32 size;
    Vec3f vel;
    M016Angles rot;

    if (racer->unk60 != 0.0f) {
        n = 0;
        if (speed < 0.0f) {
            from = obj->unk28;
            to = frame;
            if (frame < from) {
                from -= 1.0f;
            }
        } else {
            from = frame;
            to = obj->unk28;
            if (to < frame) {
                to += 1.0f;
            }
        }
        if (racer->state == 0 && racer->unk60 > 0.0f && racer->unk4 < -1.0f) {
            if (from < 0.4f && 0.4f <= to) {
                side[n] = 4.0f;
                fwd[n] = 16.0f;
                count[n] = 2 - (s32) (racer->unk4 + 1.0f);
                n++;
            }
            if (from < 0.9f && 0.9f <= to) {
                side[n] = -4.0f;
                fwd[n] = 16.0f;
                count[n] = 2 - (s32) (racer->unk4 + 1.0f);
                n++;
            }
        }
        if (racer->state == 4 && racer->unk60 >= 45.0f && racer->unk60 <= 48.5f && racer->unk4 < -1.0f &&
            from < 0.875f && 0.875f <= to) {
            fwd[0] = -12.0f;
            side[0] = -6.0f;
            count[0] = 2 - (s32) (racer->unk4 + 1.0f);
            n = 2;
            fwd[1] = -12.0f;
            count[1] = count[0];
            side[1] = 6.0f;
        }
        while (n--) {
            sine = func_80049C34(obj->trans.rotation.y_rotation);
            cosine = func_80049C30(obj->trans.rotation.y_rotation);
            x = obj->trans.x_position + (side[n] * cosine + fwd[n] * sine);
            z = obj->trans.z_position + (fwd[n] * cosine - side[n] * sine);
            while (count[n]--) {
                size = func_80048E2C(50, 100) * 0.01f;
                rot.y_rotation = func_80048E2C(-0x2555, 0x2555) + obj->trans.rotation.y_rotation;
                rot.x_rotation = func_80048E2C(0x2400, 0x3C00);
                vel.z = func_80048E2C(12, 22) * -0.1f + racer->unk4 * 0.25f;
                func_8004944C((Vec3s *) &rot, &vel);
                func_m001_00188(size, x, racer->unk5C, z, racer->unk5C, vel.x, vel.y, vel.z);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_01D58);
#endif

/* Hits taken: camera shakes, the hit flash, and damage from the level (hit points >= 0x100) */
#ifdef NON_MATCHING
static void func_m016_02220(Object *obj, Racer *racer, s32 updateRate) {
    s32 i;
    s32 hit;
    s32 shake;
    s32 shakeShift;
    s32 roll;
    s32 rollShift;
    s32 v;
    M016Hit *hits;

    hits = INTERACT(obj)->hits;
    i = 0;
    hit = FALSE;
    shake = 0;
    roll = 0;
    for (; i < INTERACT(obj)->numHits; i++, hits++) {
        if (hits->flags & 0x40) {
            hit = TRUE;
            switch (i) {
                case 0:
                case 1:
                case 2:
                case 3:
                    D_m016_080D0 |= 1;
                    D_m016_080D4 |= 1;
                    shake = 15;
                    shakeShift = 5;
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    D_m016_080D0 |= 1;
                    D_m016_080D4 |= 1;
                    shake = 15;
                    shakeShift = 5;
                    break;
                case 8:
                    D_m016_080D0 |= 2;
                    D_m016_080D4 |= 1;
                    roll = 15;
                    rollShift = 4;
                    break;
                default:
                    D_m016_080D0 |= 4;
                    D_m016_080D4 |= 1;
                    roll = 15;
                    rollShift = 3;
                    break;
            }
        }
    }
    if (hit) {
        if (shake != 0 && racer->shake == 0) {
            racer->shake = shake;
            racer->shakeShift = shakeShift;
        }
        if (roll != 0 && racer->roll == 0) {
            racer->roll = roll;
            racer->rollShift = rollShift;
        }
    }
    if (racer->shake != 0) {
        shake = func_8004966C(racer->shake << 11) >> racer->shakeShift;
        *D_m016_080D8 = 8;
        D_m016_080D8++;
        *D_m016_080D8 = shake;
        D_m016_080D8++;
        *D_m016_080D8 = 0x14;
        D_m016_080D8++;
        *D_m016_080D8 = -shake;
        D_m016_080D8++;
        if (updateRate < racer->shake) {
            racer->shake -= updateRate;
        } else {
            racer->shake = 0;
        }
    }
    if (racer->roll != 0) {
        shake = func_8004966C(racer->roll << 11) >> racer->rollShift;
        v = shake >> 1;
        *D_m016_080D8 = 6;
        D_m016_080D8++;
        *D_m016_080D8 = v;
        D_m016_080D8++;
        *D_m016_080D8 = 0x3C;
        D_m016_080D8++;
        *D_m016_080D8 = v;
        D_m016_080D8++;
        if (updateRate < racer->roll) {
            racer->roll -= updateRate;
        } else {
            racer->roll = 0;
        }
    }
    if (INTERACT(obj)->hitPoints >= 0x100) {
        if (racer->unk190 == 0x80) {
            if (racer->unk5C8 == 0) {
                racer->unk190 = 0x3C;
                hit = TRUE;
                INTERACT(obj)->hitPoints -= 0x100;
                D_m016_080D0 |= 2;
                D_m016_080D4 |= 2;
            }
        } else if (racer->unk190 == 0xC0) {
            if (racer->unk5C8 == 0) {
                racer->unk190 = 0x3C;
                hit = TRUE;
                INTERACT(obj)->hitPoints -= 0x200;
                D_m016_080D0 |= 4;
                D_m016_080D4 |= 2;
            }
        }
        if (INTERACT(obj)->hitPoints < 0x100) {
            INTERACT(obj)->hitPoints = 0;
            if (D_800A4FC4 != 0) {
                multiplayer_score_kill(obj, obj);
            }
        }
    }
    racer->unk190 &= 0x3F;
    if (updateRate < racer->unk190) {
        racer->unk190 -= updateRate;
    } else {
        racer->unk190 = 0;
    }
    if (hit && racer->state != 10) {
        player_voice_play(obj, 3);
        racer->flash = 8;
    } else if (updateRate < racer->flash) {
        racer->flash -= updateRate;
    } else {
        racer->flash = 0;
    }
    if (racer->flash != 0) {
        glitch_effect_skip_redraw();
        menu_draw_layout_box(0, racer->player, 0xFF0000FF);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_02220);
#endif

/* State 0: on foot */
#ifdef NON_MATCHING
static void func_m016_02708(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    s32 out;
    f32 x;
    s16 oldAngle;

    if (racer->unk576 != 0) {
        racer->unk576 -= (s32) updateRate;
        if (racer->unk576 < 0) {
            racer->unk576 = 0;
        }
    }
    if (obj->animationID == 0x19 && racer->unk56B == 0) {
        func_m016_04F78(obj, racer, func_m016_04E08(racer), 0);
    }
    if (gPlayerButtonsHeld & 0x10) {
        racer->unk11C -= racer->unk10A;
        racer->unk10A = 0;
        angle = racer->unk11C;
        if (!camera_follow_get()) {
            func_8004271C();
        }
        racer->unk1E2 = 0;
        racer->unk1E0 = 0;
        racer->state = 11;
        racer->unk1CE = 0;
        racer->unk1D0 = 0;
        racer->unk1E4 = 0.0f;
        racer->unk1E8 = 0.0f;
        racer->unk1C6 = obj->trans.rotation.y_rotation;
        racer->unk1C8 = obj->trans.rotation.x_rotation;
        racer->unk1CA = racer->unk1C6;
        racer->unk1CC = racer->unk1C8;
        racer->unk1DC = racer->unk1CE;
        racer->unk1DE = racer->unk1D0;
        return;
    }
    player_turn_limit_update(obj, racer, &speed, angle, 0x5000, 0x6800, 5.0f, 3.0f);
    func_m016_04934(obj, racer, 0, updateRate);
    if ((D_800A18B0->unkC & gPlayerButtonsPressed) && racer->unk534 == 0) {
        x = racer->unk4;
        if (x < 0.0f) {
            x = -x;
        }
        if (racer->unk569 != 0) {
            racer->unk57A = 7;
            racer->unk57B = 0;
            func_m016_04F78(obj, racer, 7, 0);
        } else if ((x > 1.25f && speed != 0) || racer->unk56C != 0) {
            racer->unk57A = 0;
            if (D_800A4FC4 != 0 && racer->unk189 == 7) {
                obj->velocity.y += 12.0f;
            } else {
                obj->velocity.y += 10.0f;
            }
            if (racer->unk510 != NULL) {
                obj->velocity.y -= racer->unk510->velocity.y;
            }
            func_m016_04F78(obj, racer, 6, 0);
            player_voice_play(obj, 0);
        } else {
            racer->unk57A = 24;
            racer->unk57B = 0;
            func_m016_04F78(obj, racer, 5, 0);
        }
        racer->unk57C = obj->trans.y_position;
        racer->state = 3;
        racer->unk58E = 0;
    } else if ((D_800A18B0->unk10 & gPlayerButtonsPressed) && racer->unk189 < 4) {
        x = racer->unk4;
        if (x < 0.0f) {
            x = -x;
        }
        if (x > 2.0f) {
            racer->state = 1;
            racer->unk5CA = 0;
            func_m016_04F78(obj, racer, 15, 0);
            racer->unk576 = 40;
        } else if (speed > 0.5f) {
            racer->state = 2;
            racer->unk5CA = 0;
            func_m016_04F78(obj, racer, 4, 0);
        } else {
            racer->state = 1;
            racer->unk5CA = 0;
            func_m016_04F78(obj, racer, 13, 0);
        }
        racer->unk56C = 0;
    }
    if (D_800A18B0->unk0 & gPlayerButtonsHeld) {
        racer->unk1F4 = 15;
    }
    if (!(racer->pose->unk5 & racer->unk532)) {
        racer->unk1F4 = 0;
    }
    if (racer->unk569 == 0) {
        speed = -speed;
    } else {
        angle += 0x8000;
    }
    player_quick_turn(obj, racer, &speed, &angle, 0x6AAA, 0x71C, &out, D_m016_080E4, updateRate);
    if (out != 0) {
        func_m016_04F78(obj, racer, 15, 0);
    }
    oldAngle = racer->unk11C;
    if (!camera_follow_get() && !func_8004271C()) {
        if ((racer->flags.half[0] & 0xF) && !func_8005AAB8(racer->player)) {
            racer->unk14 += (0.0333f - racer->unk14) * (1.0f - func_80049BEC(0.9f, updateRate));
        } else {
            racer->unk14 += (0.075f - racer->unk14) * (1.0f - func_80049BEC(0.975f, updateRate));
        }
    }
    x = racer->unk4;
    if (x < 0.0f) {
        x = -x;
    }
    x /= 1.0f;
    x *= x;
    if (x > 1.0f) {
        x = 1.0f;
    }
    x = 1.0f - ((racer->unk14 - 0.005f) * x + 0.005f);
    if (racer->unk13E != 0) {
        x = 0.925f;
    }
    racer->unk11C = angle_lerp(racer->unk11C, angle, 1.0f - func_80049BEC(x, updateRate));
    x = racer->unk4;
    racer->unk4 = (1.0f - func_80049BEC(0.95f, updateRate)) * (speed - racer->unk4) + racer->unk4;
    oldAngle = racer->unk11C - oldAngle;
    x -= racer->unk4;
    if (racer->state == 0 && x < -0.7f && obj->animationID != 0x15 && obj->animationID != 0x2E &&
        obj->animationID != 0x19) {
        racer->unk576 = 4;
        func_m016_04F78(obj, racer, 15, 0);
    }
    x = racer->unk4;
    if (racer->unk4 < 0.0f) {
        x = -racer->unk4;
    }
    if (x > 3.0f) {
        x = 3.0f;
    }
    racer->unk120 = oldAngle * x * 0.3333f;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_02708);
#endif

/* State 1 */
#ifdef NON_MATCHING
static void func_m016_02EB4(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    s32 deep;

    deep = FALSE;
    if (racer->unk60 > 30) {
        racer->unk5CA += (s32) updateRate;
        if (racer->unk5CA > 120) {
            deep = TRUE;
        }
    } else {
        racer->unk5CA = 0;
    }
    if ((deep || (D_800A18B0->unkC & gPlayerButtonsHeld)) && racer->unk56C == 0 &&
        collision_is_clear_above(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 60.0f, 15.0f)) {
        func_m016_04F78(obj, racer, func_m016_04E08(racer), 0);
        racer->state = 0;
        racer->unk576 = 0;
    } else if (racer->unk576 != 0) {
        racer->unk576 -= (s32) updateRate;
        racer->unk10 *= func_80049BEC(0.85f, updateRate);
        if (racer->unk576 <= 0) {
            racer->unk576 = 0;
            racer->unk4 = 0.0f;
            racer->unk10 = 0.0f;
            if (speed > 0.5f && !(gPlayerButtonsHeld & 0x10)) {
                racer->state = 2;
                func_m016_04F78(obj, racer, 0x16, 0.0f);
            } else {
                func_m016_04F78(obj, racer, 0xE, 0.0f);
            }
        }
    }
    if (racer->state == 1 && racer->unk576 == 0) {
        if ((gPlayerButtonsHeld & 0x10) && func_m016_01A60(obj, racer)) {
            racer->state = 5;
            racer->unk11C -= racer->unk10A;
            racer->unk10A = 0;
            racer->unk1E2 = 0;
            racer->unk1E0 = 0;
            racer->unk1CE = 0;
            racer->unk1D0 = 0;
            racer->unk1E4 = 0.0f;
            racer->unk1E8 = 0.0f;
            racer->unk1C6 = obj->trans.rotation.y_rotation;
            racer->unk1C8 = obj->trans.rotation.x_rotation;
            racer->unk1CA = racer->unk1C6;
            racer->unk1CC = racer->unk1C8;
            racer->unk1DC = racer->unk1CE;
            racer->unk1DE = racer->unk1D0;
            return;
        }
        func_m016_04934(obj, racer, 1, updateRate);
        if (obj->animationID != 0xB && obj->animationID != 0x31) {
            if (D_800A18B0->unk0 & gPlayerButtonsHeld) {
                racer->unk1F4 = 15;
            }
            if (!(racer->pose->unk5 & racer->unk532)) {
                racer->unk1F4 = 0;
            }
            if (speed > 0.5f) {
                func_m016_04F78(obj, racer, 0x16, 0);
                racer->state = 2;
            }
        }
    }
    racer->unk4 *= func_80049BEC(0.95f, updateRate);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_02EB4);
#endif

/* State 2 */
#ifdef NON_MATCHING
static void func_m016_0321C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    s32 unused;
    s32 rate; /* also the "stuck in deep water" flag */

    rate = FALSE;
    if (racer->unk60 > 15) {
        racer->unk5CA += (s32) updateRate;
        if (racer->unk5CA > 120) {
            rate = TRUE;
        }
    } else {
        racer->unk5CA = 0;
    }
    if ((rate || (D_800A18B0->unkC & gPlayerButtonsHeld)) && racer->unk56C == 0 &&
        collision_is_clear_above(obj->trans.x_position, obj->trans.y_position, obj->trans.z_position, 60.0f, 15.0f)) {
        func_m016_04F78(obj, racer, 8, 0);
        racer->state = 0;
        return;
    }
    func_m016_04934(obj, racer, 2, updateRate);
    player_turn_limit_update(obj, racer, &speed, angle, 0x5000, 0x6800, 1.25f, 1.25f);
    if (racer->unk569 == 0) {
        speed = -speed;
    } else {
        angle += 0x8000;
    }
    player_quick_turn(obj, racer, &speed, &angle, 0x6AAA, 0x71C, &unused, 0.029166667f, updateRate);
    rate = updateRate;
    racer->unk4 = (1.0f - func_80049BEC(0.96f, rate)) * (speed - racer->unk4) + racer->unk4;
    racer->unk11C = angle_lerp(racer->unk11C, angle, 1.0f - func_80049BEC(0.975f, rate));
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_0321C);
#endif

#ifdef NON_MATCHING
/* Matches in the probe; C only under NON_MATCHING until its caller 00000 matches */
/* State 3: in the air */
static void func_m016_03464(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    f32 fall;
    s32 i;
    s32 oldTime;
    s32 j;
    f32 q;

    racer->unk576 = 0;
    if (!(D_800A18B0->unkC & gPlayerButtonsHeld)) {
        racer->unk58E = 1;
    }
    if ((racer->pose->unk5 & racer->unk532) && racer->unk57A == 0) {
        racer->unk14A = 0;
        racer->unk14B = 0;
        racer->unk594 = 0.0f;
        racer->state = racer->unk56A;
        fall = racer->unk57C - obj->trans.y_position;
        if (racer->character == 1) {
            if (fall > 240.0f) {
                racer->unk190 = 0x80;
            } else if (fall > 400.0f) {
                racer->unk190 = 0xC0;
            }
        } else if (fall > 400.0f) {
            racer->unk190 = 0x80;
        }
        if (fall > 120.0f) {
            func_m016_04F78(obj, racer, 0x19, 0.0f);
            racer->unk56B = 30;
            racer->state = 0;
        } else if (racer->state == 2) {
            func_m016_04F78(obj, racer, 4, 0.0f);
        } else if (racer->state == 1) {
            func_m016_04F78(obj, racer, 13, 0.0f);
        } else {
            func_m016_04F78(obj, racer, func_m016_04E08(racer), 0);
            racer->state = 0;
        }
        player_voice_play(obj, 1);
        if (racer->unk60 < 1.0f) {
            obj->particleEmittersEnabled |= D_m016_07A6C[func_8004470C()->unkE2];
        }
    } else if (racer->unk57A > 0) {
        racer->unk57A -= (s32) updateRate;
        if (D_800A18B0->unkC & gPlayerButtonsHeld) {
            racer->unk57B += (s32) updateRate;
        } else {
            racer->unk57A = 0;
        }
        if (racer->unk57A <= 0) {
            if (obj->unk28 < 0.25f) {
                obj->unk28 = 0.4f;
            }
            racer->unk57A = 0;
            player_voice_play(obj, 0);
            if (racer->unk569 != 0) {
                obj->velocity.y += 10.0f;
            } else {
                if (racer->unk57B > 24) {
                    racer->unk57B = 24;
                }
                if (D_800A4FC4 != 0 && racer->unk189 == 7) {
                    fall = 15.0f;
                } else {
                    fall = 12.0f;
                }
                obj->velocity.y += 6.0f + (fall - 6.0f) * racer->unk57B / 24.0f;
            }
        }
        speed = 0.0f;
    } else if (racer->unk14A != 0) {
        obj->unk28 = 0.0f;
        q = 1.0f - func_80049BEC(0.95f, updateRate);
        racer->unk57C += (obj->trans.y_position - racer->unk57C) * q;
        oldTime = racer->unk58C;
        if ((D_800A18B0->unk10 & gPlayerButtonsHeld) && racer->unk58C > 0) {
            if (racer->unk58C > 0) {
                racer->unk58C -= (s32) updateRate;
            }
            racer->unk591 += (s32) updateRate * 2;
            q = 1.0f - func_80049BEC(0.95f, updateRate);
            racer->unk594 += ((func_80049C34(racer->unk591 << 8) * 0.125f + 1.0f) - racer->unk594) * q;
            obj->velocity.y *= func_80049BEC(0.925f, updateRate);
            D_m016_080D0 |= 1;
            D_m016_080D4 |= 1;
        } else if ((D_800A18B0->unkC & gPlayerButtonsHeld) && racer->unk58C > 0) {
            if (racer->unk58C > 0) {
                racer->unk58C -= (s32) updateRate * 2;
            }
            racer->unk594 += (1.0f - racer->unk594) * (1.0f - func_80049BEC(0.95f, updateRate));
            obj->velocity.y += (5.0f - obj->velocity.y) * (1.0f - func_80049BEC(0.95f, updateRate));
            D_m016_080D0 |= 2;
            D_m016_080D4 |= 1;
        } else {
            racer->unk594 *= func_80049BEC(0.96f, updateRate);
            if (racer->unk594 < 0.2f) {
                func_m016_04F78(obj, racer, 0x18, 0);
                racer->unk14A = 0;
                racer->unk594 = 0.0f;
            }
        }
        if (oldTime != racer->unk58C) {
            racer->unk592 = 360;
        }
        if (racer->unk58C <= 600) {
            for (i = 0, j = 0; i != 11; i++, j += 60) {
                if (j >= racer->unk58C && j < oldTime) {
                    func_80001248((u16) (i + 0x15F), 0);
                    break;
                }
            }
        }
        if (D_800A18B0->unk0 & gPlayerButtonsHeld) {
            racer->unk1F4 = 15;
        }
    } else if (obj->trans.y_position < racer->unk57C - 120.0f && obj->animationID != 0x18) {
        func_m016_04F78(obj, racer, 0x18, 0);
    } else if (racer->character == 5 && racer->unk189 == 0 && racer->unk58E != 0 && racer->unk58C > 0 &&
               ((D_800A18B0->unkC | D_800A18B0->unk10) & gPlayerButtonsHeld)) {
        func_m016_04F78(obj, racer, 0x33, 0);
        racer->unk128 = racer->unk11C;
        racer->unk14A = 1;
    }
    player_turn_limit_update(obj, racer, &speed, angle, 0x5000, 0x6800, 5.0f, 3.0f);
    if (racer->unk569 == 0) {
        speed = -speed;
    } else {
        angle += 0x8000;
    }
    racer->unk4 = (1.0f - func_80049BEC(1.0f - (racer->unk594 * -0.030000001f + 0.05f), updateRate)) *
                      (speed - racer->unk4) + racer->unk4;
    q = 1.0f - (racer->unk594 * -0.041f + 0.075f);
    racer->unk11C = angle_lerp(racer->unk11C, angle, 1.0f - func_80049BEC(q, updateRate));
    if (racer->unk14A != 0) {
        racer->unk128 = angle_lerp(racer->unk128, racer->unk11C, 1.0f - func_80049BEC(0.975f, updateRate));
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_03464);
#endif

#ifdef NON_MATCHING
/* Matches in the probe; C only under NON_MATCHING until its caller 00000 matches */
static void func_m016_03DB0(s16 a, s16 b, s16 c, s16 d) {
    s16 half;

    if (a < -0x2AAA) {
        a = -0x2AAA;
    }
    if (c > 0x2AAA) {
        c = 0x2AAA;
    }
    if (c < -0x2AAA) {
        c = -0x2AAA;
    }
    half = a * 0.5f;
    c -= half;
    d -= b;
    *D_m016_080D8 = 6;
    D_m016_080D8++;
    *D_m016_080D8 = half;
    D_m016_080D8++;
    *D_m016_080D8 = 8;
    D_m016_080D8++;
    *D_m016_080D8 = b;
    D_m016_080D8++;
    *D_m016_080D8 = 0x12;
    D_m016_080D8++;
    *D_m016_080D8 = a - half;
    D_m016_080D8++;
    *D_m016_080D8 = 0x3C;
    D_m016_080D8++;
    *D_m016_080D8 = c;
    D_m016_080D8++;
    *D_m016_080D8 = 0x3E;
    D_m016_080D8++;
    *D_m016_080D8 = d;
    D_m016_080D8++;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_03DB0);
#endif

/* Two empty exported functions */
void func_m016_03F20(void) {
}

void func_m016_03F28(void) {
}

/* State 5: aiming */
#ifdef NON_MATCHING
static void func_m016_03F30(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    s16 yaw;
    s16 pitch;
    s16 shake;
    f32 amount;
    s32 pad;

    func_80047C1C(racer->player, racer->character);
    racer->unk576 = 0;
    racer->unk13E = 0;
    if (!(gPlayerButtonsHeld & 0x10) && D_m016_080DC == 0) {
        racer->state = 1;
        racer->unk104 = 0x8000 - obj->trans.rotation.y_rotation;
        return;
    }
    racer->unk10 *= func_80049BEC(0.85f, updateRate);
    if (-0.1f < racer->unk10 && racer->unk10 < 0.1f) {
        racer->unk10 = 0.0f;
    }
    racer->unk4 *= func_80049BEC(0.95f, updateRate);
    if (-0.1f < racer->unk4 && racer->unk4 < 0.1f) {
        racer->unk4 = 0.0f;
    }
    if (D_m016_080DC == 0) {
        if (func_8003AABC(obj, racer, 0xE38, 0xE38, -0x2AAA, 0x2AAA, &yaw, &pitch, updateRate)) {
            cam_fov_zoom_set(racer->player, racer->unk1EC);
        }
    } else {
        yaw = racer->unk1DC;
        pitch = racer->unk1DE - racer->unk1E2;
    }
    amount = 1.0f - func_80049BEC(0.8333f, updateRate);
    if (racer->unk1E2 < -0xA80) {
        racer->unk1E2 = -0xA80;
    }
    if (racer->unk1E2 > 0x1555) {
        racer->unk1E2 = 0x1555;
    }
    racer->unk1E0 = angle_lerp(racer->unk1E0, pitch, amount);
    pitch += racer->unk1E2;
    racer->unk1DC = angle_lerp(racer->unk1DC, yaw, amount);
    racer->unk1DE = angle_lerp(racer->unk1DE, pitch, amount);
    racer->unk1CE = angle_lerp(racer->unk1CE, yaw, amount);
    racer->unk1D0 = angle_lerp(racer->unk1D0, pitch, amount);
    racer->unk1C6 = racer->unk1CE + obj->trans.rotation.y_rotation;
    racer->unk1C8 = racer->unk1D0 + obj->trans.rotation.x_rotation;
    racer->unk1CA = racer->unk1C6;
    racer->unk1CC = racer->unk1C8;
    yaw = racer->unk1CE;
    pitch = racer->unk1D0;
    if (racer->unk1F6 != 0) {
        shake = (func_8004966C(racer->unk1F6 << 12) * racer->unk1F7) >> 9;
    } else {
        shake = 0;
    }
    *D_m016_080D8 = 6;
    D_m016_080D8++;
    *D_m016_080D8 = pitch + shake;
    D_m016_080D8++;
    *D_m016_080D8 = 8;
    D_m016_080D8++;
    *D_m016_080D8 = yaw;
    D_m016_080D8++;
    *D_m016_080D8 = 0x12;
    D_m016_080D8++;
    *D_m016_080D8 = shake;
    D_m016_080D8++;
    *D_m016_080D8 = 0x3C;
    D_m016_080D8++;
    *D_m016_080D8 = racer->unk1DE - pitch;
    D_m016_080D8++;
    *D_m016_080D8 = 0x3E;
    D_m016_080D8++;
    *D_m016_080D8 = racer->unk1DC - yaw;
    D_m016_080D8++;
    if (racer->unk60 > 30.0f) {
        racer->unk5CA += (s32) updateRate;
    } else {
        racer->unk5CA = 0;
    }
    if ((func_8005AAB8(racer->player) && ((D_800A18B0->unk20 | D_800A18B0->unk1C) & gPlayerButtonsHeld)) ||
        (D_800A18B0->unkC & gPlayerButtonsPressed) || racer->unk5CA > 120) {
        racer->state = 11;
        func_m016_04F78(obj, racer, func_m016_04E08(racer), 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_03F30);
#endif

/* State 11: aiming while strafing */
#ifdef NON_MATCHING
static void func_m016_04440(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    s16 yaw;
    s16 pitch;
    f32 amount;
    M016Char *chr;

    chr = func_80047C1C(racer->player, racer->character);
    racer->unk576 = 0;
    racer->unk13E = 0;
    if (!(gPlayerButtonsHeld & 0x10) && D_m016_080DC == 0) {
        racer->state = 0;
        racer->unk104 = 0x8000 - obj->trans.rotation.y_rotation;
        return;
    }
    if (func_8005AAB8(racer->player) && chr->weapon != 7 &&
        ((s32) (D_800A18B0->unk20 | D_800A18B0->unk1C) & gPlayerButtonsHeld)) {
        if (D_800A18B0->unk1C & gPlayerButtonsHeld) {
            racer->unk4 -= 0.2f * updateRate;
            if (racer->unk4 < -(65.0f * D_m016_080E4)) {
                racer->unk4 = -(65.0f * D_m016_080E4);
            }
        } else {
            racer->unk4 += 0.2f * updateRate;
            if (racer->unk4 > 3.0f) {
                racer->unk4 = 3.0f;
            }
        }
    } else {
        racer->unk4 *= func_80049BEC(0.95f, updateRate);
        if (-0.1f < racer->unk4 && racer->unk4 < 0.1f) {
            racer->unk4 = 0.0f;
        }
    }
    func_m016_04934(obj, racer, 0, updateRate);
    if (D_m016_080DC == 0) {
        if (func_8003AABC(obj, racer, 0xE38, 0xE38, -0x2AAA, 0x2AAA, &yaw, &pitch, updateRate)) {
            cam_fov_zoom_set(racer->player, racer->unk1EC);
        }
    } else {
        yaw = racer->unk1DC;
        pitch = racer->unk1DE - racer->unk1E2;
    }
    amount = 1.0f - func_80049BEC(0.8333f, updateRate);
    racer->unk1E0 = angle_lerp(racer->unk1E0, pitch, amount);
    pitch += racer->unk1E2;
    racer->unk1DC = angle_lerp(racer->unk1DC, yaw, amount);
    racer->unk1DE = angle_lerp(racer->unk1DE, pitch, amount);
    racer->unk1CE = angle_lerp(racer->unk1CE, yaw, amount);
    racer->unk1D0 = angle_lerp(racer->unk1D0, pitch, amount);
    racer->unk1C6 = racer->unk1CE + obj->trans.rotation.y_rotation;
    racer->unk1C8 = racer->unk1D0 + obj->trans.rotation.x_rotation;
    racer->unk1CA = racer->unk1C6;
    racer->unk1CC = racer->unk1C8;
    yaw = racer->unk1CE;
    pitch = racer->unk1D0;
    if (racer->unk1F6 != 0) {
        pitch += (racer->unk1F7 * func_8004966C(racer->unk1F6 << 12)) >> 8;
    }
    func_m016_03DB0(pitch, yaw, racer->unk1DE, racer->unk1DC);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_04440);
#endif

#ifdef NON_MATCHING
static void func_m016_04840(Object *obj, Racer *racer, s32 updateRate) {
    f32 x;
    f32 z;
    s32 angle;
    s32 i;

    x = racer->unk4;
    z = racer->unk10;
    if (x < 0.0f) {
        x = -x;
    }
    if (z < 0.0f) {
        z = -z;
    }
    if (z < x) {
        angle = -func_80049C84(racer->unk10, x);
    } else {
        angle = 0;
    }
    i = updateRate;
    while (i--) {
        racer->unk580 += func_80049D80(racer->unk580, angle) >> 4;
    }
    *D_m016_080D8 = 0x44;
    D_m016_080D8++;
    *D_m016_080D8 = racer->unk580;
    D_m016_080D8++;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04840);
#endif

/* Sidesteps and rolls (mode 0: sidestep, 1 and 2: rolls) on the shoulder buttons */
#ifdef NON_MATCHING
static void func_m016_04934(Object *obj, Racer *racer, s32 mode, f32 updateRate) {
    s32 anim;
    f32 z;
    f32 x;
    f32 to;
    void *table;
    f32 rate;

    racer->unk56C = 0;
    switch (racer->roll2) {
        case 1:
        case 2:
        case 3:
        case 4:
            if (obj->unk28 == 1.0f) {
                racer->roll2 = 0;
                racer->unk10 = 0.0f;
            }
            break;
    }
    if (racer->roll2 == 0) {
        if (mode == 2) {
            if (D_800A18B0->unk14 & gPlayerButtonsHeld) {
                racer->roll2 = 3;
                func_m016_04F78(obj, racer, 0x32, 0);
                obj->unk28 = 0.0f;
                racer->unk56C = 1;
            } else if (D_800A18B0->unk18 & gPlayerButtonsHeld) {
                racer->roll2 = 4;
                func_m016_04F78(obj, racer, 0xC, 0);
                obj->unk28 = 0.0f;
                racer->unk56C = 1;
            }
        } else if (mode == 1) {
            if (D_800A18B0->unk14 & gPlayerButtonsHeld) {
                racer->roll2 = 1;
                func_m016_04F78(obj, racer, 0x31, 0);
                obj->unk28 = 0.0f;
                racer->unk56C = 1;
            } else if (D_800A18B0->unk18 & gPlayerButtonsHeld) {
                racer->roll2 = 2;
                func_m016_04F78(obj, racer, 0xB, 0);
                obj->unk28 = 0.0f;
                racer->unk56C = 1;
            }
        } else {
            if (D_800A18B0->unk14 & gPlayerButtonsHeld) {
                racer->unk10 -= 0.2f * updateRate;
                if (racer->unk10 < -2.5f) {
                    racer->unk10 = -2.5f;
                }
                anim = 9;
                racer->unk56C = 1;
            } else if (D_800A18B0->unk18 & gPlayerButtonsHeld) {
                racer->unk10 += 0.2f * updateRate;
                if (racer->unk10 > 2.5f) {
                    racer->unk10 = 2.5f;
                }
                anim = 10;
                racer->unk56C = 1;
            }
            if (racer->unk56C != 0) {
                z = racer->unk10;
                if (z < 0.0f) {
                    z = -z;
                }
                x = racer->unk4;
                if (x < 0.0f) {
                    x = -x;
                }
                if (x < z) {
                    func_m016_04F78(obj, racer, anim, obj->unk28);
                }
            }
        }
    }
    table = NULL;
    switch (racer->roll2) {
        case 3:
            if (obj->animationID == 0x32) {
                table = D_m016_07D7C;
                rate = 0.028f;
            }
            break;
        case 4:
            if (obj->animationID == 0xC) {
                table = D_m016_07D7C;
                rate = 0.028f;
            }
            break;
        case 1:
            if (obj->animationID == 0x31) {
                table = D_m016_07DAC;
                rate = 0.0175f;
            }
            break;
        case 2:
            if (obj->animationID == 0xB) {
                table = D_m016_07DAC;
                rate = 0.0175f;
            }
            break;
    }
    if (table != NULL) {
        to = obj->unk28 + rate * updateRate;
        if (to < 0.0f) {
            to = 0.0f;
        }
        if (to > 1.0f) {
            to = 1.0f;
        }
        racer->unk10 = table_lerp_diff(obj->unk28, to, table, updateRate);
        if (racer->roll2 == 3 || racer->roll2 == 1) {
            racer->unk10 = -racer->unk10;
        }
        racer->unk56C = 1;
    }
    if (racer->unk56C == 0) {
        racer->roll2 = 0;
        racer->unk10 *= func_80049BEC(0.85f, updateRate);
        if (-0.1f < racer->unk10 && racer->unk10 < 0.1f) {
            racer->unk10 = 0.0f;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_04934);
#endif

#ifdef NON_MATCHING
static s32 func_m016_04E08(Racer *racer) {
    f32 x;
    f32 z;
    f32 big;
    s32 anim;

    x = racer->unk4;
    if (x < 0.0f) {
        x = -x;
    }
    z = racer->unk10;
    if (z < 0.0f) {
        z = -z;
    }
    big = x;
    if (x < z) {
        big = z;
    }
    if (big < 0.1f) {
        if (racer->unk1FA != 0) {
            anim = 0x10;
        } else if (racer->unk1F9 != 0 || racer->unk1F4 != 0) {
            anim = 0x12;
        } else if (racer->unk198 != 0) {
            anim = 0x12;
        } else {
            anim = func_80048E2C(0x10, 0x13);
        }
    } else if (x < z) {
        if (racer->unk10 < 0.0f) {
            anim = 9;
        } else {
            anim = 0xA;
        }
    } else if (racer->unk569 != 0) {
        anim = 3;
    } else if (big > 3.5f) {
        anim = 2;
    } else if (big > 1.75f) {
        anim = 1;
    } else {
        anim = 0;
    }
    return anim;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_04E08);
#endif

#ifdef NON_MATCHING
static void func_m016_04F78(Object *obj, Racer *racer, s32 anim, f32 frame) {
    u8 pose;

    if (racer->held != NULL) {
        anim = D_m016_07870[anim].alt2;
        pose = D_m016_07870[anim].pose;
    } else if (racer->unk1F4 != 0 && weapon_usable_by_character(racer)) {
        anim = D_m016_07870[anim].alt1;
        pose = D_m016_07870[anim].alt1Pose;
    } else if (weapon_usable_by_character(racer)) {
        anim = D_m016_07870[anim].alt2;
        pose = D_m016_07870[anim].pose;
    } else {
        anim = D_m016_07870[anim].normal;
        pose = D_m016_07870[anim].pose;
    }
    if (anim != obj->animationID) {
        obj_anim_set(obj, anim, frame);
        sAnimChanges++;
    }
    racer_pose_begin(obj, racer, D_m016_077E4, pose);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_04F78);
#endif

/* Picks a random idle animation (or the plain one while carrying, aiming or hurt) */
#define IDLE_ANIM(racer, anim)                            \
    if ((racer)->unk1FA != 0) {                           \
        anim = 0x10;                                      \
    } else if ((racer)->unk1F9 != 0 || (racer)->unk1F4 != 0) { \
        anim = 0x12;                                      \
    } else if ((racer)->unk198 != 0) {                    \
        anim = 0x12;                                      \
    } else {                                              \
        anim = func_80048E2C(0x10, 0x13);                 \
    }

/* The animation state machine: moves on from the current animation, and advances it */
#ifdef NON_MATCHING
static void func_m016_05120(Object *obj, Racer *racer, f32 speed, f32 updateRate) {
    f32 ax;
    f32 az;
    f32 big;
    f32 animSpeed;
    f32 newFrame;
    f32 frame;
    s32 anim;

    ax = racer->unk4;
    if (ax < 0.0f) {
        ax = -ax;
    }
    az = racer->unk10;
    if (az < 0.0f) {
        az = -az;
    }
    big = ax;
    if (ax < az) {
        big = az;
    }
    anim = obj->animationID;
    newFrame = obj->unk28;
    frame = obj->unk28;
    animSpeed = D_m016_07974[anim];
    switch (obj->animationID) {
        case 0:
        case 28:
        case 36:
            animSpeed *= big;
            if (racer->unk569 != 0) {
                anim = 3;
                break;
            }
            if (big < 0.1f) {
                IDLE_ANIM(racer, anim);
                newFrame = 0.0f;
                break;
            }
            if (ax < az) {
                if (racer->unk10 < 0.0f) {
                    anim = 9;
                } else {
                    anim = 10;
                }
                newFrame = 0.25f;
                break;
            }
            if (big > 1.75f) {
                anim = 1;
            }
            break;
        case 1:
        case 29:
        case 35:
            animSpeed *= big;
            if (ax < az) {
                if (racer->unk10 < 0.0f) {
                    anim = 9;
                } else {
                    anim = 10;
                }
                newFrame = 0.25f;
                break;
            }
            if (big < 1.75f) {
                anim = 0;
                break;
            }
            if (big > 3.5f) {
                anim = 2;
            }
            break;
        case 2:
        case 30:
        case 34:
            animSpeed *= big;
            if (ax < az) {
                if (racer->unk10 < 0.0f) {
                    anim = 9;
                } else {
                    anim = 10;
                }
                newFrame = 0.25f;
                break;
            }
            if (big < 3.5f) {
                anim = 1;
            }
            break;
        case 3:
        case 26:
            animSpeed *= big;
            if (big < 0.1f && speed <= racer->unk4) {
                IDLE_ANIM(racer, anim);
                newFrame = 0.0f;
                break;
            }
            if (ax < az) {
                if (racer->unk10 < 0.0f) {
                    anim = 9;
                } else {
                    anim = 10;
                }
                newFrame = 0.25f;
                break;
            }
            if (racer->unk569 == 0) {
                anim = 0;
            }
            break;
        case 4:
        case 44:
            animSpeed *= big;
            if (big < 0.1f) {
                anim = 0x16;
                newFrame = 0.0f;
            }
            break;
        case 5:
            if (obj->unk28 + animSpeed * updateRate > 0.75f) {
                obj->unk28 = 0.75f;
                animSpeed = 0.0f;
            }
            break;
        case 8:
            if (obj->unk28 == 1.0f) {
                anim = func_m016_04E08(racer);
                newFrame = 0.0f;
            }
            break;
        case 9:
        case 10:
        case 31:
        case 32:
        case 47:
        case 48:
            animSpeed *= az;
            if (big < 0.1f && racer->unk56C == 0) {
                IDLE_ANIM(racer, anim);
                newFrame = 0.0f;
                break;
            }
            if (az < ax) {
                if (racer->unk569 != 0) {
                    anim = 3;
                } else if (ax < 1.75f) {
                    anim = 0;
                } else if (ax < 3.5f) {
                    anim = 1;
                } else {
                    anim = 2;
                }
                newFrame = 0.25f;
            }
            break;
        case 11:
            if (racer->roll2 != 2) {
                anim = 0xE;
                newFrame = 0.0f;
            }
            break;
        case 49:
            if (racer->roll2 != 1) {
                anim = 0xE;
                newFrame = 0.0f;
            }
            break;
        case 12:
            if (racer->roll2 != 4) {
                if (big < 0.1f) {
                    anim = 0x16;
                } else {
                    anim = 4;
                }
                newFrame = 0.0f;
            }
            break;
        case 50:
            if (racer->roll2 != 3) {
                if (big < 0.1f) {
                    anim = 0x16;
                } else {
                    anim = 4;
                }
                newFrame = 0.0f;
            }
            break;
        case 13:
            if (gPlayerButtonsHeld & 0x10) {
                animSpeed += animSpeed;
            }
            if (obj->unk28 > 0.99f) {
                anim = 0xE;
            }
            newFrame = 0.0f;
            break;
        case 14:
        case 33:
            if (big > 0.1f) {
                anim = 4;
                newFrame = 0.0f;
            }
            break;
        case 15:
            if (racer->unk576 == 0) {
                anim = func_m016_04E08(racer);
                newFrame = 0.0f;
            }
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 27:
        case 45:
            if (obj->unk28 + animSpeed * updateRate > 1.0f) {
                if (racer->unk1FA != 0) {
                    obj->animationID = -1;
                    anim = 0x10;
                } else if (racer->unk1F9 != 0 || racer->unk1F4 != 0) {
                    obj->animationID = -1;
                    anim = 0x12;
                } else if (racer->unk198 != 0) {
                    obj->animationID = -1;
                    anim = 0x12;
                } else {
                    anim = func_80048E2C(0x10, 0x13);
                }
                newFrame = 0.0f;
            }
            if (big > 0.1f) {
                if (racer->unk569 == 0) {
                    anim = 0;
                } else {
                    anim = 3;
                }
                newFrame = 0.25f;
            }
            break;
        case 21:
        case 46:
            if (obj->unk28 > 0.98f) {
                if (racer->state == 1 || racer->state == 5) {
                    anim = 0xE;
                    newFrame = 0.0f;
                } else {
                    anim = func_m016_04E08(racer);
                    newFrame = 0.0f;
                }
            }
            break;
        case 22:
            if (big > 0.1f) {
                anim = 4;
                newFrame = 0.0f;
            }
            break;
        case 38:
            if (big > 0.85f || obj->velocity.y > 0.85f) {
                anim = 0x25;
            }
            break;
        case 37:
            animSpeed *= big;
            if (big < 0.7f && obj->velocity.y < 0.7f) {
                anim = 0x26;
            }
            break;
        case 43:
            if (racer->unk1FA == 0) {
                anim = func_m016_04E08(racer);
                newFrame = 0.0f;
            } else if (obj->unk28 < 0.53f && 0.53f <= obj->unk28 + animSpeed * updateRate) {
                D_m016_080DE = 1;
            }
            break;
        case 41:
            if (obj->unk28 + animSpeed * updateRate > 1.0f) {
                anim = 0x18;
                newFrame = 0.0f;
            }
            break;
        case 42:
            if (obj->unk28 + animSpeed * updateRate > 1.0f) {
                anim = 0x28;
                newFrame = 0.0f;
            }
            break;
        case 39:
            if (racer->unk19F == 0 && racer->unk10D == 0) {
                anim = func_m016_04E08(racer);
                newFrame = 0.0f;
            }
            break;
    }
    func_8001138C(obj, animSpeed, updateRate);
    anim_sound_events_play(obj, frame, animSpeed);
    func_m016_01D58(obj, racer, frame, animSpeed);
    func_m016_04F78(obj, racer, anim, newFrame);
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_05120);
#endif

/* Moves the character: gravity, slopes, the speeds turned into a velocity, the move and its collision */
#ifdef NON_MATCHING
static void func_m016_05BB8(Object *obj, Racer *racer, f32 updateRate) {
    f32 dx;
    f32 dz;
    s32 pad0;
    s32 pad1;
    Vec3f vec;
    f32 t; /* also 1 / updateRate */
    f32 sine;
    s32 pad2;
    s32 hit;
    s16 angle;
    Vec3s rot;

    obj->velocity.y -= (1.0f - racer->unk594) * (D_800F6DC4 * updateRate);
    racer->unk74 = obj->velocity.y;
    if (racer->state != 2 && racer->state != 10) {
        if (racer->unk50 < 0.0f) {
            t = -racer->unk50 - 0.3f;
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
        racer->unk4 += D_800F6DC4 * (racer->unk50 / (4.0f - t)) * updateRate;
    }
    racer->unk8 = 0.0f;
    obj->trans.rotation.y_rotation = racer->unk11C;
    if (racer->state == 4 || (racer->state == 3 && racer->unk14A != 0)) {
        angle = racer->unk128;
    } else {
        angle = racer->unk11C;
    }
    if (racer->unk13E != 0) {
        angle = racer->unk13C;
    }
    sine = func_80049C34(angle);
    t = func_80049C30(angle);
    if (racer->state != 12) {
        obj->velocity.x = racer->unk4 * sine;
        obj->velocity.z = racer->unk4 * t;
    }
    angle = racer->unk11C;
    if (racer->unk13E != 0) {
        angle = racer->unk13C;
    }
    obj->velocity.x += racer->unk10 * func_80049C30(angle);
    obj->velocity.z -= racer->unk10 * func_80049C34(angle);
    dx = obj->velocity.x;
    dz = obj->velocity.z;
    if (racer->unk149 != 0) {
        dx += racer->unk6C;
        dz += racer->unk70;
    }
    if (gPlayerInputLocked == 1) {
        dx *= 0.65f;
        dz *= 0.65f;
        if (racer->unk4 < -0.5f || racer->unk4 > 0.5f) {
            racer->unk4 *= 0.65f;
        } else {
            racer->unk4 = 0.0f;
        }
        if (racer->unk8 < -0.5f || racer->unk8 > 0.5f) {
            racer->unk8 *= 0.65f;
        } else {
            racer->unk8 = 0.0f;
        }
    }
    if (racer->unk10D != 0) {
        func_m022_02F38(obj, racer, updateRate);
    }
    if (racer->unk19F != 0) {
        func_m129_00CF8(obj, racer, updateRate);
    }
    func_80009A04(obj, dx * updateRate + racer->unk354, obj->velocity.y * updateRate,
                  dz * updateRate + racer->unk35C);
    t = 1.0f / updateRate;
    dx = (obj->trans.x_position - racer->unk3C) * t;
    dz = (obj->trans.z_position - racer->unk44) * t;
    hit = func_80034BB0(obj, racer, updateRate);
    obj->velocity.x = (obj->trans.x_position - racer->unk3C) * t;
    obj->velocity.y = (obj->trans.y_position - racer->unk40) * t;
    obj->velocity.z = (obj->trans.z_position - racer->unk44) * t;
    if (racer->unk534 != 0 && obj->velocity.y > 0.0f) {
        obj->velocity.y = 0.0f;
    } else if (racer->unk510 != NULL && racer->unk532 != 0) {
        obj->velocity.y = 0.0f;
    }
    if (racer->unk13E != 0) {
        angle = racer->unk13C;
    } else {
        angle = racer->unk11C;
    }
    rot.y_rotation = -angle;
    rot.x_rotation = -obj->trans.rotation.x_rotation;
    rot.z_rotation = -obj->trans.rotation.z_rotation;
    vec.x = dx; vec.y = 0.0f; vec.z = dz; func_80049394(&rot, &vec);
    dx = vec.x;
    if (racer->unk149 != 0) {
        racer->unk149 -= (s32) updateRate;
        if (racer->unk149 < 0) {
            racer->unk149 = 0;
        }
    } else {
        t = racer->unk4 - vec.z;
        if (t > 0.5f) {
            racer->unk4 -= t - 0.5f;
        }
        if (t < -0.5f) {
            racer->unk4 -= t + 0.5f;
        }
        t = racer->unk8 - dx;
        if (t > 0.5f) {
            racer->unk8 -= t - 0.5f;
        }
        if (t < -0.5f) {
            racer->unk8 -= t + 0.5f;
        }
    }
    if (hit) {
        if (racer->unk4 < -2.0f) {
            racer->unk4 = -2.0f;
        }
        if (racer->unk4 > 2.0f) {
            racer->unk4 = 2.0f;
        }
        if (racer->unk8 < -2.0f) {
            racer->unk8 = -2.0f;
        }
        if (racer->unk8 > 2.0f) {
            racer->unk8 = 2.0f;
        }
        if (racer->unk10 < -1.75f) {
            racer->unk10 = -1.75f;
        }
        if (racer->unk10 > 1.75f) {
            racer->unk10 = 1.75f;
        }
        racer->unk199 = 1;
    } else {
        racer->unk199 = 0;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_05BB8);
#endif

#ifdef NON_MATCHING
static void func_m016_06290(Object *obj, Racer *racer, s32 updateRate) {
    s16 i;
    s16 pitch;
    s16 yaw;

    if (racer->state != 10 && racer->state != 2) {
        pitch = racer->unk1C8 - obj->trans.rotation.x_rotation;
        if (pitch < -0x2AAA) {
            pitch = -0x2AAA;
        }
        if (pitch > 0x2AAA) {
            pitch = 0x2AAA;
        }
        yaw = racer->unk1C6 - obj->trans.rotation.y_rotation;
        if (yaw < -0x1000) {
            yaw = -0x1000;
        }
        if (yaw > 0x1000) {
            yaw = 0x1000;
        }
    } else {
        pitch = 0;
        yaw = 0;
    }
    for (i = 0; i < updateRate; i++) {
        racer->unk1D0 += (pitch - racer->unk1D0) >> 2;
        racer->unk1CE += (yaw - racer->unk1CE) >> 2;
    }
    if (racer->state != 10 && racer->unk1F5 > 0) {
        racer->unk1F5 -= updateRate;
        pitch = racer->unk1DA;
        /* each clamp on one line (IDO schedules by source line) */
        if (pitch < -14563.553f) { pitch = -0x38E3; } if (pitch > 14563.553f) { pitch = 0x38E3; }
        yaw = racer->unk1D8;
        if (yaw < -3640.8882f) { yaw = -0xE38; } if (yaw > 3640.8882f) { yaw = 0xE38; }
    } else {
        pitch = 0;
        yaw = 0;
    }
    for (i = 0; i < updateRate; i++) {
        racer->unk1DE += (pitch - racer->unk1DE) >> 4;
        racer->unk1DC += (yaw - racer->unk1DC) >> 4;
    }
    if (racer->unk1F4 != 0) {
        yaw = racer->unk1CE;
        pitch = racer->unk1D0;
        if (racer->unk1F6 != 0) {
            pitch += (racer->unk1F7 * func_8004966C(racer->unk1F6 << 12)) >> 8;
        }
        func_m016_03DB0(pitch, yaw, racer->unk1DE, racer->unk1DC);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_06290);
#endif

/* State 4: swimming */
#ifdef NON_MATCHING
static void func_m016_0650C(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    s32 roll;
    s32 pitch;
    f32 x;
    f32 v;
    s16 oldAngle;
    s32 i;
    f32 *t;

    racer->flags.byte[1] &= ~0xF;
    if (racer->unk2 == 0 || racer->unk60 < 45.0f) {
        racer->unk57C = obj->trans.y_position;
        racer->state = 3;
        racer->unk58E = 0;
        racer->unk594 = 0.0f;
    }
    player_turn_limit_update(obj, racer, &speed, angle, 0x5000, 0x6800, 5.0f, 3.0f);
    if (D_800A18B0->unkC & gPlayerButtonsPressed) {
        racer->unk57A = 0;
        racer->unk57C = obj->trans.y_position;
        obj->velocity.y += 12.0f;
        if (obj->velocity.y > 12.0f) {
            obj->velocity.y = 12.0f;
        }
        func_m016_04F78(obj, racer, 6, 0);
        player_voice_play(obj, 0);
    } else {
        obj->velocity.y = func_80049BEC(0.9f, updateRate) * obj->velocity.y;
    }
    if (racer->unk569 == 0) {
        speed = -speed;
    } else {
        angle += 0x8000;
    }
    if (racer->state == 4) {
        racer->unk19E = 1;
        racer->unk594 = racer->unk60 * 0.020833334f;
        if (racer->unk594 > 1.5f) {
            racer->unk594 = 1.5f;
        }
    }
    x = racer->unk4;
    if (x < 0.0f) {
        x = -x;
    }
    i = x;
    t = &D_m016_07DDC[i];
    v = t[0] + (t[1] - t[0]) * (x - i);
    oldAngle = racer->unk11C;
    racer->unk11C = angle_lerp(racer->unk11C, angle, 1.0f - func_80049BEC(1.0f - v, updateRate));
    racer->unk4 = (1.0f - func_80049BEC(0.985f, updateRate)) * (speed - racer->unk4) + racer->unk4;
    racer->unk10 *= func_80049BEC(0.85f, updateRate);
    if (-0.1f < racer->unk10 && racer->unk10 < 0.1f) {
        racer->unk10 = 0.0f;
    }
    x = racer->unk4;
    if (x < 0.0f) {
        x = -x;
    }
    i = x;
    t = &D_m016_07DFC[i];
    v = (x - i) * (t[1] - t[0]) + t[0];
    racer->unk128 = angle_lerp(racer->unk128, racer->unk11C, 1.0f - func_80049BEC(1.0f - v, updateRate));
    oldAngle = racer->unk11C - oldAngle;
    x = racer->unk4;
    if (x < 0.0f) {
        x = -x;
    }
    roll = oldAngle * 1.5f * x;
    if (roll < -0x1000) {
        roll = -0x1000;
    }
    if (roll > 0x1000) {
        roll = 0x1000;
    }
    pitch = racer->unk4 * 512.0f;
    if (pitch > 0) {
        pitch = 0;
    }
    if (pitch < -0x2000) {
        pitch = -0x2000;
    }
    racer->unk12C = angle_lerp(racer->unk12C, roll, 1.0f - func_80049BEC(0.925f, updateRate));
    racer->unk12A = angle_lerp(racer->unk12A, pitch, 1.0f - func_80049BEC(0.925f, updateRate));
    *D_m016_080D8 = 4;
    D_m016_080D8++;
    *D_m016_080D8 = racer->unk12C;
    D_m016_080D8++;
    *D_m016_080D8 = 0;
    D_m016_080D8++;
    *D_m016_080D8 = racer->unk12A;
    D_m016_080D8++;
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_0650C);
#endif

/* State 10: dead */
#ifdef NON_MATCHING
static void func_m016_06AEC(Object *obj, Racer *racer, f32 speed, f32 angle, f32 updateRate) {
    f32 damp;
    Vec3f *p;
    Object *spawned;
    M016SpawnEntry entry;
    s32 pad;
    s32 idx;
    M016ModelInst *inst;
    M016Model *model;

    if (D_800A4FD0 == 1) {
        func_80048518(10);
    }
    obj->interactObj->hitPoints = 0;
    damp = func_80049BEC(0.85f, updateRate);
    racer->unk4 *= damp;
    if (-0.1f < racer->unk4 && racer->unk4 < 0.1f) {
        racer->unk4 = 0.0f;
    }
    racer->unk10 *= damp;
    if (-0.1f < racer->unk10 && racer->unk10 < 0.1f) {
        racer->unk10 = 0.0f;
    }
    racer->flags.byte[1] &= ~0xF;
    if (racer->unk2 == 0) {
        racer->unk594 *= damp;
    } else {
        racer->unk594 = 0.875f;
        obj->velocity.y *= func_80049BEC(0.8f, updateRate);
        if (racer->unk18A < 0xF0) {
            racer->unk18A++;
        }
        if (racer->unk18B <= (s32) updateRate) {
            inst = MODEL_INST(obj, obj->modelIndex);
            model = inst->model;
            idx = func_80048E2C(0, model->unk2F - 1);
            p = &inst->vertices[idx];
            if (p->y < racer->unk5C) {
                entry.objectID = 0xBA;
                entry.size = 0x14;
                entry.unk3 = 0;
                entry.x = p->x;
                entry.y = p->y;
                entry.z = p->z;
                entry.angle = func_80048E2C(-0x8000, 0x7FFF);
                entry.unkC = func_80048E2C(0, 0x23) * 0.01f;
                entry.unk10 = racer->unk5C;
                spawned = func_80005F14(&entry, 1);
                if (spawned != NULL) {
                    spawned->level_entry = NULL;
                }
            }
            racer->unk18B = racer->unk18A >> 2;
        } else {
            racer->unk18B -= (s32) updateRate;
        }
    }
    racer->unk19C = 0;
    if (obj->unk28 == 1.0f) {
        func_80033920(obj, racer);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_06AEC);
#endif

#ifdef NON_MATCHING
/* Matches in the probe; C only under NON_MATCHING until its caller 00000 matches (the last call's
   0.0f, not 0, gives the shared 0.0f $f2 as in the ROM) */
/* State 6 */
static void func_m016_06E24(Object *obj, Racer *racer, f32 updateRate) {
    s32 rate;
    s32 v;
    f32 f;

    racer->unk4 = 0.0f;
    racer->unk10 = 0.0f;
    racer->unk8 = 0.0f;
    obj->velocity.x = 0.0f;
    obj->velocity.z = 0.0f;
    rate = updateRate;
    if (racer->unk579 > 0) {
        v = racer->unk579 - rate;
        if (v >= 0) {
            racer->unk11C -= racer->unk17C * rate;
            racer->unk579 -= rate;
        } else if (v < 0) {
            racer->unk11C -= racer->unk17C * racer->unk579;
            racer->unk579 = 0;
        }
        if (racer->unk579 == 0) {
            obj->velocity.y = 0.0f;
            racer->unk194 = 1;
            func_80034018(racer);
            func_m016_04F78(obj, racer, 0x28, 0);
        }
    } else if (racer->unk578 > 0 && racer->unk579 == 0) {
        v = racer->unk578 - rate;
        if (v >= 0) {
            racer->unk578 -= rate;
            racer->unk4 = racer->unk170;
            racer->unk10 = racer->unk174;
        } else if (v < 0 && racer->unk578 > 0) {
            f = racer->unk578;
            racer->unk578 = 0;
            racer->unk4 = racer->unk170 * f / updateRate;
            racer->unk10 = racer->unk174 * f / updateRate;
        }
        if (racer->unk578 == 0) {
            racer->unk194 = 0;
            func_80034018(racer);
        }
    }
    if (racer->unk184 == 0) {
        racer->unk194 = 0;
        func_80034018(racer);
        racer->unk57C = obj->trans.y_position;
        func_m016_04F78(obj, racer, 0x18, 0);
        racer->state = 3;
        racer->unk58E = 0;
    } else if (racer->unk578 == 0 && (racer->pose->unk5 & racer->unk532)) {
        racer->unk149 = 8;
        racer->unk6C = func_80049C34(racer->unk11C) * 2.0f;
        racer->unk70 = func_80049C30(racer->unk11C) * 2.0f;
        func_m016_04F78(obj, racer, func_m016_04E08(racer), 0.0f);
        racer->state = 0;
        racer->unk58E = 0;
    } else if (racer->unk578 == 0 && (racer->pose->unk6 & racer->unk532)) {
        racer->state = 8;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_06E24);
#endif

/* State 8 */
#ifdef NON_MATCHING
static void func_m016_070A4(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    f32 zero;
    f32 s1;
    f32 s2;
    f32 c2;
    f32 c1;

    zero = 0.0f;
    racer->unk4 = zero;
    racer->unk8 = zero;
    racer->unk10 = zero;
    obj->velocity.x = zero;
    obj->velocity.z = zero;
    if (racer->pose->unk5 & racer->unk532) {
        racer->unk149 = 8;
        racer->unk6C = func_80049C34(racer->unk11C) * 2.0f;
        racer->unk70 = func_80049C30(racer->unk11C) * 2.0f;
        func_m016_04F78(obj, racer, func_m016_04E08(racer), zero);
        racer->state = 0;
    } else {
        if (speed == zero) {
            racer->unk578 = 1;
        }
        c1 = func_80049C30(angle);
        c2 = func_80049C30(racer->unk11C);
        s2 = func_80049C34(racer->unk11C);
        s1 = func_80049C34(angle);
        if ((racer->unk578 != 0 && speed > 2.0f && s1 * s2 + c2 * c1 > 0.9f) || (D_800A18B0->unkC & gPlayerButtonsPressed)) {
            obj->velocity.y = func_80097140(2.0f * D_800F6DC4 * (racer->unk178 - obj->trans.y_position)) +
                              (1.0f + 0.5f * D_800F6DC4 * updateRate);
            racer->unk4 = -1.0f;
            func_m016_04F78(obj, racer, 0x29, zero);
            racer->state = 7;
        } else if ((racer->unk578 != 0 && speed > 2.0f && s1 * s2 + c2 * c1 < -0.9f) ||
                   (D_800A18B0->unk10 & gPlayerButtonsPressed) || !(racer->pose->unk6 & racer->unk532)) {
            racer->unk57C = obj->trans.y_position;
            func_m016_04F78(obj, racer, 0x18, zero);
            racer->state = 3;
            racer->unk58E = 0;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016/ro", func_m016_070A4);
#endif

#ifdef NON_MATCHING
static void func_m016_07344(Object *obj, Racer *racer, f32 updateRate) {
    if (racer->pose->unk5 & racer->unk532) {
        func_m016_04F78(obj, racer, func_m016_04E08(racer), 0);
        racer->state = 0;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_07344);
#endif

/* State 12 */
#ifdef NON_MATCHING
static void func_m016_073A8(Object *obj, Racer *racer, f32 speed, s16 angle, f32 updateRate) {
    s32 v;
    f32 f;
    s32 rate;

    rate = updateRate;
    v = rate;
    racer->unk4 = 0.0f;
    racer->unk8 = 0.0f;
    racer->unk10 = 0.0f;
    obj->velocity.x = 0.0f;
    obj->velocity.z = 0.0f;
    if (obj->animationID != 0x28) {
        func_m016_04F78(obj, racer, 0x28, 0.0f);
    }
    v = racer->unk578 - v;
    if (racer->unk578 != 0) {
        if (v >= 0) {
            obj->velocity.x = racer->unk170;
            obj->velocity.z = racer->unk174;
            racer->unk11C -= racer->unk17C * rate;
            racer->unk578 -= rate;
        } else if (v < 0 && racer->unk578 > 0) {
            f = racer->unk578;
            obj->velocity.x = racer->unk170 * f / updateRate;
            obj->velocity.z = racer->unk174 * f / updateRate;
            racer->unk11C -= racer->unk17C * racer->unk578;
            racer->unk578 = 0;
        }
    } else if (racer->unk578 == 0) {
        if (racer->pose->unk6 & racer->unk532) {
            func_m016_04F78(obj, racer, 0x2A, 0.0f);
            racer->state = 8;
            racer->unk57C = racer->unk178;
            obj->velocity.y = 0.0f;
            racer->unk14A = 0;
            racer->unk14B = 0;
            racer->unk594 = 0.0f;
        } else if (racer->pose->unk5 & racer->unk532) {
            racer->state = 0;
            func_m016_04F78(obj, racer, func_m016_04E08(racer), 0.0f);
        } else if (racer->unk4AC < racer->unk178 || racer->unk533 != 0) {
            racer->unk57C = obj->trans.y_position;
            func_m016_04F78(obj, racer, 0x18, 0.0f);
            racer->state = 3;
            racer->unk58E = 0;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/modules/m016", func_m016_073A8);
#endif
