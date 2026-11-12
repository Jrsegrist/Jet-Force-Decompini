/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): voice filter parameters, the
 * effect bus's low-pass filters, effect creation and physical voice creation (cf. libultra's reverb.c and
 * synthesizer init).
 */
#include "common.h"

/* JFG: an n_audio physical voice (only the field used by the parameter calls) */
typedef struct NPVoiceRef {
    /* 0x00 */ u8 pad0[0x88];
    /* 0x88 */ s32 offset;
} NPVoiceRef;

typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ NPVoiceRef *pvoice;
} NVoice;

typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s32 paramSamples;
} NSynth;

typedef struct NParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ union {
        f32 f;
        s32 i;
    } data;
} NParam;

/* JFG: a low-pass filter (0x38 bytes; libultra ALLowPass with a second state) */
typedef struct NLowPass {
    /* 0x00 */ s16 fc;
    /* 0x02 */ s16 fgain; // JFG: the resonance (q) for the mono form
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ s16 fccoef[16];
    /* 0x28 */ s32 first;
    /* 0x2C */ void *fstate;
    /* 0x30 */ void *fstate2;
    /* 0x34 */ u8 pad34[4];
} NLowPass;

/* JFG: a resampler of an effect delay (0x38 bytes) */
typedef struct NResampler {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ void *state;
    /* 0x18 */ void *lstate;
    /* 0x1C */ u8 pad1C[0x24 - 0x1C];
    /* 0x24 */ f32 delta;
    /* 0x28 */ s32 first;
    /* 0x2C */ u8 pad2C[0x38 - 0x2C];
} NResampler;

/* JFG: an effect delay line (0x28 bytes; libultra ALDelay) */
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

/* JFG: an effect (0x30 bytes; libultra ALFx) */
typedef struct NFx {
    /* 0x00 */ u32 length;
    /* 0x04 */ NDelay *delay;
    /* 0x08 */ u8 section_count;
    /* 0x09 */ u8 pad9[0x20 - 0x9];
    /* 0x20 */ s16 *base;
    /* 0x24 */ s16 *lineB;
    /* 0x28 */ s16 *input;
    /* 0x2C */ s16 *inputB;
} NFx;

/* JFG: the synthesizer configuration (only the fields used here) */
typedef struct NSynConfig {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ s32 outputRate;
    /* 0x1C */ u8 fxType[4];
    /* 0x20 */ s32 *params[4];
} NSynConfig;

/* JFG: an n_audio physical voice (libultra PVoice, n_ layout) */
typedef struct NPVoice {
    /* 0x00 */ u8 pad0[0xC];
    /* 0x0C */ void *dc_state;
    /* 0x10 */ void *dc_lstate;
    /* 0x14 */ u8 pad14[0x28 - 0x14];
    /* 0x28 */ void *dc_dma;
    /* 0x2C */ void *dc_dmaState;
    /* 0x30 */ u8 pad30[4];
    /* 0x34 */ s32 dc_memin;
    /* 0x38 */ s32 dc_first;
    /* 0x3C */ s32 dc_lastsam;
    /* 0x40 */ void *rs_state;
    /* 0x44 */ f32 rs_ratio;
    /* 0x48 */ s32 rs_upitch;
    /* 0x4C */ f32 rs_delta;
    /* 0x50 */ s32 rs_first;
    /* 0x54 */ void *em_state;
    /* 0x58 */ s16 em_pan;
    /* 0x5A */ s16 em_volume;
    /* 0x5C */ s16 em_cvolL;
    /* 0x5E */ s16 em_cvolR;
    /* 0x60 */ s16 em_dryamt;
    /* 0x62 */ s16 em_wetamt;
    /* 0x64 */ s16 em_lratl;
    /* 0x66 */ s16 em_lratm;
    /* 0x68 */ s16 em_ltgt;
    /* 0x6A */ u8 pad6A[0x6E - 0x6A];
    /* 0x6E */ s16 em_rtgt;
    /* 0x70 */ s32 em_delta;
    /* 0x74 */ s32 em_segEnd;
    /* 0x78 */ s32 em_first;
    /* 0x7C */ void *em_ctrlList;
    /* 0x80 */ void *em_ctrlTail;
    /* 0x84 */ s32 em_motion;
    /* 0x88 */ u8 pad88[4];
    /* 0x8C */ u8 unk8C;
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ s16 unk90;
    /* 0x92 */ s16 unk92;
    /* 0x94 */ u8 pad94[0xB8 - 0x94];
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ void *unkBC;
} NPVoice;

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
extern s32 D_800A97E0[];   // JFG: the default effect parameters
extern f32 D_800AF890;
extern f32 D_800AF894;
extern f32 D_800AF898;
extern f32 D_800AF89C;
extern f32 D_800AF8A0;
extern f32 D_800AF8A4;
extern f32 D_800AF8A8;
NParam *func_8008AF34(void);                                       // n_audio: allocate a parameter update
void func_80090B14(NPVoiceRef *pvoice, s32 paramID, void *param);  // n_audio: the envelope mixer's param handler
f32 func_80091280(f32 y, f32 x);                                   // n_audio: arc tangent of y/x
void func_8008DB64(NLowPass *lp);

/* JFG: set a voice's filter frequency. */
void func_8008D8A0(NVoice *v, f32 freq) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = 0x13;
        update->data.f = freq;
        update->next = 0;
        func_80090B14(v->pvoice, 3, update);
    }
}

/* JFG: set a voice's filter gain. */
void func_8008D950(NVoice *v, u8 gain) {
    NParam *update;

    if (v->pvoice) {
        update = func_8008AF34();
        if (update == NULL) {
            return;
        }
        update->delta = D_800A97C4->paramSamples + v->pvoice->offset;
        update->type = 0x12;
        update->data.i = gain;
        update->next = 0;
        func_80090B14(v->pvoice, 3, update);
    }
}

/* JFG: two-pole low-pass coefficients for a cutoff below the given rate (b0..b2 to b, a1 and a2 to a). */
void func_8008DA00(f32 rate, f32 cutoff, f32 q, f32 *b, f32 *a) {
    f32 t;
    f32 t2;
    f32 t3;
    UNUSED f32 pad[6];

    if (cutoff >= rate - 200.0f) {
        cutoff = rate - 200.0f;
    }
    t = func_80091280(cutoff * D_800AF890, rate);
    t2 = t * t;
    t3 = t * D_800AF894 / q;
    b[0] = t2 / (1.0f + t2 + t3);
    b[1] = b[0] * 2.0f;
    b[2] = b[0];
    a[1] = ((t2 - 1.0f) * 2.0f) / (1.0f + t2 + t3);
    a[2] = ((1.0f + t2) - t3) / (1.0f + t2 + t3);
}

/* JFG: set up a delay's low-pass filter (libultra _init_lpfilter). */
void func_8008DB64(NLowPass *lp) {
    s32 i, temp;
    s16 fc;
    f32 ffc, fcoef;
    f32 scale;

    temp = lp->fc * 16384.0f;
    fc = temp >> 15;
    lp->fgain = 16384.0f - fc;
    lp->first = 0;
    for (i = 0; i < 8; i++) {
        lp->fccoef[i] = 0;
    }
    lp->fccoef[i++] = fc;
    scale = 16384.0f;
    fcoef = ffc = fc / scale;
    for (; i < 16; i++) {
        fcoef *= ffc;
        lp->fccoef[i] = fcoef * scale;
    }
}

/* JFG: 2 to the power semitones/12 (by squaring the constant per bit). */
f32 func_8008DCB4(s32 semitones) {
    f32 y;
    f32 x = 1.0f;

    if (semitones >= 0) {
        y = D_800AF898;
    } else {
        y = D_800AF89C;
        semitones = -semitones;
    }
    while (semitones) {
        if (semitones & 1) {
            x *= y;
        }
        y *= y;
        semitones >>= 1;
    }
    return x;
}

/* JFG: set up a two-pole (resonant) low-pass filter for a voice at the given rate. */
void func_8008DD48(NLowPass *lp, f32 rate) {
    s32 i;
    f32 b[3];
    f32 a[3];

    if (lp->fgain == 0) {
        return;
    }
    if (lp->fgain < 10) {
        lp->fgain = 10;
    }
    func_8008DA00(rate, lp->fc + 10.0f, lp->fgain / 10.0f, b, a);
    for (i = 3; i < 8; i++) {
        lp->fccoef[i] = 0;
    }
    lp->fccoef[0] = b[0] * (D_800AF8A0 - lp->fgain * 128.0f);
    lp->fccoef[1] = b[1] * (D_800AF8A4 - lp->fgain * 128.0f);
    lp->fccoef[2] = 0;
    lp->fccoef[8] = a[1] * -16384.0f;
    lp->fccoef[9] = a[2] * -16384.0f;
    for (i = 10; i < 16; i++) {
        lp->fccoef[i] = 0;
    }
}

/* JFG: create an effect from the bus's parameters (libultra alFxNew). */
void func_8008DF34(NFx **fxp, NSynConfig *c, s16 bus, ALHeap *hp) {
    u16 i;
    u16 j;
    u16 k;
    s32 *param = 0;
    NDelay *d;
    NFx *r;

    r = alHeapAlloc(hp, 1, sizeof(NFx));
    *fxp = r;
    switch (c->fxType[bus]) {
        case 6:
            param = c->params[bus];
            break;
        default:
            param = D_800A97E0;
            break;
    }
    j = 0;
    r->section_count = param[j++];
    r->length = param[j++];
    r->delay = alHeapAlloc(hp, r->section_count, sizeof(NDelay));
    r->base = alHeapAlloc(hp, r->length, sizeof(s16));
    r->input = r->base;
    r->lineB = alHeapAlloc(hp, r->length, sizeof(s16));
    r->inputB = r->lineB;
    for (k = 0; k < r->length; k++) {
        r->lineB[k] = 0;
        r->base[k] = r->lineB[k];
    }
    for (i = 0; i < r->section_count; i++) {
        d = &r->delay[i];
        d->input = param[j++];
        d->output = param[j++];
        d->fbcoef = param[j++];
        d->ffcoef = param[j++];
        d->gain = param[j++];
        if (param[j]) {
            d->rsinc = ((((f32) param[j++]) / 1000.0f) * 2.0f) / c->outputRate;
            d->rsgain = (((f32) param[j++]) / D_800AF8A8) * (f32) (d->output - d->input);
            d->rsval = 1.0f;
            d->rsdelta = 0;
            d->rs = alHeapAlloc(hp, 1, sizeof(NResampler));
            d->rs->state = alHeapAlloc(hp, 1, 0x20);
            d->rs->lstate = alHeapAlloc(hp, 1, 0x20);
            d->rs->delta = 0.0f;
            d->rs->first = 1;
        } else {
            d->rs = 0;
            j++;
            j++;
        }
        if (param[j]) {
            d->lp = alHeapAlloc(hp, 1, sizeof(NLowPass));
            d->lp->fstate = alHeapAlloc(hp, 1, 8);
            d->lp->fstate2 = alHeapAlloc(hp, 1, 8);
            d->lp->fc = param[j++];
            func_8008DB64(d->lp);
        } else {
            d->lp = 0;
            j++;
        }
    }
}

/* JFG: set up a physical voice's filters (libultra's PVoice init in alSynNew). */
void func_8008E4F8(NPVoice *pv, void *(*dmaNew)(void **state), ALHeap *hp) {
    pv->dc_state = alHeapAlloc(hp, 1, 0x20);
    pv->dc_lstate = alHeapAlloc(hp, 1, 0x20);
    pv->dc_dma = dmaNew(&pv->dc_dmaState);
    pv->dc_memin = 0;
    pv->dc_first = 1;
    pv->dc_lastsam = 0;
    pv->rs_state = alHeapAlloc(hp, 1, 0x20);
    pv->rs_delta = 0.0f;
    pv->rs_first = 1;
    pv->rs_ratio = 1.0f;
    pv->rs_upitch = 0;
    pv->em_state = alHeapAlloc(hp, 1, 0x50);
    pv->em_first = 1;
    pv->em_motion = 0;
    pv->em_volume = 1;
    pv->em_ltgt = 1;
    pv->em_rtgt = 1;
    pv->em_cvolL = 1;
    pv->em_cvolR = 1;
    pv->em_dryamt = 0;
    pv->em_wetamt = 0;
    pv->em_lratm = 1;
    pv->em_lratl = 0;
    pv->em_lratm = 1;
    pv->em_lratl = 0;
    pv->em_delta = 0;
    pv->em_segEnd = 0;
    pv->em_pan = 0;
    pv->em_ctrlList = 0;
    pv->em_ctrlTail = 0;
    pv->unk8C = 0;
    pv->unk92 = 0;
    pv->unk90 = 0;
    pv->unkBC = alHeapAlloc(hp, 1, 8);
    pv->unkB8 = 0;
}
