/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): set a voice's volume (cf.
 * libultra's alSynSetVol). Written from the assembly.
 */
#include "common.h"

/* JFG: an n_audio physical voice (only the field used here) */
typedef struct NPVoice {
    /* 0x00 */ u8 pad0[0x88];
    /* 0x88 */ s32 offset;
} NPVoice;

/* JFG: an n_audio voice */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ NPVoice *pvoice;
} NVoice;

/* JFG: an n_audio synthesizer (only the field used here) */
typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s32 paramSamples;
} NSynth;

/* JFG: a parameter update for the envelope mixer (libultra ALParam) */
typedef struct NParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ s32 data;
    /* 0x10 */ s32 moredata;
} NParam;

#define N_FILTER_SET_VOLUME 11
#define N_FILTER_ADD_UPDATE 3

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
NParam *func_8008AF34(void);                                   // n_audio: allocate a parameter update
s32 func_8008B124(s32 micros);                                 // n_audio: microseconds to whole sample frames
void func_80090B14(NPVoice *pvoice, s32 paramID, void *param); // n_audio: the envelope mixer's param handler

/* JFG: ramp a voice's volume to vol over t microseconds. */
void func_8008BF70(NVoice *v, s16 vol, s32 t) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = N_FILTER_SET_VOLUME;
        update->data = vol;
        update->moredata = func_8008B124(t);
        update->next = 0;
        func_80090B14(v->pvoice, N_FILTER_ADD_UPDATE, update);
    }
}
