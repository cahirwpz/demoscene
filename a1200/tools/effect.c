#include "std/debug.h"
#include "tools/effect.h"

__regargs void EffectLoad(EffectT *effect) {
  if (effect->state & EFFECT_LOADED)
    return;

  if (effect->Load) {
    effect->Load();
    LOG("[Effect] %s loaded.\n", effect->name);
  }

  effect->state |= EFFECT_LOADED;
}

__regargs void EffectInit(EffectT *effect) {
  if (effect->Init)
    effect->Init();

  LOG("[Effect] %s started.\n", effect->name);
  effect->state |= EFFECT_RUNNING;
}

__regargs void EffectKill(EffectT *effect) {
  if (effect->Kill)
    effect->Kill();

  LOG("[Effect] %s finished.\n", effect->name);
  effect->state &= ~EFFECT_RUNNING;
  effect->state &= ~EFFECT_READY;
}

__regargs void EffectUnLoad(EffectT *effect) {
  if (!(effect->state & EFFECT_LOADED))
    return;

  if (effect->UnLoad) {
    effect->UnLoad();
    LOG("[Effect] %s unloaded.\n", effect->name);
  }

  effect->state &= ~EFFECT_LOADED;
}
