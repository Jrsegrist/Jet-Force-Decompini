#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): the synthesizer's effect and output low-pass references and
 * parameters. The n_al* names of this library are runlink names only, so its functions keep address names;
 * the comments give the libultra function each one corresponds to.
 */

/* JFG: an n_audio auxiliary bus (0x48 bytes) */
typedef struct NAuxBus {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ void *fxRef;
    /* 0x24 */ void *fx;
    /* 0x28 */ u8 pad28[0x1C];
    /* 0x44 */ void *outputLP;
} NAuxBus;

/* JFG: the n_audio main bus: its filter's handler is the effect puller when effects are on */
typedef struct NMainBus {
    /* 0x00 */ void *source;
    /* 0x04 */ void *handler;
} NMainBus;

/* JFG: the n_audio synthesizer (libultra ALSynth) */
typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x30];
    /* 0x30 */ NMainBus *mainBus;
    /* 0x34 */ NAuxBus *auxBus;
    /* 0x38 */ u8 pad38[8];
    /* 0x40 */ s32 outputRate;
} NSynth;

/* JFG: an n_audio output low-pass filter */
typedef struct NLowPass {
    /* 0x00 */ s16 fgain;
    /* 0x02 */ s16 fc;
} NLowPass;

extern NSynth *D_800A97C4; // the n_audio synthesizer
extern f32 D_800AF8B0;     // the cutoff parameter's scale (.rodata)

s32 func_8008EB40(); // n_audio: the effect puller (alFxPull)
void func_8008F380(void *fx, s16 paramID, void *param);
void func_8008DD48(NLowPass *lp, f32 outputRate);

/* libultra alSynGetFXRef (n_audio form, no synthesizer argument) */
void *func_8008E710(s16 bus) {
    NMainBus *mainBus = D_800A97C4->mainBus;

    if (mainBus->handler == func_8008EB40) {
        return D_800A97C4->auxBus[bus].fxRef;
    } else {
        return NULL;
    }
}

/* JFG: the auxiliary bus's output low-pass filter, when effects are on */
void *func_8008E788(s16 bus) {
    NMainBus *mainBus = D_800A97C4->mainBus;

    if (mainBus->handler == func_8008EB40) {
        return D_800A97C4->auxBus[bus].outputLP;
    } else {
        return NULL;
    }
}

/* libultra alSynSetFXParam (n_audio form) */
void func_8008E800(void *fx, s16 paramID, void *param) {
    void *f = fx;

    func_8008F380(f, paramID, param);
}

/* JFG: set an output low-pass filter's cutoff (8) or gain (9), then reinitialise it */
void func_8008E844(NLowPass *lp, s16 paramID, s32 *param) {
    if (paramID == 8) {
        lp->fc = *param * D_800AF8B0;
    } else if (paramID == 9) {
        lp->fgain = *param;
    }
    func_8008DD48(lp, D_800A97C4->outputRate);
}
