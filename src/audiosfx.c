#include "common.h"

/*
 * The sound player: JFG's form of DKR's src/audiosfx.c (Rare's version of the libultra sound player, on n_audio).
 * Unlike the game code this object was built unoptimised, with debug information, as MIPS II code (the Makefile
 * builds it with -O1 -g2 -mips2): every variable lives on the stack, and its debug prints are real calls.
 * Functions written from DKR keep DKR's names.
 */

/* JFG: a sound's keymap (the sound's parameters packed into the key/velocity fields, as in DKR) */
typedef struct SfxKeyMap {
    /* 0x00 */ u8 velocityMin;
    /* 0x01 */ u8 velocityMax;
    /* 0x02 */ u8 keyMin; // the group in the low 6 bits
    /* 0x03 */ u8 keyMax;
    /* 0x04 */ u8 keyBase;
    /* 0x05 */ s8 detune; // the pitch slide rate (cents)
} SfxKeyMap;

typedef struct SfxSound {
    /* 0x00 */ void *envelope;
    /* 0x04 */ SfxKeyMap *keyMap;
} SfxSound;

/* JFG: a sound state (DKR ALSoundState; JFG's is larger) */
typedef struct SfxState {
    /* 0x00 */ struct SfxState *next;
    /* 0x04 */ struct SfxState *prev;
    /* 0x08 */ SfxSound *sound;
    /* 0x0C */ u8 voice[0x1C];
    /* 0x28 */ f32 slideMult;
    /* 0x2C */ f32 pitch;
    /* 0x30 */ u8 pad30[0x10];
    /* 0x40 */ s8 priority;
    /* 0x41 */ u8 pad41[2];
    /* 0x43 */ u8 flags; // DKR SOUND_FLAG_*: 1 last in sequence, 2 looped, 4 playing, 0x10 retrigger
    /* 0x44 */ u8 state;
} SfxState;

/* JFG: a sound player event (DKR ALSndpEvent's common form) */
typedef struct SfxEvent {
    /* 0x00 */ s16 type;
    /* 0x04 */ SfxState *state;
    /* 0x08 */ s32 param;
    /* 0x0C */ s32 padC;
} SfxEvent;

/* JFG: the sound player (DKR SoundPlayer) */
typedef struct SfxPlayer {
    /* 0x00 */ u8 node[0x14];
    /* 0x14 */ u8 evtq[0x34];
    /* 0x48 */ s32 maxSystemSoundChannels;
    /* 0x4C */ u8 pad4C[0xC];
    /* 0x58 */ s32 maxActiveSounds;
} SfxPlayer;

extern SfxState *D_800A95A0;  // DKR gSoundStateLists.allocHead: the allocated sound states
extern SfxPlayer *D_800A95AC; // DKR gSoundPlayerPtr
extern s32 D_800A95B0;        // DKR gSoundGlobalVolume
extern u16 *D_80104FC8;       // DKR gSoundGroupVolume
extern char D_800AF4CC[];     // JFG: the warning for stopping a NULL sound (.rodata)
extern char D_800AF4FC[];     // JFG: the warning for changing a NULL sound (.rodata)

void func_8008B610(void *evtq, void *evt, s32 delta); // n_audio: post an event (libultra alEvtqPostEvent)
void func_8008C380(void *voice);                       // n_audio: stop a voice (alSynStopVoice)
void func_8008C420(void *voice);                       // n_audio: free a voice (alSynFreeVoice)
f32 alCents2Ratio(s32 cents);
void osSyncPrintf(const char *fmt, ...);
void func_80085918(void *evtq, SfxState *state, u16 eventTypeMask);
void func_80085D50(SfxState *state);

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80084200);

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80084468);

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80084530);

/**
 * Immediately stop the sound and release all associated resources.
 * DKR: sndp_end
 */
void sndp_end(SfxState *state) {
    if (state->flags & 4) {
        func_8008C380(&state->voice);
        func_8008C420(&state->voice);
    }
    func_80085D50(state);
    func_80085918(&D_800A95AC->evtq, state, 0xFFFF);
}

/**
 * Changes the pitch slide and enqueues an event to continue the pitch slide on the next frame.
 * DKR: sndp_apply_pitch_slide
 */
void sndp_apply_pitch_slide(SfxState *state) {
    SfxEvent evt;
    f32 pitch;

    pitch = alCents2Ratio(state->sound->keyMap->detune) * state->pitch;
    evt.type = 0x10;
    evt.state = state;
    evt.param = *((s32 *) &pitch);
    func_8008B610(&D_800A95AC->evtq, &evt, 33333);
}

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80085918);

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80085A1C);

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80085B20);

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80085D50);

/**
 * Sets the priority for voice allocation and for preempting another sound.
 * DKR: sndp_set_priority (Official Name: gsSndpSetPriority)
 */
void sndp_set_priority(SfxState *state, u8 priority) {
    if (state != NULL) {
        state->priority = (s16) priority;
    }
}

/**
 * Retrieves the current sound state.
 * DKR: sndp_get_state (Official Name: gsSndpGetState)
 */
u8 sndp_get_state(SfxState *state) {
    if (state != NULL) {
        return state->state;
    } else {
        return 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/audiosfx", func_80085EEC);

/**
 * Schedules a stop event for the given sound state and disables the retrigger flag.
 * DKR: sndp_stop (Official Name: gsSndpStop); JFG prints a warning for a NULL sound.
 */
void sndp_stop(SfxState *state) {
    SfxEvent evt;

    evt.type = 0x400;
    evt.state = state;
    if (state != NULL) {
        evt.state->flags &= ~0x10;
        func_8008B610(&D_800A95AC->evtq, &evt, 0);
    } else {
        osSyncPrintf(D_800AF4CC);
    }
}

/**
 * Schedules a stop event for all sound states that have the specified flags set.
 * DKR: sndp_stop_with_flags
 */
void sndp_stop_with_flags(u8 flags) {
    OSIntMask mask;
    SfxEvent evt;
    SfxState *state;

    mask = osSetIntMask(OS_IM_NONE);
    state = D_800A95A0;
    while (state != NULL) {
        evt.type = 0x400;
        evt.state = state;
        if ((state->flags & flags) == flags) {
            evt.state->flags &= ~0x10;
            func_8008B610(&D_800A95AC->evtq, &evt, 0);
        }
        state = state->next;
    }
    osSetIntMask(mask);
}

/**
 * Stops all sounds from playing.
 * DKR: sndp_stop_all (Official Name: gsSndpStopAll)
 */
void sndp_stop_all(void) {
    sndp_stop_with_flags(1);
}

/**
 * Stops all retriggered sounds from playing.
 * DKR: sndp_stop_all_retrigger (Official Name: gsSndpStopAllRetrigger)
 */
void sndp_stop_all_retrigger(void) {
    sndp_stop_with_flags(1 | 0x10);
}

/**
 * Stops all looped sounds from playing.
 * DKR: sndp_stop_all_looped (Official Name: gsSndpStopAllLooped)
 */
void sndp_stop_all_looped(void) {
    sndp_stop_with_flags(1 | 2);
}

/**
 * Send a message to the sound player to update an existing property of the sound entry.
 * DKR: sndp_set_param (Official Name: gsSndpSetParam); JFG prints a warning for a NULL sound.
 */
void sndp_set_param(SfxState *state, s16 type, u32 param) {
    SfxEvent evt;

    evt.type = type;
    evt.state = state;
    evt.param = param;
    if (state != NULL) {
        func_8008B610(&D_800A95AC->evtq, &evt, 0);
    } else {
        osSyncPrintf(D_800AF4FC);
    }
}

/**
 * Returns the volume level of the group.
 * DKR: sndp_get_group_volume (Official Name: gsSndpGetMasterVolume)
 */
u16 sndp_get_group_volume(u8 group) {
    return D_80104FC8[group];
}

/**
 * Sets the volume for the specified group and updates the volume of all sounds in that group.
 * DKR: sndp_set_group_volume (Official Name: gsSndpSetMasterVolume); JFG counts the states it walks.
 */
void sndp_set_group_volume(u8 group, u16 volume) {
    OSIntMask mask;
    SfxState *state;
    s32 count;
    SfxEvent evt;

    mask = osSetIntMask(OS_IM_NONE);
    state = D_800A95A0;
    D_80104FC8[group] = volume;
    for (count = 0; state != NULL;) {
        if ((state->sound->keyMap->keyMin & 0x3F) == group) {
            evt.type = 0x800;
            evt.state = state;
            func_8008B610(&D_800A95AC->evtq, &evt, 0);
        }
        count++, state = state->next;
    }
    osSetIntMask(mask);
}

/**
 * Sets the global volume level for all sounds.
 * DKR: sndp_set_global_volume
 */
void sndp_set_global_volume(u32 volume) {
    if (volume > 256) {
        volume = 256;
    }
    D_800A95B0 = volume;
}

/**
 * Returns the global volume level applied to all sounds.
 * DKR: sndp_get_global_volume (Official Name: gsSndpGetGlobalVolume)
 */
s32 sndp_get_global_volume(void) {
    return D_800A95B0;
}

/**
 * Sets the number of active sound channels to the passed number or the maximum, whichever is lower.
 * DKR: sndp_set_active_sound_limit (Official Name: gsSndpLimitVoices)
 */
void sndp_set_active_sound_limit(s32 numSounds) {
    if (D_800A95AC->maxSystemSoundChannels >= numSounds) {
        D_800A95AC->maxActiveSounds = numSounds;
    } else {
        D_800A95AC->maxActiveSounds = D_800A95AC->maxSystemSoundChannels;
    }
}
