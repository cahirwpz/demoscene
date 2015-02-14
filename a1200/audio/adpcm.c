/*
 * This file contains Kalms' ADPCM decoder tailored to execution on 68000.
 */

#include "audio/adpcm.h"

/* Intel ADPCM step variation table */
static int indexTable[16] = {
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] = {
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
  19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
  130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
  337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
  876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
  2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
  5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int indexTable68000[16];
static int stepsizeTable68000[89*16];

void InitADPCM() {
  int i, delta;

  for (i = 0; i < 16; i++)
    indexTable68000[i] = indexTable[i] << 4;

  for (i = 0; i < 89; i++) {
    for (delta = 0; delta < 16; delta++) {
      int origPredictor = stepsizeTable[i];
      int predictor = origPredictor >> 3;
      if (delta & 4)
        predictor += origPredictor;
      if (delta & 2)
        predictor += origPredictor >> 1;
      if (delta & 1)
        predictor += origPredictor >> 2;
      if (delta & 8)
        predictor = -predictor;
      stepsizeTable68000[i * 16 + delta] = predictor;
    }
  }
}

void DecodeADPCM(CodecStateT* state,
                 uint8_t* input, int numSamples, int16_t* output)
{
  uint8_t *inp;     /* Input buffer pointer */
  int16_t *outp;    /* output buffer pointer */
  int delta;        /* Current adpcm output value */
  int valpred;      /* Predicted value */
  int vpdiff;       /* Current change to valpred */
  int index;        /* Current step change index */
  int inputbuffer;  /* place to keep next 4-bit value */
  int bufferstep;   /* toggle between inputbuffer/input */

  outp = output;
  inp = input;

  valpred = state->valprev;
  index = state->index;

  bufferstep = 0;
  inputbuffer = 0;

  for ( ; numSamples > 0 ; numSamples-- ) {

    if ( bufferstep ) {
      delta = inputbuffer & 0xf;
    } else {
      inputbuffer = *inp++;
      delta = (inputbuffer >> 4) & 0xf;
    }
    bufferstep = !bufferstep;

    vpdiff = stepsizeTable68000[index | delta];
    index += indexTable68000[delta];
    if ( index < 0 ) index = 0;
    if ( index > (88 << 4) ) index = (88 << 4);

    valpred += vpdiff;

    if ( valpred > 32767 )
      valpred = 32767;
    else if ( valpred < -32768 )
      valpred = -32768;

    *outp++ = valpred;
  }

  state->valprev = valpred;
  state->index = index;
}
