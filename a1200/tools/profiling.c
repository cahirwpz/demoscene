#include "std/debug.h"
#include "std/hashmap.h"
#include "std/memory.h"
#include "system/hardware.h"
#include "tools/profiling.h"

struct Timing {
  float min, max, sum;
  int n;
  int start;
  bool active;
};

static HashMapT *Timings = NULL;

void StartProfiling() {
  if (!Timings)
    Timings = NewHashMap(50);
}

void StopProfiling() {
  if (Timings) {
    void PrintTiming(const char *key, PtrT value) {
      TimingT *timing = (TimingT *)value;

      printf("%s: %.2fms (min: %.2fms, max: %.2fms)\n\r", key,
             timing->sum / timing->n, timing->min, timing->max);
    }

    LOG("Timing measurements:");
    HashMapIter(Timings, PrintTiming);

    MemUnref(Timings);
    Timings = NULL;
  }
}

__regargs TimingT *ProfileGetTiming(const char *name) {
  TimingT *timing = HashMapFind(Timings, name);

  if (!timing) {
    timing = NewRecord(TimingT);
    timing->active = false;
    timing->min = FLT_MAX;
    timing->max = FLT_MIN;
    HashMapAdd(Timings, name, timing);
    LOG("Added timing '%s'.", name);
  }

  return timing;
}

__regargs bool Profile(TimingT *timing) {
  if (!timing->active) {
    timing->start = ReadLineCounter();
    timing->active = true;
  } else {
    int ticks = ReadLineCounter() - timing->start;
    float value;
   
    /* Check for counter overflow. */
    if (ticks < 0)
      ticks += 1 << 24;

    /* Each line has 64us, but we count miliseconds. */
    value = (float)ticks * 64.0f / 1000.0f;
   
    if (timing->min > value)
      timing->min = value;
    if (timing->max < value)
      timing->max = value;

    timing->sum += value;
    timing->n++;

    timing->active = false;
  }

  return timing->active;
}
