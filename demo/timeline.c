#include "std/debug.h"
#include "std/memory.h"
#include "system/vblank.h"

#include "config.h"
#include "timeline.h"

void DoTimeSlice(TimeSliceT *slice, FrameT *frame, int thisFrame) {
  for (; slice->data.func; slice++) {
    bool invoke = false;

    if (slice->type < 0) {
      /* Recurse? */
      if ((slice->start <= thisFrame) && (thisFrame < slice->end))
        DoTimeSlice(slice->data.slice, frame, thisFrame);
    } else {
      int step = slice->type;

      switch (step) {
        case 0:
          /* Do it only once. */
          invoke = (slice->start <= thisFrame) && (slice->last < 0);
          if (invoke)
            slice->last = thisFrame;
          break;

        case 1:
          /* Do it every frame. */
          invoke = (slice->start <= thisFrame) && (thisFrame < slice->end);
          break;

        default:
          /* Do it every n-th frame. */
          if ((slice->start <= thisFrame) && (thisFrame < slice->end)) {
            if (slice->last == -1) {
              slice->last = slice->start;
              invoke = true;
            } else {
              if (thisFrame - slice->last >= step) {
                slice->last = thisFrame - ((thisFrame - slice->start) % step);
                invoke = true;
              }
            }
          }
          break;
      }

      if (invoke) {
        frame->first = slice->start;
        frame->last = slice->end - 1;
        frame->number = thisFrame - slice->start;

        slice->data.func(frame);
      }
    }
  }
}

/*
 * Makes all timing values absolute to the beginning of demo.
 */
static void CompileTimeSlice(TimeSliceT *slice, int firstFrame, int lastFrame) {
  for (; slice->name; slice++) {
    if (slice->start < 0)
      slice->start = lastFrame;
    else
      slice->start += firstFrame;

    if (slice->end < 0)
      slice->end = lastFrame;
    else
      slice->end += firstFrame;

    if (slice->type < 0)
      CompileTimeSlice(slice->data.slice, slice->start, slice->end);
    else {
      CallbackT *callback;

      for (callback = Callbacks; callback->name; callback++) {
        if (!strcmp(callback->name, slice->name)) {
          slice->data.func = callback->func;
          break;
        }
      }

      if (!slice->data.func)
        PANIC("Callback '%s' not found!", slice->name);
    }
  }
}

static void PrintTimeSlice(TimeSliceT *slice) {
  for (; slice->name != NULL; slice++) {
    char type[20];

    if (slice->type == -1)
      strcpy(type, "timeslice");
    else if (slice->type == 0)
      strcpy(type, "once");
    else if (slice->type == 1)
      strcpy(type, "each frame");
    else
      snprintf(type, sizeof(type), "every %d frames", slice->type);

    if (slice->type == -1) {
      printf("[%5d]: (start) %s\n", slice->start, slice->name);
      PrintTimeSlice(slice->data.slice);
      printf("[%5d]: (end) %s\n", slice->end, slice->name);
    } else if (slice->type == 0) {
      printf("[%5d - %5d]: (%s) {%p} %s\n",
             slice->start, slice->end, type, slice->data.func, slice->name);
    } else {
      printf("[%5d - %5d]: (%s) {%p} %s\n",
             slice->start, slice->end, type, slice->data.func, slice->name);
    }
  }
}

typedef struct {
  TimeSliceT *ts;
  int index;
  float bpm;
  float unit;
} TimeSliceInfoT;

static float JsonReadTime(JsonNodeT *value, const char *path,
                          TimeSliceInfoT *tsi)
{
  float time = 0.0f;

  value = JsonQuerySafe(value, path);

  if (value->type == JSON_ARRAY) {
    float unitBase = JsonQueryNumber(value, "0");
    const char *unitType = JsonQueryString(value, "1");

    if (!strcmp(unitType, "beat"))
      time = unitBase * (60.0f * FRAMERATE) / tsi->bpm;
    else if (!strcmp(unitType, "second"))
      time = unitBase * FRAMERATE;
    else if (!strcmp(unitType, "frame"))
      time = unitBase;
    else
      PANIC("Unit type must be one of: 'beat', 'second', 'frame'.");
  } else if (value->type == JSON_INTEGER) {
    int span = value->u.integer;
    time = (span < 0) ? -1 : (tsi->unit * span);
  } else if (value->type == JSON_REAL) {
    float span = value->u.real;
    time = (span < 0.0f) ? -1.0f : (tsi->unit * span);
  } else {
    PANIC("Not a valid time description.");
  }

  return time;
}

static void BuildTimeSlice(JsonNodeT *value, void *data) {
  const char *type = JsonQueryString(value, "type");
  TimeSliceInfoT *tsi = (TimeSliceInfoT *)data;
  TimeSliceT *ts = &tsi->ts[tsi->index++];

  if (!strcmp(type, "timeslice")) {
    JsonNodeT *parts = JsonQueryArray(value, "parts");

    TimeSliceInfoT newTsi;

    newTsi.ts = NewTable(TimeSliceT, parts->u.array.num + 1);
    newTsi.index = 0;
    newTsi.bpm = tsi->bpm;
    newTsi.unit = JsonReadTime(value, "unit", tsi);

    JsonArrayForEach(parts, BuildTimeSlice, &newTsi);

    ts->data.slice = newTsi.ts;
    ts->name  = StrDup(JsonQueryString(value, "name"));
    ts->type  = -1;
    ts->start = (int)JsonReadTime(value, "range/0", tsi);
    ts->end   = (int)JsonReadTime(value, "range/1", tsi);
    ts->last  = -1;
  } else if (!strcmp(type, "once")) {
    ts->name  = StrDup(JsonQueryString(value, "call"));
    ts->type  = 0;
    ts->start = (int)JsonReadTime(value, "range/0", tsi);
    ts->end   = (int)JsonReadTime(value, "range/1", tsi);
    ts->last  = -1;
  } else if (!strcmp(type, "each-frame")) {
    JsonNodeT *range = JsonQueryArray(value, "range");
    int step = 1;
      
    if (range->u.array.num > 2)
      step = JsonReadTime(value, "range/2", tsi);

    ASSERT(step > 0, "Step must be positive.");

    ts->name  = StrDup(JsonQueryString(value, "call"));
    ts->type  = step;
    ts->start = (int)JsonReadTime(value, "range/0", tsi);
    ts->end   = (int)JsonReadTime(value, "range/1", tsi);
    ts->last  = -1;
  }
}

float GetBeatLength() {
  return (60.0f * FRAMERATE) / JsonQueryNumber(DemoConfig, "music/bpm");
}

TimeSliceT *LoadTimeline() {
  JsonNodeT *timeline = JsonQueryArray(DemoConfig, "timeline");
  TimeSliceInfoT tsi;

  tsi.ts    = NewTable(TimeSliceT, timeline->u.array.num + 1);
  tsi.index = 0;
  tsi.bpm   = JsonQueryNumber(DemoConfig, "music/bpm");
  tsi.unit  = 1.0f; /* 1 frame */

  JsonArrayForEach(timeline, BuildTimeSlice, &tsi);

  CompileTimeSlice(tsi.ts, 0, 5407);

  return tsi.ts;
}
