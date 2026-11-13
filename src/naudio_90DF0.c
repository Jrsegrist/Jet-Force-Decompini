/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): the envelope mixer (cf.
 * libultra's env.c).
 */
#include "common.h"

/* JFG: a parameter update (libultra ALParam) */
typedef struct NParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ union {
        f32 f;
        s32 i;
    } data;
    /* 0x10 */ union {
        f32 f;
        s32 i;
    } moredata;
} NParam;

/* JFG: a voice start with all its settings (libultra ALStartParamAlt, n_ form) */
typedef struct NStartParamAlt {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0A */ s16 unity;
    /* 0x0C */ f32 pitch;
    /* 0x10 */ s16 volume;
    /* 0x12 */ u8 pan;
    /* 0x13 */ u8 fxMix; // JFG: bit 7 goes to the dry amount's low bit
    /* 0x14 */ u8 lpRate;
    /* 0x15 */ u8 lpGain;
    /* 0x18 */ f32 lpCutoff;
    /* 0x1C */ s32 samples;
    /* 0x20 */ void *wave;
} NStartParamAlt;

/* JFG: a voice start (libultra ALStartParam) */
typedef struct NStartParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0A */ s16 unity;
    /* 0x0C */ void *wave;
} NStartParam;

struct NPVoice;

/* JFG: a voice free (libultra ALFreeParam) */
typedef struct NFreeParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ struct NPVoice *pvoice;
} NFreeParam;

/* JFG: an n_audio physical voice: its envelope mixer state (n_ layout) */
typedef struct NPVoice {
    /* 0x00 */ u8 pad0[0x44];
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
    /* 0x64 */ u16 em_lratl;
    /* 0x66 */ s16 em_lratm;
    /* 0x68 */ s16 em_ltgt;
    /* 0x6A */ u16 em_rratl;
    /* 0x6C */ s16 em_rratm;
    /* 0x6E */ s16 em_rtgt;
    /* 0x70 */ s32 em_delta;
    /* 0x74 */ s32 em_segEnd;
    /* 0x78 */ s32 em_first;
    /* 0x7C */ NParam *em_ctrlList;
    /* 0x80 */ NParam *em_ctrlTail;
    /* 0x84 */ s32 em_motion;
    /* 0x88 */ s32 offset;
    /* 0x8C */ u8 lp_rate;
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ s16 lp_fc;
    /* 0x92 */ s16 lp_fgain;
    /* 0x94 */ u8 pad94[0xB8 - 0x94];
    /* 0xB8 */ s32 lp_flags;
} NPVoice;

extern s16 D_800A9820[]; // JFG: the equal-power pan table (128 entries)
extern u8 D_80105010;    // JFG: output type 4 (see naudio_8BEA0.c)
extern u8 D_80105011;    // JFG: output type 1
extern u8 D_80105012;    // JFG: output type 3
s32 func_8009238C(void *filter, s32 paramID, void *param); // n_audio: the load stage's parameter handler
void func_8008B080(NPVoice *pvoice);                        // n_audio: free a physical voice
void func_8008AFD4(NParam *param);                          // n_audio: free a parameter update
s32 func_80090B14(void *filter, s32 paramID, void *param);
Acmd *func_80090C58(void *filter, s16 *inp, s16 *outp, s32 outCount, Acmd *p);
s16 func_8009114C(s16 ivol, s32 samples, s16 ratem, u16 ratel);
s32 func_80092B28(void *filter, s32 paramID, void *param);               // n_audio: the resampler's param handler
Acmd *func_80092830(void *filter, s16 *outp, s32 outCount, Acmd *p);     // n_audio: the resampler's pull
s16 func_80090F3C(f32 vol, f32 tgt, s32 count, u16 *ratel);

/* JFG: the envelope mixer's pull: apply each parameter update due in this frame, a sub-frame at a time (libultra
   alEnvmixerPull; n_ form: frame-quantised offsets, the output-type pan and dry/wet rules, the voice filter's
   settings at start). */
Acmd *func_800901F0(void *filter, s32 sampleOffset, Acmd *p) {
    Acmd *ptr = p;
    NPVoice *e = (NPVoice *) filter;
    s16 inp;
    s32 lastOffset;
    s32 thisOffset = sampleOffset;
    s32 samples;
    s16 loutp = 0;
    s32 fVol;
    NParam *thisParam;
    s32 outCount = 184;

    inp = 0;
    while (e->em_ctrlList != 0) {
        lastOffset = thisOffset;
        thisOffset = e->em_ctrlList->delta;
        samples = (thisOffset - lastOffset + 183) / 184 * 184;
        if (samples == 0) {
            thisOffset = lastOffset;
        }
        if (samples > outCount) {
            break;
        }
        switch (e->em_ctrlList->type) {
            case 13: {
                NStartParamAlt *param = (NStartParamAlt *) e->em_ctrlList;
                s32 tmp;

                if (param->unity) {
                    e->rs_upitch = 1;
                }
                func_8009238C(e, 5, param->wave);
                e->em_motion = 1;
                e->em_first = 1;
                e->em_delta = 0;
                e->em_segEnd = (param->samples + 183) / 184 * 184;
                tmp = (param->volume + param->volume) / 2;
                e->em_volume = (s16) tmp;
                e->em_pan = param->pan;
                e->em_dryamt = (D_800A9820[param->fxMix & 0x7F] & 0xFFFE) | (param->fxMix >> 7);
                if (!D_80105010) {
                    e->em_dryamt &= 0xFFFE;
                }
                e->em_wetamt = D_800A9820[128 - (param->fxMix & 0x7F) - 1] & 0xFFFE;
                if (D_80105012) {
                    e->em_pan = (e->em_pan >> 1) + 0x20;
                } else if (D_80105011) {
                    e->em_pan = 0x40;
                }
                if (param->samples) {
                    e->em_cvolL = 1;
                    e->em_cvolR = 1;
                } else {
                    e->em_cvolL = (D_800A9820[e->em_pan] * e->em_volume) >> 15;
                    e->em_cvolR = (D_800A9820[128 - e->em_pan - 1] * e->em_volume) >> 15;
                }
                e->rs_ratio = param->pitch;
                e->lp_fgain = param->lpGain;
                e->lp_fc = param->lpCutoff;
                e->lp_flags = 1;
                e->lp_rate = param->lpRate;
            } break;
            case 11:
            case 12:
            case 16:
                ptr = func_80090C58(e, &inp, &loutp, samples, ptr);
                if (e->em_delta >= e->em_segEnd) {
                    e->em_ltgt = (D_800A9820[e->em_pan] * e->em_volume) >> 15;
                    e->em_rtgt = (D_800A9820[128 - e->em_pan - 1] * e->em_volume) >> 15;
                    e->em_delta = e->em_segEnd;
                    e->em_cvolL = e->em_ltgt;
                    e->em_cvolR = e->em_rtgt;
                } else {
                    e->em_cvolL = func_8009114C(e->em_cvolL, e->em_delta, e->em_lratm, e->em_lratl);
                    e->em_cvolR = func_8009114C(e->em_cvolR, e->em_delta, e->em_rratm, e->em_rratl);
                }
                if (e->em_cvolL == 0) {
                    e->em_cvolL = 1;
                }
                if (e->em_cvolR == 0) {
                    e->em_cvolR = 1;
                }
                if (e->em_ctrlList->type == 12) {
                    if (D_80105012) {
                        e->em_pan = ((s16) e->em_ctrlList->data.i >> 1) + 0x20;
                    } else if (D_80105011) {
                        e->em_pan = 0x40;
                    } else {
                        e->em_pan = e->em_ctrlList->data.i;
                    }
                }
                if (e->em_ctrlList->type == 11) {
                    e->em_delta = 0;
                    fVol = e->em_ctrlList->data.i;
                    fVol = (fVol + fVol) / 2;
                    e->em_volume = (s16) fVol;
                    e->em_segEnd = (e->em_ctrlList->moredata.i + 183) / 184 * 184;
                }
                if (e->em_ctrlList->type == 16) {
                    if (((e->em_dryamt ^ e->em_wetamt) & 1) ^ ((e->em_ctrlList->data.i + 1) >> 7)) {
                        if (D_80105010) {
                            if (e->em_pan > 0x40) {
                                e->em_dryamt ^= 1;
                            } else {
                                e->em_wetamt ^= 1;
                            }
                        }
                    }
                    e->em_dryamt = (D_800A9820[e->em_ctrlList->data.i & 0x7F] & 0xFFFE) | (e->em_dryamt & 1);
                    e->em_wetamt =
                        (D_800A9820[128 - (e->em_ctrlList->data.i & 0x7F) - 1] & 0xFFFE) | (e->em_wetamt & 1);
                }
                e->em_first = 1;
                break;
            case 14: {
                NStartParam *param = (NStartParam *) e->em_ctrlList;

                if (param->unity) {
                    e->rs_upitch = 1;
                }
                func_8009238C(e, 5, param->wave);
                e->em_motion = 1;
            } break;
            case 15:
                ptr = func_80090C58(e, &inp, &loutp, samples, ptr);
                func_80090B14(e, 4, 0);
                break;
            case 0: {
                NFreeParam *param = (NFreeParam *) e->em_ctrlList;

                param->pvoice->offset = 0;
                func_8008B080(param->pvoice);
            } break;
            case 7:
                ptr = func_80090C58(e, &inp, &loutp, samples, ptr);
                e->rs_ratio = e->em_ctrlList->data.f;
                break;
            case 8:
                ptr = func_80090C58(e, &inp, &loutp, samples, ptr);
                e->rs_upitch = 1;
                break;
            case 5:
                ptr = func_80090C58(e, &inp, &loutp, samples, ptr);
                func_8009238C(e, 5, (void *) e->em_ctrlList->data.i);
                break;
            default:
                ptr = func_80090C58(e, &inp, &loutp, samples, ptr);
                func_80090B14(e, e->em_ctrlList->type, (void *) e->em_ctrlList->data.i);
                break;
        }
        loutp += samples << 1;
        outCount -= samples;
        thisParam = e->em_ctrlList;
        e->em_ctrlList = e->em_ctrlList->next;
        if (e->em_ctrlList == 0) {
            e->em_ctrlTail = 0;
        }
        func_8008AFD4(thisParam);
    }
    ptr = func_80090C58(e, &inp, &loutp, outCount, ptr);
    if (e->em_delta > e->em_segEnd) {
        e->em_delta = e->em_segEnd;
    }
    return ptr;
}

/* JFG: the envelope mixer's parameter handler (libultra alEnvmixerParam). */
s32 func_80090B14(void *filter, s32 paramID, void *param) {
    NPVoice *e = filter;

    switch (paramID) {
        case 3:
            if (e->em_ctrlTail) {
                e->em_ctrlTail->next = (NParam *) param;
            } else {
                e->em_ctrlList = (NParam *) param;
            }
            e->em_ctrlTail = (NParam *) param;
            break;
        case 4:
            e->em_first = 1;
            e->em_motion = 0;
            e->em_volume = 1;
            e->em_segEnd = 0;
            e->rs_delta = 0.0f;
            e->rs_first = 1;
            e->rs_upitch = 0;
            func_80092B28(e, 4, param);
            break;
        case 9:
            e->em_motion = 1;
            break;
        default:
            func_80092B28(e, paramID, param);
            break;
    }
    return 0;
}

/* JFG: one sub-frame of the envelope mixer: resample, then the volume commands (on the first frame) and the mix
   (libultra _pullSubFrame). */
Acmd *func_80090C58(void *filter, s16 *inp, UNUSED s16 *outp, s32 outCount, Acmd *p) {
    Acmd *ptr = p;
    NPVoice *e = filter;

    if (e->em_motion != 1 || !outCount) {
        return ptr;
    }
    ptr = func_80092830(e, inp, outCount, p);
    if (e->em_first) {
        e->em_first = 0;
        e->em_ltgt = (D_800A9820[e->em_pan] * e->em_volume) >> 15;
        e->em_lratm = func_80090F3C((f32) e->em_cvolL, (f32) e->em_ltgt, e->em_segEnd, &e->em_lratl);
        e->em_rtgt = (D_800A9820[128 - e->em_pan - 1] * e->em_volume) >> 15;
        e->em_rratm = func_80090F3C((f32) e->em_cvolR, (f32) e->em_rtgt, e->em_segEnd, &e->em_rratl);
        aSetVolume(ptr++, 6, e->em_cvolL, e->em_dryamt, e->em_wetamt);
        aSetVolume(ptr++, 4, e->em_rtgt, e->em_rratm, e->em_rratl);
        aSetVolume(ptr++, 0, e->em_ltgt, e->em_lratm, e->em_lratl);
        {
            Acmd *_a = (Acmd *) ptr++;

            _a->words.w0 = (_SHIFTL(A_ENVMIXER, 24, 8) | _SHIFTL(1, 16, 8) | _SHIFTL(e->em_cvolR, 0, 16));
            _a->words.w1 = (unsigned int) osVirtualToPhysical(e->em_state);
        }
    } else {
        Acmd *_a = (Acmd *) ptr++;

        _a->words.w0 = (_SHIFTL(A_ENVMIXER, 24, 8) | _SHIFTL(0, 16, 8) | _SHIFTL(0, 0, 16));
        _a->words.w1 = (unsigned int) osVirtualToPhysical(e->em_state);
    }
    *inp += 368;
    e->em_delta += 184;
    return ptr;
}

/* JFG: the volume ramp rate (integer and fraction) from vol to tgt over count samples (libultra _getRate). */
s16 func_80090F3C(f32 vol, f32 tgt, s32 count, u16 *ratel) {
    s16 s;
    s16 tmp;
    f32 invn;
    f32 a;
    f32 b;

    if (count == 0) {
        if (tgt >= vol) {
            *ratel = 0xFFFF;
            return 0x7FFF;
        } else {
            *ratel = 0;
            return -0x8000;
        }
    }
    invn = 1.0f / count;
    if (tgt < 1.0f) {
        tgt = 1.0f;
    }
    if (vol <= 0.0f) {
        vol = 1.0f;
    }
    a = (tgt - vol) * invn * 8.0f;
    s = a;
    b = a - s;
    s--;
    b += 1.0f;
    tmp = b;
    s += tmp;
    b -= tmp;
    *ratel = 65535.0f * b;
    return s;
}

/* JFG: the volume after some samples at a ramp rate (libultra _getVol). */
s16 func_8009114C(s16 ivol, s32 samples, s16 ratem, u16 ratel) {
    s32 tmpl;

    samples = samples >> 3;
    if (samples == 0) {
        return ivol;
    }
    tmpl = ratel * samples;
    tmpl >>= 16;
    tmpl += ratem * samples;
    ivol += tmpl;
    return ivol;
}
