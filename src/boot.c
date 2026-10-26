/*
 * Boot (DKR main.c): the first C code to run, and the idle thread.
 *
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/main.c.
 */
#include "common.h"

extern OSThread gThread1; // OSThread for thread 1
extern OSThread gThread3; // OSThread for thread 3
extern u64 gThread1Stack[];
extern u64 gThread3Stack[];

/* Stack sizes in bytes (DKR src/stacks.h). */
#define STACK_IDLE 0x100
#define STACK_GAME 0x2000
#define STACKSIZE(x) (x / sizeof(u64))

void thread1_main(void *unused);
void thread3_main(void *unused);
void thread0_create(void);

/**
 * Where it all begins.
 * Once the boot procedure is finished in IPL3, this function is run.
 * It kicks things off by initialising thread1, which serves as the top level
 * Official name: boot
 */
void mainproc(void) {
    osInitialize();
    osCreateThread(&gThread1, 1, &thread1_main, 0, &gThread1Stack[STACKSIZE(STACK_IDLE)], OS_PRIORITY_IDLE);
    osStartThread(&gThread1);
}

/**
 * Initialise the crash handler thread, then initialise the main game thread.
 * Reset the start and endpoint of the game thread stack, then set thread priority to zero, effectively
 * stopping this thread, as it's no longer needed.
 */
void thread1_main(UNUSED void *unused) {
    thread0_create();
    osCreateThread(&gThread3, 3, &thread3_main, 0, &gThread3Stack[STACKSIZE(STACK_GAME)], 10);
    gThread3Stack[STACKSIZE(STACK_GAME)] = 0;
    gThread3Stack[0] = 0;
    osStartThread(&gThread3);
    osSetThreadPri(NULL, OS_PRIORITY_IDLE);
    while (1) {}
}

/**
 * Increments the start and endpoint of the stack.
 * JFG: no stack wraparound check follows (DKR prints a warning when they differ).
 * Official Name: bootCheckStack
 */
void thread3_verify_stack(void) {
    gThread3Stack[STACKSIZE(STACK_GAME)]++;
    gThread3Stack[0]++;
}
