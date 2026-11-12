/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): the effects bus (cf. libultra's
 * reverb.c and save.c).
 */
#include "common.h"

/* JFG: the n_audio command forms that differ from abi.h (12-bit count and address, an extra field) */
#define N_ALOADBUFF(pkt, c, d, s)                                                                          \
    {                                                                                                      \
        Acmd *_a = (Acmd *) pkt;                                                                           \
        _a->words.w0 = (_SHIFTL(A_LOADBUFF, 24, 8) | _SHIFTL(c, 12, 12) | _SHIFTL(d, 0, 12));              \
        _a->words.w1 = (unsigned int) (s);                                                                 \
    }
#define N_ASAVEBUFF(pkt, c, d, s)                                                                          \
    {                                                                                                      \
        Acmd *_a = (Acmd *) pkt;                                                                           \
        _a->words.w0 = (_SHIFTL(A_SAVEBUFF, 24, 8) | _SHIFTL(c, 12, 12) | _SHIFTL(d, 0, 12));              \
        _a->words.w1 = (unsigned int) (s);                                                                 \
    }

/* JFG: a low-pass filter (only the fields used here) */
typedef struct NLowPass {
    /* 0x00 */ s16 fc;
    /* 0x02 */ s16 fgain;
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s16 fccoef[16];
    /* 0x28 */ s32 first;
    /* 0x2C */ void *fstate[2];
} NLowPass;

/* JFG: an effect delay line (libultra ALDelay) */
typedef struct NDelay {
    /* 0x00 */ u32 input;
    /* 0x04 */ u32 output;
    /* 0x08 */ s16 ffcoef;
    /* 0x0A */ s16 fbcoef;
    /* 0x0C */ s16 gain;
    /* 0x10 */ f32 rsinc;
    /* 0x14 */ f32 rsval;
    /* 0x18 */ s32 rsdelta;
    /* 0x1C */ f32 rsgain;
} NDelay;

/* JFG: an effect (libultra ALFx with two buses) */
typedef struct NFx {
    /* 0x00 */ u32 length;
    /* 0x04 */ NDelay *delay;
    /* 0x08 */ u8 pad8[0x20 - 0x8];
    /* 0x20 */ s16 *base[2];
} NFx;

typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ s32 sv_dramout;
} NSynth;

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
Acmd *func_80091970(s32 sampleOffset, Acmd *p); // n_audio: the main bus's pull

INCLUDE_ASM("asm/nonmatchings/naudio_8F740", func_8008EB40);

INCLUDE_ASM("asm/nonmatchings/naudio_8F740", func_8008F380);

INCLUDE_ASM("asm/nonmatchings/naudio_8F740", func_8008F8A0);

/* JFG: load count samples of a bus's delay line into a buffer, wrapping at its end (libultra _loadBuffer). */
Acmd *func_8008FBB8(NFx *r, s32 bus, s16 *curr_ptr, s32 buff, s32 count, Acmd *p) {
    Acmd *ptr = p;
    s32 after_end, before_end;
    s16 *updated_ptr, *delay_end;

    delay_end = &r->base[bus][r->length];
    if (curr_ptr < r->base[bus]) {
        curr_ptr += r->length;
    }
    updated_ptr = curr_ptr + count;
    if (updated_ptr > delay_end) {
        after_end = updated_ptr - delay_end;
        before_end = delay_end - curr_ptr;
        N_ALOADBUFF(ptr++, before_end << 1, buff, osVirtualToPhysical(curr_ptr));
        N_ALOADBUFF(ptr++, after_end << 1, buff + (before_end << 1), osVirtualToPhysical(r->base[bus]));
    } else {
        N_ALOADBUFF(ptr++, count << 1, buff, osVirtualToPhysical(curr_ptr));
    }
    return ptr;
}

/* JFG: save a sub-frame from a buffer into a bus's delay line, wrapping at its end (libultra _saveBuffer). */
Acmd *func_8008FDB0(NFx *r, s32 bus, s16 *curr_ptr, s32 buff, Acmd *p) {
    Acmd *ptr = p;
    s32 after_end, before_end;
    s16 *updated_ptr, *delay_end;

    delay_end = &r->base[bus][r->length];
    if (curr_ptr < r->base[bus]) {
        curr_ptr += r->length;
    }
    updated_ptr = curr_ptr + 184;
    if (updated_ptr > delay_end) {
        after_end = updated_ptr - delay_end;
        before_end = delay_end - curr_ptr;
        N_ASAVEBUFF(ptr++, before_end << 1, buff, osVirtualToPhysical(curr_ptr));
        N_ASAVEBUFF(ptr++, after_end << 1, buff + (before_end << 1), osVirtualToPhysical(r->base[bus]));
    } else {
        N_ASAVEBUFF(ptr++, 184 << 1, buff, osVirtualToPhysical(curr_ptr));
    }
    return ptr;
}

/* JFG: run a delay's low-pass filter over a buffer (libultra _filterBuffer). */
Acmd *func_8008FF8C(NLowPass *lp, s32 bus, s32 buff, Acmd *p) {
    Acmd *ptr = p;
    s16 tmp = buff >> 8;

    {
        Acmd *_a = (Acmd *) ptr++;

        _a->words.w0 = (_SHIFTL(A_LOADADPCM, 24, 8) | _SHIFTL(32, 0, 24));
        _a->words.w1 = (unsigned int) osVirtualToPhysical(lp->fccoef);
    }
    {
        Acmd *_a = (Acmd *) ptr++;

        _a->words.w0 = (_SHIFTL(A_POLEF, 24, 8) | _SHIFTL(lp->first, 16, 8) | _SHIFTL(lp->fgain, 0, 16));
        _a->words.w1 = (_SHIFTL(osVirtualToPhysical(lp->fstate[bus]), 0, 24) | _SHIFTL(tmp, 24, 8));
    }
    lp->first = 0;
    return ptr;
}

/* JFG: a delay's modulation: a triangle wave from its sawtooth (libultra _doModFunc). */
f32 func_8009008C(NDelay *d, s32 count) {
    f32 val;

    d->rsval += d->rsinc * count;
    d->rsval = (d->rsval > 2.0f) ? d->rsval - 4.0f : d->rsval;
    val = d->rsval;
    val = (val < 0) ? -val : val;
    val -= 1.0f;
    return d->rsgain * val;
}

/* JFG: pull the main bus, interleave and save the output (libultra alSavePull). */
Acmd *func_80090150(s32 sampleOffset, Acmd *p) {
    Acmd *ptr = p;

    ptr = func_80091970(sampleOffset, ptr);
    {
        Acmd *_a = (Acmd *) ptr++;

        _a->words.w0 = _SHIFTL(A_INTERLEAVE, 24, 8);
    }
    N_ASAVEBUFF(ptr++, 184 << 2, 0, D_800A97C4->sv_dramout);
    return ptr;
}
