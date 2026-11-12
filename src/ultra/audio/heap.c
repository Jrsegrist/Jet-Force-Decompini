/*
 * JFG's n_audio heap object: alHeapInit in an older form than ultralib's (no base alignment; written from the
 * assembly), then libultra's src/audio/heapalloc.c, built from decompals/ultralib (lib/ultralib). Built -g
 * (Makefile UL_G_OBJS).
 */
#include <libaudio.h>

void alHeapInit(ALHeap *hp, u8 *base, s32 len) {
    hp->base = base;
    hp->len = len;
    hp->cur = hp->base;
    hp->count = 0;
}

#include "src/audio/heapalloc.c"
