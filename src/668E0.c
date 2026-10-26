#include "common.h"

extern void *D_800A64B0;
extern void *D_800A64B4;
void *mempool_alloc_safe(s32 size, u32 colourTag);

void func_80065CE0(void) {
    D_800A64B0 = mempool_alloc_safe(0x600, 0xFFFFFF);
    D_800A64B4 = mempool_alloc_safe(0x600, 0xFFFFFF);
}

extern s32 D_800A64B8;
extern s32 D_801012A0[2];

void func_80065D24(void) {
    D_800A64B8 = 1 - D_800A64B8;
    D_801012A0[D_800A64B8] = 0;
}

INCLUDE_ASM("asm/nonmatchings/668E0", func_80065D50);

INCLUDE_ASM("asm/nonmatchings/668E0", func_80065E28);
