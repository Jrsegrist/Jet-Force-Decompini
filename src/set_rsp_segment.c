/* From the Diddy Kong Racing decompilation (CC0 1.0), src/set_rsp_segment.c. */
#include "common.h"

/**
 * Set the offset for the given address, so the RSP can get the correct physical address.
 * This is used for the colour and depth buffers.
 * Official name: segSetBase
 */
void rsp_segment(Gfx **dList, s32 segment, s32 base) {
    gSPSegment((*dList)++, segment, base + K0BASE);
}
