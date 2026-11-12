#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): JFG's low-pass filter stage of a voice (after the resampler): an
 * optional resample-ratio command from its rate setting, then the pole filter with its state. Written from the
 * assembly with address names.
 */

/* JFG: the filter state (only what is used here) */
typedef struct NLPFilter {
    /* 0x00 */ u8 pad0[0x8C];
    /* 0x8C */ u8 rate;      // 1..0x3F: a resample command is added
    /* 0x8D */ u8 pad8D[3];
    /* 0x90 */ s16 fc;       // the cutoff
    /* 0x92 */ s16 fgain;    // > 0: filter on
    /* 0x94 */ u8 pad94[4];
    /* 0x98 */ u8 table[0x20]; // the coefficient table the RSP loads
    /* 0xB8 */ s32 flags;    // the pole filter flags (2: reinitialise)
    /* 0xBC */ void *state;
} NLPFilter;

extern f32 D_800AF970; // .rodata
extern f32 D_800AF974; // .rodata: the smallest ratio
extern f32 D_800AF978; // .rodata: the ratio used below it

Acmd *func_80092D30(void *filter, s16 *outp, Acmd *p); // n_audio: the resampler's pull
void func_8008DD48(void *lp, f32 outputRate);         // n_audio: initialise a mono low-pass filter
s32 func_8009238C(void *filter, s32 paramID, void *param); // n_audio: the load stage's parameter handler
f32 sqrtf(f32 x);

Acmd *func_80092830(NLPFilter *filter, s16 *outp, s32 outCount, Acmd *p) {
    Acmd *ptr = p;
    UNUSED s32 pad;
    f32 ratio;
    Acmd *cmd;
    Acmd *loadCmd;
    Acmd *poleCmd;

    ptr = func_80092D30(filter, outp, p);
    if (filter->rate != 0 && filter->rate < 0x40) {
        if (filter->rate >= 6) {
            ratio = D_800AF970 / sqrtf((f32) filter->rate + 1.0f);
        } else {
            ratio = 65536.0f / ((f32) filter->rate + 1.0f);
        }
        if (ratio < D_800AF974) {
            ratio = D_800AF978;
        }
        cmd = ptr++;
        cmd->words.w0 = *outp & 0xFFFF;
        cmd->words.w1 = (((u32) ratio & 0xFFFF) << 16) | ((filter->rate + 1) & 0xFFFF);
    }
    if (filter->fgain > 0) {
        if (filter->flags) {
            func_8008DD48(&filter->fc, 22050.0f);
        }
        loadCmd = ptr++;
        loadCmd->words.w0 = 0x0B000020;
        loadCmd->words.w1 = osVirtualToPhysical(filter->table);
        if (filter->flags == 2) {
            filter->flags = 0;
        }
        poleCmd = ptr++;
        poleCmd->words.w0 = (0x0E000000 | ((filter->flags & 0xFF) << 16)) | (*outp & 0xFFFF);
        poleCmd->words.w1 = osVirtualToPhysical(filter->state) & 0xFFFFFF & 0xFFFFFF;
        filter->flags = 0;
    }
    return ptr;
}

s32 func_80092B28(NLPFilter *filter, s32 paramID, void *param) {
    f32 *fparam = (f32 *) &param;

    switch (paramID) {
        case 4:
            filter->fgain = 0;
            func_8009238C(filter, 4, param);
            break;
        case 0x12:
            filter->fgain = (s32) param;
            filter->flags |= 2;
            break;
        case 0x13:
            filter->fc = *fparam;
            filter->flags |= 2;
            break;
        case 0x11:
            filter->rate = (s32) param;
            break;
        default:
            func_8009238C(filter, paramID, param);
            break;
    }
    return 0;
}
