#include "common.h"
#include "stdarg.h"

int vsprintf(char *s, const char *fmt, va_list args);
s32 func_80054530();

/* From the Diddy Kong Racing decompilation (CC0 1.0); see CREDITS.md. */
extern s32 sLockupDelay;
extern s32 sLockupPage;

/* DKR src/stacks.h */
#define STACK_EPC 0x1000
#define STACKSIZE(x) (x / sizeof(u64))

extern OSThread gEPCThread;
extern u64 gEPCStack[]; // just below gEPCThread
void thread0_Main(void *unused);

/**
 * Start the exception program counter thread.
 * JFG: no object stack trace to reset.
 * Official Name: diCpuTraceInit
 */
void thread0_create(void) {
    osCreateThread(&gEPCThread, 0, thread0_Main, 0, &gEPCStack[STACKSIZE(STACK_EPC)], OS_PRIORITY_MAX);
    osStartThread(&gEPCThread);
}

extern OSMesgQueue D_801024E0; // DKR: gEPCMesgQueue
extern OSMesg D_801024F8[8];   // DKR: gEPCMesgBuf
extern OSMesgQueue D_80102538; // DKR: gEPCPIQueue
extern OSMesg D_80102518[8];   // DKR: gEPCPIBuf
void stop_all_threads_except_main(void);
void func_80066E14(void);

/**
 * Main thread for the epc lockup screen. Thread 0.
 * JFG: no cheat check; shows the faulted thread instead of saving it.
 * Official Name: diCpuThread
 */
void thread0_Main(UNUSED void *unused) {
    s32 mesg;
    s32 s0 = 0;

    osCreateMesgQueue(&D_801024E0, D_801024F8, ARRAY_COUNT(D_801024F8));
    osSetEventMesg(OS_EVENT_FAULT, &D_801024E0, (OSMesg) 8);     // RMON_MESG_FAULT
    osSetEventMesg(OS_EVENT_CPU_BREAK, &D_801024E0, (OSMesg) 2); // RMON_MESG_CPU_BREAK
    osCreatePiManager(150, &D_80102538, D_80102518, ARRAY_COUNT(D_80102518));

    while (1) {
        osRecvMesg(&D_801024E0, (OSMesg *) &mesg, OS_MESG_BLOCK);
        s0 |= mesg;
        if (!(s0 & 8) && !(s0 & 2)) {
            continue;
        }
        s0 &= ~8;
        stop_all_threads_except_main();
        func_80066E14();
    }
}

/**
 * Stop all threads except for the main thread 1
 */
void stop_all_threads_except_main(void) {
    OSThread *node = __osGetActiveQueue();
    while (node->priority != -1) {
        if (node->priority > OS_PRIORITY_IDLE && node->priority <= OS_PRIORITY_APPMAX) {
            osStopThread((OSThread *) &node->next);
        }
        node = node->tlnext;
    }
}

INCLUDE_ASM("asm/nonmatchings/67780", func_80066D28);

void func_80066D28(OSThread *thread);
void func_80066EB0(OSThread *thread);

/* JFG: find the first faulted thread (or the end of the queue) and show it. */
void func_80066E14(void) {
    OSThread *thread;

    for (thread = __osGetActiveQueue(); thread->priority != -1; thread = thread->tlnext) {
        if (thread->priority > OS_PRIORITY_IDLE) {
            if (thread->flags & 2 || thread->flags & 1) {
                break;
            }
        }
    }
    if (thread->priority != -1) {
        func_80066D28(thread);
    }
    func_80066EB0(thread);
}

INCLUDE_ASM("asm/nonmatchings/67780", func_80066EB0);

void func_8006837C(void);
void func_800682F0(s32 arg0, s32 x, const char *format, ...);
s32 func_80054368(s32 address, s32 *module, s32 *offset, s32 arg3);
extern char D_800AE8FC[];
extern char D_800AE918[];

/* JFG: clear the screen and print the address (and its module and offset), then hang. */
void func_80067610(s32 address) {
    s32 offset;
    s32 module;
    s32 i;

    for (i = 0; i < 100; i++) {
        func_8006837C();
    }
    func_800682F0(30, 80, D_800AE8FC, address);
    if (func_80054368(address, &module, &offset, 0)) {
        func_800682F0(30, 100, D_800AE918, module, offset);
    }
    while (1) {
    }
}

extern s32 D_800A64D0;
extern s32 D_800A64D4;
extern s32 D_800A64D8;

/* JFG: log a line (up to 40 characters) into the 500-line ring at 0x80700000. */
void func_800676B4(const char *format, ...) {
    va_list args;
    char *dst;
    char *src;
    s32 i;
    char buffer[0x100];

    va_start(args, format);
    vsprintf(buffer, format, args);
    dst = (char *) 0x80700000 + D_800A64D4 * 41;
    src = buffer;
    i = 0;
    while (*src != '\0' && i < 40) {
        *dst++ = *src++;
        i++;
    }
    *dst = '\0';
    D_800A64D4++;
    if (D_800A64D4 >= 500) {
        D_800A64D8 = 1;
        D_800A64D4 = 0;
    }
    if (D_800A64D8) {
        D_800A64D0 = D_800A64D4;
    }
}

INCLUDE_ASM("asm/nonmatchings/67780", func_80067790);

/* JFG: a fault thread record, as big as the one __rmonSendFault sends. */
typedef struct FaultThread {
    OSThread thread;
    u8 pad1B0[0x80];
} FaultThread;
void func_80067994(FaultThread *fault);
#define S_P fault.thread.context.pc = pc;
#define S_A fault.thread.context.a0 = a0;
#define S_B fault.thread.context.a1 = a1;
#define S_C fault.thread.context.cause = -1;

INCLUDE_ASM("asm/nonmatchings/67780", func_800678C4);

s32 func_80067950(void) {
    return 0;
}

/**
 * Counts up. Automatically switches page every second.
 * Official name: diCpuTraceTick
 */
void mode_lockup(s32 updateRate) {
    sLockupDelay += updateRate;
    if (sLockupDelay > 60) {
        sLockupDelay = 0;
        sLockupPage++;
    }
}

INCLUDE_ASM("asm/nonmatchings/67780", func_80067994);

INCLUDE_ASM("asm/nonmatchings/67780", func_800680B0);

INCLUDE_ASM("asm/nonmatchings/67780", func_800681D0);

void func_800681D0(s32 arg0, s32 x, char *text);
extern s32 D_800A651C;

/* JFG: format a line and print it at x (shifted for the screen mode). */
void func_800682F0(s32 arg0, s32 x, const char *format, ...) {
    va_list args;
    char buffer[0x100];

    va_start(args, format);
    vsprintf(buffer, format, args);
    if (D_800A651C != 0) {
        if (D_800A651C == 1) {
            x -= 8;
        } else {
            x -= 0x68;
        }
        if (x < 0 || x >= 0x74) {
            return;
        }
        x *= 2;
    }
    func_800681D0(arg0, x, buffer);
}

void fb_size(s32 *width, s32 *height);
extern u16 *D_800FECB8;

/* JFG: clear the framebuffer to show next. */
void func_8006837C(void) {
    u16 *pixel;
    s32 width, height;
    s32 count;

    fb_size(&width, &height);
    count = width * height;
    pixel = D_800FECB8;
    while (count--) {
        *pixel++ = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/67780", func_800683D4);
