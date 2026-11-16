/*
 * Runlink module 156: checks a word that the boot code left in RDRAM (a 6105 CIC check).
 * Family: copy-protection checks (docs/runlink.md, "Module families").
 */
#include "common.h"

/* Returns 1 if the RDRAM word at 0xA00002E8 is 0xC86E2000, else 0 */
s32 func_m156_00000(void) {
    if (*(u32 *) 0xA00002E8 != 0xC86E2000) {
        return 0;
    }
    return 1;
}
