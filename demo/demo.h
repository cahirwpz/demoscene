#ifndef __DEMO_H__
#define __DEMO_H__

#include "std/types.h"

typedef void (*TimeFuncT)(int frameNumber);

typedef struct TimeSlice {
  TimeFuncT func;
  int start, end, step;
} TimeSliceT;

#define TIME_END ((1UL << 31) - 1)
#define DO_ONCE(FUNC, WHEN) { &FUNC, WHEN, 0, 0 }
#define EACH_FRAME(FUNC, START, END) { &FUNC, START, END, 1 }
#define EACH_NTH_FRAME(FUNC, START, END, STEP) { &FUNC, START, END, STEP }

extern TimeSliceT Timeline[];

bool SetupDemo();
void KillDemo();
void AddDemoResources();
bool HandleEvents(int frameNumber);

#endif
