#include "common.h"

/* n_audio (built -g -mips2, see the Makefile): libultra alSynDelete, n_audio form (no synthesizer argument). */

typedef struct NSynth {
    /* 0x00 */ void *head;
} NSynth;

extern NSynth *D_800A97C4; // the n_audio synthesizer

void func_8008EA90(void) {
    D_800A97C4->head = NULL;
}
