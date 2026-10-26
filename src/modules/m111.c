/* Runlink module 111: reads one cartridge word through the PI (a 6105 CIC check). */
#include "common.h"

extern void func_80099CB0(void); /* takes the PI */
extern void func_80099CF4(void); /* releases the PI */

s32 func_m111_00000(u32 addr) {
    register u32 stat;

    func_80099CB0();
    while (stat = IO_READ(PI_STATUS_REG), stat & (PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY)) {
    }
    stat = IO_READ(addr | 0xB0000454);
    func_80099CF4();
    if (stat != 0x0109082B) {
        return 0;
    }
    return 1;
}
