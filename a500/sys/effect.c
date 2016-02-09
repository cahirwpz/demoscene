#include "config.h"
#include "effect.h"

#if SHOW_MEMORY_STATS
# include "memory.h"
static void ShowMemStats() {
  Log("[Memory] CHIP: %ld/%ld FAST: %ld/%ld\n",
      MemAvail(MEMF_CHIP|MEMF_LARGEST), MemAvail(MEMF_CHIP),
      MemAvail(MEMF_FAST|MEMF_LARGEST), MemAvail(MEMF_FAST));
}
#else
# define ShowMemStats()
#endif;

#if REMOTE_CONTROL
# include "serial.h"
static __regargs void SendEffectStatus(EffectT *effect) {
  SerialPrint("ES %s %ld\n", effect->name, (LONG)effect->state);
}
#else
# define SendEffectStatus(x)
#endif

__regargs void EffectLoad(EffectT *effect) {
  if (effect->state & EFFECT_LOADED)
    return;

  if (effect->Load) {
    effect->Load();
    Log("[Effect] %s loaded.\n", effect->name);
    ShowMemStats();
  }

  effect->state |= EFFECT_LOADED;
  SendEffectStatus(effect);
}

__regargs void EffectPrepare(EffectT *effect) {
  if (effect->state & EFFECT_READY)
    return;

  if (effect->Prepare) {
    Log("[Effect] Preparing %s.\n", effect->name);
    effect->Prepare();
    ShowMemStats();
  }

  effect->state |= EFFECT_READY;
  SendEffectStatus(effect);
}

__regargs void EffectInit(EffectT *effect) {
  if (effect->Init)
    effect->Init();

  Log("[Effect] %s started.\n", effect->name);
  effect->state |= EFFECT_RUNNING;
  SendEffectStatus(effect);
}

__regargs void EffectKill(EffectT *effect) {
  if (effect->Kill)
    effect->Kill();

  Log("[Effect] %s finished.\n", effect->name);
  effect->state &= ~EFFECT_RUNNING;
  effect->state &= ~EFFECT_READY;
  SendEffectStatus(effect);
}

__regargs void EffectUnLoad(EffectT *effect) {
  if (!(effect->state & EFFECT_LOADED))
    return;

  if (effect->UnLoad) {
    effect->UnLoad();
    Log("[Effect] %s unloaded.\n", effect->name);
    ShowMemStats();
  }

  effect->state &= ~EFFECT_LOADED;
  SendEffectStatus(effect);
}
