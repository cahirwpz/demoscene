#ifndef __DEMO_H__
#define __DEMO_H__

#include "std/types.h"

extern bool ExitDemo;

bool LoadConfig();
bool LoadDemo();
void SetupResources();
void BeginDemo();
void KillDemo();
void HandleEvents(int frameNumber);

#endif
