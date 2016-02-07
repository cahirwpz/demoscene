#include "config.h"
#include "timeline.h"

WORD frameFromStart;
WORD frameTillEnd;
WORD frameCount;
WORD lastFrameCount;
TimeSlotT *currentTimeSlot;

#if SHOW_MEMORY_STATS
# include "memory.h"
# define SHOWMEM ShowMemStats
static void ShowMemStats() {
  Log("[Memory] CHIP: %ld/%ld FAST: %ld/%ld\n",
      MemAvail(MEMF_CHIP|MEMF_LARGEST), MemAvail(MEMF_CHIP),
      MemAvail(MEMF_FAST|MEMF_LARGEST), MemAvail(MEMF_FAST));
}
#else
#define SHOWMEM()
#endif;

__regargs TimeSlotT *TimelineForward(TimeSlotT *item, WORD pos) {
  for (; item->effect; item++)
    if ((item->start <= pos) && (pos < item->end))
      break;
  return item->effect ? item : NULL;
}

__regargs void LoadEffects(TimeSlotT *item, WORD phase) {
  TimeSlotT *curr = item;

  if (phase < 0)
    phase = item->phase;

  while (curr->effect)
    curr++;

  while (--curr >= item) {
    EffectT *effect = curr->effect;

    if (phase != curr->phase)
      continue;

    if (effect->state & EFFECT_LOADED)
      continue;

    if (effect->Load) {
      effect->Load();
      Log("[Effect] %s loaded.\n", curr->name);
      SHOWMEM();
    }

    effect->state |= EFFECT_LOADED;
  }
}

__regargs void UnLoadEffects(TimeSlotT *item) {
  for (; item->effect; item++) {
    EffectT *effect = item->effect;

    if (!(effect->state & EFFECT_LOADED))
      continue;

    if (effect->UnLoad)
      effect->UnLoad();

    effect->state &= ~EFFECT_LOADED;
  }
}

__regargs void PrepareEffect(EffectT *effect) {
  if (effect->state & EFFECT_READY)
    return;

  if (effect->Prepare)
    effect->Prepare();

  effect->state |= EFFECT_READY;
}

__regargs void RunEffects(TimeSlotT *item) {
  BOOL exit = FALSE;

  SetFrameCounter(item->start);

  for (; item->effect && !exit; item++) {
    EffectT *effect = item->effect;
    WORD realStart;

    if (!(ReadFrameCounter() < item->end))
      continue;

    while (ReadFrameCounter() < item->start) {
      if (LeftMouseButton()) {
        exit = TRUE;
        break;
      }
    }

    currentTimeSlot = item;

    if (!(effect->state & EFFECT_READY)) {
      if (effect->Prepare) {
        Log("[Effect] Preparing %s.\n", item->name);
        effect->Prepare();
        SHOWMEM();
      }
    }
    if (effect->Init)
      effect->Init();

    frameFromStart = 0;
    frameTillEnd = item->end - item->start;
    realStart = ReadFrameCounter();

    lastFrameCount = ReadFrameCounter();
    while (frameCount < item->end) {
      WORD t = ReadFrameCounter();
      frameCount = t;
      frameFromStart = frameCount - realStart;
      frameTillEnd = item->end - frameCount;
      if (effect->Render)
        effect->Render();
      else
        WaitVBlank();
      lastFrameCount = t;

      if (LeftMouseButton()) {
        exit = TRUE;
        break;
      }
    }

    if (effect->Kill)
      effect->Kill();

    WaitVBlank();

    if (effect->UnLoad) {
      Log("[Effect] %s finished.\n", item->name);

      effect->UnLoad();
      effect->state &= ~EFFECT_LOADED;

      SHOWMEM();
    }

    currentTimeSlot = NULL;
  }
}

void UpdateFrameCount() {
  frameCount = ReadFrameCounter();
  frameFromStart = frameCount - currentTimeSlot->start;
  frameTillEnd = currentTimeSlot->end - frameCount;
}
