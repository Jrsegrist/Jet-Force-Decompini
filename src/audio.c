/*
 * Music and sound effect control (the "am" audio manager front end).
 *
 * Mostly from the Diddy Kong Racing decompilation (CC0 1.0), src/audio.c,
 * which compiles to the same code here. Functions marked "JFG" are not in
 * DKR or differ from it. The rest are still assembly.
 */
#include "audio.h"

/************ .data ************/

extern ALCSPlayer *gMusicPlayer;  // Official Name: tuneSeqPlayer
extern ALCSPlayer *gJinglePlayer; // Official Name: ambientSeqPlayer
extern u8 gMusicBaseVolume;
extern u8 gCanPlayMusic;
extern s32 audioPrevCount;
extern u8 sfxRelativeVolume;
extern u8 gCanPlayJingle;
extern s32 gBlockMusicChange;
extern u8 gMusicNextSeqID;
extern u8 gJingleNextSeqID;
extern u8 gBlockVoiceLimitChange;

/************ .bss ************/

extern ALSeqFile *gSequenceTable;
extern u8 gCurrentSequenceID;
extern u8 gCurrentJingleID;
extern ALBankFile *gSoundBank; // Official Name: sfxBankPtr
extern SoundData *gSoundTable; // Official Name: sfxIndex
extern s32 gSoundCount;        // Official Name: maxSound
extern u32 gSoundTableSize;    // Official Name: sfxIndexSize
extern s16 sMusicTempo;
extern u8 gJinglePlaying;
extern u32 gDynamicMusicChannelMask;
extern s32 gMusicTempo;
extern u32 *gSequenceLengths; // JFG: byte length of each sequence

/******************************/

// The debug strings of functions that are still assembly, in source order.
INCLUDE_RODATA("asm/nonmatchings/audio", D_800A9FB0);

#define AL_SNDP_GROUP_VOLUME_MAX 0x7FFF
#define VOLUME_LOWER 1
#define VOLUME_LOWER_AMBIENT 2

void sndp_set_group_volume(s32 group, s32 volume);
extern u8 gAudioVolumeSetting;

/**
 * Changes the volume of each sound channel depending on what value is passed through.
 * JFG: two groups; lowering leaves the music alone.
 * Official Name: amSetMuteMode
 */
void sound_volume_change(s32 behaviour) {
    switch (behaviour) {
        case VOLUME_LOWER:
            sndp_set_group_volume(0, 0);
            sndp_set_group_volume(1, AL_SNDP_GROUP_VOLUME_MAX);
            alCSPSetVol(gJinglePlayer, 0);
            break;
        case VOLUME_LOWER_AMBIENT:
            sndp_set_group_volume(0, 0);
            break;
        default:
            sndp_set_group_volume(0, AL_SNDP_GROUP_VOLUME_MAX);
            sndp_set_group_volume(1, AL_SNDP_GROUP_VOLUME_MAX);
            alCSPSetVol(gJinglePlayer, (s16) (sndp_get_global_volume() * sfxRelativeVolume));
            break;
    }
    gAudioVolumeSetting = behaviour;
}

/**
 * Queue a new music sequence to play if not blocked.
 * Stops any playing existing music beforehand.
 * JFG: checks the sequence's length instead of the music volume slider.
 * Official Name: amTunePlay
 */
void music_play(u8 seqID) {
    if (gBlockMusicChange == FALSE && gSequenceLengths[seqID] <= 0x8000) {
        if (gCanPlayMusic) {
            music_stop();
            music_sequence_start(seqID, gMusicPlayer);
        }
        gMusicTempo = alCSPGetTempo(gMusicPlayer);
        audioPrevCount = osGetCount();
        gDynamicMusicChannelMask = MUSIC_CHAN_MASK_NONE;
    }
}

/**
 * Update the background music voice limit if not prevented from doing so.
 * Official Name: amTuneVoiceLimit
 */
void music_voicelimit_set(u8 voiceLimit) {
    if (gBlockVoiceLimitChange == FALSE) {
        set_voice_limit(gMusicPlayer, voiceLimit);
    }
}

/* DKR src/audio.h */
typedef struct MusicData {
    u8 volume;
    u8 tempo;
    u8 reverb;
} MusicData;

extern MusicData *D_800E950C; // DKR: gSeqSoundTable
extern s32 D_800A0680;        // JFG: music fade: ticks left
extern s32 D_800A0684;        // JFG: jingle fade: ticks left
extern s32 D_800E9524;        // JFG: music fade: base volume
extern s32 D_800E9528;        // JFG: music fade: volume per tick (16.16)
extern s32 D_800E952C;        // JFG: jingle fade: base volume
extern s32 D_800E9530;        // JFG: jingle fade: volume per tick (16.16)
void music_volume_set(u8 volume);
void music_jingle_volume_set(u8 volume);

/* JFG: fade the music volume to volume over time seconds (50 or 60 ticks a second). */
void func_800005F8(f32 time, u8 volume) {
    if (volume >= 0x80) {
        volume = 0x7F;
    }
    D_800E9524 = volume;
    if (osTvType == OS_TV_PAL) {
        D_800A0680 = time * 50.0f;
    } else {
        D_800A0680 = time * 60.0f;
    }
    if (D_800A0680 > 0) {
        D_800E9528 = ((gMusicBaseVolume - volume) << 16) / D_800A0680;
    } else {
        music_volume_set(volume);
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: 2 words: the product lands in $v1 in the target ($a1 here) */
/* JFG: fade the music to volume, scaled by the current sequence's volume. */
void func_80000724(f32 time, u8 volume) {
    if (volume >= 0x80) {
        volume = 0x7F;
    }
    func_800005F8(time, (D_800E950C[gCurrentSequenceID].volume * volume) / 127);
}
#else
INCLUDE_ASM("asm/nonmatchings/audio", func_80000724);
#endif

void func_80000798(void) {
    D_800A0680 = 0;
}

/* JFG: the second fade (D_800A0684, the jingle fade in sound_update_queue); sets the music volume when instant. */
void func_800007A4(f32 time, u8 volume) {
    if (volume >= 0x80) {
        volume = 0x7F;
    }
    D_800E952C = volume;
    if (osTvType == OS_TV_PAL) {
        D_800A0684 = time * 50.0f;
    } else {
        D_800A0684 = time * 60.0f;
    }
    if (D_800A0684 > 0) {
        D_800E9530 = ((gMusicBaseVolume - volume) << 16) / D_800A0684;
    } else {
        music_volume_set(volume);
    }
}

void func_800008D0(void) {
    D_800A0684 = 0;
}

/* JFG's sequence player (n_audio) has its target sequence at 0x18 */
typedef struct N_ALCSPlayerHead {
    /* 0x00 */ u8 pad0[0x18];
    /* 0x18 */ ALCSeq *target;
} N_ALCSPlayerHead;

/* DKR src/audio.h */
typedef struct DelayedSound {
    /* 0x00 */ u16 soundId;
    /* 0x02 */ s16 timer;
    /* 0x04 */ SoundHandle *handlePtr;
} DelayedSound;

extern OSMesgQueue D_800E9780; // JFG: a queue polled each tick
extern s32 D_800E9774;
extern s32 D_800A06A8;
extern s32 gDelayedSoundsCount;
extern DelayedSound gDelayedSounds[8];
extern u8 gMusicSequenceData[];
extern u8 gJingleSequenceData[];
extern ALCSeq *D_800E9670; // JFG: the music sequence
extern ALCSeq gJingleSequence;
extern u8 D_800A06B0;
extern u8 D_800A06B4;
void sound_play(u16 soundID, SoundHandle *handlePtr);
void music_sequence_init(ALCSPlayer *seqPlayer, void *sequence, u8 *nextSeqID, ALCSeq *seq);

/**
 * Run every frame, this handles the transitions in and out of music sequences.
 * If there's something in the queue, then begin to play that.
 * Additionally, it also handles the delayed audio queue, counting down and playing any sounds.
 * JFG: linear fades of the music and jingle volumes; two flags are cleared.
 * Official Name: amAudioTick
 */
void sound_update_queue(u8 updateRate) {
    s32 i;
    s32 j;
    s32 volume;
    OSMesg mesg;

    if (osRecvMesg(&D_800E9780, &mesg, OS_MESG_NOBLOCK) == 0) {
        D_800E9774 = 1;
    }
    if (D_800A0680 > 0 || D_800A06A8 != -1) {
        D_800A0680 -= updateRate;
        if (D_800A0680 < 0) {
            D_800A0680 = 0;
        }
        volume = ((D_800E9528 * D_800A0680) >> 16) + D_800E9524;
        music_volume_set(volume);
    }
    if (D_800A0684 > 0) {
        D_800A0684 -= updateRate;
        if (D_800A0684 < 0) {
            D_800A0684 = 0;
        }
        volume = ((D_800E9530 * D_800A0684) >> 16) + D_800E952C;
        music_jingle_volume_set(volume);
    }

    if (gDelayedSoundsCount > 0) {
        for (i = 0; i < gDelayedSoundsCount;) {
            gDelayedSounds[i].timer -= updateRate;
            if (gDelayedSounds[i].timer <= 0) {
                j = i;
                sound_play(gDelayedSounds[i].soundId, gDelayedSounds[i].handlePtr);

                gDelayedSoundsCount -= 1;
                while (j < gDelayedSoundsCount) {
                    gDelayedSounds[i].soundId = gDelayedSounds[i + 1].soundId;
                    gDelayedSounds[i].timer = gDelayedSounds[i + 1].timer;
                    gDelayedSounds[i].handlePtr = gDelayedSounds[i + 1].handlePtr;
                    j++;
                }
                j++;
            } else {
                i++;
            }
        }
    }

    music_sequence_init(gMusicPlayer, gMusicSequenceData, &gMusicNextSeqID, D_800E9670);
    music_sequence_init(gJinglePlayer, gJingleSequenceData, &gJingleNextSeqID, &gJingleSequence);
    if (sMusicTempo == -1 && ((N_ALCSPlayerHead *) gMusicPlayer)->target) {
        sMusicTempo = 60000000 / alCSPGetTempo(gMusicPlayer);
    }
    D_800A06B0 = 0;
    D_800A06B4 = 0;
}

void func_80044C20(void);

/* JFG: skip one poll if flagged, otherwise call func_80044C20 until a message arrives. */
void func_80000B80(void) {
    OSMesg msg;

    if (D_800E9774) {
        D_800E9774 = 0;
    } else {
        while (osRecvMesg(&D_800E9780, &msg, OS_MESG_NOBLOCK) != 0) {
            func_80044C20();
        }
    }
}

extern s32 D_800E9774;

void func_80000C00(void) {
    D_800E9774 = 0;
}

/**
 * Sets the channels in the sequence on or off based on the channel mask given.
 * Official Name: amTuneSetChlMask
 */
void music_dynamic_set(u16 channelMask) {
    u32 i;
    if (gMusicNextSeqID) {
        gDynamicMusicChannelMask = channelMask;
    } else {
        gMusicPlayer->chanMask = channelMask;
        for (i = 0; i != AUDIO_CHANNELS; i++) {
            if (channelMask & (1 << i)) {
                music_channel_on(i);
            } else {
                music_channel_off(i);
            }
        }
    }
}

/**
 * Mute the sequence channel. JFG: does nothing (DKR calls alSeqChOff).
 * Official Name: amTuneMuteChl
 */
void music_channel_off(u8 channel) {
}

/**
 * Unmute the sequence channel. JFG: does nothing (DKR calls alSeqChOn).
 * Official Name: amTuneUnmuteChl
 */
void music_channel_on(u8 channel) {
}

/**
 * Set the volume of the given channel for the music player.
 * Official Name: amTuneSetChlVolume
 */
void music_channel_volume_set(u8 channel, u8 volume) {
    if (channel < AUDIO_CHANNELS) {
        alCSPSetChlVol(gMusicPlayer, channel, volume);
    }
}

/**
 * Resets all audio channels for the music player to the default state.
 * JFG: no fade reset.
 * Official Name: amTuneResetChls
 */
void music_channel_reset_all(void) {
    u32 channel;
    if (gBlockMusicChange == FALSE) {
        for (channel = 0; channel < AUDIO_CHANNELS; channel++) {
            music_channel_on(channel);
            music_channel_volume_set(channel, 127);
        }
    }
}

/**
 * Multiplies the current tempo of the background music.
 * Official Name: amTuneScaleTempo
 */
void music_tempo_set_relative(f32 tempo) {
    music_tempo_set((s32) ((f32) (u32) (music_tempo() & 0xFF) * tempo));
}

/**
 * Set the tempo of the current playing background music.
 * Official name: amTuneSetTempoBPM
 */
void music_tempo_set(s32 tempo) {
    if (tempo != 0) {
        f32 inv_tempo = (1.0f / tempo);
        alCSPSetTempo(gMusicPlayer, (s32) (inv_tempo * 60000000.0f));
        sMusicTempo = tempo;
    }
}

/**
 * Return the tempo of the current playing background music.
 * Official name: amTuneGetTempoBPM
 */
s16 music_tempo(void) {
    return sMusicTempo;
}

/**
 * Play a jingle, but only if there isn't one playing already.
 * Official Name: amAmbientPlay
 */
void music_jingle_play_safe(u8 jingleID) {
    if (music_jingle_playing() == SEQUENCE_NONE) {
        music_sequence_start(gCurrentJingleID = jingleID, gJinglePlayer);
        gJinglePlaying = TRUE;
    }
}

/**
 * Sets the tempo for the jingle player.
 * Official Name: amAmbientSetTempoBPM
 */
void sound_jingle_tempo_set(s32 tempo) {
    f32 inv_tempo = (1.0f / tempo);
    alCSPSetTempo(gJinglePlayer, (s32) (inv_tempo * 60000000.0f));
}

/**
 * Stops the background music.
 * Official Name: amTuneStop
 */
void music_stop(void) {
    if (gBlockMusicChange == FALSE) {
        music_sequence_stop(gMusicPlayer);
    }
}

/**
 * Stops the currently playing jingle.
 * Official Name: amAmbientStop
 */
void music_jingle_stop(void) {
    if (music_jingle_playing() == SEQUENCE_NONE) {
        gCurrentJingleID = SEQUENCE_NONE;
        music_sequence_stop(gJinglePlayer);
    }
}

/**
 * Return the currently playing music.
 * Official Name: amTuneGetSeqNo
 */
u8 music_current_sequence(void) {
    if (gCurrentSequenceID != SEQUENCE_NONE && gMusicPlayer->state == AL_PLAYING) {
        return gCurrentSequenceID;
    } else {
        return SEQUENCE_NONE;
    }
}

/**
 * Return the currently playing jingle.
 * Official Name: amAmbientGetSeqNo
 */
u8 music_jingle_current(void) {
    return gCurrentJingleID;
}

extern u32 D_800A0690; // JFG: the user's music volume setting (0 to 256)
extern u8 D_800A06B0;

/**
 * Set the volume of the music.
 * Update music volume with this new setting.
 * JFG: clamped to 127, scaled by the volume setting; sets a flag.
 * Official Name: amTuneSetVolume
 */
void music_volume_set(u8 volume) {
    u32 newVolume;
    if (volume > 127) {
        volume = 127;
    }
    gMusicBaseVolume = volume;
    newVolume = gMusicBaseVolume * D_800A0690;
    alCSPSetVol(gMusicPlayer, (s16) newVolume);
    D_800A06B0 = TRUE;
}

/**
 * Set the user configured music volume (0 to 256).
 * Update music volume with this new setting.
 * Official Name: amTuneSetGlobalVolume
 */
void music_volume_config_set(u32 slider_val) {
    u32 volume;
    slider_val = (slider_val <= 256) ? slider_val : 256;
    D_800A0690 = slider_val;
    volume = gMusicBaseVolume * D_800A0690;
    alCSPSetVol(gMusicPlayer, (s16) volume);
}

/**
 * Return the baseline music volume, unaffected by user config.
 * Official Name: amTuneGetVolume
 */
u8 music_volume(void) {
    return gMusicBaseVolume;
}

/**
 * Set the volume for the jingle player.
 * The jingle player scales with sfx volume rather than music volume.
 * Official Name: amAmbientSetVolume
 */
void music_jingle_volume_set(u8 arg0) {
    sfxRelativeVolume = arg0;
    alCSPSetVol(gJinglePlayer, (s16) (sndp_get_global_volume() * sfxRelativeVolume));
}

/**
 * Set the panning level for every channel in the jingle player.
 * Official Name: amAmbientSetPan
 */
void music_jingle_pan_set(ALPan pan) {
    u32 iChan;
    for (iChan = 0; iChan < AUDIO_CHANNELS; iChan++) {
        alCSPSetChlPan(gJinglePlayer, iChan, pan);
    }
}

/**
 * Plays a sequence just once, allowing it to coexist with the music if necessary.
 * JFG: only sequences up to 0x1000 bytes long.
 * Official Name: amDittyPlay
 */
void music_jingle_play(u8 seqID) {
    if (gSequenceLengths[seqID] <= 0x1000) {
        gCanPlayJingle = TRUE;
        music_sequence_start(gCurrentJingleID = seqID, gJinglePlayer);
    }
}

/**
 * Returns the jingle that is playing, or SEQUENCE_NONE.
 * Official Name: amDittyPlaying
 */
u32 music_jingle_playing(void) {
    if (gCurrentJingleID && gCanPlayJingle && (gJinglePlayer->state == AL_PLAYING)) {
        return gCurrentJingleID;
    }
    gCanPlayJingle = FALSE;
    return SEQUENCE_NONE;
}

void func_800861D4(SoundHandle soundHandle);

void func_80001228(SoundHandle soundHandle) {
    func_800861D4(soundHandle);
}

s32 func_800019C8(s32 volume);
void sndp_play(ALBank *bank, s16 sndIndx, u16 volume, u8 pan, f32 pitch, u8 fxMix, SoundHandle *handlePtr);

/**
 * Plays the requested sound from the sound table.
 * JFG: one call to the sound player with the table's volume (halved), a centred pan
 * and the pitch; the handle must be given.
 * Official Name: amSndPlay
 */
void sound_play(u16 soundID, SoundHandle *handlePtr) {
    f32 pitch;
    s32 soundBite;

    if (soundID > gSoundCount) {
        if (handlePtr != NULL) {
            *handlePtr = NULL;
        }
        return;
    }
    soundBite = gSoundTable[soundID].soundBite;
    if (soundBite == 0) {
        if (handlePtr != NULL) {
            *handlePtr = NULL;
        }
        return;
    }
    pitch = (u32) gSoundTable[soundID].pitch / 100.0f;
    sndp_play(gSoundBank->bankArray[0], (s16) soundBite, func_800019C8(gSoundTable[soundID].volume << 8), 0x40, pitch,
              0, handlePtr);
}

s32 func_800019C8(s32 volume);
void sndp_play(ALBank *bank, s16 sndIndx, u16 volume, u8 pan, f32 pitch, u8 fxMix, SoundHandle *handlePtr);

/**
 * Official Name: amSndPlayDirect
 * JFG: takes the volume, pan, pitch and effect mix; the handle must be given.
 */
void sound_play_direct(u16 soundID, u8 volume, u8 pan, f32 pitch, u8 fxMix, SoundHandle *handlePtr) {
    if (soundID <= 0 || sound_count() < soundID) {
        if (handlePtr) {
            *handlePtr = NULL;
        }
        return;
    }
    sndp_play(gSoundBank->bankArray[0], (s16) soundID, func_800019C8(volume << 8), pan, pitch, fxMix, handlePtr);
}

/**
 * Set the volume of the sound relative to the baseline volume of the sound ID.
 * JFG: the volume goes through func_800019C8 (halved).
 * Official Name: amSndSetVol
 */
void sound_volume_set_relative(u16 soundID, SoundHandle soundHandle, u8 volume) {
    s32 newVolume = (s32) ((u32) gSoundTable[soundID].volume * ((u32) volume / 127.0f)) << 8;
    if (soundHandle) {
        sndp_set_param(soundHandle, AL_SNDP_VOL_EVT, func_800019C8(newVolume));
    }
}

void func_800014D8(SoundHandle soundHandle, u32 pan) {
    if (soundHandle != NULL) {
        sndp_set_param(soundHandle, AL_SNDP_PAN_EVT, pan);
    }
}

/**
 * Updates the pitch of the given sound mask.
 * Official name: amSndSetPitchDirect
 */
void sound_pitch_set(SoundHandle soundHandle, u32 pitch) {
    u32 *pitchAddr = &pitch;
    if (soundHandle != NULL) {
        sndp_set_param(soundHandle, AL_SNDP_PITCH_EVT, *pitchAddr);
    }
}

/**
 * Returns the number of sounds in the sound bank.
 * Official Name: amGetSfxCount
 */
u16 sound_count(void) {
    return gSoundBank->bankArray[0]->instArray[0]->soundCount;
}

/**
 * Writes the sound table address, size and element count into the arguments.
 * Official Name: amGetSfxSettings
 */
void sound_table_properties(SoundData **table, s32 *size, s32 *count) {
    if (table != NULL) {
        *table = gSoundTable;
    }
    if (size != NULL) {
        *size = gSoundTableSize;
    }
    if (count != NULL) {
        *count = gSoundCount;
    }
}

/**
 * Returns true if the given soundID is looped.
 * Official Name: amSoundIsLooped
 */
u8 sound_is_looped(u16 soundID) {
    if (soundID <= 0 || gSoundBank->bankArray[0]->instArray[0]->soundCount < soundID) {
        return 0;
    }
    return ((u32) (1 + gSoundBank->bankArray[0]->instArray[0]->soundArray[soundID - 1]->envelope->decayTime) == 0);
}

/**
 * Stop the current sequence then set the parameters for the next sequence.
 */
void music_sequence_start(u8 seqID, ALCSPlayer *seqPlayer) {
    music_sequence_stop(seqPlayer);
    if (seqID < gSequenceTable->seqCount) {
        if (seqPlayer == gMusicPlayer) {
            gMusicNextSeqID = seqID;
        } else {
            gJingleNextSeqID = seqID;
        }
    } else {
        stubbed_printf("Invalid midi sequence index\n");
    }
}

INCLUDE_ASM("asm/nonmatchings/audio", func_8000167C);

extern u8 D_800A0688; // DKR: gMusicPlaying
void func_8008A450(ALCSPlayer *seqp); // stops a sequence player (DKR: alCSPStop)

/**
 * Stops the current playing sequence for the given sequence player.
 * JFG: the next sequence IDs are reset to 0.
 */
void music_sequence_stop(ALCSPlayer *seqPlayer) {
    if (gMusicPlayer == seqPlayer && D_800A0688) {
        func_8008A450(seqPlayer);
        D_800A0688 = FALSE;
        gMusicNextSeqID = 0;
    } else if (gJinglePlayer == seqPlayer && gJinglePlaying) {
        func_8008A450(seqPlayer);
        gJinglePlaying = FALSE;
        gJingleNextSeqID = 0;
    }
}

/**
 * Enable or disable special audio effects. JFG: does nothing (DKR calls alFxReverbSet).
 * Official Name: amTuneSetReverbOnOff
 */
void sound_reverb_set(u8 setting) {
}

void func_80086640(void *a, void *b);
void func_8008A490(void *a, void *b);

/* JFG: calls two sequence player routines with NULLs (never used at run time). */
void func_80001998(void) {
    func_80086640(NULL, NULL);
    func_8008A490(NULL, NULL);
}

/* JFG: halves a volume. */
s32 func_800019C8(s32 volume) {
    return volume * 0.5f;
}
