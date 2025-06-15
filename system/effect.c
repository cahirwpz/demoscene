#include <custom.h>
#include <effect.h>
#include <system/cia.h>

short frameCount = 0;
short lastFrameCount = 0;
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

#define DONE 1

bool EffectIsRunning(EffectT *effect) {
  return (effect->Init.Status & DONE) ? true : false; 
}

void EffectLoad(EffectT *effect) {
  if (effect->Load.Status & DONE)
    return;

  if (effect->Load.Func) {
    Log("[Effect] Loading '%s'\n", effect->name);
    effect->Load.Func();
    ShowMemStats();
  }

  effect->Load.Status |= DONE; 
  SendEffectStatus(effect);
}

void EffectInit(EffectT *effect) {
  if (effect->Init.Status & DONE)
    return;

  if (effect->Init.Func) {
    Log("[Effect] Initializing '%s'\n", effect->name);
    effect->Init.Func();
    ShowMemStats();
  }

  effect->Init.Status |= DONE;
  SendEffectStatus(effect);
}

void EffectKill(EffectT *effect) {
  if (!(effect->Init.Status & DONE))
    return;

  if (effect->Kill) {
    Log("[Effect] Killing '%s'\n", effect->name);
    effect->Kill();
    ShowMemStats();
  }

  effect->Init.Status ^= DONE;
  SendEffectStatus(effect);
}

void EffectUnLoad(EffectT *effect) {
  if (!(effect->Load.Status & DONE))
    return;

  if (effect->UnLoad) {
    Log("[Effect] Unloading '%s'\n", effect->name);
    effect->UnLoad();
    ShowMemStats();
  }

  effect->Load.Status ^= DONE;
  SendEffectStatus(effect);
}

short ReadFrameCount(void) {
  return ReadFrameCounter();
}

void EffectRun(EffectT *effect) {
  SetFrameCounter(frameCount);

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

void TimeWarp(u_short frame) {
  frameCount = frame;
}
