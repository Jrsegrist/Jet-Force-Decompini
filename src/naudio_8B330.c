#include "common.h"

/*
 * n_audio (built -g -mips2, see the Makefile): the synthesizer (libultra synthesizer.c, n_audio form: the
 * synthesizer is the global D_800A97C4 rather than an argument). The parameter pool also counts its use
 * (D_800A97D0, with the high-water mark in D_800A97D4). n_al* names are runlink names only: address names here.
 */

/* JFG: a parameter update (libultra ALParam, from synthInternals.h) */
typedef struct NParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ s32 data;
} NParam;

/* JFG: the n_audio synthesizer (libultra ALSynth; JFG's has no maxAuxBusses) */
typedef struct NSynth {
    /* 0x00 */ ALPlayer *head;
    /* 0x04 */ ALLink pFreeList;
    /* 0x0C */ ALLink pAllocList;
    /* 0x14 */ ALLink pLameList;
    /* 0x1C */ s32 paramSamples;
    /* 0x20 */ s32 curSamples;
    /* 0x24 */ ALDMANew dma;
    /* 0x28 */ ALHeap *heap;
    /* 0x2C */ NParam *paramList;
    /* 0x30 */ void *mainBus;
    /* 0x34 */ void *auxBus;
    /* 0x38 */ void *outputFilter;
    /* 0x3C */ s32 numPVoices;
    /* 0x40 */ s32 outputRate;
} NSynth;

extern NSynth *D_800A97C4; // the n_audio synthesizer
extern s32 D_800A97D0;      // JFG: parameters in use
extern s32 D_800A97D4;      // JFG: the most parameters in use
extern f32 D_800AF870;      // microseconds per second (.rodata)

s32 func_8008B0C0(s32 micros);

INCLUDE_ASM("asm/nonmatchings/naudio_8B330", func_8008A730);

INCLUDE_ASM("asm/nonmatchings/naudio_8B330", func_8008AD10);

/* libultra __allocParam: take a parameter update from the pool */
NParam *func_8008AF34(void) {
    NParam *update = 0;

    if (D_800A97C4->paramList) {
        update = D_800A97C4->paramList;
        D_800A97C4->paramList = (D_800A97C4->paramList)->next;
        update->next = 0;
        D_800A97D0++;
        if (D_800A97D0 > D_800A97D4) {
            D_800A97D4 = D_800A97D0;
        }
    }
    return update;
}

/* libultra _freeParam: give a parameter update back to the pool */
void func_8008AFD4(NParam *param) {
    param->next = D_800A97C4->paramList;
    D_800A97C4->paramList = param;
    D_800A97D0--;
}

/* libultra _collectPVoices: move the voices waiting to be freed to the free list */
void func_8008B014(void) {
    ALLink *link;
    struct PVoice_s *pvoice;

    while ((link = D_800A97C4->pLameList.next) != 0) {
        pvoice = (struct PVoice_s *) link;
        alUnlink(link);
        alLink(link, &D_800A97C4->pFreeList);
    }
}

/* libultra _freePVoice: queue a physical voice to be freed */
void func_8008B080(struct PVoice_s *pvoice) {
    alUnlink((ALLink *) pvoice);
    alLink((ALLink *) pvoice, &D_800A97C4->pLameList);
}

/* libultra _timeToSamplesNoRound */
s32 func_8008B0C0(s32 micros) {
    f32 samples = (f32) micros * D_800A97C4->outputRate / D_800AF870 + 0.5f;

    return (s32) samples;
}

/* libultra _timeToSamples: whole 16-sample frames */
s32 func_8008B124(s32 micros) {
    return func_8008B0C0(micros) & ~0xF;
}

/* libultra __nextSampleTime: the player due first */
s32 func_8008B160(ALPlayer **client) {
    UNUSED s32 pad;
    ALMicroTime delta = 0x7FFFFFFF;
    ALPlayer *player;

    *client = 0;
    for (player = D_800A97C4->head; player != 0; player = player->next) {
        if ((player->samplesLeft - D_800A97C4->curSamples) < delta) {
            *client = player;
            delta = player->samplesLeft - D_800A97C4->curSamples;
        }
    }
    return (*client)->samplesLeft;
}
