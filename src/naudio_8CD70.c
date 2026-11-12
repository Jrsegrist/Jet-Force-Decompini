/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): synthesizer voice parameters
 * (cf. libultra's alSynSetPan, alSynSetPitch, alSynSetFXMix, alSynStopVoice and alSynFreeVoice).
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

/* JFG: a parameter update for the envelope mixer (libultra ALParam / ALFreeParam) */
typedef struct NParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ union {
        f32 f;
        s32 i;
        NPVoice *pvoice;
    } data;
} NParam;

#define N_FILTER_FREE_VOICE 0
#define N_FILTER_SET_PITCH 7
#define N_FILTER_SET_PAN 12
#define N_FILTER_STOP_VOICE 15
#define N_FILTER_SET_FXAMT 16
#define N_FILTER_ADD_UPDATE 3

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
NParam *func_8008AF34(void);                                   // n_audio: allocate a parameter update
void func_80090B14(NPVoice *pvoice, s32 paramID, void *param); // n_audio: the envelope mixer's param handler
void func_8008B080(NPVoice *pvoice);                           // n_audio: free a physical voice

/* JFG: set a voice's pan. */
void func_8008C170(NVoice *v, u8 pan) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = N_FILTER_SET_PAN;
        update->data.i = pan;
        update->next = 0;
        func_80090B14(v->pvoice, N_FILTER_ADD_UPDATE, update);
    }
}

/* JFG: set a voice's pitch. */
void func_8008C220(NVoice *v, f32 pitch) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = N_FILTER_SET_PITCH;
        update->data.f = pitch;
        update->next = 0;
        func_80090B14(v->pvoice, N_FILTER_ADD_UPDATE, update);
    }
}

/* JFG: set a voice's effect mix. */
void func_8008C2D0(NVoice *v, u8 fxmix) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = N_FILTER_SET_FXAMT;
        update->data.i = fxmix;
        update->next = 0;
        func_80090B14(v->pvoice, N_FILTER_ADD_UPDATE, update);
    }
}

/* JFG: stop a voice. */
void func_8008C380(NVoice *v) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = N_FILTER_STOP_VOICE;
        update->next = 0;
        func_80090B14(v->pvoice, N_FILTER_ADD_UPDATE, update);
    }
}

/* JFG: free a voice: hand its physical voice back now, or at the end of its pending updates. */
void func_8008C420(NVoice *voice) {
    UNUSED void *f;
    NParam *update;

    if (voice->pvoice) {
        if (voice->pvoice->offset) {
            update = func_8008AF34();
            if (update == NULL) {
                return;
            }
            update->delta = D_800A97C4->paramSamples + voice->pvoice->offset;
            update->type = N_FILTER_FREE_VOICE;
            update->data.pvoice = voice->pvoice;
            func_80090B14(voice->pvoice, N_FILTER_ADD_UPDATE, update);
        } else {
            func_8008B080(voice->pvoice);
        }
        voice->pvoice = NULL;
    }
}
