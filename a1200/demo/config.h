#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "json/json.h"
#include "demo.h"

extern const char *DemoConfigPath;
extern JsonNodeT *DemoConfig;

bool ReadConfig();
void LoadResources();

#endif
