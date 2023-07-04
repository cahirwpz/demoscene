#include <custom.h>
#include <effect.h>
#include <system/cia.h>

int frameCount = 0;
int lastFrameCount = 0;
bool exitLoop = false;

#define SHOW_MEMORY_STATS 0
#define REMOTE_CONTROL 0

#if SHOW_MEMORY_STATS
# include <system/memory.h>
static void ShowMemStats(void) {
  Log("[Memory] CHIP: %d/%d FAST: %d/%d\n",
      MemAvail(MEMF_CHIP|MEMF_LARGEST), MemAvail(MEMF_CHIP),
      MemAvail(MEMF_FAST|MEMF_LARGEST), MemAvail(MEMF_FAST));
}
#else
# define ShowMemStats()
#endif

#if REMOTE_CONTROL
# include <system/file.h>
static void SendEffectStatus(EffectT *effect) {
  FilePrintf("ES %s %d\n", effect->name, effect->state);
}
#else
# define SendEffectStatus(x)
#endif

void EffectLoad(EffectT *effect) {
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

void EffectInit(EffectT *effect) {
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

void EffectKill(EffectT *effect) {
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

void EffectUnLoad(EffectT *effect) {
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

void EffectRun(EffectT *effect) {
  SetFrameCounter(0);

  lastFrameCount = ReadFrameCounter();

  do {
    int t = ReadFrameCounter();
    exitLoop = LeftMouseButton();
    frameCount = t;
    if ((lastFrameCount != frameCount) && effect->Render)
      effect->Render();
    lastFrameCount = t;
  } while (!exitLoop);
}
