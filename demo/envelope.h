#ifndef __ENVELOPE_H__
#define __ENVELOPE_H__

#include "std/types.h"

typedef enum { ENV_POLYLINE, ENV_SMOOTHSTEP } EnvTypeT;

typedef struct EnvPoint {
  float time;
  float value[3];
} EnvPointT;

typedef struct Envelope {
  EnvTypeT type;
  size_t dimensions;
  EnvPointT *point;
} EnvelopeT;

EnvelopeT *NewEnvelope(EnvTypeT type, size_t dimensions, size_t points);
void EnvelopeEvaluate(EnvelopeT *env, float time, float *value);
void PrintEnvelope(EnvelopeT *env);

#endif
