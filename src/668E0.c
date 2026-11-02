#include "common.h"

/* Size: 0xC bytes */
typedef struct ListEntry {
    s32 a;
    s32 b;
    s32 c;
} ListEntry;

extern ListEntry *D_800A64B0[2]; // JFG: two lists of 128 entries, used in turn
void *mempool_alloc_safe(s32 size, u32 colourTag);

void debug_trace_init(void) {
    D_800A64B0[0] = mempool_alloc_safe(0x600, 0xFFFFFF);
    D_800A64B0[1] = mempool_alloc_safe(0x600, 0xFFFFFF);
}

extern s32 D_800A64B8;
extern s32 D_801012A0[2];

void debug_trace_flip(void) {
    D_800A64B8 = 1 - D_800A64B8;
    D_801012A0[D_800A64B8] = 0;
}

/* JFG: add an entry to the current list (up to 128). */
void debug_trace_add(s32 a, s32 b, s32 c) {
    if (D_800A64B0[D_800A64B8] != NULL && D_801012A0[D_800A64B8] < 128) {
        D_800A64B0[D_800A64B8][D_801012A0[D_800A64B8]].a = a;
        D_800A64B0[D_800A64B8][D_801012A0[D_800A64B8]].b = b;
        D_800A64B0[D_800A64B8][D_801012A0[D_800A64B8]].c = c;
        D_801012A0[D_800A64B8]++;
    }
}

/* JFG: in the previous list, find the entries with the nearest .a at or below key and above key, and
   return their fields (b, c, a); the first output of each is 0 when there is none. */
void func_80065E28(u32 key, s32 *belowB, s32 *belowC, s32 *belowA, s32 *aboveB, s32 *aboveC, s32 *aboveA) {
    s32 prev = 1 - D_800A64B8;
    u32 lo = 0;
    u32 hi = -1;
    ListEntry *below = NULL;
    ListEntry *above = NULL;
    s32 i;
    u32 v;

    for (i = 0; i < D_801012A0[prev]; i++) {
        v = D_800A64B0[prev][i].a;
        if (key >= v) {
            if (lo < v) {
                lo = v;
                below = &D_800A64B0[prev][i];
            }
        }
        if (key < v) {
            if (v < hi) {
                hi = v;
                above = &D_800A64B0[prev][i];
            }
        }
    }
    if (below != NULL) {
        *belowB = below->b;
        *belowC = below->c;
        *belowA = below->a;
    } else {
        *belowB = 0;
    }
    if (above != NULL) {
        *aboveB = above->b;
        *aboveC = above->c;
        *aboveA = above->a;
    } else {
        *aboveB = 0;
    }
}
