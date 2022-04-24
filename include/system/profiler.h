#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <types.h>

typedef struct Profile {
  const char *name;
  u_int lines, total;
  u_short min, max;
  u_short count;
} ProfileT;

#define PROFILE(NAME)                                                          \
  static ProfileT *_##NAME##_profile = &(ProfileT){                            \
    .name = #NAME, .lines = 0, .total = 0, .min = 65535, .max = 0, .count = 0};

#define ProfilerStart(NAME) _ProfilerStart(_##NAME##_profile)
#define ProfilerStop(NAME) _ProfilerStop(_##NAME##_profile)
void _ProfilerStart(ProfileT *prof);
void _ProfilerStop(ProfileT *prof);

#endif
