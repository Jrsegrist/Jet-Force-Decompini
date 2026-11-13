/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): start a voice with all its
 * settings (cf. libultra's alSynStartVoiceParams; JFG's also sets the voice's low-pass filter). Written from the
 * assembly.
 */
#include "common.h"

/* JFG: an n_audio physical voice (only the field used here) */
typedef struct NPVoice {
    /* 0x00 */ u8 pad0[0x88];
    /* 0x88 */ s32 offset;
} NPVoice;

/* JFG: an n_audio voice (only the fields used here) */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ NPVoice *pvoice;
    /* 0x0C */ u8 padC[0x1A - 0xC];
    /* 0x1A */ s16 unityPitch;
} NVoice;

/* JFG: an n_audio synthesizer (only the field used here) */
typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s32 paramSamples;
} NSynth;

/* JFG: a voice start with all its settings (libultra ALStartParamAlt, n_ form; see naudio_90DF0.c) */
typedef struct NStartParamAlt {
    /* 0x00 */ struct NStartParamAlt *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0A */ s16 unity;
    /* 0x0C */ f32 pitch;
    /* 0x10 */ s16 volume;
    /* 0x12 */ u8 pan;
    /* 0x13 */ u8 fxMix;
    /* 0x14 */ u8 lpRate;
    /* 0x15 */ u8 lpGain;
    /* 0x18 */ f32 lpCutoff;
    /* 0x1C */ s32 samples;
    /* 0x20 */ void *wave;
} NStartParamAlt;

#define N_FILTER_START_VOICE_ALT 13
#define N_FILTER_ADD_UPDATE 3

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
NStartParamAlt *func_8008AF34(void);                           // n_audio: allocate a parameter update
s32 func_8008B124(s32 micros);                                 // n_audio: microseconds to whole sample frames
void func_80090B14(NPVoice *pvoice, s32 paramID, void *param); // n_audio: the envelope mixer's param handler

/* JFG: start a voice playing a wave, with its pitch, volume, pan, effect mix and filter, ramping in over t. */
void func_8008C040(NVoice *v, void *wave, f32 pitch, s16 vol, u8 pan, u8 fxmix, u8 lpGain, f32 lpCutoff,
                   u8 lpRate, s32 t) {
    NStartParamAlt *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->next = 0;
        update->type = N_FILTER_START_VOICE_ALT;
        update->unity = v->unityPitch;
        update->pan = pan;
        update->volume = vol;
        update->fxMix = fxmix;
        update->pitch = pitch;
        update->lpRate = lpRate;
        update->lpGain = lpGain;
        update->lpCutoff = lpCutoff;
        update->samples = func_8008B124(t);
        update->wave = wave;
        func_80090B14(v->pvoice, N_FILTER_ADD_UPDATE, update);
    }
}
