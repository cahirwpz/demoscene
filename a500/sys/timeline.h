#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "effect.h"
#include "hardware.h"

typedef struct TimelineItem {
  WORD start, end;
  EffectT *effect;
  const char *name;
} TimelineItemT;

#ifndef FRAMES_PER_ROW
#error "FRAMES_PER_ROW: not defined!"
#endif

#define _TL(begin, end, effect) \
  { ((((begin) >> 8) & 0xff) * 64 + ((begin) & 0x3f)) * FRAMES_PER_ROW, \
    ((((end) >> 8) & 0xff) * 64 + ((end) & 0x3f)) * FRAMES_PER_ROW, \
    &(effect), #effect }

extern WORD frameCount;
extern WORD lastFrameCount;
extern WORD frameFromStart;
extern WORD frameTillEnd;

__regargs void LoadEffects(TimelineItemT *item, WORD n, WORD start);
__regargs void UnLoadEffects(TimelineItemT *item, WORD n);
__regargs void RunEffects(TimelineItemT *item, WORD n, WORD start);

__regargs void PrepareEffect(EffectT *effect);
void UpdateFrameCount();

#endif
