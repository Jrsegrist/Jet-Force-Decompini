/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): compact sequence player
 * channel switches and fades (sent as controller messages).
 */
#include "common.h"

/* JFG: a compact sequence player's channel state (0x34 bytes) */
typedef struct NChanState {
    /* 0x00 */ u8 pad0[0xD];
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 padF[0x34 - 0xF];
} NChanState;

/* JFG: an n_audio compact sequence player (only the fields used here) */
typedef struct NSeqPlayer {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ u16 chanMask;
    /* 0x32 */ u8 pad32[2];
    /* 0x34 */ u8 maxChannels;
    /* 0x35 */ u8 pad35[0x64 - 0x35];
    /* 0x64 */ NChanState *chanState;
} NSeqPlayer;

void func_800911E0(NSeqPlayer *seqp, s32 ticks, s32 status, s32 byte1, s32 byte2); // n_audio: send a MIDI message

/* JFG: switch every channel on. */
void func_8008C500(NSeqPlayer *seqp) {
    s32 i;

    seqp->chanMask = 0xFFFF;
    for (i = 0; i < seqp->maxChannels; i++) {
        seqp->chanState[i].unkE = 0xFF;
        seqp->chanState[i].unkD = 0xFF;
    }
}

/* JFG: switch a channel off (controller 0xFF, 0). */
void func_8008C598(NSeqPlayer *seqp, s32 chan) {
    func_800911E0(seqp, 0, chan | 0xB0, 0xFF, 0);
}

/* JFG: switch a channel on (controller 0xFF, 0xFF). */
void func_8008C5E0(NSeqPlayer *seqp, s32 chan) {
    seqp->chanMask |= 1 << chan;
    func_800911E0(seqp, 0, chan | 0xB0, 0xFF, 0xFF);
}

/* JFG: fade a channel (controller 0xFF). */
void func_8008C648(NSeqPlayer *seqp, s32 chan, u8 value) {
    func_800911E0(seqp, 0, chan | 0xB0, 0xFF, value);
}

/* JFG: a channel's surround setting (controller 0x41). */
void func_8008C698(NSeqPlayer *seqp, s32 chan, u8 value) {
    func_800911E0(seqp, 0, chan | 0xB0, 0x41, value);
}

/* JFG: force a channel's fade (controller 0xFC). */
void func_8008C6E8(NSeqPlayer *seqp, s32 chan, u8 value) {
    func_800911E0(seqp, 0, chan | 0xB0, 0xFC, value);
}
