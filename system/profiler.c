#include <debug.h>
#include <common.h>
#include <cia.h>

#include "profiler.h"
#include "effect.h"

void _ProfilerStart(ProfileT *prof) {
  prof->lines = ReadLineCounter();
}

void _ProfilerStop(ProfileT *prof) {
  u_short lines = ReadLineCounter() - prof->lines;

  if (lines > 32767)
    lines = -lines;

  if (lines < prof->min)
    prof->min = lines;
  if (lines > prof->max)
    prof->max = lines;

  prof->total += lines;
  prof->count++;

  /* Report every second! */
  if (div16(lastFrameCount, 50) < div16(frameCount, 50))
    Log("%s took %d-%d-%d (min-avg-max) raster lines.\n",
        prof->name, prof->min, div16(prof->total, prof->count), prof->max);
}
