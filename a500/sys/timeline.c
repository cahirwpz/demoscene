#include "timeline.h"

WORD frameFromStart;
WORD frameTillEnd;
WORD frameCount;
WORD lastFrameCount;
TimeSlotT *currentTimeSlot;

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

    EffectLoad(effect);
  }
}

__regargs void UnLoadEffects(TimeSlotT *item) {
  for (; item->effect; item++)
    EffectUnLoad(item->effect);
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

    EffectPrepare(effect);
    EffectInit(effect);

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

    EffectKill(effect);
    WaitVBlank();
    EffectUnLoad(effect);

    currentTimeSlot = NULL;
  }
}

void UpdateFrameCount() {
  frameCount = ReadFrameCounter();
  frameFromStart = frameCount - currentTimeSlot->start;
  frameTillEnd = currentTimeSlot->end - frameCount;
}
