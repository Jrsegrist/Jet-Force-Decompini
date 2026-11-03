#include "common.h"
#include "stdarg.h"

int vsprintf(char *s, const char *fmt, va_list args);

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

extern OSMesgQueue gEPCMesgQueue;
extern OSMesg gEPCMesgBuf[8];
extern OSMesgQueue gEPCPIQueue;
extern OSMesg gEPCPIBuf[8];
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

    osCreateMesgQueue(&gEPCMesgQueue, gEPCMesgBuf, ARRAY_COUNT(gEPCMesgBuf));
    osSetEventMesg(OS_EVENT_FAULT, &gEPCMesgQueue, (OSMesg) 8);     // RMON_MESG_FAULT
    osSetEventMesg(OS_EVENT_CPU_BREAK, &gEPCMesgQueue, (OSMesg) 2); // RMON_MESG_CPU_BREAK
    osCreatePiManager(150, &gEPCPIQueue, gEPCPIBuf, ARRAY_COUNT(gEPCPIBuf));

    while (1) {
        osRecvMesg(&gEPCMesgQueue, (OSMesg *) &mesg, OS_MESG_BLOCK);
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

extern s32 gLogStart;
extern s32 gLogLine;
extern s32 gLogWrapped;
extern void *D_80102554;
extern u8 D_80102558[0x200];
extern char D_800AE850[];
extern char D_800AE858[];
s32 write_controller_pak_file(s32 controllerIndex, s32 fileNumber, char *fileName, char *fileExt, u8 *dataToWrite,
                              s32 fileSize);

#ifdef NON_MATCHING
/* NON_MATCHING: one register: the 0x80700000 argument is built in t4 (target t6) */
/* JFG: save the log, the faulted thread and its stack (at 0x80700000) to the controller pak. */
void func_80066D28(OSThread *thread) {
    u8 *dst;
    void *stack;
    s32 size;

    *(s32 *) 0x80705014 = gLogWrapped;
    *(s32 *) 0x80705018 = gLogStart;
    *(s32 *) 0x8070501C = gLogLine;
    dst = (u8 *) 0x80705094;
    bcopy(thread, dst, 0x230);
    dst += 0x200;
    stack = *(void **) ((u8 *) thread + 0xF4); // JFG: the thread's sp
    bcopy(stack, dst, 0x200);
    D_80102554 = stack;
    bcopy(stack, D_80102558, 0x200);
    dst += 0x200;
    size = dst - (u8 *) 0x80700000;
    if (size & 0x1F) {
        size = (size & ~0x1F) + 0x20;
    }
    write_controller_pak_file(0, -1, D_800AE850, D_800AE858, (u8 *) 0x80700000, size);
}
#else
INCLUDE_ASM("asm/nonmatchings/67780", func_80066D28);
#endif

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

void epc_clear_framebuffer(void);
void epc_printf(s32 arg0, s32 x, const char *format, ...);
s32 func_80054368(s32 address, s32 *module, s32 *offset, s32 arg3);
extern char D_800AE8FC[];
extern char D_800AE918[];

/* JFG: clear the screen and print the address (and its module and offset), then hang (0x80067610). */
void lockup_show_address(s32 address) {
    s32 offset;
    s32 module;
    s32 i;

    for (i = 0; i < 100; i++) {
        epc_clear_framebuffer();
    }
    epc_printf(30, 80, D_800AE8FC, address);
    if (func_80054368(address, &module, &offset, 0)) {
        epc_printf(30, 100, D_800AE918, module, offset);
    }
    while (1) {
    }
}

extern s32 gLogStart;
extern s32 gLogLine;
extern s32 gLogWrapped;

/* JFG: log a line (up to 40 characters) into the 500-line ring at 0x80700000 (0x800676B4). */
void log_printf(const char *format, ...) {
    va_list args;
    char *dst;
    char *src;
    s32 i;
    char buffer[0x100];

    va_start(args, format);
    vsprintf(buffer, format, args);
    dst = (char *) 0x80700000 + gLogLine * 41;
    src = buffer;
    i = 0;
    while (*src != '\0' && i < 40) {
        *dst++ = *src++;
        i++;
    }
    *dst = '\0';
    gLogLine++;
    if (gLogLine >= 500) {
        gLogWrapped = 1;
        gLogLine = 0;
    }
    if (gLogWrapped) {
        gLogStart = gLogLine;
    }
}

extern char D_800AE92C[];

/* JFG: clear the screen and list up to 20 log lines, from line start after the oldest (0x80067790). */
void log_show(s32 start) {
    s32 y;
    s32 done = FALSE;
    s32 i;
    s32 line;

    epc_clear_framebuffer();
    line = start + gLogStart;
    if (line >= 500) {
        line -= 500;
    }
    i = 0;
    y = 30;
    do { // JFG: 20 lines at most, 8 pixels apart
        if (!gLogWrapped && line >= gLogLine) {
            done = TRUE;
        }
        if (!done) {
            epc_printf(20, y, D_800AE92C, i + start, line, (char *) 0x80700000 + line * 41);
            line++;
            if (line >= 500) {
                line -= 500;
            }
            y += 8;
        }
        i++;
    } while (i < 20 && !done);
}

/* JFG: a fault thread record (0x230 bytes, the size bcopied in func_80066D28). */
typedef struct FaultThread {
    OSThread thread;
    u8 pad1B0[0x80];
} FaultThread;
void func_80067994(FaultThread *fault);

/* JFG: show the crash screen for a fake thread (pc, a0, a1) and hang.
   (The (u64) cast on pc, a no-op for the u32 field, gives the ROM's load order; found by the permuter.) */
void func_800678C4(u32 pc, s32 a0, u32 a1) {
    FaultThread fault;

    bzero(&fault, sizeof(fault));
    fault.thread.context.pc = (u64) pc;
    fault.thread.context.a0 = a0;
    fault.thread.context.a1 = a1;
    fault.thread.context.cause = -1;
    func_80067994(&fault);
    osWritebackDCacheAll();
    while (1) {
    }
}

/* Returns whether a crash record is waiting (always FALSE in JFG) (0x80067950). */
s32 get_lockup_status(void) {
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

void fb_size(s32 *width, s32 *height);
extern u16 *gVideoLastFramebuffer;
extern s32 D_800A651C; // JFG: the screen mode (nonzero: draw each glyph row twice)
extern s32 D_800A6520; // JFG: the text colour set
extern u16 D_800A6768[4];
extern u16 D_800A6770[4];
extern u16 D_800A6778[4];

/* JFG: draw a 5-row glyph (2 bits per pixel, 4 colours) straight into the framebuffer at (x, y). */
void epc_draw_glyph(s32 x, s32 y, u16 *glyph) {
    u16 *pixel;
    u16 *colours;
    u16 *dst;
    u16 bits;
    s32 i = 5;
    s32 rep;
    s32 width, height;

    fb_size(&width, &height);
    pixel = &gVideoLastFramebuffer[y * width + x];
    if (D_800A6520 == 2) {
        colours = D_800A6778;
    } else if (D_800A6520 != 0) {
        colours = D_800A6770;
    } else {
        colours = D_800A6768;
    }
    while (i--) {
        rep = 1;
        if (D_800A651C) {
            rep = 2;
        }
        while (rep--) {
            dst = pixel;
            bits = *glyph;
            while (bits) {
                *dst++ = colours[bits & 3];
                bits >>= 2;
            }
            pixel += width;
        }
        glyph++;
    }
}

extern u16 D_800A6524[]; // JFG: the font: 5 rows per character, from '!'

/* JFG: draw a string with the 8-pixel font (upper case only; newline, tab and space move the cursor). */
void epc_draw_string(s32 x, s32 y, char *text) {
    u8 ch;

    while (*text != '\0') {
        ch = *text++;
        if (ch >= 'a' && ch <= 'z') {
            ch -= 0x20;
        }
        if (ch == '\n') {
            y += 6;
            x = 32;
        } else if (ch == '\t') {
            x = x - (x & 0xF) + 16;
        } else if (ch == ' ') {
            x += 4;
        } else if (ch > ' ' && ch <= 'Z') {
            epc_draw_glyph(x, y, &D_800A6524[(ch - '!') * 5]);
            x += 8;
        }
    }
}

void epc_draw_string(s32 arg0, s32 x, char *text);
extern s32 D_800A651C;

/* JFG: format a line and print it at x (shifted for the screen mode) (0x800682F0). */
void epc_printf(s32 arg0, s32 x, const char *format, ...) {
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
    epc_draw_string(arg0, x, buffer);
}

void fb_size(s32 *width, s32 *height);
extern u16 *gVideoLastFramebuffer;

/* JFG: clear the framebuffer to show next (0x8006837C). */
void epc_clear_framebuffer(void) {
    u16 *pixel;
    s32 width, height;
    s32 count;

    fb_size(&width, &height);
    count = width * height;
    pixel = gVideoLastFramebuffer;
    while (count--) {
        *pixel++ = 0;
    }
}

s32 func_80054530(); // a module function (patched in by the runtime linker)

/* JFG: send a fault thread record (0x230 bytes) to the debugger, in as many pieces as it takes (packet type 2). */
void fault_send_thread(u8 *fault) {
    volatile f32 f;
    u32 sent = 0;

    f = 0.0f; // touch the FPU so its registers are saved in the thread context
    while (sent < 0x230) {
        sent += func_80054530(fault + sent, 0x230 - sent, 2);
    }
}
