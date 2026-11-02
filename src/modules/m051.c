/*
 * Runlink module 51: another boss driven like a player character. It
 * shares most of its code with modules 50 and 52 (see m050.c): the jump and
 * slow-down helpers, the dying helper and the movement and landing code are
 * the same; its animation tables are smaller, it has no armed animations,
 * and its head and neck commands differ.
 */
#include "module.h"

/* The flag word at 0x540 of a player's state block (as 2BAA0.c's RacerFlags) */
typedef struct M051Flags {
    u32 unk31 : 1;
    u32 unk30 : 1;
    u32 weaponDir : 2;
    u32 unk27 : 1;
    u32 useOverride : 1;
    u32 unk25 : 1;
    u32 unk24 : 1;
    u32 unk8 : 16;
    u32 unk7 : 1;
    u32 unk0 : 7;
} M051Flags;

/* What a player's held items point at */
typedef struct M051Item {
    /* 0x00 */ u8 pad0[0x93];
    /* 0x93 */ u8 heldBy[4]; /* per player */
} M051Item;

/* The level entry of the object at M051Player.arena */
typedef struct M051ArenaEntry {
    LevelObjectEntryCommon common;
    /* 0x0A */ u8 padA[2];
    /* 0x0C */ u16 radius;
} M051ArenaEntry;

/* The boss's state block (obj->unk68): a player's */
typedef struct M051Player {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x4 - 0x2];
    /* 0x004 */ f32 unk4;
    /* 0x008 */ f32 unk8;
    /* 0x00C */ u8 padC[0x10 - 0xC];
    /* 0x010 */ f32 speed;
    /* 0x014 */ u8 pad14[0x78 - 0x14];
    /* 0x078 */ u8 unk78[0xA4];
    /* 0x11C */ s16 unk11C; /* facing */
    /* 0x11E */ u8 pad11E[0x13E - 0x11E];
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 pad13F[0x14A - 0x13F];
    /* 0x14A */ s8 unk14A;
    /* 0x14B */ u8 pad14B[0x18E - 0x14B];
    /* 0x18E */ u8 unk18E;
    /* 0x18F */ u8 pad18F[0x19C - 0x18F];
    /* 0x19C */ s8 unk19C;
    /* 0x19D */ u8 pad19D[0x1A8 - 0x19D];
    /* 0x1A8 */ M051Item *items[6];
    /* 0x1C0 */ u8 pad1C0[0x1C6 - 0x1C0];
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
    /* 0x1E4 */ u8 pad1E4[0x1EC - 0x1E4];
    /* 0x1EC */ s32 unk1EC;
    /* 0x1F0 */ u8 pad1F0[0x1F4 - 0x1F0];
    /* 0x1F4 */ u8 unk1F4;
    /* 0x1F5 */ u8 pad1F5;
    /* 0x1F6 */ s8 unk1F6;
    /* 0x1F7 */ s8 unk1F7;
    /* 0x1F8 */ u8 unk1F8;
    /* 0x1F9 */ u8 pad1F9[0x240 - 0x1F9];
    /* 0x240 */ s16 commands[0x538 - 0x240 >> 1]; /* joint angle commands (sCommand) */
    /* 0x538 */ u8 unk538;
    /* 0x539 */ u8 pad539[0x53D - 0x539];
    /* 0x53D */ u8 unk53D;
    /* 0x53E */ u8 pad53E[0x540 - 0x53E];
    /* 0x540 */ M051Flags flags;
    /* 0x544 */ f32 unk544;
    /* 0x548 */ f32 unk548;
    /* 0x54C */ f32 unk54C;
    /* 0x550 */ u8 pad550[0x568 - 0x550];
    /* 0x568 */ u8 state; /* 0x20 set: on the ground */
    /* 0x569 */ u8 pad569[0x56B - 0x569];
    /* 0x56B */ s8 unk56B;
    /* 0x56C */ u8 pad56C[0x576 - 0x56C];
    /* 0x576 */ u8 unk576;
    /* 0x577 */ s8 unk577;
    /* 0x578 */ u8 pad578[0x57C - 0x578];
    /* 0x57C */ f32 jumpStartY;
    /* 0x580 */ u8 pad580[0x585 - 0x580];
    /* 0x585 */ s8 weapon;
    /* 0x586 */ u8 pad586[0x5BC - 0x586];
    /* 0x5BC */ void *unk5BC;
} M051Player;

/* The object whose level entry gives the arena's centre and radius (state block 0xFC) */
#define ARENA(p) (*(Object **) ((u8 *) (p) + 0xFC))

/* What func_80047C1C returns: the character's weapons */
typedef struct M051Weapons {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 owned; /* bit per weapon */
    /* 0x0C */ u8 padC[0x6F - 0xC];
    /* 0x6F */ u8 unk6F;
    /* 0x70 */ u8 weapon; /* the current one */
} M051Weapons;

/* What get_settings returns (only the field used here) */
typedef struct M051Settings {
    /* 0x000 */ u8 pad0[0x534];
    /* 0x534 */ u8 buttonWeapons[4];
} M051Settings;

typedef struct M051ModelData {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 unk11;
} M051ModelData;

typedef struct M051Pos {
    /* 0x00 */ u8 pad0[0x70];
    /* 0x70 */ f32 y;
} M051Pos;

typedef struct M051ModelInstance {
    /* 0x00 */ M051ModelData *model;
    /* 0x04 */ u8 pad4[0xA - 0x4];
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 matrixIndex;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ void *matrices[2];
    /* 0x18 */ u8 pad18[0x70 - 0x18];
    /* 0x70 */ M051Pos *unk70;
} M051ModelInstance;

typedef struct M051Shadow {
    /* 0x00 */ u8 pad0[0xE];
    /* 0x0E */ s8 unkE;
} M051Shadow;

/* Data from a header that several player-like modules share */
typedef struct M051Shape {
    /* 0x00 */ f32 unk0;
    /* 0x04 */ f32 unk4;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
} M051Shape;

typedef struct M051ShapeSet {
    /* 0x00 */ M051Shape **shapes;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08 */ s32 count;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 unk10;
} M051ShapeSet;

/* A move's animations */
typedef struct M051Move {
    /* 0x0 */ u8 anim;
    /* 0x1 */ u8 animArmed; /* when the character can use its weapon (the one this module plays) */
    /* 0x2 */ u8 unk2;
    /* 0x3 */ u8 unk3;
    /* 0x4 */ u8 unk4;
} M051Move;

typedef struct M051Event {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ char name[26];
} M051Event;

typedef struct M051Rates {
    /* 0x0 */ f32 unk0;
    /* 0x4 */ f32 unk4;
    /* 0x8 */ f32 unk8;
    /* 0xC */ f32 unkC;
} M051Rates;

extern s32 gPlayerButtonsHeld;
extern s32 gPlayerButtonsPressed;
extern s32 gPlayerInputLocked;
extern u32 *D_800A18B0; /* button masks: [1] next weapon, [2] previous weapon, [3] jump */
extern u32 D_800A197C[4]; /* the buttons that pick a weapon */
extern u8 D_800A5070;
extern u8 D_800A5078;
extern f32 D_800F6DC4; /* gravity */

extern M051Weapons *func_80047C1C(s32 player, s32 character);
extern M051Settings *get_settings(void);
extern void player_input_lock_set(s32 locked);
extern void player_input_latch(s32 player);
extern void func_80034018(M051Player *p);
extern void music_play(u8 seqID);
extern void func_m006_015E0(s32);
extern void player_drop_held_object(Object *obj);
extern void player_items_unlink(M051Player *p);
extern s32 weapon_select(Object *obj, M051Player *p, s32 weapon);
extern void func_80038324();
extern void weapon_select_prev();
extern void func_m107_00D08(void *, f32, f32, s32);
extern void func_8003D21C(M051ModelInstance *, M051ModelData *, Object *, f32, f32);
extern s32 func_8000BC28(Object *, M051ModelInstance *, M051ModelData *, s32, u8 *, f32 *, f32 *, f32 *);
extern void func_800221F0(Object *, M051ModelInstance *, ObjectShading *, void *);
extern void obj_tex_animate(Object *, M051ModelData *, M051ModelInstance *, s32);
extern s32 func_80037550(Object *obj, M051Player *p, f32 updateRate);
extern s32 func_80033F4C(s32 weapon);
extern void func_800116BC(Object *obj, f32 arg1);
extern f32 func_80049BEC(f32 base, s32 exponent);
extern void player_voice_play(Object *obj, s32 arg1);
extern s32 func_8003AABC(Object *obj, M051Player *p, s32, s32, s32, s32, s16 *, s16 *, f32);
extern void cam_fov_zoom_set(s32 player, s32 arg1);
extern s16 angle_lerp(s16 from, s16 to, f32 t);
extern s32 func_8004966C(s16 angle);
extern void func_80048518(s32 arg0);
extern void func_80033920(Object *obj, M051Player *p);
extern s32 weapon_usable_by_character(M051Player *p);
extern void func_8002B1D4(Object *obj, f32 frame, f32 speed);
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz);
extern f32 func_80097140(f32 x); /* square root */
extern s32 func_80082D80(f32 x, f32 z, s32 flags, f32 ***heights);
extern s32 func_8001B078(f32 x, f32 z, s32 flags, f32 ***heights);

static M051Shape sShapes[4] = {
    { 0.0f, 18.0f, -9.0f, 18.0f, 2, 1 },
    { 0.0f, 18.0f, 9.0f, 18.0f, 2, 1 },
    { -8.0f, 22.0f, -2.0f, 22.0f, 2, 1 },
    { 8.0f, 22.0f, -2.0f, 22.0f, 2, 1 },
};

static M051Shape *sShapeList[4] = { &sShapes[0], &sShapes[1], &sShapes[2], &sShapes[3] };

static M051ShapeSet sShapeSets[2] = {
    { &sShapeList[0], 0x00, 0x03, 0x00, 0x00, 5, 36.0f, 54.0f },
    { &sShapeList[2], 0x00, 0x03, 0x00, 0x00, 5, 44.0f, 60.0f },
};

static M051Move sMoves[29] = {
    { 0x00, 0x00, 0x00, 0, 0 }, { 0x01, 0x01, 0x01, 0, 0 }, { 0x02, 0x02, 0x02, 0, 0 }, { 0x03, 0x03, 0x03, 0, 0 },
    { 0x04, 0x04, 0x04, 0, 0 }, { 0x05, 0x05, 0x05, 0, 0 }, { 0x06, 0x06, 0x06, 0, 0 }, { 0x07, 0x07, 0x07, 0, 0 },
    { 0x08, 0x08, 0x08, 0, 0 }, { 0x09, 0x12, 0x09, 0, 0 }, { 0x0A, 0x12, 0x0A, 0, 0 }, { 0x0B, 0x12, 0x0B, 0, 0 },
    { 0x0C, 0x12, 0x0C, 0, 0 }, { 0x0D, 0x12, 0x0D, 0, 0 }, { 0x0E, 0x0E, 0x0E, 0, 0 }, { 0x0F, 0x0F, 0x0F, 0, 0 },
    { 0x10, 0x10, 0x10, 0, 0 }, { 0x11, 0x11, 0x11, 0, 0 }, { 0x09, 0x12, 0x09, 0, 0 }, { 0x13, 0x13, 0x13, 0, 0 },
    { 0x14, 0x14, 0x14, 0, 0 }, { 0x15, 0x15, 0x15, 0, 0 }, { 0x16, 0x16, 0x16, 0, 0 }, { 0x17, 0x17, 0x17, 0, 0 },
    { 0x18, 0x18, 0x18, 0, 0 }, { 0x19, 0x19, 0x19, 0, 0 }, { 0x1A, 0x1A, 0x1A, 0, 0 }, { 0x1B, 0x1B, 0x1B, 0, 0 },
    { 0x1C, 0x1C, 0x1C, 0, 0 },
};

/* Animation speeds, per animation */
static f32 sAnimSpeeds[53] = {
    0.014f, 0.011f,  0.007f,  0.014f, 0.012f,  0.02f,   0.0167f, 0.01f,   0.025f,  0.005f,  0.0075f,
    0.005f, 0.005f,  0.02f,   0.02f,  0.005f,  0.01f,   0.0334f, 0.005f,  0.01f,   0.01f,   0.02f,
    0.01f,  0.0067f, 0.015f,  0.0175f, 0.02f,  0.015f,  0.012f,  0.02f,   0.021f,  0.012f,  0.025f,
    0.015f, 0.015f,  0.015f,  0.015f, 0.015f,  0.02f,   0.005f,  0.02f,   0.025f,  0.015f,  0.022f,
    0.012f, 0.017f,  0.01f,   0.0067f, 0.02f,  0.05f,   0.05f,   0.02f,   0.0067f,
};

static s16 sUnusedShorts[16] = { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 };

static s32 sUnusedInts[6] = { 2, 4, 2, 8, 16, 64 };

static M051Event sScoreEvents[13] = {
    { 500, -250, 0xFF06, "Player hit (soft)" },
    { 1000, -500, 0xFE0C, "Player hit (normal)" },
    { 2000, -1000, 0xFC18, "Player hit (hard)" },
    { 400, 0, 0x8032, "Player Fired" },
    { 800, 0, 0x8064, "Player Threw Grenade" },
    { 0, 250, 0x8032, "Enemy hit (soft)" },
    { 0, 500, 0x8064, "Enemy hit (normal)" },
    { 0, 1000, 0x80C8, "Enemy hit (hard)" },
    { 0, 1500, 0x8190, "Killed Enemy" },
    { -4000, 0, 0x07D0, "Player Saved a Bear" },
    { 2000, -1000, 0xFC18, "Player hit a Bear" },
    { 4000, -2000, 0xF830, "Player Killed a Bear" },
    { 1000, 0, 0xFE0C, "Enemy killed a Bear" },
};

static M051Rates sRates[19] = {
    { 0.004f, 0.8f }, { 0.007f, 0.85f }, { 0.01f, 0.85f }, { 0.004f, 0.5f }, { 0.01f, 0.5f },
    { 0.01f, 0.5f },  { 0.01f, 0.5f },   { 0.01f, 0.5f },  { 0.01f, 0.5f },  { 0.01f, 0.5f },
    { 0.004f, 0.5f }, { 0.004f, 0.8f },  { 0.004f, 0.8f }, { 0.004f, 0.84f }, { 0.004f, 0.8f },
    { 0.004f, 0.8f }, { 0.004f, 0.8f },  { 0.004f, 0.8f }, { 0.004f, 0.8f },
};

static s16 *sCommand;  /* where the next joint angle command goes */
static s16 sHitTimer;  /* the state block's unk577, for the turning code */

static void func_m051_008F0(Object *obj, M051Player *p, f32 updateRate);
static void func_m051_00DC0(Object *obj, M051Player *p, f32 updateRate);
static void func_m051_00E98(Object *obj, M051Player *p, f32 updateRate);
static void func_m051_00F44(M051Player *p, s16 pitch, s16 yaw);
static void func_m051_01038(s16 aimPitch, s16 aimYaw);
static s32 func_m051_010D4(M051Player *p);
static void func_m051_01118(Object *obj, M051Player *p, s32 move, f32 frame);
static void func_m051_01170(Object *obj, M051Player *p, f32 updateRate);
static void func_m051_0134C(Object *obj, M051Player *p, f32 updateRate);

void func_m051_00000(Object *obj, s32 updateRate) {
    M051Player *p;
    s32 k;
    UNUSED s32 pad94;
    UNUSED s32 pad90;
    M051ModelData *model;
    M051ModelInstance *inst;
    M051Weapons *weapons;
    s32 prevWeapon;
    s32 i;
    M051Settings *settings;
    f32 height;

    p = obj->unk68;
    weapons = func_80047C1C(p->player, p->character);
    sCommand = p->commands;
    obj->particleEmittersEnabled = 0;
    p->unk1F4 = 15;
    p->unk1F8 = 0;
    p->unk14A = 0;
    if (p->unk18E != 0 || p->state == 0x2A) {
        player_input_lock_set(1);
    }
    if (p->state != 0x2A) {
        p->state |= 0x20;
        if (p->unk1E2 < -0x800) {
            p->unk19C += updateRate * 8;
            if (p->unk19C > 40) {
                p->unk19C = 40;
            }
        } else if (p->unk1E2 > 0x800) {
            p->unk19C += updateRate * 8;
            if (p->unk19C > 40) {
                p->unk19C = 40;
            }
        } else {
            p->unk19C -= updateRate * 8;
            if (p->unk19C < 0) {
                p->unk19C = 0;
            }
        }
    } else {
        p->unk1E2 -= p->unk1E2 >> 2;
        p->unk19C -= updateRate * 8;
        if (p->unk19C < 0) {
            p->unk19C = 0;
        }
    }
    if (p->state != 0x23) {
        p->jumpStartY = obj->trans.y_position;
    }
    sHitTimer = p->unk577;
    if (p->unk56B != 0) {
        player_input_lock_set(1);
        p->unk56B -= updateRate;
        if (p->unk56B < 0) {
            p->unk56B = 0;
        }
    }
    if (p->unk577 != 0) {
        player_input_lock_set(1);
        p->unk577 -= updateRate;
        if (p->unk577 < 0) {
            p->unk577 = 0;
        }
    }
    player_input_latch(p->player);
    func_80034018(p);
    if (p->unk4 != 0.0f) {
        p->unk4 = 0.0f;
    }
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
    if (p->unk8 < -20.0f) {
        p->unk8 = -20.0f;
    }
    if (p->unk8 > 20.0f) {
        p->unk8 = 20.0f;
    }
    if (obj->interactObj->hitPoints < 0x100 && p->state != 0x2A) {
        if (D_800A5070 != 0) {
            obj->interactObj->hitPoints = 0x2200;
        } else {
            obj->interactObj->hitPoints = 0;
            music_play(0x38);
            p->state = 0x2A;
            gPlayerButtonsHeld = 0;
            gPlayerButtonsPressed = 0;
            p->unk53D = 0;
            func_m006_015E0(1);
            player_drop_held_object(obj);
            player_items_unlink(p);
            func_m051_01118(obj, p, 0x17, 0.0f);
        }
        p->unk19C = 0;
    }
    switch (p->state & ~0x20) {
        case 0:
            func_m051_008F0(obj, p, updateRate);
            break;
        case 3:
            func_m051_00DC0(obj, p, updateRate);
            break;
        case 10:
            func_m051_00E98(obj, p, updateRate);
            break;
    }
    if (p->unk1F6 != 0) {
        p->unk1F6 -= updateRate;
        if (p->unk1F6 < 0) {
            p->unk1F6 = 0;
        }
    }
    if (D_800A5078 != 0) {
        weapons->unk6F = 15;
        weapons->owned = 0x7FFF;
    }
    p->flags.weaponDir = 0;
    if (weapons->unk6F >= 2) {
        prevWeapon = weapons->weapon;
        if (p->weapon != weapons->weapon && gPlayerInputLocked == 0) {
            weapon_select(obj, p, p->weapon);
        } else if (D_800A18B0[1] & gPlayerButtonsPressed) {
            func_80038324(obj, p);
        } else if (D_800A18B0[2] & gPlayerButtonsPressed) {
            weapon_select_prev(obj, p);
        } else {
            i = 0;
            settings = get_settings();
            for (; i < 4; i++) {
                if ((D_800A197C[i] & gPlayerButtonsPressed) && weapon_select(obj, p, settings->buttonWeapons[i])) {
                    break;
                }
            }
        }
        if (prevWeapon != weapons->weapon) {
            p->unk538 = 0;
            p->unk53D = 0;
            p->weapon = weapons->weapon;
        }
    }
    func_m051_01170(obj, p, updateRate);
    func_m051_0134C(obj, p, updateRate);
    if (p->unk5BC != NULL) {
        if ((p->state & ~0x20) == 1) {
            func_m107_00D08(p->unk5BC, 1.0f, 0.75f, 120);
        } else {
            func_m107_00D08(p->unk5BC, 1.0f, 1.0f, 120);
        }
    }
    *sCommand = 0x1000;
    sCommand++;
    inst = obj->modelInstances[obj->modelIndex];
    if (inst != NULL && inst->unkA != 0) {
        model = inst->model;
        func_8003D21C(inst, model, obj, 1.0f, 0.0f);
        p->flags.useOverride = func_8000BC28(obj, inst, model, weapons->weapon, p->unk78, &p->unk544, &p->unk548,
                                             &p->unk54C);
        func_800221F0(obj, inst, obj->shading, inst->matrices[inst->matrixIndex]);
        if (model->unk11 > 0) {
            obj_tex_animate(obj, model, inst, inst->unkA);
        }
        inst->unkA = 0;
        height = inst->unk70->y + 4.0f - obj->trans.y_position;
        if (height >= 64.0f) {
            height = 63.9f;
        }
        ((M051Shadow *) obj->shadow)->unkE = height * 4.0f;
    }
    if (func_80033F4C(func_80037550(obj, p, updateRate))) {
        if (obj->animationID == 0xE) {
            func_800116BC(obj, 0);
        } else {
            func_m051_01118(obj, p, 0xE, 0.0f);
        }
        p->unk577 = 40;
    }
    obj_spawn_particle(obj, updateRate);
    if (gPlayerButtonsPressed & 0x10) {
        if (p->items[5] != NULL) {
            k = 6;
            while (k--) {
                if (p->items[k] != NULL) {
                    p->items[k]->heldBy[p->player] = 0;
                    p->items[k] = NULL;
                }
            }
        }
    }
}

/* On the ground: jumping, speeding up and slowing down, turning and aiming */
static void func_m051_008F0(Object *obj, M051Player *p, f32 updateRate) {
    M051Weapons *weapons;
    s32 prevFacing;
    s16 aimYaw;
    s16 aimPitch;
    f32 rate;
    s32 facing;

    weapons = func_80047C1C(p->player, p->character);
    p->unk576 = 0;
    p->unk13E = 0;
    if (weapons->weapon != 7 && (D_800A18B0[3] & gPlayerButtonsPressed)) {
        p->jumpStartY = obj->trans.y_position;
        p->state = 0x23;
        obj->velocity.y += 10.0f;
        func_m051_01118(obj, p, 5, 0.0f);
        player_voice_play(obj, 0);
    } else if (gPlayerButtonsHeld & 2) {
        p->speed -= 0.2f * updateRate;
        if (p->speed < -3.0f) {
            p->speed = -3.0f;
        }
    } else if (gPlayerButtonsHeld & 1) {
        p->speed += 0.2f * updateRate;
        if (p->speed > 3.0f) {
            p->speed = 3.0f;
        }
    } else {
        p->speed *= func_80049BEC(0.95f, updateRate);
        if (p->speed > -0.1f && p->speed < 0.1f) {
            p->speed = 0.0f;
        }
    }
    prevFacing = p->unk11C & 0xFFFF;
    p->unk11C = obj->trans.rotation.y_rotation;
    if (sHitTimer == 0) {
        if (func_8003AABC(obj, p, 0xE38, 0xE38, -0x2AAA, 0x2AAA, &aimYaw, &aimPitch, updateRate)) {
            cam_fov_zoom_set(p->player, p->unk1EC);
        }
    } else {
        aimYaw = p->unk1DC;
        aimPitch = p->unk1DE - p->unk1E2;
    }
    facing = p->unk11C & 0xFFFF;
    p->unk11C = prevFacing;
    if (facing < prevFacing && prevFacing - facing >= 0x8000) {
        facing += 0x10000;
    }
    if (prevFacing < facing && facing - prevFacing >= 0x8000) {
        prevFacing += 0x10000;
    }
    if (prevFacing < facing) {
        if (facing - prevFacing > 0x2800) {
            facing = prevFacing + 0x2800;
        }
    } else if (prevFacing - facing > 0x2800) {
        facing = prevFacing - 0x2800;
    }
    obj->trans.rotation.y_rotation = facing;
    rate = 1.0f - func_80049BEC(0.8333f, updateRate);
    p->unk1E0 = angle_lerp(p->unk1E0, aimPitch, rate);
    aimPitch += p->unk1E2;
    p->unk1DC = angle_lerp(p->unk1DC, aimYaw, rate);
    p->unk1DE = angle_lerp(p->unk1DE, aimPitch, rate);
    p->unk1CE = angle_lerp(p->unk1CE, aimYaw, rate);
    p->unk1D0 = angle_lerp(p->unk1D0, aimPitch, rate);
    p->unk1C6 = p->unk1CE + obj->trans.rotation.y_rotation;
    p->unk1C8 = p->unk1D0 + obj->trans.rotation.x_rotation;
    p->unk1CA = p->unk1C6;
    p->unk1CC = p->unk1C8;
    aimYaw = p->unk1CE;
    aimPitch = p->unk1D0;
    if (p->unk1F6 != 0) {
        aimPitch += (p->unk1F7 * func_8004966C(p->unk1F6 << 12)) >> 8;
    }
    func_m051_01038(p->unk1DE, p->unk1DC);
    func_m051_00F44(p, aimPitch, aimYaw);
}

/* In the air */
static void func_m051_00DC0(Object *obj, M051Player *p, f32 updateRate) {
    if (obj->velocity.y < 0.0f && obj->trans.y_position < p->jumpStartY) {
        p->speed *= func_80049BEC(0.95f, updateRate);
        if (p->speed > -0.1f && p->speed < 0.1f) {
            p->speed = 0.0f;
        }
    }
}

/* Dying */
static void func_m051_00E98(Object *obj, M051Player *p, f32 updateRate) {
    obj->interactObj->hitPoints = 0;
    func_80048518(10);
    p->speed *= func_80049BEC(0.85f, updateRate);
    p->unk19C = 0;
    if (obj->unk28 == 1.0f) {
        func_80033920(obj, p);
    }
}

/* Writes the joint angle commands for the head (they depend on the character) */
static void func_m051_00F44(M051Player *p, s16 pitch, s16 yaw) {
    if (p->character == 6) {
        *sCommand = 0x36;
        sCommand++;
        *sCommand = pitch;
        sCommand++;
        *sCommand = 0x38;
        sCommand++;
        *sCommand = yaw;
        sCommand++;
    } else {
        *sCommand = 0x5A;
        sCommand++;
        *sCommand = pitch;
        sCommand++;
        *sCommand = 0x5C;
        sCommand++;
        *sCommand = yaw;
        sCommand++;
    }
}

/* Writes the joint angle commands for the neck, its pitch clamped */
static void func_m051_01038(s16 aimPitch, s16 aimYaw) {
    if (aimPitch > 0x2AAA) {
        aimPitch = 0x2AAA;
    } else if (aimPitch < -0x2AAA) {
        aimPitch = -0x2AAA;
    }
    *sCommand = 0x30;
    sCommand++;
    *sCommand = aimPitch;
    sCommand++;
    *sCommand = 0x32;
    sCommand++;
    *sCommand = aimYaw;
    sCommand++;
}

/* The move for the current speed (the same for every speed but zero) */
static s32 func_m051_010D4(M051Player *p) {
    if (p->speed < 0.0f) {
        return 0;
    }
    if (p->speed > 0.0f) {
        return 0;
    }
    return 0x12;
}

/* Starts a move's animation, unless it is playing already (or the boss is dead) */
static void func_m051_01118(Object *obj, M051Player *p, s32 move, f32 frame) {
    if (obj->animationID != 0x17) {
        move = sMoves[move].animArmed;
        if (move != obj->animationID) {
            func_8001149C(obj, move, frame);
        }
    }
}

/* Advances the animation, and picks the next one at the end of some */
static void func_m051_01170(Object *obj, M051Player *p, f32 updateRate) {
    f32 speed;
    f32 frame;
    f32 startFrame;
    f32 nextFrame;
    s32 move;
    s32 anim;

    anim = obj->animationID;
    speed = sAnimSpeeds[anim];
    move = anim;
    frame = obj->unk28;
    startFrame = obj->unk28;
    nextFrame = speed * updateRate + frame;
    switch (anim) {
        case 18:
            move = func_m051_010D4(p);
            if (p->speed != 0.0f) {
                frame = 0.25f;
            }
            break;
        case 0:
            speed *= p->speed;
            move = func_m051_010D4(p);
            if (p->speed == 0.0f) {
                frame = 0.0f;
            }
            break;
        case 14:
            if (nextFrame >= 1.0f) {
                move = func_m051_010D4(p);
                frame = 0.0f;
            }
            break;
        case 16:
            if (nextFrame > 1.0f) {
                speed = (1.0f - frame) / updateRate;
            }
            break;
        case 17:
            if (nextFrame > 1.0f) {
                speed = (1.0f - frame) / updateRate;
            }
            if (p->unk56B == 0) {
                move = func_m051_010D4(p);
                frame = 0.0f;
            }
            break;
    }
    func_8001138C(obj, speed, updateRate);
    func_8002B1D4(obj, startFrame, speed);
    func_m051_01118(obj, p, move, frame);
}

/* Moves the boss: gravity, staying inside the arena, and landing */
static void func_m051_0134C(Object *obj, M051Player *p, f32 updateRate) {
    M051ArenaEntry *entry;
    s32 count2;
    s32 count1;
    f32 dx;
    f32 top;
    f32 dz;
    f32 radius;
    f32 dist;
    f32 height;
    f32 floor1;
    f32 floor2;
    s32 i;
    f32 **heights2;
    f32 **heights1;

    top = obj->trans.y_position + 0.1f;
    obj->velocity.x = func_80049C30(p->unk11C) * p->speed;
    obj->velocity.y -= D_800F6DC4 * updateRate;
    obj->velocity.z = func_80049C34(p->unk11C) * -p->speed;
    func_80009A04(obj, obj->velocity.x * updateRate, obj->velocity.y * updateRate, obj->velocity.z * updateRate);
    if (ARENA(p) != NULL) {
        entry = (M051ArenaEntry *) ARENA(p)->level_entry;
        dx = entry->common.x - obj->trans.x_position;
        dz = entry->common.z - obj->trans.z_position;
        radius = entry->radius;
        dist = func_80097140(dx * dx + dz * dz);
        if (radius < dist && dist > 0.0f) {
            dist = dist / radius - 1.0f;
            func_80009A04(obj, dx * dist, 0.0f, dz * dist);
        }
    }
    count1 = func_80082D80(obj->trans.x_position, obj->trans.z_position, 0x800, &heights1);
    count2 = func_8001B078(obj->trans.x_position, obj->trans.z_position, 0x800, &heights2);
    if (count2 != 0 || count1 != 0) {
        if (count2 != 0) {
            for (i = 0; i < count2; i++) {
                if (*heights2[i] < top) {
                    break;
                }
            }
            if (i >= count2) {
                i = count2 - 1;
            }
            floor2 = *heights2[i];
        } else {
            floor2 = -32000.0f;
        }
        if (count1 != 0) {
            for (i = 0; i < count1; i++) {
                if (*heights1[i] < top) {
                    break;
                }
            }
            if (i >= count1) {
                i = count1 - 1;
            }
            floor1 = *heights1[i];
        } else {
            floor1 = -32000.0f;
        }
        if (floor2 < floor1) {
            height = obj->trans.y_position - floor1;
        } else {
            height = obj->trans.y_position - floor2;
        }
        if (height <= 0.0f) {
            if (p->state == 0x23 && obj->animationID == 0x10) {
                p->unk56B = 10;
                func_m051_01118(obj, p, 0x11, 0.0f);
            } else if (p->state != 0x20) {
                func_m051_01118(obj, p, func_m051_010D4(p), 0.0f);
            }
            if (p->state != 0x2A) {
                p->state = 0x20;
            }
            obj->velocity.y = 0.0f;
            func_80009A04(obj, 0.0f, -height, 0.0f);
        } else if (obj->animationID == 5 && obj->trans.y_position < p->jumpStartY && height > 25.0f) {
            func_m051_01118(obj, p, 0x10, 0.0f);
        } else if (height > 25.0f) {
            if (p->state != 0x23) {
                if (p->state != 0x2A) {
                    p->state = 0x23;
                }
                func_m051_01118(obj, p, 0x10, 0.0f);
            }
        }
    }
}
