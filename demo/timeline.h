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

/*
 * Symbols.
 */

typedef struct Symbol {
  char *name;
  void *ptr;
} SymbolT;

#define CALLBACK(SYMBOL) static void SYMBOL(FrameT *frame)
#define PARAMETER(TYPE, SYMBOL, VALUE) static TYPE SYMBOL = VALUE

extern SymbolT CallbackSymbols[];
extern SymbolT ParameterSymbols[];

/*
 * Time slice.
 */

typedef struct Callback {
  char *name;
  TimeFuncT func;
} CallbackT;

typedef enum { ST_INTEGER, ST_REAL, ST_RESOURCE } SetterTypeT;

typedef struct Setter {
  SetterTypeT type;
  char *name;
  void *ptr;
  union {
    char *resource;
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
void PrintTimeSlice(TimeSliceT *slice);
TimeSliceT *LoadTimeline();

#endif
