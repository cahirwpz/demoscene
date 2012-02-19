#ifndef __STD_RESOURCE_H__
#define __STD_RESOURCE_H__

#include "std/types.h"

void StartResourceManager();
void StopResourceManager();

bool ResourcesAlloc();
bool ResourcesInit();

void AddLazyRscSimple(const char *name,
                      AllocFuncT allocFunc, FreeFuncT freeFunc);
void AddLazyRscWithInit(const char *name,
                        AllocFuncT allocFunc, FreeFuncT freeFunc,
                        InitFuncT initFunc);
void AddRscStatic(const char *name, void *ptr);
void *GetResource(const char *name);

#endif
