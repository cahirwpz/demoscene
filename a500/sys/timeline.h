#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "effect.h"
#include "hardware.h"

typedef struct TimeSlot {
  WORD start, end, phase;
  EffectT *effect;
  const char *name;
} TimeSlotT;

#ifndef FRAMES_PER_ROW
#error "FRAMES_PER_ROW: not defined!"
#endif

#define _TS(begin, end, phase, effect) \
  { ((((begin) >> 8) & 0xff) * 64 + ((begin) & 0x3f)) * FRAMES_PER_ROW, \
    ((((end) >> 8) & 0xff) * 64 + ((end) & 0x3f)) * FRAMES_PER_ROW, \
    phase, &(effect), #effect }

#define _TS_END() {0, 0, 0, NULL, NULL}

extern WORD frameCount;
extern WORD lastFrameCount;
extern WORD frameFromStart;
extern WORD frameTillEnd;
extern TimeSlotT *currentTimeSlot;

__regargs TimeSlotT *TimelineForward(TimeSlotT *slot, WORD pos);

__regargs void LoadEffects(TimeSlotT *slot);
__regargs void UnLoadEffects(TimeSlotT *slot);
__regargs void RunEffects(TimeSlotT *slot);
__regargs void PrepareEffect(EffectT *effect);
void UpdateFrameCount();

#endif
