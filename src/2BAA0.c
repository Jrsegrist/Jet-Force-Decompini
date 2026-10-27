#include "common.h"

extern f32 D_800F6DC4;
extern void *D_800F6DC0;
extern s32 D_800A2D38;
extern s32 D_800F6DF8[24];

/* JFG: flag word at 0x540 of a player's state block. */
typedef struct RacerFlags {
    u32 unk31 : 1;
    u32 unk30 : 1;
    u32 weaponDir : 2; /* 1: last weapon change went backwards, 2: forwards (see func_80037420) */
    u32 unk27 : 1;
    u32 useOverride : 1; /* use overridePos as the position (func_8003B418) */
    u32 unk25 : 1;
    u32 unk24 : 1;
    u32 unk8 : 16;
    u32 unk7 : 1;
    u32 unk0 : 7;
} RacerFlags;

/* JFG: a player's state block (the object's 0x68 block); only the fields used here. */
typedef struct Racer {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0xC0 - 0x2];
    /* 0x0C0 */ s32 sound;
    /* 0x0C4 */ u8 padC4[0x1A8 - 0xC4];
    /* 0x1A8 */ u8 *items[6];
    /* 0x1C0 */ u8 pad1C0[0x53A - 0x1C0];
    /* 0x53A */ u16 soundId;
    /* 0x53C */ u8 pad53C[0x540 - 0x53C];
    /* 0x540 */ RacerFlags flags;
    /* 0x544 */ f32 overridePos[3];
    /* 0x550 */ u8 pad550[0x558 - 0x550];
    /* 0x558 */ void *light;
    /* 0x55C */ u8 pad55C[0x560 - 0x55C];
    /* 0x560 */ u8 *attachment;
} Racer;

s32 func_8003B51C(s32 value, s32 delta);
void func_8002AEA0(s32 arg0, s32 arg1);
u8 *func_80047C1C(s32 player, s32 character);
void *spawn_object(void *entry, s32 arg1);
void func_800478A4(s32 weapon);
void func_80001228(s32 soundHandle);
void sound_play(u16 soundID, s32 *handlePtr);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002AEA0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B064);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B1D4);

/* Size: 0x2C bytes */
typedef struct CameraBlend {
    /* 0x00 */ f32 blend;
    /* 0x04 */ u8 pad4[0x28];
} CameraBlend;
extern CameraBlend D_800F6E70[];

f32 func_8002B3F0(s32 index) {
    return D_800F6E70[index].blend;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B418);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B960);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002BD04);

typedef struct Transform2BAA0 {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 position[3];
} Transform2BAA0;

s32 func_8003FC1C(void);
void func_8003FC60(s32 index, s32 value);

void func_8002C070(UNUSED s32 arg0, u8 *racer, UNUSED s32 arg2) {
    u8 *target = *(u8 **) (racer + 0x50C);
    u8 *state;

    if (target != NULL) {
        state = *(u8 **) (target + 0x68);
        ((Transform2BAA0 *) D_800F6DC0)->position[0] = *(f32 *) (target + 0xC);
        ((Transform2BAA0 *) D_800F6DC0)->position[1] = *(f32 *) (target + 0x10);
        ((Transform2BAA0 *) D_800F6DC0)->position[2] = *(f32 *) (target + 0x14);
        ((Transform2BAA0 *) D_800F6DC0)->rotation[2] = *(s16 *) (state + 0x64);
        ((Transform2BAA0 *) D_800F6DC0)->rotation[1] = *(s16 *) (state + 0x62);
        ((Transform2BAA0 *) D_800F6DC0)->rotation[0] = -*(s16 *) (state + 0x60);
        func_8003FC60(func_8003FC1C(), 0);
    }
}

/* An empty function right after func_8002C070 (no symbol of its own in the ROM's split). */
void func_8002C110(void) {
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002C118);

typedef struct ViewPoint {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 unk6;
    /* 0x08 */ f32 position[3];
} ViewPoint;

extern ViewPoint D_800F6DC8;
void func_80042A74(ViewPoint *view, void *transform, s32 arg2, u8 arg3);

void func_8002C908(u8 *obj, u8 *racer, f32 arg2) {
    D_800F6DC8.position[0] = *(f32 *) (obj + 0xC);
    D_800F6DC8.position[1] = *(f32 *) (obj + 0x10);
    D_800F6DC8.position[2] = *(f32 *) (obj + 0x14);
    D_800F6DC8.rotation[0] = *(s16 *) (obj + 0);
    D_800F6DC8.rotation[1] = *(s16 *) (obj + 2);
    D_800F6DC8.rotation[2] = *(s16 *) (obj + 4);
    D_800F6DC8.unk6 = 0;
    func_80042A74(&D_800F6DC8, D_800F6DC0, arg2, racer[0x574]);
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002C9A0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002CC70);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002CF6C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002EB6C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002ED94);

void func_8002F014(s32 obj) {
    if (D_800A2D38 < 24) {
        D_800F6DF8[D_800A2D38] = obj;
        D_800A2D38++;
    }
}

void func_8002F04C(s32 obj) {
    s32 i;
    s32 found;

    i = 0;
    found = -1;
    for (; i < D_800A2D38; i++) {
        if (obj == D_800F6DF8[i]) {
            found = i;
        }
    }
    if (found != -1) {
        for (i = found; i < D_800A2D38 - 1; i++) {
            D_800F6DF8[i] = D_800F6DF8[i + 1];
        }
        D_800A2D38--;
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F0DC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F2B0);

extern f32 D_800A2BE0[4];
extern f32 D_800A2BF0[4];
extern f32 D_800A2C00[4];
extern f32 D_800A2C10[4];
extern f32 D_800A2C20[4];
extern f32 D_800A2C30[4];
extern f32 D_800A2C50[4];
extern f32 D_800A2C60[];
extern f32 D_800A2C84[];
extern f32 D_800A2CAC;
extern f32 D_800A2CCC[];

void func_8002F450(s32 index, f32 scale) {
    index &= 3;
    D_800A2C60[1] = D_800A2C20[index] * scale;
    D_800A2C84[1] = D_800A2C30[index] * scale;
    D_800A2CCC[1] = D_800A2C50[index] * scale;
    D_800A2C60[4] = D_800A2BE0[index] * scale;
    D_800A2C84[4] = D_800A2BF0[index] * scale;
    D_800A2CAC = D_800A2C00[index] * scale;
    D_800A2CCC[4] = D_800A2C10[index] * scale;
}

extern f32 D_800A2D40;
extern f32 D_800ABE40;
extern f32 D_800ABE44;
f32 func_80049BEC(f32 rate, s32 updateRate);

void func_8002F50C(UNUSED s32 arg0, u8 *arg1, s32 updateRate) {
    f32 target;
    f32 rate;
    u8 *racer;

    if (*(u8 **) (arg1 + 0x24) != NULL) {
        racer = *(u8 **) (*(u8 **) (arg1 + 0x24) + 0x68);
        if (racer[0x568] == 2 || racer[0x568] == 10) {
            target = 0.5f;
            rate = D_800ABE40;
        } else if (racer[0x568] == 1 || (racer[0x568] == 2 && (s8) racer[1] != 2)) {
            target = 0.75f;
            rate = D_800ABE44;
        } else {
            target = 1.0f;
            rate = 0.875f;
        }
        D_800A2D40 += (target - D_800A2D40) * (1.0f - func_80049BEC(rate, updateRate));
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F5F0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F728);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031090);

typedef struct ModelEntry {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 pad2;
    /* 0x4 */ s32 unk4;
} ModelEntry;

extern f32 D_800ABEA4;

void func_800319C0(u8 *obj, u8 *arg1, u8 *racer, s32 updateRate) {
    s16 value = 0x300;
    s32 i;
    u8 **model;
    u8 *header;
    ModelEntry *entry;

    *(s16 *) (arg1 + 0x92) += updateRate;
    if ((*(u16 *) (racer + 0x540) & 0xF) && (*(s16 *) (arg1 + 0x92) & 0x20)) {
        value = 0;
    } else if (**(f32 **) (obj + 0x54) < D_800ABEA4) {
        value = 0x100;
    }
    model = **(u8 ****) (obj + 0x6C);
    header = model[0];
    entry = (ModelEntry *) model[0x74 / 4];
    for (i = 0; i < header[0x2C]; i++, entry++) {
        if ((entry->unk4 & 0xFF) == 3) {
            entry->unk0 = value;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031A80);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031D00);

/* JFG: clear the six item links of a player (each item's flag for this player at 0x93). */
void func_80031F44(Racer *racer) {
    s32 i = 6;
    u8 *item;

    if (racer->items[5] != NULL) {
        while (i--) {
            item = racer->items[i];
            if (item != NULL) {
                item[racer->player + 0x93] = 0;
                racer->items[i] = NULL;
            }
        }
    }
}

/* JFG: zero the bytes from start up to end. */
void func_80031F9C(u8 *start, u8 *end) {
    u8 *p = start;
    while (p < end) {
        *p++ = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80031FE8);

void func_80032128(s32 arg0) {
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80032130);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80032A48);

extern u8 D_800A4FC4;
extern u8 D_800A4FEC;
extern s8 D_800FED63[][0xA0];
s32 func_80048558(void);
void func_80048518(s32 arg0);
u32 input_pressed(s32 player);
void func_800339D8(s32 arg0, s8 *player);

void func_80033920(s32 arg0, s8 *player) {
    u32 buttons;

    if (func_80048558() == 1) {
        func_80048518(10);
    }
    buttons = input_pressed(*player);
    if (D_800A4FC4 != 0 && (D_800FED63[*player][0] == 0 || D_800A4FEC != 0)) {
        buttons = 0;
    }
    if (buttons & 0xF00F) {
        func_800339D8(arg0, player);
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800339D8);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033BB8);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033D74);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80033ED4);

s32 func_80033F4C(s32 weapon) {
    if (weapon == 8 || weapon == 10 || weapon == 11 || weapon == 12 || weapon == 13 || weapon == 14) {
        return 1;
    }
    if (weapon == 9) {
        return 2;
    }
    return 0;
}

/* JFG: step an angle from `from` towards `to` by the fraction t, the short way round. */
s16 func_80033FA4(s16 from, s16 to, f32 t) {
    s32 diff = (to - from) & 0xFFFF;
    s32 back = (from - to) & 0xFFFF;

    if (back < diff) {
        diff = -back;
    }
    return from + (s32) (diff * t);
}

s32 *get_misc_asset(s32 index);

void func_80034018(s8 *racer) {
    f32 *speeds;
    f32 *scales;

    scales = (f32 *) get_misc_asset(1);
    speeds = (f32 *) get_misc_asset(2);
    D_800F6DC4 = scales[racer[1] & 3] * speeds[racer[0x575]];
    if ((u8) racer[0x194] == 1) {
        D_800F6DC4 = 0.0f;
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800340A4);

extern u8 D_800A2DB4;
void func_800496B0(s32, s16 *, void *, u8 *);

void func_80034164(s16 *pos, u8 *obj) {
    s16 v[3];

    v[0] = pos[0];
    v[1] = pos[1];
    v[2] = 0;
    func_800496B0(3, v, &D_800A2DB4, obj + 0x18);
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800341B4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800344B4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800346C4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800349AC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80034BB0);

typedef struct PoseTarget {
    /* 0x00 */ f32 **points;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 pad5[3];
    /* 0x08 */ s32 steps;
    /* 0x0C */ u8 padC[8];
} PoseTarget;

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035514);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035628);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035894);

/* JFG: in some racer states (6, 7, 8 and 12), copy the racer's angle at 0x17E to 0x120 and to the object. */
void func_80035AEC(u8 *obj, u8 *racer, UNUSED s32 arg2) {
    u8 state = racer[0x568];

    if (state == 8 || state == 6 || state == 7 || state == 0xC) {
        s16 angle = *(s16 *) (racer + 0x17E);
        *(s16 *) (racer + 0x120) = angle;
        *(s16 *) (obj + 4) = angle;
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035B30);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035E14);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800364D4);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80036A30);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80036DBC);

extern f32 D_800ABF74;
extern f32 D_800ABF78;

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800370D0);

void func_8001CE10(s32 arg0, f32 *from, f32 *to, f32 *radius, s32 arg4, s32 arg5, s32 arg6);
s32 func_80016208(void *obj, f32 *from, f32 *to, f32 radius, s32 arg4, void *hit);

/* JFG: 1 unless a collision test from (x, y, z) up by `height` hits a surface with flags 0x48. */
s32 func_80037194(f32 x, f32 y, f32 z, f32 height, f32 radius) {
    u8 hit[0x28];
    f32 from[3];
    f32 to[3];
    s32 clear;

    clear = 1;
    from[0] = x;
    from[1] = y;
    from[2] = z;
    to[0] = x;
    to[1] = y + height;
    to[2] = z;
    func_8001CE10(1, from, to, &radius, 0, 0, 0);
    if (func_80016208(NULL, from, to, radius, 1, hit) != 0 && (hit[0x25] & 0x48)) {
        clear = 0;
    }
    return clear;
}

extern s32 D_800F6DBC;

void func_80037244(s32 arg0) {
    D_800F6DBC = arg0;
}

extern s32 D_800F6DA0;
extern s32 D_800F6DA4;
extern s32 D_800F6DA8;
extern s32 D_800F6DAC;
extern s32 D_800F6DB0;
extern s32 D_800F6DB4;
extern s32 D_800F6DB8;
u16 input_held(s32 player);
u16 input_released(s32 player);
s32 input_clamp_stick_x(s32 player);
s32 input_clamp_stick_y(s32 player);
s8 func_8004359C(s32 player);
s8 func_8004360C(s32 player);

/* JFG: latch a player's input for this frame (all zero when input is off, see func_80037244). */
void func_80037250(s32 player) {
    if (player >= 0 && player < 4 && D_800F6DBC == 0) {
        D_800F6DAC = input_clamp_stick_x(player);
        D_800F6DB4 = func_8004359C(player);
        D_800F6DB0 = input_clamp_stick_y(player);
        D_800F6DB8 = func_8004360C(player);
        D_800F6DA0 = input_held(player);
        D_800F6DA4 = input_pressed(player);
        D_800F6DA8 = input_released(player);
    } else {
        D_800F6DAC = 0;
        D_800F6DB4 = 0;
        D_800F6DB0 = 0;
        D_800F6DB8 = 0;
        D_800F6DA0 = 0;
        D_800F6DA4 = 0;
        D_800F6DA8 = 0;
    }
}

extern u8 D_800A2DD8[8];
extern u8 D_800A2DE8[8];
extern s32 D_800A2DF0[8];
void func_8004B3D8(s32 arg0, u8 arg1, s32 arg2);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003733C);

s32 func_800373E4(u8 *chr, s32 weapon) {
    switch (weapon) {
        case 6:
            return 110;
        case 4:
            return 30;
    }
    return (chr + weapon)[0x54] * 32;
}

/* JFG: which way round the 16 weapon slots (owned bits at 0xA) is shorter from the current weapon to `weapon`:
   1 = forwards, 2 = backwards. */
s32 func_80037420(u8 *chr, s32 weapon) {
    s32 start;
    s32 bit;
    s32 forwards;
    s32 backwards;

    start = 1 << chr[0x70];
    weapon = 1 << weapon;
    forwards = 0;
    bit = start;
    while (bit != weapon) {
        if (*(u16 *) (chr + 0xA) & bit) {
            forwards++;
        }
        bit <<= 1;
        if (bit == 0x10000) {
            bit = 1;
        }
    }
    bit = start;
    backwards = 0;
    while (bit != weapon) {
        if (*(u16 *) (chr + 0xA) & bit) {
            backwards++;
        }
        if (bit == 1) {
            bit = 0x10000;
        }
        bit >>= 1;
    }
    if (backwards >= forwards) {
        return 2;
    }
    return 1;
}

extern u8 D_800A5074;

s32 func_800374C8(u8 *racer, u8 *chr) {
    u8 weapon;
    u8 *held;

    if (D_800A5074 != 0) {
        return 1;
    }
    weapon = chr[0x70];
    if (weapon == 0 || ((u16 *) (chr + 0x14))[weapon] != 0) {
        return 1;
    }
    if (weapon == 13) {
        held = *(u8 **) (racer + 0x5CC);
        if (held != NULL && *(s16 *) (held + 0x48) == 0x59) {
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80037550);

void func_8003B820(u8 *obj);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038324);

/* JFG: switch to `weapon` if owned and not already selected; returns 1 if switched. */
s32 func_80038430(u8 *obj, Racer *racer, s32 weapon) {
    u8 *chr = func_80047C1C(racer->player, racer->character);
    u8 cur = chr[0x70];

    if (weapon != cur && (*(u16 *) (chr + 0xA) & (1 << weapon))) {
        if (cur == 2 && racer->sound != 0) {
            func_80001228(racer->sound);
            racer->soundId = 0;
        }
        racer->flags.weaponDir = func_80037420(chr, weapon);
        racer->flags.unk30 = 0;
        racer->flags.unk7 = 0;
        chr[0x70] = weapon;
        func_8003B820(obj);
        func_80031F44(racer);
        return 1;
    }
    return 0;
}

/* JFG: switch to the previous owned weapon (slots 15..0, wrapping). */
void func_80038528(u8 *obj, Racer *racer) {
    s32 weapon;
    s32 cur;
    u8 *chr;
    u16 bit;

    chr = func_80047C1C(racer->player, racer->character);
    cur = chr[0x70];
    bit = 1 << cur;
    weapon = cur;
    do {
        if (weapon == 0) {
            weapon = 15;
            bit = 0x8000;
        } else {
            weapon--;
            bit >>= 1;
        }
    } while (!(*(u16 *) (chr + 0xA) & bit));
    if (weapon != cur) {
        if (cur == 2 && racer->sound != 0) {
            func_80001228(racer->sound);
            racer->soundId = 0;
        }
        chr[0x70] = weapon;
        racer->flags.unk7 = 0;
        racer->flags.weaponDir |= 1;
        racer->flags.unk30 = 0;
        func_8003B820(obj);
        func_80031F44(racer);
    }
}

extern u8 D_800A2E10[3];
void func_8002130C(void *light, u8 red, u8 green, u8 blue);

void func_80038630(UNUSED s32 arg0, Racer *racer) {
    if (racer->light != NULL) {
        func_8002130C(racer->light, D_800A2E10[0], D_800A2E10[1], D_800A2E10[2]);
    }
    if (racer->sound != 0) {
        func_80001228(racer->sound);
    }
    sound_play(3, &racer->sound);
    racer->soundId = 3;
    racer->flags.unk25 = 1;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800386B0);

u8 **get_racer_objects(s32 *numRacers);
void mempool_free(void *data);

void func_80038F04(void) {
    s32 numRacers;
    u8 **racers = get_racer_objects(&numRacers);

    while (numRacers--) {
        u8 *racer = *(u8 **) (*racers++ + 0x68);
        if (*(void **) (racer + 0x564) != NULL) {
            mempool_free(*(void **) (racer + 0x564));
        }
    }
}

typedef struct Spawn13E {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ s32 owner;
} Spawn13E;

void func_80038F7C(s32 owner, Racer *racer, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8, s32 arg9) {
    Spawn13E setup;
    u8 *obj;

    setup.objectID = 0x13E;
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.unkC = arg5;
    setup.unkE = arg6;
    setup.unk10 = arg7;
    setup.unkA = arg8;
    setup.unkB = arg9;
    setup.owner = owner;
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
    }
    func_800478A4(3);
    if (racer->sound != 0) {
        func_80001228(racer->sound);
    }
    sound_play(0xE1, &racer->sound);
    racer->soundId = 0xE1;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039098);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039308);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039544);

typedef struct SpawnB6 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ u8 padA[6];
    /* 0x10 */ s32 owner;
    /* 0x14 */ s16 unk14[3][3];
    /* 0x26 */ s16 unk26;
    /* 0x28 */ u8 unk28;
    /* 0x29 */ u8 unk29;
    /* 0x2A */ u8 unk2A;
} SpawnB6;

extern u8 D_800A1553;

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800396CC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800397E0);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039918);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039A38);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039B98);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039D08);

typedef struct Spawn13A {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ s32 owner;
} Spawn13A;

void func_80039ECC(s32 owner, UNUSED s32 arg1, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8) {
    u8 *obj;
    Spawn13A setup;

    setup.objectID = 0x13A;
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.unkA = arg8 * 6;
    setup.unkC = arg5;
    setup.unkE = arg6;
    setup.unk10 = arg7;
    setup.owner = owner;
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        sound_play(0x2F, NULL);
        func_8002AEA0(owner, 2);
        *(s32 *) (obj + 0x3C) = 0;
        func_800478A4(14);
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039FCC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003AABC);

extern u8 D_800A1490[][0x30];

s32 func_8003B114(s8 *ids) {
    u8 *chr = func_80047C1C(ids[0], ids[1]);
    return (D_800A1490[chr[0x70]][0] >> ids[1]) & 1;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B16C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B290);

void func_8003B418(u8 *obj, Racer *racer, f32 *x, f32 *y, f32 *z) {
    u8 *model;

    if (racer->flags.useOverride) {
        *x = racer->overridePos[0];
        *y = racer->overridePos[1];
        *z = racer->overridePos[2];
        return;
    }
    model = **(u8 ***) (obj + 0x6C);
    if (*(f32 **) (model + 0x68) != NULL) {
        *x = (*(f32 **) (model + 0x68))[0];
        *y = (*(f32 **) (model + 0x68))[1];
        *z = (*(f32 **) (model + 0x68))[2];
        return;
    }
    *x = *(f32 *) (obj + 0xC);
    *y = *(f32 *) (obj + 0x10);
    *z = *(f32 *) (obj + 0x14);
}

/* JFG: addresses of two tables inside an object's state block (at 0x68). */
u8 *func_8003B4D8(u8 **obj) {
    u8 *state = obj[0x68 / 4];
    return state + 0x240;
}

u8 *func_8003B4E4(u8 **obj) {
    u8 *state = obj[0x68 / 4];
    return state + 0x96;
}

f32 func_8003B4F0(void) {
    return D_800F6DC4;
}

/* JFG: an entry of a table of 0x30-byte records. */
u8 *func_8003B500(s32 index) {
    return D_800A1490[index];
}

s32 func_8003B51C(s32 value, s32 delta) {
    if (delta != 0) {
        if ((delta & 0xC000) == 0x8000) {
            if (value < 0) {
                value += delta & 0x3FFF;
                if (value > 0) {
                    value = 0;
                }
            } else {
                value -= delta & 0x3FFF;
                if (value < 0) {
                    value = 0;
                }
            }
        } else {
            value += delta;
            if (value < -0x8000) {
                value = -0x8000;
            }
            if (value >= 0x8000) {
                value = 0x7FFF;
            }
        }
    }
    return value;
}

extern s16 D_800A11C0[][16];

void func_8003B590(u8 *obj, s32 index) {
    u8 *racer;
    s16 *rates;

    if (*(s16 *) (obj + 0x48) == 1) {
        racer = *(u8 **) (obj + 0x68);
        if (index >= 13) {
            index = 0;
        }
        rates = D_800A11C0[index];
        *(s16 *) (racer + 0x586) = func_8003B51C(*(s16 *) (racer + 0x586), rates[0]);
        *(s16 *) (racer + 0x588) = func_8003B51C(*(s16 *) (racer + 0x588), rates[1]);
        *(s16 *) (racer + 0x58A) = func_8003B51C(*(s16 *) (racer + 0x58A), rates[2]);
    }
}

extern s16 D_800F6DEC;
extern s16 D_800F6DEE;
extern s16 D_800F6DF0;
extern s16 D_800F6DF2;
extern u8 D_800A2EB0;

void func_8003B620(s16 a, s16 b, s16 c, s16 d) {
    D_800F6DEC = a;
    D_800F6DEE = b;
    D_800F6DF0 = c;
    D_800F6DF2 = d;
    D_800A2EB0 = 1;
}

s32 func_8003B660(s16 *a, s16 *b, s16 *c, s16 *d) {
    if (D_800A2EB0 != 0) {
        *a = D_800F6DEC;
        *b = D_800F6DEE;
        *c = D_800F6DF0;
        *d = D_800F6DF2;
        D_800A2EB0 = 0;
        return 1;
    }
    return 0;
}

extern u8 D_800A2EB0;

void func_8003B6BC(void) {
    D_800A2EB0 = 0;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B6C8);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003B740);

/* JFG: empty a player's hand (the held object at 0x5CC); a held object 0x59 is also stopped and reset. */
void func_8003B820(u8 *obj) {
    u8 *racer = *(u8 **) (obj + 0x68);
    u8 *held = *(u8 **) (racer + 0x5CC);

    if (held != NULL) {
        if (*(s16 *) (held + 0x48) == 0x59) {
            s32 *state = *(s32 **) (held + 0x68);
            *(f32 *) (held + 0x1C) = 0.0f;
            *(f32 *) (held + 0x20) = 0.0f;
            *(f32 *) (held + 0x24) = 0.0f;
            *state = 0;
        }
        *(u8 **) (racer + 0x5CC) = NULL;
    }
}
