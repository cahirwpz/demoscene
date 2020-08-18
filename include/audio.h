#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <custom.h>

typedef struct Sound {
  u_int length;
  u_short rate;
  char *samples;
} SoundT;

typedef enum { CH0, CH1, CH2, CH3 } ChannelT;

/* Volume level is 6-bit value from range [0, 64). */
static inline void AudioSetVolume(ChannelT num, u_short level) {
  custom->aud[num].ac_vol = (u_short)level;
}

/* Maximum play rate is 31150 Hz which corresponds to numer of raster lines
 * displayed each second (assuming PAL signal). */
static inline void AudioSetSampleRate(ChannelT num, u_short rate) {
  custom->aud[num].ac_per = (u_short)(3579546L / rate);
}

/* Pointer to sound samples must be word-aligned, length must be even. */
static inline void AudioAttachSample(ChannelT num, char *samples,
                                     size_t length) {
  custom->aud[num].ac_ptr = (u_short *)samples;
  custom->aud[num].ac_len = (u_short)(length >> 1);
}

static inline void AudioAttachSound(ChannelT num, SoundT *sound) {
  AudioAttachSample(num, sound->samples, sound->length);
  AudioSetSampleRate(num, sound->rate);
}

static inline void AudioPlay(ChannelT num) {
  EnableDMA(1 << (DMAB_AUD0 + num));
}

static inline void AudioStop(ChannelT num) {
  DisableDMA(1 << (DMAB_AUD0 + num));
}

#endif
