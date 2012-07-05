#ifndef __STD_RANDOM_H__
#define __STD_RANDOM_H__

#include "std/types.h"

typedef struct Random {
  uint32_t value;
  uint32_t seed;
} RandomT;

int NextRandom(RandomT *random asm("a0"));

#endif
