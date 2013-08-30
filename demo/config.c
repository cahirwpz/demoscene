#include "gfx/pixbuf.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/fileio.h"

#include "config.h"

JsonNodeT *DemoConfig = NULL;

bool ReadConfig() {
  char *json;
  bool result = false;

  if ((json = ReadTextSimple(DemoConfigPath))) {
    if ((DemoConfig = JsonParse(json))) {
      if (!JsonQueryObject(DemoConfig, "resources", NULL)) {
        LOG("%s: No 'resources' section!", DemoConfigPath);
      } else {
        result = true;
      }
    }

    MemUnref(json);
  }

  return result;
}

void LoadResources() {
  JsonNodeT *resources = JsonQueryObject(DemoConfig, "resources", NULL);

  void LoadFile(const char *key, JsonNodeT *value) {
    const char *type = JsonQueryString(value, "type", NULL);
    const char *path = JsonQueryString(value, "path", NULL);

    if (!strcmp(type, "image")) {
      ResAdd(key, NewPixBufFromFile(path));
    } else if (!strcmp(type, "palette")) {
      ResAdd(key, NewPaletteFromFile(path));
    } else {
      PANIC("Resource '%s' has wrong type '%s'!", key, type);
    }
  }

  JsonObjectForEach(resources, LoadFile);
}
