#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "std/types.h"

typedef struct Frame {
  int number;
  int first, last;
  float beat;
} FrameT;

typedef void (*TimeFuncT)(FrameT *frame);

typedef struct TimeSlice TimeSliceT;

typedef union TimeSliceData {
  TimeFuncT func;
  TimeSliceT *slice;
} TimeSliceDataT;

struct TimeSlice {
  TimeSliceDataT data;
  char *name;
  int16_t type;
  int16_t start, end;
  int16_t last;
};

#define TIME_END -1
#define DO_ONCE(FUNC, WHEN, END) \
  { (TimeSliceDataT)(TimeFuncT)NULL, #FUNC, 0, WHEN, END, -1 }
#define EACH_FRAME(FUNC, START, END) \
  { (TimeSliceDataT)(TimeFuncT)NULL, #FUNC, 1, START, END, -1 }
#define EACH_NTH_FRAME(FUNC, START, END, STEP) \
  { (TimeSliceDataT)(TimeFuncT)NULL, #FUNC, STEP, START, END, -1 }
#define TIMESLICE(SLICE, START, END) \
  { (TimeSliceDataT)SLICE, #SLICE, -1, START, END, -1 }
#define THE_END \
  { (TimeSliceDataT)(TimeFuncT)NULL, NULL, 0, 0, 0, 0 }

#define CALLBACK(FUNC) static void FUNC(FrameT *frame)

typedef struct Callback {
  const char *name;
  TimeFuncT func;
} CallbackT;

extern CallbackT Callbacks[];

void DoTimeSlice(TimeSliceT *slice, FrameT *frame, int thisFrame);
float GetBeatLength();
TimeSliceT *LoadTimeline();

#endif
