#ifndef _MEMORY_H_
#define _MEMORY_H_

/* Adapted from the Diddy Kong Racing decompilation (CC0 1.0), src/memory.h. */

#include "common.h"

typedef enum MemoryPools {
    POOL_MAIN,
    POOL_OBJECT,
    POOL_UNUSED_2,
    POOL_UNUSED_3,

    POOL_COUNT
} MemoryPools;

typedef enum MempoolFlags {
    SLOT_FREE = 0,            // The slot is free.
    SLOT_USED = (1 << 0),     // The slot is used.
    SLOT_LOCKED = (1 << 1),   // The slot is used, and cannot be freed by normal means.
    SLOT_SAFEGUARD = (1 << 2) // The slot is used, and marks the stopping point of a global pool clear.
} MempoolFlags;

#define MEMSLOT_NONE -1

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

/* Size: 0x14 bytes */
typedef struct MemoryPoolSlot {
    /* 0x00 */ u8 *data;
    /* 0x04 */ s32 size;
    /* 0x08 */ s16 flags;
    /* 0x0A */ s16 prevIndex;
    /* 0x0C */ s16 nextIndex;
    /* 0x0E */ s16 index;
    /* 0x10 */ u32 colourTag;
} MemoryPoolSlot;

/* Size: 0x10 bytes */
typedef struct MemoryPool {
    /* 0x00 */ s32 maxNumSlots;
    /* 0x04 */ s32 curNumSlots;
    /* 0x08 */ MemoryPoolSlot *slots;
    /* 0x0C */ s32 size;
} MemoryPool;

void *mempool_alloc_safe(s32 size, u32 colourTag);
MemoryPoolSlot *mempool_slot_find(MemoryPools poolIndex, s32 size, u32 colourTag);
void *mempool_alloc_pool(MemoryPoolSlot *slots, s32 size);
void mempool_free(void *data);
void mempool_free_addr(u8 *address);
s32 mempool_locked_set(u8 *address);
s32 mempool_get_pool(u8 *address);
void mempool_slot_clear(MemoryPools poolIndex, s32 slotIndex);
s32 mempool_slot_assign(MemoryPools poolIndex, s32 slotIndex, s32 size, s32 slotIsTaken, s32 newSlotIsTaken,
                        u32 colourTag);
MemoryPoolSlot *get_memory_pool_address(MemoryPools poolIndex);
u8 *align16(u8 *address);
u8 *align4(u8 *address);
void mempool_print_slots(void);

#endif
