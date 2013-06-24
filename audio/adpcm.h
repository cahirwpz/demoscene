#ifndef __AUDIO_ADPCM_H__
#define __AUDIO_ADPCM_H__

#include "std/types.h"

typedef struct {
  int valprev;
  int index;
} CodecStateT;

void InitADPCM();
void DecodeADPCM(CodecStateT *state,
                 uint8_t *input, int numSamples, int16_t *output);

#endif
