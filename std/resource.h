#ifndef __STD_RESOURCE_H__
#define __STD_RESOURCE_H__

#include "std/types.h"

void StartResourceManager();
void StopResourceManager();

void AddRscSimple(const char *name, void *ptr, FreeFuncT freeFunc);
void AddRscStatic(const char *name, void *ptr);
void *GetResource(const char *name);

#endif
