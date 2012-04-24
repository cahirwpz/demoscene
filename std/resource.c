#include <string.h>

#include "std/atompool.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"
#include "std/list.h"

typedef struct Resource {
  StrT name;
  PtrT ptr;
  bool dynamic;
} ResourceT;

static ListT *ResList;
static AtomPoolT *ResPool;

static void Relinquish(ResourceT *res) {
  if (res->dynamic) {
    LOG("Freeing resource '%s' at %p.", res->name, res->ptr);
    MemUnref(res->ptr);
  }

  MemUnref(res->name);
}

static CmpT FindByName(const ResourceT *res, const StrT name) {
  return strcmp(res->name, name);
}

void StartResourceManager() {
  ResList = NewList();
  ResPool = NewAtomPool(sizeof(ResourceT), 16);
}

void StopResourceManager() {
  DeleteListFull(ResList, (FreeFuncT)Relinquish);
  DeleteAtomPool(ResPool);
}

static void ResAddInternal(const StrT name, PtrT ptr, bool dynamic) {
  if (!ptr)
    PANIC("Missing content for resource '%s'.", name);

  {
    ResourceT *res = AtomNew(ResPool);

    res->name = StrDup(name);
    res->ptr = ptr;
    res->dynamic = dynamic;

    ListPushFront(ResList, res);
  }
}

void ResAddStatic(const StrT name, PtrT ptr) {
  ResAddInternal(name, ptr, FALSE);
}

void ResAdd(const StrT name, PtrT ptr) {
  ResAddInternal(name, ptr, TRUE);
}

PtrT ResGet(const StrT name) {
  ResourceT *res = ListSearch(ResList, (CompareFuncT)FindByName, (PtrT)name);

  if (!res)
    PANIC("Resource '%s' not found.", name);
  
  return res->ptr;
}
