#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): a voice's distortion, and libultra's __alCSeqNextDelta (cseq.c)
 * in its own object. n_al* names are runlink names only: address names here.
 */

/* JFG: an n_audio physical voice (only what is used here) */
typedef struct NPVoice {
    /* 0x00 */ u8 pad0[0x88];
    /* 0x88 */ s32 offset;
} NPVoice;

/* JFG: an n_audio voice (libultra ALVoice) */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[8];
    /* 0x08 */ NPVoice *pvoice;
} NVoice;

/* JFG: an n_audio parameter update (libultra ALParam) */
typedef struct NParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ s32 data;
} NParam;

typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s32 paramSamples;
} NSynth;

extern NSynth *D_800A97C4; // the n_audio synthesizer

NParam *func_8008AF34(void);                              // n_audio: __allocParam
void func_80090B14(NPVoice *filter, s32 paramID, void *param); // n_audio: the envelope mixer's parameter handler

/* JFG: set a voice's distortion (an n_audio parameter, type 0x11) */
void func_8008E900(NVoice *v, u8 distortion) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = 0x11;
        update->data = distortion;
        update->next = 0;
        func_80090B14(v->pvoice, 3, update);
    }
}

/*
 * libultra's __alCSeqNextDelta (cseq.c), written from the assembly: the ticks to the next event of any valid
 * track (each track's delta first counted down by the last delta, when flagged). FALSE when no track is valid.
 */
u32 __alCSeqNextDelta(ALCSeq *seq, s32 *pDeltaTicks) {
    u32 track;
    u32 nearest = 0xFFFFFFFF;
    u32 elapsed = seq->lastDeltaTicks;

    if (!seq->validTracks) {
        return FALSE;
    }
    for (track = 0; track < 16; track++) {
        if ((seq->validTracks >> track) & 1) {
            if (seq->deltaFlag) {
                seq->evtDeltaTicks[track] -= elapsed;
            }
            if (seq->evtDeltaTicks[track] < nearest) {
                nearest = seq->evtDeltaTicks[track];
            }
        }
    }
    seq->deltaFlag = 0;
    *pDeltaTicks = nearest;
    return TRUE;
}
