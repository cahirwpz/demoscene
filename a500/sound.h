#ifndef __SOUND_H__
#define __SOUND_H__

#include <exec/types.h>

typedef struct Sound {
  ULONG length;
  UWORD rate;
  UBYTE channel;
  UBYTE volume;
  BYTE* sample;
} SoundT;

__regargs SoundT *NewSound(ULONG length, UWORD rate);
__regargs void DeleteSound(SoundT *sound);

typedef enum { CHAN_0, CHAN_1, CHAN_2, CHAN_3 } ChanT;

void AudioSetVolume(ChanT num, UBYTE level);
void AudioSetPeriod(ChanT num, UWORD period);
void AudioSetSampleRate(ChanT num, UWORD rate);
void AudioAttachSample(ChanT num, APTR data, ULONG length);
void AudioAttachSound(ChanT num, SoundT *sound);
void AudioPlay(ChanT num);
void AudioStop(ChanT num);

#endif
