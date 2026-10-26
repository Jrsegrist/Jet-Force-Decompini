/* Runlink module 98: checks 16 bytes of the cartridge at 0xDC0 (a 6105 CIC check). */
#include "common.h"

extern u8 *D_800A3324; /* buffer for the check */
extern void func_8004D640(u32, void *, u32);

s32 func_m098_00000(void) {
    if (D_800A3324 != NULL) {
        func_8004D640(0xDC0, D_800A3324, 0x10);
        if (((u32 *) D_800A3324)[0] == 0xB06DA99E && ((u32 *) D_800A3324)[1] == 0x5A0B4670) {
            return 1;
        }
    }
    return 0;
}
