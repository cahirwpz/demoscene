#ifndef __STD_RESOURCE_H__
#define __STD_RESOURCE_H__

#include "std/types.h"

typedef struct Resource {
  const char *Name;
  void *Ptr;
  AllocFuncT AllocFunc;
  FreeFuncT FreeFunc;
  InitFuncT InitFunc;
} ResourceT;

void StartResourceManager();
void StopResourceManager();

bool ResourcesAlloc();
bool ResourcesInit();

void *GetResource(const char *name);

#endif
