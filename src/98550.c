/*
 * libultra's controller set-up (controller), an earlier form than ultralib's:
 * osContInit works out the 0.5 s start-up wait from osClockRate at run time,
 * and the PIF RAM is cleared as 16 words (pifstatus included). Written from
 * the assembly; built like the game's other 2.0I os/io objects (-O1 -mips2).
 */
#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

#define PIFRAM_WORDS(ram) ((u32 *) &(ram))
#define USEC_TO_CYCLES(n) (((u64) (n) * osClockRate) / 1000000)

OSPifRam __osContPifRam;
u8 __osContLastCmd;
u8 __osMaxControllers;

OSTimer __osEepromTimer;
OSMesgQueue __osEepromTimerQ ALIGNED(0x8);
OSMesg __osEepromTimerMsg;

s32 __osContinitialized = FALSE;

s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data) {
    OSMesg dummy;
    s32 ret = 0;
    OSTime t;
    OSTimer mytimer;
    OSMesgQueue timerMesgQueue;

    if (__osContinitialized) {
        return 0;
    }
    __osContinitialized = TRUE;
    t = osGetTime();
    if (t < USEC_TO_CYCLES(500000)) {
        osCreateMesgQueue(&timerMesgQueue, &dummy, 1);
        osSetTimer(&mytimer, USEC_TO_CYCLES(500000) - t, 0, &timerMesgQueue, &dummy);
        osRecvMesg(&timerMesgQueue, &dummy, OS_MESG_BLOCK);
    }
    __osMaxControllers = MAXCONTROLLERS;
    __osPackRequestData(CONT_CMD_REQUEST_STATUS);
    ret = __osSiRawStartDma(OS_WRITE, &__osContPifRam);
    osRecvMesg(mq, &dummy, OS_MESG_BLOCK);
    ret = __osSiRawStartDma(OS_READ, &__osContPifRam);
    osRecvMesg(mq, &dummy, OS_MESG_BLOCK);
    __osContGetInitData(bitpattern, data);
    __osContLastCmd = CONT_CMD_REQUEST_STATUS;
    __osSiCreateAccessQueue();
    osCreateMesgQueue(&__osEepromTimerQ, &__osEepromTimerMsg, 1);
    return ret;
}

void __osContGetInitData(u8 *pattern, OSContStatus *data) {
    u8 *ptr;
    __OSContRequesFormat requestHeader;
    int i;
    u8 bits = 0;

    ptr = (u8 *) &__osContPifRam;
    for (i = 0; i < __osMaxControllers; i++, ptr += sizeof(requestHeader), data++) {
        requestHeader = *(__OSContRequesFormat *) ptr;
        data->errno = CHNL_ERR(requestHeader);
        if (data->errno != 0) {
            continue;
        }
        data->type = requestHeader.typel << 8 | requestHeader.typeh;
        data->status = requestHeader.status;
        bits |= 1 << i;
    }
    *pattern = bits;
}

void __osPackRequestData(u8 cmd) {
    u8 *ptr;
    __OSContRequesFormat requestHeader;
    s32 i;

    for (i = 0; i < 16; i++) {
        PIFRAM_WORDS(__osContPifRam)[i] = 0;
    }
    __osContPifRam.pifstatus = CONT_CMD_EXE;
    ptr = (u8 *) &__osContPifRam;
    requestHeader.dummy = CONT_CMD_NOP;
    requestHeader.txsize = CONT_CMD_RESET_TX;
    requestHeader.rxsize = CONT_CMD_RESET_RX;
    requestHeader.cmd = cmd;
    requestHeader.typeh = CONT_CMD_NOP;
    requestHeader.typel = CONT_CMD_NOP;
    requestHeader.status = CONT_CMD_NOP;
    requestHeader.dummy1 = CONT_CMD_NOP;
    for (i = 0; i < __osMaxControllers; i++) {
        *(__OSContRequesFormat *) ptr = requestHeader;
        ptr += sizeof(requestHeader);
    }
    *ptr = CONT_CMD_END;
}
