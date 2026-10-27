/*
 * Runlink module 54: a player-steered craft. The stick turns and banks it
 * and changes its speed, a button boosts it, and another fires its weapon
 * (with a cooldown) as a projectile from its nose. Two attached models
 * pulse with the bank.
 */
#include "module.h"

/* A weapon the craft can fire (indexed by M054Craft.weapon) */
typedef struct M054Weapon {
    /* 0x0 */ u8 cooldown; /* frames between shots */
    /* 0x1 */ u8 unk1;
    /* 0x2 */ u8 autoFire; /* fires while the button is held */
    /* 0x3 */ u8 pad3;
    /* 0x4 */ s16 unk4;
    /* 0x6 */ s16 unk6;
    /* 0x8 */ f32 speed; /* added to the craft's */
} M054Weapon;

/*
 * Two tables that several modules carry unused (a shared header's static
 * data): scores for events, and pairs of rates.
 */
typedef struct M054Event {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ char name[26];
} M054Event;

typedef struct M054Rates {
    /* 0x0 */ f32 unk0;
    /* 0x4 */ f32 unk4;
    /* 0x8 */ f32 unk8;
    /* 0xC */ f32 unkC;
} M054Rates;

/* An attached model (what M054Craft.models point at) */
typedef struct M054Model {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ s16 unk20;
} M054Model;

/* The craft's data (obj->unk68) */
typedef struct M054Craft {
    /* 0x000 */ s8 unk0; /* the controller / player */
    /* 0x001 */ u8 pad1[0x14A - 0x1];
    /* 0x14A */ s8 bank; /* -127..127, from the stick */
    /* 0x14B */ u8 pad14B[0x240 - 0x14B];
    /* 0x240 */ s16 unk240;
    /* 0x242 */ s16 unk242;
    /* 0x244 */ s16 unk244;
    /* 0x246 */ s16 unk246;
    /* 0x248 */ s16 unk248;
    /* 0x24A */ u8 pad24A[0x344 - 0x24A];
    /* 0x344 */ s8 fireTimer;
    /* 0x345 */ u8 weapon;
    /* 0x346 */ s16 turn;
    /* 0x348 */ s16 roll;
    /* 0x34A */ s16 tilt;
    /* 0x34C */ f32 speed;
    /* 0x350 */ f32 boost;
    /* 0x354 */ u8 pad354[0x598 - 0x354];
    /* 0x598 */ M054Model *models[2];
    /* 0x5A0 */ u8 pad5A0[0x5A8 - 0x5A0];
    /* 0x5A8 */ s8 modelPhase[4];
    /* 0x5AC */ u8 modelScale[4];
} M054Craft;

/* The projectile the craft fires (func_80005F14) */
typedef struct M054Shot {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s8 unk2;
    /* 0x03 */ s8 unk3;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ s16 angle;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s8 unk10;
    /* 0x11 */ s8 unk11;
    /* 0x12 */ u8 pad12;
    /* 0x13 */ s8 unk13;
    /* 0x14 */ f32 speed;
    /* 0x18 */ Object *owner;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} M054Shot;

/* What func_8004470C returns */
typedef struct M054Level {
    /* 0x00 */ u8 pad0[0x28];
    /* 0x28 */ f32 unk28;
} M054Level;

extern s32 gPlayerButtonsHeld; /* buttons held */
extern s32 gPlayerButtonsPressed; /* buttons pressed */
extern s32 gPlayerStickX; /* stick x */
extern s32 gPlayerStickY; /* stick y */

extern s32 func_8004966C(s16 angle);
extern void func_800694CC(M054Model *model, s16 angle);
extern void func_8006999C(M054Model *model, s16 size, u8 scale);
extern void func_80005F14(M054Shot *shot, s32 count);
extern void player_input_latch(s32 player);
extern void set_active_camera(s32 player);
extern M054Level *func_8004470C(void); /* DKR levelGetLevel */
extern s32 func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */
extern f32 func_80049BEC(f32 base, s32 exponent);
extern s16 angle_lerp(s16 from, s16 to, f32 rate);
extern void func_8002EB6C(Object *obj, M054Craft *craft, f32 rate);

static M054Weapon sCraftWeapons[1] = {
    { 10, 3, 1, 0, 10, 0x50, 12.0f },
};

static M054Event sScoreEvents[13] = {
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

static M054Rates D_m054_0094C[19] = {
    { 0.004f, 0.8f }, { 0.007f, 0.85f }, { 0.01f, 0.85f }, { 0.004f, 0.5f }, { 0.01f, 0.5f },
    { 0.01f, 0.5f },  { 0.01f, 0.5f },   { 0.01f, 0.5f },  { 0.01f, 0.5f },  { 0.01f, 0.5f },
    { 0.004f, 0.5f }, { 0.004f, 0.8f },  { 0.004f, 0.8f }, { 0.004f, 0.84f }, { 0.004f, 0.8f },
    { 0.004f, 0.8f }, { 0.004f, 0.8f },  { 0.004f, 0.8f }, { 0.004f, 0.8f },
};

/* Pulses the two attached models: brighter while banking, dimmer when level (module m054, 0x00000). */
static void craft_models_pulse(M054Craft *craft, s32 updateRate) {
    s32 i;
    M054Model *model;
    s32 scale;
    s32 size;

    for (i = 0; i < 2; i++) {
        model = craft->models[i];
        if (model != NULL) {
            scale = craft->modelScale[i];
            if (craft->bank != 0) {
                scale += updateRate * 16;
                if (scale >= 0x100) {
                    scale = 0xFF;
                }
            } else {
                scale -= updateRate * 8;
                if (scale < 0x80) {
                    scale = 0x80;
                }
            }
            craft->modelScale[i] = scale;
            craft->modelPhase[i] += updateRate * 16;
            size = model->unk20;
            size += (model->unk20 * func_8004966C(craft->modelPhase[i] << 8)) >> 20;
            size = (size * scale) >> 8;
            func_800694CC(model, craft->modelPhase[i] << 8);
            func_8006999C(model, size, craft->modelScale[i]);
        }
    }
}

/* Fires the craft's weapon when the fire button is pressed (or held, for automatic ones) (module m054, 0x00140). */
static void craft_weapon_fire(Object *obj, M054Craft *craft, s32 updateRate) {
    M054Shot shot;
    M054Weapon *weapon;

    weapon = &sCraftWeapons[craft->weapon];
    if (craft->fireTimer > 0) {
        craft->fireTimer -= updateRate;
    }
    if ((gPlayerButtonsPressed & 0x2000) || ((gPlayerButtonsHeld & 0x2000) && craft->fireTimer <= 0 && weapon->autoFire != 0)) {
        shot.unk0 = weapon->unk6;
        shot.unk2 = 0x24;
        shot.unk3 = 0;
        shot.x = func_80049C34(obj->trans.rotation.y_rotation + 0x8000) * -32.0f + obj->trans.x_position;
        shot.y = obj->trans.y_position;
        shot.z = func_80049C30(obj->trans.rotation.y_rotation + 0x8000) * -32.0f + obj->trans.z_position;
        shot.unkA = weapon->unk1;
        shot.unkB = weapon->unk4;
        shot.angle = obj->trans.rotation.y_rotation + 0x8000;
        shot.unkE = 0;
        shot.unk10 = 0;
        shot.unk11 = 0;
        shot.speed = craft->speed + weapon->speed;
        shot.unk1C = 0;
        shot.unk20 = 0;
        shot.unk13 = -1;
        shot.owner = obj;
        func_80005F14(&shot, 1);
        func_80001248(3, 0);
        craft->fireTimer = weapon->cooldown;
    }
}

void func_m054_0033C(Object *obj, s32 updateRate) {
    f32 boost;
    f32 speed;
    s16 target;
    f32 rate;
    s16 unused; /* unused, but it has a stack slot in the original */
    M054Craft *craft;
    s16 i;
    f32 rate2;

    craft = obj->unk68;
    rate2 = rate = updateRate;
    player_input_latch(craft->unk0);
    if (gPlayerButtonsHeld & 0x10) {
        boost = 1.5f;
    } else {
        boost = 1.0f;
    }
    for (i = updateRate; i > 0; i--) {
        craft->boost += (boost - craft->boost) * 0.1f;
    }
    target = (-gPlayerStickX << 16) / 6300;
    i = updateRate;
    if (i > 0) {
        do {
            craft->turn += (target - craft->turn) >> 4;
        } while (--i > 0);
    }
    if (((target - craft->turn) >> 4) == 0) {
        craft->turn = target;
    }
    obj->trans.rotation.y_rotation += (s32) (craft->turn * rate * craft->boost);
    target = (-gPlayerStickX * 0x1400) / 70;
    i = updateRate;
    if (i > 0) {
        do {
            craft->tilt += (target - craft->tilt) >> 4;
        } while (--i > 0);
    }
    if (((target - craft->tilt) >> 4) == 0) {
        craft->tilt = target;
    }
    obj->trans.rotation.z_rotation = craft->tilt * craft->boost;
    speed = 14.0f;
    if (gPlayerStickY < -63) {
        craft->bank = -127;
    } else if (gPlayerStickY >= 64) {
        craft->bank = 127;
    } else {
        craft->bank = gPlayerStickY * 2;
    }
    if (gPlayerStickY > 0) {
        speed += gPlayerStickY * 7.0f / 70.0f;
    } else if (gPlayerStickY < 0) {
        speed += gPlayerStickY * 7.0f / 70.0f;
    }
    for (i = updateRate; i > 0; i--) {
        craft->speed += (speed - craft->speed) * 0.1f;
    }
    obj->velocity.x = func_80049C34(obj->trans.rotation.y_rotation + 0x8000) * -craft->speed;
    obj->velocity.y = func_8004470C()->unk28;
    obj->velocity.z = func_80049C30(obj->trans.rotation.y_rotation + 0x8000) * -craft->speed;
    obj->trans.y_position = 0.0f;
    func_80009A04(obj, obj->velocity.x * rate, obj->velocity.y * rate, obj->velocity.z * rate);
    craft_weapon_fire(obj, craft, updateRate);
    craft->roll = angle_lerp(craft->roll, (craft->bank << 7) - 0x4000, 1.0f - func_80049BEC(0.9f, updateRate));
    craft->unk240 = 6;
    craft->unk244 = 12;
    craft->unk248 = 0x1000;
    craft->unk242 = craft->roll;
    craft->unk246 = craft->roll;
    craft_models_pulse(craft, updateRate);
    set_active_camera(craft->unk0);
    func_8002EB6C(obj, craft, rate2);
}
