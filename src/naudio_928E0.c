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

Acmd *func_80092674(Acmd *ptr, NLoadFilter *f, s32 tsam, s32 nbytes, s16 outp, s16 inp, u32 flags);

/* libultra alAdpcmPull (n_audio form): decode outCount samples of the voice's wave, through its loop, padding
   past the end of the wave with silence */
Acmd *func_80091CE0(NLoadFilter *filter, s16 *outp, s32 outCount, Acmd *p) {
    Acmd *ptr = p;
    s16 inp;
    s32 tsam;
    s32 nframes;
    s32 nbytes;
    s32 overFlow;
    s32 startZero;
    s32 nOver;
    s32 nSam;
    s32 op;
    s32 nLeft;
    s32 bEnd;
    s32 decoded = 0;
    s32 looped = 0;
    NLoadFilter *f = filter;

    if (!outCount) {
        return ptr;
    }
    inp = 0;
    {
        Acmd *_a = ptr++;

        _a->words.w0 = _SHIFTL(A_LOADADPCM, 24, 8) | _SHIFTL(f->bookSize, 0, 24);
        _a->words.w1 = (u32) f->table->waveInfo.adpcmWave.book->book & 0x1FFFFFFF;
    }
    looped = (f->sample + outCount > f->loopEnd) && (f->loopCount != 0);
    if (looped) {
        nSam = f->loopEnd - f->sample;
    } else {
        nSam = outCount;
    }
    if (f->lastsam) {
        nLeft = 16 - f->lastsam;
    } else {
        nLeft = 0;
    }
    tsam = nSam - nLeft;
    if (tsam < 0) {
        tsam = 0;
    }
    nframes = (tsam + 15) >> 4;
    nbytes = nframes * 9;
    if (looped) {
        ptr = func_80092674(ptr, f, tsam, nbytes, *outp, inp, f->first);
        if (f->lastsam) {
            *outp += f->lastsam << 1;
        } else {
            *outp += 32;
        }
        f->lastsam = f->loopStart & 0xF;
        f->memin = (s32) f->table->base + 9 * ((s32) (f->loopStart >> 4) + 1);
        f->sample = f->loopStart;
        bEnd = *outp;
        while (outCount > nSam) {
            outCount -= nSam;
            op = (((nframes + 1) << 5) + bEnd + 16) & ~0x1F;
            bEnd += nSam << 1;
            if (f->loopCount != -1 && f->loopCount != 0) {
                f->loopCount--;
            }
            nSam = (outCount < f->loopEnd - f->loopStart) ? outCount : f->loopEnd - f->loopStart;
            tsam = nSam - 16 + f->lastsam;
            if (tsam < 0) {
                tsam = 0;
            }
            nframes = (tsam + 15) >> 4;
            nbytes = nframes * 9;
            ptr = func_80092674(ptr, f, tsam, nbytes, op, inp, f->first | 2);
            {
                Acmd *_a = ptr++;

                _a->words.w0 = _SHIFTL(A_DMEMMOVE, 24, 8) | _SHIFTL((f->lastsam << 1) + op, 0, 24);
                _a->words.w1 = _SHIFTL(bEnd, 16, 16) | _SHIFTL(nSam << 1, 0, 16);
            }
        }
        f->lastsam = (f->lastsam + outCount) & 0xF;
        f->sample += outCount;
        f->memin += nframes * 9;
        return ptr;
    }
    nSam = nframes << 4;
    overFlow = f->memin + nbytes - ((s32) f->table->base + f->table->len);
    if (overFlow < 0) {
        overFlow = 0;
    }
    nOver = (overFlow / 9) << 4;
    if (nOver > nSam + nLeft) {
        nOver = nSam + nLeft;
    }
    nbytes -= overFlow;
    if ((nOver - (nOver & 0xF)) < outCount) {
        decoded = 1;
        ptr = func_80092674(ptr, f, nSam - nOver, nbytes, *outp, inp, f->first);
        if (f->lastsam) {
            *outp += f->lastsam << 1;
        } else {
            *outp += 32;
        }
        f->lastsam = (f->lastsam + outCount) & 0xF;
        f->sample += outCount;
        f->memin += nframes * 9;
    } else {
        f->lastsam = 0;
        f->memin += nframes * 9;
    }
    if (nOver) {
        f->lastsam = 0;
        if (decoded) {
            startZero = (nLeft + nSam - nOver) << 1;
        } else {
            startZero = 0;
        }
        {
            Acmd *_a = ptr++;

            _a->words.w0 = _SHIFTL(A_CLEARBUFF, 24, 8) | _SHIFTL(*outp + startZero, 0, 24);
            _a->words.w1 = nOver << 1;
        }
    }
    return ptr;
}

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
