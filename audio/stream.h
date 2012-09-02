#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#include "std/types.h"

typedef struct AudioStream AudioStreamT;

AudioStreamT *AudioStreamOpen(const char *filename);
size_t AudioStreamFeed(AudioStreamT *audio);
bool AudioStreamPlay(AudioStreamT *audio);
void AudioStreamStop(AudioStreamT *audio);

uint32_t AudioStreamHungryWait(AudioStreamT *audio, uint32_t extraSignals);
bool AudioStreamIsHungry(AudioStreamT *audio);

#endif
