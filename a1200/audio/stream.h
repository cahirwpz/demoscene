#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#include "std/types.h"

typedef struct AudioBuffer {
  uint32_t length;
  struct {
    uint8_t *hi, *lo;
  } left;
  struct {
    uint8_t *hi, *lo;
  } right;
} AudioBufferT;

typedef struct AudioStream AudioStreamT;

AudioStreamT *AudioStreamOpen(const char *filename);
void AudioStreamFeed(AudioStreamT *audio);
bool AudioStreamPlay(AudioStreamT *audio);
void AudioStreamStop(AudioStreamT *audio);
void AudioStreamSetVolume(AudioStreamT *audio, float volume);
void AudioStreamUpdatePos(AudioStreamT *audio);
AudioBufferT *AudioStreamGetBuffer(AudioStreamT *audio);

#endif
