#include "common.h"

extern f32 D_800F6DC4;
extern void *D_800F6DC0;
extern s32 D_800A2D38;
extern s32 D_800F6DF8[24];

/* JFG: flag word at 0x540 of a player's state block. */
typedef struct RacerFlags {
    u32 unk31 : 1;
    u32 unk30 : 1;
    u32 weaponDir : 2; /* 1: last weapon change went backwards, 2: forwards (see weapon_switch_direction) */
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
void player_drop_held_object(u8 *obj);
void *spawn_object(void *entry, s32 arg1);
void stats_shot_add(s32 weapon);
void sound_stop(s32 soundHandle);
void sound_play(u16 soundID, s32 *handlePtr);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002AEA0);

typedef struct SoundSet {
    /* 0x00 */ u8 count;
    /* 0x04 */ s16 *sounds;
} SoundSet;

extern SoundSet D_800A26C4[];
extern f32 D_800ABDA8;
s32 rand_range(s32 min, s32 max);
f32 sqrtf(f32 x);
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, void **handlePtr);
void func_80052734(u8 *arg0, s16 x, s16 y, s16 z, u8 arg4, u8 arg5, u8 arg6);

#ifdef NON_MATCHING
/* NON_MATCHING: 8 words: register allocation / scheduling around the sound-table index and the rand_range call */
/* JFG: play a random sound of set `index` (per character) at obj if it is moving fast enough. */
void func_8002B064(u8 *obj, u8 index) {
    u8 *racer = *(u8 **) (obj + 0x68);
    SoundSet *set = &D_800A26C4[index];
    s16 *sounds = set->sounds + set->count * (racer[1] & 3);
    s16 sound = sounds[rand_range(0, set->count - 1)];

    if (D_800ABDA8 < sqrtf(*(f32 *) (obj + 0x1C) * *(f32 *) (obj + 0x1C) + *(f32 *) (obj + 0x24) * *(f32 *) (obj + 0x24))) {
        audspat_play_sound_at_position(sound, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14), 4, NULL);
        func_80052734(obj + 1, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14), 1, 1, sound);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002B064);
#endif

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

s32 cam_get_active_camera_id(void);
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
        func_8003FC60(cam_get_active_camera_id(), 0);
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

typedef struct Flags94 {
    u8 unk7 : 1;
    u8 unk0 : 7;
} Flags94;

extern u8 D_800A4FC8;
void func_80031D00(u8 *obj, f32 *x, f32 *y, f32 *z, s32 arg4);
void ignore_bounds_check(void);
s32 move_object(u8 *obj, f32 xPos, f32 yPos, f32 zPos);
void func_80079D1C(u8 *obj, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, f32 arg7, f32 arg8,
                   f32 arg9, f32 argA, s32 argB);

void func_8002F5F0(u8 *obj, u8 *arg1) {
    f32 *state = *(f32 **) (obj + 0x68);

    *(s32 *) (state + 9) = *(s32 *) (arg1 + 0xC);
    D_800A2D40 = 1.0f;
    func_80031D00(obj, &state[0], &state[1], &state[2], 0);
    ignore_bounds_check();
    move_object(obj, state[0] - *(f32 *) (obj + 0xC), state[1] - *(f32 *) (obj + 0x10),
                state[2] - *(f32 *) (obj + 0x14));
    *(s16 *) (state + 6) = 0;
    *(s16 *) ((u8 *) state + 0x1A) = 0;
    func_80079D1C(obj, -1, 1, 2, 0, 0, 0, 125.0f, 125.0f, 1.0f, 1.0f, 0x11);
    state[3] = *(f32 *) (obj + 0xC);
    state[4] = *(f32 *) (obj + 0x10);
    state[5] = *(f32 *) (obj + 0x14);
    state[0x48 / 4] = 0.0f;
    state[0x4C / 4] = 0.0f;
    ((Flags94 *) (state + 0x94 / 4))->unk7 = D_800A4FC8;
    *(s32 *) (obj + 0x88) = 1;
}

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

/* JFG: clear the six item links of a player (each item's flag for this player at 0x93) (0x80031F44). */
void player_items_unlink(Racer *racer) {
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

/* JFG: zero the bytes from start up to end (0x80031F9C). */
void mem_zero_range(u8 *start, u8 *end) {
    u8 *p = start;
    while (p < end) {
        *p++ = 0;
    }
}

typedef struct RespawnInfo {
    /* 0x00 */ u8 pad0[0xA];
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ s8 player;
    /* 0x11 */ s8 character;
    /* 0x12 */ u8 pad12[2];
} RespawnInfo;

s32 get_level_segment_index_from_position(f32 xPos, f32 yPos, f32 zPos);
void free_object(void *object);
void func_80032130(u8 *obj, RespawnInfo *info);

/* JFG: put a player back at (x, y, z) and reset its state block (0x80031FE8). */
void player_respawn_at(u8 *obj, f32 x, f32 y, f32 z, s16 arg4, s16 arg5, s16 arg6) {
    RespawnInfo info;
    u8 *racer = *(u8 **) (obj + 0x68);

    info.player = ((Racer *) racer)->player;
    info.character = ((Racer *) racer)->character;
    info.unkE = arg4;
    info.unkC = arg5;
    info.unkA = arg6;
    *(s16 *) (obj + 6) &= ~0x400;
    *(f32 *) (obj + 0xC) = x;
    *(f32 *) (obj + 0x10) = y;
    *(f32 *) (obj + 0x14) = z;
    *(s16 *) (obj + 0x2E) = get_level_segment_index_from_position(x, y, z);
    obj[0x39] = 0xFF;
    if (*(u8 **) (racer + 0x1A0) != NULL) {
        *(s32 *) (*(u8 **) (*(u8 **) (racer + 0x1A0) + 0x68) + 0xA0) = 0;
    }
    if (*(void **) (racer + 0x5BC) != NULL) {
        free_object(*(void **) (racer + 0x5BC));
        *(void **) (racer + 0x5BC) = NULL;
    }
    mem_zero_range(racer, racer + 0xBC);
    mem_zero_range(racer + 0xF4, racer + 0x100);
    mem_zero_range(racer + 0x104, racer + 0x23C);
    mem_zero_range(racer + 0x340, racer + 0x364);
    mem_zero_range(racer + 0x510, racer + 0x550);
    mem_zero_range(racer + 0x568, racer + 0x598);
    mem_zero_range(racer + 0x5BC, racer + 0x5CA);
    *(s16 *) (racer + 0x5CA) = 0;
    player_drop_held_object(obj);
    player_items_unlink((Racer *) racer);
    func_80032130(obj, &info);
}

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

/* JFG: step an angle from `from` towards `to` by the fraction t, the short way round (0x80033FA4). */
s16 angle_lerp(s16 from, s16 to, f32 t) {
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

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words: the target schedules the lb of 0x530 before the sw to 0x360 */
/* JFG: start moving towards the points of a new pose: per-step deltas (0x424) from the current points (0x364). */
void func_80035514(UNUSED s32 arg0, u8 *racer, PoseTarget *poses, u8 index) {
    PoseTarget *pose = &poses[index];
    f32 *from = (f32 *) (racer + 0x364);
    f32 *step;
    s32 i;
    f32 steps;

    if (pose != *(PoseTarget **) (racer + 0x360)) {
        racer[0x531] = pose->unk4;
        *(PoseTarget **) (racer + 0x360) = pose;
        racer[0x535] = pose->steps;
        step = (f32 *) (racer + 0x424);
        steps = (s8) racer[0x535];
        for (i = 0; i < (s8) racer[0x530]; i++) {
            step[0] = ((*(PoseTarget **) (racer + 0x360))->points[i][0] - from[0]) / steps;
            step[1] = ((*(PoseTarget **) (racer + 0x360))->points[i][1] - from[1]) / steps;
            step[2] = ((*(PoseTarget **) (racer + 0x360))->points[i][2] - from[2]) / steps;
            from += 3;
            step += 3;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035514);
#endif

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

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words: operand order of the two `cur + ...` add.s instructions */
/* JFG: ease the value at 0x48 towards the target at 0x4C; on arrival the target becomes 1.0. */
void func_800370D0(u8 *obj, f32 rate) {
    f32 cur = *(f32 *) (obj + 0x48);
    f32 target = *(f32 *) (obj + 0x4C);

    if (cur < target) {
        *(f32 *) (obj + 0x48) = cur + ((target - cur) * 0.125f + D_800ABF74) * rate;
        if (target <= *(f32 *) (obj + 0x48)) {
            *(f32 *) (obj + 0x48) = target;
            *(f32 *) (obj + 0x4C) = 1.0f;
        }
    } else {
        *(f32 *) (obj + 0x48) = cur + ((target - cur) * 0.125f - D_800ABF78) * rate;
        if (*(f32 *) (obj + 0x48) <= target) {
            *(f32 *) (obj + 0x48) = target;
            *(f32 *) (obj + 0x4C) = 1.0f;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800370D0);
#endif

void func_8001CE10(s32 arg0, f32 *from, f32 *to, f32 *radius, s32 arg4, s32 arg5, s32 arg6);
s32 func_80016208(void *obj, f32 *from, f32 *to, f32 radius, s32 arg4, void *hit);

/* JFG: 1 unless a collision test from (x, y, z) up by `height` hits a surface with flags 0x48 (0x80037194). */
s32 collision_is_clear_above(f32 x, f32 y, f32 z, f32 height, f32 radius) {
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

extern s32 gPlayerInputLocked;

/* Locks (non-zero) or unlocks a player's latched input (0x80037244). */
void player_input_lock_set(s32 arg0) {
    gPlayerInputLocked = arg0;
}

extern s32 gPlayerButtonsHeld;
extern s32 gPlayerButtonsPressed;
extern s32 gPlayerButtonsReleased;
extern s32 gPlayerStickX;
extern s32 gPlayerStickY;
extern s32 D_800F6DB4;
extern s32 D_800F6DB8;
u16 input_held(s32 player);
u16 input_released(s32 player);
s32 input_clamp_stick_x(s32 player);
s32 input_clamp_stick_y(s32 player);
s8 input_stick_x_raw(s32 player);
s8 input_stick_y_raw(s32 player);

/* JFG: latch a player's input for this frame (all zero when input is off, see player_input_lock_set) (0x80037250). */
void player_input_latch(s32 player) {
    if (player >= 0 && player < 4 && gPlayerInputLocked == 0) {
        gPlayerStickX = input_clamp_stick_x(player);
        D_800F6DB4 = input_stick_x_raw(player);
        gPlayerStickY = input_clamp_stick_y(player);
        D_800F6DB8 = input_stick_y_raw(player);
        gPlayerButtonsHeld = input_held(player);
        gPlayerButtonsPressed = input_pressed(player);
        gPlayerButtonsReleased = input_released(player);
    } else {
        gPlayerStickX = 0;
        D_800F6DB4 = 0;
        gPlayerStickY = 0;
        D_800F6DB8 = 0;
        gPlayerButtonsHeld = 0;
        gPlayerButtonsPressed = 0;
        gPlayerButtonsReleased = 0;
    }
}

extern u8 D_800A2DD8[8];
extern u8 D_800A2DE8[8];
extern s32 D_800A2DF0[8];
void func_8004B3D8(s32 arg0, u8 arg1, s32 arg2);

#ifdef NON_MATCHING
/* NON_MATCHING: the unused third parameter gets a home store (sw a2) the target lacks; rest identical */
void func_8003733C(s32 arg0, s32 flags, UNUSED s32 arg2, s32 index, s32 level) {
    if (index >= 0 && index < 8) {
        flags |= D_800A2DD8[index];
    }
    if (level >= 0x20) {
        if (level >= 0x80) {
            flags |= 4;
        } else if (level >= 0x40) {
            flags |= 2;
        } else {
            flags |= 1;
        }
    }
    if (flags != 0) {
        func_8004B3D8(arg0, D_800A2DE8[flags & 7], D_800A2DF0[flags & 7]);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003733C);
#endif

s32 func_800373E4(u8 *chr, s32 weapon) {
    switch (weapon) {
        case 6:
            return 110;
        case 4:
            return 30;
    }
    return (chr + weapon)[0x54] * 32;
}

/* Returns which way round the weapon slots is shorter to a weapon (1 forwards, 2 backwards) (0x80037420). */
/* JFG: which way round the 16 weapon slots (owned bits at 0xA) is shorter from the current weapon to `weapon`:
   1 = forwards, 2 = backwards. */
s32 weapon_switch_direction(u8 *chr, s32 weapon) {
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

/* Returns whether the current weapon can fire (has ammo) (0x800374C8). */
s32 weapon_has_ammo(u8 *racer, u8 *chr) {
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

#ifdef NON_MATCHING
/* NON_MATCHING: 6 words: register allocation of the loop constants (15 / owned mask) */
/* JFG: switch to the next owned weapon (slots 0..15, wrapping). */
void func_80038324(u8 *obj, Racer *racer) {
    s32 weapon;
    s32 cur;
    u8 *chr;
    u16 bit;
    u16 owned;

    chr = func_80047C1C(racer->player, racer->character);
    cur = chr[0x70];
    bit = 1 << cur;
    owned = *(u16 *) (chr + 0xA);
    weapon = cur;
    do {
        if (weapon == 15) {
            weapon = 0;
            bit = 1;
        } else {
            weapon++;
            bit <<= 1;
        }
    } while (!(owned & bit));
    if (weapon != cur) {
        if (cur == 2 && racer->sound != 0) {
            sound_stop(racer->sound);
            racer->soundId = 0;
        }
        chr[0x70] = weapon;
        racer->flags.unk7 = 0;
        racer->flags.weaponDir |= 2;
        racer->flags.unk30 = 0;
        player_drop_held_object(obj);
        player_items_unlink(racer);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80038324);
#endif

/* JFG: switch to `weapon` if owned and not already selected; returns 1 if switched (0x80038430). */
s32 weapon_select(u8 *obj, Racer *racer, s32 weapon) {
    u8 *chr = func_80047C1C(racer->player, racer->character);
    u8 cur = chr[0x70];

    if (weapon != cur && (*(u16 *) (chr + 0xA) & (1 << weapon))) {
        if (cur == 2 && racer->sound != 0) {
            sound_stop(racer->sound);
            racer->soundId = 0;
        }
        racer->flags.weaponDir = weapon_switch_direction(chr, weapon);
        racer->flags.unk30 = 0;
        racer->flags.unk7 = 0;
        chr[0x70] = weapon;
        player_drop_held_object(obj);
        player_items_unlink(racer);
        return 1;
    }
    return 0;
}

/* JFG: switch to the previous owned weapon (slots 15..0, wrapping) (0x80038528). */
void weapon_select_prev(u8 *obj, Racer *racer) {
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
            sound_stop(racer->sound);
            racer->soundId = 0;
        }
        chr[0x70] = weapon;
        racer->flags.unk7 = 0;
        racer->flags.weaponDir |= 1;
        racer->flags.unk30 = 0;
        player_drop_held_object(obj);
        player_items_unlink(racer);
    }
}

extern u8 D_800A2E10[3];
void light_colour_set(void *light, u8 red, u8 green, u8 blue);

void func_80038630(UNUSED s32 arg0, Racer *racer) {
    if (racer->light != NULL) {
        light_colour_set(racer->light, D_800A2E10[0], D_800A2E10[1], D_800A2E10[2]);
    }
    if (racer->sound != 0) {
        sound_stop(racer->sound);
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
    stats_shot_add(3);
    if (racer->sound != 0) {
        sound_stop(racer->sound);
    }
    sound_play(0xE1, &racer->sound);
    racer->soundId = 0xE1;
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039098);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039308);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039544);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800396CC);

typedef struct SpawnF0 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
} SpawnF0;

s32 rand_range(s32 min, s32 max);

/* JFG: spawn two objects 0xF0 at (x, y, z) with randomised angles. */
void func_800397E0(UNUSED s32 arg0, UNUSED s32 arg1, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7) {
    s32 i;
    u8 *obj;
    SpawnF0 setup;

    setup.objectID = 0xF0;
    setup.size = 0x14;
    setup.unk3 = 0;
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.unk10 = arg7;
    for (i = 0; i != 2;) {
        setup.unkA = rand_range(-0x800, 0x800) + arg5;
        setup.unkC = rand_range(-0x800, 0x800) + arg6;
        setup.unkE = rand_range(2, 4);
        obj = spawn_object(&setup, 1);
        i++;
        if (obj != NULL) {
            *(s32 *) (obj + 0x3C) = 0;
        }
    }
}

typedef struct Spawn72 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ s8 unkA;
    /* 0x0B */ s8 unkB;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ s32 owner;
    /* 0x18 */ u8 unk18;
} Spawn72;

void func_80039918(s32 owner, UNUSED s32 arg1, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8, s32 arg9) {
    u8 *obj;
    Spawn72 setup;

    stats_shot_add(8);
    setup.objectID = 0x72;
    setup.size = 0x1C;
    setup.unk3 = 0;
    setup.unk18 = 0;
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
    sound_play(0x2F, NULL);
    func_8002AEA0(owner, 2);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
    }
}

typedef struct Spawn39 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ s16 padA;
    /* 0x0C */ s32 owner;
} Spawn39;

f32 func_80049C30(s32 angle); /* cosine of a 16-bit angle */
f32 func_80049C34(s32 angle); /* sine of a 16-bit angle */

#ifdef NON_MATCHING
/* NON_MATCHING: 15 words: temp registers in the float-to-s16 conversions are one off (target skips t8) */
/* JFG: spawn object 0x39 at (x, y, z), moving at `speed` along (yaw, pitch). */
void func_80039A38(s32 owner, UNUSED s32 arg1, f32 x, f32 y, f32 z, s16 yaw, s16 pitch, f32 speed) {
    Spawn39 setup;
    u8 *obj;

    setup.objectID = 0x39;
    setup.size = 0xA;
    setup.unk3 = 0;
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.owner = owner;
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(f32 *) (obj + 0x1C) = -func_80049C34(yaw) * func_80049C30(pitch) * speed;
        *(f32 *) (obj + 0x20) = func_80049C34(pitch) * speed;
        *(f32 *) (obj + 0x24) = -func_80049C30(yaw) * func_80049C30(pitch) * speed;
        sound_play(0x2F, NULL);
        func_8002AEA0(owner, 2);
        *(s32 *) (obj + 0x3C) = 0;
        stats_shot_add(9);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039A38);
#endif

typedef struct SpawnGrenade {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ u8 type;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ s32 owner;
    /* 0x18 */ s8 unk18;
    /* 0x19 */ s8 unk19;
} SpawnGrenade;

void func_80039B98(s32 owner, Racer *racer, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8, s32 arg9,
                   u8 type, u8 argB) {
    SpawnGrenade setup;
    u8 *obj;
    s32 weapon;

    if (type == 0) {
        setup.objectID = 0x127;
        weapon = 11;
    } else if (type == 1) {
        setup.objectID = 0x147;
        weapon = 11;
    } else {
        setup.objectID = 0x148;
        weapon = 12;
    }
    setup.size = 0x1C;
    setup.unk3 = 1;
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.type = type;
    setup.unkB = argB;
    setup.unkC = arg5;
    setup.unkE = arg6;
    setup.unk10 = arg7;
    setup.owner = owner;
    setup.unk18 = arg8;
    setup.unk19 = arg9;
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
        if (type == 2) {
            *(u8 **) ((u8 *) racer + 0x1A0) = obj;
        }
        stats_shot_add(weapon);
    }
    sound_play(0x2F, NULL);
    func_8002AEA0(owner, 2);
}

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
        stats_shot_add(14);
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80039FCC);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8003AABC);

extern u8 gWeaponInfo[][0x30];

/* Returns whether the character can use its current weapon (0x8003B114). */
s32 weapon_usable_by_character(s8 *ids) {
    u8 *chr = func_80047C1C(ids[0], ids[1]);
    return (gWeaponInfo[chr[0x70]][0] >> ids[1]) & 1;
}

/* JFG: clear every player's link at 0x1A4 that points to obj. */
void func_8003B16C(s32 obj) {
    u8 **racers;
    s32 numRacers;
    s32 i;
    u8 *racer;

    racers = get_racer_objects(&numRacers);
    for (i = 0; i < numRacers; i++, racers++) {
        racer = *(u8 **) (*racers + 0x68);
        if (obj == *(s32 *) (racer + 0x1A4)) {
            *(s32 *) (racer + 0x1A4) = 0;
        }
    }
}

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

/* JFG: an entry of a table of 0x30-byte records (0x8003B500). */
u8 *weapon_info_get(s32 index) {
    return gWeaponInfo[index];
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

/* JFG: empty a player's hand (the held object at 0x5CC); a held object 0x59 is also stopped and reset (0x8003B820). */
void player_drop_held_object(u8 *obj) {
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
