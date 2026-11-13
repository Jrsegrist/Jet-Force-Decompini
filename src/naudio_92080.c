/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): pull an auxiliary bus (cf.
 * libultra's alAuxBusPull). JFG's first mixes the voices whose filter rate is 0x40 or more and sets a gain
 * from the loudest of them, then the rest of the bus's voices. Written from the assembly.
 */
#include "common.h"

struct NVoice;

/* JFG: an n_audio physical voice (only the fields used here) */
typedef struct NPVoice {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ struct NVoice *vvoice;
    /* 0x0C */ u8 padC[0x8C - 0xC];
    /* 0x8C */ u8 lp_rate;
} NPVoice;

/* JFG: an n_audio voice (only the fields used here) */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ NPVoice *pvoice;
    /* 0x0C */ u8 padC[0x18 - 0xC];
    /* 0x18 */ s16 fxBus;
} NVoice;

/* JFG: an auxiliary bus (0x48 bytes; see naudio_8B330.c) */
typedef struct NAuxBus {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ u32 sourceCount;
    /* 0x18 */ s32 maxSources;
    /* 0x1C */ NPVoice **sources;
    /* 0x20 */ u8 pad20[0x48 - 0x20];
} NAuxBus;

/* JFG: an n_audio synthesizer (only the field used here) */
typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ NAuxBus *auxBus;
} NSynth;

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
f32 sqrtf(f32 x);
Acmd *func_800901F0(void *filter, s32 sampleOffset, Acmd *p); // n_audio: the envelope mixer's pull

/* JFG: pull an auxiliary bus's voices into the effect input; *flag counts the voices that produced output. */
Acmd *func_80091480(s32 sampleOffset, Acmd *p, s32 bus, s32 *flag) {
    Acmd *ptr = p;
    NAuxBus *m = &D_800A97C4->auxBus[bus];
    NPVoice **sources = m->sources;
    u32 i;
    s32 count = 0;
    u32 maxV = 1;
    u32 gain;

    aClearBuffer(ptr++, 0x7C0, 0x2E0);
    *flag = 0;
    for (i = 0; i < m->sourceCount; i++) {
        if (sources[i]->vvoice && sources[i]->vvoice->fxBus == bus && sources[i]->vvoice->pvoice &&
            sources[i]->vvoice->pvoice->lp_rate >= 0x40) {
            ptr = func_800901F0(sources[i], sampleOffset, ptr);
            (*flag)++;
            count++;
            if (sources[i]->vvoice->pvoice->lp_rate > maxV) {
                maxV = sources[i]->vvoice->pvoice->lp_rate;
            }
        }
    }
    if (count) {
        maxV -= 0x3E;
        if (maxV >= 7) {
            gain = 26755.0f / sqrtf(maxV);
        } else {
            gain = 65536.0f / maxV;
        }
        {
            Acmd *_a = ptr++;

            _a->words.w0 = 0x4E0;
            _a->words.w1 = _SHIFTL(gain, 16, 16) | _SHIFTL(maxV, 0, 16);
        }
        {
            Acmd *_a = ptr++;

            _a->words.w0 = 0x650;
            _a->words.w1 = _SHIFTL(gain, 16, 16) | _SHIFTL(maxV, 0, 16);
        }
    }
    for (i = 0; i < m->sourceCount; i++) {
        if ((sources[i]->vvoice && sources[i]->vvoice->fxBus == bus &&
             (!sources[i]->vvoice->pvoice || sources[i]->vvoice->pvoice->lp_rate < 0x40)) ||
            (!sources[i]->vvoice && !bus)) {
            Acmd *prev = ptr;

            ptr = func_800901F0(sources[i], sampleOffset, ptr);
            if (ptr != prev) {
                (*flag)++;
            }
        }
    }
    return ptr;
}
