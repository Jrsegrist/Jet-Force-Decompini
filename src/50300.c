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
#define SC_PARALLEL_TASK 0x10

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
    /* 0x6C */ s32 unk6C; // the task kind (sc_task_type_name)
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

/************ .bss ************/

s32 D_800FE950; // the SP was seen running last retrace (arms the hang report)
s32 D_800FE954; // the same for the DP
u64 D_800FE958; // the time the last graphics task was asked to yield

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

/* Debug: the name of a task type (a task's 0x6C field: 1 audio, 2 game, 3-7 the other kinds). */
char *sc_task_type_name(s32 type) {
    switch (type) {
        case 1:
            return "(Audio task)";
        case 2:
            return "(Game task)";
        case 3:
            return "(DI task)\n";
        case 4:
            return "(DI benchmark test)\n";
        case 5:
            return "(Clone task)\n";
        case 6:
            return "(Refract task)\n";
        case 7:
            return "(Blur task)\n";
    }
    return "(Unknown task type %d)\n";
}

/* Debug: printed a task's fields (the prints were compiled out; their strings stay). */
void sc_task_print(SchedTask *t) {
    stubbed_printf("\nRCP TASK INFO\n");
    stubbed_printf("-------------\n");
    stubbed_printf("\ttype\t\t= %u\n");
    stubbed_printf("\tflags\t\t= %u\n");
    stubbed_printf("\tucode_boot\t\t= %p\n");
    stubbed_printf("\tucode_boot_size\t\t= %u\n");
    stubbed_printf("\tucode\t\t= %p\n");
    stubbed_printf("\tucode_size\t\t= %u\n");
    stubbed_printf("\tucode_data\t\t= %p\n");
    stubbed_printf("\tucode_data_size\t\t= %u\n");
    stubbed_printf("\toutput_buff\t\t= %p\n");
    stubbed_printf("\toutput_buff_size\t\t= %u\n");
    stubbed_printf("\tdata_ptr\t\t= %p\n");
    stubbed_printf("\tdata_size\t\t= %u\n");
    stubbed_printf("\tframebuffer\t\t= %p\n");
}

void __osSpSetStatus(u32 data);
void func_80065F80(Gfx *from, Gfx *to, s32 arg2);
void debug_trace_find_nearest(u32 addr, s32 *belowB, s32 *belowC, s32 *belowA, s32 *aboveB, s32 *aboveC, s32 *aboveA);

/* The RSP and RDP set back to run a task: SP halt, broke, interrupts and signals cleared; DP flags cleared */
#define SP_RESET_STATUS 0xAAAA82
#define DP_RESET_STATUS 0x1D6

/*
 * Debug: finds the display list command that hangs the RCP, by bisection: ends the list after a
 * command (a full sync), runs the task again and waits for the RDP to finish (up to 10 retraces),
 * then moves forwards or backwards by half the remaining count. A hang in a branch to another
 * display list continues in that list. Returns the command found.
 */
Gfx *rcp_hang_bisect(OSSched *sc, s32 count, Gfx *dl, OSMesgQueue *mq, Gfx *start) {
    Gfx *next;
    UNUSED s32 pad0;
    u64 saved0;
    u64 saved1;
    s32 found = FALSE;
    UNUSED s32 pad[2];
    OSMesg msg = NULL;
    s32 retraces;
    s32 done;
    Gfx *sub;
    Gfx *from;
    Gfx *p;

    do {
        next = dl + 1;
        __osSpSetStatus(SP_RESET_STATUS);
        done = FALSE;
        retraces = 0;
        osDpSetStatus(DP_RESET_STATUS);
        saved0 = *(u64 *) dl;
        saved1 = *(u64 *) (dl + 1);
        gDPFullSync(&dl[0]);
        gSPEndDisplayList(&dl[1]);
        osWritebackDCacheAll();
        osSpTaskLoad(&sc->curRSPTask->list);
        osSpTaskStartGo(&sc->curRSPTask->list);
        do {
            osRecvMesg(mq, &msg, OS_MESG_BLOCK);
            switch ((s32) msg) {
                case SC_MSG_RETRACE:
                    retraces++;
                    break;
                case SC_MSG_SP_DONE:
                    break;
                case SC_MSG_DP_DONE:
                    done = TRUE;
                    break;
            }
        } while (retraces < 10 && !done);
        *(u64 *) dl = saved0;
        *(u64 *) next = saved1;
        if (count < 2) {
            if (*(u8 *) dl == G_DL) {
                stubbed_printf("Branch DisplayList Crash gfx ptr = %x\n");
                stubbed_printf("*** CHILD DISPLAY LIST\n");
                from = dl - 40;
                if ((u32) from < 0x80000000) {
                    from = (Gfx *) ((u32) from + 0x80000000);
                }
                if ((s32) from < (s32) start) {
                    from = start;
                }
                func_80065F80(from, dl, 0x50);
                p = (Gfx *) dl->words.w1;
                if ((u32) p < 0x80000000) {
                    p = (Gfx *) ((u32) p + 0x80000000);
                }
                sub = p;
                for (count = 0; *(s8 *) p != (s8) G_ENDDL; p++) {
                    count++;
                }
                if (count & 1) {
                    count = count / 2 + 1;
                } else {
                    count = count / 2;
                }
                dl = sub;
                dl += count;
                if ((u32) dl < 0x80000000) {
                    dl = (Gfx *) ((u32) dl + 0x80000000);
                }
                func_80065F80(sub, dl, 0xA0);
                return rcp_hang_bisect(sc, count, dl, mq, sub);
            }
            found = TRUE;
        }
        if (!found) {
            if (count & 1) {
                count = count / 2 + 1;
            } else {
                count = count / 2;
            }
            if (done) {
                dl += count;
            } else {
                dl -= count;
            }
        }
    } while (!found);
    return dl;
}

/*
 * Debug: finds the command the running graphics task hangs on (rcp_hang_bisect, from the middle of its
 * display list, with the scheduler's interrupts sent to a queue of its own), and the nearest marked
 * positions in the display list before and after it (debug_trace_find_nearest); dumps the commands before it.
 */
Gfx *rcp_hang_find(OSSched *sc, s32 *belowB, s32 *belowC, s32 *belowA, s32 *aboveB, s32 *aboveC, s32 *aboveA) {
    u32 half;
    OSMesg msgBuf[8];
    OSMesgQueue mq;
    Gfx *dl;
    OSTask *task;
    s32 nextB;
    s32 prevB;
    s32 nextC;
    s32 prevC;
    s32 nextA;
    s32 prevA;
    Gfx *from;

    task = &sc->curRSPTask->list;
    half = task->t.data_size >> 1;
    dl = (Gfx *) task->t.data_ptr + half;
    osCreateMesgQueue(&mq, msgBuf, 8);
    osSetEventMesg(OS_EVENT_SP, &mq, (OSMesg) SC_MSG_SP_DONE);
    osSetEventMesg(OS_EVENT_DP, &mq, (OSMesg) SC_MSG_DP_DONE);
    osViSetEvent(&mq, (OSMesg) SC_MSG_RETRACE, 1);
    dl = rcp_hang_bisect(sc, half, dl, &mq, (Gfx *) task->t.data_ptr);
    osSetEventMesg(OS_EVENT_SP, &sc->interruptQ, (OSMesg) SC_MSG_SP_DONE);
    osSetEventMesg(OS_EVENT_DP, &sc->interruptQ, (OSMesg) SC_MSG_DP_DONE);
    osViSetEvent(&sc->interruptQ, (OSMesg) SC_MSG_RETRACE, 1);
    stubbed_printf("Crash gfx ptr = %x\n");
    *aboveB = 0;
    *belowB = 0;
    debug_trace_find_nearest((u32) dl, &prevB, &prevC, &prevA, &nextB, &nextC, &nextA);
    if (nextB != 0 || prevB != 0) {
        stubbed_printf("Surrounding traces:\n");
        if (prevB != 0) {
            stubbed_printf("Previous:\t%s:%5d gfx=%x\n");
            *belowB = prevB;
            *belowC = prevC;
            *belowA = prevA;
        }
        if (nextB != 0) {
            stubbed_printf("Next:\t\t%s:%5d gfx=%x\n");
            *aboveB = nextB;
            *aboveC = nextC;
            *aboveA = nextA;
        }
    } else {
        stubbed_printf("No traces available\n");
    }
    from = dl - 40;
    if ((u32) from < 0x80000000) {
        from = (Gfx *) ((u32) from + 0x80000000);
    }
    if ((s32) from < (s32) task->t.data_ptr) {
        from = (Gfx *) task->t.data_ptr;
    }
    func_80065F80(from, dl, 0x50);
    /*
     * The retrace handler's compiled-out messages come next in .rodata, before its own strings; they
     * are here while that function is assembly.
     */
#ifndef NON_MATCHING
    stubbed_printf("scheduler: Looks like the SP has crashed %s");
    stubbed_printf("scheduler: Looks like the DP has crashed %s");
#endif
    return dl;
}

extern u8 D_800A32D4;  // display list overflowed
extern u8 D_800A32D8;  // matrix buffer overflowed
extern u8 D_800A32DC;  // vertex buffer overflowed
extern u8 D_800A32E0;  // triangle buffer overflowed
extern s8 D_800A38C8;  // retraces until the next audio frame
extern u64 D_800A38D0; // retrace counter
extern u8 D_800A38B8;
extern s32 D_800FECB0;
extern s32 gVideoLastFramebuffer;
extern u16 *gVideoDepthBuffer;
void osDpSetStatus(u32 data);
void rsp_segment(Gfx **dList, s32 segment, s32 base);
void set_render_printf_position(u16 x, u16 state);
void render_printf(const char *format, ...);
void debug_text_print(Gfx **dList);
s32 func_80001EE8(void);


#ifdef NON_MATCHING
/* NON_MATCHING: 42 diffs: the scheduling around the volatile flag at entry and at the end, curRSPTask in v1 (copied to v0) where the ROM has v0, the retrace counter's u64 address materialised, the held task in v0/v1 swapped */
/*
 * VI retrace: a task running for more than 10 retraces has hung: reports it once (and, for a graphics task,
 * finds the command it hangs on), then resets the processor. When it found the command, shows it, the
 * buffer overflows and the version on screen and stops. Then takes the queued tasks, schedules, sends a
 * held buffer-swapping task's message after two retraces, and the retrace message to the clients.
 */
void __scHandleRetrace(OSSched *sc) {
    OSMesg msg = NULL;
    UNUSED s32 pad[2];
    SchedTask *sp = NULL;
    SchedTask *dp = NULL;
    UNUSED u8 pad8;
    volatile u8 dpHung;
    Gfx *crash[2]; // [0] DP, [1] SP: the command each hangs on
    s32 traceA[2][2]; // [0] DP, [1] SP; [0] the trace after, [1] before
    s32 traceB[2][2];
    s32 traceC[2][2];
    char *version;
    OSMesg msgBuf[8];
    OSMesgQueue mq;
    SchedTask *task;
    Gfx *gfx;
    s32 rspHung = FALSE;
    /* */

    s32 state;
    SchedTask *held;
    SchedClient *client;
    Gfx *dl;

    dpHung = FALSE;
    if (sc->curRSPTask != NULL) {
        D_800A38C0++;

    }
    if (sc->curRDPTask != NULL) {
        D_800A38C4++;
    }
    crash[1] = NULL;
    crash[0] = NULL;
    held = sc->curRSPTask;
    if (D_800A38C0 > 10 && held != NULL) {
        if (D_800FE950) {
            stubbed_printf("scheduler: Looks like the SP has crashed %s", sc_task_type_name(sc->curRSPTask->unk6C));
            sc_task_print(sc->curRSPTask);
            if (sc->curRSPTask->list.t.type == M_GFXTASK) {
                crash[1] = rcp_hang_find(sc, &traceB[1][1], &traceC[1][1], &traceA[1][1], &traceB[1][0], &traceC[1][0],
                                         &traceA[1][0]);
            }
            D_800FE950 = 0;
        }
        D_800A38C0 = 0;
        rspHung = TRUE;
        __osSpSetStatus(SP_RESET_STATUS);
    } else if (held != NULL) {
        D_800FE950 = 1;
    }
    held = sc->curRDPTask;
    if (D_800A38C4 > 10 && held != NULL) {
        if (held->unk68 == 0) {
            osSendMesg(held->msgQ, &D_800A38B8, OS_MESG_BLOCK);
        }
        if (D_800FE954) {
            stubbed_printf("scheduler: Looks like the DP has crashed %s", sc_task_type_name(sc->curRDPTask->unk6C));
            sc_task_print(sc->curRDPTask);
            if (sc->curRDPTask->list.t.type == M_GFXTASK) {
                crash[0] = rcp_hang_find(sc, &traceB[0][1], &traceC[0][1], &traceA[0][1], &traceB[0][0], &traceC[0][0],
                                         &traceA[0][0]);
            }
            D_800FE954 = 0;
        }
        dpHung = TRUE;
        sc->retraceCount = 0;
        D_800A38C4 = 0;
        __osSpSetStatus(SP_RESET_STATUS);
        osDpSetStatus(DP_RESET_STATUS);
    } else if (held != NULL) {

        D_800FE954 = 1;
    }
    if (crash[1] != NULL || crash[0] != NULL) {
        version = "9.6";

        osCreateMesgQueue(&mq, msgBuf, 8);
        osSetEventMesg(OS_EVENT_SP, &mq, (OSMesg) SC_MSG_SP_DONE);
        osSetEventMesg(OS_EVENT_DP, &mq, (OSMesg) SC_MSG_DP_DONE);
        osViSetEvent(&mq, (OSMesg) SC_MSG_RETRACE, 1);
        if (sc->curRSPTask != NULL) {
            task = sc->curRSPTask;
        } else {
            task = sc->curRDPTask;
        }
        gfx = (Gfx *) task->list.t.data_ptr;
        rsp_segment(&gfx, 0, 0);
        rsp_segment(&gfx, 1, gVideoLastFramebuffer);
        rsp_segment(&gfx, 2, (s32) gVideoDepthBuffer);
        rsp_segment(&gfx, 4, D_800FECB0 - 0x500);
        if (crash[1] != NULL) {
            set_render_printf_position(30, 30);
            render_printf("SP CRASHED, gfx=%x\n", crash[1]);
        }
        if (crash[0] != NULL) {
            set_render_printf_position(30, 70);
            render_printf("DP CRASHED, gfx=%x\n", crash[0]);
        }
        state = 110;
        if (D_800A32D4) {
            set_render_printf_position(30, 110);
            render_printf("** GFX overflow **\n");
            state = 120;
        }
        if (D_800A32D8) {
            set_render_printf_position(30, state);
            render_printf("** MTX overflow **\n");
            state += 10;
        }
        if (D_800A32DC) {
            set_render_printf_position(30, state);
            render_printf("** VTX overflow **\n");
            state += 10;
        }
        if (D_800A32E0) {
            set_render_printf_position(30, state);
            render_printf("** POL overflow **\n");
            state += 10;
        }
        dpHung = FALSE;
        dpHung = TRUE;
        crash[1] = NULL;
        crash[0] = NULL;
        set_render_printf_position(30, state + 10);
        state = 120;

        rspHung = TRUE;
        render_printf("Version %s\n", version);
        debug_text_print(&gfx);
        __osSpSetStatus(SP_RESET_STATUS);
        osDpSetStatus(DP_RESET_STATUS);
        dl = gfx++; // gDPFullSync
        dl->words.w1 = 0;
        dl->words.w0 = 0xE9000000;
        dl = gfx++; // gSPEndDisplayList
        dl->words.w1 = 0;
        dl->words.w0 = 0xB8000000;
        osWritebackDCacheAll();
        osSpTaskLoad(&task->list);
        osSpTaskStartGo(&task->list);
        while (TRUE) {}
    }
    if (rspHung) {
        sc->curRSPTask = NULL;
    }
    if (dpHung) {
        sc->curRDPTask = NULL;
    }
    while (osRecvMesg(&sc->cmdQ, &msg, OS_MESG_NOBLOCK) != -1) {
        __scAppendList(sc, (SchedTask *) msg);
    }
    state = ((sc->curRSPTask == NULL) << 1) | (sc->curRDPTask == NULL);
    if (__scSchedule(sc, &sp, &dp, state) != state) {
        __scExec(sc, sp, dp);
    }
    D_800A38D0++;
    sc->retraceCount++;
    held = sc->heldTask;
    if (held != NULL && sc->retraceCount >= 2) {
        if (held->msgQ != NULL) {
            if (held->unk68 != 0 || held->msg != NULL) {
                osSendMesg(held->msgQ, sc->heldTask->msg, OS_MESG_BLOCK);
            } else {
                osSendMesg(held->msgQ, &D_800A38B0, OS_MESG_BLOCK);
            }
        }
        sc->retraceCount = 0;
        sc->heldTask = NULL;
    }
    for (client = sc->clientList; client != NULL; client = client->next) {
        if (client->id == 1) {
            D_800A38C8--;
            if (D_800A38C8 <= 0) {
                osSendMesg(client->msgQ, (OSMesg) sc, OS_MESG_NOBLOCK);
                if (sc->audioListHead != NULL) {
                    __scHandleAudioReady(sc);
                }
                D_800A38C8 = func_80001EE8();
            }
        } else if (client->id == 2) {
            osSendMesg(client->msgQ, (OSMesg) sc, OS_MESG_NOBLOCK);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/50300", func_80050154);
#endif

/* SP done: requeues a yielded task, or completes the task, then schedules the next ones.
   (The dead `state = 0;` in the not-yielded arm gives the ROM's scheduling.) */
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
            state = 0;
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

/* Asks the running graphics task to yield (for audio), noting the time. */
void __scYield(OSSched *sc) {
    SchedTask *t = sc->curRSPTask;

    if (t->list.t.type == M_GFXTASK) {
        t->state |= SC_YIELD;
        D_800FE958 = osGetTime();
        osSpTaskYield();
    }
}

/*
 * Picks the next tasks for the free processors (availRCP: SC_NEEDS_RSP, SC_NEEDS_RDP): audio first when
 * it is due, else the next graphics task by its type; tries again while it found something.
 * Returns the processors left free. (IDO turns the tail call into a loop.)
 */
s32 __scSchedule(OSSched *sc, SchedTask **sp, SchedTask **dp, s32 availRCP) {
    s32 avail = availRCP;
    SchedTask *gfx = sc->gfxListHead;
    SchedTask *audio = sc->audioListHead;

    if (sc->doAudio && (avail & SC_NEEDS_RSP)) {
        if (gfx != NULL && (gfx->flags & SC_PARALLEL_TASK)) {
            *sp = gfx;
            avail &= ~SC_NEEDS_RSP;
        } else {
            *sp = audio;
            avail &= ~SC_NEEDS_RSP;
            sc->doAudio = FALSE;
            sc->audioListHead = sc->audioListHead->next;
            if (sc->audioListHead == NULL) {
                sc->audioListTail = NULL;
            }
        }
    } else if (__scTaskReady(gfx)) {
        switch (gfx->flags & SC_TYPE_MASK) {
            case SC_TYPE_XBUS:
                if (gfx->state & SC_YIELDED) {
                    if (avail & SC_NEEDS_RSP) {
                        *sp = gfx;
                        avail &= ~SC_NEEDS_RSP;
                        if (gfx->state & SC_NEEDS_RDP) {
                            *dp = gfx;
                            avail &= ~SC_NEEDS_RDP;
                        }
                        sc->gfxListHead = sc->gfxListHead->next;
                        if (sc->gfxListHead == NULL) {
                            sc->gfxListTail = NULL;
                        }
                    }
                } else if (avail == (SC_NEEDS_RSP | SC_NEEDS_RDP)) {
                    *dp = gfx;
                    *sp = gfx;
                    avail &= ~(SC_NEEDS_RSP | SC_NEEDS_RDP);
                    sc->gfxListHead = sc->gfxListHead->next;
                    if (sc->gfxListHead == NULL) {
                        sc->gfxListTail = NULL;
                    }
                }
                break;
            case 2:
            case 6:
            case 7:
                if ((gfx->state & SC_NEEDS_RSP) && (avail & SC_NEEDS_RSP)) {
                    *sp = gfx;
                    avail &= ~SC_NEEDS_RSP;
                }
                if ((gfx->state & SC_NEEDS_RDP) && (avail & SC_NEEDS_RDP)) {
                    *dp = gfx;
                    avail &= ~SC_NEEDS_RDP;
                    sc->gfxListHead = sc->gfxListHead->next;
                    if (sc->gfxListHead == NULL) {
                        sc->gfxListTail = NULL;
                    }
                }
                break;
            case 1:
            case 4:
            case 5:
                break;
        }
    }
    if (avail != availRCP) {
        avail = __scSchedule(sc, sp, dp, avail);
    }
    return avail;
}
