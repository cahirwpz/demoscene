#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <exec/types.h>

typedef APTR (*AllocFuncType)();
typedef VOID (*FreeFuncType)(APTR);
typedef BOOL (*InitFuncType)(APTR);

struct Resource {
  const char *Name;
  APTR Ptr;
  AllocFuncType AllocFunc;
  FreeFuncType FreeFunc;
  InitFuncType InitFunc;
};

BOOL ResourcesAlloc();
BOOL ResourcesInit();
void ResourcesFree();
APTR GetResource(const char *name);

#endif
