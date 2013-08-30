#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "json/json.h"

extern const char *ConfigPath;

JsonNodeT *ReadConfig();
void LoadResources(JsonNodeT *config);

#endif
