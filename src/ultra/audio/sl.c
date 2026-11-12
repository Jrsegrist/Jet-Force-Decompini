/*
 * JFG's n_audio list object: only the alLink and alUnlink of libultra's src/audio/sl.c (n_audio has its own
 * initialisation), written from the assembly. Built -g (Makefile UL_G_OBJS).
 */
#include <libaudio.h>

void alLink(ALLink *ln, ALLink *to) {
    ln->next = to->next;
    ln->prev = to;
    if (to->next) {
        to->next->prev = ln;
    }
    to->next = ln;
}

void alUnlink(ALLink *ln) {
    if (ln->next) {
        ln->next->prev = ln->prev;
    }
    if (ln->prev) {
        ln->prev->next = ln->next;
    }
}
