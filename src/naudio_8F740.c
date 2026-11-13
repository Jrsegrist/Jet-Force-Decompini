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

/* JFG: a resampler of an effect delay (only the fields used here) */
typedef struct NResampler {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ void *state[2];
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ f32 delta;
    /* 0x28 */ s32 first;
} NResampler;

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
    /* 0x20 */ NLowPass *lp;
    /* 0x24 */ NResampler *rs;
} NDelay;

/* JFG: an effect (libultra ALFx with two buses) */
typedef struct NFx {
    /* 0x00 */ u32 length;
    /* 0x04 */ NDelay *delay;
    /* 0x08 */ u8 section_count;
    /* 0x09 */ u8 pad9[0x20 - 0x9];
    /* 0x20 */ s16 *base[2];
    /* 0x28 */ s16 *input[2];
} NFx;

/* JFG: an n_audio auxiliary bus (0x48 bytes; only the field used here) */
typedef struct NAuxBus {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ NFx *fxRef;
    /* 0x24 */ u8 pad24[0x48 - 0x24];
} NAuxBus;

typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ NAuxBus *auxBus;
    /* 0x38 */ u8 pad38[0x40 - 0x38];
    /* 0x40 */ s32 outputRate;
    /* 0x44 */ u8 pad44[4];
    /* 0x48 */ s32 sv_dramout;
} NSynth;

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
extern u8 D_80105014[2];   // JFG: per reverb: routing flag A (see naudio_8BEA0.c)
extern u8 D_80105016[2];   // JFG: per reverb: routing flag B
void func_8008DB64(NLowPass *lp); // n_audio: initialise a delay's low-pass filter
Acmd *func_80091970(s32 sampleOffset, Acmd *p); // n_audio: the main bus's pull
Acmd *func_80091480(s32 sampleOffset, Acmd *p, s32 bus, s32 *flag); // n_audio: an auxiliary bus's pull
Acmd *func_8008F8A0(NFx *r, NDelay *d, s32 bus, s32 buff, Acmd *p);
Acmd *func_8008FBB8(NFx *r, s32 bus, s16 *curr_ptr, s32 buff, s32 count, Acmd *p);
Acmd *func_8008FDB0(NFx *r, s32 bus, s16 *curr_ptr, s32 buff, Acmd *p);
Acmd *func_8008FF8C(NLowPass *lp, s32 bus, s32 buff, Acmd *p);
f32 func_8009008C(NDelay *d, s32 count);

/* JFG: pull an auxiliary bus's effect: its sources, then each delay of each of its lines (libultra alFxPull, n_
   two-line form). */
Acmd *func_8008EB40(s32 sampleOffset, Acmd *p, s32 bus) {
    Acmd *ptr = p;
    NFx *r = D_800A97C4->auxBus[bus].fxRef;
    s16 i, buff1, buff2, input, output;
    s16 *in_ptr, *out_ptr, gain, *prev_out_ptr = 0;
    NDelay *d, *pd;
    s32 flag = 1;
    u32 j;

    ptr = func_80091480(sampleOffset, p, bus, &flag);
    input = 0x7C0;
    output = 0x930;
    buff1 = 0;
    buff2 = 0x170;
    if (!D_80105014[bus]) {
        aMix(ptr++, 0, 0xC000, 0x7C0, input);
        aMix(ptr++, 0, 0x4000, 0x930, input);
    }
    ptr = func_8008FDB0(r, 0, r->input[0], input, ptr);
    if (D_80105014[bus]) {
        ptr = func_8008FDB0(r, 1, r->input[1], 0x930, ptr);
    }
    for (j = 0; j <= D_80105014[bus]; j++) {
        aClearBuffer(ptr++, output, 184 << 1);
        for (i = 0; i < r->section_count; i++) {
            d = &r->delay[i];
            in_ptr = &r->input[j][-d->input];
            out_ptr = &r->input[j][-d->output];
            if (D_80105016[bus] && D_80105014[bus]) {
                d->ffcoef = -d->ffcoef;
                d->fbcoef = -d->fbcoef;
            }
            if (in_ptr == prev_out_ptr) {
                s16 t = buff2;
                buff2 = buff1;
                buff1 = t;
            } else {
                ptr = func_8008FBB8(r, j, in_ptr, buff1, 184, ptr);
            }
            ptr = func_8008F8A0(r, d, j, buff2, ptr);
            if (d->ffcoef) {
                aMix(ptr++, 0, (u16) d->ffcoef, buff1, buff2);
                if (!d->rs && !d->lp) {
                    ptr = func_8008FDB0(r, j, out_ptr, buff2, ptr);
                }
            }
            if (d->fbcoef) {
                aMix(ptr++, 0, (u16) d->fbcoef, buff2, buff1);
                ptr = func_8008FDB0(r, j, in_ptr, buff1, ptr);
            }
            if (d->lp) {
                ptr = func_8008FF8C(d->lp, j, buff2, ptr);
            }
            if (!d->rs) {
                ptr = func_8008FDB0(r, j, out_ptr, buff2, ptr);
            }
            if (d->gain) {
                if (D_80105014[bus]) {
                    aMix(ptr++, 0, (u16) d->gain, buff2, output);
                } else {
                    u32 g = d->gain * 1.4142f; // JFG: about sqrt(2), the gain scale of an unrouted effect

                    if (g >= 0x8000) {
                        g = 0x7FFF;
                    }
                    aMix(ptr++, 0, (u16) g, buff2, output);
                }
            }
            prev_out_ptr = &r->input[j][d->output];
        }
        if (D_80105014[bus] && j == 0) {
            ptr = func_8008FBB8(r, 1, r->input[1], input, 184, ptr);
            if (D_80105016[bus]) {
                aMix(ptr++, 0, 0x5A82, output, 0x650);
            } else {
                aMix(ptr++, 0, 0x5A82, output, 0x4E0);
            }
        } else {
        }
        aDMEMMove(ptr++, output, 0x7C0, 184 << 1);
        r->input[j] += 184;
        if (r->input[j] > &r->base[j][r->length]) {
            r->input[j] -= r->length;
        }
    }
    return ptr;
}

/* JFG: set one parameter of one delay of an effect (libultra alFxParamHdl; n_ form: the delay in paramID >> 3,
   times in milliseconds, the delay's times clamped to the line). 173123.4 = 120000 / ln(2). */
s32 func_8008F380(void *filter, s32 paramID, void *param) {
    NFx *f = (NFx *) filter;
    s32 p = paramID & 7;
    s32 s = paramID >> 3;
    s32 val = *(s32 *) param;
    f32 fval;

    if (s >= f->section_count) {
        return 0;
    }
    switch (p) {
        case 0:
            f->delay[s].input = ((s32) val * D_800A97C4->outputRate / 1000) & 0xFFFFFFF8;
            break;
        case 1:
            f->delay[s].output = ((s32) val * D_800A97C4->outputRate / 1000) & 0xFFFFFFF8;
            break;
        case 2:
            f->delay[s].fbcoef = (s16) val;
            break;
        case 3:
            f->delay[s].ffcoef = (s16) val;
            break;
        case 4:
            f->delay[s].gain = (s16) val;
            break;
        case 5:
            f->delay[s].rsinc = ((((f32) val) / 1000) * 2.0f) / D_800A97C4->outputRate;
            break;
        case 6:
            fval = (f32) val;
            break;
        case 7:
            if (f->delay[s].lp) {
                f->delay[s].lp->fc = (s16) val;
                func_8008DB64(f->delay[s].lp);
            }
            break;
    }
    if (f->delay[s].input >= f->length - 16) {
        f->delay[s].input = f->length - 16;
    }
    if (f->delay[s].input >= f->length - 8) {
        f->delay[s].input = f->length - 8;
    }
    if (f->delay[s].input >= f->delay[s].output) {
        f->delay[s].output = f->delay[s].input + 8;
    }
    if (f->delay[s].rs) {
        if (p != 6) {
            if (f->delay[s].output - f->delay[s].input) {
                fval = (f->delay[s].rsgain / (f32) (f->delay[s].output - f->delay[s].input)) * 173123.4f;
            } else {
                fval = 0;
            }
        }
        f->delay[s].rsgain = (f32) (f->delay[s].output - f->delay[s].input) * (fval / 173123.4f);
    }
    return 0;
}

/* JFG: load a delay's output into a buffer, through its chorus resampler when it has one (libultra
   _loadOutputBuffer). */
Acmd *func_8008F8A0(NFx *r, NDelay *d, s32 bus, s32 buff, Acmd *p) {
    Acmd *ptr = p;
    s32 ratio, count, rbuff = 0x2E0;
    s16 *out_ptr;
    f32 fincount, fratio, delta;
    s32 ramalign = 0, length;
    s32 incount = 184;
    s16 tmp;

    if (d->rs) {
        length = d->output - d->input;
        delta = func_8009008C(d, incount);
        delta /= length;
        delta = (s32) (delta * 32768);
        delta = delta / 32768;
        fratio = 1.0f - delta;
        fincount = d->rs->delta + (fratio * (f32) incount);
        count = (s32) fincount;
        d->rs->delta = fincount - (f32) count;
        out_ptr = &r->input[bus][-(d->output - d->rsdelta)];
        ramalign = ((s32) out_ptr & 0x7) >> 1;
        ptr = func_8008FBB8(r, bus, out_ptr - ramalign, rbuff, count + ramalign, ptr);
        ratio = (s32) (fratio * 32768);
        tmp = buff >> 8;
        {
            Acmd *_a = (Acmd *) ptr++;

            _a->words.w0 = (_SHIFTL(A_RESAMPLE, 24, 8) | _SHIFTL(osVirtualToPhysical(d->rs->state[bus]), 0, 24));
            _a->words.w1 = (_SHIFTL(d->rs->first, 30, 2) | _SHIFTL(ratio, 14, 16) |
                            _SHIFTL(rbuff + (ramalign << 1), 2, 12) | _SHIFTL(tmp, 0, 2));
        }
        d->rs->first = 0;
        d->rsdelta += count - incount;
    } else {
        out_ptr = &r->input[bus][-d->output];
        ptr = func_8008FBB8(r, bus, out_ptr, buff, 184, ptr);
    }
    return ptr;
}

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
