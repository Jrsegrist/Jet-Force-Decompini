#include "common.h"

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001CE10);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D344);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D464);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001D500);

extern s32 D_800A0E80[4];
extern s32 D_800A0E90[4];
extern s32 D_800A0EA0[4];
extern s32 D_800A0EC8;
extern s32 D_800F5B00;
extern s32 D_800F5B04;
extern s32 D_800F5B08;
extern s32 D_800F5B0C;
extern s32 D_800F5B10;
void mempool_free(void *data);
s32 mempool_alloc_safe(s32 size, u32 colourTag);

/* JFG: allocate the three shadow buffers (each split in four) for the given counts. */
void shadow_buffers_alloc(s32 arg0, s32 arg1, s32 count0, s32 count1, s32 count2) {
    s32 size0;
    s32 size1;
    s32 size2;
    s32 i;

    D_800F5B0C = arg0;
    D_800F5B10 = arg1;
    D_800F5B00 = count0;
    D_800F5B04 = count1;
    D_800F5B08 = count2;
    size0 = count0 * 10;
    D_800A0E80[0] = mempool_alloc_safe(size0 * 4, 0xFFFF00FF);
    size1 = count1 * 16;
    D_800A0E90[0] = mempool_alloc_safe(size1 * 4, 0xFFFF00FF);
    size2 = count2 * 8;
    D_800A0EA0[0] = mempool_alloc_safe(size2 * 4, 0xFFFF00FF);
    for (i = 1; i < 4; i++) {
        D_800A0E80[i] = D_800A0E80[i - 1] + size0;
        D_800A0E90[i] = D_800A0E90[i - 1] + size1;
        D_800A0EA0[i] = D_800A0EA0[i - 1] + size2;
    }
    D_800A0EC8 = 0;
}

/* JFG: free the three shadow buffers. */
void shadow_buffers_free(void) {
    if (((void **) D_800A0E80)[0] != NULL) {
        mempool_free(((void **) D_800A0E80)[0]);
        ((void **) D_800A0E80)[0] = NULL;
    }
    if (((void **) D_800A0E90)[0] != NULL) {
        mempool_free(((void **) D_800A0E90)[0]);
        ((void **) D_800A0E90)[0] = NULL;
    }
    if (((void **) D_800A0EA0)[0] != NULL) {
        mempool_free(((void **) D_800A0EA0)[0]);
        ((void **) D_800A0EA0)[0] = NULL;
    }
}

/* JFG: flip between two buffers. */
void func_8001D9B4(void) {
    D_800A0EC8 ^= 1;
}

void func_8001D9D0(s32 flags, s32 *a, s32 *b, s32 *c) {
    s32 index = D_800A0EC8;
    if (flags & 2) {
        index += 2;
    }
    *a = D_800A0E80[index];
    *b = D_800A0E90[index];
    *c = D_800A0EA0[index];
}

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001DA1C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E17C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001E8B8);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001EE1C);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001F4BC);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001FA24);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_8001FFE0);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_800203C4);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020890);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020954);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_800209E4);

INCLUDE_ASM("asm/nonmatchings/1DA10", func_80020AF8);
