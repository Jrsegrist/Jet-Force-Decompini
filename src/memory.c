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

/******************************/

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A3D0);

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A448);

INCLUDE_ASM("asm/nonmatchings/memory", func_8004A458);

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

INCLUDE_ASM("asm/nonmatchings/memory", func_8004AA08);

INCLUDE_ASM("asm/nonmatchings/memory", func_8004AA14);

INCLUDE_ASM("asm/nonmatchings/memory", func_8004AA80);

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

INCLUDE_ASM("asm/nonmatchings/memory", func_8004AEAC);

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
