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

/******************************/

// The debug strings of functions that are still assembly, in source order.
INCLUDE_RODATA("asm/nonmatchings/audio", D_800A9FB0);

INCLUDE_ASM("asm/nonmatchings/audio", func_80000450);

INCLUDE_ASM("asm/nonmatchings/audio", func_80000510);

/**
 * Update the background music voice limit if not prevented from doing so.
 * Official Name: amTuneVoiceLimit
 */
void music_voicelimit_set(u8 voiceLimit) {
    if (gBlockVoiceLimitChange == FALSE) {
        set_voice_limit(gMusicPlayer, voiceLimit);
    }
}

INCLUDE_ASM("asm/nonmatchings/audio", func_800005F8);

INCLUDE_ASM("asm/nonmatchings/audio", func_80000724);

INCLUDE_ASM("asm/nonmatchings/audio", func_80000798);

INCLUDE_ASM("asm/nonmatchings/audio", func_800007A4);

INCLUDE_ASM("asm/nonmatchings/audio", func_800008D0);

INCLUDE_ASM("asm/nonmatchings/audio", func_800008DC);

INCLUDE_ASM("asm/nonmatchings/audio", func_80000B80);

INCLUDE_ASM("asm/nonmatchings/audio", func_80000C00);

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

INCLUDE_ASM("asm/nonmatchings/audio", func_80000FE4);

INCLUDE_ASM("asm/nonmatchings/audio", func_80001050);

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

INCLUDE_ASM("asm/nonmatchings/audio", func_80001170);

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

INCLUDE_ASM("asm/nonmatchings/audio", func_80001228);

INCLUDE_ASM("asm/nonmatchings/audio", func_80001248);

INCLUDE_ASM("asm/nonmatchings/audio", func_8000134C);

INCLUDE_ASM("asm/nonmatchings/audio", func_800013FC);

INCLUDE_ASM("asm/nonmatchings/audio", func_800014D8);

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

INCLUDE_ASM("asm/nonmatchings/audio", func_80001900);

/**
 * Enable or disable special audio effects. JFG: does nothing (DKR calls alFxReverbSet).
 * Official Name: amTuneSetReverbOnOff
 */
void sound_reverb_set(u8 setting) {
}

INCLUDE_ASM("asm/nonmatchings/audio", func_80001998);

/* JFG: halves a volume. */
s32 func_800019C8(s32 volume) {
    return volume * 0.5f;
}
