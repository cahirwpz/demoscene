#ifndef __DEMO_H__
#define __DEMO_H__

#include "std/types.h"

typedef void (*TimeFuncT)(int frameNumber);

typedef struct TimeSlice {
  TimeFuncT func;
  ssize_t start, end, step;
} TimeSliceT;

#define TIME_END -1
#define EACH_FRAME(FUNC, START, END) { &FUNC, START, END, -1 }
#define EACH_NTH_FRAME(FUNC, START, END, STEP) { &FUNC, START, END, STEP }

extern TimeSliceT Timeline[];

bool SetupDemo();
void KillDemo();
void AddDemoResources();
bool HandleEvents(int frameNumber);

#endif
