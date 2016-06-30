#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "config.h"
#include "effect.h"
#include "hardware.h"

typedef struct TimeSlot {
  WORD start, end, phase;
  EffectT *effect;
} TimeSlotT;

#ifndef FRAMES_PER_ROW
#error "FRAMES_PER_ROW: not defined!"
#endif

#define PT_FRAME(pos) \
  (((((pos) >> 8) & 0xff) * 64 + ((pos) & 0x3f)) * FRAMES_PER_ROW)

#define _TS(begin, end, phase, effect) \
  { PT_FRAME(begin), PT_FRAME(end), phase, &(effect) }

#define _TS_END() {0, 0, 0, NULL}

extern WORD frameCount;
extern WORD lastFrameCount;
extern WORD frameFromStart;
extern WORD frameTillEnd;
extern TimeSlotT *currentTimeSlot;

__regargs TimeSlotT *TimelineForward(TimeSlotT *slot, WORD pos);

__regargs void LoadEffects(TimeSlotT *slot, WORD phase);
__regargs void UnLoadEffects(TimeSlotT *slot);
__regargs void RunEffects(TimeSlotT *slot);
__regargs void PrepareEffect(EffectT *effect);
void UpdateFrameCount();

#endif
