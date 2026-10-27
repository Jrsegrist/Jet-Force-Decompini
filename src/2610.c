#include "common.h"

/* The audio manager thread (part of DKR's AMAudioMgr __am). */
extern OSThread sAudioMgrThread;

INCLUDE_ASM("asm/nonmatchings/2610", func_80001A10);

/* DKR AudioMsg, reduced */
typedef struct AudioMsgGen {
    s16 type;
} AudioMsgGen;
extern void *D_800E97A0;       // DKR: gAudioSched
extern u8 D_800E97A8[];         // DKR: audioStack (the scheduler client)
extern OSMesgQueue D_800F1AF4;  // DKR: __am.audioFrameMsgQ
extern OSMesgQueue D_800F1B0C;  // DKR: __am.audioReplyMsgQ
extern u8 D_800F19F0[];         // DKR: __am.g
void func_80001EF8(void);
void func_80002484(void);
void func_8008A5AC(void *synth); // DKR: alClose
void osScAddClient();

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__amMain):
 * the audio manager thread. JFG: the frame and done handlers take no arguments.
 */
void func_80001D7C(UNUSED void *arg) {
    AudioMsgGen *msg = NULL;
    void *lastInfo = 0;
    s32 done = 0;

    osScAddClient(D_800E97A0, D_800E97A8, &D_800F1AF4, OS_MESG_BLOCK);
    while (!done) {
        (void) osRecvMesg(&D_800F1AF4, (OSMesg *) &msg, OS_MESG_BLOCK);
        switch (msg->type) {
            case 1:
                func_80001EF8();
                osRecvMesg(&D_800F1B0C, (OSMesg *) &lastInfo, OS_MESG_BLOCK);
                func_80002484();
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

INCLUDE_ASM("asm/nonmatchings/2610", func_80001EF8);

INCLUDE_ASM("asm/nonmatchings/2610", func_80002484);

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
extern AudioMgrHandle D_800F1B68; // DKR: dmaState
extern u8 D_800F1B78[];
extern u32 D_800A06D0;            // DKR: audFrameCt
extern u32 D_800A06D4;            // DKR: nextDMA
extern OSMesgQueue D_800F2898;    // DKR: audDMAMessageQ

extern OSIoMesg D_800F2178[]; // DKR: audDMAIOMesgBuf

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__amDMA): the sample DMA
 * callback. JFG: no debug print when no buffer is free.
 */
s32 func_800024C4(s32 addr, s32 len, UNUSED void *state) {
    s32 addrEnd;
    AMDMABuffer *lastDmaPtr;
    void *foundBuffer;
    s32 delta;
    s32 buffEnd;
    AMDMABuffer *dmaPtr;
    UNUSED s32 pad;

    lastDmaPtr = NULL;
    delta = addr & 1;
    dmaPtr = D_800F1B68.firstUsed;
    addrEnd = addr + len;
    while (dmaPtr) {
        buffEnd = dmaPtr->startAddr + 0x280;
        if (dmaPtr->startAddr > (u32) addr) {
            break;
        } else if (addrEnd <= buffEnd) {
            dmaPtr->lastFrame = D_800A06D0;
            foundBuffer = dmaPtr->ptr + addr - dmaPtr->startAddr;
            return (int) osVirtualToPhysical(foundBuffer);
        }
        lastDmaPtr = dmaPtr;
        dmaPtr = (AMDMABuffer *) dmaPtr->node.next;
    }
    dmaPtr = D_800F1B68.firstFree;
    if (!dmaPtr && !lastDmaPtr) {
        lastDmaPtr = D_800F1B68.firstUsed;
    }
    if (!dmaPtr) {
        return (int) osVirtualToPhysical(lastDmaPtr->ptr) + delta;
    }
    D_800F1B68.firstFree = (AMDMABuffer *) dmaPtr->node.next;
    alUnlink((ALLink *) dmaPtr);
    if (lastDmaPtr) {
        alLink((ALLink *) dmaPtr, (ALLink *) lastDmaPtr);
    } else if (D_800F1B68.firstUsed) {
        lastDmaPtr = D_800F1B68.firstUsed;
        D_800F1B68.firstUsed = dmaPtr;
        dmaPtr->node.next = (ALLink *) lastDmaPtr;
        dmaPtr->node.prev = 0;
        lastDmaPtr->node.prev = (ALLink *) dmaPtr;
    } else {
        D_800F1B68.firstUsed = dmaPtr;
        dmaPtr->node.next = 0;
        dmaPtr->node.prev = 0;
    }
    foundBuffer = dmaPtr->ptr;
    addr -= delta;
    dmaPtr->startAddr = addr;
    dmaPtr->lastFrame = D_800A06D0;
    osPiStartDma(&D_800F2178[D_800A06D4++], OS_MESG_PRI_HIGH, OS_READ, addr, foundBuffer, 0x280, &D_800F2898);
    return (int) osVirtualToPhysical(foundBuffer) + delta;
}

s32 func_800024C4(s32 addr, s32 len, void *state);

void *func_80002694(AudioMgrHandle **out) {
    if (!D_800F1B68.initialised) {
        D_800F1B68.firstUsed = NULL;
        D_800F1B68.firstFree = (AMDMABuffer *) D_800F1B78;
        D_800F1B68.initialised = TRUE;
    }
    *out = &D_800F1B68;
    return func_800024C4;
}

/**
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/audiomgr.c (__clearAudioDMA):
 * move the DMA buffers not used for a frame back to the free list.
 */
void func_800026CC(void) {
    u32 i;
    OSIoMesg *iomsg = 0;
    AMDMABuffer *dmaPtr, *nextPtr;

    for (i = 0; i < D_800A06D4; i++) {
        osRecvMesg(&D_800F2898, (OSMesg *) &iomsg, OS_MESG_NOBLOCK);
    }
    dmaPtr = D_800F1B68.firstUsed;
    while (dmaPtr) {
        nextPtr = (AMDMABuffer *) dmaPtr->node.next;
        if (dmaPtr->lastFrame + 1 < D_800A06D0) {
            if (D_800F1B68.firstUsed == dmaPtr) {
                D_800F1B68.firstUsed = (AMDMABuffer *) dmaPtr->node.next;
            }
            alUnlink((ALLink *) dmaPtr);
            if (D_800F1B68.firstFree) {
                alLink((ALLink *) dmaPtr, (ALLink *) D_800F1B68.firstFree);
            } else {
                D_800F1B68.firstFree = dmaPtr;
                dmaPtr->node.next = 0;
                dmaPtr->node.prev = 0;
            }
        }
        dmaPtr = nextPtr;
    }
    D_800A06D4 = 0;
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

extern s32 D_800A06C0;

s32 func_8000280C(void) {
    return D_800A06C0 == 0;
}
