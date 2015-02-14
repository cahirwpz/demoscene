#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"

#include "envelope.h"

static inline float Linear(float start, float end, float t) {
  return (end - start) * t + start;
}

static inline float SmoothStep(float start, float end, float t) {
  return t * t * (3.0f - 2.0f * t);
}

static void DeleteEnvelope(EnvelopeT *envelope) {
  MemUnref(envelope->point);
}

TYPEDECL(EnvelopeT, (FreeFuncT)DeleteEnvelope);

EnvelopeT *NewEnvelope(EnvTypeT type, size_t dimensions, size_t points) {
  EnvelopeT *envelope = NewInstance(EnvelopeT);

  envelope->type = type;
  envelope->dimensions = dimensions;
  envelope->point = NewTable(EnvPointT, points);

  return envelope;
}

void PrintEnvelope(EnvelopeT *env) {
  int i, j;

  for (i = 0; i < TableSize(env->point); i++) {
    LOG("point[%d], time = %f", i, env->point[i].time);
    for (j = 0; j < env->dimensions; j++)
      LOG("point[%d], value[%d] = %f", i, j, env->point[i].value[j])
  }
}

void EnvelopeEvaluate(EnvelopeT *env, float time, float *value) {
  size_t n = TableSize(env->point);
  EnvPointT *point = env->point;
  float *start = NULL;
  float *end = NULL;
  float t = 0.0f;
  int i;

  if (time < point[0].time) {
    start = point[0].value;
    end = point[0].value;
    t = 0.0;
  } else if (point[n - 1].time <= time) {
    start = point[n - 1].value;
    end = point[n - 1].value;
    t = 1.0;
  } else {
    for (i = 1; i < n; i++) {
      if (time < point[i].time) {
        float first = point[i - 1].time;
        float last = point[i].time;
        start = point[i - 1].value;
        end = point[i].value;
        t = (time - first) / (last - first);
        break;
      }
    }
  }

  for (i = 0; i < env->dimensions; i++) {
    if (env->type == ENV_POLYLINE)
      value[i] = Linear(start[i], end[i], t);
    else if (env->type == ENV_SMOOTHSTEP)
      value[i] = Linear(start[i], end[i], SmoothStep(start[i], end[i], t));
  }
}
