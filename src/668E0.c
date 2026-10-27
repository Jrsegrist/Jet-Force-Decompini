#include "common.h"

/* Size: 0xC bytes */
typedef struct ListEntry {
    s32 a;
    s32 b;
    s32 c;
} ListEntry;

extern ListEntry *D_800A64B0[2]; // JFG: two lists of 128 entries, used in turn
void *mempool_alloc_safe(s32 size, u32 colourTag);

void func_80065CE0(void) {
    D_800A64B0[0] = mempool_alloc_safe(0x600, 0xFFFFFF);
    D_800A64B0[1] = mempool_alloc_safe(0x600, 0xFFFFFF);
}

extern s32 D_800A64B8;
extern s32 D_801012A0[2];

void func_80065D24(void) {
    D_800A64B8 = 1 - D_800A64B8;
    D_801012A0[D_800A64B8] = 0;
}

/* JFG: add an entry to the current list (up to 128). */
void func_80065D50(s32 a, s32 b, s32 c) {
    if (D_800A64B0[D_800A64B8] != NULL && D_801012A0[D_800A64B8] < 128) {
        D_800A64B0[D_800A64B8][D_801012A0[D_800A64B8]].a = a;
        D_800A64B0[D_800A64B8][D_801012A0[D_800A64B8]].b = b;
        D_800A64B0[D_800A64B8][D_801012A0[D_800A64B8]].c = c;
        D_801012A0[D_800A64B8]++;
    }
}

INCLUDE_ASM("asm/nonmatchings/668E0", func_80065E28);
