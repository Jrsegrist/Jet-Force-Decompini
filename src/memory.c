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

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A4A4);

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A57C);

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A60C);

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A69C);

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

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A828);

/**
 * Sets the tick timer for the free queue.
 * JFG: only sets it (DKR also flushes the queue when it is zero).
 * Official Name: mmSetDelay
 */
void mempool_free_timer(s32 state) {
    gFreeQueueTimer = state;
}

INCLUDE_ASM("asm/nonmatchings/memory", func_8004AA14);

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

INCLUDE_ASM("asm/nonmatchings/memory", func_8004AAC4);

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

INCLUDE_ASM("asm/nonmatchings/memory", func_8004AC90);

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

// mempool_slot_clear: differs from DKR's
INCLUDE_ASM("asm/nonmatchings/memory", func_8004AD2C);

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

// mempool_slot_assign: differs from DKR's
INCLUDE_ASM("asm/nonmatchings/memory", func_8004AEBC);

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
