#ifndef __STD_RANDOM_H__
#define __STD_RANDOM_H__

#include "std/types.h"

typedef struct Random {
  uint32_t value;
  uint32_t seed;
} RandomT;

__regargs int NextRandom(RandomT *random);
__regargs float NextRandomFloat(RandomT *random, float lo, float hi);

#endif
