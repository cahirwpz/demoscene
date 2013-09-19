#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/vblank.h"

#include "config.h"
#include "envelope.h"
#include "timeline.h"

float DemoBeat = 0.0f;
int DemoEndFrame = 0;

static void DeleteTimeSlice(TimeSliceT *slice) {
  if (slice->type == TS_NODE)
    MemUnref(slice->u.slice);
}

TYPEDECL(TimeSliceT, (FreeFuncT)DeleteTimeSlice);

void DoTimeSlice(TimeSliceT *slice, int thisFrame) {
  for (; slice->type; slice++) {
    bool invoke = false;

    if (slice->type == TS_NODE) {
      /* Recurse? */
      if ((slice->start <= thisFrame) && (thisFrame < slice->end))
        DoTimeSlice(slice->u.slice, thisFrame);
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
              case ST_STRING:
                *(char **)setter->ptr = setter->u.string;
                break;
              case ST_RESOURCE:
                *(void **)setter->ptr = R_(setter->u.resource);
                break;
              case ST_ENVELOPE:
                {
                  EnvelopeT *env = R_(setter->u.resource);
                  float time = (float)(thisFrame - slice->start) / DemoBeat;
                  EnvelopeEvaluate(env, time, (float *)setter->ptr);
                }
                break;
            }
          }
        }

        /* Invoke callbacks. */
        if (callback) {
          FrameT frame = {
            .first = slice->start,
            .last = slice->end - 1,
            .number = thisFrame - slice->start
          };

          for (; callback->func; callback++)
            callback->func(&frame);
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

void ResetTimeSlice(TimeSliceT *slice) {
  for (; slice->type; slice++) {

    if (slice->type == TS_NODE)
      ResetTimeSlice(slice->u.slice);
    else
      slice->last = -1;
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
      printf("[%5d]: (start)\r\n", slice->start);
      PrintTimeSlice(slice->u.slice);
      printf("[%5d]: (end)\r\n", slice->end);
    } else {
      CallbackT *callback = slice->u.actions.callbacks;
      SetterT *setter = slice->u.actions.setters;

      printf("[%5d - %5d]: (%s)\r\n", slice->start, slice->end, type);

      if (setter) {
        for (; setter->name; setter++) {
          printf("  set: '%s' = ", setter->name);

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
          printf("  call: '%s' \r\n", callback->name);
      }
    }
  }
}

typedef struct {
  TimeSliceT *ts;
  int index;
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
      time = unitBase * DemoBeat;
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
  CallbackT *callbacks = NULL;
  JsonNodeT *call;

  if ((call = JsonQuery(value, "call"))) {
    int no_callbacks = call->u.array.num;

    void ReadCall(size_t i, JsonNodeT *value, void *data) {
      callbacks[i].name = StrDup(value->u.string);
    }

    callbacks = NewTable(CallbackT, no_callbacks + 1);

    JsonArrayForEach(call, ReadCall, NULL);
  }

  return callbacks;
}

static void JsonReadSetter(const char *key, JsonNodeT *value, void *data) {
  SetterT **setter_ptr = (SetterT **)data;
  SetterT *setter = (*setter_ptr)++;

  setter->name = StrDup(key);

  switch (value->type) {
    case JSON_OBJECT:
      if (JsonQuery(value, "resource")) {
        setter->type = ST_RESOURCE;
        setter->u.resource = StrDup(JsonQueryString(value, "resource"));
      } else if (JsonQuery(value, "envelope")) {
        setter->type = ST_ENVELOPE;
        setter->u.resource = StrDup(JsonQueryString(value, "envelope"));
      } else {
        PANIC("Unknown setter type: '%s'.", value->u.object.item[0].key);
      }
      break;

    case JSON_INTEGER:
      setter->type = ST_INTEGER;
      setter->u.integer = value->u.integer;
      break;

    case JSON_REAL:
      setter->type = ST_REAL;
      setter->u.real = value->u.real;
      break;

    case JSON_STRING:
      setter->type = ST_STRING;
      setter->u.string = value->u.string;
      break;

    default:
      PANIC("Unhandled JSON type: %d.", value->type);
      break;
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

static void BuildTimeSlice(size_t index, JsonNodeT *value, void *data) {
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

  ts->start = (int)JsonReadTime(value, "range/0", tsi);
  ts->end   = (int)JsonReadTime(value, "range/1", tsi);
  ts->step  = 0;
  ts->last  = -1;

  if (tsType == TIMESLICE) {
    JsonNodeT *parts = JsonQueryArray(value, "parts");

    TimeSliceInfoT newTsi;

    newTsi.ts = NewTableOfType(TimeSliceT, parts->u.array.num + 1);
    newTsi.index = 0;
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

TimeSliceT *LoadTimeline() {
  JsonNodeT *timeline = JsonQueryArray(DemoConfig, "timeline");
  TimeSliceInfoT tsi;

  DemoBeat = (60.0f * FRAMERATE) / JsonQueryNumber(DemoConfig, "music/bpm");
  DemoEndFrame = JsonQueryInteger(DemoConfig, "end-frame");

  tsi.ts    = NewTableOfType(TimeSliceT, timeline->u.array.num + 1);
  tsi.index = 0;
  tsi.unit  = 1.0f; /* 1 frame */

  JsonArrayForEach(timeline, BuildTimeSlice, &tsi);

  CompileTimeSlice(tsi.ts, 0, DemoEndFrame);

  return tsi.ts;
}
