#include "timeline.h"
#if REMOTE_CONTROL
# include "serial.h"
#endif

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

#if REMOTE_CONTROL
  char cmd[16];
  WORD cmdLen = 0;
  memset(cmd, 0, sizeof(cmd));
#endif

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
#if REMOTE_CONTROL
      SerialPrint("F %ld\n", (LONG)t);
#endif
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
#if REMOTE_CONTROL
      {
        LONG c;

        while ((c = SerialGet()) >= 0) {
          if (c == '\n') {
            Log("[Serial] Received line '%s'\n", cmd);
            memset(cmd, 0, sizeof(cmd));
            cmdLen = 0;
          } else {
            cmd[cmdLen++] = c;
          }
        }
      }
#endif
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
