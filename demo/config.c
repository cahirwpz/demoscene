#include "engine/mesh.h"
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
    } else if (!strcmp(type, "mesh3d")) {
      ResAdd(key, NewMeshFromFile(path));
    } else {
      PANIC("Resource '%s' has wrong type '%s'!", key, type);
    }
  }

  JsonObjectForEach(resources, LoadFile, NULL);
}
