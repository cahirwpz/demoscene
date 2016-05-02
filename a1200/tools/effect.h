#ifndef __TOOLS_EFFECT_H__
#define __TOOLS_EFFECT_H__

#include "std/types.h"

typedef enum {
  EFFECT_LOADED = 1,
  EFFECT_READY = 2,
  EFFECT_RUNNING = 4
} EffectStateT;

typedef struct Effect {
  const char *name;
  void (*Load)();
  void (*UnLoad)();
  void (*Init)();
  void (*Kill)();
  void (*Loop)();
  EffectStateT state;
} EffectT;

__regargs void EffectLoad(EffectT *effect);
__regargs void EffectInit(EffectT *effect);
__regargs void EffectKill(EffectT *effect);
__regargs void EffectUnLoad(EffectT *effect);

#define EFFECT(NAME, A1, A2, A3, A4, A5) \
  EffectT NAME = { #NAME, (A1), (A2), (A3), (A4), (A5) };

#endif
