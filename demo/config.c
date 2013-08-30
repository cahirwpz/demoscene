#include "gfx/pixbuf.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "system/fileio.h"

#include "config.h"

JsonNodeT *ReadConfig() {
  JsonNodeT *node = NULL;
  char *json;

  if ((json = ReadTextSimple(ConfigPath))) {
    node = JsonParse(json);
    MemUnref(json);
  }

  return node;
}

void LoadResources(JsonNodeT *config) {
  JsonNodeT *resources = JsonQueryObject(config, "resources");

  void LoadFile(const char *key, JsonNodeT *value) {
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

  JsonObjectForEach(resources, LoadFile);
}
