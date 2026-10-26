#ifndef _MEMORY_H_
#define _MEMORY_H_

/* Adapted from the Diddy Kong Racing decompilation (CC0 1.0), src/memory.h. */

#include "common.h"

/* Colour tags label allocations by owner (debug memory views). */
#define COLOUR_TAG_RED 0xFF0000FF
#define COLOUR_TAG_GREEN 0x00FF00FF
#define COLOUR_TAG_BLUE 0x0000FFFF
#define COLOUR_TAG_YELLOW 0xFFFF00FF
#define COLOUR_TAG_MAGENTA 0xFF00FFFF
#define COLOUR_TAG_CYAN 0x00FFFFFF
#define COLOUR_TAG_WHITE 0xFFFFFFFF
#define COLOUR_TAG_GREY 0x7F7F7FFF
#define COLOUR_TAG_ORANGE 0xFF7F7FFF
#define COLOUR_TAG_BLACK 0x000000FF

void *mempool_alloc_safe(s32 size, u32 colourTag);
void mempool_free(void *data);
s32 mempool_locked_set(u8 *address);

#endif
