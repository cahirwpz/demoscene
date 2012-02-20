//#define NDEBUG

#include <string.h>

#include "std/atompool.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "std/slist.h"

typedef struct Resource {
  const char *Name;
  void *Ptr;
  AllocFuncT AllocFunc;
  FreeFuncT FreeFunc;
  InitFuncT InitFunc;
} ResourceT;

static SListT *ResList;
static AtomPoolT *ResPool;

static bool Acquire(ResourceT *res) {
  if (res->AllocFunc) {
    res->Ptr = res->AllocFunc();

    if (!res->Ptr) {
      LOG("Failed to Allocate resource '%s'.", res->Name);
      return FALSE;
    }

    LOG("Allocated resource '%s' at %p.", res->Name, res->Ptr);
  }

  return TRUE;
}

static bool Initialize(ResourceT *res) {
  if (res->InitFunc) {
    LOG("Initiating resource '%s'.", res->Name);

    if (!res->InitFunc(res->Ptr))
      return FALSE;
  }

  return TRUE;
}

static bool Relinquish(ResourceT *res) {
  bool needFree = TRUE;

  if (res->FreeFunc)
    res->FreeFunc(res->Ptr);
  else if (res->AllocFunc)
    DELETE(res->Ptr);
  else
    needFree = FALSE;

  if (needFree)
    LOG("Freeing resource '%s' at %p.", res->Name, res->Ptr);

  return TRUE;
}

static bool FindByName(ResourceT *res, const char *name) {
  return strcmp(res->Name, name);
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

  res->Name = StrDup(name);
  res->Ptr = ptr;
  res->AllocFunc = allocFunc;
  res->InitFunc = initFunc;
  res->FreeFunc = freeFunc;

  SL_PushFront(ResList, res);
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

  LOG("Fetched resource '%s' located at %p.", res->Name, res->Ptr);
  
  return res->Ptr;
}
