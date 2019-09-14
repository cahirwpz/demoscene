#ifndef __SOUND_H__
#define __SOUND_H__

#include "types.h"

typedef struct Sound {
  u_int length;
  u_short rate;
  u_char channel;
  u_char volume;
  char* sample;
} SoundT;

__regargs SoundT *NewSound(u_int length, u_short rate);
__regargs void DeleteSound(SoundT *sound);

typedef enum { CHAN_0, CHAN_1, CHAN_2, CHAN_3 } ChanT;

void AudioSetVolume(ChanT num, u_char level);
void AudioSetPeriod(ChanT num, u_short period);
void AudioSetSampleRate(ChanT num, u_short rate);
void AudioAttachSample(ChanT num, void *data, u_int length);
void AudioAttachSound(ChanT num, SoundT *sound);
void AudioPlay(ChanT num);
void AudioStop(ChanT num);

#endif
