//#define NDEBUG

#include <string.h>

#include "std/atompool.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "std/slist.h"

extern ResourceT ResourceList[];

static SListT *ResList;
static AtomPoolT *ResPool;

static bool Acquire(ResourceT *res) {
  if (res->AllocFunc) {
    res->Ptr = res->AllocFunc();

    if (!res->Ptr) {
      LOG("Failed to Allocate resource '%s'.", res->Name);
      return FALSE;
    }
  }

  LOG("Allocated resource '%s' at %p.", res->Name, res->Ptr);
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
  LOG("Freeing resource '%s' at %p.", res->Name, res->Ptr);

  if (res->FreeFunc)
    res->FreeFunc(res->Ptr);
  else if (res->AllocFunc)
    DELETE(res->Ptr);

  return TRUE;
}

static bool FindByName(ResourceT *res, const char *name) {
  return strcmp(res->Name, name);
}

void StartResourceManager() {
  ResourceT *res;

  ResList = NewSList();
  ResPool = NewAtomPool(sizeof(ResourceT), 16);

  for (res = ResourceList; res->Name; res++) {
    ResourceT *newRes = AtomNew(ResPool);

    memcpy(newRes, res, sizeof(ResourceT)); 
  }
}

void StopResourceManager() {
  SL_ForEach(ResList, (IterFuncT)Relinquish, NULL);

  DeleteSList(ResList);
  DeleteAtomPool(ResPool);
}

bool ResourcesAlloc() {
  return SL_ForEach(ResList, (IterFuncT)Acquire, NULL) ? TRUE : FALSE;
}

bool ResourcesInit() {
  return SL_ForEach(ResList, (IterFuncT)Initialize, NULL) ? TRUE : FALSE;
}

void *GetResource(const char *name) {
  ResourceT *res = SL_ForEach(ResList, (IterFuncT)FindByName, (void *)name);

  if (res) {
    LOG("Fetched resource '%s' at %p.", res->Name, res->Ptr);
  } else {
    LOG("Resource '%s' not found.", name);
  }
  
  return res;
}
