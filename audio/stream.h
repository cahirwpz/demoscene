#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#include "std/types.h"

typedef struct AudioStream AudioStreamT;

AudioStreamT *AudioStreamOpen(const char *filename);
uint32_t AudioStreamGetSignal(AudioStreamT *audio);
size_t AudioStreamFeed(AudioStreamT *audio);
bool AudioStreamPlay(AudioStreamT *audio);
void AudioStreamStop(AudioStreamT *audio);

#endif
