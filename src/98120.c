/*
 * libultra's osInitialize. The code is the SDK 2.0J form, but this game's
 * object does not define osViClock (its older VI object, src/9B780.c, does),
 * so ultralib's file cannot be included. Written from the assembly; built
 * -O2 -mips2.
 */
#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PR/rcp.h"
#include "PR/R4300.h"
#include "PRinternal/piint.h"

typedef struct {
    u32 words[4];
} ExceptionVector;

extern ExceptionVector __osExceptionPreamble[];
extern OSPiHandle __Dom1SpeedParam;
extern OSPiHandle __Dom2SpeedParam;
extern u32 osViClock;

OSTime osClockRate = OS_CLOCK_RATE;
u32 __osShutdown = 0;
u32 __OSGlobalIntMask = OS_IM_ALL;
u32 __osFinalrom;

static void createSpeedParam(void);

void osInitialize(void) {
    u32 pifdata;
    UNUSED u32 pad;

    __osFinalrom = TRUE;
    __osSetSR(__osGetSR() | SR_CU1);
    __osSetFpcCsr(FPCSR_FS | FPCSR_EV | FPCSR_RM_RN);
    while (__osSiRawReadIo(PIF_RAM_END - 3, &pifdata)) {
    }
    while (__osSiRawWriteIo(PIF_RAM_END - 3, pifdata | 8)) {
    }
    /* install the exception handler at the four vectors */
    *(ExceptionVector *) UT_VEC = *__osExceptionPreamble;
    *(ExceptionVector *) XUT_VEC = *__osExceptionPreamble;
    *(ExceptionVector *) ECC_VEC = *__osExceptionPreamble;
    *(ExceptionVector *) E_VEC = *__osExceptionPreamble;
    osWritebackDCache((void *) UT_VEC, E_VEC - UT_VEC + sizeof(ExceptionVector));
    osInvalICache((void *) UT_VEC, E_VEC - UT_VEC + sizeof(ExceptionVector));
    createSpeedParam();
    osUnmapTLBAll();
    osMapTLBRdb();
    osClockRate = osClockRate * 3 / 4;
    if (osResetType == 0) {
        bzero(osAppNMIBuffer, OS_APP_NMI_BUFSIZE);
    }
    if (osTvType == OS_TV_PAL) {
        osViClock = VI_PAL_CLOCK;
    } else if (osTvType == OS_TV_MPAL) {
        osViClock = VI_MPAL_CLOCK;
    } else {
        osViClock = VI_NTSC_CLOCK;
    }
    /* hang if an RCP interrupt is pending */
    if (__osGetCause() & CAUSE_IP5) {
        while (TRUE) {
        }
    }
    IO_WRITE(AI_CONTROL_REG, AI_CONTROL_DMA_ON);
    IO_WRITE(AI_DACRATE_REG, AI_MAX_DAC_RATE - 1);
    IO_WRITE(AI_BITRATE_REG, AI_MAX_BIT_RATE - 1);
}

/* Record the cartridge domains' PI timings as the boot code left them. */
static void createSpeedParam(void) {
    __Dom1SpeedParam.type = DEVICE_TYPE_INIT;
    __Dom1SpeedParam.latency = IO_READ(PI_BSD_DOM1_LAT_REG);
    __Dom1SpeedParam.pulse = IO_READ(PI_BSD_DOM1_PWD_REG);
    __Dom1SpeedParam.pageSize = IO_READ(PI_BSD_DOM1_PGS_REG);
    __Dom1SpeedParam.relDuration = IO_READ(PI_BSD_DOM1_RLS_REG);
    __Dom2SpeedParam.type = DEVICE_TYPE_INIT;
    __Dom2SpeedParam.latency = IO_READ(PI_BSD_DOM2_LAT_REG);
    __Dom2SpeedParam.pulse = IO_READ(PI_BSD_DOM2_PWD_REG);
    __Dom2SpeedParam.pageSize = IO_READ(PI_BSD_DOM2_PGS_REG);
    __Dom2SpeedParam.relDuration = IO_READ(PI_BSD_DOM2_RLS_REG);
}
