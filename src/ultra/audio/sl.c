/*
 * JFG's n_audio list object: only the alLink and alUnlink of libultra's src/audio/sl.c (n_audio has its own
 * initialisation), written from the assembly. Built -g (Makefile UL_G_OBJS).
 */
#include <libaudio.h>

/* Insert element after the given one. */
void alLink(ALLink *element, ALLink *after) {
    element->next = after->next;
    element->prev = after;
    if (after->next) {
        after->next->prev = element;
    }
    after->next = element;
}

/* Take element out of its list. */
void alUnlink(ALLink *element) {
    if (element->next) {
        element->next->prev = element->prev;
    }
    if (element->prev) {
        element->prev->next = element->next;
    }
}
