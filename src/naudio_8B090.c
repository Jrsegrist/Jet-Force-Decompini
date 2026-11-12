/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): compact sequence player
 * messages and library init (cf. libultra's alCSPSetChlXXX and alInit).
 */
#include "common.h"

/* JFG: an instrument of an n_audio bank (only the field used here) */
typedef struct NInstrument {
    /* 0x00 */ u8 pad0[3];
    /* 0x03 */ u8 msg; // JFG: bit 0 kept, the message in bits 1-7
} NInstrument;

typedef struct NBank {
    /* 0x00 */ s16 instCount;
    /* 0x02 */ u8 pad2[0xC - 0x2];
    /* 0x0C */ NInstrument *instArray[1];
} NBank;

/* JFG: a compact sequence player's channel state (0x34 bytes) */
typedef struct NChanState {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ u8 msg;
    /* 0x11 */ u8 pad11[0x34 - 0x11];
} NChanState;

/* JFG: an n_audio compact sequence player (only the fields used here) */
typedef struct NSeqPlayer {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ NBank *bank;
    /* 0x24 */ u8 pad24[0x64 - 0x24];
    /* 0x64 */ NChanState *chanState;
    /* 0x68 */ u8 pad68[0x88 - 0x68];
    /* 0x88 */ void *msgQ;
} NSeqPlayer;

/* JFG: set the message queue a sequence player posts its messages to. */
void func_8008A490(NSeqPlayer *seqp, void *msgQ) {
    seqp->msgQ = msgQ;
}

/* JFG: set the message an instrument of the player's bank sends. */
void func_8008A4A4(NSeqPlayer *seqp, u8 inst, u8 msg) {
    NInstrument *instrument;

    if (inst < seqp->bank->instCount) {
        instrument = seqp->bank->instArray[inst];
        if (instrument != NULL) {
            instrument->msg = (msg << 1) | (instrument->msg & 1);
        }
    }
}

/* JFG: set the message a channel of the player sends. */
void func_8008A50C(NSeqPlayer *seqp, s32 chan, u8 msg) {
    seqp->chanState[chan].msg = msg;
}

extern void *D_800A97C0; // JFG: the audio globals (libultra alGlobals)
extern void *D_800A97C4; // JFG: the n_audio synthesizer (its first member)
void func_8008A730(void *config);
void func_8008EA90(void);

/* JFG: the library init: keep the globals and create the synthesizer (libultra alInit). */
void func_8008A540(void *globals, void *config) {
    if (D_800A97C0 == NULL) {
        D_800A97C0 = globals;
        if (D_800A97C4 == NULL) {
            D_800A97C4 = D_800A97C0;
            func_8008A730(config);
        }
    }
}

/* JFG: the library shutdown (libultra alClose). */
void func_8008A5AC(void *globals) {
    if (D_800A97C0 != NULL) {
        func_8008EA90();
        D_800A97C0 = NULL;
        D_800A97C4 = NULL;
    }
}
