#include "common.h"
#include "stdarg.h"

int vsprintf(char *s, const char *fmt, va_list args);

/*
 * The strings that 67780's .data points at (a version and the exception names, still asm .data at
 * 0x800A64C0 and 0x800A64DC): they open the file's .rodata, so they are defined here, by address.
 */
const char D_800AE750[] = "9.6";
const char D_800AE754[] = "interrupt";
const char D_800AE760[] = "TLB modify";
const char D_800AE76C[] = "TLB load or fetch";
const char D_800AE780[] = "TLB store";
const char D_800AE78C[] = "addr load or fetch";
const char D_800AE7A0[] = "addr store";
const char D_800AE7AC[] = "bus error fetch";
const char D_800AE7BC[] = "bus error reference";
const char D_800AE7D0[] = "syscall";
const char D_800AE7D8[] = "breakpoint";
const char D_800AE7E4[] = "reserved instruction";
const char D_800AE7FC[] = "co-pro unusable";
const char D_800AE80C[] = "overflow";
const char D_800AE818[] = "trap";
const char D_800AE820[] = "virtual coherency";
const char D_800AE834[] = "floating point";

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
void crash_find_faulted_thread(void);

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
        stubbed_printf(">fault< ");
        stop_all_threads_except_main();
        crash_find_faulted_thread();
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
    write_controller_pak_file(0, -1, "CORE", "", (u8 *) 0x80700000, size);
}
#else
INCLUDE_ASM("asm/nonmatchings/67780", func_80066D28);
#endif

void func_80066D28(OSThread *thread);
void func_80066EB0(OSThread *thread);

/* JFG: find the first faulted thread (or the end of the queue) and show it. */
void crash_find_faulted_thread(void) {
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
s32 func_80054368(u32 address, s32 *module, s32 *offset, char *name);

/* JFG: clear the screen and print the address (and its module and offset), then hang (0x80067610). */
void lockup_show_address(s32 address) {
    s32 offset;
    s32 module;
    s32 i;

    for (i = 0; i < 100; i++) {
        epc_clear_framebuffer();
    }
    epc_printf(30, 80, "Watchpoint exception at %x\n", address);
    if (func_80054368(address, &module, &offset, NULL)) {
        epc_printf(30, 100, "Module %d at %08x", module, offset);
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
            epc_printf(20, y, "%04d: %04d: %s", i + start, line, (char *) 0x80700000 + line * 41);
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
void crash_screen_draw(FaultThread *fault);

/* JFG: show the crash screen for a fake thread (pc, a0, a1) and hang.
   (The (u64) cast on pc, a no-op for the u32 field, gives the ROM's load order; found by the permuter.) */
void crash_show_fake_thread(u32 pc, s32 a0, u32 a1) {
    FaultThread fault;

    bzero(&fault, sizeof(fault));
    fault.thread.context.pc = (u64) pc;
    fault.thread.context.a0 = a0;
    fault.thread.context.a1 = a1;
    fault.thread.context.cause = -1;
    crash_screen_draw(&fault);
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

/* JFG: the debug "current item" the crash screen reports (kind 1 to 3, an index into a name table). */
typedef struct CrashItem {
    /* 0x00 */ u8 pad0[0x48];
    /* 0x48 */ s16 id;
} CrashItem;

extern s32 D_800A64C0;
extern char *D_800A64DC[16]; // JFG: the exception names, by cause code
extern s32 D_800A333C;
extern CrashItem *D_800A3340;
extern s32 D_800A3344;
extern s32 D_800A3348;
extern char *D_800A334C[];
extern s32 gMainPoolFreeBytes;
extern u8 D_80102550; // JFG: flags shown at the bottom of the crash screen
u8 *level_header(void);
u8 *get_racer_object_by_port(s32 port);

/**
 * JFG: the crash screen: the thread, the pc and ra (with their module and offset when they are in
 * one), the exception, the debug item being handled, the free memory, the registers, then the
 * level header, player 1 and the debug flags.
 */
void crash_screen_draw(FaultThread *fault) {
    __OSThreadContext *ctx;
    s32 module;
    s32 offset;
    char name[16];
    u8 *ptr;

    epc_clear_framebuffer();
    ctx = &fault->thread.context;
    epc_printf(0x20, 0x18, "Fault in thread %d   (%s)", fault->thread.id, D_800A64C0);
    if (ctx->pc == 0) {
        epc_printf(0x20, 0x22, "epc NULL");
    } else if (func_80054368(ctx->pc, &module, &offset, name)) {
        epc_printf(0x20, 0x22, "epc %d,%x", module, offset);
    } else {
        epc_printf(0x20, 0x22, "epc %08x,?,?,?", ctx->pc);
    }
    if ((u32) ctx->ra == 0) {
        epc_printf(0x20, 0x28, "ra   NULL");
    } else if (func_80054368((u32) ctx->ra, &module, &offset, name)) {
        epc_printf(0x20, 0x28, "ra   %d,%x", module, offset);
    } else {
        epc_printf(0x20, 0x28, "ra   %08x,?,?,?", (u32) ctx->ra);
    }
    if (ctx->cause == -1) {
        epc_printf(0x20, 0x2E, " cause\t\tmmAlloc(%d,%8x)\n", (u32) ctx->a0, (u32) ctx->a1);
    } else {
        if (((ctx->cause >> 2) & 0x1F) < 16) {
            epc_printf(0x20, 0x2E, "%s exception", D_800A64DC[(ctx->cause >> 2) & 0x1F]);
        } else {
            epc_printf(0x20, 0x2E, "cause\t\t\t%08x", ctx->cause);
        }
    }
    if (D_800A3348 != 4 && D_800A333C != 0) {
        switch (D_800A3348) {
            case 1:
                if (D_800A3344 != 0) {
                    epc_printf(0x20, 0x34, "crashed %s %s", D_800A334C[D_800A333C], D_800A3344);
                } else if (D_800A3340 != NULL) {
                    epc_printf(0x20, 0x34, "crashed %s type %d", D_800A334C[D_800A333C], D_800A3340->id);
                } else {
                    epc_printf(0x20, 0x34, "crashed %s unknown", D_800A334C[D_800A333C]);
                }
                break;
            case 3:
                if (D_800A3344 != 0) {
                    epc_printf(0x20, 0x34, "error %s %s", D_800A334C[D_800A333C], D_800A3344);
                } else if (D_800A3340 != NULL) {
                    epc_printf(0x20, 0x34, "error %s type %d", D_800A334C[D_800A333C], D_800A3340->id);
                } else {
                    epc_printf(0x20, 0x34, "error %s unknown", D_800A334C[D_800A333C]);
                }
                break;
            case 2:
                if (D_800A3344 != 0) {
                    epc_printf(0x20, 0x34, "aborted %s %s", D_800A334C[D_800A333C], D_800A3344);
                } else if (D_800A3340 != NULL) {
                    epc_printf(0x20, 0x34, "aborted %s type %d", D_800A334C[D_800A333C], D_800A3340->id);
                } else {
                    epc_printf(0x20, 0x34, "aborted %s unknown", D_800A334C[D_800A333C]);
                }
                break;
        }
    } else {
        epc_printf(0x20, 0x34, "badvaddr\t%08x", ctx->badvaddr);
    }
    epc_printf(0x20, 0x3A, "free ram\t\t%08x", gMainPoolFreeBytes);
    epc_printf(0x20, 0x44, "at 0x%08x v0 0x%08x", (u32) ctx->at, (u32) ctx->v0);
    epc_printf(0x20, 0x4A, "v1 0x%08x a0 0x%08x", (u32) ctx->v1, (u32) ctx->a0);
    epc_printf(0x20, 0x50, "a1 0x%08x a2 0x%08x", (u32) ctx->a1, (u32) ctx->a2);
    epc_printf(0x20, 0x56, "a3 0x%08x t0 0x%08x", (u32) ctx->a3, (u32) ctx->t0);
    epc_printf(0x20, 0x5C, "t1 0x%08x t2 0x%08x", (u32) ctx->t1, (u32) ctx->t2);
    epc_printf(0x20, 0x62, "t3 0x%08x t4 0x%08x", (u32) ctx->t3, (u32) ctx->t4);
    epc_printf(0x20, 0x68, "t5 0x%08x t6 0x%08x", (u32) ctx->t5, (u32) ctx->t6);
    epc_printf(0x20, 0x6E, "t7 0x%08x s0 0x%08x", (u32) ctx->t7, (u32) ctx->s0);
    epc_printf(0x20, 0x74, "s1 0x%08x s2 0x%08x", (u32) ctx->s1, (u32) ctx->s2);
    epc_printf(0x20, 0x7A, "s3 0x%08x s4 0x%08x", (u32) ctx->s3, (u32) ctx->s4);
    epc_printf(0x20, 0x80, "s5 0x%08x s6 0x%08x", (u32) ctx->s5, (u32) ctx->s6);
    epc_printf(0x20, 0x86, "s7 0x%08x t8 0x%08x", (u32) ctx->s7, (u32) ctx->t8);
    epc_printf(0x20, 0x8C, "t9 0x%08x gp 0x%08x", (u32) ctx->t9, (u32) ctx->gp);
    epc_printf(0x20, 0x92, "sp 0x%08x s8 0x%08x", (u32) ctx->sp, (u32) ctx->s8);
    epc_printf(0x20, 0x98, "sr 0x%08x", ctx->sr);
    ptr = level_header();
    if (ptr != NULL && ((u32) ptr & 0x80000000)) {
        epc_printf(0x20, 0xA4, "Level is\t\t\t%s", ptr);
    }
    ptr = get_racer_object_by_port(0);
    if (ptr != NULL && ((u32) ptr & 0x80000000)) {
        epc_printf(0x20, 0xAA, "Position is\t%.1f, %.1f, %.1f", ptr + 0xC, ptr + 0x10, ptr + 0x14);
    }
    if (D_80102550 & 1) {
        epc_printf(0x20, 0xB0, "Track polygon buffer overflow");
    }
    if (D_80102550 & 2) {
        epc_printf(0x20, 0xB6, "Track edge buffer overflow");
    }
    if (D_80102550 & 4) {
        epc_printf(0x20, 0xBC, "Object polygon buffer overflow");
    }
    if (D_80102550 & 8) {
        epc_printf(0x20, 0xC2, "Object edge buffer overflow");
    }
}

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
