#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "json/json.h"

typedef struct DemoConfig {
  bool showFrame; /* show the number of current frame */
  bool timeKeys;  /* enable rewinding and fast-forward keys */
} DemoConfigT;

extern const char *DemoConfigPath;
extern DemoConfigT DemoConfig;

bool ReadConfig();
void LoadResources();

#endif
