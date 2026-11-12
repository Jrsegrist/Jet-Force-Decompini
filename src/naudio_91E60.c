#include "common.h"

/* n_audio (built -g -mips2, see the Makefile): libultra alSynSetPriority, n_audio form. */

/* JFG: an n_audio voice (libultra ALVoice) */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x16];
    /* 0x16 */ s16 priority;
} NVoice;

void func_80091260(NVoice *voice, s16 priority) {
    voice->priority = priority;
}
