#include "common.h"
#include "effect.h"
#include "hardware.h"

#define SHOW_MEMORY_STATS 0
#define REMOTE_CONTROL 0

#if SHOW_MEMORY_STATS
# include "memory.h"
static void ShowMemStats(void) {
  Log("[Memory] CHIP: %d/%d FAST: %d/%d\n",
      MemAvail(MEMF_CHIP|MEMF_LARGEST), MemAvail(MEMF_CHIP),
      MemAvail(MEMF_FAST|MEMF_LARGEST), MemAvail(MEMF_FAST));
}
#else
# define ShowMemStats()
#endif

#if REMOTE_CONTROL
# include "serial.h"
static __regargs void SendEffectStatus(EffectT *effect) {
  SerialPrint("ES %s %d\n", effect->name, effect->state);
}
#else
# define SendEffectStatus(x)
#endif

__regargs void EffectLoad(EffectT *effect) {
  if (effect->state & EFFECT_LOADED)
    return;

  if (effect->Load) {
    Log("[Effect] Loading '%s'\n", effect->name);
    effect->Load();
    ShowMemStats();
  }

  effect->state |= EFFECT_LOADED;
  SendEffectStatus(effect);
}

__regargs void EffectInit(EffectT *effect) {
  if (effect->state & EFFECT_READY)
    return;

  if (effect->Init) {
    Log("[Effect] Initializing '%s'\n", effect->name);
    effect->Init();
    ShowMemStats();
  }

  effect->state |= EFFECT_READY;
  SendEffectStatus(effect);
}

__regargs void EffectKill(EffectT *effect) {
  if (!(effect->state & EFFECT_READY))
    return;

  if (effect->Kill) {
    Log("[Effect] Killing '%s'\n", effect->name);
    effect->Kill();
    ShowMemStats();
  }

  effect->state &= ~EFFECT_READY;
  SendEffectStatus(effect);
}

__regargs void EffectUnLoad(EffectT *effect) {
  if (!(effect->state & EFFECT_LOADED))
    return;

  if (effect->UnLoad) {
    Log("[Effect] Unloading '%s'\n", effect->name);
    effect->UnLoad();
    ShowMemStats();
  }

  effect->state &= ~EFFECT_LOADED;
  SendEffectStatus(effect);
}

int frameCount;
int lastFrameCount;
bool exitLoop;

__regargs void EffectRun(EffectT *effect) {
  SetFrameCounter(0);

  lastFrameCount = ReadFrameCounter();

  do {
    int t = ReadFrameCounter();
    exitLoop = LeftMouseButton();
    frameCount = t;
    if (effect->Render)
      effect->Render();
    lastFrameCount = t;
  } while (!exitLoop);
}
