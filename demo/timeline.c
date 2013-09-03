#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/vblank.h"

#include "config.h"
#include "timeline.h"

static void DeleteTimeSlice(TimeSliceT *slice) {
  if (slice->type == TS_NODE)
    MemUnref(slice->u.slice);
  MemUnref(slice->name);
}

TYPEDECL(TimeSliceT, (FreeFuncT)DeleteTimeSlice);

void DoTimeSlice(TimeSliceT *slice, FrameT *frame, int thisFrame) {
  for (; slice->type; slice++) {
    bool invoke = false;

    if (slice->type == TS_NODE) {
      /* Recurse? */
      if ((slice->start <= thisFrame) && (thisFrame < slice->end))
        DoTimeSlice(slice->u.slice, frame, thisFrame);
    }
   
    if (slice->type == TS_LEAF) {
      switch (slice->step) {
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
              if (thisFrame - slice->last >= slice->step) {
                slice->last = thisFrame - ((thisFrame - slice->start) % slice->step);
                invoke = true;
              }
            }
          }
          break;
      }

      if (invoke) {
        CallbackT *callback = slice->u.actions.callbacks;
        SetterT *setter = slice->u.actions.setters;
        
        /* Invoke parameter setters. */
        if (setter) {
          for (; setter->ptr; setter++) {
            switch (setter->type) {
              case ST_INTEGER:
                *(int *)setter->ptr = setter->u.integer;
                break;
              case ST_REAL:
                *(float *)setter->ptr = setter->u.real;
                break;
              case ST_RESOURCE:
                *(void **)setter->ptr = R_(setter->u.resource);
                break;
            }
          }
        }

        /* Invoke callbacks. */
        if (callback) {
          frame->first = slice->start;
          frame->last = slice->end - 1;
          frame->number = thisFrame - slice->start;

          for (; callback->func; callback++)
            callback->func(frame);
        }

      }
    }
  }
}

/*
 * Makes all timing values absolute to the beginning of demo,
 * binds callback and parameter symbols.
 */
static void CompileTimeSlice(TimeSliceT *slice, int firstFrame, int lastFrame) {
  for (; slice->type; slice++) {
    slice->start = (slice->start < 0) ? lastFrame : (slice->start + firstFrame);
    slice->end = (slice->end < 0) ? lastFrame : (slice->end + firstFrame);

    if (slice->type == TS_NODE)
      CompileTimeSlice(slice->u.slice, slice->start, slice->end);

    if (slice->type == TS_LEAF) {
      {
        CallbackT *callback = slice->u.actions.callbacks;

        if (callback) {
          for (; callback->name; callback++) {
            SymbolT *symbol = CallbackSymbols;

            for (; symbol->name; symbol++) {
              if (!strcmp(symbol->name, callback->name)) {
                callback->func = (TimeFuncT)symbol->ptr;
                break;
              }
            }

            if (!callback->func)
              PANIC("Callback '%s' not found!", callback->name);
          }
        }
      }

      {
        SetterT *setter = slice->u.actions.setters;

        if (setter) {
          for (; setter->name; setter++) {
            SymbolT *symbol = ParameterSymbols;

            for (; symbol->name; symbol++) {
              if (!strcmp(symbol->name, setter->name)) {
                setter->ptr = symbol->ptr;
                break;
              }
            }

            if (!setter->ptr)
              PANIC("Parameter '%s' not found!", setter->name);
          }
        }
      }
    }
  }
}

void PrintTimeSlice(TimeSliceT *slice) {
  for (; slice->type; slice++) {
    char type[20];

    if (slice->type == TS_NODE)
      strcpy(type, "timeslice");
    else if (slice->step == 0)
      strcpy(type, "once");
    else if (slice->step == 1)
      strcpy(type, "each frame");
    else
      snprintf(type, sizeof(type), "every %d frames", slice->step);

    if (slice->type == TS_NODE) {
      printf("[%5d]: (start) %s\r\n", slice->start, slice->name);
      PrintTimeSlice(slice->u.slice);
      printf("[%5d]: (end) %s\r\n", slice->end, slice->name);
    } else {
      CallbackT *callback = slice->u.actions.callbacks;
      SetterT *setter = slice->u.actions.setters;

      printf("[%5d - %5d]: (%s) %s\r\n",
          slice->start, slice->end, type, slice->name);

      if (setter) {
        for (; setter->name; setter++) {
          printf("  set: '%s' {%p} = ", setter->name, setter->ptr);

          switch (setter->type) {
            case ST_INTEGER:
              printf("(int) %d", setter->u.integer);
              break;
            case ST_REAL:
              printf("(float) %f", setter->u.real);
              break;
            case ST_RESOURCE:
              printf("(resource) '%s'", setter->u.resource);
              break;
            default:
              break;
          }

          puts("\r");
        }
      }

      if (callback) {
        for (; callback->name; callback++)
          printf("  call: '%s' {%p} \r\n", callback->name, callback->func);
      }
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

static CallbackT *BuildCallbacks(JsonNodeT *value, TimeSliceInfoT *tsi) {
  CallbackT *callbacks = NewTable(CallbackT, 2);
  callbacks[0].name = StrDup(JsonQueryString(value, "call"));
  return callbacks;
}

static void JsonReadSetter(const char *key, JsonNodeT *value, void *data) {
  SetterT **setter_ptr = (SetterT **)data;
  SetterT *setter = (*setter_ptr)++;

  setter->name = StrDup(key);

  if (JsonQuery(value, "resource")) {
    setter->type = ST_RESOURCE;
    setter->u.resource = StrDup(JsonQueryString(value, "resource"));
  } else if (JsonQuery(value, "int")) {
    setter->type = ST_INTEGER;
    setter->u.integer = JsonQueryInteger(value, "int");
  } else if (JsonQuery(value, "float")) {
    setter->type = ST_REAL;
    setter->u.real = JsonQueryNumber(value, "float");
  } else {
    PANIC("Unknown setter type.");
  }
}

static SetterT *BuildSetters(JsonNodeT *value, TimeSliceInfoT *tsi) {
  SetterT *setters = NULL;
  JsonNodeT *set;

  if ((set = JsonQuery(value, "set"))) {
    int no_setters = set->u.object.num;
    SetterT *setter;

    setter = setters = NewTable(SetterT, no_setters + 1);

    JsonObjectForEach(set, JsonReadSetter, &setter);
  }

  return setters;
}

typedef enum { UNKNOWN = 0, TIMESLICE, ONCE, EACH_FRAME } TSTypeT;

static void BuildTimeSlice(JsonNodeT *value, void *data) {
  const char *type = JsonQueryString(value, "type");
  TimeSliceInfoT *tsi = (TimeSliceInfoT *)data;
  TimeSliceT *ts = &tsi->ts[tsi->index++];
  TSTypeT tsType = UNKNOWN;

  if (!strcmp(type, "timeslice"))
    tsType = TIMESLICE;
  else if (!strcmp(type, "once"))
    tsType = ONCE;
  else if (!strcmp(type, "each-frame"))
    tsType = EACH_FRAME;
  else
    PANIC("Unknown time slice type: '%s'.", type);

  if (JsonQuery(value, "name"))
    ts->name  = StrDup(JsonQueryString(value, "name"));
  ts->start = (int)JsonReadTime(value, "range/0", tsi);
  ts->end   = (int)JsonReadTime(value, "range/1", tsi);
  ts->step  = 0;
  ts->last  = -1;

  if (tsType == TIMESLICE) {
    JsonNodeT *parts = JsonQueryArray(value, "parts");

    TimeSliceInfoT newTsi;

    newTsi.ts = NewTableOfType(TimeSliceT, parts->u.array.num + 1);
    newTsi.index = 0;
    newTsi.bpm = tsi->bpm;
    newTsi.unit = JsonReadTime(value, "unit", tsi);

    ts->type = TS_NODE;
    ts->u.slice = newTsi.ts;

    JsonArrayForEach(parts, BuildTimeSlice, &newTsi);
  } else {
    ts->type = TS_LEAF;

    if (tsType == EACH_FRAME) {
      JsonNodeT *range = JsonQueryArray(value, "range");
      int step = 1;

      if (range->u.array.num > 2)
        step = JsonReadTime(value, "range/2", tsi);

      ASSERT(step > 0, "Step must be positive.");

      ts->step = step;
    }

    ts->u.actions.callbacks = BuildCallbacks(value, tsi);
    ts->u.actions.setters = BuildSetters(value, tsi);
  }
}

float GetBeatLength() {
  return (60.0f * FRAMERATE) / JsonQueryNumber(DemoConfig, "music/bpm");
}

TimeSliceT *LoadTimeline() {
  JsonNodeT *timeline = JsonQueryArray(DemoConfig, "timeline");
  TimeSliceInfoT tsi;

  tsi.ts    = NewTableOfType(TimeSliceT, timeline->u.array.num + 1);
  tsi.index = 0;
  tsi.bpm   = JsonQueryNumber(DemoConfig, "music/bpm");
  tsi.unit  = 1.0f; /* 1 frame */

  JsonArrayForEach(timeline, BuildTimeSlice, &tsi);

  CompileTimeSlice(tsi.ts, 0, 5407);

  return tsi.ts;
}
