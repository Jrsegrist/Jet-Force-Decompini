/*
 * Runlink module 133: reads one cartridge word through the PI (a 6105 CIC check).
 * Family: copy-protection checks (docs/runlink.md, "Module families").
 */
#include "common.h"

extern void func_80099CB0(void); /* takes the PI */
extern void func_80099CF4(void); /* releases the PI */

/* Waits for the PI, reads the cartridge word at addr | 0xB0000A5C; returns 1 if it is 0x1740FFF9 */
s32 func_m133_00000(u32 addr) {
    register u32 stat;

    func_80099CB0();
    while (stat = IO_READ(PI_STATUS_REG), stat & (PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY)) {
    }
    stat = IO_READ(addr | 0xB0000A5C);
    func_80099CF4();
    if (stat != 0x1740FFF9) {
        return 0;
    }
    return 1;
}
