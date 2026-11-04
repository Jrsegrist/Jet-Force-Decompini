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

INCLUDE_ASM("asm/nonmatchings/modules/m053", func_m053_021E4);

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
