/*
 * n_audio (the game's n_audio library, built unoptimised with debug information): the compact sequence player
 * (cf. libultra's csplayer.c).
 */
#include "common.h"

void alLink(ALLink *element, ALLink *after);
void alUnlink(ALLink *element);

/* JFG: a MIDI message (libultra ALMIDIEvent; the compact sequence's note duration in ticks) */
typedef struct NMidiEvent {
    /* 0x00 */ s32 ticks;
    /* 0x04 */ u8 status;
    /* 0x05 */ u8 byte1;
    /* 0x06 */ u8 byte2;
    /* 0x08 */ u32 duration;
} NMidiEvent;

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
        NMidiEvent midi;
        struct {
            /* 0x04 */ struct NVoice *voice;
        } note;
        struct {
            /* 0x04 */ struct NVoice *voice;
            /* 0x08 */ s32 delta;
            /* 0x0C */ u8 vol;
        } vol;
        struct {
            /* 0x04 */ struct NVoiceState *vs;
            /* 0x08 */ void *oscState;
            /* 0x0C */ u8 chan;
        } osc;
        struct {
            /* 0x04 */ s16 vol;
        } spvol;
        struct {
            /* 0x04 */ u8 chan;
            /* 0x05 */ u8 priority;
        } sppriority;
        struct {
            /* 0x04 */ struct NCSeq *seq;
        } spseq;
        struct {
            /* 0x04 */ void *bank;
        } spbank;
        struct {
            /* 0x04 */ f32 a;
            /* 0x08 */ f32 b;
        } fxmix; // JFG: the player's two effect mix scales (unk80, unk84)
        struct {
            /* 0x04 */ u8 bus;
            /* 0x05 */ u8 param; // < 8: an effect parameter, else the output low-pass's
            /* 0x06 */ u8 delay;
            /* 0x08 */ s32 value;
        } fxparam;
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

/* JFG: an instrument (libultra ALInstrument; only the fields used here) */
typedef struct NInstrument {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ u8 tremType;
    /* 0x05 */ u8 tremRate;
    /* 0x06 */ u8 tremDepth;
    /* 0x07 */ u8 tremDelay;
    /* 0x08 */ u8 vibType;
    /* 0x09 */ u8 vibRate;
    /* 0x0A */ u8 vibDepth;
    /* 0x0B */ u8 vibDelay;
} NInstrument;

/* JFG: a bank (libultra ALBank; only the fields used here) */
typedef struct NBank {
    /* 0x00 */ s16 instCount;
    /* 0x02 */ u8 pad2[0xC - 0x2];
    /* 0x0C */ NInstrument *instArray[1];
} NBank;

/* JFG: a sequence player's channel state (0x34 bytes; the fields from 0x18 on override the instrument's when
   0x24 is set) */
typedef struct NChanState {
    /* 0x00 */ NInstrument *instrument;
    /* 0x04 */ s16 bendRange;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 pan;
    /* 0x08 */ u8 priority;
    /* 0x09 */ u8 vol;
    /* 0x0A */ u8 fxmix; // JFG: bit 7: a second effect bus flag
    /* 0x0B */ u8 fxBus;
    /* 0x0C */ u8 sustain;
    /* 0x0D */ u8 unkD; // JFG: a second volume scale (0xFF = none), ramped towards unkE at rate unkF
    /* 0x0E */ u8 unkE;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 msg; // JFG: bit 0: note-on messages, bit 1: note-off messages, bits 2-7: their value
    /* 0x11 */ u8 filterGain;
    /* 0x12 */ s8 filterKey; // JFG: 0x40 = no shift
    /* 0x13 */ u8 distortion;
    /* 0x14 */ f32 pitchBend;
    /* 0x18 */ s32 attackTime;
    /* 0x1C */ s32 decayTime;
    /* 0x20 */ s32 releaseTime;
    /* 0x24 */ u8 override;
    /* 0x25 */ u8 attackVolume;
    /* 0x26 */ u8 decayVolume;
    /* 0x27 */ s8 detune;
    /* 0x28 */ u8 tremType;
    /* 0x29 */ u8 tremRate;
    /* 0x2A */ u8 tremDepth;
    /* 0x2B */ u8 tremDelay;
    /* 0x2C */ u8 vibType;
    /* 0x2D */ u8 vibRate;
    /* 0x2E */ u8 vibDepth;
    /* 0x2F */ u8 vibDelay;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 oscParam; // JFG: the oscillators' extra parameter
    /* 0x32 */ u8 bankHi;   // JFG: the high part of the program number
    /* 0x33 */ u8 pad33;
} NChanState;

/* JFG: the oscillator state fields the vibrato depth controller sets */
typedef struct NOscState {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ u8 type;
    /* 0x05 */ u8 pad5[0xC - 0x5];
    /* 0x0C */ f32 depthA;
    /* 0x10 */ f32 depthB;
} NOscState;

/* JFG: an n_audio voice configuration (libultra ALVoiceConfig) */
typedef struct NVoiceConfig {
    /* 0x00 */ s16 priority;
    /* 0x02 */ s16 fxBus;
    /* 0x04 */ u8 unityPitch;
} NVoiceConfig;

/* JFG: an envelope (libultra ALEnvelope) */
typedef struct NEnvelope {
    /* 0x00 */ s32 attackTime;
    /* 0x04 */ s32 decayTime;
    /* 0x08 */ s32 releaseTime;
    /* 0x0C */ u8 attackVolume;
    /* 0x0D */ u8 decayVolume;
} NEnvelope;

/* JFG: an n_audio synthesizer (only the field used here) */
typedef struct NSynth {
    /* 0x00 */ u8 pad0[0x3C];
    /* 0x3C */ s32 maxAuxBusses;
} NSynth;

/* JFG: an n_audio voice (0x1C bytes) */
typedef struct NVoice {
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ void *clientPrivate;
    /* 0x14 */ u8 pad14[0x1C - 0x14];
} NVoice;

/* JFG: the key map of a sound (only the fields used here) */
typedef struct NKeyMap {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ u8 keyBase;
    /* 0x05 */ s8 detune;
} NKeyMap;

typedef struct NSound {
    /* 0x00 */ NEnvelope *envelope;
    /* 0x04 */ NKeyMap *keyMap;
    /* 0x08 */ void *wavetable;
} NSound;

/* JFG: a sequence player's voice state */
typedef struct NVoiceState {
    /* 0x00 */ struct NVoiceState *next;
    /* 0x04 */ NVoice voice;
    /* 0x20 */ NSound *sound;
    /* 0x24 */ s32 envEndTime;
    /* 0x28 */ f32 pitch;
    /* 0x2C */ f32 vibrato;
    /* 0x30 */ u8 envGain;
    /* 0x31 */ u8 channel;
    /* 0x32 */ u8 key;
    /* 0x33 */ u8 velocity;
    /* 0x34 */ u8 envPhase;
    /* 0x35 */ u8 phase;
    /* 0x36 */ u8 tremelo;
    /* 0x37 */ u8 flags;
    /* 0x38 */ void *tremOsc;
    /* 0x3C */ NOscState *vibOsc;
} NVoiceState;

/* JFG: an n_audio compact sequence player (libultra ALCSPlayer) */
typedef struct NCSPlayer {
    /* 0x00 */ void *nodeNext;
    /* 0x04 */ void *clientData;
    /* 0x08 */ void *handler;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ s32 samplesLeft;
    /* 0x14 */ void *drvr;
    /* 0x18 */ NCSeq *target;
    /* 0x1C */ s32 curTime;
    /* 0x20 */ NBank *bank;
    /* 0x24 */ s32 uspt;
    /* 0x28 */ s32 nextDelta;
    /* 0x2C */ s32 state;
    /* 0x30 */ u16 chanMask;
    /* 0x32 */ s16 vol;
    /* 0x34 */ u8 maxChannels;
    /* 0x35 */ u8 debugFlags;
    /* 0x36 */ u8 pad36[2];
    /* 0x38 */ NEvent nextEvent;
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

extern NSynth *D_800A97C4; // JFG: the n_audio synthesizer
extern s32 D_800A95C0[];   // JFG: the envelope time table (controller values to microseconds)
extern s32 D_80104FD0[];   // JFG: per channel: the last note's duration
NSound *func_8008CBE8(NCSPlayer *seqp, u8 key, u8 vel, u8 chan);         // n_audio: __lookupSoundQuick
NVoiceState *func_8008CA94(NCSPlayer *seqp, u8 key, u8 vel, u8 channel); // n_audio: __mapVoice
NVoiceState *func_8008CB58(NCSPlayer *seqp, u8 key, u8 channel);         // n_audio: __lookupVoice
s32 func_8008BC60(NVoice *voice, NVoiceConfig *vc);                      // n_audio: alSynAllocVoice
f32 func_8008B200(s32 cents);                                            // n_audio: alCents2Ratio
void func_8008C040(NVoice *voice, void *wave, f32 pitch, s16 vol, u8 pan, u8 fxmix, u8 lpGain, f32 lpFreq,
                   u8 distortion, s32 delta);                            // n_audio: alSynStartVoiceParams
u8 func_8008CFD4(NVoiceState *vs, NCSPlayer *seqp);                      // n_audio: __vsPan
void func_8008C170(NVoice *v, u8 pan);                                   // n_audio: alSynSetPan
void func_8008E900(NVoice *v, u8 distortion);                            // n_audio: set a voice's distortion
void func_8008D424(NCSPlayer *seqp, NInstrument *inst, s32 chan);        // n_audio: __setInstChanState
f32 func_80004768(u8 depth);                                             // the oscillators' depth in cents
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
void func_80089B70(NCSeq *seq, NEvent *evt, s32 loop);                  // n_audio: alCSeqNextEvent
void func_80087440(NCSPlayer *seqp, NEvent *event);
void func_80089404(NCSPlayer *seqp, NEvent *event);
void func_800897B0(NCSPlayer *seqp);
extern s32 D_800A97BC; // JFG: set: a sequence's end doesn't stop its player

s32 func_800868B0(void *node);
void func_8008712C(NCSPlayer *seqp);
void func_8008C380(NVoice *v);                                           // n_audio: alSynStopVoice
void func_8008C420(NVoice *voice);                                       // n_audio: alSynFreeVoice
void func_8008D760(NCSPlayer *seqp, NVoiceState *vs);                    // n_audio: __seqpStopOsc
void func_8008C740(NCSPlayer *seqp, NVoice *voice);                      // n_audio: __unmapVoice
void func_8008C220(NVoice *v, f32 pitch);                                // n_audio: alSynSetPitch
u8 func_8008CE88(NVoiceState *vs, NCSPlayer *seqp);                      // n_audio: a voice state's effect mix
void func_8008C2D0(NVoice *v, u8 fxmix);                                 // n_audio: alSynSetFXMix
void *func_8008E710(s16 bus);                                            // n_audio: an effect bus's effect
void func_8008E800(void *fx, s32 paramID, void *param);                  // n_audio: set an effect parameter
void *func_8008E788(s16 bus);                                            // n_audio: an effect bus's output low-pass
void func_8008E844(void *lp, s32 paramID, void *param);                  // n_audio: set an output low-pass parameter
void func_8008BA30(NEventQueue *evtq, s16 type);                         // n_audio: alEvtqFlushType
char func_8008C974(NCSPlayer *seqp, NVoice *voice, s32 killTime);        // n_audio: __voiceNeedsNoteKill
void func_8008C800(NCSPlayer *seqp, NVoice *voice, s32 deltaTime);       // n_audio: __seqpReleaseVoice
void func_8008D060(NCSPlayer *seqp, void *bank);                         // n_audio: __initFromBank
s32 func_8008B55C(NEventQueue *evtq, NEvent *evt);                       // n_audio: alEvtqNextEvent

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
    seqp->nextEvent.type = 9;
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

/* JFG: the sequence player's voice handler: handle every event due now (libultra __CSPVoiceHandler; n_ form:
   the effect mix and effect parameter events, a sequence change resets the channel mask). */
s32 func_800868B0(void *node) {
    NCSPlayer *seqp = (NCSPlayer *) node;
    NEvent evt;
    NVoice *voice;
    s32 delta;
    NVoiceState *vs;
    void *oscState;
    f32 oscValue;
    u8 chan;
    UNUSED s32 pad;
    void *fx;
    void *lp;

    do {
        switch (seqp->nextEvent.type) {
            case 0:
                func_8008712C(seqp);
                break;
            case 9:
                evt.type = 9;
                func_8008B610(&seqp->evtq, &evt, seqp->frameTime);
                break;
            case 5:
                voice = seqp->nextEvent.msg.note.voice;
                func_8008C380(voice);
                func_8008C420(voice);
                vs = (NVoiceState *) voice->clientPrivate;
                if (vs->flags) {
                    func_8008D760(seqp, vs);
                }
                func_8008C740(seqp, voice);
                break;
            case 6:
                voice = seqp->nextEvent.msg.vol.voice;
                vs = (NVoiceState *) voice->clientPrivate;
                if (vs->envPhase == 0) {
                    vs->envPhase = 1;
                }
                delta = seqp->nextEvent.msg.vol.delta;
                vs->envEndTime = seqp->curTime + delta;
                vs->envGain = seqp->nextEvent.msg.vol.vol;
                func_8008BF70(voice, func_8008CD60(vs, seqp), delta);
                break;
            case 22:
                vs = seqp->nextEvent.msg.osc.vs;
                oscState = seqp->nextEvent.msg.osc.oscState;
                delta = ((s32 (*)(void *, f32 *)) seqp->updateOsc)(oscState, &oscValue);
                vs->tremelo = (u8) oscValue;
                func_8008BF70(&vs->voice, func_8008CD60(vs, seqp), func_8008CF90(vs, seqp->curTime));
                evt.type = 22;
                evt.msg.osc.vs = vs;
                evt.msg.osc.oscState = oscState;
                func_8008B610(&seqp->evtq, &evt, delta);
                break;
            case 23:
                vs = seqp->nextEvent.msg.osc.vs;
                oscState = seqp->nextEvent.msg.osc.oscState;
                chan = seqp->nextEvent.msg.osc.chan;
                delta = ((s32 (*)(void *, f32 *)) seqp->updateOsc)(oscState, &oscValue);
                vs->vibrato = oscValue;
                func_8008C220(&vs->voice, vs->pitch * vs->vibrato * seqp->chanState[chan].pitchBend);
                if (seqp->chanState[chan].filterGain) {
                    func_8008D8A0(&vs->voice, 440.0f * func_8008DCB4(vs->key - vs->sound->keyMap->keyBase) *
                                                  seqp->chanState[chan].pitchBend * vs->vibrato);
                }
                evt.type = 23;
                evt.msg.osc.vs = vs;
                evt.msg.osc.oscState = oscState;
                evt.msg.osc.chan = chan;
                func_8008B610(&seqp->evtq, &evt, delta);
                break;
            case 2:
            case 21:
                func_80087440(seqp, &seqp->nextEvent);
                break;
            case 7:
                func_80089404(seqp, &seqp->nextEvent);
                break;
            case 10:
                seqp->vol = seqp->nextEvent.msg.spvol.vol;
                for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                    func_8008BF70(&vs->voice, func_8008CD60(vs, seqp), func_8008CF90(vs, seqp->curTime));
                }
                break;
            case 24:
                seqp->unk80 = seqp->nextEvent.msg.fxmix.a;
                seqp->unk84 = seqp->nextEvent.msg.fxmix.b;
                for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                    func_8008C2D0(&vs->voice, func_8008CE88(vs, seqp));
                }
                break;
            case 25:
                if (seqp->nextEvent.msg.fxparam.param < 8) {
                    fx = func_8008E710(seqp->nextEvent.msg.fxparam.bus);
                    if (fx) {
                        func_8008E800(fx, (seqp->nextEvent.msg.fxparam.delay << 3) |
                                              (seqp->nextEvent.msg.fxparam.param & 7),
                                      &seqp->nextEvent.msg.fxparam.value);
                    }
                } else {
                    lp = func_8008E788(seqp->nextEvent.msg.fxparam.bus);
                    if (lp) {
                        func_8008E844(lp, seqp->nextEvent.msg.fxparam.param, &seqp->nextEvent.msg.fxparam.value);
                    }
                }
                break;
            case 15:
                if (seqp->state != 1) {
                    seqp->state = 1;
                    func_800897B0(seqp);
                }
                break;
            case 16:
                if (seqp->state == 2) {
                    for (vs = seqp->vAllocHead; vs != 0; vs = seqp->vAllocHead) {
                        func_8008C380(&vs->voice);
                        func_8008C420(&vs->voice);
                        if (vs->flags) {
                            func_8008D760(seqp, vs);
                        }
                        func_8008C740(seqp, &vs->voice);
                    }
                    seqp->state = 0;
                }
                break;
            case 17:
                if (seqp->state == 1) {
                    func_8008BA30(&seqp->evtq, 0);
                    func_8008BA30(&seqp->evtq, 21);
                    func_8008BA30(&seqp->evtq, 2);
                    func_8008BA30(&seqp->evtq, 22);
                    func_8008BA30(&seqp->evtq, 23);
                    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                        if (func_8008C974(seqp, &vs->voice, 50000)) {
                            func_8008C800(seqp, &vs->voice, 50000);
                        }
                    }
                    seqp->state = 2;
                    evt.type = 16;
                    func_8008B610(&seqp->evtq, &evt, 0x7FFFFFFF);
                }
                break;
            case 12:
                chan = seqp->nextEvent.msg.sppriority.chan;
                seqp->chanState[chan].priority = seqp->nextEvent.msg.sppriority.priority;
                break;
            case 13:
                seqp->target = seqp->nextEvent.msg.spseq.seq;
                seqp->chanMask = 0xFFFF;
                if (seqp->bank) {
                    func_8008D060(seqp, seqp->bank);
                }
                break;
            case 14:
                seqp->bank = seqp->nextEvent.msg.spbank.bank;
                func_8008D060(seqp, seqp->bank);
                break;
            case 4:
            case 3:
            case 1:
                break;
        }
        seqp->nextDelta = func_8008B55C(&seqp->evtq, &seqp->nextEvent);
    } while (seqp->nextDelta == 0);
    seqp->curTime += seqp->nextDelta;
    return seqp->nextDelta;
}

/* JFG: handle the sequence's next event (libultra __CSPHandleNextSeqEvent; the end stops the player only when
   D_800A97BC is clear). */
void func_8008712C(NCSPlayer *seqp) {
    NEvent evt;

    if (seqp->target == NULL) {
        return;
    }
    func_80089B70(seqp->target, &evt, 1);
    switch (evt.type) {
        case 1:
            func_80087440(seqp, &evt);
            func_800897B0(seqp);
            break;
        case 3:
            func_80089404(seqp, &evt);
            func_800897B0(seqp);
            break;
        case 4:
            if (!D_800A97BC) {
                seqp->state = 2;
                evt.type = 16;
                func_8008B610(&seqp->evtq, &evt, 0x7FFFFFFF);
            }
            break;
        case 18:
        case 19:
        case 20:
            func_800897B0(seqp);
            break;
        default:
            break;
    }
}

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
    scale = seqp->chanState[chan].pitchBend;
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

/* JFG: handle a MIDI message (libultra __CSPHandleMIDIMsg; n_ form: muted channels, per-channel envelope and
   oscillator overrides, the filter, distortion, the channel volume ramp, note messages to the player's queue and
   more controllers). */
void func_80087440(NCSPlayer *seqp, NEvent *event) {
    NVoice *voice;
    NVoiceState *vs;
    s32 status;
    u8 chan;
    u8 key;
    u8 vel;
    u8 byte1;
    u8 byte2;
    NMidiEvent *midi = &event->msg.midi;
    s16 vol;
    NEvent evt;
    s32 deltaTime;
    NVoiceState *vstate;
    u8 pan;
    NChanState *cs;
    s32 type;

    status = midi->status & 0xF0;
    chan = midi->status & 0xF;
    byte1 = key = midi->byte1;
    byte2 = vel = midi->byte2;

    switch (status) {
        case 0x90:
            if (vel != 0) {
                NVoiceConfig config;
                NSound *sound;
                s16 cents;
                f32 pitch;
                f32 oscValue;
                u8 fxmix;
                u8 lpGain;
                f32 lpFreq;
                void *oscState;
                NInstrument *inst;

                if (seqp->state != 1 || !(seqp->chanMask & (1 << chan))) {
                    if (midi->duration) {
                        evt.type = 0x15;
                        evt.msg.midi.status = chan | 0x80;
                        evt.msg.midi.byte1 = key;
                        evt.msg.midi.byte2 = 0;
                        deltaTime = seqp->uspt * midi->duration;
                        D_80104FD0[chan] = deltaTime;
                        func_8008B610(&seqp->evtq, &evt, deltaTime);
                    }
                    break;
                }
                cs = &seqp->chanState[chan];
                sound = func_8008CBE8(seqp, key, vel, chan);
                if (!sound) {
                    return;
                }
                config.priority = cs->priority;
                config.fxBus = cs->fxBus;
                config.unityPitch = 0;
                vstate = func_8008CA94(seqp, key, vel, chan);
                if (!vstate) {
                    return;
                }
                voice = &vstate->voice;
                func_8008BC60(voice, &config);
                vstate->sound = sound;
                vstate->envPhase = 0;
                if (cs->sustain > 63) {
                    vstate->phase = 2;
                } else {
                    vstate->phase = 0;
                }
                cents = (key - sound->keyMap->keyBase) * 100 + sound->keyMap->detune;
                if (cs->override) {
                    cents += cs->detune;
                }
                vstate->pitch = func_8008B200(cents);
                if (cs->override) {
                    vstate->envGain = cs->attackVolume;
                    vstate->envEndTime = seqp->curTime + cs->attackTime;
                } else {
                    vstate->envGain = sound->envelope->attackVolume;
                    vstate->envEndTime = seqp->curTime + sound->envelope->attackTime;
                }
                vstate->flags = 0;
                if (cs->override) {
                    type = cs->tremType;
                } else {
                    inst = seqp->chanState[chan].instrument;
                    type = inst->tremType;
                }
                oscValue = 127.0f;
                if (type) {
                    if (seqp->initOsc) {
                        if (cs->override) {
                            deltaTime = ((s32 (*)(void **, f32 *, u8, u8, u8, u8, u8)) seqp->initOsc)(
                                &oscState, &oscValue, cs->tremType, cs->tremRate, cs->tremDepth, cs->tremDelay,
                                cs->oscParam);
                        } else {
                            deltaTime = ((s32 (*)(void **, f32 *, u8, u8, u8, u8, u8)) seqp->initOsc)(
                                &oscState, &oscValue, inst->tremType, inst->tremRate, inst->tremDepth,
                                inst->tremDelay, cs->oscParam);
                        }
                        if (deltaTime) {
                            evt.type = 0x16;
                            evt.msg.osc.vs = vstate;
                            evt.msg.osc.oscState = oscState;
                            func_8008B610(&seqp->evtq, &evt, deltaTime);
                            vstate->flags |= 1;
                            vstate->tremOsc = oscState;
                        }
                    }
                }
                vstate->tremelo = (u8) oscValue;
                oscValue = 1.0f;
                if (cs->override) {
                    type = cs->vibType;
                } else {
                    type = inst->vibType;
                }
                if (type) {
                    if (seqp->initOsc) {
                        if (cs->override) {
                            deltaTime = ((s32 (*)(void **, f32 *, u8, u8, u8, u8, u8)) seqp->initOsc)(
                                &oscState, &oscValue, cs->vibType, cs->vibRate, cs->vibDepth, cs->vibDelay,
                                cs->oscParam);
                        } else {
                            deltaTime = ((s32 (*)(void **, f32 *, u8, u8, u8, u8, u8)) seqp->initOsc)(
                                &oscState, &oscValue, inst->vibType, inst->vibRate, inst->vibDepth,
                                inst->vibDelay, cs->oscParam);
                        }
                        if (deltaTime) {
                            evt.type = 0x17;
                            evt.msg.osc.vs = vstate;
                            evt.msg.osc.oscState = oscState;
                            evt.msg.osc.chan = chan;
                            func_8008B610(&seqp->evtq, &evt, deltaTime);
                            vstate->flags |= 2;
                            vstate->vibOsc = oscState;
                        }
                    }
                }
                vstate->vibrato = oscValue;
                pitch = vstate->pitch * cs->pitchBend * vstate->vibrato;
                fxmix = func_8008CE88(vstate, seqp);
                lpGain = cs->filterGain;
                if (lpGain) {
                    lpFreq = func_8008DCB4(cents / 100 + (u8) cs->filterKey - 64) * 440.0f * cs->pitchBend;
                } else {
                    lpFreq = 127.0f;
                }
                pan = func_8008CFD4(vstate, seqp);
                vol = func_8008CD60(vstate, seqp);
                if (cs->override) {
                    deltaTime = cs->attackTime;
                } else {
                    deltaTime = sound->envelope->attackTime;
                }
                func_8008C040(voice, sound->wavetable, pitch, vol, pan, fxmix, lpGain, lpFreq, cs->distortion,
                              deltaTime);
                evt.type = 6;
                evt.msg.vol.voice = voice;
                if (cs->override) {
                    evt.msg.vol.vol = cs->decayVolume;
                    evt.msg.vol.delta = cs->decayTime;
                } else {
                    evt.msg.vol.vol = sound->envelope->decayVolume;
                    evt.msg.vol.delta = sound->envelope->decayTime;
                }
                func_8008B610(&seqp->evtq, &evt, deltaTime);
                if (midi->duration) {
                    evt.type = 0x15;
                    evt.msg.midi.status = chan | 0x80;
                    evt.msg.midi.byte1 = key;
                    evt.msg.midi.byte2 = 0;
                    deltaTime = seqp->uspt * midi->duration;
                    D_80104FD0[chan] = deltaTime;
                    func_8008B610(&seqp->evtq, &evt, deltaTime);
                }
                if ((cs->msg & 1) && seqp->msgQ) {
                    osSendMesg(seqp->msgQ, (OSMesg) ((D_80104FD0[chan] & ~0xFF) | (cs->msg >> 2)), 0);
                }
                break;
            }
        case 0x80:
            vstate = func_8008CB58(seqp, key, chan);
            if (!vstate) {
                return;
            }
            cs = &seqp->chanState[chan];
            if (vstate->phase == 2) {
                vstate->phase = 4;
            } else {
                vstate->phase = 3;
                if (cs->override) {
                    func_8008C800(seqp, &vstate->voice, cs->releaseTime);
                } else {
                    func_8008C800(seqp, &vstate->voice, vstate->sound->envelope->releaseTime);
                }
            }
            if ((cs->msg & 2) && seqp->msgQ) {
                osSendMesg(seqp->msgQ, (OSMesg) ((key << 16) | 8 | (cs->msg >> 2)), 0);
            }
            break;
        case 0xA0:
            vstate = func_8008CB58(seqp, key, chan);
            if (!vstate) {
                return;
            }
            vstate->velocity = byte2;
            func_8008BF70(&vstate->voice, func_8008CD60(vstate, seqp), func_8008CF90(vstate, seqp->curTime));
            break;
        case 0xD0:
            for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                if (vs->channel == chan) {
                    vs->velocity = byte1;
                    func_8008BF70(&vs->voice, func_8008CD60(vs, seqp), func_8008CF90(vs, seqp->curTime));
                }
            }
            break;
        case 0xB0:
            switch (byte1) {
                case 10:
                    seqp->chanState[chan].pan = byte2;
                    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                        if (vs->channel == chan) {
                            pan = func_8008CFD4(vs, seqp);
                            func_8008C170(&vs->voice, pan);
                        }
                    }
                    break;
                case 0xFD:
                    seqp->chanState[chan].unkF = byte2;
                    break;
                case 0xFF:
                    if (seqp->chanState[chan].unkF == 0) {
                        seqp->chanState[chan].unkF = 0x90;
                    }
                    if (seqp->chanState[chan].unkE != byte2) {
                        if (seqp->chanState[chan].unkE == seqp->chanState[chan].unkD) {
                            seqp->chanState[chan].unkE = byte2;
                        } else {
                            seqp->chanState[chan].unkE = byte2;
                            break;
                        }
                    } else {
                        break;
                    }
                    midi->byte1 = 0xFE;
                case 0xFE: {
                    u8 cur;
                    u8 target;
                    s32 step;

                    cur = seqp->chanState[chan].unkD;
                    target = seqp->chanState[chan].unkE;
                    vel = seqp->chanState[chan].unkF;
                    step = target - cur;
                    if (step > 0) {
                        if (vel & 0x80) {
                            vel = (vel & 0x7F) << 1;
                        }
                        if (step > vel) {
                            step = vel;
                        }
                    } else {
                        vel &= 0x7F;
                        if (step < -vel) {
                            step = -vel;
                        }
                    }
                    cur += step;
                    seqp->chanState[chan].unkD = cur;
                    if (cur != target) {
                        func_8008B610(&seqp->evtq, event, seqp->uspt * 100);
                    }
                    if (cur) {
                        seqp->chanMask |= 1 << chan;
                    } else {
                        seqp->chanMask &= ~(1 << chan);
                    }
                    func_8008723C(seqp, chan);
                } break;
                case 0xFC:
                    seqp->chanState[chan].unkD = byte2;
                    seqp->chanState[chan].unkE = byte2;
                    if (byte2 == 0) {
                        seqp->chanMask &= (1 << chan) ^ 0xFFFF;
                    } else {
                        seqp->chanMask |= 1 << chan;
                    }
                    func_8008723C(seqp, chan);
                    break;
                case 33:
                    seqp->chanState[chan].filterGain = byte2;
                    func_800872E8(seqp, chan);
                    break;
                case 34:
                    seqp->chanState[chan].filterKey = byte2;
                    func_800872E8(seqp, chan);
                    break;
                case 35:
                    seqp->chanState[chan].distortion = byte2;
                    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                        if (vs->channel == chan) {
                            func_8008E900(&vs->voice, byte2);
                        }
                    }
                    break;
                case 30:
                    if (seqp->msgQ) {
                        osSendMesg(seqp->msgQ, (OSMesg) (((byte2 & 7) | 0x10) | ((seqp->samplesLeft << 5) & ~0xFF)),
                                   0);
                    }
                    break;
                case 7:
                    seqp->chanState[chan].vol = byte2;
                    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                        if ((vs->channel == chan) && (vs->envPhase != 3)) {
                            vol = func_8008CD60(vs, seqp);
                            func_8008BF70(&vs->voice, vol, func_8008CF90(vs, seqp->curTime));
                        }
                    }
                    break;
                case 16:
                    seqp->chanState[chan].priority = byte2;
                    break;
                case 0x40:
                    seqp->chanState[chan].sustain = byte2;
                    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                        if ((vs->channel == chan) && (vs->phase != 3)) {
                            if (byte2 > 63) {
                                if (vs->phase == 0) {
                                    vs->phase = 2;
                                }
                            } else {
                                if (vs->phase == 2) {
                                    vs->phase = 0;
                                } else if (vs->phase == 4) {
                                    vs->phase = 3;
                                    // JFG: cs is not set on this path (the ROM reads it too)
                                    if (cs->override) {
                                        func_8008C800(seqp, &vs->voice,
                                                      (seqp->chanState[chan].releaseTime < 16000)
                                                          ? 16000
                                                          : seqp->chanState[chan].releaseTime);
                                    } else {
                                        func_8008C800(seqp, &vs->voice,
                                                      (vstate->sound->envelope->releaseTime < 16000)
                                                          ? 16000
                                                          : vstate->sound->envelope->releaseTime);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case 0x5B:
                    seqp->chanState[chan].fxmix = (seqp->chanState[chan].fxmix & 0x80) | byte2;
                    byte2 = seqp->chanState[chan].fxmix >> 7;
                case 0x41:
                    seqp->chanState[chan].fxmix = (seqp->chanState[chan].fxmix & 0x7F) | (byte2 << 7);
                    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                        if (vs->channel == chan) {
                            func_8008C2D0(&vs->voice, seqp->chanState[chan].fxmix);
                        }
                    }
                    break;
                case 0x5C:
                    if (byte2 < D_800A97C4->maxAuxBusses && byte2 >= 0) {
                        seqp->chanState[chan].fxBus = byte2;
                    }
                    break;
                case 26:
                    break;
                case 32:
                    seqp->chanState[chan].bankHi = byte2;
                    break;
                case 20:
                    seqp->chanState[chan].attackTime = D_800A95C0[byte2];
                    break;
                case 21:
                    seqp->chanState[chan].attackVolume = byte2;
                    break;
                case 22:
                    seqp->chanState[chan].decayTime = D_800A95C0[byte2];
                    break;
                case 23:
                    seqp->chanState[chan].decayVolume = byte2;
                    break;
                case 24:
                    seqp->chanState[chan].releaseTime = D_800A95C0[byte2];
                    break;
                case 2:
                    seqp->chanState[chan].detune = byte2 - 64;
                    break;
                case 3:
                    seqp->chanState[chan].bendRange = seqp->chanState[chan].bendRange / 100;
                    seqp->chanState[chan].bendRange = seqp->chanState[chan].bendRange * 100;
                    seqp->chanState[chan].bendRange = seqp->chanState[chan].bendRange + byte2;
                    break;
                case 4:
                    seqp->chanState[chan].bendRange = seqp->chanState[chan].bendRange % 100;
                    seqp->chanState[chan].bendRange = seqp->chanState[chan].bendRange + byte2 * 100;
                    break;
                case 11:
                    if (byte2) {
                        byte2 += 0x80;
                    }
                    seqp->chanState[chan].vibType = byte2;
                    seqp->chanState[chan].override = 1;
                    break;
                case 12:
                    seqp->chanState[chan].vibRate = byte2;
                    seqp->chanState[chan].override = 1;
                    break;
                case 13:
                    seqp->chanState[chan].vibDepth = byte2 << 1;
                    seqp->chanState[chan].override = 1;
                    break;
                case 14:
                    seqp->chanState[chan].vibDelay = byte2;
                    seqp->chanState[chan].override = 1;
                    break;
                case 15:
                    seqp->chanState[chan].tremType = byte2;
                    seqp->chanState[chan].override = 1;
                    break;
                case 17:
                    seqp->chanState[chan].tremRate = byte2;
                    seqp->chanState[chan].override = 1;
                    break;
                case 18:
                    seqp->chanState[chan].tremDepth = byte2;
                    seqp->chanState[chan].override = 1;
                    break;
                case 19:
                    seqp->chanState[chan].tremDelay = byte2;
                    seqp->chanState[chan].override = 1;
                    break;
                case 1:
                    byte2 = byte2 << 1;
                    for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                        if (vs->channel == chan && vs->vibOsc) {
                            NOscState *osc = vs->vibOsc;

                            switch (osc->type & ~0x80) {
                                case 2:
                                    osc->depthB = -func_80004768(byte2);
                                case 3:
                                case 4:
                                case 5:
                                    osc->depthA = func_80004768(byte2);
                                    break;
                                case 7:
                                case 9:
                                case 13:
                                    osc->depthA = func_80004768(byte2) / 2.0f;
                                    break;
                                case 10:
                                    osc->depthA = func_80004768(byte2) * 2.0f;
                                    break;
                                default:
                                    osc->depthA = func_80004768(byte2);
                                    break;
                            }
                        }
                    }
                    break;
                case 25:
                    seqp->chanState[chan].oscParam = byte2;
                    break;
                default:
                    break;
            }
            break;
        case 0xC0:
            type = (seqp->chanState[chan].bankHi << 7) + key;
            if (type < seqp->bank->instCount) {
                NInstrument *inst = seqp->bank->instArray[type];

                func_8008D424(seqp, inst, chan);
            } else {
            }
            break;
        case 0xE0: {
            s32 bendVal;
            f32 bendRatio;
            s32 cents;

            bendVal = ((byte2 << 7) + byte1) - 8192;
            cents = (seqp->chanState[chan].bendRange * bendVal) / 8192;
            bendRatio = func_8008B200(cents);
            seqp->chanState[chan].pitchBend = bendRatio;
            for (vs = seqp->vAllocHead; vs != 0; vs = vs->next) {
                if (vs->channel == chan) {
                    func_8008C220(&vs->voice, vs->pitch * bendRatio * vs->vibrato);
                    if (seqp->chanState[chan].filterGain) {
                        func_8008D8A0(&vs->voice, func_8008DCB4(vs->key - vs->sound->keyMap->keyBase) * 440.0f *
                                                      bendRatio * vs->vibrato);
                    }
                }
            }
        } break;
        default:
            break;
    }
}

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
