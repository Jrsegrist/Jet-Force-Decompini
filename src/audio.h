#ifndef _AUDIO_H_
#define _AUDIO_H_

/* Adapted from the Diddy Kong Racing decompilation (CC0 1.0), src/audio.h and src/audiosfx.h. */

#include "common.h"
#include "PR/libaudio.h"

#define AUDIO_CHANNELS 16
#define MUSIC_CHAN_MASK_NONE 0xFFFFFFFF
#define SEQUENCE_NONE 0

#define AL_SNDP_PLAY_EVT (1 << 0)
#define AL_SNDP_RELEASE_EVT (1 << 1)
#define AL_SNDP_PAN_EVT (1 << 2)
#define AL_SNDP_VOL_EVT (1 << 3)
#define AL_SNDP_PITCH_EVT (1 << 4)

typedef struct ALSoundState *SoundHandle;

/* Size: 0x0A bytes */
typedef struct SoundData {
    u16 soundBite; // Audio file index.
    u8 volume;     // Multiplied by 256. 128 = 32768, max volume.
    u8 minVolume;
    u8 pitch; // Fractional. 100 = 1.0f.
    u8 unk5;
    u16 range; // Ingame units distance, same as any other.
    u8 priority;
    u8 unk9;
} SoundData;

/* Rare's additions to the sequence player (not in the standard headers). */
extern void set_voice_limit(ALCSPlayer *seqp, u8 voiceLimit);

/* The sound player (audiosfx.c in DKR). */
s32 sndp_get_global_volume(void);
void sndp_set_param(SoundHandle soundMask, s16 type, u32 paramValue);

void music_voicelimit_set(u8 voiceLimit);
void music_dynamic_set(u16 channelMask);
void music_channel_off(u8 channel);
void music_channel_on(u8 channel);
void music_channel_volume_set(u8 channel, u8 volume);
void music_tempo_set_relative(f32 tempo);
void music_tempo_set(s32 tempo);
s16 music_tempo(void);
void music_jingle_play_safe(u8 jingleID);
void sound_jingle_tempo_set(s32 tempo);
void music_stop(void);
void music_jingle_stop(void);
u8 music_current_sequence(void);
u8 music_jingle_current(void);
u8 music_volume(void);
void music_jingle_volume_set(u8 arg0);
void music_jingle_pan_set(ALPan pan);
u32 music_jingle_playing(void);
u16 sound_count(void);
void sound_pitch_set(SoundHandle soundHandle, u32 pitch);
void sound_table_properties(SoundData **table, s32 *size, s32 *count);
u8 sound_is_looped(u16 soundID);
void music_sequence_start(u8 seqID, ALCSPlayer *seqPlayer);
void music_sequence_stop(ALCSPlayer *seqPlayer);
void sound_reverb_set(u8 setting);

#endif
