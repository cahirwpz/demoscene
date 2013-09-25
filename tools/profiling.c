#include "std/debug.h"
#include "std/hashmap.h"
#include "std/memory.h"
#include "system/timer.h"
#include "tools/profiling.h"

struct Timing {
  double min, max, sum;
  int n;
  uint64_t start;
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

      LOG("%s: %.2fms (min: %.2fms, max: %.2fms)",
          key, timing->sum * 1000.0f / timing->n,
          timing->min * 1000.0f, timing->max * 1000.0f);
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
    timing->min = 60.0f;
    HashMapAdd(Timings, name, timing);
    LOG("Added timing '%s'.", name);
  }

  return timing;
}

__regargs bool Profile(TimingT *timing) {
  if (!timing->active) {
    timing->start = TimerRead();
    timing->active = true;
  } else {
    double value = TimerDiff(timing->start, TimerRead());

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
