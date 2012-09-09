#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#include "std/types.h"

typedef struct AudioStream AudioStreamT;

AudioStreamT *AudioStreamOpen(const StrT filename);
size_t AudioStreamFeed(AudioStreamT *audio);
ssize_t AudioStreamFeedIfHungry(AudioStreamT *audio);
bool AudioStreamPlay(AudioStreamT *audio);
void AudioStreamStop(AudioStreamT *audio);

uint32_t AudioStreamHungryWait(AudioStreamT *audio, uint32_t extraSignals);
void AudioStreamRewind(AudioStreamT *audio);

#endif
