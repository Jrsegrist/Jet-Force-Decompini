/*
 * osCreateThread in the game's form: new threads run with SR_FR set (the
 * FPU's 32 64-bit registers); a negative id clears it again and the thread
 * gets the positive id. Written from the assembly; built with the game's
 * flags (-O2 -mips1).
 */
#include "PR/os_internal.h"
#include "PR/R4300.h"
#include "PRinternal/osint.h"

void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri p) {
    register u32 saveMask;
    OSIntMask mask;

    t->id = id;
    t->priority = p;
    t->next = NULL;
    t->queue = NULL;
    t->context.pc = (u32) entry;
    t->context.a0 = (s64) (s32) arg;
    t->context.sp = (s64) (s32) sp - 16;
    t->context.ra = (s64) (s32) __osCleanupThread;
    mask = OS_IM_ALL;
    t->context.sr = (mask & (SR_IMASK | SR_IE)) | SR_EXL | SR_FR;
    t->context.rcp = (mask & RCP_IMASK) >> RCP_IMASKSHIFT;
    t->context.fpcsr = FPCSR_FS | FPCSR_EV | FPCSR_RM_RN;
    t->fp = 0;
    t->state = OS_STATE_STOPPED;
    t->flags = 0;
    if (id < 0) {
        t->id = -id;
        t->context.sr &= ~SR_FR;
    }
    saveMask = __osDisableInt();
    t->tlnext = __osActiveQueue;
    __osActiveQueue = t;
    __osRestoreInt(saveMask);
}
