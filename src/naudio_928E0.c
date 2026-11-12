#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): the ADPCM/raw sample loader of a voice (libultra load.c, n_audio
 * form: one command loads a chunk). Written from the assembly with address names; the wave tables, loops and
 * books are libaudio's types.
 */

/* JFG: the n_audio load filter (only what is used here) */
typedef struct NLoadFilter {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ ADPCM_STATE *state;
    /* 0x10 */ ADPCM_STATE *lstate;
    /* 0x14 */ u32 loopStart;
    /* 0x18 */ u32 loopEnd;
    /* 0x1C */ u32 loopCount;
    /* 0x20 */ ALWaveTable *table;
    /* 0x24 */ s32 bookSize;
    /* 0x28 */ s32 (*dma)(s32 addr, s32 len, void *state);
    /* 0x2C */ void *dmaState;
    /* 0x30 */ s32 sample;
    /* 0x34 */ s32 lastsam;
    /* 0x38 */ s32 first;
    /* 0x3C */ s32 memin;
} NLoadFilter;

INCLUDE_ASM("asm/nonmatchings/naudio_928E0", func_80091CE0);

/* libultra alLoadParam: set the wave table (5) or reset (4) */
void func_8009238C(void *filter, s32 paramID, void *param) {
    NLoadFilter *a = (NLoadFilter *) filter;

    switch (paramID) {
        case 5:
            a->table = (ALWaveTable *) param;
            a->memin = (s32) a->table->base;
            a->sample = 0;
            switch (a->table->type) {
                case AL_ADPCM_WAVE:
                    a->table->len = 9 * (a->table->len / 9);
                    a->bookSize = 2 * a->table->waveInfo.adpcmWave.book->order *
                                  a->table->waveInfo.adpcmWave.book->npredictors * 8;
                    if (a->table->waveInfo.adpcmWave.loop) {
                        a->loopStart = a->table->waveInfo.adpcmWave.loop->start;
                        a->loopEnd = a->table->waveInfo.adpcmWave.loop->end;
                        a->loopCount = a->table->waveInfo.adpcmWave.loop->count;
                        bcopy(a->table->waveInfo.adpcmWave.loop->state, a->lstate, sizeof(ADPCM_STATE));
                    } else {
                        a->loopStart = a->loopEnd = a->loopCount = 0;
                    }
                    break;
                case AL_RAW16_WAVE:
                    if (a->table->waveInfo.rawWave.loop) {
                        a->loopStart = a->table->waveInfo.rawWave.loop->start;
                        a->loopEnd = a->table->waveInfo.rawWave.loop->end;
                        a->loopCount = a->table->waveInfo.rawWave.loop->count;
                    } else {
                        a->loopStart = a->loopEnd = a->loopCount = 0;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 4:
            a->lastsam = 0;
            a->first = 1;
            a->sample = 0;
            if (a->table) {
                a->memin = (s32) a->table->base;
                if (a->table->type == AL_ADPCM_WAVE) {
                    if (a->table->waveInfo.adpcmWave.loop) {
                        a->loopCount = a->table->waveInfo.adpcmWave.loop->count;
                    }
                } else if (a->table->type == AL_RAW16_WAVE) {
                    if (a->table->waveInfo.rawWave.loop) {
                        a->loopCount = a->table->waveInfo.rawWave.loop->count;
                    }
                }
            }
            break;
        default:
            break;
    }
}

/* libultra _decodeChunk: DMA a chunk in, set the loop state, then decode */
Acmd *func_80092674(Acmd *ptr, NLoadFilter *f, s32 tsam, s32 nbytes, s16 outp, s16 inp, u32 flags) {
    s32 dramAlign;
    s32 dramLoc;

    if (nbytes > 0) {
        dramLoc = (f->dma)(f->memin, nbytes, f->dmaState);
        dramAlign = dramLoc & 7;
        nbytes += dramAlign;
        {
            Acmd *loadCmd = ptr++;
            loadCmd->words.w0 = (0x04000000 | (((nbytes - (nbytes & 7) + 8) & 0xFFF) << 12)) | (inp & 0xFFF);
            loadCmd->words.w1 = dramLoc - dramAlign;
        }
    } else {
        dramAlign = 0;
    }
    if (flags & 2) {
        Acmd *loopCmd = ptr++;
        loopCmd->words.w0 = 0x0F000000;
        loopCmd->words.w1 = (u32) f->lstate & 0x1FFFFFFF;
    }
    {
        Acmd *decodeCmd = ptr++;
        decodeCmd->words.w0 = 0x01000000 | (((u32) f->state & 0x1FFFFFFF) & 0xFFFFFF);
        decodeCmd->words.w1 = ((flags & 0xF) << 28) | (((tsam << 1) & 0xFFF) << 16) |
                              ((dramAlign & 0xF) << 12) | (outp & 0xFFF);
    }
    f->first = 0;
    return ptr;
}
