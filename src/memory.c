/*
 * Memory pools.
 *
 * From the Diddy Kong Racing decompilation (CC0 1.0), src/memory.c. Jet Force
 * Gemini's allocator differs in most places (no interrupt masking, colour
 * tags from the runlink loader); those functions are still assembly.
 */
#include "memory.h"

/************ .bss ************/

extern MemoryPool gMemoryPools[POOL_COUNT];
extern s32 gNumberOfMemoryPools;
extern s32 gFreeQueueTimer;
extern s32 gFreeQueueCount;
extern u8 gExpansionPak;
extern s32 gRamEnd;
extern u8 gMainMemoryPool[]; // the end of the game's .bss

#define RAM_END 0x80400000
#define EXPANSION_RAM_END 0x80600000
#define MAIN_POOL_SLOT_COUNT 1600

extern s32 D_800FDD90;                // JFG: free RAM in the main pool (bytes)
extern s32 D_800FDD98[POOL_COUNT];    // JFG: free bytes per pool
extern s32 D_800A33F4;                // JFG: a forced colour tag (or -1)
extern s32 D_800A33F8;                // JFG: a second forced colour tag (or -1)
void func_80054368(s32 address, s32 *module, s32 *offset, s32 arg3);

void mempool_free_queue(void *dataAddress);
void mempool_free_timer(s32 state);
MemoryPoolSlot *mempool_init(MemoryPoolSlot *slots, s32 poolSize, s32 numSlots);

/******************************/

/**
 * Creates the main memory pool, from the end of the game's .bss to the end of RAM
 * (with or without the Expansion Pak). Contains 1600 allocation slots.
 * Official Name: mmInit
 */
void mempool_init_main(void) {
    gNumberOfMemoryPools = -1;
    if (gExpansionPak) {
        gRamEnd = EXPANSION_RAM_END;
    } else {
        gRamEnd = RAM_END;
    }
    mempool_init((MemoryPoolSlot *) gMainMemoryPool, gRamEnd - (s32) gMainMemoryPool, MAIN_POOL_SLOT_COUNT);
    mempool_free_timer(2);
    gFreeQueueCount = 0;
}

extern u8 gExpansionPak;

u8 func_8004A448(void) {
    return gExpansionPak;
}

/**
 * Creates a new memory pool that's contained inside another one.
 * JFG: no interrupt masking.
 * Official name: mmAllocRegion
 */
MemoryPoolSlot *mempool_new_sub(s32 poolDataSize, s32 numSlots) {
    s32 size;
    MemoryPoolSlot *slots;
    UNUSED s32 unused_2;
    MemoryPoolSlot *newPool;

    size = poolDataSize + (numSlots * sizeof(MemoryPoolSlot));
    slots = (MemoryPoolSlot *) mempool_alloc_safe(size, COLOUR_TAG_WHITE);
    newPool = mempool_init(slots, size, numSlots);
    return newPool;
}

/**
 * Create and initialise a memory pool in RAM that will act as the place where arbitrary allocations can go.
 * Will return the location of the first free slot in that pool.
 * JFG: also records the pool's free size (and the main pool's in D_800FDD90).
 */
MemoryPoolSlot *mempool_init(MemoryPoolSlot *slots, s32 poolSize, s32 numSlots) {
    MemoryPoolSlot *firstSlot;
    s32 poolCount;
    s32 i;
    s32 firstSlotSize;

poolCount = ++gNumberOfMemoryPools;
    firstSlotSize = poolSize - (numSlots * sizeof(MemoryPoolSlot));
    gMemoryPools[poolCount].maxNumSlots = numSlots;
    gMemoryPools[poolCount].curNumSlots = 0;
    gMemoryPools[poolCount].slots = slots;
    gMemoryPools[poolCount].size = poolSize;
    firstSlot = slots;
    for (i = 0; i < gMemoryPools[poolCount].maxNumSlots; i++) {
        firstSlot->index = i;
        firstSlot++;
    }
    firstSlot = &gMemoryPools[poolCount].slots[0];
    slots += numSlots;
    if ((s32) slots & 0xF) {
        firstSlot->data = (u8 *) (((s32) slots & ~0xF) + 0x10);
    } else {
        firstSlot->data = (u8 *) slots;
    }
firstSlot->size = firstSlotSize;
    firstSlot->flags = SLOT_FREE;
    firstSlot->prevIndex = MEMSLOT_NONE;
    firstSlot->nextIndex = MEMSLOT_NONE;
    gMemoryPools[poolCount].curNumSlots++;
    if (poolCount == POOL_MAIN) {
        D_800FDD90 = firstSlotSize;
    }
    D_800FDD98[poolCount] = firstSlotSize;
    return gMemoryPools[poolCount].slots;
}

/**
 * Reserves and returns memory from the main memory pool.
 * JFG: no assert checks; the colour tag is replaced by a debug tag (see mempool_alloc_fixed).
 */
void *mempool_alloc_safe(s32 size, u32 colourTag) {
    MemoryPoolSlot *addr;
    s32 module;
    s32 offset;
    UNUSED s32 pad;
    volatile s32 ra;

    ra = 0x666; // the return address in debug builds
    if (D_800A33F4 != -1) {
        colourTag = D_800A33F4 | 0xFF000000;
    } else if (D_800A33F8 != -1) {
        colourTag = D_800A33F8 | 0xFE000000;
    } else {
        func_80054368(ra - 8, &module, &offset, 0);
        colourTag = (module << 24) | offset;
    }
    addr = mempool_slot_find(POOL_MAIN, size, colourTag);
    return addr;
}

/**
 * Reserves and returns memory from the main memory pool (the same as mempool_alloc_safe in JFG).
 */
MemoryPoolSlot *mempool_alloc(s32 size, u32 colourTag) {
    MemoryPoolSlot *addr;
    s32 module;
    s32 offset;
    UNUSED s32 pad;
    volatile s32 ra;

    ra = 0x666; // the return address in debug builds
    if (D_800A33F4 != -1) {
        colourTag = D_800A33F4 | 0xFF000000;
    } else if (D_800A33F8 != -1) {
        colourTag = D_800A33F8 | 0xFE000000;
    } else {
        func_80054368(ra - 8, &module, &offset, 0);
        colourTag = (module << 24) | offset;
    }
    addr = mempool_slot_find(POOL_MAIN, size, colourTag);
    return addr;
}

/**
 * Search the existing empty slots and try to find one that can meet the size requirement.
 * Afterwards, write the new allocation data to the slot in question and return the address.
 * JFG: no interrupt masking.
 */
MemoryPoolSlot *mempool_slot_find(MemoryPools poolIndex, s32 size, u32 colourTag) {
    s32 slotSize;
    MemoryPoolSlot *curSlot;
    UNUSED s32 pad;
    MemoryPool *pool;
    MemoryPoolSlot *slots;
    s32 nextIndex;
    s32 currIndex;

    pool = &gMemoryPools[poolIndex];
    if (pool->maxNumSlots == pool->curNumSlots + 1) {
        return NULL;
    }
    currIndex = MEMSLOT_NONE;
    if (size & 0xF) {
        size = (size & ~0xF) + 0x10;
    }
    slots = pool->slots;
    slotSize = 0x7FFFFFFF;
    nextIndex = 0;
    do {
        curSlot = &slots[nextIndex];
        if (curSlot->flags == SLOT_FREE) {
            if (curSlot->size >= size && curSlot->size < slotSize) {
                slotSize = curSlot->size;
                currIndex = nextIndex;
            }
        }
        nextIndex = curSlot->nextIndex;
    } while (nextIndex != MEMSLOT_NONE);
    if (currIndex != MEMSLOT_NONE) {
        mempool_slot_assign(poolIndex, (s32) currIndex, size, 1, 0, colourTag);
        return (MemoryPoolSlot *) (slots + currIndex)->data;
    }
    return NULL;
}

/**
 * Allocate memory from a specific pool.
 * Official name: mmAllocR
 */
void *mempool_alloc_pool(MemoryPoolSlot *slots, s32 size) {
    s32 i;
    for (i = gNumberOfMemoryPools; i != 0; i--) {
        if (slots == gMemoryPools[i].slots) {
            return mempool_slot_find(i, size, 0);
        }
    }
    return (void *) NULL;
}

/**
 * Allocates memory from the main pool at a fixed address.
 * Rearranges the memory slots to place one at that address if possible.
 * JFG: no interrupt masking; the colour tag is replaced by a debug tag (a forced
 * one, or the caller's runtime-linked module and offset).
 * Official Name: mmAllocAtAddr
 */
void *mempool_alloc_fixed(s32 size, u8 *address, u32 colourTag) {
    s32 i;
    MemoryPoolSlot *curSlot;
    MemoryPoolSlot *slots;
    s32 module;
    s32 offset;
    UNUSED s32 pad;
    volatile s32 ra;

    ra = 0x666; // the return address in debug builds
    if (D_800A33F4 != -1) {
        colourTag = D_800A33F4 | 0xFF000000;
    } else if (D_800A33F8 != -1) {
        colourTag = D_800A33F8 | 0xFE000000;
    } else {
        func_80054368(ra - 8, &module, &offset, 0);
        colourTag = (module << 24) | offset;
    }
    if ((gMemoryPools[POOL_MAIN].curNumSlots + 1) == gMemoryPools[POOL_MAIN].maxNumSlots) {
        return NULL;
    }
    if (size & 0xF) {
        size = (size & ~0xF) + 0x10;
    }
    slots = gMemoryPools[POOL_MAIN].slots;
    for (i = 0; i != MEMSLOT_NONE; i = curSlot->nextIndex) {
        curSlot = &slots[i];
        if (curSlot->flags == SLOT_FREE) {
            if ((u32) address >= (u32) curSlot->data &&
                (u32) address + size <= (u32) curSlot->data + curSlot->size) {
                if (address == (u8 *) curSlot->data) {
                    mempool_slot_assign(POOL_MAIN, i, size, 1, 0, colourTag);
                    return curSlot->data;
                } else {
                    i = mempool_slot_assign(POOL_MAIN, i, (u32) address - (u32) curSlot->data, 0, 1, colourTag);
                    mempool_slot_assign(POOL_MAIN, i, size, 1, 0, colourTag);
                    return (slots + i)->data;
                }
            }
        }
    }
    return NULL;
}

/**
 * Sets the tick timer for the free queue.
 * JFG: only sets it (DKR also flushes the queue when it is zero).
 * Official Name: mmSetDelay
 */
void mempool_free_timer(s32 state) {
    gFreeQueueTimer = state;
}

extern void *gFreeQueueAddress[256];

/* DKR: the flush loop of mempool_free_timer - frees everything in the free queue now. */
void func_8004AA14(void) {
    while (gFreeQueueCount > 0) {
        mempool_free_addr(gFreeQueueAddress[--gFreeQueueCount]);
    }
}

/**
 * Unallocates data from the pool that contains the data. Will free immediately if the free queue
 * state is set to 0, otherwise the data will just be marked for deletion.
 * JFG: no interrupt masking; a dummy flags value is kept on the stack.
 * Official Name: mmFree
 */
void mempool_free(void *data) {
    volatile u32 intFlags = 0x666;
    if (gFreeQueueTimer == 0) {
        mempool_free_addr(data);
    } else {
        mempool_free_queue(data);
    }
}

extern void *gFreeQueueAddress[256];
extern u8 gFreeQueueTimers[256];
void func_800542C4(void);
s32 func_80053370(s32 module);
void func_80054530(void); // a module function (patched in by the runtime linker)

/**
 * Frees all the addresses in the free queue.
 * JFG: first reacts to low memory (through the runtime linker); no interrupt masking.
 * Official Name: mmFreeTick
 */
void mempool_free_queue_clear(void) {
    s32 i;

    if (D_800FDD90 < 0x14000) {
        func_800542C4();
        if (D_800FDD90 < 0xC000) {
            if (func_80053370(3)) {
                func_80054530();
            }
        }
    }

    for (i = 0; i < gFreeQueueCount;) {
        gFreeQueueTimers[i]--;
        if (gFreeQueueTimers[i] == 0) {
            mempool_free_addr(gFreeQueueAddress[i]);
            gFreeQueueAddress[i] = gFreeQueueAddress[gFreeQueueCount - 1];
            gFreeQueueTimers[i] = gFreeQueueTimers[gFreeQueueCount - 1];
            gFreeQueueCount--;
        } else {
            i++;
        }
    }
}

/**
 * Searches the memory pools for a slot matching the given address.
 * If a slot is found, free it.
 */
void mempool_free_addr(u8 *address) {
    s32 slotIndex;
    s32 poolIndex;
    MemoryPool *pool;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;

    poolIndex = mempool_get_pool(address);
    pool = gMemoryPools;
    slots = pool[poolIndex].slots;
    for (slotIndex = 0; slotIndex != MEMSLOT_NONE; slotIndex = slot->nextIndex) {
        slot = &slots[slotIndex];

        if (address == (u8 *) slot->data) {
            if (slot->flags == SLOT_USED || slot->flags == SLOT_SAFEGUARD) {
                mempool_slot_clear(poolIndex, slotIndex);
            }
            break;
        }
        slot = &slots[slotIndex];
    }
    // DKR: stubbed_printf("\n*** mm Error *** ---> No match found for mmFree.\n");
    // (the string is not in Jet Force Gemini's ROM)
}

/**
 * Adds data to the free queue, to be freed after gFreeQueueTimer ticks.
 * JFG: addresses and timers are separate arrays.
 */
void mempool_free_queue(void *dataAddress) {
    gFreeQueueAddress[gFreeQueueCount] = dataAddress;
    gFreeQueueTimers[gFreeQueueCount] = gFreeQueueTimer;
    gFreeQueueCount++;
}

/**
 * Returns the index of the memory pool containing the memory address.
 */
s32 mempool_get_pool(u8 *address) {
    s32 i;
    MemoryPool *pool;

    for (i = gNumberOfMemoryPools; i > 0; i--) {
        pool = &gMemoryPools[i];
        if ((u8 *) pool->slots >= address) {
            continue;
        }
        if (address < pool->size + (u8 *) pool->slots) {
            break;
        }
    }
    return i;
}

/**
 * Clears the current slot of all information, effectively freeing the allocated memory.
 * Unused slots before and after will be merged with this slot
 * JFG: also adds the slot's size back to the free counters.
 */
#ifdef NON_MATCHING
/* NON_MATCHING: the slot->flags store and the free-counter updates are scheduled differently (15 words) */
void mempool_slot_clear(MemoryPools poolIndex, s32 slotIndex) {
    s32 nextIndex;
    s32 prevIndex;
    s32 tempNextIndex;
    MemoryPool *pool;
    MemoryPoolSlot *slots;
    MemoryPoolSlot *slot;
    MemoryPoolSlot *nextSlot;
    MemoryPoolSlot *prevSlot;

    pool = &gMemoryPools[poolIndex];
    slots = pool->slots;
    pool = pool; // Fakematch
    slot = &slots[slotIndex];
    nextIndex = slot->nextIndex;
    prevIndex = slot->prevIndex;
    slot = slot; // Fakematch
    nextSlot = &slots[nextIndex];
    prevSlot = &slots[prevIndex];
    if (poolIndex == POOL_MAIN) {
        D_800FDD90 += slot->size;
    }
    D_800FDD98[poolIndex] += slot->size;
    slot->flags = SLOT_FREE;
    if (nextIndex != MEMSLOT_NONE) {
        if (nextSlot->flags == SLOT_FREE) {
            slot->size += nextSlot->size;
            tempNextIndex = nextSlot->nextIndex;
            slot->nextIndex = tempNextIndex;
            if (tempNextIndex != MEMSLOT_NONE) {
                slots[tempNextIndex].prevIndex = slotIndex;
            }
            pool->curNumSlots--;
            slots[pool->curNumSlots].index = nextIndex;
        }
    }
    if (prevIndex != MEMSLOT_NONE) {
        if (prevSlot->flags == SLOT_FREE) {
            prevSlot->size += slot->size;
            tempNextIndex = slot->nextIndex;
            prevSlot->nextIndex = tempNextIndex;
            if (tempNextIndex != MEMSLOT_NONE) {
                slots[tempNextIndex].prevIndex = prevIndex;
            }
            pool->curNumSlots--;
            slots[pool->curNumSlots].index = slotIndex;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/memory", func_8004AD2C);
#endif

/**
 * Return the address of the first slot of a given memory pool.
 * Official Name: mmGetSlotPtr
 */
MemoryPoolSlot *get_memory_pool_address(MemoryPools poolIndex) {
    return gMemoryPools[poolIndex].slots;
}

extern s32 gFreeQueueTimer;

s32 func_8004AEAC(void) {
    return gFreeQueueTimer;
}

/**
 * Initialise and attempts to fit the new memory block in the slot given.
 * Updates the linked list with any entries before and after then returns the new slot index.
 * If the region cannot fit, return the old slot instead.
 * JFG: a taken slot's size is taken off the free counters.
 */
s32 mempool_slot_assign(MemoryPools poolIndex, s32 slotIndex, s32 size, s32 slotIsTaken, s32 newSlotIsTaken,
                        u32 colourTag) {
    MemoryPool *pool;
    MemoryPoolSlot *poolSlots;
    s32 index;
    s32 nextIndex;
    s32 poolSize;

    if (slotIsTaken == 1) {
        if (poolIndex == POOL_MAIN) {
            D_800FDD90 -= size;
        }
        D_800FDD98[poolIndex] -= size;
    }
    pool = &gMemoryPools[poolIndex];
    poolSlots = pool->slots;
    pool = pool; // Fakematch
    poolSlots[slotIndex].flags = slotIsTaken;
    poolSize = poolSlots[slotIndex].size;
    poolSlots[slotIndex].size = size;
    poolSlots[slotIndex].colourTag = colourTag;
    index = poolSlots[pool->curNumSlots].index;
    if (size < poolSize) {
        index = (pool->curNumSlots + poolSlots)->index;
        pool->curNumSlots++;
        poolSlots[index].data = &poolSlots[slotIndex].data[size];
        poolSlots[index].size = poolSize;
        poolSlots[index].size -= size;
        poolSlots[index].flags = newSlotIsTaken;
        poolSize = poolSlots[slotIndex].nextIndex;
        nextIndex = poolSize;
        poolSlots[index].prevIndex = slotIndex;
        poolSlots[index].nextIndex = nextIndex;
        poolSlots[slotIndex].nextIndex = index;
        if (nextIndex != MEMSLOT_NONE) {
            poolSlots[nextIndex].prevIndex = index;
        }
        return index;
    }
    return slotIndex;
}

/**
 * Returns the passed in address aligned to the next 16-byte boundary.
 * Official name: mmAlign16
 */
u8 *align16(u8 *address) {
    s32 remainder = (s32) address & 0xF;
    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 16);
    }
    return address;
}

/**
 * Returns the passed in address aligned to the next 4-byte boundary.
 * Official name: mmAlign4
 */
u8 *align4(u8 *address) {
    s32 remainder = (s32) address & 0x3;
    if (remainder > 0) {
        address = (u8 *) (((s32) address - remainder) + 4);
    }
    return address;
}

/**
 * Prints out the status of each memory pool slot.
 * Will mark based on what flags the slot has.
 * Official name: mmSlotPrint
 */
void mempool_print_slots(void) {
    int flags;
    int nextIndex;
    int i;
    MemoryPoolSlot *slot;

    // The debug prints (DKR: "Region = %d...", "FREE", "ALLOCATED", ...) are
    // compiled out. Their strings are in the ROM's .rodata, which is still
    // assembly; they come back here once this file's .rodata is split out.
    for (i = 0; i <= gNumberOfMemoryPools; i++) {
        slot = &gMemoryPools[i].slots[0];

        do {
            flags = slot->flags;
            nextIndex = slot->nextIndex;

            switch (flags) {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    break;
                default:
                    break;
            }
            if (nextIndex == -1) {
                continue;
            } else {
                slot = &gMemoryPools[i].slots[slot->nextIndex];
            }
        } while (nextIndex != -1);
    }
}
