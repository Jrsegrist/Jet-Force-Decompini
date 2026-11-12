#include "common.h"

/* n_audio (built -g -mips2, see the Makefile): libultra alSynAllocFX, n_audio form (no synthesizer argument). */

/* JFG: an n_audio auxiliary bus (0x48 bytes) */
typedef struct NAuxBus {
    /* 0x00 */ u8 pad0[0x24];
    /* 0x24 */ void *fx;
    /* 0x28 */ u8 pad28[0x20];
} NAuxBus;

typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x34];
    /* 0x34 */ NAuxBus *auxBus;
} NSynth;

extern NSynth *D_800A97C4; // the n_audio synthesizer

void func_8008DF34(void **fx, void *config, s16 bus, ALHeap *hp); // n_audio: create the effect (alFxNew)

void *func_8008EAB0(s16 bus, void *config, ALHeap *hp) {
    func_8008DF34(&D_800A97C4->auxBus[bus].fx, config, bus, hp);
    return D_800A97C4->auxBus[bus].fx;
}
