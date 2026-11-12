/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): the event queue, adding players
 * to the synthesizer and voice allocation (cf. libultra's event.c, synaddplayer.c and synallocvoice.c).
 */
#include "common.h"

void alLink(ALLink *element, ALLink *after);
void alUnlink(ALLink *element);

/* JFG: an n_audio event (0x10 bytes) */
typedef struct NEvent {
    /* 0x00 */ s16 type;
    /* 0x02 */ u8 pad2[0x10 - 0x2];
} NEvent;

/* JFG: an n_audio event queue entry (0x1C bytes) */
typedef struct NEventListItem {
    /* 0x00 */ ALLink node;
    /* 0x08 */ s32 delta;
    /* 0x0C */ NEvent evt;
} NEventListItem;

/* JFG: an n_audio event queue */
typedef struct NEventQueue {
    /* 0x00 */ ALLink freeList;
    /* 0x08 */ ALLink allocList;
    /* 0x10 */ s16 eventCount;
    /* 0x12 */ u16 lostCount; // JFG: events that found no free entry (up to 60000)
    /* 0x14 */ s16 maxCount;  // JFG: the most events queued at once
} NEventQueue;

/* JFG: the clock an external event is timed against (only the field used here) */
typedef struct NEventClock {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s32 time;
} NEventClock;

/* JFG: a synthesizer client (libultra ALPlayer) */
typedef struct NPlayer {
    /* 0x00 */ struct NPlayer *next;
    /* 0x04 */ u8 pad4[0x10 - 0x4];
    /* 0x10 */ s32 samplesLeft;
} NPlayer;

struct NVoice;

/* JFG: an n_audio physical voice (only the fields used here) */
typedef struct NPVoice {
    /* 0x00 */ ALLink node;
    /* 0x08 */ struct NVoice *vvoice;
    /* 0x0C */ u8 padC[0x88 - 0xC];
    /* 0x88 */ s32 offset;
} NPVoice;

/* JFG: an n_audio voice */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ NPVoice *pvoice;
    /* 0x0C */ void *table;
    /* 0x10 */ u8 pad10[0x14 - 0x10];
    /* 0x14 */ s16 state;
    /* 0x16 */ s16 priority;
    /* 0x18 */ s16 fxBus;
    /* 0x1A */ s16 unityPitch;
} NVoice;

/* JFG: a voice's configuration (libultra ALVoiceConfig) */
typedef struct NVoiceConfig {
    /* 0x00 */ s16 priority;
    /* 0x02 */ s16 fxBus;
    /* 0x04 */ u8 unityPitch;
} NVoiceConfig;

/* JFG: an n_audio synthesizer (only the fields used here) */
typedef struct NSynth {
    /* 0x00 */ NPlayer *head;
    /* 0x04 */ ALLink pFreeList;
    /* 0x0C */ ALLink pAllocList;
    /* 0x14 */ ALLink pLameList;
    /* 0x1C */ s32 paramSamples;
    /* 0x20 */ s32 curSamples;
} NSynth;

/* JFG: a parameter update for the envelope mixer (libultra ALParam) */
typedef struct NParam {
    /* 0x00 */ struct NParam *next;
    /* 0x04 */ s32 delta;
    /* 0x08 */ s16 type;
    /* 0x0C */ s32 data;
    /* 0x10 */ s32 moredata;
} NParam;

#define N_EVTQ_END 0x7FFFFFFF
#define N_FILTER_RESET 11
#define N_FILTER_STOP_VOICE 15
#define N_FILTER_ADD_UPDATE 3

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
extern s32 D_80105020;     // JFG: the external event clock
NParam *func_8008AF34(void);                                   // n_audio: allocate a parameter update
void func_80090B14(NPVoice *pvoice, s32 paramID, void *param); // n_audio: the envelope mixer's param handler

/* JFG: an empty event queue with the given free entries (libultra alEvtqNew). */
void func_8008B4B0(NEventQueue *evtq, NEventListItem *items, s32 itemCount) {
    s32 i;

    evtq->eventCount = 0;
    evtq->maxCount = 0;
    evtq->lostCount = 0;
    evtq->allocList.next = 0;
    evtq->allocList.prev = 0;
    evtq->freeList.next = 0;
    evtq->freeList.prev = 0;
    for (i = 0; i < itemCount; i++) {
        alLink((ALLink *) &items[i], &evtq->freeList);
    }
}

/* JFG: take the next event off the queue; returns its delta (libultra alEvtqNextEvent). */
s32 func_8008B55C(NEventQueue *evtq, NEvent *evt) {
    NEventListItem *item;
    s32 delta;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    item = (NEventListItem *) evtq->allocList.next;
    if (item) {
        alUnlink((ALLink *) item);
        bcopy(&item->evt, evt, sizeof(*evt));
        alLink((ALLink *) item, &evtq->freeList);
        delta = item->delta;
        evtq->eventCount--;
    } else {
        evt->type = -1;
        delta = 0;
    }
    osSetIntMask(mask);
    return delta;
}

/* JFG: queue an event delta microseconds after the one before it (libultra alEvtqPostEvent). */
void func_8008B610(NEventQueue *evtq, NEvent *evt, s32 delta) {
    NEventListItem *item;
    NEventListItem *nextItem;
    ALLink *node;
    s32 postAtEnd = 0;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    item = (NEventListItem *) evtq->freeList.next;
    if (!item) {
        if (evtq->lostCount < 60000) {
            evtq->lostCount++;
        }
        osSetIntMask(mask);
        return;
    }
    alUnlink((ALLink *) item);
    bcopy(evt, &item->evt, sizeof(*evt));
    evtq->eventCount++;
    if (evtq->eventCount > evtq->maxCount) {
        evtq->maxCount = evtq->eventCount;
    }
    if (delta == N_EVTQ_END) {
        postAtEnd = -1;
    }
    for (node = &evtq->allocList; node != 0; node = node->next) {
        if (!node->next) {
            if (postAtEnd) {
                item->delta = 0;
            } else {
                item->delta = delta;
            }
            alLink((ALLink *) item, node);
            break;
        } else {
            nextItem = (NEventListItem *) node->next;
            if (delta < nextItem->delta) {
                item->delta = delta;
                nextItem->delta -= delta;
                alLink((ALLink *) item, node);
                break;
            }
            delta -= nextItem->delta;
        }
    }
    osSetIntMask(mask);
}

/* JFG: queue an event at an absolute time on the external clock (kept no earlier than clock->time). */
void func_8008B7EC(NEventClock *clock, NEventQueue *evtq, NEvent *evt, s32 delta) {
    NEventListItem *item;
    NEventListItem *nextItem;
    ALLink *node;
    s32 postAtEnd = 0;
    UNUSED s32 pad = 0;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    item = (NEventListItem *) evtq->freeList.next;
    if (!item) {
        if (evtq->lostCount < 60000) {
            evtq->lostCount++;
        }
        osSetIntMask(mask);
        return;
    }
    alUnlink((ALLink *) item);
    bcopy(evt, &item->evt, sizeof(*evt));
    evtq->eventCount++;
    if (evtq->eventCount > evtq->maxCount) {
        evtq->maxCount = evtq->eventCount;
    }
    if (delta == N_EVTQ_END) {
        postAtEnd = -1;
    } else {
        D_80105020 += delta;
        if (D_80105020 < clock->time) {
            D_80105020 = clock->time;
        }
        delta = D_80105020 - clock->time;
    }
    for (node = &evtq->allocList; node != 0; node = node->next) {
        if (!node->next) {
            if (postAtEnd) {
                item->delta = 0;
            } else {
                item->delta = delta;
            }
            alLink((ALLink *) item, node);
            break;
        } else {
            nextItem = (NEventListItem *) node->next;
            if (delta < nextItem->delta) {
                item->delta = delta;
                nextItem->delta -= delta;
                alLink((ALLink *) item, node);
                break;
            }
            delta -= nextItem->delta;
        }
    }
    osSetIntMask(mask);
}

/* JFG: drop every queued event of a type (libultra alEvtqFlushType). */
void func_8008BA30(NEventQueue *evtq, s16 type) {
    ALLink *thisNode;
    ALLink *nextNode;
    NEventListItem *thisItem;
    NEventListItem *nextItem;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    thisNode = evtq->allocList.next;
    while (thisNode != 0) {
        nextNode = thisNode->next;
        thisItem = (NEventListItem *) thisNode;
        nextItem = (NEventListItem *) nextNode;
        if (thisItem->evt.type == type) {
            if (nextItem) {
                nextItem->delta += thisItem->delta;
            }
            alUnlink(thisNode);
            alLink(thisNode, &evtq->freeList);
            evtq->eventCount--;
        }
        thisNode = nextNode;
    }
    osSetIntMask(mask);
}

/* JFG: add a client to the synthesizer (libultra alSynAddPlayer). */
void func_8008BB10(NPlayer *client) {
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    client->samplesLeft = D_800A97C4->curSamples;
    client->next = D_800A97C4->head;
    D_800A97C4->head = client;
    osSetIntMask(mask);
}

/* JFG: the same for the sound player. */
void func_8008BB80(NPlayer *client) {
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    client->samplesLeft = D_800A97C4->curSamples;
    client->next = D_800A97C4->head;
    D_800A97C4->head = client;
    osSetIntMask(mask);
}

/* JFG: the same for the sequence player. */
void func_8008BBF0(NPlayer *client) {
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    client->samplesLeft = D_800A97C4->curSamples;
    client->next = D_800A97C4->head;
    D_800A97C4->head = client;
    osSetIntMask(mask);
}

s32 func_8008BE38(NPVoice **pvoice, s16 priority);

/* JFG: give a voice a physical voice, stealing one of lower priority if needed (libultra alSynAllocVoice). */
s32 func_8008BC60(NVoice *voice, NVoiceConfig *vc) {
    NPVoice *pvoice = 0;
    UNUSED void *f;
    NParam *update;
    s32 stolen;

    voice->priority = vc->priority;
    voice->unityPitch = vc->unityPitch;
    voice->table = 0;
    voice->fxBus = vc->fxBus;
    voice->state = 0;
    voice->pvoice = 0;
    stolen = func_8008BE38(&pvoice, vc->priority);
    if (pvoice) {
        if (stolen) {
            pvoice->offset = 512 + 40;
            pvoice->vvoice->pvoice = 0;
            pvoice->vvoice = voice;
            voice->pvoice = pvoice;
            update = func_8008AF34();
            if (update) {
                update->delta = D_800A97C4->paramSamples;
                update->type = N_FILTER_RESET;
                update->data = 0;
                update->moredata = 0x170;
                func_80090B14(voice->pvoice, N_FILTER_ADD_UPDATE, update);
            } else {
            }
            update = func_8008AF34();
            if (update) {
                update->delta = D_800A97C4->paramSamples + pvoice->offset;
                update->type = N_FILTER_STOP_VOICE;
                update->next = 0;
                func_80090B14(voice->pvoice, N_FILTER_ADD_UPDATE, update);
            } else {
            }
        } else {
            pvoice->offset = 0;
            pvoice->vvoice = voice;
            voice->pvoice = pvoice;
        }
    }
    return (pvoice != 0);
}

/* JFG: take a physical voice: a lame one, a free one, or the lowest-priority one that is not ramping
   (stolen: returns TRUE). */
s32 func_8008BE38(NPVoice **pvoice, s16 priority) {
    ALLink *dl;
    NPVoice *pv;
    s32 stolen = 0;

    if ((dl = D_800A97C4->pLameList.next) != 0) {
        *pvoice = (NPVoice *) dl;
        alUnlink(dl);
        alLink(dl, &D_800A97C4->pAllocList);
    } else if ((dl = D_800A97C4->pFreeList.next) != 0) {
        *pvoice = (NPVoice *) dl;
        alUnlink(dl);
        alLink(dl, &D_800A97C4->pAllocList);
    } else {
        for (dl = D_800A97C4->pAllocList.next; dl != 0; dl = dl->next) {
            pv = (NPVoice *) dl;
            if (pv->vvoice->priority <= priority && pv->offset == 0) {
                *pvoice = pv;
                priority = pv->vvoice->priority;
                stolen = 1;
            }
        }
    }
    return stolen;
}
