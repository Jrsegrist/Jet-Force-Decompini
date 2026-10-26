#include "common.h"

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

INCLUDE_ASM("asm/nonmatchings/67780", func_80066BD4);

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

INCLUDE_ASM("asm/nonmatchings/67780", func_80066E14);

INCLUDE_ASM("asm/nonmatchings/67780", func_80066EB0);

INCLUDE_ASM("asm/nonmatchings/67780", func_80067610);

INCLUDE_ASM("asm/nonmatchings/67780", func_800676B4);

INCLUDE_ASM("asm/nonmatchings/67780", func_80067790);

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

INCLUDE_ASM("asm/nonmatchings/67780", func_800682F0);

INCLUDE_ASM("asm/nonmatchings/67780", func_8006837C);

INCLUDE_ASM("asm/nonmatchings/67780", func_800683D4);
