#include "timeline.h"

#define MEMSTATS 1

WORD frameFromStart;
WORD frameTillEnd;
WORD frameCount;
WORD lastFrameCount;

static TimelineItemT *currentItem;

#ifdef MEMSTATS
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

__regargs void LoadEffects(TimelineItemT *item, WORD n, WORD start) {
  item += n - 1;

  for (; --n >= 0; item--) {
    EffectT *effect = item->effect;

    if (item->start < start)
      continue;

    if (effect->state & EFFECT_LOADED)
      continue;

    if (effect->Load) {
      effect->Load();
      Log("[Effect] %s loaded.\n", item->name);
      SHOWMEM();
    }

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

  if (effect->Prepare) {
    effect->Prepare();
#if SHOW_MEMUSED
#endif
  }

  effect->state |= EFFECT_READY;
}

__regargs void RunEffects(TimelineItemT *item, WORD n, WORD start) {
  BOOL exit = FALSE;

  SetFrameCounter(start);

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

    currentItem = NULL;
  }
}

void UpdateFrameCount() {
  frameCount = ReadFrameCounter();
  frameFromStart = frameCount - currentItem->start;
  frameTillEnd = currentItem->end - frameCount;
}
