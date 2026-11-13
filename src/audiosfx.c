#include "common.h"

/*
 * The sound player: JFG's form of DKR's src/audiosfx.c (Rare's version of the libultra sound player, on n_audio).
 * Unlike the game code this object was built unoptimised, with debug information, as MIPS II code (the Makefile
 * builds it with -g -mips2; probe it with -O1 -g2 -mips2): every variable lives on the stack, and its debug
 * prints are real calls. Functions written from DKR keep DKR's names. Sounds, keymaps, envelopes and banks are
 * libaudio's ALSound, ALKeyMap, ALEnvelope and ALBank.
 */

/* JFG: a sound state (DKR ALSoundState; JFG's is larger) (0x48 bytes) */
typedef struct SfxState {
    /* 0x00 */ struct SfxState *next;
    /* 0x04 */ struct SfxState *prev;
    /* 0x08 */ ALSound *sound;
    /* 0x0C */ u8 voice[0x1C];
    /* 0x28 */ f32 slideMult;
    /* 0x2C */ f32 pitch;
    /* 0x30 */ struct SfxState **userHandle;
    /* 0x34 */ s32 retries;
    /* 0x38 */ s16 volume;
    /* 0x3A */ s16 envVolume; // JFG: the envelope level of the current phase
    /* 0x3C */ s32 phaseEnd;  // JFG: the player time the current phase ends
    /* 0x40 */ u8 priority;
    /* 0x41 */ u8 pan;
    /* 0x42 */ u8 fxmix;
    /* 0x43 */ u8 flags; // DKR SOUND_FLAG_*: 1 last in sequence, 2 looped, 4 playing, 0x10 retrigger, 0x20 slide
    /* 0x44 */ u8 state; // DKR SOUND_STATE_* (5 = initialised)
    /* 0x45 */ u8 pad45[3];
} SfxState;

/* JFG: a sound player event (DKR ALSndpEvent's common form) */
typedef struct SfxEvent {
    /* 0x00 */ s16 type;
    /* 0x04 */ SfxState *state;
    /* 0x08 */ s32 param;
    /* 0x0C */ s32 padC;
} SfxEvent;

/* JFG: the retrigger event (DKR ALSndpEvent's retrigger form) */
typedef struct SfxRetriggerEvent {
    /* 0x00 */ s16 type;
    /* 0x04 */ SfxState *state;
    /* 0x08 */ s32 soundIndex;
    /* 0x0C */ ALBank *bank;
} SfxRetriggerEvent;

/* JFG: n_audio's event queue */
typedef struct SfxEventQueue {
    /* 0x00 */ ALLink freeList;
    /* 0x08 */ ALLink allocList;
    /* 0x10 */ s16 eventCount;
    /* 0x12 */ u8 pad12[6];
} SfxEventQueue;

/* JFG: an event as the queue holds it (its type read unsigned) */
typedef struct SfxQueuedEvent {
    /* 0x00 */ u16 type;
    /* 0x04 */ SfxState *state;
    /* 0x08 */ s32 param;
    /* 0x0C */ s32 padC;
} SfxQueuedEvent;

/* JFG: an event queue item (libaudio ALEventListItem with n_audio's event) */
typedef struct SfxEventListItem {
    /* 0x00 */ ALLink node;
    /* 0x08 */ s32 delta;
    /* 0x0C */ SfxQueuedEvent evt;
} SfxEventListItem;

/* JFG: the sound player (DKR SoundPlayer) */
typedef struct SfxPlayer {
    /* 0x00 */ struct SfxPlayer *next; // the n_audio player node
    /* 0x04 */ void *clientData;
    /* 0x08 */ s32 (*handler)(void *);
    /* 0x0C */ s32 callTime;
    /* 0x10 */ s32 samplesLeft;
    /* 0x14 */ SfxEventQueue evtq;
    /* 0x2C */ SfxEvent nextEvent;
    /* 0x3C */ void *drvr;
    /* 0x40 */ SfxState *lastSoundState;
    /* 0x44 */ SfxState *soundStatesArray;
    /* 0x48 */ s32 maxSystemSoundChannels;
    /* 0x4C */ s32 frameTime;
    /* 0x50 */ s32 nextDelta;
    /* 0x54 */ s32 curTime;
    /* 0x58 */ s32 maxActiveSounds;
} SfxPlayer;

/* JFG: the sound player's configuration (DKR audioMgrConfig) */
typedef struct SfxConfig {
    /* 0x00 */ s32 maxSounds;
    /* 0x04 */ s32 maxEvents;
    /* 0x08 */ s32 maxChannels;
    /* 0x0C */ ALHeap *heap;
    /* 0x10 */ u16 numGroups;
} SfxConfig;

extern SfxState *D_800A95A0;  // DKR gSoundStateLists.allocHead: the allocated sound states
extern SfxState *D_800A95A4;  // DKR gSoundStateLists.allocTail
extern SfxState *D_800A95A8;  // DKR gSoundStateLists.freeHead
extern SfxPlayer *D_800A95AC; // DKR gSoundPlayerPtr
extern s32 D_800A95B0;        // DKR gSoundGlobalVolume
extern s16 D_800A95B4;        // DKR gNumActiveSounds
extern u16 *D_80104FC8;       // DKR gSoundGroupVolume
#define SFX_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SFX_MIN(a, b) ((a) < (b) ? (a) : (b))
/* A sound's volume: its group's volume times the envelope level, the sound's and the sample's volume (DKR) */
#define SFX_VOLUME(state, sound, keyMap)                                                                        \
    SFX_MAX(0, ((s16 *) D_80104FC8)[(keyMap)->keyMin & 0x3F] *                                                  \
                       ((state)->envVolume * (state)->volume * (sound)->sampleVolume / 16129) / 0x7FFF -        \
                   1)

void func_8008B4B0(SfxEventQueue *evtq, SfxEventListItem *items, s32 count); // n_audio: new event queue (alEvtqNew)
s32 func_8008B55C(SfxEventQueue *evtq, SfxEvent *evt);  // n_audio: next event (alEvtqNextEvent)
void func_8008B610(void *evtq, void *evt, s32 delta);   // n_audio: post an event (alEvtqPostEvent)
void func_8008BB80(SfxPlayer *player);                  // n_audio: add a sound player to the synthesizer
s32 func_8008BC60(void *voice, ALVoiceConfig *config);  // n_audio: allocate a voice (alSynAllocVoice)
void func_8008BF70(void *voice, s32 vol, s32 t);        // n_audio: ramp a voice's volume (alSynSetVol)
void func_8008C040(void *voice, void *wave, f32 pitch, s32 vol, s32 pan, s32 fxmix, s32 lpGain, f32 lpCutoff,
                   s32 lpRate, s32 t);                  // n_audio: start a voice with all its settings
void func_8008C170(void *voice, s32 pan);               // n_audio: set a voice's pan (alSynSetPan)
void func_8008C220(void *voice, f32 pitch);             // n_audio: set a voice's pitch (alSynSetPitch)
void func_8008C2D0(void *voice, s32 fxmix);             // n_audio: set a voice's effect mix (alSynSetFXMix)
void func_8008C380(void *voice);                        // n_audio: stop a voice (alSynStopVoice)
void func_8008C420(void *voice);                        // n_audio: free a voice (alSynFreeVoice)
void osSyncPrintf(const char *fmt, ...);
void sndp_handle_event(SfxQueuedEvent *event);
void sndp_end(SfxState *state);
void sndp_apply_pitch_slide(SfxState *state);
void sndp_remove_events(SfxEventQueue *evtq, SfxState *state, u16 eventTypeMask);
u16 sndp_get_state_counts(u16 *numFree, u16 *numAllocated);
void sndp_deallocate(SfxState *state);
s32 sndp_voice_handler(void *node);
SfxState *sndp_play(ALBank *bank, s16 sndIndx, u16 volume, u8 pan, f32 pitch, u8 fxmix, SfxState **handlePtr);

/**
 * Initialise a sound player and ready it for use with the sound event system.
 * DKR: sndp_init_player (Official Name: gsSndpNew); JFG's frame time is 16000.
 */
void sndp_init_player(SfxConfig *c) {
    u32 i;
    void *items;
    SfxEvent evt;
    SfxState *sounds;

    D_800A95AC->maxSystemSoundChannels = c->maxChannels;
    D_800A95AC->maxActiveSounds = c->maxChannels;
    D_800A95AC->lastSoundState = NULL;
    D_800A95AC->frameTime = 16000;
    items = alHeapDBAlloc(0, 0, c->heap, 1, c->maxSounds * sizeof(SfxState));
    D_800A95AC->soundStatesArray = items;
    items = alHeapDBAlloc(0, 0, c->heap, 1, c->maxEvents * sizeof(SfxEventListItem));
    func_8008B4B0(&D_800A95AC->evtq, items, c->maxEvents);
    D_800A95A8 = D_800A95AC->soundStatesArray;
    for (i = 1; i < (u32) c->maxSounds; i++) {
        sounds = D_800A95AC->soundStatesArray;
        alLink((ALLink *) &sounds[i].next, (ALLink *) &(&sounds[i] - 1)->next);
    }
    D_80104FC8 = alHeapDBAlloc(0, 0, c->heap, sizeof(*D_80104FC8), c->numGroups);
    for (i = 0; i < c->numGroups; i++) {
        D_80104FC8[i] = 0x7FFF;
    }
    D_800A95AC->next = NULL;
    D_800A95AC->handler = sndp_voice_handler;
    D_800A95AC->clientData = D_800A95AC;
    func_8008BB80(D_800A95AC);
    evt.type = 0x20;
    func_8008B610(&D_800A95AC->evtq, &evt, D_800A95AC->frameTime);
    D_800A95AC->nextDelta = func_8008B55C(&D_800A95AC->evtq, &D_800A95AC->nextEvent);
}

/**
 * Copy of _sndpVoiceHandler from libultra: processes all queued events that are due at the current time.
 * DKR: sndp_voice_handler
 */
s32 sndp_voice_handler(void *node) {
    SfxPlayer *sndp = (SfxPlayer *) node;
    SfxEvent evt;

    do {
        switch (sndp->nextEvent.type) {
            case 0x20:
                evt.type = 0x20;
                func_8008B610(&sndp->evtq, &evt, sndp->frameTime);
                break;
            default:
                sndp_handle_event((SfxQueuedEvent *) &sndp->nextEvent);
                break;
        }
        sndp->nextDelta = func_8008B55C(&sndp->evtq, &sndp->nextEvent);
    } while (sndp->nextDelta == 0);
    sndp->curTime += sndp->nextDelta;
    return sndp->nextDelta;
}

/**
 * Modified version of _handleEvent from libultra: processes the current event for the sound player (for a composite
 * sound, for each sound in its sequence).
 * DKR: sndp_handle_event; JFG's starts the voice with all its settings at once, keeps the envelope level and when
 * its current phase ends (for volume changes), and has no player argument.
 */
void sndp_handle_event(SfxQueuedEvent *event) {
    ALVoiceConfig config;
    ALSound *sound;
    ALKeyMap *keyMap;
    u8 pan;
    SfxEvent spAC;
    SfxQueuedEvent nextStateEvent;
    s32 delta;
    s32 fxMix;
    s32 volume;
    s32 newPan;
    s32 limitReached;
    s32 isEventForSingleSound;
    s32 lastInSequence;
    s32 isVoiceAllocated;
    SfxState *soundState;
    SfxState *nextState;

    lastInSequence = TRUE;
    isVoiceAllocated = FALSE;
    soundState = NULL;
    nextState = NULL;

    do {
        if (nextState != NULL) {
            nextStateEvent.state = soundState;
            nextStateEvent.type = event->type;
            nextStateEvent.param = event->param;
            event = &nextStateEvent;
        }
        soundState = event->state;
        sound = soundState->sound;
        if (sound == NULL) {
            u16 numFree, numAlloc;

            sndp_get_state_counts(&numFree, &numAlloc);
            osSyncPrintf("Bad soundState: voices =%d, states free =%d, states busy =%d, type %d data %x\n",
                         D_800A95B4, numFree, numAlloc, event->type, event->param);
            return;
        }
        keyMap = sound->keyMap;
        nextState = soundState->next;
        switch (event->type) {
            case 1:
                if (soundState->state != 5 && soundState->state != 4) {
                    return;
                }
                if (soundState->state == 1) {
                    osSyncPrintf("playing a playing sound\n");
                }
                config.fxBus = 0;
                config.priority = soundState->priority;
                config.unityPitch = 0;
                limitReached = D_800A95B4 >= D_800A95AC->maxActiveSounds;
                if (!limitReached || (soundState->flags & 0x10)) {
                    isVoiceAllocated = func_8008BC60(soundState->voice, &config);
                }
                if (!isVoiceAllocated) {
                    if ((soundState->flags & 0x12) || soundState->retries > 0) {
                        soundState->state = 4;
                        soundState->retries--;
                        func_8008B610(&D_800A95AC->evtq, event, 33333);
                    } else if (limitReached) {
                        SfxState *iterState = D_800A95A4;

                        do {
                            if (iterState->priority <= soundState->priority && !(iterState->flags & 0x12) &&
                                (iterState->flags & 4) && iterState->state != 3) {
                                SfxEvent interruptEvent;

                                limitReached = FALSE;
                                interruptEvent.type = 0x80;
                                interruptEvent.state = iterState;
                                iterState->state = 3;
                                func_8008B610(&D_800A95AC->evtq, &interruptEvent, 1000);
                                func_8008BF70(iterState->voice, 0, 1000);
                            }
                            iterState = iterState->prev;
                        } while (limitReached && iterState != NULL);
                        if (!limitReached) {
                            soundState->retries = 2;
                            func_8008B610(&D_800A95AC->evtq, event, 1001);
                        } else {
                            sndp_end(soundState);
                        }
                    } else {
                        sndp_end(soundState);
                    }
                    return;
                }
                soundState->flags |= 4;
                soundState->envVolume = sound->envelope->attackVolume;
                delta = sound->envelope->attackTime / soundState->pitch / soundState->slideMult;
                soundState->phaseEnd = D_800A95AC->curTime + delta;
                volume = SFX_VOLUME(soundState, sound, keyMap);
                volume = (u32) (volume * D_800A95B0) >> 8;
                newPan = soundState->pan + sound->samplePan - 0x40;
                pan = SFX_MIN(SFX_MAX(newPan, 0), 127);
                fxMix = (soundState->fxmix & 0x7F) + (keyMap->keyMax & 0xF) * 8;
                fxMix = SFX_MIN(127, SFX_MAX(0, fxMix));
                fxMix |= soundState->fxmix & 0x80;
                func_8008C040(soundState->voice, sound->wavetable, soundState->pitch * soundState->slideMult, volume,
                              pan, fxMix, 0, 0.0f, 0, delta);
                soundState->state = 1;
                D_800A95B4++;
                if (!(soundState->flags & 2)) {
                    if (!delta) {
                        soundState->envVolume = sound->envelope->decayVolume;
                        volume = SFX_VOLUME(soundState, sound, keyMap);
                        volume = (u32) (volume * D_800A95B0) >> 8;
                        delta = sound->envelope->decayTime / soundState->slideMult / soundState->pitch;
                        soundState->phaseEnd = D_800A95AC->curTime + delta;
                        func_8008BF70(soundState->voice, volume, delta);
                        spAC.type = 2;
                        spAC.state = soundState;
                        func_8008B610(&D_800A95AC->evtq, &spAC, delta);
                        if (soundState->flags & 0x20) {
                            sndp_apply_pitch_slide(soundState);
                        }
                    } else {
                        spAC.type = 0x40;
                        spAC.state = soundState;
                        delta = sound->envelope->attackTime / soundState->pitch / soundState->slideMult;
                        func_8008B610(&D_800A95AC->evtq, &spAC, delta);
                    }
                }
                break;
            case 2:
            case 0x400:
            case 0x1000:
                if (event->type != 0x1000 || (soundState->flags & 2)) {
                    switch (soundState->state) {
                        case 1:
                            sndp_remove_events(&D_800A95AC->evtq, soundState, 0x40);
                            delta = sound->envelope->releaseTime / soundState->slideMult / soundState->pitch;
                            func_8008BF70(soundState->voice, 0, delta);
                            if (delta) {
                                spAC.type = 0x80;
                                spAC.state = soundState;
                                func_8008B610(&D_800A95AC->evtq, &spAC, delta);
                                soundState->state = 2;
                            } else {
                                sndp_end(soundState);
                            }
                            break;
                        case 4:
                        case 5:
                            sndp_end(soundState);
                            break;
                        default:
                            break;
                    }
                    if (event->type == 2) {
                        event->type = 0x1000;
                    }
                }
                break;
            case 4:
                soundState->pan = event->param;
                if (soundState->state == 1) {
                    newPan = soundState->pan + sound->samplePan - 0x40;
                    pan = SFX_MIN(SFX_MAX(newPan, 0), 127);
                    func_8008C170(soundState->voice, pan);
                }
                break;
            case 0x10:
                soundState->pitch = *(f32 *) &event->param;
                if (soundState->state == 1) {
                    func_8008C220(soundState->voice, soundState->pitch * soundState->slideMult);
                    if (soundState->flags & 0x20) {
                        sndp_apply_pitch_slide(soundState);
                    }
                }
                break;
            case 0x100:
                soundState->fxmix = event->param;
                if (soundState->state == 1) {
                    fxMix = (soundState->fxmix & 0x7F) + (keyMap->keyMax & 0xF) * 8;
                    fxMix = SFX_MIN(127, SFX_MAX(0, fxMix));
                    fxMix |= soundState->fxmix & 0x80;
                    func_8008C2D0(soundState->voice, fxMix);
                }
                break;
            case 8:
                soundState->volume = event->param;
                if (soundState->state == 1) {
                    volume = SFX_VOLUME(soundState, sound, keyMap);
                    volume = (u32) (volume * D_800A95B0) >> 8;
                    func_8008BF70(soundState->voice, volume,
                                  SFX_MAX(1000, soundState->phaseEnd - D_800A95AC->curTime));
                }
                break;
            case 0x800:
                if (soundState->state == 1) {
                    delta = sound->envelope->releaseTime / soundState->slideMult / soundState->pitch;
                    volume = SFX_VOLUME(soundState, sound, keyMap);
                    volume = (u32) (volume * D_800A95B0) >> 8;
                    func_8008BF70(soundState->voice, volume, delta);
                }
                break;
            case 0x40:
                if (!(soundState->flags & 2)) {
                    soundState->envVolume = sound->envelope->decayVolume;
                    volume = SFX_VOLUME(soundState, sound, keyMap);
                    volume = (u32) (volume * D_800A95B0) >> 8;
                    delta = sound->envelope->decayTime / soundState->slideMult / soundState->pitch;
                    soundState->phaseEnd = D_800A95AC->curTime + delta;
                    func_8008BF70(soundState->voice, volume, delta);
                    spAC.type = 2;
                    spAC.state = soundState;
                    func_8008B610(&D_800A95AC->evtq, &spAC, delta);
                    if (soundState->flags & 0x20) {
                        sndp_apply_pitch_slide(soundState);
                    }
                }
                break;
            case 0x80:
                sndp_end(soundState);
                break;
            case 0x200:
                if (soundState->flags & 0x10) {
                    SfxState *newSound = sndp_play(((SfxRetriggerEvent *) event)->bank,
                                                   ((SfxRetriggerEvent *) event)->soundIndex, soundState->volume,
                                                   soundState->pan, soundState->pitch, soundState->fxmix,
                                                   soundState->userHandle);
                }
                break;
            default:
                osSyncPrintf("Nonsense sndp event\n");
                break;
        }
        isEventForSingleSound = event->type & (1 | 0x10 | 0x40 | 0x80 | 0x200);
        if ((soundState = nextState) != NULL && !isEventForSingleSound) {
            lastInSequence = soundState->flags & 1;
        }
    } while (!lastInSequence && soundState != NULL && !isEventForSingleSound);
}

/**
 * Immediately stop the sound and release all associated resources.
 * DKR: sndp_end
 */
void sndp_end(SfxState *state) {
    if (state->flags & 4) {
        func_8008C380(&state->voice);
        func_8008C420(&state->voice);
    }
    sndp_deallocate(state);
    sndp_remove_events(&D_800A95AC->evtq, state, 0xFFFF);
}

/**
 * Changes the pitch slide and enqueues an event to continue the pitch slide on the next frame.
 * DKR: sndp_apply_pitch_slide
 */
void sndp_apply_pitch_slide(SfxState *state) {
    SfxEvent evt;
    f32 pitch;

    pitch = alCents2Ratio(state->sound->keyMap->detune) * state->pitch;
    evt.type = 0x10;
    evt.state = state;
    evt.param = *((s32 *) &pitch);
    func_8008B610(&D_800A95AC->evtq, &evt, 33333);
}

/**
 * Modified version of _sndpRemoveEvents from libultra: removes all events of the given sound state, filtered by
 * eventTypeMask, from the event queue. JFG also counts the queue's events down.
 * DKR: sndp_remove_events
 */
void sndp_remove_events(SfxEventQueue *evtq, SfxState *state, u16 eventTypeMask) {
    ALLink *thisNode;
    ALLink *nextNode;
    SfxEventListItem *thisItem;
    SfxEventListItem *nextItem;
    SfxQueuedEvent *thisEvent;
    OSIntMask mask;

    mask = osSetIntMask(OS_IM_NONE);
    thisNode = evtq->allocList.next;
    while (thisNode != NULL) {
        nextNode = thisNode->next;
        thisItem = (SfxEventListItem *) thisNode;
        nextItem = (SfxEventListItem *) nextNode;
        thisEvent = &thisItem->evt;
        if (thisEvent->state == state && (thisEvent->type & eventTypeMask)) {
            if (nextItem) {
                nextItem->delta += thisItem->delta;
            }
            alUnlink(thisNode);
            evtq->eventCount--;
            alLink(thisNode, &evtq->freeList);
        }
        thisNode = nextNode;
    }
    osSetIntMask(mask);
}

/**
 * Writes the number of free and allocated sound states, and returns the allocated count walked backwards.
 * DKR: sndp_get_state_counts (Official Name: getSoundStateCounts)
 */
u16 sndp_get_state_counts(u16 *numFree, u16 *numAllocated) {
    OSIntMask mask;
    u16 allocatedCounter;
    u16 freeCounter;
    u16 allocatedRevCounter;
    SfxState *allocatedPtr;
    SfxState *freePtr;
    SfxState *allocatedRevPtr;

    mask = osSetIntMask(OS_IM_NONE);
    allocatedPtr = D_800A95A0;
    freePtr = D_800A95A8;
    allocatedRevPtr = D_800A95A4;
    for (allocatedCounter = 0; allocatedPtr != NULL; allocatedCounter++, allocatedPtr = allocatedPtr->next) {
    }
    for (freeCounter = 0; freePtr != NULL; freeCounter++, freePtr = freePtr->next) {
    }
    for (allocatedRevCounter = 0; allocatedRevPtr != NULL; allocatedRevCounter++, allocatedRevPtr = allocatedRevPtr->prev) {
    }
    *numFree = freeCounter;
    *numAllocated = allocatedCounter;
    osSetIntMask(mask);
    return allocatedRevCounter;
}

/**
 * Allocates a sound and sets its initial parameters, including values from the ALKeyMap structure.
 * Returns NULL if there are no free sound states available.
 * DKR: sndp_allocate
 */
SfxState *sndp_allocate(UNUSED ALBank *bank, ALSound *sound) {
    SfxState *state;
    ALKeyMap *keyMap;
    s32 isLooping;
    OSIntMask mask;

    keyMap = sound->keyMap;
    mask = osSetIntMask(OS_IM_NONE);
    state = D_800A95A8;
    if (state != NULL) {
        D_800A95A8 = state->next;
        alUnlink((ALLink *) state);
        if (D_800A95A0 != NULL) {
            state->next = D_800A95A0;
            state->prev = NULL;
            D_800A95A0->prev = state;
            D_800A95A0 = state;
        } else {
            state->next = state->prev = NULL;
            D_800A95A0 = state;
            D_800A95A4 = state;
        }
        osSetIntMask(mask);
        isLooping = sound->envelope->decayTime == -1;
        state->sound = sound;
        state->priority = isLooping + 0x40;
        state->state = 5;
        state->pitch = 1.0f;
        state->retries = 2;
        state->flags = keyMap->keyMax & 0xF0;
        state->userHandle = NULL;
        if (state->flags & 0x20) {
            state->slideMult = alCents2Ratio(keyMap->keyBase * 100 - 6000);
        } else {
            state->slideMult = alCents2Ratio(keyMap->keyBase * 100 + keyMap->detune - 6000);
        }
        if (isLooping) {
            state->flags |= 2;
        }
        state->fxmix = 0;
        state->pan = 0x40;
        state->volume = 0x7FFF;
    } else {
        osSetIntMask(mask);
    }
    return state;
}

/**
 * Deallocates a sound state and returns it to the free list.
 * DKR: sndp_deallocate
 */
void sndp_deallocate(SfxState *state) {
    if (D_800A95A0 == state) {
        D_800A95A0 = state->next;
    }
    if (D_800A95A4 == state) {
        D_800A95A4 = state->prev;
    }
    alUnlink((ALLink *) state);
    if (D_800A95A8 != NULL) {
        state->next = D_800A95A8;
        state->prev = NULL;
        D_800A95A8->prev = state;
        D_800A95A8 = state;
    } else {
        state->next = state->prev = NULL;
        D_800A95A8 = state;
    }
    if (state->flags & 4) {
        D_800A95B4--;
    }
    state->state = 0;
    if (state->userHandle != NULL) {
        if (*state->userHandle == state) {
            *state->userHandle = NULL;
        }
        state->userHandle = NULL;
    }
}

/**
 * Sets the priority for voice allocation and for preempting another sound.
 * DKR: sndp_set_priority (Official Name: gsSndpSetPriority)
 */
void sndp_set_priority(SfxState *state, u8 priority) {
    if (state != NULL) {
        state->priority = (s16) priority;
    }
}

/**
 * Retrieves the current sound state.
 * DKR: sndp_get_state (Official Name: gsSndpGetState)
 */
u8 sndp_get_state(SfxState *state) {
    if (state != NULL) {
        return state->state;
    } else {
        return 0;
    }
}

/**
 * Plays a sound (for a composite sound, every sound in its sequence) and schedules its retrigger.
 * DKR: sndp_play_with_priority; JFG takes the volume, pan, pitch and effects mix instead of a priority, and
 * prints when no sound state is free.
 */
SfxState *sndp_play(ALBank *bank, s16 sndIndx, u16 volume, u8 pan, f32 pitch, u8 fxmix, SfxState **handlePtr) {
    SfxState *soundState;
    SfxState *lastSoundState;
    ALKeyMap *keyMap;
    ALSound *sound;
    s16 retriggeredSoundID;
    s32 retriggerInterval;
    s32 duration;
    s32 totalDuration;
    s32 newPan;

    lastSoundState = NULL;
    retriggeredSoundID = 0;
    totalDuration = 0;
    if (sndIndx != 0) {
        do {
            sound = bank->instArray[0]->soundArray[sndIndx - 1];
            soundState = sndp_allocate(bank, sound);
            if (soundState != NULL) {
                SfxEvent playEvent;

                D_800A95AC->lastSoundState = soundState;
                playEvent.type = 1;
                playEvent.state = soundState;
                newPan = pan + soundState->pan - 0x40;
                if (newPan >= 0x80) {
                    newPan = 0x7F;
                } else if (newPan < 0) {
                    newPan = 0;
                }
                soundState->pan = newPan;
                soundState->volume = ((u32) volume * soundState->volume) >> 15;
                soundState->pitch *= pitch;
                soundState->fxmix = fxmix;
                duration = sound->keyMap->velocityMax * 33333;
                if (soundState->flags & 0x10) {
                    soundState->flags &= ~0x10;
                    func_8008B610(&D_800A95AC->evtq, &playEvent, totalDuration + 1);
                    retriggerInterval = duration + 1;
                    retriggeredSoundID = sndIndx;
                } else {
                    func_8008B610(&D_800A95AC->evtq, &playEvent, duration + 1);
                }
                lastSoundState = soundState;
            } else {
                osSyncPrintf("Sound state allocate failed - sndId %d\n", sndIndx);
            }
            totalDuration += duration;
            keyMap = sound->keyMap;
            sndIndx = keyMap->velocityMin + (keyMap->keyMin & 0xC0) * 4;
        } while (sndIndx != 0 && soundState != NULL);
        if (lastSoundState != NULL) {
            lastSoundState->flags |= 1;
            lastSoundState->userHandle = handlePtr;
            if (retriggeredSoundID != 0) {
                SfxRetriggerEvent evtRetrigger;

                lastSoundState->flags |= 0x10;
                evtRetrigger.type = 0x200;
                evtRetrigger.state = lastSoundState;
                evtRetrigger.soundIndex = retriggeredSoundID;
                evtRetrigger.bank = bank;
                func_8008B610(&D_800A95AC->evtq, &evtRetrigger, retriggerInterval);
            }
        }
    }
    if (handlePtr != NULL) {
        *handlePtr = lastSoundState;
    }
    return lastSoundState;
}

/**
 * Schedules a stop event for the given sound state and disables the retrigger flag.
 * DKR: sndp_stop (Official Name: gsSndpStop); JFG prints a warning for a NULL sound.
 */
void sndp_stop(SfxState *state) {
    SfxEvent evt;

    evt.type = 0x400;
    evt.state = state;
    if (state != NULL) {
        evt.state->flags &= ~0x10;
        func_8008B610(&D_800A95AC->evtq, &evt, 0);
    } else {
        osSyncPrintf("WARNING: Attempt to stop NULL sound aborted\n");
    }
}

/**
 * Schedules a stop event for all sound states that have the specified flags set.
 * DKR: sndp_stop_with_flags
 */
void sndp_stop_with_flags(u8 flags) {
    OSIntMask mask;
    SfxEvent evt;
    SfxState *state;

    mask = osSetIntMask(OS_IM_NONE);
    state = D_800A95A0;
    while (state != NULL) {
        evt.type = 0x400;
        evt.state = state;
        if ((state->flags & flags) == flags) {
            evt.state->flags &= ~0x10;
            func_8008B610(&D_800A95AC->evtq, &evt, 0);
        }
        state = state->next;
    }
    osSetIntMask(mask);
}

/**
 * Stops all sounds from playing.
 * DKR: sndp_stop_all (Official Name: gsSndpStopAll)
 */
void sndp_stop_all(void) {
    sndp_stop_with_flags(1);
}

/**
 * Stops all retriggered sounds from playing.
 * DKR: sndp_stop_all_retrigger (Official Name: gsSndpStopAllRetrigger)
 */
void sndp_stop_all_retrigger(void) {
    sndp_stop_with_flags(1 | 0x10);
}

/**
 * Stops all looped sounds from playing.
 * DKR: sndp_stop_all_looped (Official Name: gsSndpStopAllLooped)
 */
void sndp_stop_all_looped(void) {
    sndp_stop_with_flags(1 | 2);
}

/**
 * Send a message to the sound player to update an existing property of the sound entry.
 * DKR: sndp_set_param (Official Name: gsSndpSetParam); JFG prints a warning for a NULL sound.
 */
void sndp_set_param(SfxState *state, s16 type, u32 param) {
    SfxEvent evt;

    evt.type = type;
    evt.state = state;
    evt.param = param;
    if (state != NULL) {
        func_8008B610(&D_800A95AC->evtq, &evt, 0);
    } else {
        osSyncPrintf("WARNING: Attempt to modify NULL sound aborted\n");
    }
}

/**
 * Returns the volume level of the group.
 * DKR: sndp_get_group_volume (Official Name: gsSndpGetMasterVolume)
 */
u16 sndp_get_group_volume(u8 group) {
    return D_80104FC8[group];
}

/**
 * Sets the volume for the specified group and updates the volume of all sounds in that group.
 * DKR: sndp_set_group_volume (Official Name: gsSndpSetMasterVolume); JFG counts the states it walks.
 */
void sndp_set_group_volume(u8 group, u16 volume) {
    OSIntMask mask;
    SfxState *state;
    s32 count;
    SfxEvent evt;

    mask = osSetIntMask(OS_IM_NONE);
    state = D_800A95A0;
    D_80104FC8[group] = volume;
    for (count = 0; state != NULL;) {
        if ((state->sound->keyMap->keyMin & 0x3F) == group) {
            evt.type = 0x800;
            evt.state = state;
            func_8008B610(&D_800A95AC->evtq, &evt, 0);
        }
        count++, state = state->next;
    }
    osSetIntMask(mask);
}

/**
 * Sets the global volume level for all sounds.
 * DKR: sndp_set_global_volume
 */
void sndp_set_global_volume(u32 volume) {
    if (volume > 256) {
        volume = 256;
    }
    D_800A95B0 = volume;
}

/**
 * Returns the global volume level applied to all sounds.
 * DKR: sndp_get_global_volume (Official Name: gsSndpGetGlobalVolume)
 */
s32 sndp_get_global_volume(void) {
    return D_800A95B0;
}

/**
 * Sets the number of active sound channels to the passed number or the maximum, whichever is lower.
 * DKR: sndp_set_active_sound_limit (Official Name: gsSndpLimitVoices)
 */
void sndp_set_active_sound_limit(s32 numSounds) {
    if (D_800A95AC->maxSystemSoundChannels >= numSounds) {
        D_800A95AC->maxActiveSounds = numSounds;
    } else {
        D_800A95AC->maxActiveSounds = D_800A95AC->maxSystemSoundChannels;
    }
}
