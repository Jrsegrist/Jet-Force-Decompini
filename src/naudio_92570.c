#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): the main bus pull (libultra mainbus.c, n_audio form): clear the
 * output, then for each auxiliary bus pull it, mix it into the outputs by the surround routing (D_80105014 /
 * D_80105016, see naudio_8BEA0.c) and run its output low-pass filter. Written from the assembly with address names.
 */

/* JFG: an auxiliary bus's output low-pass filter */
typedef struct NOutputLP {
    /* 0x00 */ s16 fgain;
    /* 0x02 */ s16 fc; // > 0: on
    /* 0x04 */ u8 pad4[4];
    /* 0x08 */ u8 fccoef[0x20];
    /* 0x28 */ s32 first;
    /* 0x2C */ void *stateL;
    /* 0x30 */ void *stateR;
} NOutputLP;

/* JFG: an n_audio auxiliary bus (0x48 bytes) */
typedef struct NAuxBus {
    /* 0x00 */ u8 pad0[0x44];
    /* 0x44 */ NOutputLP *outputLP;
} NAuxBus;

typedef struct NFilter {
    /* 0x00 */ void *source;
    /* 0x04 */ Acmd *(*handler)(s32 sampleOffset, Acmd *p, s32 bus);
} NFilter;

typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ NFilter *auxPull;
    /* 0x34 */ NAuxBus *auxBus;
    /* 0x38 */ u8 pad38[4];
    /* 0x3C */ s32 maxAuxBusses;
    /* 0x40 */ s32 outputRate;
} NSynth;

extern NSynth *D_800A97C4;  // the n_audio synthesizer
extern u8 D_80105014[2];    // JFG: per reverb: routing flag A
extern u8 D_80105016[2];    // JFG: per reverb: routing flag B

void func_8008DD48(NOutputLP *lp, f32 outputRate); // n_audio: initialise a mono low-pass filter

Acmd *func_80091970(s32 sampleOffset, Acmd *p) {
    Acmd *ptr = p;
    s32 i;
    UNUSED s32 pad;

    {
        Acmd *c = ptr++;
        c->words.w0 = 0x020004E0;
        c->words.w1 = 0x2E0;
    }
    for (i = 0; i < D_800A97C4->maxAuxBusses; i++) {
        ptr = D_800A97C4->auxPull->handler(sampleOffset, ptr, i);
        if (D_80105014[i]) {
            if (D_80105016[i]) {
                Acmd *c = ptr++;
                c->words.w0 = 0x0C008000;
                c->words.w1 = 0x07C004E0;
            } else {
                Acmd *c = ptr++;
                c->words.w0 = 0x0C007FFF;
                c->words.w1 = 0x07C00650;
            }
        } else {
            if (D_80105016[i]) {
                Acmd *c = ptr++;
                c->words.w0 = 0x0C008000;
                c->words.w1 = 0x07C00650;
            } else {
                Acmd *c = ptr++;
                c->words.w0 = 0x0C007FFF;
                c->words.w1 = 0x07C00650;
            }
            {
                Acmd *c = ptr++;
                c->words.w0 = 0x0C007FFF;
                c->words.w1 = 0x07C004E0;
            }
        }
        if (D_800A97C4->auxBus[i].outputLP->fc > 0) {
            NOutputLP *lp = D_800A97C4->auxBus[i].outputLP;

            if (lp->first) {
                func_8008DD48(lp, D_800A97C4->outputRate);
            }
            {
                Acmd *c = ptr++;
                c->words.w0 = 0x0B000020;
                c->words.w1 = osVirtualToPhysical(lp->fccoef);
            }
            {
                Acmd *c = ptr++;
                c->words.w0 = 0x0E0004E0;
                c->words.w1 = osVirtualToPhysical(lp->stateL) & 0xFFFFFF & 0xFFFFFF;
            }
            {
                Acmd *c = ptr++;
                c->words.w0 = 0x0E000650;
                c->words.w1 = osVirtualToPhysical(lp->stateR) & 0xFFFFFF & 0xFFFFFF;
            }
            lp->first = 0;
        }
    }
    return ptr;
}
