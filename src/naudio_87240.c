/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): the compact sequence player
 * (cf. libultra's csplayer.c).
 */
#include "common.h"

void alLink(ALLink *element, ALLink *after);
void alUnlink(ALLink *element);

/* JFG: an n_audio event (0x10 bytes) */
typedef struct NEvent {
    /* 0x00 */ s16 type;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ union {
        struct {
            /* 0x04 */ s32 ticks;
            /* 0x08 */ u8 status;
            /* 0x09 */ u8 type;
            /* 0x0A */ u8 len;
            /* 0x0B */ u8 byte1;
            /* 0x0C */ u8 byte2;
            /* 0x0D */ u8 byte3;
        } tempo;
    } msg;
} NEvent;

/* JFG: the tempo part of an event (from its ticks on) */
typedef struct NTempoEvent {
    /* 0x00 */ s32 ticks;
    /* 0x04 */ u8 status;
    /* 0x05 */ u8 type;
    /* 0x06 */ u8 len;
    /* 0x07 */ u8 byte1;
    /* 0x08 */ u8 byte2;
    /* 0x09 */ u8 byte3;
} NTempoEvent;

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
    /* 0x12 */ u16 lostCount;
    /* 0x14 */ s16 maxCount;
} NEventQueue;

/* JFG: a compact sequence (only the field used here) */
typedef struct NCSeq {
    /* 0x00 */ u8 pad0[0x8];
    /* 0x08 */ f32 qnpt;
} NCSeq;

/* JFG: a sequence player's channel state (0x34 bytes) */
typedef struct NChanState {
    /* 0x00 */ u8 pad0[0x11];
    /* 0x11 */ u8 filterGain;
    /* 0x12 */ s8 filterKey; // JFG: 0x40 = no shift
    /* 0x13 */ u8 pad13;
    /* 0x14 */ f32 filterScale;
    /* 0x18 */ u8 pad18[0x34 - 0x18];
} NChanState;

/* JFG: an n_audio voice (0x1C bytes) */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x1C];
} NVoice;

/* JFG: the key map of a sound (only the field used here) */
typedef struct NKeyMap {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ u8 keyBase;
} NKeyMap;

typedef struct NSound {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ NKeyMap *keyMap;
} NSound;

/* JFG: a sequence player's voice state */
typedef struct NVoiceState {
    /* 0x00 */ struct NVoiceState *next;
    /* 0x04 */ NVoice voice;
    /* 0x20 */ NSound *sound;
    /* 0x24 */ u8 pad24[0x31 - 0x24];
    /* 0x31 */ u8 channel;
    /* 0x32 */ u8 key;
    /* 0x33 */ u8 pad33;
    /* 0x34 */ u8 envPhase;
    /* 0x35 */ u8 pad35[0x40 - 0x35];
} NVoiceState;

/* JFG: an n_audio compact sequence player (libultra ALCSPlayer) */
typedef struct NCSPlayer {
    /* 0x00 */ void *nodeNext;
    /* 0x04 */ void *clientData;
    /* 0x08 */ void *handler;
    /* 0x0C */ u8 padC[0x14 - 0xC];
    /* 0x14 */ void *drvr;
    /* 0x18 */ NCSeq *target;
    /* 0x1C */ s32 curTime;
    /* 0x20 */ void *bank;
    /* 0x24 */ s32 uspt;
    /* 0x28 */ s32 nextDelta;
    /* 0x2C */ s32 state;
    /* 0x30 */ u16 chanMask;
    /* 0x32 */ s16 vol;
    /* 0x34 */ u8 maxChannels;
    /* 0x35 */ u8 debugFlags;
    /* 0x36 */ u8 pad36[2];
    /* 0x38 */ s16 unk38;
    /* 0x3A */ u8 pad3A[0x48 - 0x3A];
    /* 0x48 */ NEventQueue evtq;
    /* 0x60 */ s32 frameTime;
    /* 0x64 */ NChanState *chanState;
    /* 0x68 */ NVoiceState *vAllocHead;
    /* 0x6C */ NVoiceState *vAllocTail;
    /* 0x70 */ NVoiceState *vFreeList;
    /* 0x74 */ void *initOsc;
    /* 0x78 */ void *updateOsc;
    /* 0x7C */ void *stopOsc;
    /* 0x80 */ f32 unk80;
    /* 0x84 */ f32 unk84;
    /* 0x88 */ void *msgQ;
    /* 0x8C */ u8 maxVoices;
    /* 0x8D */ u8 unk8D;
} NCSPlayer;

/* JFG: a sequence player's configuration (libultra ALSeqpConfig) */
typedef struct NSeqpConfig {
    /* 0x00 */ s32 maxVoices;
    /* 0x04 */ s32 maxEvents;
    /* 0x08 */ u8 maxChannels;
    /* 0x09 */ u8 debugFlags;
    /* 0x0C */ ALHeap *heap;
    /* 0x10 */ void *initOsc;
    /* 0x14 */ void *updateOsc;
    /* 0x18 */ void *stopOsc;
} NSeqpConfig;

extern void *D_800A97C4; // JFG: the n_audio synthesizer
void func_8008C500(NCSPlayer *seqp);                                    // n_audio: all channels on
void func_8008D148(NCSPlayer *seqp);                                    // n_audio: init the channel states
void func_8008B4B0(NEventQueue *evtq, NEventListItem *items, s32 count); // n_audio: alEvtqNew
void func_8008BBF0(NCSPlayer *seqp);                                    // n_audio: add a sequence player
void func_8008B610(NEventQueue *evtq, NEvent *evt, s32 delta);          // n_audio: alEvtqPostEvent
s16 func_8008CD60(NVoiceState *vs, NCSPlayer *seqp);                     // n_audio: a voice state's volume
s32 func_8008CF90(NVoiceState *vs, s32 t);                               // n_audio: a voice state's delta
void func_8008BF70(NVoice *voice, s16 vol, s32 delta);                   // n_audio: set a voice's volume
void func_8008D950(NVoice *voice, s16 gain);                             // n_audio: set a voice's filter gain
void func_8008D8A0(NVoice *voice, f32 freq);                             // n_audio: set a voice's filter frequency
f32 func_8008DCB4(s32 semitones);                                        // n_audio: semitones to a ratio
s32 func_8008E9B0(NCSeq *seq, s32 *deltaTicks);                          // n_audio: __alCSeqNextDelta
void func_80089768(NCSPlayer *seqp, f32 tempo);
void func_80089654(NEventQueue *evtq, NEventListItem *item);

void func_800868B0(void);

/* JFG: set up a compact sequence player (libultra alCSPNew). */
void func_80086640(NCSPlayer *seqp, NSeqpConfig *c) {
    s32 i;
    UNUSED s32 pad[2];
    NEventListItem *items;
    NVoiceState *vs;
    NVoiceState *voices;
    ALHeap *hp = c->heap;

    seqp->bank = 0;
    seqp->target = NULL;
    seqp->drvr = D_800A97C4;
    seqp->chanMask = 0xFFFF;
    func_8008C500(seqp);
    seqp->uspt = 488;
    seqp->nextDelta = 0;
    seqp->state = 0;
    seqp->vol = 0x7FFF;
    seqp->debugFlags = c->debugFlags;
    seqp->frameTime = 16000;
    seqp->curTime = 0;
    seqp->initOsc = c->initOsc;
    seqp->updateOsc = c->updateOsc;
    seqp->stopOsc = c->stopOsc;
    seqp->unk80 = 0.0f;
    seqp->unk84 = 1.0f;
    seqp->msgQ = 0;
    seqp->unk8D = 0;
    seqp->maxVoices = c->maxVoices;
    seqp->unk38 = 9;
    seqp->maxChannels = c->maxChannels;
    seqp->chanState = alHeapAlloc(hp, c->maxChannels, sizeof(NChanState));
    func_8008D148(seqp);
    voices = alHeapAlloc(hp, c->maxVoices, sizeof(NVoiceState));
    seqp->vFreeList = 0;
    for (i = 0; i < c->maxVoices; i++) {
        vs = &voices[i];
        vs->next = seqp->vFreeList;
        seqp->vFreeList = vs;
    }
    seqp->vAllocHead = 0;
    seqp->vAllocTail = 0;
    items = alHeapAlloc(hp, c->maxEvents, sizeof(NEventListItem));
    func_8008B4B0(&seqp->evtq, items, c->maxEvents);
    seqp->nodeNext = NULL;
    seqp->handler = func_800868B0;
    seqp->clientData = seqp;
    func_8008BBF0(seqp);
}

INCLUDE_ASM("asm/nonmatchings/naudio_87240", func_800868B0);

INCLUDE_ASM("asm/nonmatchings/naudio_87240", func_8008712C);

/* JFG: update the volume of a channel's playing voices. */
void func_8008723C(NCSPlayer *seqp, u8 chan) {
    NVoiceState *vs;
    s16 vol;

    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
        if (vs->channel == chan && vs->envPhase != 3) {
            vol = func_8008CD60(vs, seqp);
            func_8008BF70(&vs->voice, vol, func_8008CF90(vs, seqp->curTime));
        }
    }
}

/* JFG: update the filter of a channel's voices (gain, and the frequency from the key and the channel's shift). */
void func_800872E8(NCSPlayer *seqp, u8 chan) {
    NVoiceState *vs;
    s16 gain;
    s8 shift;
    f32 scale;

    shift = seqp->chanState[chan].filterKey - 64;
    scale = seqp->chanState[chan].filterScale;
    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
        if (vs->channel == chan) {
            gain = seqp->chanState[chan].filterGain;
            func_8008D950(&vs->voice, gain);
            if (gain != 0) {
                func_8008D8A0(&vs->voice,
                              func_8008DCB4(vs->key - vs->sound->keyMap->keyBase + shift) * 440.0f * scale);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/naudio_87240", func_80087440);

/* JFG: a tempo change: re-time the queued note offs to the new tempo (libultra __CSPHandleMetaMsg). */
void func_80089404(NCSPlayer *seqp, NEvent *event) {
    NTempoEvent *tevt = (NTempoEvent *) &event->msg;
    s32 tempo;
    s32 oldUspt;
    u32 ticks;
    s32 tempDelta;
    s32 curDelta = 0;
    NEventListItem *thisNode;
    NEventListItem *nextNode;
    NEventListItem *firstTemp = 0;

    if (event->msg.tempo.status == 0xFF) {
        if (event->msg.tempo.type == 0x51) {
            oldUspt = seqp->uspt;
            tempo = (tevt->byte1 << 16) | (tevt->byte2 << 8) | (tevt->byte3 << 0);
            func_80089768(seqp, (f32) tempo);
            thisNode = (NEventListItem *) seqp->evtq.allocList.next;
            while (thisNode) {
                curDelta += thisNode->delta;
                nextNode = (NEventListItem *) thisNode->node.next;
                if (thisNode->evt.type == 0x15) {
                    alUnlink((ALLink *) thisNode);
                    seqp->evtq.eventCount--;
                    if (firstTemp) {
                        alLink((ALLink *) thisNode, (ALLink *) firstTemp);
                    } else {
                        thisNode->node.next = 0;
                        thisNode->node.prev = 0;
                        firstTemp = thisNode;
                    }
                    tempDelta = curDelta;
                    if (nextNode) {
                        curDelta -= thisNode->delta;
                        nextNode->delta += thisNode->delta;
                    }
                    thisNode->delta = tempDelta;
                }
                thisNode = nextNode;
            }
            thisNode = firstTemp;
            while (thisNode) {
                nextNode = (NEventListItem *) thisNode->node.next;
                ticks = thisNode->delta / oldUspt;
                thisNode->delta = ticks * seqp->uspt;
                func_80089654(&seqp->evtq, thisNode);
                thisNode = nextNode;
            }
        }
    }
}

/* JFG: put an event back into the queue at its delta (libultra __CSPRepostEvent). */
void func_80089654(NEventQueue *evtq, NEventListItem *item) {
    OSIntMask mask;
    ALLink *node;
    NEventListItem *nextItem;

    mask = osSetIntMask(OS_IM_NONE);
    for (node = &evtq->allocList; node != 0; node = node->next) {
        if (!node->next) {
            alLink((ALLink *) item, node);
            evtq->eventCount++;
            break;
        } else {
            nextItem = (NEventListItem *) node->next;
            if (item->delta < nextItem->delta) {
                nextItem->delta -= item->delta;
                alLink((ALLink *) item, node);
                evtq->eventCount++;
                break;
            }
            item->delta -= nextItem->delta;
        }
    }
    osSetIntMask(mask);
}

/* JFG: microseconds per tick from a tempo (libultra __setUsptFromTempo). */
void func_80089768(NCSPlayer *seqp, f32 tempo) {
    if (seqp->target) {
        seqp->uspt = (s32) ((f32) tempo * seqp->target->qnpt);
    } else {
        seqp->uspt = 488;
    }
}

/* JFG: queue the sequence's next event (libultra __CSPPostNextSeqEvent). */
void func_800897B0(NCSPlayer *seqp) {
    NEvent evt;
    u32 deltaTicks;

    if (seqp->state != 1 || seqp->target == NULL) {
        return;
    }
    if (!func_8008E9B0(seqp->target, (s32 *) &deltaTicks)) {
        return;
    }
    evt.type = 0;
    func_8008B610(&seqp->evtq, &evt, seqp->uspt * deltaTicks);
}

/* JFG: the most voices the player may use. */
void func_80089844(NCSPlayer *seqp, u8 maxVoices) {
    seqp->maxVoices = maxVoices;
}
