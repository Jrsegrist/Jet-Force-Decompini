/*
 * libultra's VI set-up (vi), an earlier form than ultralib's: __osViInit also
 * sets osViClock for the TV type (this object defines it) and leaves the
 * frame buffer pointers alone. Written from the assembly; built like the
 * game's other 2.0I os/io objects (-O1 -mips2).
 */
#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PR/R4300.h"
#include "PR/rcp.h"
#include "PRinternal/viint.h"

#define CLOCK_PAL 0x2F5B2D2
#define CLOCK_MPAL 0x2E6025C
#define CLOCK_NTSC 0x2E6D354

static __OSViContext vi[2] ALIGNED(0x8) = { 0 };
__OSViContext *__osViCurr = &vi[0];
__OSViContext *__osViNext = &vi[1];
u32 osViClock = CLOCK_NTSC;

void __osViInit(void) {
    bzero(vi, sizeof(vi));
    __osViCurr = &vi[0];
    __osViNext = &vi[1];
    __osViNext->retraceCount = 1;
    __osViCurr->retraceCount = 1;
    if (osTvType == OS_TV_TYPE_PAL) {
        __osViNext->modep = &osViModePalLan1;
        osViClock = CLOCK_PAL;
    } else if (osTvType == OS_TV_TYPE_MPAL) {
        __osViNext->modep = &osViModeMpalLan1;
        osViClock = CLOCK_MPAL;
    } else {
        __osViNext->modep = &osViModeNtscLan1;
        osViClock = CLOCK_NTSC;
    }
    __osViNext->state = VI_STATE_BLACK;
    __osViNext->control = __osViNext->modep->comRegs.ctrl;
    while (IO_READ(VI_CURRENT_REG) > 10) {
    }
    IO_WRITE(VI_CONTROL_REG, 0);
    __osViSwapContext();
}
