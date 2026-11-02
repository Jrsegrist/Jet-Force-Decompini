#include "common.h"

s32 drm_check_rdram_a(void) {
    s32 ok = FALSE;
    if (*(u32 *) 0xA02FB1F4 == 0xAD090010) {
        ok = TRUE;
    }
    *(u32 *) 0xA02FB1F4 = 0;
    return ok;
}

s32 drm_check_rdram_b(void) {
    s32 ok = FALSE;
    if (*(u32 *) 0xA02FE1C0 == 0xAD170014) {
        ok = TRUE;
    }
    *(u32 *) 0xA02FE1C0 = 0;
    return ok;
}
