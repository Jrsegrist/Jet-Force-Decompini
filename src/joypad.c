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
u8 sControllerConnected[MAXCONTROLLERS]; // JFG
s32 sNumControllers;                     // JFG

/******************************/

/**
 * Return the serial interface message queue.
 * Official name: joyMessageQ
 */
OSMesgQueue *si_mesg(void) {
    return &sSIMesgQueue;
}

/**
 * Initialise the player controllers, and return the status when finished.
 * JFG: counts the connected controllers, and only checks the first one's
 * presence (input stays off for everyone without it).
 * Official name: joyInit
 */
s32 input_init(void) {
    s32 i;
    u8 bitpattern;

    osCreateMesgQueue(&sSIMesgQueue, &sSIMesgBuf, 1);
    osSetEventMesg(OS_EVENT_SI, &sSIMesgQueue, gSIMesg);
    osContInit(&sSIMesgQueue, &bitpattern, gControllerStatus);
    osContStartReadData(&sSIMesgQueue);
    input_assign_players();

    for (i = 0; i < MAXCONTROLLERS; i++) {
        sPlayerInputEnabled[i] = TRUE;
        sControllerConnected[i] = FALSE;
    }
    sNumControllers = 0;
    for (i = 0; i < MAXCONTROLLERS; i++) {
        if ((bitpattern & (1 << i)) && !(gControllerStatus[i].errno & CONT_NO_RESPONSE_ERROR)) {
            sControllerConnected[i] = TRUE;
            sNumControllers++;
        }
    }

    if (sControllerConnected[0]) {
        return CONTROLLER_EXISTS;
    }
    i = 0;
    while (i < MAXCONTROLLERS) {
        sPlayerInputEnabled[i++] = FALSE;
    }
    return CONTROLLER_MISSING;
}

/* JFG's save flag bits for input_update */
#define SAVE_FLAG_SAVE_GAME 0x40000000       // file (flags >> 3) & 7
#define SAVE_FLAG_CLEAR_GAME 0x20000000      // file (flags >> 6) & 7
#define SAVE_FLAG_CLEAR_ALL_GAMES 0x10000000
#define SAVE_FLAG_ERASE 0x01000000
#define SAVE_FLAG_LOAD_GAME 0x80000000       // file flags & 7
#define SAVE_FLAG_LOAD_ALL_GAMES 0x08000000
#define SAVE_FLAG_LOAD_GLOBAL_FLAGS 0x04000000
#define SAVE_FLAG_SAVE_GLOBAL_FLAGS 0x02000000
#define NUMBER_OF_SAVE_FILES 6
#define SAVE_FILE_SIZE 0x538
typedef struct SaveFile {
    u8 data[SAVE_FILE_SIZE];
} SaveFile;

extern u8 D_800FF318[]; // JFG: the global flags
u8 *func_80047C0C(void);
u8 *func_8004841C(void);
void func_8004BC10(s32 file, u8 *game);
void func_8004BCB4(s32 file, u8 *game);
void func_8004BD78(void);
void func_8004BAF4(s32 file, u8 *game);
void func_8004BE6C(u8 *flags);
void func_8004BEE4(u8 *flags);
void func_8004B56C(s32 updateRate);

/**
 * Reads arg0 for a set of flags on whether to read, write, or erase any save data.
 * Also reads the latest inputs from the controllers, and sets their values.
 * JFG: its own save flags, the connected controllers are counted, and input
 * is cleared per player rather than for everyone.
 * Official name: joyRead
 * NON_MATCHING: the same instructions, but IDO shares one saved register between
 * settings and allSaves where the target gives each its own (five saved
 * registers, frame 0x58 instead of 0x50).
 */
#ifdef NON_MATCHING
s32 input_update(s32 saveDataFlags, s32 updateRate) {
    u8 *allSaves;
    OSMesg unusedMsg;
    u8 *settings;
    s32 i;

    if (osRecvMesg(&sSIMesgQueue, &unusedMsg, OS_MESG_NOBLOCK) == 0) {
        // Back up old controller data
        for (i = 0; i < MAXCONTROLLERS; i++) {
            gControllerPrevData[i] = gControllerCurrData[i];
        }
        osContGetReadData(gControllerCurrData);
        sNumControllers = 0;
        for (i = 0; i < MAXCONTROLLERS; i++) {
            if (gControllerCurrData[i].errno == 0) {
                sControllerConnected[i] = TRUE;
                sNumControllers++;
            } else {
                sControllerConnected[i] = FALSE;
            }
        }
        if (saveDataFlags != 0) {
            settings = func_80047C0C();
            if (saveDataFlags & SAVE_FLAG_SAVE_GAME) {
                func_8004BC10((saveDataFlags >> 3) & 7, settings);
            }
            if (saveDataFlags & SAVE_FLAG_CLEAR_GAME) {
                func_8004BCB4((saveDataFlags >> 6) & 7, settings);
            }
            if (saveDataFlags & SAVE_FLAG_CLEAR_ALL_GAMES) {
                for (i = 0; i < NUMBER_OF_SAVE_FILES; i++) {
                    func_8004BCB4(i, settings);
                }
            }
            if (saveDataFlags & SAVE_FLAG_ERASE) {
                func_8004BD78();
            }
            if (saveDataFlags & SAVE_FLAG_LOAD_GAME) {
                func_8004BAF4(saveDataFlags & 7, settings);
            }
            if (saveDataFlags & SAVE_FLAG_LOAD_ALL_GAMES) {
                allSaves = func_8004841C();
                for (i = 0; i < NUMBER_OF_SAVE_FILES; i++) {
                    func_8004BAF4(i, allSaves + i * SAVE_FILE_SIZE);
                }
            }
            if (saveDataFlags & SAVE_FLAG_LOAD_GLOBAL_FLAGS) {
                func_8004BE6C(D_800FF318);
            }
            if (saveDataFlags & SAVE_FLAG_SAVE_GLOBAL_FLAGS) {
                func_8004BEE4(D_800FF318);
            }
            // Reset all flags
            saveDataFlags = 0;
        }
        func_8004B56C(updateRate);
        osContStartReadData(&sSIMesgQueue);
    }
    for (i = 0; i < MAXCONTROLLERS; i++) {
        if (!sPlayerInputEnabled[i]) {
            gControllerCurrData[i].button = 0;
            gControllerCurrData[i].stick_x = 0;
            gControllerCurrData[i].stick_y = 0;
        }
        // XOR the diff between the last read of the controller data with the current read to see what buttons have been
        // pushed and released.
        gControllerButtonsPressed[i] =
            ((gControllerCurrData[i].button ^ gControllerPrevData[i].button) & gControllerCurrData[i].button) &
            gButtonMask;
        gControllerButtonsReleased[i] =
            ((gControllerCurrData[i].button ^ gControllerPrevData[i].button) & gControllerPrevData[i].button) &
            gButtonMask;
    }
    return saveDataFlags;
}
#else
INCLUDE_ASM("asm/nonmatchings/joypad", func_80043130);
#endif

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

void func_80054530();
void func_800436C0(s32 arg0, s32 arg1) {
    osRecvMesg(&sSIMesgQueue, NULL, OS_MESG_BLOCK);
    func_80054530(arg0, &sSIMesgQueue);
    osRecvMesg(&sSIMesgQueue, NULL, OS_MESG_BLOCK);
    func_80054530(&sSIMesgQueue);
    osRecvMesg(&sSIMesgQueue, NULL, OS_MESG_BLOCK);
    osContStartReadData(&sSIMesgQueue);
    func_80054530(arg1);
}

/* JFG */
s32 func_80043744(void) {
    return 1;
}
