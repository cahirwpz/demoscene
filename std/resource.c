//#define NDEBUG

#include <string.h>

#include "std/atompool.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "std/slist.h"

typedef struct Resource {
  const char *name;
  void *ptr;
  AllocFuncT allocFunc;
  InitFuncT initFunc;
  FreeFuncT freeFunc;
} ResourceT;

static SListT *ResList;
static AtomPoolT *ResPool;

static bool Acquire(ResourceT *res) {
  if (res->allocFunc) {
    res->ptr = res->allocFunc();

    if (!res->ptr) {
      LOG("Failed to Allocate resource '%s'.", res->name);
      return FALSE;
    }

    LOG("Allocated resource '%s' at %p.", res->name, res->ptr);
  }

  return TRUE;
}

static bool Initialize(ResourceT *res) {
  if (res->initFunc) {
    LOG("Initiating resource '%s'.", res->name);

    if (!res->initFunc(res->ptr))
      return FALSE;
  }

  return TRUE;
}

static bool Relinquish(ResourceT *res) {
  bool needFree = TRUE;

  if (res->freeFunc)
    res->freeFunc(res->ptr);
  else if (res->allocFunc)
    DELETE(res->ptr);
  else
    needFree = FALSE;

  if (needFree)
    LOG("Freeing resource '%s' at %p.", res->name, res->ptr);

  return TRUE;
}

static bool FindByName(ResourceT *res, const char *name) {
  return strcmp(res->name, name);
}

void StartResourceManager() {
  ResList = NewSList();
  ResPool = NewAtomPool(sizeof(ResourceT), 16);
}

void StopResourceManager() {
  SL_ForEach(ResList, (IterFuncT)Relinquish, NULL);

  DeleteSList(ResList);
  DeleteAtomPool(ResPool);
}

bool ResourcesAlloc() {
  return SL_ForEach(ResList, (IterFuncT)Acquire, NULL) ? FALSE : TRUE;
}

bool ResourcesInit() {
  return SL_ForEach(ResList, (IterFuncT)Initialize, NULL) ? FALSE : TRUE;
}

static void AddResource(const char *name, void *ptr, 
                        AllocFuncT allocFunc, FreeFuncT freeFunc,
                        InitFuncT initFunc)
{
  ResourceT *res = AtomNew(ResPool);

  res->name = StrDup(name);
  res->ptr = ptr;
  res->allocFunc = allocFunc;
  res->initFunc = initFunc;
  res->freeFunc = freeFunc;

  SL_PushFront(ResList, res);
}

void AddRscSimple(const char *name, void *ptr, FreeFuncT freeFunc) {
  if (!ptr)
    PANIC("Missing content for resource '%s'.", name);

  AddResource(name, ptr, NULL, freeFunc, NULL);
}

void AddLazyRscSimple(const char *name,
                      AllocFuncT allocFunc, FreeFuncT freeFunc) {
  AddResource(name, NULL, allocFunc, freeFunc, NULL);
}

void AddLazyRscWithInit(const char *name,
                        AllocFuncT allocFunc, FreeFuncT freeFunc,
                        InitFuncT initFunc) {
  AddResource(name, NULL, allocFunc, freeFunc, initFunc);
}

void AddRscStatic(const char *name, void *ptr) {
  AddResource(name, ptr, NULL, NULL, NULL);
}

void *GetResource(const char *name) {
  ResourceT *res = SL_ForEach(ResList, (IterFuncT)FindByName, (void *)name);

  if (!res)
    PANIC("Resource '%s' not found.", name);

  LOG("Fetched resource '%s' located at %p.", res->name, res->ptr);
  
  return res->ptr;
}
