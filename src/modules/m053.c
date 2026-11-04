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

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_014F4);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_01850);

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_019B0);

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

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_023D0);

/* Does nothing (called from 00038) */
void func_m053_02708(UNUSED s32 arg0, UNUSED s32 arg1, UNUSED s32 arg2, UNUSED s32 arg3) {
}

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_0271C);

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
