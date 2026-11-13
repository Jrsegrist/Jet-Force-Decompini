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

/* JFG: a parameter update block as allocated (libultra ALStartParamAlt's size) */
typedef struct NParamBlock {
    /* 0x00 */ NParam *next;
    /* 0x04 */ u8 pad4[0x24 - 0x4];
} NParamBlock;

/* JFG: the n_audio synthesizer configuration (libultra ALSynConfig; one byte per bus for the effect type) */
typedef struct NSynConfig {
    /* 0x00 */ s32 maxVVoices;
    /* 0x04 */ s32 maxPVoices;
    /* 0x08 */ s32 maxUpdates;
    /* 0x0C */ s32 maxFXbusses;
    /* 0x10 */ void *dmaproc;
    /* 0x14 */ ALHeap *heap;
    /* 0x18 */ s32 outputRate;
    /* 0x1C */ u8 fxType[4];
} NSynConfig;

/* JFG: a physical voice (only what is used here; 0xC0 bytes) */
typedef struct NPVoiceLink {
    /* 0x00 */ ALLink node;
    /* 0x08 */ void *vvoice;
    /* 0x0C */ u8 padC[0xC0 - 0xC];
} NPVoiceLink;

/* JFG: a bus's low-pass filter (only what is used here; 0x38 bytes) */
typedef struct NBusFilter {
    /* 0x00 */ s16 fc;
    /* 0x02 */ s16 fgain;
    /* 0x04 */ u8 pad4[0x2C - 0x4];
    /* 0x2C */ void *fstate[2];
    /* 0x34 */ u8 pad34[4];
} NBusFilter;

/* JFG: an auxiliary bus (0x48 bytes) */
typedef struct NAuxBus {
    /* 0x00 */ u8 pad0[0x14];
    /* 0x14 */ s32 sourceCount;
    /* 0x18 */ s32 maxSources;
    /* 0x1C */ NPVoiceLink **sources;
    /* 0x20 */ void *fx;
    /* 0x24 */ u8 pad24[0x44 - 0x24];
    /* 0x44 */ NBusFilter *filter;
} NAuxBus;

/* JFG: the main bus (0x14 bytes) */
typedef struct NMainBus {
    /* 0x00 */ void *source;
    /* 0x04 */ void *handler;
    /* 0x08 */ u8 pad8[0x14 - 0x8];
} NMainBus;

/* JFG: the n_audio synthesizer (libultra ALSynth) */
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
    /* 0x30 */ NMainBus *mainBus;
    /* 0x34 */ NAuxBus *auxBus;
    /* 0x38 */ s32 numPVoices;
    /* 0x3C */ s32 maxAuxBusses;
    /* 0x40 */ s32 outputRate;
    /* 0x44 */ s32 maxOutSamples;
    /* 0x48 */ s32 sv_dramout;
    /* 0x4C */ s32 sv_first;
} NSynth;

extern NSynth *D_800A97C4; // the n_audio synthesizer
extern s32 D_800A97D0;      // JFG: parameters in use
extern s32 D_800A97D4;      // JFG: the most parameters in use
extern f32 D_800AF870;      // microseconds per second (.rodata)

s32 func_8008B0C0(s32 micros);
s32 func_8008B160(ALPlayer **client);
void func_8008B014(void);
Acmd *func_80090150(s32 sampleOffset, Acmd *p);
Acmd *func_8008EB40(s32 sampleOffset, Acmd *p, s32 bus);    // n_audio: an auxiliary bus's effect pull
void *func_8008EAB0(s32 bus, NSynConfig *c, ALHeap *hp);    // n_audio: allocate a bus's effect
void func_8008E4F8(NPVoiceLink *pv, ALDMANew dmaNew, ALHeap *hp); // n_audio: initialise a physical voice

/* libultra alSynNew (n_audio form): set up the synthesizer, its buses, physical voices and parameter pool */
void func_8008A730(NSynConfig *c) {
    s32 i;
    s32 j;
    UNUSED s32 pad3C;
    NPVoiceLink *pv;
    UNUSED s32 pad34;
    NPVoiceLink *pvoices;
    ALHeap *hp = c->heap;
    UNUSED s32 pad28;
    NParamBlock *params;
    NParam *paramPtr;

    D_800A97C4->head = 0;
    D_800A97C4->numPVoices = c->maxPVoices;
    D_800A97C4->curSamples = 0;
    D_800A97C4->paramSamples = 0;
    D_800A97C4->outputRate = c->outputRate;
    D_800A97C4->maxOutSamples = 184;
    D_800A97C4->dma = (ALDMANew) c->dmaproc;
    D_800A97C4->sv_dramout = 0;
    D_800A97C4->sv_first = 1;
    if (c->maxFXbusses > 2) {
        D_800A97C4->maxAuxBusses = 2;
    } else {
        D_800A97C4->maxAuxBusses = c->maxFXbusses;
    }
    D_800A97C4->auxBus = alHeapDBAlloc(0, 0, hp, D_800A97C4->maxAuxBusses, sizeof(NAuxBus));
    for (i = 0; i < D_800A97C4->maxAuxBusses; i++) {
        D_800A97C4->auxBus[i].sourceCount = 0;
        D_800A97C4->auxBus[i].maxSources = c->maxPVoices;
        D_800A97C4->auxBus[i].sources = alHeapDBAlloc(0, 0, hp, c->maxPVoices, sizeof(NPVoiceLink *));
        if (c->fxType[i]) {
            D_800A97C4->auxBus[i].fx = func_8008EAB0(i, c, hp);
        } else {
            D_800A97C4->auxBus[i].fx = 0;
        }
        D_800A97C4->auxBus[i].filter = alHeapDBAlloc(0, 0, hp, 1, sizeof(NBusFilter));
        D_800A97C4->auxBus[i].filter->fgain = 0;
        D_800A97C4->auxBus[i].filter->fstate[0] = alHeapDBAlloc(0, 0, hp, 1, 8);
        D_800A97C4->auxBus[i].filter->fstate[1] = alHeapDBAlloc(0, 0, hp, 1, 8);
    }
    D_800A97C4->mainBus = alHeapDBAlloc(0, 0, hp, 1, sizeof(NMainBus));
    D_800A97C4->mainBus->handler = func_8008EB40;
    D_800A97C4->pFreeList.next = 0;
    D_800A97C4->pFreeList.prev = 0;
    D_800A97C4->pLameList.next = 0;
    D_800A97C4->pLameList.prev = 0;
    D_800A97C4->pAllocList.next = 0;
    D_800A97C4->pAllocList.prev = 0;
    pvoices = alHeapDBAlloc(0, 0, hp, c->maxPVoices, sizeof(NPVoiceLink));
    for (i = 0; i < c->maxPVoices; i++) {
        pv = &pvoices[i];
        alLink((ALLink *) pv, &D_800A97C4->pFreeList);
        pv->vvoice = 0;
        func_8008E4F8(pv, D_800A97C4->dma, hp);
        for (j = 0; j < D_800A97C4->maxAuxBusses; j++) {
            D_800A97C4->auxBus[j].sources[D_800A97C4->auxBus[j].sourceCount++] = pv;
        }
    }
    params = alHeapDBAlloc(0, 0, hp, c->maxUpdates, sizeof(NParamBlock));
    D_800A97C4->paramList = 0;
    for (i = 0; i < c->maxUpdates; i++) {
        paramPtr = (NParam *) &params[i];
        paramPtr->next = D_800A97C4->paramList;
        D_800A97C4->paramList = paramPtr;
    }
    D_800A97C4->heap = hp;
}

/* libultra alAudioFrame (n_audio form): run the players that are due, then build the command list */
Acmd *func_8008AD10(Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen) {
    ALPlayer *client;
    s16 tmp = 0;
    Acmd *cmdlEnd = cmdList;
    Acmd *cmdPtr;
    s32 nOut;
    s16 *lOutBuf = outBuf;

    if (D_800A97C4->head == 0) {
        *cmdLen = 0;
        return cmdList;
    }
    for (D_800A97C4->paramSamples = func_8008B160(&client); D_800A97C4->paramSamples - D_800A97C4->curSamples < outLen; D_800A97C4->paramSamples = func_8008B160(&client)) {
        D_800A97C4->paramSamples &= ~0xF;
        client->samplesLeft += func_8008B0C0((*client->handler)(client));
    }
    D_800A97C4->paramSamples &= ~0xF;
    while (outLen > 0) {
        nOut = (D_800A97C4->maxOutSamples < outLen) ? D_800A97C4->maxOutSamples : outLen;
        cmdPtr = cmdlEnd;
        D_800A97C4->sv_dramout = (s32) lOutBuf;
        cmdlEnd = func_80090150(D_800A97C4->curSamples, cmdPtr);
        outLen -= nOut;
        lOutBuf += nOut << 1;
        D_800A97C4->curSamples += nOut;
    }
    *cmdLen = (s32) (cmdlEnd - cmdList);
    func_8008B014();
    return cmdlEnd;
}

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
