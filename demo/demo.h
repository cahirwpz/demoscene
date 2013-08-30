#ifndef __DEMO_H__
#define __DEMO_H__

#include "std/types.h"

typedef struct Frame {
  int16_t first, last;
  int16_t number;
} FrameT;

typedef void (*TimeFuncT)(FrameT *frame);

typedef struct TimeSlice TimeSliceT;

typedef union TimeSliceData {
  TimeFuncT func;
  TimeSliceT *slice;
} TimeSliceDataT;

struct TimeSlice {
  TimeSliceDataT data;
  int16_t type;
  int16_t start, end;
  int16_t last;
};

#define TIME_END ((1 << 15) - 1)
#define DO_ONCE(FUNC, WHEN, END) { (TimeSliceDataT)&FUNC, 0, WHEN, END, -1 }
#define EACH_FRAME(FUNC, START, END) { (TimeSliceDataT)&FUNC, 1, START, END, -1 }
#define EACH_NTH_FRAME(FUNC, START, END, STEP) { (TimeSliceDataT)&FUNC, STEP, START, END, -1 }
#define TIMESLICE(SLICE, START, END) { (TimeSliceDataT)SLICE, -1, START, END, -1 }
#define THE_END { (TimeSliceDataT)(TimeFuncT)NULL, 0, 0, 0, 0 }

extern TimeSliceT TheDemo[];

extern bool ExitDemo;

bool LoadConfig();
bool LoadDemo();
void SetupResources();
void BeginDemo();
void KillDemo();
void HandleEvents(int frameNumber);

#endif
