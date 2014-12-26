#include "timeline.h"

#define SHOW_MEMUSED 0

#if SHOW_MEMUSED
# include "memory.h"
#endif

WORD frameFromStart;
WORD frameTillEnd;
WORD frameCount;
WORD lastFrameCount;
void (*currentInterruptHandler)();

static TimelineItemT *currentItem;

__regargs void LoadEffects(TimelineItemT *item, WORD n) {
  item += n - 1;

  for (; --n >= 0; item--) {
    EffectT *effect = item->effect;

    if (effect->state & EFFECT_LOADED)
      continue;

    if (effect->Load)
      effect->Load();

    effect->state |= EFFECT_LOADED;
  }
}

__regargs void UnLoadEffects(TimelineItemT *item, WORD n) {
  for (; --n >= 0; item++) {
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

__regargs void RunEffects(TimelineItemT *item, WORD n) {
  BOOL exit = FALSE;

  SetFrameCounter(0);

  for (; --n >= 0 && !exit; item++) {
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

    currentItem = item;

    if (!(effect->state & EFFECT_READY))
      if (effect->Prepare)
        effect->Prepare();
    if (effect->Init)
      effect->Init();

    frameFromStart = 0;
    frameTillEnd = item->end - item->start;
    realStart = ReadFrameCounter();

    currentInterruptHandler = effect->InterruptHandler;

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

    currentInterruptHandler = NULL;

    if (effect->Kill)
      effect->Kill();

    WaitVBlank();

    if (effect->UnLoad) {
      effect->UnLoad();
      effect->state &= ~EFFECT_LOADED;

#if SHOW_MEMUSED
      Log("Chip memory used   : %ld bytes.\n", MemUsed(MEMF_CHIP));
      Log("Public memory used : %ld bytes.\n", MemUsed(MEMF_PUBLIC));
#endif
    }

    currentItem = NULL;
  }
}

void UpdateFrameCount() {
  frameCount = ReadFrameCounter();
  frameFromStart = frameCount - currentItem->start;
  frameTillEnd = currentItem->end - frameCount;
}
