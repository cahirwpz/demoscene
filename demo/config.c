#include "gfx/pixbuf.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/fileio.h"
#include "system/vblank.h"

#include "config.h"

JsonNodeT *DemoConfig = NULL;

bool ReadConfig() {
  char *json;
  bool result = false;

  if ((json = ReadTextSimple(DemoConfigPath))) {
    if ((DemoConfig = JsonParse(json)))
      result = true;

    MemUnref(json);
  }

  return result;
}

void LoadResources() {
  JsonNodeT *resources = JsonQueryObject(DemoConfig, "resources");

  void LoadFile(const char *key, JsonNodeT *value, void *data) {
    const char *type = JsonQueryString(value, "type");
    const char *path = JsonQueryString(value, "path");

    if (!strcmp(type, "image")) {
      ResAdd(key, NewPixBufFromFile(path));
    } else if (!strcmp(type, "palette")) {
      ResAdd(key, NewPaletteFromFile(path));
    } else {
      PANIC("Resource '%s' has wrong type '%s'!", key, type);
    }
  }

  JsonObjectForEach(resources, LoadFile, NULL);
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

  return tsi.ts;
}
