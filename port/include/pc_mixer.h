/* pc_mixer.h — our replacement for the GBA's sound output. */
#ifndef PC_MIXER_H
#define PC_MIXER_H

struct SoundInfo;
struct SoundChannel;

#define PC_AUDIO_RATE 48000

/* After the engine's per-VBlank update: render one frame of audio. */
void PcMixerVBlank(struct SoundInfo *si);
static inline void PcMixerChannelActive(int index, struct SoundChannel *chan) { (void)index; (void)chan; }

/* audio.c: hand rendered stereo frames to the audio device. */
void PcAudioPush(const float *stereo, int frames);

#endif
