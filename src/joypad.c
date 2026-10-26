/*
 * Controller input.
 *
 * Mostly the Diddy Kong Racing decompilation's src/joypad.c (CC0 1.0), which
 * compiles to the same code here. Functions marked "JFG" are not in DKR.
 */
#include "joypad.h"

/************ .data ************/

extern u16 gButtonMask; // Cleared when the anti-tamper checks fail

/************ .bss ************/

OSMesgQueue sSIMesgQueue;
OSMesg sSIMesgBuf;
OSMesg gSIMesg;
OSContStatus gControllerStatus[MAXCONTROLLERS];
OSContPad gControllerCurrData[MAXCONTROLLERS];
OSContPad gControllerPrevData[MAXCONTROLLERS];
u16 gControllerButtonsPressed[MAXCONTROLLERS];
u16 gControllerButtonsReleased[MAXCONTROLLERS];
u8 sPlayerID[MAXCONTROLLERS];
u8 sPlayerInputEnabled[MAXCONTROLLERS]; // JFG
s32 D_800FB108;                         // JFG
s32 D_800FB10C;                         // JFG

/******************************/

/**
 * Return the serial interface message queue.
 * Official name: joyMessageQ
 */
OSMesgQueue *si_mesg(void) {
    return &sSIMesgQueue;
}

INCLUDE_ASM("asm/nonmatchings/joypad", func_80042FEC);

INCLUDE_ASM("asm/nonmatchings/joypad", func_80043130);

/**
 * Set the first 4 player ID's to the controller numbers, so players can input in the menus after boot.
 * Official name: joyResetMap
 * (IDO only unrolls this loop into stores through one lui because sPlayerID
 * is defined in this file.)
 */
void input_assign_players(void) {
    s32 i;
    for (i = 0; i < MAXCONTROLLERS; i++) {
        sPlayerID[i] = i;
    }
}

/* JFG: stop taking input for this player. */
void func_80043404(s32 player) {
    sPlayerInputEnabled[player & 3] = FALSE;
}

/* JFG: take input for this player again. */
void func_80043418(s32 player) {
    sPlayerInputEnabled[player & 3] = TRUE;
}

/**
 * Assign the first four player ID's to the index of the connected players.
 * Assign the next four player ID's to the index of the players who are not connected.
 * Official name: joyCreateMap
 */
void charselect_assign_players(s8 *activePlayers) {
    s32 i;
    s32 temp = 0;
    for (i = 0; i < MAXCONTROLLERS; i++) {
        if (activePlayers[i]) {
            sPlayerID[temp++] = i;
        }
    }
    for (i = 0; i < MAXCONTROLLERS; i++) {
        if (!activePlayers[i]) {
            sPlayerID[temp++] = i;
        }
    }
}

/**
 * Returns the id of the selected index.
 * Official name: joyGetController
 */
u8 input_player_id(s32 player) {
    return sPlayerID[player];
}

/**
 * Returns the buttons that are currently pressed down on the controller.
 * Official name: joyGetButtons
 */
u16 input_held(s32 player) {
    return gControllerCurrData[sPlayerID[player]].button;
}

/**
 * Returns the buttons that are newly pressed during that frame.
 * Official name: joyGetPressed
 */
u32 input_pressed(s32 player) {
    return gControllerButtonsPressed[sPlayerID[player]];
}

/**
 * Returns the buttons that are no longer pressed in that frame.
 * Official name: joyGetReleased
 */
u16 input_released(s32 player) {
    return gControllerButtonsReleased[sPlayerID[player]];
}

/**
 * Clamps the X joystick axis of the selected player and returns it.
 * Official name: joyGetStickX
 */
s32 input_clamp_stick_x(s32 player) {
    return input_clamp_stick_mag(gControllerCurrData[sPlayerID[player]].stick_x);
}

/* JFG: the raw X joystick axis of the selected player. */
s8 func_8004359C(s32 player) {
    return gControllerCurrData[sPlayerID[player]].stick_x;
}

/**
 * Clamps the Y joystick axis of the selected player and returns it.
 * Official name: joyGetStickY
 */
s32 input_clamp_stick_y(s32 player) {
    return input_clamp_stick_mag(gControllerCurrData[sPlayerID[player]].stick_y);
}

/* JFG: the raw Y joystick axis of the selected player. */
s8 func_8004360C(s32 player) {
    return gControllerCurrData[sPlayerID[player]].stick_y;
}

/**
 * Keeps the joysticks axis reads no higher than JOYSTICK_MAX_RANGE.
 * Will also pull the reading towards the centre.
 */
s8 input_clamp_stick_mag(s8 stickMag) {
    if (stickMag < JOYSTICK_DEADZONE && stickMag > -JOYSTICK_DEADZONE) {
        return 0;
    }
    if (stickMag > 0) {
        stickMag -= JOYSTICK_DEADZONE;
        if (stickMag > JOYSTICK_MAX_RANGE) {
            stickMag = JOYSTICK_MAX_RANGE;
        }
    } else {
        stickMag += JOYSTICK_DEADZONE;
        if (stickMag < -JOYSTICK_MAX_RANGE) {
            stickMag = -JOYSTICK_MAX_RANGE;
        }
    }
    return stickMag;
}

/**
 * Used when the anti-tamper checks fail.
 * Official Name: joySetSecurity
 */
void drm_disable_input(void) {
    gButtonMask = 0;
}

INCLUDE_ASM("asm/nonmatchings/joypad", func_800436C0);

/* JFG */
s32 func_80043744(void) {
    return 1;
}
