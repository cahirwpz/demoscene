#ifndef __AUDIO_WAVE_H__
#define __AUDIO_WAVE_H__

#include "system/rwops.h"

typedef enum {
  WAVE_PCM = 1,
  WAVE_IMA_ADPCM = 17
} WaveFmtT;

typedef struct {
  RwOpsT *file;
  WaveFmtT format;
  uint8_t  channels;
  uint8_t  bitsPerSample;
  uint16_t blockAlign;
  uint16_t samplesPerSec;
  size_t   samplesNum;
  size_t   samplesOffset;
} WaveFileT;

bool WaveFileOpen(WaveFileT *file, const char *filename);
void WaveFileClose(WaveFileT *file);
void WaveFileChangePosition(WaveFileT *file, float second);
size_t WaveFileReadSamples(WaveFileT *wave, PtrT samples, size_t requested);

static inline int SampleWidth(WaveFileT *file) {
  return file->channels * file->bitsPerSample / 8;
}

#endif
