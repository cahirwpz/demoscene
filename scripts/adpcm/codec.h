#ifndef __CODEC_H__
#define __CODEC_H__

#include <stdint.h>

typedef struct {
  int valprev;
  int index;
} CodecState;

void encode(CodecState *state, int16_t *input, int numSamples, uint8_t *output);
void decode(CodecState *state, uint8_t *input, int numSamples, int16_t *output);

void initDecode68000();
void decode68000(CodecState *state,
                 uint8_t *input, int numSamples, int16_t *output);

#endif
