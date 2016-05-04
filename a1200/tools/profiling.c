#include "std/debug.h"
#include "std/hashmap.h"
#include "std/memory.h"
#include "system/hardware.h"
#include "tools/profiling.h"

struct Timing {
  int min, max, sum, n;
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

      printf("%s: %.2fms (min: %.2fms, max: %.2fms)\n\r",
             key, timing->sum * 1000.0f / (timing->n * 64.0f),
             timing->min * 1000.0f / 64.0f, timing->max * 1000.0f / 64.0f);
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
    int value = timing->start - ReadLineCounter();
   
    /* Check for counter overflow. */
    if (value < 0)
      value += 1 << 24;
   
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
