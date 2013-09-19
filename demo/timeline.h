#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "std/types.h"

extern float DemoBeat;
extern int DemoEndFrame;

/*
 * Callback type description.
 */

typedef struct Frame {
  int number;
  int first, last;
} FrameT;

typedef void (*TimeFuncT)(FrameT *frame);

static inline float FrameTime(FrameT *frame) {
  return (float)frame->number / (frame->last - frame->first);
}

/*
 * Symbols.
 */

typedef struct Symbol {
  char *name;
  void *ptr;
} SymbolT;

#define CALLBACK(SYMBOL) static void SYMBOL(FrameT *frame)
#define PARAMETER(TYPE, SYMBOL, VALUE) static TYPE SYMBOL = VALUE
#define ARRAY(TYPE, SIZE, SYMBOL, ...) static TYPE SYMBOL[SIZE] = { __VA_ARGS__ }

extern SymbolT CallbackSymbols[];
extern SymbolT ParameterSymbols[];

/*
 * Time slice.
 */

typedef struct Callback {
  char *name;
  TimeFuncT func;
} CallbackT;

typedef enum { ST_INTEGER, ST_REAL, ST_STRING, ST_RESOURCE, ST_ENVELOPE } SetterTypeT;

typedef struct Setter {
  SetterTypeT type;
  char *name;
  void *ptr;
  union {
    char *resource;
    char *string;
    int integer;
    float real;
  } u;
} SetterT;

typedef enum { TS_LEAF = 1, TS_NODE } TimeSliceTypeT;

typedef struct TimeSlice TimeSliceT;

struct TimeSlice {
  TimeSliceTypeT type;

  int16_t start, end, step;
  int16_t last;

  union {
    TimeSliceT *slice;
    struct {
      CallbackT *callbacks;
      SetterT *setters;
    } actions;
  } u;
};

void DoTimeSlice(TimeSliceT *slice, int thisFrame);
void ResetTimeSlice(TimeSliceT *slice);
void PrintTimeSlice(TimeSliceT *slice);
TimeSliceT *LoadTimeline();

#endif
