/*
 * libultra osAiSetNextBuffer, an earlier form than ultralib's: the hardware
 * bug test is `(end & 0x3FFF) == 0x2000`. Written from the assembly; built
 * like the game's other 2.0I os/io objects (-O1 -mips2).
 */
#include "PR/os_internal.h"
#include "PR/rcp.h"

extern s32 __osAiDeviceBusy(void);
extern u8 D_800A9A50; /* the hardware bug flag */

s32 osAiSetNextBuffer(void *bufPtr, u32 size) {
    char *bptr = bufPtr;

    if (D_800A9A50) {
        bptr = (char *) bufPtr - 0x2000;
    }
    if ((((u32) bufPtr + size) & 0x3FFF) == 0x2000) {
        D_800A9A50 = TRUE;
    } else {
        D_800A9A50 = FALSE;
    }
    if (__osAiDeviceBusy()) {
        return -1;
    }
    IO_WRITE(AI_DRAM_ADDR_REG, osVirtualToPhysical(bptr));
    IO_WRITE(AI_LEN_REG, size);
    return 0;
}
