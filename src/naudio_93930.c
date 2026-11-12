#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): the voice resampler (libultra resample.c, n_audio form: a fixed
 * 184-sample frame, the ADPCM stage pulled directly, and a straight copy when resampling is off), then a table
 * arc-cosine and arc-sine on 16-bit fixed point. Written from the assembly with address names.
 */

/* JFG: the resampler state (only what is used here) */
typedef struct NResampler {
    /* 0x00 */ u8 pad0[0x40];
    /* 0x40 */ void *state;
    /* 0x44 */ f32 ratio;
    /* 0x48 */ s32 noResample; // copy the decoded samples instead
    /* 0x4C */ f32 delta;
    /* 0x50 */ s32 first;
} NResampler;

extern f64 D_800AF990; // .rodata: the largest ratio
extern f32 D_800AF998; // .rodata: the ratio used above it
extern u16 D_800A9920[]; // the arc-cosine table, coarse steps
extern u16 D_800A999C[]; // finer steps near the end
extern u16 D_800A9A1C[]; // finest steps at the end

Acmd *func_80091CE0(void *filter, s16 *outp, s32 count, Acmd *p);   // n_audio: the ADPCM stage's pull
s32 func_8009238C(void *filter, s32 paramID, void *param);           // n_audio: the load stage's parameter handler

Acmd *func_80092D30(NResampler *r, s16 *outp, Acmd *p) {
    Acmd *ptr = p;
    s16 inp = 0x170;
    s32 inCount;
    s32 incr;
    f32 finCount;

    if (r->noResample) {
        ptr = func_80091CE0(r, &inp, 184, p);
        {
            Acmd *moveCmd = ptr++;
            moveCmd->words.w0 = (inp & 0xFFFFFF) | 0x0A000000;
            moveCmd->words.w1 = ((*outp & 0xFFFF) << 16) | 0x170;
        }
    } else {
        if (r->ratio > D_800AF990) {
            r->ratio = D_800AF998;
        }
        r->ratio = (s32) (r->ratio * 32768.0f);
        r->ratio = r->ratio / 32768.0f;
        finCount = r->delta + r->ratio * 184.0f;
        inCount = (s32) finCount;
        r->delta = finCount - (f32) inCount;
        ptr = func_80091CE0(r, &inp, inCount, p);
        incr = (s32) (r->ratio * 32768.0f);
        {
            Acmd *resampleCmd = ptr++;
            resampleCmd->words.w0 = (osVirtualToPhysical(r->state) & 0xFFFFFF) | 0x05000000;
            resampleCmd->words.w1 = ((r->first & 3) << 30) | ((incr & 0xFFFF) << 14) | ((inp & 0xFFF) << 2);
        }
        r->first = 0;
    }
    return ptr;
}

s32 func_80092F74(void *filter, s32 paramID, void *param) {
    void *f = filter;

    switch (paramID) {
        default:
            func_8009238C(filter, paramID, param);
            break;
    }
    return 0;
}

/* JFG: interpolate the arc-cosine table (steps get finer towards 0x7FFF) */
s32 func_80092FD0(s32 x) {
    s32 a;
    s32 b;
    s32 index;
    s32 mask;
    s32 shift;
    u16 *table;

    if (x >= 0x7FE0) {
        mask = 7;
        shift = 3;
        table = D_800A9A1C;
        x -= 0x7FE0;
    } else if (x >= 0x7800) {
        mask = 0x1F;
        shift = 5;
        table = D_800A999C;
        x -= 0x7800;
    } else {
        mask = 0x1FF;
        shift = 9;
        table = D_800A9920;
    }
    index = x >> shift;
    a = table[index];
    b = table[index + 1];
    a = a - (((a - b) * (x & mask)) >> shift);
    return a;
}

/* JFG: arc-cosine of a 16-bit fixed-point value (0..0xFFFF for 0..pi) */
u16 func_800930D0(s16 x) {
    s32 v;

    if (x >= 0) {
        v = x;
    } else {
        v = -x;
    }
    v = func_80092FD0(v);
    if (x < 0) {
        v = 0xFFFF - v;
    }
    return v;
}

/* JFG: arc-sine of a 16-bit fixed-point value */
s16 func_80093148(s16 x) {
    s32 v;

    if (x >= 0) {
        v = x;
    } else {
        v = -x;
    }
    v = func_80092FD0(v);
    if (x >= 0) {
        v = 0x7FFF - v;
    } else {
        v = v - 0x8000;
    }
    return v;
}
