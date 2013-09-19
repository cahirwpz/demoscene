#include "engine/mesh.h"
#include "gfx/pixbuf.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/fileio.h"
#include "system/vblank.h"

#include "config.h"
#include "envelope.h"

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

static EnvelopeT *ReadEnvelope(EnvTypeT type, JsonNodeT *value) {
  JsonNodeT *points = JsonQueryArray(value, "points");
  int dimensions = JsonQueryInteger(value, "dimensions");
  EnvelopeT *env = NewEnvelope(type, dimensions, points->u.array.num);

  void ReadValue(size_t index, JsonNodeT *node, void *data) {
    ASSERT(node->type == JSON_INTEGER || node->type == JSON_REAL,
           "Item '%ld' is not a number.", index);

    ((float *)data)[index] = (node->type == JSON_INTEGER) ?
      (float)node->u.integer : node->u.real;
  }

  void ReadPoint(size_t index, JsonNodeT *value, void *data) {
    JsonArrayForEach(value, ReadValue, (void *)&env->point[index]);
  }

  JsonArrayForEach(points, ReadPoint, NULL);

  return env;
}

void LoadResources() {
  JsonNodeT *resources = JsonQueryObject(DemoConfig, "resources");

  void LoadFile(const char *key, JsonNodeT *value, void *data) {
    const char *type = JsonQueryString(value, "type");

    if (!strcmp(type, "image")) {
      ResAdd(key, NewPixBufFromFile(JsonQueryString(value, "path")));
    } else if (!strcmp(type, "palette")) {
      ResAdd(key, NewPaletteFromFile(JsonQueryString(value, "path")));
    } else if (!strcmp(type, "mesh3d")) {
      ResAdd(key, NewMeshFromFile(JsonQueryString(value, "path")));
    } else if (!strcmp(type, "envelope:polyline")) {
      ResAdd(key, ReadEnvelope(ENV_POLYLINE, value));
    } else if (!strcmp(type, "envelope:smoothstep")) {
      ResAdd(key, ReadEnvelope(ENV_SMOOTHSTEP, value));
    } else {
      PANIC("Resource '%s' has wrong type '%s'!", key, type);
    }
  }

  JsonObjectForEach(resources, LoadFile, NULL);
}
