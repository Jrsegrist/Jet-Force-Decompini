/* Runlink module 71: checks 16 bytes of the cartridge at 0xF80 (a 6105 CIC check). */
#include "common.h"

extern u8 *D_800A3324; /* buffer for the check */
extern void func_8004D640(u32, void *, u32);

s32 func_m071_00000(void) {
    if (D_800A3324 != NULL) {
        func_8004D640(0xF80, D_800A3324, 0x10);
        if (((u32 *) D_800A3324)[0] == 0x6E5F923A && ((u32 *) D_800A3324)[1] == 0xFB0BE5F6) {
            return 1;
        }
    }
    return 0;
}
