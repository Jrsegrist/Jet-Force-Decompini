#ifndef _JOYPAD_H_
#define _JOYPAD_H_

/* Adapted from the Diddy Kong Racing decompilation (CC0 1.0), src/joypad.h. */

#include "common.h"

#define CONTROLLER_MISSING -1
#define CONTROLLER_EXISTS 0

/* Jet Force Gemini uses a smaller dead zone and range than DKR (8 and 70). */
#define JOYSTICK_DEADZONE 5
#define JOYSTICK_MAX_RANGE 65

OSMesgQueue *si_mesg(void);
void input_assign_players(void);
void charselect_assign_players(s8 *activePlayers);
u8 input_player_id(s32 player);
u16 input_held(s32 player);
u32 input_pressed(s32 player);
u16 input_released(s32 player);
s32 input_clamp_stick_x(s32 player);
s32 input_clamp_stick_y(s32 player);
s8 input_clamp_stick_mag(s8 stickMag);
void drm_disable_input(void);

#endif
