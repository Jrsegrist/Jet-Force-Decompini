/*
 * Runlink module 53: a vehicle the players ride (its init, control and camera; still mostly
 * assembly).
 */
#include "module.h"

/* The module's .rodata and .bss (still assembly) */
extern f32 rl_m053_03710;
extern f32 rl_m053_03828[4];
extern f32 rl_m053_03838[4];
extern f32 rl_m053_03848[4];

/* Resets player `player`'s camera values */
void func_m053_00000(s32 player) {
    rl_m053_03828[player] = rl_m053_03710;
    rl_m053_03838[player] = 0.0f;
    rl_m053_03848[player] = 0.0f;
}

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_00038);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_00EA0);

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
extern void func_8003AABC(s16 *angles, M053Steer *s, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s16 *yaw, s16 *pitch,
                          f32 updateRate);
extern s16 angle_lerp(s16 from, s16 to, f32 t);
extern f32 func_80049BEC(f32 rate, s32 updateRate);
extern s32 func_8004966C(s16 angle); /* sine */
extern s16 *rl_m053_03820;          /* .bss (still assembly): the camera commands */
extern f32 rl_m053_037A0;           /* .rodata (still assembly) */
extern f32 rl_m053_037A4;
extern f32 rl_m053_037A8;
extern f32 rl_m053_037AC;

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
    s->unk4 *= func_80049BEC(rl_m053_037A0, (s32) updateRateF);
    if (rl_m053_037A4 < s->unk4 && s->unk4 < rl_m053_037A8) {
        s->unk4 = 0.0f;
    }
    func_8003AABC(angles, s, 0xE38, 0xE38, -0x2AAA, 0x2AAA, &yaw, &pitch, updateRateF);
    k = 1.0f - func_80049BEC(rl_m053_037AC, (s32) updateRateF);
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
INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_014F4);
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
extern f32 rl_m053_037B0; /* .rodata (still assembly) */
extern f32 rl_m053_037B4;
extern f32 rl_m053_037B8;

/* Slows the vehicle down by the frame's rate (called from 00038) */
void func_m053_01850(Object *obj, M053Slow *s, UNUSED s32 arg2, UNUSED s32 arg3, f32 updateRateF) {
    s32 updateRate;
    f32 f;

    obj->interactObj->hitPoints = 0;
    updateRate = updateRateF;
    f = func_80049BEC(rl_m053_037B0, updateRate);
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
        obj->velocity.y *= func_80049BEC(rl_m053_037B4, updateRate);
    }
    if (rl_m053_037B8 < obj->unk28) {
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

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_01BB4);

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

extern void func_8006B090(Object *obj, s32 updateRate); /* moves a ripple */
extern f32 rl_m053_037D0; /* .rodata (still assembly): 0230C's limits */
extern f32 rl_m053_037D4;

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
        if (child != NULL && (w->unk4 < rl_m053_037D0 || rl_m053_037D4 < w->unk4)) {
            *child |= 2;
        }
    }
    func_8006B090(obj, updateRate);
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
extern f32 rl_m053_037D8;           /* .rodata (still assembly) */
extern f32 rl_m053_037DC;
extern f32 rl_m053_037E0;
extern f32 rl_m053_037E4;

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
    if (a < rl_m053_037D8) { a = -0x38E3; } if (rl_m053_037DC < a) {
        a = 0x38E3;
    }
    if (b < rl_m053_037E0) {
        b = -0xE38;
    }
    if (rl_m053_037E4 < b) {
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

extern f32 rl_m053_037E8; /* .rodata (still assembly) */
extern f32 rl_m053_037EC;
extern f32 rl_m053_037F0;
extern f32 rl_m053_037F4;
extern f32 rl_m053_037F8;

/* Plays the vehicle's two animations and eases unk14 towards 0, 1 or 2 (called from 00038) */
void func_m053_0271C(Object *obj, M053Anim *s, UNUSED s32 arg2, f32 updateRateF) {
    f32 k;

    k = 1.0f - func_80049BEC(rl_m053_037E8, updateRateF);
    switch (obj->animationID) {
        case 0:
            func_8001138C(obj, 0.005f, updateRateF);
            if (s->unk4 < rl_m053_037EC || rl_m053_037F0 < s->unk4) {
                obj_anim_set(obj, 1, obj->unk28);
            }
            s->unk14 += (0.0f - s->unk14) * k;
            break;
        case 1:
            func_8001138C(obj, 0.01f, updateRateF);
            if (rl_m053_037F4 < s->unk4 && s->unk4 < rl_m053_037F8) {
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

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_028DC);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_02B48);

INCLUDE_ASM("asm/nonmatchings/modules/m053", data);

INCLUDE_ASM("asm/nonmatchings/modules/m053", rodata);

INCLUDE_ASM("asm/nonmatchings/modules/m053", bss);
