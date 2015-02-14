#ifndef __TOOLS_PROFILING_H__
#define __TOOLS_PROFILING_H__

#include "std/types.h"

typedef struct Timing TimingT;

void StartProfiling();
void StopProfiling();

__regargs TimingT *ProfileGetTiming(const char *name);
__regargs bool Profile(TimingT *timing);

#define PROFILE(NAME) \
  while (({ \
            static TimingT *_Timing ## NAME = NULL; \
            if (!_Timing ## NAME) \
              _Timing ## NAME = ProfileGetTiming(#NAME); \
            Profile(_Timing ## NAME); \
          }))


#endif
