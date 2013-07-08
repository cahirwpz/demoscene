#ifndef __SYSTEM_AUDIO_H__
#define __SYSTEM_AUDIO_H__

#include "std/types.h"

typedef enum { CHAN_0, CHAN_1, CHAN_2, CHAN_3 } ChanT;

typedef void (*AudioIntHandlerT)(ChanT num, PtrT userData);

bool InitAudio();
void KillAudio();

bool AudioFilter(bool on);
void AudioSetVolume(ChanT num, uint8_t level);
void AudioSetSampleRate(ChanT num, uint16_t rate);
void AudioAttachSamples(ChanT num, uint16_t *data, uint32_t length);
void AudioPlay(ChanT num);
void AudioStop(ChanT num);

void AudioIntActivate(ChanT num, bool on);
void AudioIntSetHandler(ChanT num, AudioIntHandlerT handler, PtrT userData);

uint16_t *AllocAudioData(size_t length);

#endif
