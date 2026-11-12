#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): libultra alCSPSendMidi, n_audio form: the event goes through the
 * extern-event poster with the player.
 */

/* JFG: a sequence player MIDI event (libultra ALEvent's midi form) */
typedef struct NMidiEvent {
    /* 0x00 */ s16 type;
    /* 0x04 */ s32 ticks;
    /* 0x08 */ u8 status;
    /* 0x09 */ u8 byte1;
    /* 0x0A */ u8 byte2;
    /* 0x0C */ u32 duration;
} NMidiEvent;

void func_8008B7EC(void *seqp, void *evtq, NMidiEvent *evt, s32 delta); // n_audio: post an extern event

void func_800911E0(u8 *seqp, s32 ticks, u8 status, u8 byte1, u8 byte2) {
    NMidiEvent evt;
    s32 delta;

    evt.type = 2;
    evt.ticks = 0;
    evt.status = status;
    evt.byte1 = byte1;
    evt.byte2 = byte2;
    evt.duration = 0;
    delta = ticks;
    func_8008B7EC(seqp, seqp + 0x48, &evt, delta);
}
