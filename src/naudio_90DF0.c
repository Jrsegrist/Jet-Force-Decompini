/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): the envelope mixer (cf.
 * libultra's env.c).
 */
#include "common.h"

/* JFG: a parameter update (libultra ALParam) */
typedef struct NParam {
    /* 0x00 */ struct NParam *next;
} NParam;

/* JFG: an n_audio physical voice: its envelope mixer state (n_ layout) */
typedef struct NPVoice {
    /* 0x00 */ u8 pad0[0x48];
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
} NPVoice;

extern s16 D_800A9820[]; // JFG: the equal-power pan table (128 entries)
extern f32 D_800AF934;
s32 func_80092B28(void *filter, s32 paramID, void *param);               // n_audio: the resampler's param handler
Acmd *func_80092830(void *filter, s16 *outp, s32 outCount, Acmd *p);     // n_audio: the resampler's pull
s16 func_80090F3C(f32 vol, f32 tgt, s32 count, u16 *ratel);

INCLUDE_ASM("asm/nonmatchings/naudio_90DF0", func_800901F0);

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
    *ratel = D_800AF934 * b;
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
