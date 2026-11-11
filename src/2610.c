#include "common.h"

/* The audio manager thread (part of DKR's AMAudioMgr __am). */
extern OSThread sAudioMgrThread;

/* DKR AMDMABuffer */
typedef struct AMDMABuffer {
    /* 0x00 */ ALLink node;
    /* 0x08 */ u32 startAddr;
    /* 0x0C */ u32 lastFrame;
    /* 0x10 */ u8 *ptr;
} AMDMABuffer;
/* DKR AMDMAState */
typedef struct AudioMgrHandle {
    /* 0x00 */ u8 initialised;
    /* 0x04 */ AMDMABuffer *firstUsed;
    /* 0x08 */ AMDMABuffer *firstFree;
} AudioMgrHandle;
#define NUM_DMA_BUFFERS 76
extern AMDMABuffer dmaBuffs[NUM_DMA_BUFFERS];
extern OSMesgQueue audDMAMessageQ;

/* DKR AudioMsg, reduced */
typedef struct AudioMsgGen {
    s16 type;
} AudioMsgGen;
extern void *gAudioSched;
extern u8 audioStack[];         // DKR: audioStack (the scheduler client)
extern OSMesgQueue D_800F1AF4;  // DKR: __am.audioFrameMsgQ
extern OSMesgQueue D_800F1B0C;  // DKR: __am.audioReplyMsgQ
extern u8 D_800F19F0[];         // DKR: __am.g

/* JFG's synthesizer configuration (n_audio, FX types as bytes) */
typedef struct AMSynConfig {
    /* 0x00 */ s32 maxVVoices;
    /* 0x04 */ s32 maxPVoices;
    /* 0x08 */ s32 maxUpdates;
    /* 0x0C */ s32 maxFXbusses;
    /* 0x10 */ void *dmaproc;
    /* 0x14 */ ALHeap *heap;
    /* 0x18 */ s32 outputRate;
    /* 0x1C */ u8 fxType[4];
    /* 0x20 */ s32 *params[4];
} AMSynConfig;

/* JFG's custom reverb parameters (copied to the stack for each custom FX bus) */
typedef struct AMFxParams {
    s32 words[0x108 / 4];
} AMFxParams;

extern AMFxParams D_800A06F4;
extern ALHeap *D_800E97A4;          // DKR: gAudioHeap
extern s32 gVideoRefreshRate;
extern s32 D_800F2168;              // DKR: frameSize
extern s32 D_800F216C;              // DKR: minFrameSize
extern s32 D_800F2170;              // JFG: the size of the frame buffers
extern u8 D_800F17B8[];             // the audio thread's stack top
extern OSMesg D_800F1B24[];
extern OSMesg D_800F1B44[];
extern OSMesg D_800F28B0[];
void func_8008A540(void *g, AMSynConfig *c); // DKR: alInit
void *alHeapDBAlloc(u8 *file, s32 line, ALHeap *hp, s32 num, s32 size);
void *__amDmaNew(AudioMgrHandle **out);
void __amMain(void *arg);

/**
 * Sets up the audio manager: the synthesizer (with JFG's custom reverb), the frame size, the command
 * lists and output buffers, the sample DMA buffers, the message queues and the thread.
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c, JFG's form.
 * Official Name: amCreateAudioMgr
 */
void amCreateAudioMgr(AMSynConfig *c, OSPri pri, void *audSched) {
    s32 i;
    f32 fsize;
    u8 *out;
    AMFxParams params;

    gAudioSched = audSched;
    D_800E97A4 = c->heap;
    c->outputRate = osAiSetFrequency(22020);
    c->dmaproc = __amDmaNew;
    for (i = 0; i < c->maxFXbusses; i++) {
        if (c->fxType[i] == AL_FX_CUSTOM) {
            params = D_800A06F4;
            c->params[i] = (s32 *) &params;
        }
    }
    func_8008A540(D_800F19F0, c);
    fsize = (f32) c->outputRate * 2 / (f32) gVideoRefreshRate;
    D_800F2168 = (s32) fsize;
    if (D_800F2168 < fsize) {
        D_800F2168++;
    }
    D_800F2168 = (D_800F2168 / 184) * 184 + 184;
    D_800F216C = D_800F2168 - 184;
    D_800F2170 = 0x1000;
    out = alHeapDBAlloc(0, 0, c->heap, 1, 0x6F40);
    ((u8 **) ((u8 *) &sAudioMgrThread + 0x280))[0] = out;
    ((u8 **) ((u8 *) &sAudioMgrThread + 0x280))[1] = out + 0x37A0;
    ((u8 **) ((u8 *) &sAudioMgrThread + 0x280))[2] = ((u8 **) ((u8 *) &sAudioMgrThread + 0x280))[0];
    ((u8 **) ((u8 *) &sAudioMgrThread + 0x280))[3] = ((u8 **) ((u8 *) &sAudioMgrThread + 0x280))[1];
    out = alHeapDBAlloc(0, 0, c->heap, 1, D_800F2170 * 12);
    for (i = 0; i < 3; i++) {
        ((u8 **) ((u8 *) &sAudioMgrThread + 0x310))[i] = out;
        ((u8 **) ((u8 *) &sAudioMgrThread + 0x31C))[i] = out;
        ((s16 *) ((u8 *) &sAudioMgrThread + 0x308))[i] = 0;
        out += D_800F2170 * 4;
    }
    out = alHeapDBAlloc(0, 0, c->heap, 1, NUM_DMA_BUFFERS * 0x280);
    dmaBuffs[0].node.prev = NULL;
    dmaBuffs[0].node.next = NULL;
    for (i = 0; i < NUM_DMA_BUFFERS - 1; i++) {
        alLink((ALLink *) &dmaBuffs[i + 1], (ALLink *) &dmaBuffs[i]);
        dmaBuffs[i].ptr = out;
        out += 0x280;
    }
    dmaBuffs[i].ptr = out;
    osCreateMesgQueue(&D_800F1B0C, D_800F1B44, 8);
    osCreateMesgQueue(&D_800F1AF4, D_800F1B24, 8);
    osCreateMesgQueue(&audDMAMessageQ, D_800F28B0, NUM_DMA_BUFFERS);
    osCreateThread(&sAudioMgrThread, -4, __amMain, NULL, D_800F17B8, pri);
}
void __amHandleFrameMsg(void);
void __amHandleDoneMsg(void);
void func_8008A5AC(void *synth); // DKR: alClose
void osScAddClient();

/* The audio manager thread: handles each retrace message until told to quit (0x80001D7C). */
/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__amMain):
 * the audio manager thread. JFG: the frame and done handlers take no arguments.
 */
void __amMain(UNUSED void *arg) {
    AudioMsgGen *msg = NULL;
    void *lastInfo = 0;
    s32 done = 0;

    osScAddClient(gAudioSched, audioStack, &D_800F1AF4, OS_MESG_BLOCK);
    while (!done) {
        (void) osRecvMesg(&D_800F1AF4, (OSMesg *) &msg, OS_MESG_BLOCK);
        switch (msg->type) {
            case 1:
                __amHandleFrameMsg();
                osRecvMesg(&D_800F1B0C, (OSMesg *) &lastInfo, OS_MESG_BLOCK);
                __amHandleDoneMsg();
                break;
            case 4:
                break;
            case 10:
                done = 1;
                break;
            default:
                break;
        }
    }
    func_8008A5AC(D_800F19F0);
}

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c.
 * Official Name: amGo
 */
void audioStartThread(void) {
    osStartThread(&sAudioMgrThread);
}

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c.
 */
void audioStopThread(void) {
    osStopThread(&sAudioMgrThread);
}

extern s32 D_800A06EC;

s32 func_80001EE8(void) {
    return D_800A06EC;
}

/* DKR OSScTask (the audio task) */
typedef struct AMTask {
    /* 0x00 */ void *next;
    /* 0x04 */ u32 state;
    /* 0x08 */ u32 flags;
    /* 0x0C */ void *framebuffer;
    /* 0x10 */ OSTask list;
    /* 0x50 */ OSMesgQueue *msgQ;
    /* 0x54 */ OSMesg msg;
    /* 0x58 */ s32 unk58;
    /* 0x5C */ s32 unk5C;
    /* 0x60 */ s32 unk60;
    /* 0x64 */ s32 unk64;
    /* 0x68 */ s32 unk68;
    /* 0x6C */ s32 unk6C;
} AMTask;

/* JFG's audio manager (DKR AMAudioMgr, with a second set of command lists and output buffers for the
   high quality mode) */
typedef struct AMAudioMgr {
    /* 0x000 */ OSThread thread;
    /* 0x1B0 */ u8 pad1B0[0x280 - 0x1B0];
    /* 0x280 */ Acmd *ACMDList[2];
    /* 0x288 */ Acmd *ACMDListNormal[2];
    /* 0x290 */ Acmd *ACMDListHQ[2];
    /* 0x298 */ AMTask task;
    /* 0x308 */ s16 frameSamples[3];
    /* 0x310 */ s16 *data[3];
    /* 0x31C */ s16 *dataNormal[3];
    /* 0x328 */ s16 *dataHQ[3];
} AMAudioMgr;
#define gAudioMgr (*(AMAudioMgr *) &sAudioMgrThread)

extern AudioMgrHandle dmaState;
extern AMDMABuffer *D_800A06C0; // JFG: the extra sample DMA buffers of the high quality mode
extern s32 D_800A06C4;          // DKR: curAcmdList
extern s32 D_800A06C8;          // JFG: the output buffer being filled (0 to 2)
extern s32 D_800A06CC;          // JFG: the output buffer being played
extern u32 audFrameCt;
extern s32 D_800A06D8;          // JFG: alternates the frame size when the AI has samples left
extern s32 D_800A06DC;          // JFG: the requested mode (1: high quality)
extern s32 D_800A06E0;          // JFG: the current mode
extern s32 D_800A06E4;          // JFG: frames until the high quality buffers are freed
extern s32 D_800A06E8;          // JFG: the high quality mode is ready
extern s32 D_800A06EC;
extern s32 D_800A06F0;          // JFG: the longest command list so far
extern s32 D_800F29E0;          // JFG: the high quality frame size multiplier (ramps up to 6)
#define OS_SC_NEEDS_RSP 0x0002 // DKR sched.h
void __clearAudioDMA(void);
void *mempool_alloc_safe(s32 size, u32 colourTag);
void mempool_free(void *data);
Acmd *func_8008AD10(Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen); // the synthesizer's frame (DKR: alAudioFrame)
OSMesgQueue *osScGetCmdQ(void *sc);

/**
 * Handle a retrace: play the last frame's output, switch the high quality mode on or off (it needs
 * bigger buffers and 76 more sample DMA buffers, freed 12 frames after leaving it), then build and
 * send this frame's audio task.
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__amHandleFrameMsg), JFG's form:
 * three output buffers, and no arguments.
 */
#ifdef NON_MATCHING
/* NON_MATCHING: the ROM keeps the `== 1` test of the mode in v0 (xori), shifting the t-registers by one */
void __amHandleFrameMsg(void) {
    s16 *audioPtr;
    Acmd *cmdp;
    s32 cmdLen;
    s32 samplesLeft;
    AMDMABuffer *buf;
    u8 *out;
    s32 i;
    UNUSED s32 pad1;
    UNUSED s32 pad2;

    __clearAudioDMA();
    samplesLeft = IO_READ(AI_LEN_REG) >> 2;
    osAiSetNextBuffer(gAudioMgr.data[D_800A06CC], gAudioMgr.frameSamples[D_800A06CC] << 2);
    if (D_800A06DC == 1) {
        D_800F29E0 += 2;
        if (D_800F29E0 >= 6) {
            D_800F29E0 = 6;
            D_800A06E8 = 1;
        } else {
            D_800A06E8 = 0;
        }
    }
    if (D_800A06E4 > 0) {
        if (--D_800A06E4 <= 0) {
            mempool_free(gAudioMgr.ACMDListHQ[0]);
            mempool_free(gAudioMgr.dataHQ[0]);
            for (i = 0; i < 76; i++) {
                buf = &D_800A06C0[i];
                if (buf == dmaState.firstUsed) {
                    dmaState.firstUsed = (AMDMABuffer *) D_800A06C0[i].node.next;
                }
                if (buf == dmaState.firstFree) {
                    dmaState.firstFree = (AMDMABuffer *) D_800A06C0[i].node.next;
                }
                alUnlink((ALLink *) buf);
            }
            mempool_free(D_800A06C0);
            D_800A06C0 = NULL;
        }
    }
    if (D_800A06DC != D_800A06E0) {
        D_800A06E8 = 0;
        if (!(D_800A06DC ^ 1) && D_800A06C0 == NULL) {
            gAudioMgr.ACMDListHQ[0] = mempool_alloc_safe(0x29B80, -1);
            gAudioMgr.ACMDListHQ[1] = (Acmd *) ((u8 *) gAudioMgr.ACMDListHQ[0] + 0x14DC0);
            out = mempool_alloc_safe(D_800F2170 * 72, -1);
            for (i = 0; i < 3; i++) {
                gAudioMgr.dataHQ[i] = (s16 *) out;
                gAudioMgr.data[i] = (s16 *) out;
                out += D_800F2170 * 24;
            }
            gAudioMgr.ACMDList[0] = gAudioMgr.ACMDListHQ[0];
            gAudioMgr.ACMDList[1] = gAudioMgr.ACMDListHQ[1];
            D_800A06C0 = mempool_alloc_safe(76 * (sizeof(AMDMABuffer) + 0x280), -1);
            if (dmaState.firstFree != NULL) {
                alLink((ALLink *) D_800A06C0, (ALLink *) dmaState.firstFree);
            } else {
                dmaState.firstFree = D_800A06C0;
                D_800A06C0->node.next = NULL;
                D_800A06C0->node.prev = NULL;
            }
            out = (u8 *) &D_800A06C0[76];
            for (i = 0; i < 75; i++) {
                alLink((ALLink *) &D_800A06C0[i + 1], (ALLink *) &D_800A06C0[i]);
                D_800A06C0[i].ptr = out;
                out += 0x280;
            }
            D_800A06C0[i].ptr = out;
            D_800F29E0 = 1;
        } else {
            for (i = 0; i < 3; i++) {
                gAudioMgr.data[i] = gAudioMgr.dataNormal[i];
            }
            D_800A06E4 = 12;
            gAudioMgr.ACMDList[0] = gAudioMgr.ACMDListNormal[0];
            gAudioMgr.ACMDList[1] = gAudioMgr.ACMDListNormal[1];
        }
        D_800A06E0 = D_800A06DC;
    }
    if (D_800A06DC == 0) {
        D_800A06EC = 2;
    } else {
        D_800A06EC = D_800F29E0 * 2;
    }
    audioPtr = (s16 *) osVirtualToPhysical(gAudioMgr.data[D_800A06C8]);
    if ((samplesLeft >= 313) & D_800A06D8) {
        D_800A06D8 = 0;
        gAudioMgr.frameSamples[D_800A06C8] = D_800F216C;
    } else {
        D_800A06D8 = 1;
        gAudioMgr.frameSamples[D_800A06C8] = D_800F2168;
    }
    if (D_800A06DC == 1) {
        gAudioMgr.frameSamples[D_800A06C8] *= D_800F29E0;
    }
    cmdp = func_8008AD10(gAudioMgr.ACMDList[D_800A06C4], &cmdLen, audioPtr, gAudioMgr.frameSamples[D_800A06C8]);
    if (D_800A06F0 < cmdLen) {
        D_800A06F0 = cmdLen;
    }
    gAudioMgr.task.msgQ = &D_800F1B0C;
    gAudioMgr.task.unk6C = 1;
    gAudioMgr.task.unk58 = -1;
    gAudioMgr.task.flags = OS_SC_NEEDS_RSP;
    gAudioMgr.task.next = NULL;
    gAudioMgr.task.msg = NULL;
    gAudioMgr.task.unk60 = 0xFF;
    gAudioMgr.task.unk5C = 0;
    gAudioMgr.task.unk64 = 0;
    gAudioMgr.task.list.t.type = M_AUDTASK;
    gAudioMgr.task.list.t.flags = 2;
    gAudioMgr.task.list.t.ucode_boot = (u64 *) rspbootTextStart;
    gAudioMgr.task.list.t.ucode_boot_size = (s32) rspbootTextEnd - (s32) rspbootTextStart;
    gAudioMgr.task.list.t.ucode = (u64 *) aspMainTextStart;
    gAudioMgr.task.list.t.ucode_data = (u64 *) aspMainDataStart;
    gAudioMgr.task.list.t.ucode_size = 0x1000;
    gAudioMgr.task.list.t.ucode_data_size = 0x800;
    gAudioMgr.task.list.t.data_ptr = (u64 *) gAudioMgr.ACMDList[D_800A06C4];
    gAudioMgr.task.list.t.yield_data_ptr = NULL;
    gAudioMgr.task.list.t.data_size = (cmdp - gAudioMgr.ACMDList[D_800A06C4]) * sizeof(Acmd);
    gAudioMgr.task.list.t.yield_data_size = 0;
    osSendMesg(osScGetCmdQ(gAudioSched), (OSMesg) &gAudioMgr.task, OS_MESG_NOBLOCK);
    D_800A06C4 ^= 1;
    D_800A06CC = D_800A06C8++;
    audFrameCt++;
    D_800A06C8 %= 3;
}
#else
INCLUDE_ASM("asm/nonmatchings/2610", func_80001EF8);
#endif

extern s32 D_800A07FC; // DKR: firstTime (a static set to 1)
u32 osAiGetLength(void);

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__amHandleDoneMsg):
 * checks for the audio interface running out of samples (the report is stubbed out).
 * JFG: takes no arguments.
 */
void __amHandleDoneMsg(void) {
    s32 samplesLeft;
    s32 *firstTime = &D_800A07FC; // fake? a plain global shares one address register

    samplesLeft = osAiGetLength() >> 2;
    if (samplesLeft == 0 && !*firstTime) {
        stubbed_printf("audio: ai out of samples\n");
        *firstTime = 0;
    }
}

extern AudioMgrHandle dmaState;
extern u32 audFrameCt;
extern u32 nextDMA;

extern OSIoMesg audDMAIOMesgBuf[];

/* Returns a DMA'd sample buffer for a ROM address, starting a new DMA when no buffer holds it (0x800024C4). */
/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__amDMA): the sample DMA
 * callback.
 */
s32 __amDMA(s32 addr, s32 len, UNUSED void *state) {
    s32 addrEnd;
    AMDMABuffer *lastDmaPtr;
    void *foundBuffer;
    s32 delta;
    s32 buffEnd;
    AMDMABuffer *dmaPtr;
    UNUSED s32 pad;

    lastDmaPtr = NULL;
    delta = addr & 1;
    dmaPtr = dmaState.firstUsed;
    addrEnd = addr + len;
    while (dmaPtr) {
        buffEnd = dmaPtr->startAddr + 0x280;
        if (dmaPtr->startAddr > (u32) addr) {
            break;
        } else if (addrEnd <= buffEnd) {
            dmaPtr->lastFrame = audFrameCt;
            foundBuffer = dmaPtr->ptr + addr - dmaPtr->startAddr;
            return (int) osVirtualToPhysical(foundBuffer);
        }
        lastDmaPtr = dmaPtr;
        dmaPtr = (AMDMABuffer *) dmaPtr->node.next;
    }
    dmaPtr = dmaState.firstFree;
    if (!dmaPtr && !lastDmaPtr) {
        lastDmaPtr = dmaState.firstUsed;
    }
    if (!dmaPtr) {
        stubbed_printf("OH DEAR - No audio DMA buffers left\n");
        return (int) osVirtualToPhysical(lastDmaPtr->ptr) + delta;
    }
    dmaState.firstFree = (AMDMABuffer *) dmaPtr->node.next;
    alUnlink((ALLink *) dmaPtr);
    if (lastDmaPtr) {
        alLink((ALLink *) dmaPtr, (ALLink *) lastDmaPtr);
    } else if (dmaState.firstUsed) {
        lastDmaPtr = dmaState.firstUsed;
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = (ALLink *) lastDmaPtr;
        dmaPtr->node.prev = 0;
        lastDmaPtr->node.prev = (ALLink *) dmaPtr;
    } else {
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = 0;
        dmaPtr->node.prev = 0;
    }
    foundBuffer = dmaPtr->ptr;
    addr -= delta;
    dmaPtr->startAddr = addr;
    dmaPtr->lastFrame = audFrameCt;
    osPiStartDma(&audDMAIOMesgBuf[nextDMA++], OS_MESG_PRI_HIGH, OS_READ, addr, foundBuffer, 0x280, &audDMAMessageQ);
    return (int) osVirtualToPhysical(foundBuffer) + delta;
}

s32 __amDMA(s32 addr, s32 len, void *state);

/* Initialises the DMA state once and returns the sample DMA callback (0x80002694). */
void *__amDmaNew(AudioMgrHandle **out) {
    if (!dmaState.initialised) {
        dmaState.firstUsed = NULL;
        dmaState.firstFree = (AMDMABuffer *) dmaBuffs;
        dmaState.initialised = TRUE;
    }
    *out = &dmaState;
    return __amDMA;
}

/* Moves the DMA buffers not used for a frame back to the free list (0x800026CC). */
/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__clearAudioDMA):
 * move the DMA buffers not used for a frame back to the free list.
 */
void __clearAudioDMA(void) {
    u32 i;
    OSIoMesg *iomsg = 0;
    AMDMABuffer *dmaPtr, *nextPtr;

    for (i = 0; i < nextDMA; i++) {
        if (osRecvMesg(&audDMAMessageQ, (OSMesg *) &iomsg, OS_MESG_NOBLOCK) == -1) {
            stubbed_printf("Dma not done\n");
        }
    }
    dmaPtr = dmaState.firstUsed;
    while (dmaPtr) {
        nextPtr = (AMDMABuffer *) dmaPtr->node.next;
        if (dmaPtr->lastFrame + 1 < audFrameCt) {
            if (dmaState.firstUsed == dmaPtr) {
                dmaState.firstUsed = (AMDMABuffer *) dmaPtr->node.next;
            }
            alUnlink((ALLink *) dmaPtr);
            if (dmaState.firstFree) {
                alLink((ALLink *) dmaPtr, (ALLink *) dmaState.firstFree);
            } else {
                dmaState.firstFree = dmaPtr;
                dmaPtr->node.next = 0;
                dmaPtr->node.prev = 0;
            }
        }
        dmaPtr = nextPtr;
    }
    nextDMA = 0;
}

extern s32 D_800A06DC;
extern s32 D_800A06E4;

void func_800027D8(s32 mode) {
    if ((D_800A06DC = mode) == 1) {
        D_800A06E4 = 0;
    }
}

extern s32 D_800A06E8;

s32 func_800027FC(void) {
    return D_800A06E8;
}

s32 func_8000280C(void) {
    return D_800A06C0 == NULL;
}
