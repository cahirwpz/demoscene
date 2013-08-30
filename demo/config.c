#include "gfx/pixbuf.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/fileio.h"

#include "config.h"

static JsonNodeT *_config = NULL;

bool ReadConfig() {
  char *json;
  bool result = false;

  if ((json = ReadTextSimple(DemoConfigPath))) {
    if ((_config = JsonParse(json))) {
      if (!JsonQueryObject(_config, "resources", NULL)) {
        LOG("%s: No 'resources' section!", DemoConfigPath);
      } else {
        DemoConfig.showFrame = JsonQueryBoolean(_config, "flags/show-frame", false);
        DemoConfig.timeKeys = JsonQueryBoolean(_config, "flags/time-keys", false);

        result = true;
      }
    }

    MemUnref(json);
  }

  return result;
}

void LoadResources() {
  JsonNodeT *resources = JsonQueryObject(_config, "resources", NULL);

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

void KillConfig() {
  MemUnref(_config);
}

ADD2EXIT(KillConfig, 0);
