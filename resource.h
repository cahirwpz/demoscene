#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <exec/types.h>

typedef APTR (*AllocFuncType)();
typedef VOID (*FreeFuncType)(APTR);
typedef BOOL (*InitFuncType)(APTR);

struct ResourceDesc {
  const char *Name;
  AllocFuncType AllocFunc;
  FreeFuncType FreeFunc;
  InitFuncType InitFunc;
};

struct ResourceNode {
  struct MinNode Node;
  struct ResourceDesc *Desc;
  APTR Ptr;
};

BOOL ResourcesAlloc();
BOOL ResourcesInit();
void ResourcesFree();
APTR GetResource(const char *name);

#endif
