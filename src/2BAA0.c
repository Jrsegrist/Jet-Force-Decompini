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
    /* 0x002 */ u8 pad2[0x4 - 0x2];
    /* 0x004 */ f32 velX;
    /* 0x008 */ u8 pad8[0x10 - 0x8];
    /* 0x010 */ f32 velZ;
    /* 0x014 */ u8 pad14[0x3C - 0x14];
    /* 0x03C */ f32 pos[3];
    /* 0x048 */ u8 pad48[0x4C - 0x48];
    /* 0x04C */ f32 unk4C;
    /* 0x050 */ u8 pad50[0x60 - 0x50];
    /* 0x060 */ f32 unk60;
    /* 0x064 */ u8 pad64[0x6C - 0x64];
    /* 0x06C */ f32 pushX;
    /* 0x070 */ f32 pushZ;
    /* 0x074 */ u8 pad74[0xB8 - 0x74];
    /* 0x0B8 */ u16 lastVoice;
    /* 0x0BA */ u8 padBA[2];
    /* 0x0BC */ void *voice;
    /* 0x0C0 */ s32 sound;
    /* 0x0C4 */ u8 padC4[0x104 - 0xC4];
    /* 0x104 */ s16 unk104; // the chase camera's yaw
    /* 0x106 */ u8 pad106[0x10A - 0x106];
    /* 0x10A */ s16 unk10A;
    /* 0x10C */ u8 pad10C[0x11C - 0x10C];
    /* 0x11C */ s16 unk11C;
    /* 0x11E */ u8 pad11E[0x122 - 0x11E];
    /* 0x122 */ s16 unk122;
    /* 0x124 */ u8 pad124[2];
    /* 0x126 */ s16 unk126;
    /* 0x128 */ u8 pad128[0x12E - 0x128];
    /* 0x12E */ s8 breathTimer;
    /* 0x12F */ u8 breathNow;
    /* 0x130 */ f32 breathPhase;
    /* 0x134 */ f32 breathLevel;
    /* 0x138 */ u8 pad138[0x13C - 0x138];
    /* 0x13C */ s16 unk13C;
    /* 0x13E */ u8 unk13E;
    /* 0x13F */ u8 unk13F;
    /* 0x140 */ u8 pad140[0x149 - 0x140];
    /* 0x149 */ u8 unk149;
    /* 0x14A */ u8 pad14A[0x14C - 0x14A];
    /* 0x14C */ f32 segStart[3]; // a segment in the platform's local space, and the position along it
    /* 0x158 */ f32 segEnd[3];
    /* 0x164 */ f32 segT;
    /* 0x168 */ f32 *segNormal;
    /* 0x16C */ s32 unk16C;
    /* 0x170 */ f32 unk170;
    /* 0x174 */ f32 unk174;
    /* 0x178 */ f32 unk178;
    /* 0x17C */ s16 unk17C;
    /* 0x17E */ s16 unk17E;
    /* 0x180 */ s16 unk180;
    /* 0x182 */ s16 unk182;
    /* 0x184 */ s8 unk184;
    /* 0x185 */ u8 pad185;
    /* 0x186 */ s8 squashTimer;
    /* 0x187 */ u8 squashEvent;
    /* 0x188 */ u8 squashCheck;
    /* 0x189 */ u8 pad189[0x198 - 0x189];
    /* 0x198 */ u8 unk198;
    /* 0x199 */ u8 pad199[0x19C - 0x199];
    /* 0x19C */ s8 unk19C;
    /* 0x19D */ u8 unk19D;
    /* 0x19E */ u8 pad19E[0x1A4 - 0x19E];
    /* 0x1A4 */ s32 unk1A4;
    /* 0x1A8 */ u8 *items[6];
    /* 0x1C0 */ u8 pad1C0[0x1D4 - 0x1C0];
    /* 0x1D4 */ s32 unk1D4;
    /* 0x1D8 */ u8 pad1D8[0x1E2 - 0x1D8];
    /* 0x1E2 */ s16 unk1E2;
    /* 0x1E4 */ u8 pad1E4[0x1F4 - 0x1E4];
    /* 0x1F4 */ s8 unk1F4;
    /* 0x1F5 */ u8 pad1F5[0x1F9 - 0x1F5];
    /* 0x1F9 */ u8 unk1F9;
    /* 0x1FA */ u8 unk1FA;
    /* 0x1FB */ u8 pad1FB[0x360 - 0x1FB];
    /* 0x360 */ u8 *pose;
    /* 0x364 */ f32 points[8][3]; // the pose points (relative to the object)
    /* 0x3C4 */ u8 pad3C4[0x484 - 0x3C4];
    /* 0x484 */ f32 posePoints[8][3];
    /* 0x4E4 */ u8 pad4E4[0x510 - 0x4E4];
    /* 0x510 */ s16 *platform; // the platform object stood on
    /* 0x514 */ void *squashObj;
    /* 0x518 */ u8 pad518[0x524 - 0x518];
    /* 0x524 */ f32 prevPos[3];
    /* 0x530 */ s8 numPoints;
    /* 0x531 */ u8 pointMask;
    /* 0x532 */ u8 pad532[0x53A - 0x532];
    /* 0x53A */ u16 soundId;
    /* 0x53C */ u8 pad53C[0x540 - 0x53C];
    /* 0x540 */ RacerFlags flags;
    /* 0x544 */ f32 overridePos[3];
    /* 0x550 */ u8 pad550[0x558 - 0x550];
    /* 0x558 */ void *light;
    /* 0x55C */ u8 pad55C[0x560 - 0x55C];
    /* 0x560 */ u8 *attachment;
    /* 0x564 */ u8 pad564[0x568 - 0x564];
    /* 0x568 */ u8 unk568;
    /* 0x569 */ u8 unk569;
    /* 0x56A */ u8 pad56A[0x574 - 0x56A];
    /* 0x574 */ u8 unk574;
    /* 0x575 */ u8 pad575;
    /* 0x576 */ u8 unk576;
    /* 0x577 */ u8 pad577;
    /* 0x578 */ s8 unk578;
    /* 0x579 */ s8 unk579;
    /* 0x57A */ u8 unk57A;
    /* 0x57B */ u8 pad57B[0x5CC - 0x57B];
    /* 0x5CC */ u8 *held;
} Racer;

s32 func_8003B51C(s32 value, s32 delta);
void player_voice_play(s32 owner, u8 index);
u8 *func_80047C1C(s32 player, s32 character);
void player_drop_held_object(u8 *obj);
void *spawn_object(void *entry, s32 arg1);
void stats_shot_add(s32 weapon);
void sound_stop(s32 soundHandle);
void sound_play(u16 soundID, s32 *handlePtr);

typedef struct SoundSet {
    /* 0x00 */ u8 count;
    /* 0x01 */ u8 flags; // 1: may cut off the sound still playing
    /* 0x04 */ s16 *sounds;
} SoundSet;

extern SoundSet D_800A26C4[];
extern f32 D_800ABDA8;
s32 rand_range(s32 min, s32 max);
f32 sqrtf(f32 x);
void audspat_play_sound_at_position(u16 soundId, f32 x, f32 y, f32 z, u8 flags, void **handlePtr);
void enemy_module_notify(u8 *arg0, s16 x, s16 y, s16 z, u8 arg4, u8 arg5, u8 arg6);
void audspat_point_stop(void *handle);

/* JFG: play a random sound of set `index` (per character, never the last one again) at obj, keeping its handle
 * in the player's 0xBC; a sound still playing is stopped first if the set allows it, otherwise nothing plays. */
void player_voice_play(s32 owner, u8 index) {
    u8 *obj = (u8 *) owner;
    Racer *racer = *(Racer **) (obj + 0x68);
    SoundSet *set;
    s16 *sounds;
    s16 sound;

    if (racer->voice != NULL) {
        if (!(D_800A26C4[index].flags & 1)) {
            return;
        }
        audspat_point_stop(racer->voice);
    }
    set = &D_800A26C4[index];
    sounds = set->count * (racer->character & 3) + set->sounds;
    if (set->count == 1) {
        racer->lastVoice = 0x7FFF;
    }
    sound = racer->lastVoice;
    while (sound == racer->lastVoice) {
        sound = sounds[rand_range(0, set->count - 1)];
    }
    audspat_play_sound_at_position(sound, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14), 4,
                                   &racer->voice);
    enemy_module_notify(obj + 1, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14), 1, 1, sound);
    racer->lastVoice = sound;
}

/* JFG: play a random sound of set `index` (per character) at obj if it is moving fast enough. */
void obj_play_move_sound(u8 *obj, u8 index) {
    Racer *racer;
    s16 *sounds;
    s16 sound;
    SoundSet *set;

    racer = *(Racer **) (obj + 0x68);
    set = &D_800A26C4[index];
    sounds = set->count * (racer->character & 3) + set->sounds;
    sound = sounds[rand_range(0, set->count - 1)];
    if (D_800ABDA8 < sqrtf(*(f32 *) (obj + 0x24) * *(f32 *) (obj + 0x24) + *(f32 *) (obj + 0x1C) * *(f32 *) (obj + 0x1C))) {
        audspat_play_sound_at_position(sound, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14), 4, NULL);
        enemy_module_notify(obj + 1, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14), 1, 1, sound);
    }
}

/* JFG: a sound event of an animation: at `frame` (0..1) play voice set `id` (kind 0) or move sound set `id`
 * (kind 1) */
typedef struct AnimSoundEvent {
    /* 0x00 */ f32 frame;
    /* 0x04 */ u8 kind;
    /* 0x05 */ u8 id;
} AnimSoundEvent;

/* JFG: the sound events of one animation of a character; lists end with anim -1 */
typedef struct AnimSoundList {
    /* 0x00 */ s8 anim;
    /* 0x04 */ AnimSoundEvent *events;
    /* 0x08 */ u8 count;
} AnimSoundList;

extern AnimSoundList *D_800A2BC0[];
extern s16 D_800A265C[];
extern s16 D_800A2664[];
extern s16 D_800A266C[];
extern s16 D_800A2674[];

/* JFG: plays the sound events of the object's current animation (0x3B) whose frames lie between `from` and the
 * object's frame (0x28), both mirrored when playing backwards, wrapping past 1. Sets 4/5 (footsteps) follow 0x60. */
void anim_sound_events_play(u8 *obj, f32 from, f32 speed) {
    Racer *racer = *(Racer **) (obj + 0x68);
    AnimSoundList *list;
    AnimSoundEvent *event;
    s32 i;
    s32 wrap;
    f32 to;
    f32 frame;

    if (racer->unk60 > 0.0f) {
        D_800A26C4[4].sounds = D_800A266C;
        D_800A26C4[5].sounds = D_800A2674;
    } else {
        D_800A26C4[4].sounds = D_800A265C;
        D_800A26C4[5].sounds = D_800A2664;
    }
    list = D_800A2BC0[racer->character];
    if (list != NULL) {
        to = *(f32 *) (obj + 0x28);
        if (speed < 0.0f) {
            to = 1.0f - to;
            from = 1.0f - from;
        }
        if (to < from) {
            to += 1.0f;
            wrap = TRUE;
        } else {
            wrap = FALSE;
        }
        while (list->anim != -1) {
            if (list->anim == *(s8 *) (obj + 0x3B)) {
                event = list->events;
                for (i = 0; i < list->count; i++, event++) {
                    frame = event->frame;
                    if (wrap && frame <= from) {
                        frame += 1.0f;
                    }
                    if (from <= frame && frame <= to) {
                        switch (event->kind) {
                            case 0:
                                player_voice_play((s32) obj, event->id);
                                break;
                            case 1:
                                obj_play_move_sound(obj, event->id);
                                break;
                        }
                    }
                }
            }
            list++;
        }
    }
}

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

typedef struct Transform2BAA0 {
    /* 0x00 */ s16 rotation[3];
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 scale;
    /* 0x0C */ f32 position[3];
    /* 0x18 */ f32 prevPosition[3];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x3D - 0x2C];
    /* 0x3D */ u8 unk3D;
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ f32 unk40;
    /* 0x44 */ u8 colour[4];
    /* 0x48 */ u8 pad48;
    /* 0x49 */ u8 unk49;
} Transform2BAA0;

extern f32 D_800ABDB0;
extern f32 D_800ABDB4;
void *cam_get_active_camera(void);
s16 angle_lerp(s16 from, s16 to, f32 t);
f32 func_80049BEC(f32 rate, s32 updateRate);
void func_800496B0(s32, s16 *, void *, u8 *);

/* JFG: the over-the-shoulder camera: pitched with the player's aim (0x1E2, less for weapon 7), pulled in as the aim
 * leaves +-0x38E, 60 units up; the camera eases to it */
void player_camera_eye_place(s16 *obj, Racer *racer, f32 updateRate) {
    UNUSED s32 pad;
    s16 angles[3];
    f32 x;
    f32 y;
    f32 z;
    UNUSED s32 pad2[2];
    f32 off[3];
    f32 eye[3];
    u8 *chr;
    f32 tilt;

    chr = func_80047C1C(racer->player, racer->character);
    racer->unk10A = 0;
    *(f32 *) ((u8 *) cam_get_active_camera() + 0x2C) = 60.0f;
    angles[0] = obj[0];
    angles[1] = -obj[1];
    angles[2] = 0;
    if (chr[0x70] != 7) {
        if (racer->unk1E2 > 0) {
            angles[1] -= (racer->unk1E2 * 0xDC) >> 8;
        } else {
            angles[1] -= (racer->unk1E2 * 0xA0) >> 8;
        }
    } else {
        angles[1] -= racer->unk1E2;
    }
    tilt = 0.0f;
    if (racer->unk1E2 > 0x38E) {
        tilt = (racer->unk1E2 - 0x38E) / D_800ABDB0;
    }
    if (racer->unk1E2 < -0x38E) {
        tilt = (-0x38E - racer->unk1E2) / D_800ABDB4;
    }
    if (tilt > 1.0f) {
        tilt = 1.0f;
    }
    off[0] = 0.0f;
    off[1] = 0.0f;
    off[2] = -105.0f * tilt + 150.0f;
    angles[1] = -angles[1];
    tilt = 1.0f;
    func_800496B0(1, angles, off, (u8 *) off);
    eye[0] = 0.0f;
    eye[2] = 0.0f;
    eye[1] = 60.0f;
    func_800496B0(1, obj, eye, (u8 *) eye);
    x = (*(f32 *) (obj + 6) + off[0]) + eye[0];
    y = (*(f32 *) (obj + 8) + off[1]) + eye[1];
    z = (*(f32 *) (obj + 10) + off[2]) + eye[2];
    if (racer->unk574) {
        tilt = 1.0f - func_80049BEC(0.875f, updateRate);
    }
    ((Transform2BAA0 *) D_800F6DC0)->rotation[0] =
        angle_lerp(((Transform2BAA0 *) D_800F6DC0)->rotation[0], 0x8000 - angles[0], tilt);
    ((Transform2BAA0 *) D_800F6DC0)->rotation[1] =
        angle_lerp(((Transform2BAA0 *) D_800F6DC0)->rotation[1], -angles[1], tilt);
    ((Transform2BAA0 *) D_800F6DC0)->rotation[2] = angle_lerp(((Transform2BAA0 *) D_800F6DC0)->rotation[2], angles[2], tilt);
    tilt = 1.0f;
    if (racer->unk574) {
        tilt = 1.0f - func_80049BEC(0.875f, updateRate);
    }
    ((Transform2BAA0 *) D_800F6DC0)->position[0] += (x - ((Transform2BAA0 *) D_800F6DC0)->position[0]) * tilt;
    ((Transform2BAA0 *) D_800F6DC0)->position[1] += (y - ((Transform2BAA0 *) D_800F6DC0)->position[1]) * tilt;
    ((Transform2BAA0 *) D_800F6DC0)->position[2] += (z - ((Transform2BAA0 *) D_800F6DC0)->position[2]) * tilt;
}

extern f32 D_800ABDB8;
extern f32 D_800ABDBC;
extern f32 D_800ABDC0;
extern f32 D_800ABDC4;
f32 sqrtf(f32 x);
f32 func_80049C34(s32 angle);
f32 func_80049C30(s32 angle);
s32 func_80049C84(f32 x, f32 z);

#define CHASECAM ((Transform2BAA0 *) D_800F6DC0)

/* JFG: the chase camera: its yaw (racer 0x104) eases to behind the player, its distance to 120 and height to 50;
 * the camera eases to that point, then turns to look at a point 45 up and 80 ahead of the player */
void func_8002BD04(s16 *obj, Racer *racer, f32 updateRate) {
    f32 dist;
    s16 angle;
    f32 dx;
    f32 dz;
    f32 y;
    f32 dy;
    s32 rate;
    f32 t;
    s16 pitch;
    f32 look[3];

    dx = CHASECAM->position[0] - *(f32 *) (obj + 6);
    dz = CHASECAM->position[2] - *(f32 *) (obj + 10);
    dy = CHASECAM->position[1] - *(f32 *) (obj + 8);
    dist = sqrtf(dx * dx + dz * dz);
    rate = updateRate;
    angle = 0x6000 - obj[0];
    angle = angle_lerp(racer->unk104, angle, 1.0f - func_80049BEC(D_800ABDB8, rate));
    dz = D_800ABDBC;
    racer->unk104 = angle;
    dist = (1.0f - func_80049BEC(dz, rate)) * (120.0f - dist) + dist;
    dy = (1.0f - func_80049BEC(dz, rate)) * (50.0f - dy) + dy;
    dx = *(f32 *) (obj + 6) + func_80049C34(racer->unk104) * dist;
    y = *(f32 *) (obj + 8) + dy;
    dz = *(f32 *) (obj + 10) - func_80049C30(racer->unk104) * dist;
    t = 1.0f - func_80049BEC(D_800ABDC0, rate);
    CHASECAM->position[0] += (dx - CHASECAM->position[0]) * t;
    CHASECAM->position[1] += (y - CHASECAM->position[1]) * t;
    CHASECAM->position[2] += (dz - CHASECAM->position[2]) * t;
    look[0] = 0.0f;
    look[1] = 45.0f;
    look[2] = -80.0f;
    func_800496B0(1, obj, look, (u8 *) look);
    dx = CHASECAM->position[0] - (*(f32 *) (obj + 6) + look[0]);
    y = CHASECAM->position[1] - (*(f32 *) (obj + 8) + look[1]);
    dz = CHASECAM->position[2] - (*(f32 *) (obj + 10) + look[2]);
    angle = 0x8000 - func_80049C84(dx, dz);
    pitch = func_80049C84(y, sqrtf(dx * dx + dz * dz));
    t = 1.0f - func_80049BEC(D_800ABDC4, rate);
    CHASECAM->rotation[0] = angle_lerp(CHASECAM->rotation[0], angle, t);
    CHASECAM->rotation[1] = angle_lerp(CHASECAM->rotation[1], pitch, t);
    CHASECAM->rotation[2] = angle_lerp(CHASECAM->rotation[2], 0, t);
}


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
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
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

extern s32 gPlayerButtonsPressed;
s16 angle_lerp(s16 from, s16 to, f32 t);
f32 func_80049BEC(f32 rate, s32 updateRate);
/* The module trap called with floats in a2/a3; needs `func_80054530_ppfff = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_ppfff(u8 *arg0, u8 *arg1, f32 x, f32 y, f32 z);
void func_80054530();

#ifdef NON_MATCHING
/* NON_MATCHING: s0/s1 swapped (the target keeps obj then racer in s0, the view address in s1) and the target stores
   the free-view angles through %lo(D_800F6DC8) rather than the address register; frame and order match */
/* JFG: player camera update: C buttons toggle the free view (0x3D); the camera module (racer 0x1FC/0x21C) is
 * given the object's position (and angles in the free view), and the camera eases to the result */
void func_8002C9A0(u8 *obj, u8 *racer, f32 updateRate) {
    f32 x;
    f32 y;
    f32 z;
    f32 t;
    s16 rx;
    s16 ry;
    s16 rz;
    ViewPoint *view;
    UNUSED s32 pad;

    if (gPlayerButtonsPressed & 0xF00) {
        ((Transform2BAA0 *) D_800F6DC0)->unk3D = (((Transform2BAA0 *) D_800F6DC0)->unk3D + 1) & 1;
    }
    view = &D_800F6DC8;
    view->position[0] = *(f32 *) (obj + 0xC);
    view->position[1] = *(f32 *) (obj + 0x10);
    view->unk14 = 0.0f;
    view->unk18 = 0.0f;
    view->position[2] = *(f32 *) (obj + 0x14);
    view->unk1C = -48.0f;
    if (((Transform2BAA0 *) D_800F6DC0)->unk3D) {
        D_800F6DC8.rotation[0] = *(s16 *) (obj + 0);
        D_800F6DC8.rotation[1] = *(s16 *) (obj + 2);
        D_800F6DC8.unk6 = 0;
        D_800F6DC8.rotation[2] = *(s16 *) (obj + 4);
    } else {
        D_800F6DC8.unk6 = 2;
    }
    func_80054530_ppfff(racer + 0x1FC, racer + 0x21C, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10),
                        *(f32 *) (obj + 0x14));
    func_80054530(racer + 0x1FC, racer + 0x21C, view, &x, &y, &z, &rx, &ry, &rz);
    t = 1.0f;
    if (racer[0x574]) {
        t = 1.0f - func_80049BEC(0.875f, updateRate);
    }
    ((Transform2BAA0 *) D_800F6DC0)->rotation[0] = angle_lerp(((Transform2BAA0 *) D_800F6DC0)->rotation[0], rx, t);
    ((Transform2BAA0 *) D_800F6DC0)->rotation[1] = angle_lerp(((Transform2BAA0 *) D_800F6DC0)->rotation[1], ry, t);
    ((Transform2BAA0 *) D_800F6DC0)->rotation[2] = angle_lerp(((Transform2BAA0 *) D_800F6DC0)->rotation[2], rz, t);
    t = 1.0f;
    if (racer[0x574]) {
        t = 1.0f - func_80049BEC(0.875f, updateRate);
    }
    ((Transform2BAA0 *) D_800F6DC0)->position[0] += (x - ((Transform2BAA0 *) D_800F6DC0)->position[0]) * t;
    ((Transform2BAA0 *) D_800F6DC0)->position[1] += (y - ((Transform2BAA0 *) D_800F6DC0)->position[1]) * t;
    ((Transform2BAA0 *) D_800F6DC0)->position[2] += (z - ((Transform2BAA0 *) D_800F6DC0)->position[2]) * t;
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002C9A0);
#endif

/* JFG: what func_80018808 writes back here (the object hit, then the point) */
typedef struct TrackHit2BAA0 {
    /* 0x00 */ struct {
        u8 pad0[0x48];
        s16 behaviorId;
    } *obj;
    /* 0x04 */ f32 pos[3];
    /* 0x10 */ u8 pad10[0x28 - 0x10];
} TrackHit2BAA0;

s32 func_80018808(s32 arg0, f32 *from, f32 *to, TrackHit2BAA0 *result, s32 flags, s32 arg5);
s32 func_8001A218(f32 x, f32 z, f32 bottom, f32 top, f32 radius, s32 arg5, s32 arg6, f32 ***heights);

#ifdef NON_MATCHING
/* NON_MATCHING: the last (empty) search loop: the target guards it with n > 0 and builds &heights[i] from an i
   still in $zero; all else matches */
/* JFG: move pos to where the track stops the way from `from` (passing through objects 0x30/0x31 once), then out
   of the floors and ceilings around it; on a clear way also not below its starting height. The int copies of x
   and z are unused (the target keeps only their rounding-mode switches). */
void func_8002CC70(f32 *from, f32 *pos, UNUSED s32 arg2) {
    f32 **p;
    TrackHit2BAA0 hit;
    s32 i;
    s32 done;
    s32 mode;
    s32 x;
    s32 z;
    s32 n;
    f32 yOrig;
    f32 start[3];
    f32 lo;
    f32 hi;
    f32 **heights;

    if (&from) {}
    start[0] = from[0];
    mode = 1;
    start[1] = from[1];
    done = FALSE;
    start[2] = from[2];
    yOrig = pos[1];
    do {
        if (func_80018808(0, start, pos, &hit, 0, 0) == 0) {
            done = TRUE;
        } else if (mode != 2 && hit.obj != NULL && (hit.obj->behaviorId == 0x30 || hit.obj->behaviorId == 0x31)) {
            mode = 2;
            start[0] = hit.pos[0];
            start[1] = hit.pos[1];
            start[2] = hit.pos[2];
            pos[1] = hit.pos[1];
        } else {
            mode = 3;
            pos[0] = hit.pos[0];
            done = TRUE;
            pos[1] = hit.pos[1];
            pos[2] = hit.pos[2];
        }
    } while (!done);
    x = pos[0];
    z = pos[2];
    if (x) {}
    if (z) {}
    n = func_8001A218(pos[0], pos[2], -32000.0f, 32000.0f, 16.0f, 0xC00, 1, &heights);
    while (n--) {
        hi = *heights[n] + 1.0f;
        lo = *heights[n] - 16.0f;
        if (lo < pos[1] && pos[1] < hi) {
            pos[1] = lo;
        }
    }
    n = func_8001A218(pos[0], pos[2], -32000.0f, 32000.0f, 16.0f, 0xC00, 0, &heights);
    while (n--) {
        hi = *heights[n] + 16.0f;
        lo = *heights[n] - 1.0f;
        if (lo < pos[1] && pos[1] < hi) {
            pos[1] = hi;
        }
    }
    if (mode == 1) {
        n = func_8001A218(pos[0], pos[2], -32000.0f, 32000.0f, 16.0f, 0xC00, 0, &heights);
        if (n != 0) {
            done = FALSE;
            i = 0;
            for (p = &heights[i] + 1; p < &heights[n] && !done; p++) {
            }
        }
        if (pos[1] < yOrig) {
            pos[1] = yOrig;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002CC70);
#endif

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002CF6C);

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002EB6C);

/* JFG: the zone of a zone object (its 0x68 block): an entry circle and a larger exit one, each with a height
 * range that applies when the flag is set */
typedef struct TrackZone {
    /* 0x00 */ f32 enterRadius;
    /* 0x04 */ f32 enterTop;
    /* 0x08 */ f32 enterBottom;
    /* 0x0C */ f32 exitRadius;
    /* 0x10 */ f32 exitTop;
    /* 0x14 */ f32 exitBottom;
    /* 0x18 */ s32 flags; // 0x4000: enter height range, 0x8000: exit height range
    /* 0x1C */ f32 offsetX;
    /* 0x20 */ f32 offsetZ;
    /* 0x24 */ f32 pos[3];
} TrackZone;

/* Size: 0x2C bytes */
typedef struct ZoneTrack {
    /* 0x00 */ u8 *obj;
    /* 0x04 */ TrackZone *zone;
    /* 0x08 */ f32 offsetX;
    /* 0x0C */ f32 offsetZ;
    /* 0x10 */ f32 unk10[7];
} ZoneTrack;
extern ZoneTrack D_800F6E58[4];

void *cam_get_active_camera(void);
void func_8002B418(u8 *obj, Racer *racer, f32 rate);

/* JFG: set up the player camera behind obj: angles 0, distances 48/150/40, height obj + 80, white, then clear the
 * player's zone track and run the camera update eight times to settle it */
void player_camera_setup(u8 *obj, Racer *racer) {
    s32 i;

    D_800F6DC0 = cam_get_active_camera();
    ((Transform2BAA0 *) D_800F6DC0)->rotation[2] = 0;
    ((Transform2BAA0 *) D_800F6DC0)->rotation[1] = 0;
    ((Transform2BAA0 *) D_800F6DC0)->rotation[0] = 0;
    ((Transform2BAA0 *) D_800F6DC0)->unk28 = 48.0f;
    ((Transform2BAA0 *) D_800F6DC0)->unk3D = 0;
    ((Transform2BAA0 *) D_800F6DC0)->colour[0] = 0xFF;
    ((Transform2BAA0 *) D_800F6DC0)->colour[1] = 0xFF;
    ((Transform2BAA0 *) D_800F6DC0)->colour[2] = 0xFF;
    ((Transform2BAA0 *) D_800F6DC0)->colour[3] = 0xFF;
    ((Transform2BAA0 *) D_800F6DC0)->unk40 = 0.0f;
    ((Transform2BAA0 *) D_800F6DC0)->unk24 = 150.0f;
    ((Transform2BAA0 *) D_800F6DC0)->unk28 = 40.0f;
    ((Transform2BAA0 *) D_800F6DC0)->position[1] = *(f32 *) (obj + 0x10) + 80.0f;
    ((Transform2BAA0 *) D_800F6DC0)->prevPosition[0] = ((Transform2BAA0 *) D_800F6DC0)->position[0];
    ((Transform2BAA0 *) D_800F6DC0)->prevPosition[1] = ((Transform2BAA0 *) D_800F6DC0)->position[1];
    ((Transform2BAA0 *) D_800F6DC0)->prevPosition[2] = ((Transform2BAA0 *) D_800F6DC0)->position[2];
    ((Transform2BAA0 *) D_800F6DC0)->unk49 = 1;
    if (racer->player >= 0 && racer->player < 4) {
        D_800F6E58[racer->player].obj = NULL;
        D_800F6E58[racer->player].offsetX = 0.0f;
        D_800F6E58[racer->player].offsetZ = 0.0f;
        D_800F6E58[racer->player].unk10[0] = 1.0f;
        D_800F6E58[racer->player].unk10[1] = 1.0f;
        D_800F6E58[racer->player].unk10[2] = 0.0f;
        D_800F6E58[racer->player].unk10[3] = 0.0f;
        D_800F6E58[racer->player].unk10[4] = 0.0f;
        D_800F6E58[racer->player].unk10[5] = 1.0f;
        D_800F6E58[racer->player].unk10[6] = 0.0f;
    }
    racer->unk574 = 0;
    for (i = 0; i < 8; i++) {
        func_8002B418(obj, racer, 1.0f);
    }
    racer->unk574 = 1;
}

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

#ifdef NON_MATCHING
/* NON_MATCHING: float registers: the target keeps z and y in $f20/$f22 (dz in $f14), mine z stays in $f14 and
   only y gets a saved register */
/* JFG: for a player at (x, y, z): leaves its current zone object when outside its exit circle (or height range),
 * then if it has none, takes the last listed zone object whose entry circle (and height range) contains it. */
void func_8002F0DC(s32 player, f32 x, f32 y, f32 z) {
    ZoneTrack *track;
    u8 *obj;
    TrackZone *zone;
    s32 i;
    f32 dx;
    f32 dz;

    if (player < 0 || player >= 4) {
        return;
    }
    track = &D_800F6E58[player];
    obj = track->obj;
    if (obj != NULL && (zone = track->zone) != NULL) {
        dx = *(f32 *) (obj + 0xC) - x;
        dz = *(f32 *) (obj + 0x14) - z;
        if (zone->exitRadius * zone->exitRadius < dx * dx + dz * dz) {
            track->obj = NULL;
            obj = NULL;
        } else if (zone->flags & 0x8000) {
            if (y < zone->exitBottom || zone->exitTop < y) {
                track->obj = NULL;
                obj = NULL;
            }
        }
    }
    if (obj == NULL) {
        for (i = 0; i < D_800A2D38; i++) {
            obj = (u8 *) D_800F6DF8[i];
            zone = *(TrackZone **) (obj + 0x68);
            dx = *(f32 *) (obj + 0xC) - x;
            dz = *(f32 *) (obj + 0x14) - z;
            if (dx * dx + dz * dz < zone->enterRadius * zone->enterRadius) {
                track->obj = obj;
                track->zone = zone;
                if (zone->flags & 0x4000) {
                    if (y < zone->enterBottom || zone->enterTop < y) {
                        track->obj = NULL;
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_8002F0DC);
#endif

extern f32 D_800ABE3C;
f32 func_80049BEC(f32 rate, s32 updateRate);

/* JFG: eases the player's zone offset towards its current zone's (x/z offsets by flags 1/2), adds it to *x and
 * *z; flag 4 gives a fixed position, flag 8 clear clears *keep */
void player_zone_offset_update(s32 player, f32 *z, f32 *x, s32 *hasPos, f32 *pos, s32 *keep, s32 ease, s32 updateRate) {
    UNUSED s32 pad;
    f32 offsetX;
    f32 offsetZ;
    ZoneTrack *track;
    TrackZone *zone;
    f32 t;

    *hasPos = 0;
    *keep = 1;
    if (player >= 0 && player < 4) {
        track = &D_800F6E58[player];
        offsetX = 0.0f;
        offsetZ = 0.0f;
        if (track->obj != NULL && (zone = track->zone) != NULL) {
            if (zone->flags & 1) {
                offsetX += zone->offsetX;
            }
            if (zone->flags & 2) {
                offsetZ += zone->offsetZ;
            }
            if (zone->flags & 4) {
                *hasPos = 1;
                pos[0] = zone->pos[0];
                pos[1] = zone->pos[1];
                pos[2] = zone->pos[2];
            }
            if (!(zone->flags & 8)) {
                *keep = 0;
            }
        }
        t = 1.0f;
        if (ease) {
            t = 1.0f - func_80049BEC(D_800ABE3C, updateRate);
        }
        track->offsetX += (offsetX - track->offsetX) * t;
        track->offsetZ += (offsetZ - track->offsetZ) * t;
        *x += track->offsetX;
        *z += track->offsetZ;
    }
}

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
void camera_point_around_player(u8 *obj, f32 *x, f32 *y, f32 *z, s32 arg4);
void ignore_bounds_check(void);
s32 move_object(u8 *obj, f32 xPos, f32 yPos, f32 zPos);
void func_80079D1C(u8 *obj, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, f32 arg7, f32 arg8,
                   f32 arg9, f32 argA, s32 argB);

void func_8002F5F0(u8 *obj, u8 *arg1) {
    f32 *state = *(f32 **) (obj + 0x68);

    *(s32 *) (state + 9) = *(s32 *) (arg1 + 0xC);
    D_800A2D40 = 1.0f;
    camera_point_around_player(obj, &state[0], &state[1], &state[2], 0);
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

/* JFG: object line query (as 7A7D0's CollQuery, 0x54 bytes) */
typedef struct LineQuery {
    /* 0x00 */ f32 unk0[3];
    /* 0x0C */ f32 end[3];
    /* 0x18 */ f32 start[3];
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x54 - 0x2C];
} LineQuery;

/* JFG: the aim lock of an object's state block (0x68) */
typedef struct AimLock {
    /* 0x00 */ u8 pad0[0x50];
    /* 0x50 */ s32 target[3];
    /* 0x5C */ u8 charge;
    /* 0x5D */ s8 lockTimer;
    /* 0x5E */ s16 holdTimer;
} AimLock;

u8 **objGetObjList(s32 *start, s32 *end);
s32 func_8007C2E8(s32 arg0, void *query, s32 count, u8 **objs, u8 **hitObj, u8 *self);
f32 sqrtf(f32 x);

/* JFG: how many solid objects (interaction kind 1 or 2, not types 1 or 0x98) the line from start to end hits;
 * keeps the aim lock on the hit object (locks after 20 frames on the same one, holds 90 after losing it), *dist
 * gets a distance to it */
s32 line_aim_lock_test(u8 *obj, f32 *start, f32 *end, f32 *dist, s32 updateRate) {
    s32 count;
    s32 first;
    s32 last;
    LineQuery query;
    u8 **objs;
    u8 *hitObj;
    AimLock *state;
    s32 n;
    u8 *o;
    u8 *list[256];

    state = *(AimLock **) (obj + 0x68);
    query.start[0] = start[0];
    query.start[1] = start[1];
    query.start[2] = start[2];
    count = 0;
    query.end[0] = end[0];
    query.end[1] = end[1];
    query.end[2] = end[2];
    query.unk24 = 1.0f;
    query.unk28 = 1.0f;
    objs = objGetObjList(&first, &last);
    n = last - first;
    objs = first + objs;
    while (n--) {
        o = *objs++;
        if (*(u8 **) (o + 0x4C) != NULL && (*(u16 *) (*(u8 **) (o + 0x40) + 0x14) & 0x40) &&
            ((*(u8 **) (o + 0x4C))[0x30] == 1 || (*(u8 **) (o + 0x4C))[0x30] == 2) && *(s16 *) (o + 0x48) != 1 &&
            *(s16 *) (o + 0x48) != 0x98) {
            list[count++] = o;
        }
    }
    if (count > 0) {
        count = func_8007C2E8(0, &query, count, list, &hitObj, obj);
    }
    state->charge += updateRate;
    if (state->charge > 32) {
        state->charge = 32;
    }
    if (count > 0) {
        if (state->target[0] == ((s32 *) hitObj)[0]) {
            state->lockTimer -= updateRate;
            if (state->lockTimer <= 0) {
                state->lockTimer = 0;
                state->holdTimer = 90;
            }
        } else {
            state->lockTimer = 20;
            state->holdTimer = 0;
            state->charge = 0;
        }
        state->target[0] = ((s32 *) hitObj)[0];
        state->target[1] = ((s32 *) hitObj)[1];
        state->target[2] = ((s32 *) hitObj)[2];
        /* (sic: the same term three times) */
        *dist = sqrtf((query.start[0] - *(f32 *) (hitObj + 0x10)) * (query.start[0] - *(f32 *) (hitObj + 0x10)) +
                      ((query.start[0] - *(f32 *) (hitObj + 0x10)) * (query.start[0] - *(f32 *) (hitObj + 0x10)) +
                       (query.start[0] - *(f32 *) (hitObj + 0x10)) * (query.start[0] - *(f32 *) (hitObj + 0x10))));
    } else {
        state->holdTimer -= updateRate;
        if (state->holdTimer <= 0) {
            state->target[0] = 0;
            state->lockTimer = 20;
        }
    }
    return count;
}

extern f32 D_800A2D74[5][3];
void func_800496B0(s32, s16 *, void *, u8 *);
void func_8001CE10(s32 arg0, f32 *from, f32 *to, f32 *radius, s32 arg4, s32 arg5, s32 arg6);
s32 func_80016208(void *obj, f32 *from, f32 *to, f32 radius, s32 arg4, void *hit);

/* JFG: the camera position around the player: five points around the target object (state 0x24) at heights
 * scaled by D_800A2D40; keeps the last one (0x88, from 1) that the level doesn't block from the first, skipping 1 */
/* (the unused fifth parameter is reused for the collision flags: it gives the target's frame) */
void camera_point_around_player(u8 *obj, f32 *x, f32 *y, f32 *z, s32 flags) {
    u8 *state;
    u8 hit[0x30];
    f32 radius[3];
    f32 out[5][3];
    s32 i;
    u8 *target;

    flags = *(u8 *) (*(u8 **) (*(u8 **) (obj + 0x40) + 0x50) + 0x10);
    state = *(u8 **) (obj + 0x68);
    target = *(u8 **) (state + 0x24);
    i = 5;
    while (i--) {
        D_800A2D74[i][1] = 48.0f * D_800A2D40;
    }
    func_800496B0(5, (s16 *) target, D_800A2D74, (u8 *) out);
    i = 4;
    do {
        out[i][0] += *(f32 *) (target + 0xC);
        out[i][1] += *(f32 *) (target + 0x10);
        out[i][2] += *(f32 *) (target + 0x14);
    } while (i--);
    radius[0] = 10.0f;
    radius[1] = 10.0f;
    radius[2] = 10.0f;
    func_8001CE10(3, out[0], out[2], radius, 0x2000, 0, 0);
    if (func_80016208(obj, out[0], out[1], 10.0f, flags, hit)) {
        i = *(s32 *) (obj + 0x88);
        do {
            if (i == 1) {
                i++;
            }
            if (!func_80016208(obj, out[0], out[i], 10.0f, flags, hit)) {
                *(s32 *) (obj + 0x88) = i;
            } else {
                i = (i & 3) + 1;
            }
        } while (i != *(s32 *) (obj + 0x88));
    } else {
        *(s32 *) (obj + 0x88) = 1;
    }
    *x = out[*(s32 *) (obj + 0x88)][0];
    *y = out[*(s32 *) (obj + 0x88)][1];
    *z = out[*(s32 *) (obj + 0x88)][2];
}

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
void player_restart(s32 arg0, s8 *player);

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
        player_restart(arg0, player);
    }
}

u8 **objGetObjList(s32 *start, s32 *end);
s32 obj_dist_racer(f32 x, f32 y, f32 z, f32 radius, s32 is2dCheck, u8 **sortObj);
void level_next_select(void);

/* JFG: restart a player: refill the current weapon and clear its 0x0 and its object's 0x4C->0x6; in multiplayer
 * respawn it at a random respawn point (object 6) with no other player near, else at its own position.
 * The candidate list really is 4 entries in the original frame (more respawn points overrun it into the
 * locals around it, as in the ROM). */
void player_restart(s32 owner, s8 *player) {
    s32 start;
    s32 end;
    u8 *near[8];
    u8 **list;
    u8 *obj;
    s32 n;
    s32 count;
    u8 *candidates[4];
    u8 *chr;

    chr = func_80047C1C(player[0], player[1]);
    *(u16 *) (chr + 0x14) = *(u16 *) (chr + 0x34);
    *(s16 *) (*(u8 **) ((u8 *) owner + 0x4C) + 6) = 0;
    *(s16 *) chr = 0;
    if (func_80048558() >= 2) {
        list = objGetObjList(&start, &end);
        n = 0;
        while (start < end) {
            obj = list[start++];
            if (*(s16 *) (obj + 0x48) == 6) {
                count = obj_dist_racer(*(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14), 32.0f, 0,
                                       near);
                if (count == 0 || (count == 1 && near[0] == (u8 *) owner)) {
                    candidates[n++] = obj;
                }
            }
        }
        if (n == 0) {
            obj = (u8 *) owner;
        } else if (n == 1) {
            obj = candidates[0];
        } else {
            obj = candidates[rand_range(0, n - 1)];
        }
        player_respawn_at((u8 *) owner, *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10), *(f32 *) (obj + 0x14),
                          *(s16 *) (obj + 0), *(s16 *) (obj + 2), *(s16 *) (obj + 4));
    } else {
        level_next_select();
    }
}

s32 func_8005AAB8(s32 player);
s32 camera_follow_get(void);
s32 func_8004271C(void);

/* JFG: counts unk19C up by rate (to 40, or 64 when arg1's 0x70 is 7 in states 11/5) while the 0x540 low bits
 * are set and nothing blocks it, otherwise down by 16*rate to 0. */
void player_charge_meter_update(Racer *racer, u8 *arg1, s32 rate) {
    s32 enabled = *(u16 *) &racer->flags & 0xF;
    s32 limit;

    if (func_8005AAB8(racer->player) != 0) {
        enabled = 0;
    }
    limit = 40;
    if ((racer->character & 3) == 3) {
        enabled = 0;
    }
    if (racer->unk568 == 11 || racer->unk568 == 5) {
        enabled = 1;
        if (arg1 != NULL && arg1[0x70] == 7) {
            limit = 64;
        }
    } else if (racer->unk568 == 8 || racer->unk568 == 6 || racer->unk568 == 7 || racer->unk568 == 4) {
        enabled = 0;
    } else if (camera_follow_get() != 0) {
        enabled = 0;
    }
    if (racer->unk19D != 0 || racer->unk1F9 != 0 || racer->unk1FA != 0 || racer->unk198 != 0) {
        enabled = 0;
    }
    if (enabled && func_8004271C() == 0) {
        racer->unk19C += rate;
        if (limit < racer->unk19C) {
            racer->unk19C = limit;
        }
    } else if (racer->unk19C != 0) {
        racer->unk19C -= rate * 16;
        if (racer->unk19C < 0) {
            racer->unk19C = 0;
        }
    }
}

extern s16 D_800A2DB0;

/* JFG: hysteresis on the angle between angle and the player's 0x11C: unk569 is set when it leaves +-wide (and
 * func_8005AAB8 is 0) and cleared inside +-narrow or after 30 frames; *out = min(*out, near or far limit). */
void player_turn_limit_update(UNUSED s32 arg0, Racer *racer, f32 *out, s16 angle, s16 narrow, s16 wide, f32 limit,
                   f32 turnLimit) {
    s16 diff = angle - racer->unk11C;

    if (racer->unk569) {
        if (-narrow < diff && diff < narrow) {
            racer->unk569 = 0;
        }
        if (!(*(u16 *) &racer->flags & 0xF)) {
            D_800A2DB0++;
            if (D_800A2DB0 >= 30) {
                racer->unk569 = 0;
            }
        } else {
            D_800A2DB0 = 0;
        }
    } else if (*(u16 *) &racer->flags & 0xF) {
        if (func_8005AAB8(racer->player) == 0 && (diff < -wide || wide < diff)) {
            racer->unk569 = 1;
        }
    }
    if (racer->unk569) {
        if (turnLimit < *out) {
            *out = turnLimit;
        }
    } else if (limit < *out) {
        *out = limit;
    }
}

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

/* JFG: (table(b) - table(a)) / div, the table sampled in steps of 0.1 with linear interpolation. */
f32 table_lerp_diff(f32 a, f32 b, f32 *table, f32 div) {
    s32 i;
    f32 hi;

    b *= 10.0f;
    i = b;
    b -= i;
    hi = table[i] + (table[i + 1] - table[i]) * b;
    a *= 10.0f;
    i = a;
    a -= i;
    hi -= table[i] + (table[i + 1] - table[i]) * a;
    return hi / div;
}

extern u8 D_800A2DB4;
void func_800496B0(s32, s16 *, void *, u8 *);

void func_80034164(s16 *pos, u8 *obj) {
    s16 v[3];

    v[0] = pos[0];
    v[1] = pos[1];
    v[2] = 0;
    func_800496B0(3, v, &D_800A2DB4, obj + 0x18);
}

void obj_spawn_particle(u8 *obj, s32 updateRate);
s32 func_8004966C(s16 angle); /* sine of a 16-bit angle (fixed point) */
void effect_mesh_set_uvs(void *handle, s16 pitch);
void effect_mesh_update(void *handle, s16 value, u8 volume);

/* JFG: the player's four engine-like loops (0x598, state at 0x5B8, volume 0x5AC, phase 0x5A8): 0 waits for its
 * bit in 0x14A (spawning particles), 1 fades in, 2 plays (wobbling with the phase), 3 fades out. */
void player_loop_sounds_update(u8 *obj, u8 *racer, s32 emitOn, s32 emitStart, s32 updateRate) {
    s32 i;
    s32 bit;
    void *handle;
    u8 *p;
    s32 volume;
    s32 phase;
    s32 value;
    u8 *thing;
    s32 saved;
    u8 *q;

    thing = NULL;
    if (*(u8 ***) (obj + 0x78) != NULL && (thing = **(u8 ***) (obj + 0x78)) != NULL) {
        if (updateRate * 8 < thing[0x3B]) {
            thing[0x3B] -= updateRate * 8;
        } else {
            thing[0x3B] = 0;
        }
    }
    for (i = 0, bit = 1, q = racer; i < 4; i++, q += 4, bit <<= 1) {
        handle = *(void **) (q + 0x598);
        p = racer + i;
        if (handle == NULL) {
            continue;
        }
        value = *(s16 *) ((u8 *) handle + 0x20);
        volume = p[0x5AC];
        phase = (s8) p[0x5A8];
        switch (p[0x5B8]) {
            case 0:
                volume = 0;
                if (((s8 *) racer)[0x14A] & bit) {
                    saved = *(s32 *) (obj + 0x84);
                    *(s32 *) (obj + 0x84) = emitStart;
                    obj_spawn_particle(obj, 2);
                    *(s32 *) (obj + 0x84) = saved;
                    p[0x5B8] = 1;
                }
                break;
            case 1:
                volume += updateRate * 32;
                value = (*(s16 *) ((u8 *) handle + 0x20) * volume) >> 7;
                if (volume >= 256) {
                    volume = 255;
                    if (thing != NULL) {
                        thing[0x3B] = 0x80;
                    }
                    if (((s8 *) racer)[0x14A] & bit) {
                        p[0x5B8] = 2;
                    } else {
                        p[0x5B8] = 3;
                    }
                }
                break;
            case 2:
                volume += updateRate * 16;
                if (volume >= 256) {
                    volume = 255;
                }
                phase += updateRate * 16;
                value = ((func_8004966C(phase << 8) + 0x18000) * ((value * volume) >> 8)) >> 16;
                *(s32 *) (obj + 0x84) = emitOn;
                if (!(((s8 *) racer)[0x14A] & bit)) {
                    p[0x5B8] = 3;
                }
                break;
            case 3:
                volume -= updateRate * 8;
                if (volume <= 0) {
                    volume = 0;
                    p[0x5B8] = 0;
                    break;
                }
                phase += updateRate * 16;
                value = ((func_8004966C(phase << 8) + 0x18000) * ((value * volume) >> 8)) >> 16;
                if (((s8 *) racer)[0x14A] & bit) {
                    p[0x5B8] = 2;
                }
                break;
        }
        p[0x5AC] = volume;
        p[0x5A8] = phase;
        if (volume != 0) {
            effect_mesh_set_uvs(handle, (s8) p[0x5A8] << 8);
            effect_mesh_update(handle, value, p[0x5AC]);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800344B4);

extern u8 *D_800A18B0;
extern s32 gPlayerStickX;
extern s32 gPlayerStickY;
f32 func_80049C34(s32 angle);
f32 func_80049C30(s32 angle);

/* JFG: the quick turn: when the stick points more than `wide` away from the player's facing (*angle), start a turn
 * towards the object's angle (moving backwards fast: a reverse turn, *out = 1); while turning, *speed follows the
 * stick in the turn's frame, and the turn ends within `wide` of the start or `narrow` of the object's angle */
void player_quick_turn(s16 *obj, Racer *racer, f32 *speed, s16 *angle, s16 wide, s16 narrow, s32 *out, f32 scale) {
    f32 sinA;
    f32 cosA;
    f32 s;
    f32 c;
    f32 x;
    f32 y;
    s16 diff;
    s32 two = 2; // kept in one register for the state test and both unk576 stores

    *out = 0;
    if ((*(s32 *) (D_800A18B0 + 0xC) & gPlayerButtonsPressed) || racer->unk569 != 0 ||
        (racer->unk1F4 != 0 && func_8005AAB8(racer->player) == 0) || (racer->unk568 != 0 && racer->unk568 != two)) {
        racer->unk13E = 0;
        return;
    }
    if (racer->unk13E == 0) {
        diff = *obj - *angle;
        if (diff < -wide || wide < diff) {
            racer->unk13C = *obj;
            racer->unk13E = 1;
            racer->unk13F = 0;
            if (racer->velX < -2.5f) {
                racer->unk576 = two;
                racer->unk13F = 1;
                *out = 1;
            }
        }
        return;
    }
    if (racer->unk13F) {
        racer->unk576 = two;
        *speed = 0.0f;
    } else {
        s = -func_80049C34(racer->unk13C);
        c = -func_80049C30(racer->unk13C);
        sinA = func_80049C34(*obj);
        cosA = func_80049C30(*obj);
        x = gPlayerStickX;
        y = -gPlayerStickY;
        s *= x * cosA + y * sinA;
        c *= y * cosA - x * sinA;
        *speed = sqrtf(s * s + c * c) * scale;
    }
    diff = racer->unk13C - *angle;
    if (-wide < diff && diff < wide) {
        racer->unk13E = 0;
    }
    diff = *angle - *obj;
    if (-narrow < diff && diff < narrow) {
        racer->unk13E = 0;
    }
    if (racer->unk13E == 0) {
        if (*speed > 0.0f) {
            *speed = -*speed;
        }
        if (racer->velX > 0.0f) {
            racer->velX = -racer->velX;
        }
        racer->unk576 = 0;
    }
}

extern f32 D_800ABF68; // JFG: how fast the breathing level rises
extern f32 D_800ABF6C; // and falls
extern f32 D_800ABF70; // JFG: the breath length per breath period

/* JFG: the player's breathing: the level eases towards the speed (the larger of |velX| and |velZ|, 1 from 4 up), the
   phase advances by one breath every 180 - 130 * level frames, and a breath starts (breathNow, breathTimer) each
   time it wraps. */
void player_breath_update(Racer *racer, f32 updateRate) {
    f32 vz;
    f32 speed;

    if (racer->unk568 == 10) {
        racer->breathTimer = 0;
        racer->breathNow = 0;
    }
    speed = racer->velX;
    if (speed < 0.0f) {
        speed = -speed;
    }
    vz = racer->velZ;
    if (vz < 0.0f) {
        vz = -vz;
    }
    if (speed < vz) {
        speed = vz;
    }
    if (speed >= 4.0f) {
        speed = 1.0f;
    } else {
        speed *= 0.25f;
    }
    if (racer->breathLevel < speed) {
        vz = D_800ABF68;
    } else {
        vz = D_800ABF6C;
    }
    racer->breathLevel += (speed - racer->breathLevel) * (1.0f - func_80049BEC(1.0f - vz, (s32) updateRate));
    racer->breathPhase += updateRate / (180.0f + -130.0f * racer->breathLevel);
    if (racer->breathPhase >= 1.0f) {
        racer->breathTimer = (180.0f + -130.0f * racer->breathLevel) * D_800ABF70;
        racer->breathNow = 1;
        racer->breathPhase -= 1.0f;
    } else {
        racer->breathTimer -= (s32) updateRate;
        if (racer->breathTimer < 0) {
            racer->breathTimer = 0;
        }
        racer->breathNow = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80034BB0);

typedef struct PoseTarget {
    /* 0x00 */ f32 **points;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 pad5[3];
    /* 0x08 */ s32 steps;
    /* 0x0C */ u8 padC[8];
} PoseTarget;

/* JFG: the pose fields of the racer data (the struct fields let IDO load numPoints before the stores) */
typedef struct PoseRacer {
    /* 0x000 */ s8 player;
    /* 0x001 */ s8 character;
    /* 0x002 */ u8 pad2[0x12A - 0x2];
    /* 0x12A */ s16 unk12A;
    /* 0x12C */ s16 unk12C;
    /* 0x12E */ u8 pad12E[0x360 - 0x12E];
    /* 0x360 */ PoseTarget *pose;
    /* 0x364 */ f32 points[8][3];
    /* 0x3C4 */ f32 world[8][3];
    /* 0x424 */ f32 steps[16][3];
    /* 0x4E4 */ u8 pad4E4[0x4C];
    /* 0x530 */ s8 numPoints;
    /* 0x531 */ u8 unk531;
    /* 0x532 */ u8 pad532[3];
    /* 0x535 */ s8 numSteps;
    /* 0x536 */ u8 pad536[0x568 - 0x536];
    /* 0x568 */ u8 unk568;
} PoseRacer;

/* JFG: start moving towards the points of a new pose: per-step deltas (0x424) from the current points (0x364). */
void racer_pose_begin(UNUSED s32 arg0, PoseRacer *racer, PoseTarget *poses, u8 index) {
    PoseTarget *pose = &poses[index];
    f32 *from = racer->points[0];
    f32 *step;
    s32 i;
    f32 steps;

    if (pose != racer->pose) {
        racer->unk531 = pose->unk4;
        racer->numSteps = pose->steps;
        racer->pose = pose;
        step = racer->steps[0];
        steps = racer->numSteps;
        for (i = 0; i < racer->numPoints; i++) {
            step[0] = (racer->pose->points[i][0] - from[0]) / steps;
            step[1] = (racer->pose->points[i][1] - from[1]) / steps;
            step[2] = (racer->pose->points[i][2] - from[2]) / steps;
            from += 3;
            step += 3;
        }
    }
}

void func_800492DC(s16 *rotation, f32 *vec);

#ifdef NON_MATCHING
/* NON_MATCHING: 4 words: the target keeps the new step count in a0 with a copy in v1 for the `< 0` test */
/* JFG: step the pose points towards their target by `updateRate` steps (fewer when the pose ends), then place
 * them in the world: rotated by the object's angles (the racer's own pitch/roll in state 4) where the target
 * point asks for it (byte 0x11), plus the object's position */
void func_80035628(u8 *obj, PoseRacer *racer, f32 updateRate) {
    f32 *from;
    f32 *to;
    f32 *step;
    f32 t;
    s32 i;
    s32 n;
    UNUSED s32 pad;
    s16 angles[3];
    UNUSED s32 pad2;
    UNUSED s32 pad3;

    angles[0] = *(s16 *) obj;
    if (racer->unk568 == 4 && (racer->character & 3) != 2) {
        angles[1] = racer->unk12A;
        angles[2] = racer->unk12C;
    } else {
        angles[1] = *(s16 *) (obj + 2);
        angles[2] = *(s16 *) (obj + 4);
    }
    from = racer->points[0];
    to = racer->world[0];
    if (racer->numSteps > 0) {
        n = racer->numSteps - (s32) updateRate;
        if (n >= 0) {
            racer->numSteps = n;
            t = updateRate;
        } else if (n < 0) {
            t = racer->numSteps;
            racer->numSteps = 0;
        }
    } else {
        t = 0.0f;
    }
    step = racer->steps[0];
    for (i = 0; i < racer->numPoints; i++) {
        if (t > 0.0f) {
            from[0] += step[0] * t;
            from[1] += step[1] * t;
            from[2] += step[2] * t;
        }
        to[0] = from[0];
        to[1] = from[1];
        to[2] = from[2];
        if (((u8 *) racer->pose->points[i])[0x11]) {
            func_800492DC(angles, to);
        }
        to[0] += *(f32 *) (obj + 0xC);
        to[1] += *(f32 *) (obj + 0x10);
        to[2] += *(f32 *) (obj + 0x14);
        from += 3;
        to += 3;
        step += 3;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035628);
#endif

void player_platform_segment_point(u8 *obj, Racer *racer, f32 *x, f32 *y, f32 *z);
void func_80048B60(MtxF *mf, f32 x, f32 y, f32 z, f32 *ox, f32 *oy, f32 *oz);

/* JFG: a platform object's motion block (its 0x5C): world-to-local and local-to-world matrices, double buffered */
typedef struct PlatformData {
    /* 0x000 */ MtxF toLocal[2];
    /* 0x080 */ MtxF toWorld[2];
    /* 0x100 */ u8 pad100[0x28];
    /* 0x128 */ s16 prevYaw;
    /* 0x12A */ u8 pad12A;
    /* 0x12B */ u8 cur;
    /* 0x12C */ s16 active;
} PlatformData;

/* JFG: ride the platform object the player stands on: turn with it and move by the platform's motion since the
 * last frame (its previous and current matrices, or player_platform_segment_point in states 6, 7, 8 and 12) */
void player_platform_ride(u8 *obj, Racer *racer, UNUSED f32 updateRate) {
    s16 *platform;
    PlatformData *data;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 x;
    f32 y;
    f32 z;
    s16 turn;
    u8 cur;

    racer->prevPos[0] = racer->pos[0];
    racer->prevPos[1] = racer->pos[1];
    racer->prevPos[2] = racer->pos[2];
    platform = racer->platform;
    if (platform != NULL) {
        data = *(PlatformData **) ((u8 *) platform + 0x5C);
    } else {
        data = NULL;
    }
    if (data != NULL && data->active != 0) {
        turn = platform[0] - data->prevYaw;
        if (racer->unk13E) {
            racer->unk13C += turn;
        }
        if (racer->unk568 == 8 || racer->unk568 == 6 || racer->unk568 == 7 || racer->unk568 == 12) {
            player_platform_segment_point(obj, racer, &x, &y, &z);
            dx = x - *(f32 *) (obj + 0xC);
            dy = y - *(f32 *) (obj + 0x10);
            dz = z - *(f32 *) (obj + 0x14);
        } else {
            cur = data->cur;
            func_80048B60(&data->toLocal[cur ^ 1], *(f32 *) (obj + 0xC), *(f32 *) (obj + 0x10),
                          *(f32 *) (obj + 0x14), &dx, &dy, &dz);
            func_80048B60(&data->toWorld[cur], dx, dy, dz, &x, &y, &z);
            dx = x - *(f32 *) (obj + 0xC);
            dy = y - *(f32 *) (obj + 0x10);
            dz = z - *(f32 *) (obj + 0x14);
        }
        if (racer->unk568 == 8 || racer->unk568 == 7) {
            racer->unk11C = racer->unk182;
        } else {
            racer->unk11C += turn;
        }
        move_object(obj, dx, dy, dz);
        racer->pos[0] += dx;
        racer->pos[1] += dy;
        racer->pos[2] += dz;
    }
}

/* JFG: in some racer states (6, 7, 8 and 12), copy the racer's angle at 0x17E to 0x120 and to the object. */
void racer_angle_sync(u8 *obj, u8 *racer, UNUSED s32 arg2) {
    u8 state = racer[0x568];

    if (state == 8 || state == 6 || state == 7 || state == 0xC) {
        s16 angle = *(s16 *) (racer + 0x17E);
        *(s16 *) (racer + 0x120) = angle;
        *(s16 *) (obj + 4) = angle;
    }
}

s32 func_80049C84(f32 x, f32 z);
void func_80049394(s16 *rotation, f32 *vec);
f32 sqrtf(f32 x);

s32 func_80049C84(f32 x, f32 z);
void func_80049394(s16 *rotation, f32 *vec);
f32 sqrtf(f32 x);

/* JFG: where the racer stands on its platform segment (states 6, 7, 8 and 12): the point at segT along the segment
 * through the platform's last matrix; also aims the racer along the segment (0x182 yaw, 0x17E pitch). */
void player_platform_segment_point(u8 *obj, Racer *racer, f32 *x, f32 *y, f32 *z) {
    PlatformData *data;
    f32 *seg = racer->segStart;
    f32 local[3];
    f32 world[3];
    f32 ax;
    f32 ay;
    f32 az;
    f32 bx;
    f32 by;
    f32 bz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dist;
    s16 rot[3];
    u8 prev;
    u8 cur;

    data = *(PlatformData **) ((u8 *) racer->platform + 0x5C);
    cur = data->cur;
    prev = data->cur ^ 1;
    dx = racer->segEnd[0] - racer->segStart[0];
    dy = racer->segEnd[1] - racer->segStart[1];
    dz = racer->segEnd[2] - racer->segStart[2];
    world[0] = racer->segStart[0] + racer->segT * dx;
    world[1] = racer->segStart[1] + racer->segT * dy;
    world[2] = racer->segStart[2] + racer->segT * dz;
    func_80048B60(&data->toWorld[prev], world[0], world[1], world[2], &local[0], &local[1], &local[2]);
    local[0] -= *(f32 *) (obj + 0xC);
    local[1] -= *(f32 *) (obj + 0x10);
    local[2] -= *(f32 *) (obj + 0x14);
    func_80048B60(&data->toWorld[cur], seg[0], seg[1], seg[2], &ax, &ay, &az);
    func_80048B60(&data->toWorld[cur], seg[3], seg[4], seg[5], &bx, &by, &bz);
    dx = bx - ax;
    dy = by - ay;
    dz = bz - az;
    racer->unk182 = (func_80049C84(dx, dz) - 0x4000) & 0xFFF0;
    dist = sqrtf(dx * dx + dz * dz);
    racer->unk17E = func_80049C84(dy, dist) & 0xFFF0;
    world[0] = seg[6] * dx + ax;
    world[1] = seg[6] * dy + ay;
    world[2] = seg[6] * dz + az;
    racer->unk178 = world[1];
    rot[0] = -racer->unk122;
    rot[1] = 0;
    rot[2] = -racer->unk126;
    func_80049394(rot, local);
    rot[0] = racer->unk182;
    rot[1] = 0;
    rot[2] = racer->unk17E;
    func_800492DC(rot, local);
    *x = world[0] - local[0];
    *y = world[1] - local[1];
    *z = world[2] - local[2];
}

/* JFG: how a ledge may be grabbed (the hang move's parameters) */
typedef struct HangParams {
    /* 0x00 */ f32 rangeX; // grab only below this speed (x and z), unless ...
    /* 0x04 */ f32 rangeZ;
    /* 0x08 */ f32 distance;
    /* 0x0C */ f32 edgeGap;
    /* 0x10 */ u8 pad10[4];
    /* 0x14 */ s32 turnFrames;
    /* 0x18 */ s32 frames;
} HangParams;

s32 func_80081658(f32 *from, f32 *to, f32 *seg, s32 *hit); // the ledge crossed between two points (platforms)
s32 func_80019888(f32 *from, f32 *to, f32 *seg);           // the same on the track
s32 func_80081CB8(f32 *seg, f32 *next);                    // the ledge next to seg (platforms)
s32 track_edge_find_next(f32 *seg, f32 *next);
void func_8004A2FC(MtxF *mf, f32 *in, f32 *out);

#ifdef NON_MATCHING
/* NON_MATCHING: the frame and the declared stack slots match; the float registers are numbered differently from the
 * else branch on (the target loads the segment ends into f4..f18 in turn) and the temporaries sit one word lower. */
/* JFG: grab the ledge the player has just crossed, if the move allows it: snap to the point along it, enter the hang
 * state (6) facing the wall, and slide towards the middle when the next ledge turns away. */
void func_80035E14(u8 *obj, Racer *racer, f32 arg2, f32 arg3, UNUSED s32 arg4, HangParams *params) {
    s32 hit;
    f32 *seg;
    s32 result;
    s32 angle;
    f32 adj[6];
    PlatformData *data;
    f32 hang[3];
    s16 next;
    f32 normal[3];
    UNUSED s32 pad0;
    f32 dist;
    f32 ax;
    f32 ay;
    f32 az;
    f32 bx;
    f32 by;
    f32 bz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 s;
    UNUSED s32 pad[3];
    f32 velX;
    f32 velZ;
    UNUSED s32 pad2;
    s32 ok;
    s16 side;
    s16 roll;
    s16 pitch;
    s16 yaw;
    u8 cur;
    f32 s2;
    f32 c2;
    f32 c1;

    if (racer->unk184 == 0) {
        return;
    }
    seg = racer->segStart;
    if (racer->platform != NULL) {
        result = func_80081658(racer->pos, (f32 *) (obj + 0xC), seg, &hit);
    } else {
        result = func_80019888(racer->pos, (f32 *) (obj + 0xC), seg);
    }
    if (result != -1) {
        return;
    }
    if (racer->platform != NULL) {
        data = *(PlatformData **) ((u8 *) racer->platform + 0x5C);
        cur = data->cur;
        func_80048B60(&data->toWorld[cur], seg[0], seg[1], seg[2], &ax, &ay, &az);
        func_80048B60(&data->toWorld[cur], seg[3], seg[4], seg[5], &bx, &by, &bz);
        func_8004A2FC(&data->toLocal[cur], *(f32 **) &seg[7], normal);
    } else {
        ax = seg[0];
        ay = seg[1];
        az = seg[2];
        bx = seg[3];
        by = seg[4];
        bz = seg[5];
        normal[0] = (*(f32 **) &seg[7])[0];
        normal[1] = (*(f32 **) &seg[7])[1];
        normal[2] = (*(f32 **) &seg[7])[2];
    }
    dx = bx - ax;
    dy = by - ay;
    dz = bz - az;
    hang[0] = seg[6] * dx + ax;
    hang[1] = seg[6] * dy + ay;
    hang[2] = seg[6] * dz + az;
    dist = sqrtf(dx * dx + dz * dz);
    yaw = func_80049C84(dx, dz);
    pitch = func_80049C84(dy, dist);
    angle = -yaw;
    s = func_80049C34(angle);
    roll = func_80049C84(normal[2] * s + normal[0] * func_80049C30(angle), normal[1]);
    ok = 0;
    velX = racer->velX;
    velZ = racer->velZ;
    angle = arg3;
    c1 = func_80049C30(angle);
    side = yaw - 0x4000;
    c2 = func_80049C30(side);
    s2 = func_80049C34(side);
    if (-params->rangeX < velX && velX < params->rangeX && -params->rangeZ < velZ && velZ < params->rangeZ) {
        ok = 1;
    } else if (arg2 == 0.0f && (velX != 0.0f || velZ != 0.0f)) {
        ok = 1;
    } else if (func_80049C34(angle) * s2 + c2 * c1 > 0.0f) {
        ok = 1;
    }
    if (ok && *(f32 *) (obj + 0x10) - 10.0f < hang[1] && hang[1] < *(f32 *) (obj + 0x10) + 10.0f) {
        *(f32 *) (obj + 0xC) = hang[0];
        *(f32 *) (obj + 0x10) = hang[1];
        *(f32 *) (obj + 0x14) = hang[2];
        racer->unk568 = 6;
        racer->unk579 = params->turnFrames;
        racer->unk17C = racer->unk11C - side;
        racer->unk578 = params->frames;
        racer->unk17C /= params->turnFrames;
        racer->unk576 = 0;
        racer->unk13E = 0;
        racer->unk16C = (s32) racer->platform;
        racer->unk170 = params->distance / params->frames;
        racer->unk17E = pitch;
        racer->unk182 = side;
        racer->unk180 = roll;
        racer->unk1F4 = 0;
        racer->unk178 = hang[1];
        if (racer->platform != NULL) {
            result = func_80081CB8(seg, adj);
        } else {
            result = track_edge_find_next(seg, adj);
        }
        racer->unk174 = 0.0f;
        if (result) {
            if (racer->platform != NULL) {
                func_80048B60(&data->toWorld[cur], adj[0], adj[1], adj[2], &ax, &ay, &az);
                func_80048B60(&data->toWorld[cur], adj[3], adj[4], adj[5], &bx, &by, &bz);
                next = func_80049C84(bx - ax, bz - az);
            } else {
                next = func_80049C84(adj[3] - adj[0], adj[5] - adj[2]);
            }
            next -= yaw;
            if (next < -3000 || next > 3000) {
                dist = sqrtf(dx * dx + dy * dy + dz * dz);
                if (seg[6] < 0.5f) {
                    dist *= seg[6];
                    if (dist < params->edgeGap) {
                        racer->unk174 = (params->edgeGap - dist) / params->frames;
                    }
                } else {
                    dist *= 1.0f - seg[6];
                    if (dist < params->edgeGap) {
                        racer->unk174 = -(params->edgeGap - dist) / params->frames;
                    }
                }
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/2BAA0", func_80035E14);
#endif

INCLUDE_ASM("asm/nonmatchings/2BAA0", func_800364D4);

void *func_800822D8(f32 *point, s32 radius, void *ignore, f32 *outY);
void falling_objects_hit_box(f32 *min, f32 *max, void *ignore);
s32 func_8001A218(f32 x, f32 z, f32 bottom, f32 top, f32 radius, s32 arg5, s32 arg6, f32 ***heights);

/* JFG: squash check before moving (in states 6, 7, 8 and 12): the pose points placed in the world; the lowest
 * object over a free point (down to the pose height) squashes the player, or the platform stood on does when
 * it is moving and the player's cylinder hits the track; a squash pushes the player off (or leaves the platform) */
void player_squash_precheck(u8 *obj, Racer *racer, UNUSED s32 updateRate) {
    s32 i;
    f32 *out;
    f32 *src;
    u32 mask;
    void *squash;
    f32 lowest;
    f32 points[8][3];
    f32 min[3];
    f32 max[3];
    void *hit;
    f32 y;
    u8 *motion;
    UNUSED s32 pad;
    s16 angles[3];

    min[0] = *(f32 *) (obj + 0xC) - *(f32 *) (racer->pose + 0x10);
    min[1] = *(f32 *) (obj + 0x10);
    min[2] = *(f32 *) (obj + 0x14) - *(f32 *) (racer->pose + 0x10);
    max[0] = *(f32 *) (racer->pose + 0x10) + *(f32 *) (obj + 0xC);
    max[1] = *(f32 *) (racer->pose + 0xC) + *(f32 *) (obj + 0x10);
    max[2] = *(f32 *) (racer->pose + 0x10) + *(f32 *) (obj + 0x14);
    falling_objects_hit_box(min, max, racer->platform);
    if (racer->unk568 == 6 || racer->unk568 == 8 || racer->unk568 == 7 || racer->unk568 == 12) {
        src = racer->points[0];
        angles[0] = ((s16 *) obj)[0];
        out = points[0];
        angles[1] = ((s16 *) obj)[1];
        angles[2] = ((s16 *) obj)[2];
        for (i = 0; i < racer->numPoints; i++) {
            out[0] = src[0];
            out[1] = src[1];
            out[2] = src[2];
            if ((*(u8 ***) racer->pose)[i][0x11]) {
                func_800492DC(angles, out);
            }
            out[0] += *(f32 *) (obj + 0xC);
            out[1] += *(f32 *) (obj + 0x10);
            out[2] += *(f32 *) (obj + 0x14);
            src += 3;
            out += 3;
        }
        mask = racer->pointMask;
        squash = NULL;
        src = points[0];
        lowest = *(f32 *) (racer->pose + 0xC) + *(f32 *) (obj + 0x10);
        for (i = 0; i < racer->numPoints; i++) {
            if (!(mask & 1)) {
                hit = func_800822D8(src, *(s32 *) ((*(u8 ***) racer->pose)[i] + 0xC), racer->platform, &y);
                if (hit != NULL && y <= lowest) {
                    lowest = y;
                    squash = hit;
                }
            }
            src += 3;
            mask >>= 1;
        }
        if (squash != NULL) {
            if (racer->platform != NULL) {
                racer->squashTimer = 30;
                racer->squashObj = racer->platform;
                racer->platform = NULL;
                racer->unk16C = 0;
            } else {
                racer->squashTimer = 30;
                racer->squashObj = squash;
                racer->unk149 = 10;
                racer->pushX = func_80049C34(racer->unk11C) * 5.0f;
                racer->pushZ = func_80049C30(racer->unk11C) * 5.0f;
            }
        } else if (racer->platform != NULL) {
            motion = *(u8 **) ((u8 *) racer->platform + 0x5C);
            if (*(s16 *) (motion + 0x12C) != 0 &&
                func_8001A218(*(f32 *) (obj + 0xC), *(f32 *) (obj + 0x14), *(f32 *) (obj + 0x10),
                              *(f32 *) (racer->pose + 0xC) + *(f32 *) (obj + 0x10) + 20.0f,
                              *(f32 *) (racer->pose + 0x10), 0, 0, 0)) {
                racer->squashTimer = 30;
                racer->squashObj = racer->platform;
                racer->platform = NULL;
                racer->unk16C = 0;
            }
        }
    }
}

/* JFG: squash checks: the timer (0x186) clears the squashing object; event 1 starts checking, event 2 pushes the
 * player off the platform; while checking, the lowest object over a free pose point (down to the pose height)
 * squashes the player: it pushes off (by the object's direction 0x8C, or away from it) unless on a platform */
void player_squash_update(u8 *obj, Racer *racer, s32 updateRate) {
    s32 i;
    f32 *point;
    u32 mask;
    void *squash;
    f32 y;
    f32 lowest;
    void *hit;
    f32 push;
    s16 angle;
    f32 pushZ;
    f32 dx;
    f32 dz;

    if (racer->squashTimer != 0) {
        racer->squashTimer -= updateRate;
        if (racer->squashTimer <= 0) {
            racer->squashObj = NULL;
            racer->squashTimer = 0;
        }
    }
    if (racer->squashEvent == 1) {
        racer->squashCheck = 1;
    } else if (racer->squashEvent == 2) {
        racer->squashTimer = 30;
        racer->squashObj = racer->platform;
        racer->platform = NULL;
        dx = *(f32 *) (obj + 0xC) - *(f32 *) ((u8 *) racer->squashObj + 0xC);
        dz = *(f32 *) (obj + 0x14) - *(f32 *) ((u8 *) racer->squashObj + 0x14);
        angle = func_80049C84(dx, dz);
        racer->unk149 = 10;
        racer->pushX = func_80049C34(angle) * 10.0f;
        racer->pushZ = func_80049C30(angle) * 10.0f;
    }
    squash = NULL;
    if (racer->squashCheck) {
        mask = racer->pointMask;
        point = racer->posePoints[0];
        lowest = *(f32 *) (racer->pose + 0xC) + *(f32 *) (obj + 0x10);
        for (i = 0; i < racer->numPoints; i++) {
            if (!(mask & 1)) {
                hit = func_800822D8(point, *(s32 *) ((*(u8 ***) racer->pose)[i] + 0xC), racer->platform, &y);
                if (hit != NULL && y <= lowest) {
                    lowest = y;
                    squash = hit;
                }
            }
            point += 3;
            mask >>= 1;
        }
        if (squash != NULL) {
            if (racer->platform != NULL) {
                racer->squashTimer = updateRate;
                racer->squashObj = squash;
                racer->unk4C = (lowest - *(f32 *) (obj + 0x10)) / *(f32 *) (racer->pose + 0xC);
                racer->unk57A = 0;
                if (*(f32 *) (obj + 0x20) > 0.0f) {
                    *(f32 *) (obj + 0x20) = 0.0f;
                }
            } else {
                racer->squashTimer = updateRate;
                racer->squashObj = squash;
                if (*(s32 *) ((u8 *) squash + 0x8C) == 0) {
                    angle = func_80049C84(*(f32 *) (obj + 0xC) - *(f32 *) ((u8 *) squash + 0xC),
                                          *(f32 *) (obj + 0x14) - *(f32 *) ((u8 *) squash + 0x14));
                } else {
                    angle = *(s32 *) ((u8 *) squash + 0x8C) * 0xB6 + 0x8000;
                }
                push = func_80049C34(angle) * 5.0f;
                pushZ = func_80049C30(angle) * 5.0f;
                racer->unk149 = 10;
                racer->pushX = push;
                racer->pushZ = pushZ;
                racer->unk4C = (lowest - *(f32 *) (obj + 0x10)) / *(f32 *) (racer->pose + 0xC);
                racer->unk57A = 0;
                if (*(f32 *) (obj + 0x20) > 0.0f) {
                    *(f32 *) (obj + 0x20) = 0.0f;
                }
            }
        } else {
            racer->squashCheck = 0;
        }
    }
}

extern f32 D_800ABF74;
extern f32 D_800ABF78;

/* JFG: ease the value at 0x48 towards the target at 0x4C; on arrival the target becomes 1.0.
   (The (f32) casts set the add.s operand order.) */
void racer_ease_to_target(u8 *obj, f32 rate) {
    f32 cur = *(f32 *) (obj + 0x48);
    f32 target = *(f32 *) (obj + 0x4C);

    if (cur < target) {
        *(f32 *) (obj + 0x48) = (f32) cur + ((target - cur) * 0.125f + D_800ABF74) * rate;
        if (target <= *(f32 *) (obj + 0x48)) {
            *(f32 *) (obj + 0x48) = target;
            *(f32 *) (obj + 0x4C) = 1.0f;
        }
    } else {
        *(f32 *) (obj + 0x48) = (f32) cur + ((target - cur) * 0.125f - D_800ABF78) * rate;
        if (*(f32 *) (obj + 0x48) <= target) {
            *(f32 *) (obj + 0x48) = target;
            *(f32 *) (obj + 0x4C) = 1.0f;
        }
    }
}

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
void rumble_raise(s32 arg0, u8 arg1, s32 arg2);

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words: the table byte is loaded into t7 (target t6, with t7 unused after it); the
   `if (arg2) {}` stops the home store of the unused parameter and the no-op mask fixes the later temps */
void func_8003733C(s32 arg0, s32 flags, UNUSED s32 arg2, s32 index, s32 level) {
    if (arg2) {}
    if (index >= 0 && index < 8) {
        flags |= D_800A2DD8[index] & 0xFF;
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
        rumble_raise(arg0, D_800A2DE8[flags & 7], D_800A2DF0[flags & 7]);
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
            if (!chr) {}
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

typedef struct Spawn50 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 unkE;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 weapon;
    /* 0x13 */ s8 unk13;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ s32 owner;
    /* 0x1C */ u8 *unk1C;
    /* 0x20 */ u8 *unk20;
} Spawn50;

extern u8 D_800A1790[][12];
extern u8 D_800A1850[][6];

/* JFG: fire object 0x50 for the character's current weapon (its two weapon table entries), with the weapon's
 * sound: 8 for weapon 2 (kept if already playing), 3 for weapon 7 in state 11 or when arg8 is set, else 0xE */
void weapon_fire_standard(s32 owner, Racer *racer, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8, s32 arg9,
                   s32 arg10) {
    Spawn50 setup;
    u8 *obj;
    u8 *chr;

    chr = func_80047C1C(racer->player, racer->character);
    setup.objectID = 0x50;
    setup.size = sizeof(Spawn50);
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.unkA = arg8;
    setup.unkB = arg9;
    setup.unkC = arg5;
    setup.unkE = arg6;
    setup.unk10 = arg10;
    setup.unk14 = arg7;
    setup.owner = owner;
    setup.unk3 = 0;
    setup.unk11 = 0;
    setup.unk1C = D_800A1790[chr[0x70]];
    setup.unk20 = D_800A1850[chr[0x70]];
    setup.weapon = chr[0x70];
    setup.unk13 = -1;
    stats_shot_add(chr[0x70]);
    if (chr[0x70] == 2) {
        if (racer->sound == 0 || racer->soundId != 8) {
            if (racer->sound != 0) {
                sound_stop(racer->sound);
            }
            sound_play(8, &racer->sound);
            racer->soundId = 8;
        }
    } else {
        if (racer->sound != 0) {
            sound_stop(racer->sound);
        }
        if (chr[0x70] == 7 && racer->unk568 == 11) {
            racer->soundId = 3;
            sound_play(3, &racer->sound);
        } else if (arg8) {
            racer->soundId = 3;
            sound_play(3, &racer->sound);
            racer->flags.unk25 = 1;
        } else {
            racer->soundId = 0xE;
            sound_play(0xE, &racer->sound);
        }
    }
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
    }
}

typedef struct SpawnA4 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ f32 unkC;
    /* 0x10 */ s32 owner;
    /* 0x14 */ void *chain;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 chainId;
    /* 0x26 */ u8 unk26;
} SpawnA4;

/* JFG: fire three 0xA4 shots without a chain: two at random spins (the second turned by 0x8000), then a straight
 * one, each one weaker (unkB - 1) */
void weapon_fire_triple_shot(s32 owner, Racer *racer, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8, s32 arg9) {
    SpawnA4 setup;
    u8 *obj;

    stats_shot_add(5);
    setup.objectID = 0xA4;
    setup.size = sizeof(SpawnA4);
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.unk1E = arg5;
    setup.unk20 = arg6;
    setup.unkC = arg7;
    setup.unkA = arg8;
    setup.unkB = arg9;
    setup.owner = owner;
    setup.unk3 = 0;
    setup.unk24 = 0;
    setup.unk26 = 5;
    if (racer->sound != 0) {
        sound_stop(racer->sound);
    }
    setup.unk18 = 0;
    setup.unk1A = rand_range(0x40, 0x300);
    if (rand_range(0, 1) == 1) {
        setup.unk1A = -setup.unk1A;
    }
    setup.unk1C = rand_range(0, 10);
    setup.unk22 = rand_range(1, 5);
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
    }
    setup.unkB--;
    setup.unk18 = -0x8000;
    setup.unk1A = rand_range(0x40, 0x300);
    setup.unk1C = rand_range(0, 10);
    setup.unk22 = rand_range(1, 5);
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
    }
    setup.unkB--;
    setup.unk18 = -0x8000;
    setup.unk1A = 0;
    setup.unk1C = 0;
    setup.unk22 = 0;
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
    }
    if (racer->sound != 0) {
        sound_stop(racer->sound);
    }
    sound_play(0x95, &racer->sound);
    racer->soundId = 0x95;
}

void *chain_slot_find_shortest(s32 obj, u8 *id);

/* JFG: fire object 0xA4 at (x, y, z), attached to the owner's shortest chain; weaker without one. */
void weapon_fire_chain_shot(s32 owner, Racer *racer, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8,
                   u32 arg9) {
    SpawnA4 setup;
    u8 *obj;
    u8 id;

    stats_shot_add(1);
    setup.objectID = 0xA4;
    setup.size = sizeof(SpawnA4);
    setup.unk3 = 0;
    setup.chain = chain_slot_find_shortest(owner, &id);
    setup.chainId = id;
    setup.unk24 = 1;
    setup.unkB = arg9;
    setup.x = x;
    setup.y = y;
    setup.z = z;
    setup.unk1E = arg5;
    setup.unk20 = arg6;
    setup.unkC = arg7;
    setup.unkA = arg8;
    if (setup.chain == NULL) {
        setup.unkB = (arg9 & 0xFF) >> 2;
    }
    setup.owner = owner;
    setup.unk26 = 1;
    if (racer->sound != 0) {
        sound_stop(racer->sound);
    }
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(s32 *) (obj + 0x3C) = 0;
    }
    if (racer->sound != 0) {
        sound_stop(racer->sound);
    }
    sound_play(0x95, &racer->sound);
    racer->soundId = 0x95;
}

typedef struct SpawnB6 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x04 */ s16 x, y, z;
    /* 0x0C */ u8 padC[4];
    /* 0x10 */ s32 owner;
    /* 0x14 */ s16 unk14[3];
    /* 0x1A */ s16 unk1A[3];
    /* 0x20 */ s16 unk20[3];
    /* 0x26 */ s16 unk26;
    /* 0x28 */ u8 unk28;
    /* 0x29 */ u8 unk29;
    /* 0x2A */ u8 unk2A;
} SpawnB6;

extern u8 D_800A1553;

/* JFG: give the player its attached object 0xB6 (spawned once), flag it, and set the player's sound to 0x97. */
void player_attachment_spawn(s32 owner, Racer *racer, UNUSED s32 arg2) {
    SpawnB6 setup;
    u8 *obj = racer->attachment;
    u8 *data;

    if (obj == NULL) {
        setup.objectID = 0xB6;
        setup.size = sizeof(SpawnB6);
        setup.owner = owner;
        setup.unk28 = D_800A1553;
        setup.x = 0;
        setup.y = 0;
        setup.z = 0;
        setup.unk26 = 400;
        setup.unk2A = 0;
        setup.unk14[0] = 0;
        setup.unk1A[0] = 0x800;
        setup.unk20[0] = 0x2000;
        setup.unk14[1] = 0x4000;
        setup.unk1A[1] = -0x1200;
        setup.unk20[1] = 0x2000;
        setup.unk14[2] = 0;
        setup.unk1A[2] = 0x800;
        setup.unk20[2] = -0x2000;
        setup.unk29 = 0;
        racer->attachment = spawn_object(&setup, 1);
        *(s32 *) (racer->attachment + 0x3C) = 0;
        obj = racer->attachment;
    }
    data = *(u8 **) (obj + 0x68);
    data[0xA7D] = 1;
    if (racer->soundId != 0x97) {
        if (racer->sound != 0) {
            sound_stop(racer->sound);
        }
        racer->soundId = 0x97;
    }
}

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
    player_voice_play(owner, 2);
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

/* JFG: spawn object 0x39 at (x, y, z), moving at `speed` along (yaw, pitch). */
void func_80039A38(s32 owner, UNUSED s32 arg1, f32 x, f32 y, f32 z, s16 yaw, s16 pitch, f32 speed) {
    Spawn39 setup;
    u8 *obj;

    setup.objectID = 0x39;
    setup.size = 0xA;
    setup.owner = owner;
    setup.unk3 = 0;
    setup.x = x;
    setup.y = y;
    setup.z = z;
    obj = spawn_object(&setup, 1);
    if (obj != NULL) {
        *(f32 *) (obj + 0x1C) = -func_80049C34(yaw) * func_80049C30(pitch) * speed;
        *(f32 *) (obj + 0x20) = func_80049C34(pitch) * speed;
        *(f32 *) (obj + 0x24) = -func_80049C30(yaw) * func_80049C30(pitch) * speed;
        sound_play(0x2F, NULL);
        player_voice_play(owner, 2);
        *(s32 *) (obj + 0x3C) = 0;
        stats_shot_add(9);
    }
}

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
    player_voice_play(owner, 2);
}

typedef struct SpawnF7 {
    /* 0x00 */ s16 objectID;
    /* 0x02 */ u8 size;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ s16 x, y, z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ s32 owner;
} SpawnF7;

/* The module trap called with a float in a3; needs `func_80054530_pssf = 0x80054530;` in undefined_syms.us.txt. */
void func_80054530_pssf(u8 *obj, s16 arg1, s16 arg2, f32 arg3);

/* JFG: throws the held object 0x59 (along arg5/arg6 at arg7) if the player holds one (any other held object is
 * dropped), otherwise spawns object 0xF7, which characters 2 and 6 keep holding; returns whether it threw or got
 * something to hold (then with sound 0xF1). */
s32 weapon_throw_or_hold(s32 owner, Racer *racer, f32 x, f32 y, f32 z, s16 arg5, s16 arg6, f32 arg7, s32 arg8, s32 arg9) {
    s32 done;
    SpawnF7 setup;
    u8 *obj;
    u8 *held;

    done = FALSE;
    held = racer->held;
    if (held != NULL) {
        if (*(s16 *) (held + 0x48) == 0x59) {
            func_80054530_pssf(held, arg5, arg6, arg7);
            racer->held = NULL;
            done = TRUE;
        } else {
            player_drop_held_object((u8 *) owner);
        }
    }
    if (!done) {
        setup.objectID = 0xF7;
        setup.x = x;
        setup.y = y;
        setup.z = z;
        setup.unkA = arg5;
        setup.unkC = arg6;
        setup.unkE = arg8;
        setup.unkF = arg9;
        setup.unk10 = arg7;
        setup.owner = owner;
        if (racer->character == 2 || racer->character == 6) {
            setup.owner = 0;
            done = TRUE;
        }
        obj = spawn_object(&setup, 1);
        if (obj != NULL) {
            *(s32 *) (obj + 0x3C) = 0;
            if (setup.owner != 0) {
                racer->held = obj;
            }
        }
    }
    if (done) {
        if (racer->sound != 0) {
            sound_stop(racer->sound);
        }
        sound_play(0xF1, &racer->sound);
        racer->soundId = 0xF1;
        player_voice_play(owner, 2);
        return TRUE;
    }
    return FALSE;
}

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
        player_voice_play(owner, 2);
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
void players_unlink_object(s32 obj) {
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

/* JFG: clear every player's references (0x1A4, 0x1D4) to an object. */
void players_forget_object(s32 obj) {
    s32 i;
    s32 numRacers;
    u8 **racers = get_racer_objects(&numRacers);
    Racer *racer;

    for (i = 0; i < numRacers; i++) {
        racer = *(Racer **) (*racers + 0x68);
        if (obj == racer->unk1A4) {
            racer->unk1A4 = 0;
        }
        if (obj == racer->unk1D4) {
            racer->unk1D4 = 0;
        }
        racers++;
    }
}

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
