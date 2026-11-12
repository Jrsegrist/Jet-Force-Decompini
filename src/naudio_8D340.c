/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): sequence player internals:
 * voice mapping, voice volume/mix/pan and channel state (cf. libultra's seqplayer.c helpers).
 */
#include "common.h"

void alLink(ALLink *element, ALLink *after);
void alUnlink(ALLink *element);

/* JFG: a sound's envelope */
typedef struct NEnvelope {
    /* 0x00 */ s32 attackTime;
    /* 0x04 */ s32 decayTime;
    /* 0x08 */ s32 releaseTime;
    /* 0x0C */ u8 attackVolume;
    /* 0x0D */ u8 decayVolume;
} NEnvelope;

/* JFG: a sound's key map */
typedef struct NKeyMap {
    /* 0x00 */ u8 velocityMin;
    /* 0x01 */ u8 velocityMax;
    /* 0x02 */ u8 keyMin;
    /* 0x03 */ u8 keyMax;
} NKeyMap;

typedef struct NSound {
    /* 0x00 */ NEnvelope *envelope;
    /* 0x04 */ NKeyMap *keyMap;
    /* 0x08 */ void *wavetable;
    /* 0x0C */ u8 samplePan;
    /* 0x0D */ u8 sampleVolume;
} NSound;

typedef struct NInstrument {
    /* 0x00 */ u8 volume;
    /* 0x01 */ u8 pan;
    /* 0x02 */ u8 priority;
    /* 0x03 */ u8 flags;
    /* 0x04 */ u8 tremType;
    /* 0x05 */ u8 tremRate;
    /* 0x06 */ u8 tremDepth;
    /* 0x07 */ u8 tremDelay;
    /* 0x08 */ u8 vibType;
    /* 0x09 */ u8 vibRate;
    /* 0x0A */ u8 vibDepth;
    /* 0x0B */ u8 vibDelay;
    /* 0x0C */ s16 bendRange;
    /* 0x0E */ s16 soundCount;
    /* 0x10 */ NSound *soundArray[1];
} NInstrument;

typedef struct NBank {
    /* 0x00 */ s16 instCount;
    /* 0x02 */ u8 flags;
    /* 0x03 */ u8 pad3;
    /* 0x04 */ s32 sampleRate;
    /* 0x08 */ NInstrument *percussion;
    /* 0x0C */ NInstrument *instArray[1];
} NBank;

/* JFG: a sequence player's channel state (0x34 bytes) */
typedef struct NChanState {
    /* 0x00 */ NInstrument *instrument;
    /* 0x04 */ s16 bendRange;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 pan;
    /* 0x08 */ u8 priority;
    /* 0x09 */ u8 vol;
    /* 0x0A */ u8 fxmix;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 sustain;
    /* 0x0D */ u8 unkD; // JFG: a second volume scale (0xFF = none)
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ f32 pitchBend;
    /* 0x18 */ s32 attackTime;
    /* 0x1C */ s32 decayTime;
    /* 0x20 */ s32 releaseTime;
    /* 0x24 */ u8 unk24;
    /* 0x25 */ u8 attackVolume;
    /* 0x26 */ u8 decayVolume;
    /* 0x27 */ u8 unk27;
    /* 0x28 */ u8 tremType;
    /* 0x29 */ u8 tremRate;
    /* 0x2A */ u8 tremDepth;
    /* 0x2B */ u8 tremDelay;
    /* 0x2C */ u8 vibType;
    /* 0x2D */ u8 vibRate;
    /* 0x2E */ u8 vibDepth;
    /* 0x2F */ u8 vibDelay;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 unk32;
    /* 0x33 */ u8 pad33;
} NChanState;

/* JFG: an n_audio voice (0x1C bytes) */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ void *clientPrivate;
    /* 0x14 */ u8 pad14[0x1C - 0x14];
} NVoice;

/* JFG: a sequence player's voice state */
typedef struct NVoiceState {
    /* 0x00 */ struct NVoiceState *next;
    /* 0x04 */ NVoice voice;
    /* 0x20 */ NSound *sound;
    /* 0x24 */ s32 envEndTime;
    /* 0x28 */ u8 pad28[0x30 - 0x28];
    /* 0x30 */ u8 envGain;
    /* 0x31 */ u8 channel;
    /* 0x32 */ u8 key;
    /* 0x33 */ u8 velocity;
    /* 0x34 */ u8 envPhase;
    /* 0x35 */ u8 phase;
    /* 0x36 */ u8 tremelo;
    /* 0x37 */ u8 flags;
} NVoiceState;

/* JFG: an n_audio event (0x10 bytes) */
typedef struct NEvent {
    /* 0x00 */ s16 type;
    /* 0x02 */ u8 pad2[2];
    /* 0x04 */ void *msg0; // JFG: the voice, or the voice state of an oscillator event
    /* 0x08 */ void *msg1; // JFG: an oscillator's state
    /* 0x0C */ u8 padC[4];
} NEvent;

typedef struct NEventListItem {
    /* 0x00 */ ALLink node;
    /* 0x08 */ s32 delta;
    /* 0x0C */ NEvent evt;
} NEventListItem;

typedef struct NEventQueue {
    /* 0x00 */ ALLink freeList;
    /* 0x08 */ ALLink allocList;
    /* 0x10 */ s16 eventCount;
    /* 0x12 */ u16 lostCount;
    /* 0x14 */ s16 maxCount;
} NEventQueue;

/* JFG: an n_audio sequence player (only the fields used here) */
typedef struct NSeqPlayer {
    /* 0x00 */ u8 pad0[0x1C];
    /* 0x1C */ s32 curTime;
    /* 0x20 */ u8 pad20[0x32 - 0x20];
    /* 0x32 */ s16 vol;
    /* 0x34 */ u8 maxChannels;
    /* 0x35 */ u8 pad35[0x48 - 0x35];
    /* 0x48 */ NEventQueue evtq;
    /* 0x60 */ s32 frameTime;
    /* 0x64 */ NChanState *chanState;
    /* 0x68 */ NVoiceState *vAllocHead;
    /* 0x6C */ NVoiceState *vAllocTail;
    /* 0x70 */ NVoiceState *vFreeList;
    /* 0x74 */ void *initOsc;
    /* 0x78 */ void *updateOsc;
    /* 0x7C */ void (*stopOsc)(void *oscState);
    /* 0x80 */ f32 unk80;
    /* 0x84 */ f32 unk84;
    /* 0x88 */ void *msgQ;
    /* 0x8C */ u8 maxVoices;
    /* 0x8D */ u8 voiceCount;
} NSeqPlayer;

void func_80091260(NVoice *voice, s16 priority);              // n_audio: set a voice's priority
void func_8008BF70(NVoice *voice, s16 vol, s32 delta);        // n_audio: set a voice's volume
void func_8008B610(NEventQueue *evtq, NEvent *evt, s32 delta); // n_audio: alEvtqPostEvent
void func_8008D1D4(NSeqPlayer *seqp, s32 chan);
void func_8008D424(NSeqPlayer *seqp, NInstrument *inst, s32 chan);

/* JFG: move a voice's state back to the free list (libultra __unmapVoice). */
void func_8008C740(NSeqPlayer *seqp, NVoice *voice) {
    NVoiceState *prev = 0;
    NVoiceState *vs;

    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
        if (&vs->voice == voice) {
            if (prev) {
                prev->next = vs->next;
            } else {
                seqp->vAllocHead = vs->next;
            }
            if (seqp->vAllocTail == vs) {
                seqp->vAllocTail = prev;
            }
            vs->next = seqp->vFreeList;
            seqp->vFreeList = vs;
            seqp->voiceCount--;
            return;
        }
        prev = vs;
    }
}

/* JFG: release a voice: drop its pending envelope events, ramp it down and queue its end (libultra
   __seqpReleaseVoice). */
void func_8008C800(NSeqPlayer *seqp, NVoice *voice, s32 deltaTime) {
    NEvent evt;
    NVoiceState *vs = (NVoiceState *) voice->clientPrivate;

    if (vs->envPhase == 0) {
        ALLink *thisNode;
        ALLink *nextNode;
        NEventListItem *thisItem, *nextItem;

        thisNode = seqp->evtq.allocList.next;
        while (thisNode != 0) {
            nextNode = thisNode->next;
            thisItem = (NEventListItem *) thisNode;
            nextItem = (NEventListItem *) nextNode;
            if (thisItem->evt.type == 6) {
                if (thisItem->evt.msg0 == voice) {
                    if (nextItem) {
                        nextItem->delta += thisItem->delta;
                    }
                    alUnlink(thisNode);
                    alLink(thisNode, &seqp->evtq.freeList);
                }
            }
            thisNode = nextNode;
        }
    }
    vs->velocity = 0;
    vs->envPhase = 3;
    vs->envGain = 0;
    vs->envEndTime = seqp->curTime + deltaTime;
    func_80091260(voice, 0);
    func_8008BF70(voice, 0, deltaTime);
    evt.type = 5;
    evt.msg0 = voice;
    deltaTime += 16000 * 2;
    func_8008B610(&seqp->evtq, &evt, deltaTime);
}

/* JFG: whether a voice still needs a note kill: drops its note end event if that comes after killTime
   (libultra __voiceNeedsNoteKill). */
char func_8008C974(NSeqPlayer *seqp, NVoice *voice, s32 killTime) {
    ALLink *thisNode;
    ALLink *nextNode;
    NEventListItem *thisItem;
    s32 itemTime = 0;
    char needNoteKill = TRUE;

    thisNode = seqp->evtq.allocList.next;
    while (thisNode != 0) {
        nextNode = thisNode->next;
        thisItem = (NEventListItem *) thisNode;
        itemTime += thisItem->delta;
        if (thisItem->evt.type == 5) {
            if (thisItem->evt.msg0 == voice) {
                if (itemTime > killTime) {
                    if ((NEventListItem *) nextNode) {
                        ((NEventListItem *) nextNode)->delta += thisItem->delta;
                    }
                    alUnlink(thisNode);
                    alLink(thisNode, &seqp->evtq.freeList);
                } else {
                    needNoteKill = FALSE;
                }
                break;
            }
        }
        thisNode = nextNode;
    }
    return needNoteKill;
}

/* JFG: give a note a voice state (none past the voice limit) (libultra __mapVoice). */
NVoiceState *func_8008CA94(NSeqPlayer *seqp, u8 key, u8 vel, u8 channel) {
    NVoiceState *vs = seqp->vFreeList;

    if (seqp->voiceCount > seqp->maxVoices) {
        return 0;
    }
    if (vs) {
        seqp->vFreeList = vs->next;
        vs->next = 0;
        if (!seqp->vAllocHead) {
            seqp->vAllocHead = vs;
        } else {
            seqp->vAllocTail->next = vs;
        }
        seqp->vAllocTail = vs;
        vs->channel = channel;
        vs->key = key;
        vs->velocity = vel;
        vs->voice.clientPrivate = vs;
        seqp->voiceCount++;
    }
    return vs;
}

/* JFG: the playing voice state of a key on a channel (libultra __lookupVoice). */
NVoiceState *func_8008CB58(NSeqPlayer *seqp, u8 key, u8 channel) {
    NVoiceState *vs;

    vs = seqp->vAllocHead;
    while (vs != 0) {
        if (vs->key == key && vs->channel == channel && vs->phase != 3 && vs->phase != 4) {
            return vs;
        }
        vs = vs->next;
    }
    return 0;
}

/* JFG: the sound of the channel's instrument for a key and velocity, by binary search (libultra
   __lookupSoundQuick). */
NSound *func_8008CBE8(NSeqPlayer *seqp, u8 key, u8 vel, u8 chan) {
    NInstrument *inst = seqp->chanState[chan].instrument;
    s32 l = 1;
    s32 r = inst->soundCount;
    s32 i;
    NKeyMap *keymap;

    while (r >= l) {
        i = (l + r) / 2;
        keymap = inst->soundArray[i - 1]->keyMap;
        if (key >= keymap->keyMin && key <= keymap->keyMax && vel >= keymap->velocityMin &&
            vel <= keymap->velocityMax) {
            return inst->soundArray[i - 1];
        } else if (key < keymap->keyMin || (vel < keymap->velocityMin && key <= keymap->keyMax)) {
            r = i - 1;
        } else {
            l = i + 1;
        }
    }
    return 0;
}

/* JFG: a voice state's volume (libultra __vsVol; also the channel's second volume scale). */
s16 func_8008CD60(NVoiceState *vs, NSeqPlayer *seqp) {
    u32 t1, t2;

    t1 = (vs->tremelo * vs->velocity * vs->envGain) >> 6;
    t2 = (vs->sound->sampleVolume * seqp->vol * seqp->chanState[vs->channel].vol) >> 14;
    if (seqp->chanState[vs->channel].unkD != 0xFF) {
        t2 = (seqp->chanState[vs->channel].unkD * t2 + 1) >> 8;
    }
    t1 *= t2;
    t1 >>= 15;
    return (s16) t1;
}

#define NMIN(a, b) (((a) > (b)) ? (b) : (a))
#define NMAX(a, b) (((a) < (b)) ? (b) : (a))

/* JFG: a voice state's effect mix: the channel's, plus the player's offset, scaled (top bit kept). */
u8 func_8008CE88(NVoiceState *vs, NSeqPlayer *seqp) {
    s32 flag;
    s32 tmp;

    flag = seqp->chanState[vs->channel].fxmix & 0x80;
    tmp = ((seqp->chanState[vs->channel].fxmix & 0x7F) + (s32) (seqp->unk80 * 127.0f)) * seqp->unk84;
    return NMAX(NMIN(tmp, 127), 0) | flag;
}

/* JFG: the time left of a voice state's envelope phase (libultra __vsDelta). */
s32 func_8008CF90(NVoiceState *vs, s32 t) {
    s32 delta = vs->envEndTime - t;

    if ((s32) delta >= 0) {
        return delta;
    } else {
        return 1000;
    }
}

/* JFG: a voice state's pan (libultra __vsPan). */
u8 func_8008CFD4(NVoiceState *vs, NSeqPlayer *seqp) {
    s32 tmp;

    tmp = seqp->chanState[vs->channel].pan - 64 + vs->sound->samplePan;
    tmp = (tmp > 0) ? tmp : 0;
    tmp = (tmp < 127) ? tmp : 127;
    return (u8) tmp;
}

/* JFG: give every channel the bank's first instrument (and channel 9 its percussion) (libultra __initFromBank). */
void func_8008D060(NSeqPlayer *seqp, NBank *b) {
    s32 i;
    NInstrument *inst = 0;

    for (i = 0; !inst; i++) {
        inst = b->instArray[i];
    }
    for (i = 0; i < seqp->maxChannels; i++) {
        func_8008D1D4(seqp, i);
        func_8008D424(seqp, inst, i);
    }
    if (b->percussion) {
        func_8008D1D4(seqp, i);
        func_8008D424(seqp, b->percussion, 9);
    }
}

/* JFG: clear every channel's state (libultra __initChanState). */
void func_8008D148(NSeqPlayer *seqp) {
    int i;

    for (i = 0; i < seqp->maxChannels; i++) {
        seqp->chanState[i].instrument = 0;
        func_8008D1D4(seqp, i);
    }
}

/* JFG: a channel's performance defaults (libultra __resetPerfChanState). */
void func_8008D1D4(NSeqPlayer *seqp, s32 chan) {
    seqp->chanState[chan].unk6 = 0;
    seqp->chanState[chan].fxmix = 0;
    seqp->chanState[chan].pan = 64;
    seqp->chanState[chan].vol = 127;
    seqp->chanState[chan].priority = 5;
    seqp->chanState[chan].sustain = 0;
    seqp->chanState[chan].bendRange = 200;
    seqp->chanState[chan].pitchBend = 1.0f;
    seqp->chanState[chan].unk10 = 0;
    seqp->chanState[chan].unkD = 0xFF;
    seqp->chanState[chan].unkE = 0xFF;
    seqp->chanState[chan].unkF = 0;
    seqp->chanState[chan].unkB = 0;
    seqp->chanState[chan].unk13 = 0;
    seqp->chanState[chan].unk12 = 0;
    seqp->chanState[chan].unk11 = 0;
    seqp->chanState[chan].unk32 = 0;
}

/* JFG: a channel's instrument settings (libultra __setInstChanState). */
void func_8008D424(NSeqPlayer *seqp, NInstrument *inst, s32 chan) {
    NSound *sound;

    seqp->chanState[chan].instrument = inst;
    seqp->chanState[chan].pan = inst->pan;
    seqp->chanState[chan].vol = inst->volume;
    seqp->chanState[chan].priority = inst->priority;
    seqp->chanState[chan].bendRange = inst->bendRange;
    if (inst->soundCount == 0) {
        return;
    }
    sound = inst->soundArray[0];
    seqp->chanState[chan].attackTime = sound->envelope->attackTime;
    seqp->chanState[chan].decayTime = sound->envelope->decayTime;
    seqp->chanState[chan].releaseTime = sound->envelope->releaseTime;
    seqp->chanState[chan].attackVolume = sound->envelope->attackVolume;
    seqp->chanState[chan].decayVolume = sound->envelope->decayVolume;
    seqp->chanState[chan].unk27 = 0;
    seqp->chanState[chan].tremType = inst->tremType;
    seqp->chanState[chan].tremRate = inst->tremRate;
    seqp->chanState[chan].tremDepth = inst->tremDepth;
    seqp->chanState[chan].tremDelay = inst->tremDelay;
    seqp->chanState[chan].vibType = inst->vibType;
    seqp->chanState[chan].vibRate = inst->vibRate;
    seqp->chanState[chan].vibDepth = inst->vibDepth;
    seqp->chanState[chan].vibDelay = inst->vibDelay;
    seqp->chanState[chan].unk24 = 0;
    seqp->chanState[chan].unk31 = 0;
}

/* JFG: stop a voice state's oscillators and drop their events (libultra __seqpStopOsc). */
void func_8008D760(NSeqPlayer *seqp, NVoiceState *vs) {
    NEventListItem *thisNode;
    NEventListItem *nextNode;
    s16 evtType;

    thisNode = (NEventListItem *) seqp->evtq.allocList.next;
    while (thisNode != 0) {
        nextNode = (NEventListItem *) thisNode->node.next;
        evtType = thisNode->evt.type;
        if (evtType == 0x16 || evtType == 0x17) {
            if (thisNode->evt.msg0 == vs) {
                (*seqp->stopOsc)(thisNode->evt.msg1);
                alUnlink((ALLink *) thisNode);
                if (nextNode) {
                    nextNode->delta += thisNode->delta;
                }
                alLink((ALLink *) thisNode, &seqp->evtq.freeList);
                if (evtType == 0x16) {
                    vs->flags = vs->flags & 0xFE;
                } else {
                    vs->flags = vs->flags & 0xFD;
                }
                if (vs->flags == 0) {
                    return;
                }
            }
        }
        thisNode = nextNode;
    }
}
