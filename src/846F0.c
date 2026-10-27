#include "common.h"
#include "PR/os_internal_flash.h"

/*
 * The libultra flash library (FlashRAM saves), built with the game's flags as
 * one object. Five functions are ultralib's and are included from its sources
 * (one with another cache call); the other five are an earlier form (no timer
 * between status polls; the word ultralib calls __osFlashVersion holds the
 * page-to-address multiplier, 0x40 or 0x80, instead of a version flag),
 * written here from the assembly.
 */

extern u32 D_80104F58; /* page number to flash address multiplier */

#include "../lib/ultralib/src/flash/flashreinit.c"

OSPiHandle *osFlashInit(void) {
    u32 flashType;
    u32 flashMaker;

    osCreateMesgQueue(&__osFlashMessageQ, __osFlashMsgBuf, 1);
    if (__osFlashHandler.baseAddress == PHYS_TO_K1(FLASH_START_ADDR)) {
        return &__osFlashHandler;
    }
    __osFlashHandler.type = DEVICE_TYPE_FLASH;
    __osFlashHandler.baseAddress = PHYS_TO_K1(FLASH_START_ADDR);
    __osFlashHandler.latency = FLASH_LATENCY;
    __osFlashHandler.pulse = FLASH_PULSE;
    __osFlashHandler.pageSize = FLASH_PAGE_SIZE;
    __osFlashHandler.relDuration = FLASH_REL_DURATION;
    __osFlashHandler.domain = PI_DOMAIN2;
    __osFlashHandler.speed = 0;
    bzero(&__osFlashHandler.transferInfo, sizeof(__OSTranxInfo));
    osEPiLinkHandle(&__osFlashHandler);
    osFlashReadId(&flashType, &flashMaker);
    if (flashMaker == FLASH_VERSION_MX_C || flashMaker == FLASH_VERSION_MX_A ||
        flashMaker == FLASH_VERSION_MX_PROTO_A) {
        D_80104F58 = 0x40;
    } else {
        D_80104F58 = 0x80;
    }
    return &__osFlashHandler;
}

#include "../lib/ultralib/src/flash/flashreadstatus.c"

/* This SDK's osFlashReadId writes the ID buffer back before the DMA instead of invalidating it. */
#define osInvalDCache osWritebackDCache
#include "../lib/ultralib/src/flash/flashreadid.c"
#undef osInvalDCache

#include "../lib/ultralib/src/flash/flashclearstatus.c"

s32 osFlashAllErase(void) {
    u32 status;

    osEPiWriteIo(&__osFlashHandler, __osFlashHandler.baseAddress | FLASH_CMD_REG, FLASH_CMD_CHIP_ERASE);
    osEPiWriteIo(&__osFlashHandler, __osFlashHandler.baseAddress | FLASH_CMD_REG, FLASH_CMD_EXECUTE_ERASE);
    do {
        osEPiReadIo(&__osFlashHandler, __osFlashHandler.baseAddress, &status);
    } while ((status & FLASH_STATUS_ERASE_BUSY) == FLASH_STATUS_ERASE_BUSY);
    osEPiReadIo(&__osFlashHandler, __osFlashHandler.baseAddress, &status);
    osFlashClearStatus();
    if ((status & 0xFF) == 8 || (status & 0xFF) == 0x48 || (status & 8) == 8) {
        return FLASH_STATUS_ERASE_OK;
    } else {
        return FLASH_STATUS_ERASE_ERROR;
    }
}

s32 osFlashSectorErase(u32 page_num) {
    u32 status;

    osEPiWriteIo(&__osFlashHandler, __osFlashHandler.baseAddress | FLASH_CMD_REG, FLASH_CMD_SECTOR_ERASE | page_num);
    osEPiWriteIo(&__osFlashHandler, __osFlashHandler.baseAddress | FLASH_CMD_REG, FLASH_CMD_EXECUTE_ERASE);
    do {
        osEPiReadIo(&__osFlashHandler, __osFlashHandler.baseAddress, &status);
    } while ((status & FLASH_STATUS_ERASE_BUSY) == FLASH_STATUS_ERASE_BUSY);
    osEPiReadIo(&__osFlashHandler, __osFlashHandler.baseAddress, &status);
    osFlashClearStatus();
    if ((status & 0xFF) == 8 || (status & 0xFF) == 0x48 || (status & 8) == 8) {
        return FLASH_STATUS_ERASE_OK;
    } else {
        return FLASH_STATUS_ERASE_ERROR;
    }
}

#include "../lib/ultralib/src/flash/flashwritebuffer.c"

s32 osFlashWriteArray(u32 page_num) {
    u32 status;

    osEPiWriteIo(&__osFlashHandler, __osFlashHandler.baseAddress | FLASH_CMD_REG, FLASH_CMD_PROGRAM_PAGE | page_num);
    do {
        osEPiReadIo(&__osFlashHandler, __osFlashHandler.baseAddress, &status);
    } while ((status & FLASH_STATUS_WRITE_BUSY) == FLASH_STATUS_WRITE_BUSY);
    status = 0xFFFFFF;
    osEPiReadIo(&__osFlashHandler, __osFlashHandler.baseAddress, &status);
    osFlashClearStatus();
    if ((status & 0xFF) == 4 || (status & 0xFF) == 0x44 || (status & 4) == 4) {
        return FLASH_STATUS_WRITE_OK;
    } else {
        return FLASH_STATUS_WRITE_ERROR;
    }
}

s32 osFlashReadArray(OSIoMesg *mb, s32 priority, u32 page_num, void *dramAddr, u32 n_pages, OSMesgQueue *mq) {
    u32 ret;
    u32 dummy;

    osEPiWriteIo(&__osFlashHandler, __osFlashHandler.baseAddress | FLASH_CMD_REG, FLASH_CMD_READ_ARRAY);
    osEPiReadIo(&__osFlashHandler, __osFlashHandler.baseAddress, &dummy);
    mb->hdr.pri = priority;
    mb->hdr.retQueue = mq;
    mb->dramAddr = dramAddr;
    mb->devAddr = page_num * D_80104F58;
    mb->size = n_pages * FLASH_BLOCK_SIZE;
    ret = osEPiStartDma(&__osFlashHandler, mb, OS_READ);
    return ret;
}
