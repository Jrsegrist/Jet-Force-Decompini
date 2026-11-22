/*
 * Runlink module 53: a vehicle the players ride (its init, control and camera; still mostly
 * assembly).
 * Family: other object behaviours (docs/runlink.md, "Module families").
 */
#include "module.h"

/* The module's .rodata and .bss (still assembly) */
extern f32 rl_m053_03828[4];
extern f32 rl_m053_03838[4];
extern f32 rl_m053_03848[4];

/* Resets player `player`'s camera values */
void func_m053_00000(s32 player) {
    rl_m053_03828[player] = 0.15416667f;
    rl_m053_03838[player] = 0.0f;
    rl_m053_03848[player] = 0.0f;
}

INCLUDE_ASM("asm/nonmatchings/modules/m053/ro", func_m053_00038);

INCLUDE_ASM("asm/nonmatchings/modules/m053/ro", func_m053_00EA0);

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
extern void player_stick_aim_update(s16 *angles, M053Steer *s, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s16 *yaw, s16 *pitch,
                          f32 updateRate);
extern s16 angle_lerp(s16 from, s16 to, f32 t);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_8004966C(s16 angle); /* sine */
extern s16 *rl_m053_03820;          /* .bss (still assembly): the camera commands */

/* Steering the vehicle and its camera while the button is held (called from 00038) */
#ifdef NON_MATCHING
/* Registers and operand order of the 1C6..1CC stores (t-registers one apart) */
void func_m053_014F4(s16 *angles, M053Steer *s, UNUSED s32 arg2, UNUSED s32 arg3, f32 updateRateF) {
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
    s->unk1C6 = s->unk1CE + angles[0];
    s->unk1C8 = s->unk1D0 + angles[1];
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
#else
INCLUDE_ASM("asm/nonmatchings/modules/m053/ro", func_m053_014F4);
#endif

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
void func_m053_01850(Object *obj, M053Slow *s, UNUSED s32 arg2, UNUSED s32 arg3, f32 updateRateF) {
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
f32 func_m053_019B0(s16 *yaw, M053Turn *s, f32 speed, s16 target) {
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

INCLUDE_ASM("asm/nonmatchings/modules/m053/ro", func_m053_01BB4);

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

extern void player_voice_play(Object *obj, s32 index);
extern void menu_draw_layout_box(s32 arg0, s32 index, s32 colour);

/* The vehicle being hit: the voice, a hit point less and the player's screen flashing red (called from
   00038) */
void func_m053_021E4(Object *obj, M053HitState *s, s32 updateRate) {
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
void func_m053_0230C(Object *obj, M053Water *w, s32 updateRate) {
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
void func_m053_023D0(s16 *angles, M053Cam *s, s32 updateRate) {
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
void func_m053_02708(UNUSED s32 arg0, UNUSED s32 arg1, UNUSED s32 arg2, UNUSED s32 arg3) {
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
void func_m053_0271C(Object *obj, M053Anim *s, UNUSED s32 arg2, f32 updateRateF) {
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
void func_m053_028CC(UNUSED s32 arg0, M053State *state, UNUSED s32 arg2) {
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
extern void func_80009A04(Object *obj, f32 dx, f32 dy, f32 dz); /* DKR objMoveXYZ */

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
