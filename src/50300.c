#include "common.h"

/*
 * Rare's RCP scheduler: a thread that takes the VI retrace, SP/DP done and pre-NMI interrupts and runs
 * the audio and graphics tasks queued on it. Written from the assembly; the layouts are JFG's.
 */

/* Interrupt messages the scheduler thread receives */
#define SC_MSG_RETRACE 0x29A
#define SC_MSG_SP_DONE 0x29B
#define SC_MSG_DP_DONE 0x29C
#define SC_MSG_PRE_NMI 0x29D
#define SC_MSG_AUDIO_READY 0x63

/* SchedTask.state */
#define SC_NEEDS_RDP 0x1
#define SC_NEEDS_RSP 0x2
#define SC_YIELD 0x10
#define SC_YIELDED 0x20
/* SchedTask.flags */
#define SC_RCP_MASK 0x3
#define SC_TYPE_MASK 0x7
#define SC_TYPE_XBUS 0x3
#define SC_SWAPBUFFER 0x20

typedef struct SchedTask {
    /* 0x00 */ struct SchedTask *next;
    /* 0x04 */ u32 state;
    /* 0x08 */ u32 flags;
    /* 0x0C */ void *framebuffer;
    /* 0x10 */ OSTask list;
    /* 0x50 */ OSMesgQueue *msgQ;
    /* 0x54 */ OSMesg msg;
    /* 0x58 */ u8 pad58[0x68 - 0x58];
    /* 0x68 */ s32 unk68;
} SchedTask;

typedef struct SchedClient {
    /* 0x00 */ u8 id;
    /* 0x04 */ struct SchedClient *next;
    /* 0x08 */ OSMesgQueue *msgQ;
} SchedClient;

typedef struct SchedMsg {
    /* 0x00 */ s16 type;
    /* 0x02 */ u8 pad2[0x1E];
} SchedMsg;

typedef struct OSSched {
    /* 0x000 */ SchedMsg retraceMsg;
    /* 0x020 */ SchedMsg prenmiMsg;
    /* 0x040 */ OSMesgQueue interruptQ;
    /* 0x058 */ OSMesg intBuf[8];
    /* 0x078 */ OSMesgQueue cmdQ;
    /* 0x090 */ OSMesg cmdMsgBuf[8];
    /* 0x0B0 */ OSThread thread;
    /* 0x260 */ u8 pad260[0x2E0 - 0x260];
    /* 0x2E0 */ SchedClient *clientList;
    /* 0x2E4 */ SchedTask *audioListHead;
    /* 0x2E8 */ SchedTask *gfxListHead;
    /* 0x2EC */ SchedTask *audioListTail;
    /* 0x2F0 */ SchedTask *gfxListTail;
    /* 0x2F4 */ SchedTask *curRSPTask;
    /* 0x2F8 */ SchedTask *curRDPTask;
    /* 0x2FC */ SchedTask *heldTask; /* a buffer-swapping task done too early */
    /* 0x300 */ u32 retraceCount;
    /* 0x304 */ s32 doAudio;
} OSSched;

extern OSViMode D_800A9AB0;
extern OSViMode D_800A9B00;
extern OSViMode D_800A9B50;
extern u64 D_800FE958;
extern s32 D_800A38C0;
extern s32 D_800A38C4;
extern s32 D_800A38CC;
extern u8 D_800A38B0;

void __scMain(OSSched *sc);
void __scHandleAudioReady(OSSched *sc);
void __scHandleRetrace(OSSched *sc);
void __scHandleRSP(OSSched *sc);
void __scHandleRDP(OSSched *sc);
s32 __scTaskComplete(OSSched *sc, SchedTask *t);
void __scAppendList(OSSched *sc, SchedTask *t);
void __scExec(OSSched *sc, SchedTask *sp, SchedTask *dp);
void __scYield(OSSched *sc);
s32 __scSchedule(OSSched *sc, SchedTask **sp, SchedTask **dp, s32 availRCP);

/* Sets up the scheduler: VI mode by TV type, the message queues and events, then starts its thread. */
void osCreateScheduler(OSSched *sc, void *stack, OSPri priority, u8 mode, u8 numFields) {
    sc->curRSPTask = NULL;
    sc->curRDPTask = NULL;
    sc->clientList = NULL;
    sc->audioListHead = NULL;
    sc->gfxListHead = NULL;
    sc->audioListTail = NULL;
    sc->gfxListTail = NULL;
    sc->retraceCount = 0;
    sc->heldTask = NULL;
    sc->retraceMsg.type = 1;
    sc->prenmiMsg.type = 4;
    osCreateViManager(OS_PRIORITY_VIMGR);
    switch (mode) {
        case 14:
            osViSetMode(&D_800A9AB0);
            break;
        case 28:
            osViSetMode(&D_800A9B00);
            break;
        default:
            osViSetMode(&D_800A9B50);
            break;
    }
    osViBlack(TRUE);
    osCreateMesgQueue(&sc->interruptQ, sc->intBuf, 8);
    osCreateMesgQueue(&sc->cmdQ, sc->cmdMsgBuf, 8);
    osSetEventMesg(OS_EVENT_SP, &sc->interruptQ, (OSMesg) SC_MSG_SP_DONE);
    osSetEventMesg(OS_EVENT_DP, &sc->interruptQ, (OSMesg) SC_MSG_DP_DONE);
    osSetEventMesg(OS_EVENT_PRENMI, &sc->interruptQ, (OSMesg) SC_MSG_PRE_NMI);
    osViSetEvent(&sc->interruptQ, (OSMesg) SC_MSG_RETRACE, numFields);
    osCreateThread(&sc->thread, 5, (void (*)(void *)) __scMain, sc, stack, priority);
    osStartThread(&sc->thread);
}

/* Adds a client (sent the retrace and pre-NMI messages) to the scheduler's list. */
void osScAddClient(OSSched *sc, SchedClient *c, OSMesgQueue *msgQ, u8 id) {
    OSIntMask mask = osSetIntMask(1);

    c->msgQ = msgQ;
    c->next = sc->clientList;
    c->id = id;
    sc->clientList = c;
    osSetIntMask(mask);
}

/* Removes a client from the scheduler's list. */
void osScRemoveClient(OSSched *sc, SchedClient *c) {
    SchedClient *client = sc->clientList;
    SchedClient *prev = NULL;
    OSIntMask mask = osSetIntMask(1);

    while (client != NULL) {
        if (client == c) {
            if (prev != NULL) {
                prev->next = c->next;
            } else {
                sc->clientList = c->next;
            }
            break;
        }
        prev = client;
        client = client->next;
    }
    osSetIntMask(mask);
}

/* The scheduler's command queue (at 0x78 in its state) (0x8004F950). */
void *osScGetCmdQ(u8 *sc) {
    return sc + 0x78;
}

/* The scheduler's interrupt queue (at 0x40 in its state) (0x8004F958). */
void *osScGetInterruptQ(u8 *sc) {
    return sc + 0x40;
}

void func_8004F960(f32 *a, f32 *b, f32 *c) {
    *a = 0.0f;
    *b = 0.0f;
    *c = 0.0f;
}

/* The scheduler thread: waits for interrupt messages and task submissions, and runs the tasks. */
void __scMain(OSSched *sc) {
    OSMesg msg = NULL;
    SchedClient *client;
    s32 state;
    UNUSED s32 pad;
    SchedTask *sp = NULL;
    SchedTask *dp = NULL;

    while (TRUE) {
        osRecvMesg(&sc->interruptQ, &msg, OS_MESG_BLOCK);
        switch ((s32) msg) {
            case SC_MSG_RETRACE:
                __scHandleRetrace(sc);
                D_800A38CC++;
                break;
            case SC_MSG_SP_DONE:
                __scHandleRSP(sc);
                break;
            case SC_MSG_DP_DONE:
                __scHandleRDP(sc);
                break;
            case SC_MSG_AUDIO_READY:
                __scHandleAudioReady(sc);
                break;
            case SC_MSG_PRE_NMI:
                for (client = sc->clientList; client != NULL; client = client->next) {
                    osSendMesg(client->msgQ, (OSMesg) &sc->prenmiMsg, OS_MESG_NOBLOCK);
                }
                break;
            default:
                __scAppendList(sc, (SchedTask *) msg);
                state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
                if (__scSchedule(sc, &sp, &dp, state) != state) {
                    __scExec(sc, sp, dp);
                }
                break;
        }
    }
}

/* Audio is ready: yields the running graphics task for it, or schedules it. */
void __scHandleAudioReady(OSSched *sc) {
    s32 state;
    SchedTask *sp = NULL;
    SchedTask *dp = NULL;

    if (sc->audioListHead != NULL) {
        sc->doAudio = TRUE;
    }
    if (sc->doAudio && sc->curRSPTask != NULL) {
        __scYield(sc);
    } else {
        state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
        if (__scSchedule(sc, &sp, &dp, state) != state) {
            __scExec(sc, sp, dp);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/50300", func_8004FBD8);

void func_8004FC64(s32 arg0) {
}

INCLUDE_ASM("asm/nonmatchings/50300", func_8004FC6C);

INCLUDE_ASM("asm/nonmatchings/50300", func_8004FF64);

INCLUDE_ASM("asm/nonmatchings/50300", func_80050154);

#ifdef NON_MATCHING
/* NON_MATCHING: the not-yielded arm stores state after the flags move (two words swapped) */
/* SP done: requeues a yielded task, or completes the task, then schedules the next ones. */
void __scHandleRSP(OSSched *sc) {
    SchedTask *t;
    SchedTask *sp = NULL;
    SchedTask *dp = NULL;
    s32 state;

    t = sc->curRSPTask;
    sc->curRSPTask = NULL;
    if (t->state & SC_YIELD) {
        if (osSpTaskYielded(&t->list)) {
            t->state |= SC_YIELDED;
            if ((t->flags & SC_TYPE_MASK) == SC_TYPE_XBUS) {
                t->next = sc->gfxListHead;
                sc->gfxListHead = t;
                if (sc->gfxListTail == NULL) {
                    sc->gfxListTail = t;
                }
            }
        } else {
            t->state &= ~SC_NEEDS_RSP;
        }
        if ((t->flags & SC_TYPE_MASK) != SC_TYPE_XBUS) {
        }
    } else {
        t->state &= ~SC_NEEDS_RSP;
        __scTaskComplete(sc, t);
    }
    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/50300", func_800507F8);
#endif

/* DP done: completes the task, then schedules the next ones. */
void __scHandleRDP(OSSched *sc) {
    SchedTask *t;
    SchedTask *sp = NULL;
    SchedTask *dp = NULL;
    s32 state;

    t = sc->curRDPTask;
    sc->curRDPTask = NULL;
    t->state &= ~SC_NEEDS_RDP;
    __scTaskComplete(sc, t);
    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }
}

/* A task may run when the framebuffer swap has happened (current = next framebuffer). */
SchedTask *__scTaskReady(SchedTask *t) {
    void *fb;

    if (t != NULL) {
        fb = osViGetCurrentFramebuffer();
        if (osViGetNextFramebuffer() != fb) {
            return NULL;
        }
        return t;
    }
    return NULL;
}

/* A task done on both processors: sends its message (a buffer-swapping task waits for two retraces). */
s32 __scTaskComplete(OSSched *sc, SchedTask *t) {
    if ((t->state & SC_RCP_MASK) == 0) {
        if (t->msgQ != NULL) {
            if (t->flags & SC_SWAPBUFFER) {
                if (sc->retraceCount < 2) {
                    sc->heldTask = t;
                    return TRUE;
                }
                if (t->unk68 != 0 || t->msg != NULL) {
                    osSendMesg(t->msgQ, t->msg, OS_MESG_BLOCK);
                } else {
                    osSendMesg(t->msgQ, &D_800A38B0, OS_MESG_BLOCK);
                }
                sc->retraceCount = 0;
                return TRUE;
            }
            if (t->unk68 != 0 || t->msg != NULL) {
                osSendMesg(t->msgQ, t->msg, OS_MESG_BLOCK);
            } else {
                osSendMesg(t->msgQ, &D_800A38B0, OS_MESG_BLOCK);
            }
        }
        return TRUE;
    }
    return FALSE;
}

/* Appends a task to the audio or graphics list. */
void __scAppendList(OSSched *sc, SchedTask *t) {
    s32 type = t->list.t.type;

    if (type == M_AUDTASK) {
        if (sc->audioListTail != NULL) {
            sc->audioListTail->next = t;
        } else {
            sc->audioListHead = t;
        }
        sc->audioListTail = t;
    } else {
        if (sc->gfxListTail != NULL) {
            sc->gfxListTail->next = t;
        } else {
            sc->gfxListHead = t;
        }
        sc->gfxListTail = t;
    }
    t->next = NULL;
    t->state = t->flags & SC_RCP_MASK;
}

/* Starts a task on the RSP and/or sets the RDP's next buffer. */
void __scExec(OSSched *sc, SchedTask *sp, SchedTask *dp) {
    if (sp != NULL) {
        if (sp->list.t.type == M_AUDTASK) {
            osWritebackDCacheAll();
        }
        sp->state &= ~(SC_YIELD | SC_YIELDED);
        osSpTaskLoad(&sp->list);
        osSpTaskStartGo(&sp->list);
        D_800A38C0 = 0;
        D_800A38C4 = 0;
        sc->curRSPTask = sp;
        if (sp == dp) {
            sc->curRDPTask = dp;
        }
    }
    if (dp != NULL && dp != sp) {
        osDpSetNextBuffer(dp->list.t.output_buff, *dp->list.t.output_buff_size);
        sc->curRDPTask = dp;
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: the time's two words share one lui in the ROM: D_800FE958 is this file's .bss */
/* Asks the running graphics task to yield (for audio), noting the time. */
void __scYield(OSSched *sc) {
    SchedTask *t = sc->curRSPTask;

    if (t->list.t.type == M_GFXTASK) {
        t->state |= SC_YIELD;
        D_800FE958 = osGetTime();
        osSpTaskYield();
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/50300", func_80050C3C);
#endif

INCLUDE_ASM("asm/nonmatchings/50300", func_80050C90);
