#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/list.h"
#include "std/resource.h"

typedef struct Resource {
  StrT name;
  PtrT ptr;
  bool dynamic;
} ResourceT;

static void DeleteResource(ResourceT *res) {
  if (res->dynamic) {
    LOG("Freeing resource '%s' at %p.", res->name, res->ptr);
    MemUnref(res->ptr);
  }
  MemUnref(res->name);
}

TYPEDECL(ResourceT, (FreeFuncT)DeleteResource);

static ResourceT *NewResource(const StrT name, PtrT ptr, bool dynamic) {
  ResourceT *res;

  if (!ptr)
    PANIC("Missing content for resource '%s'.", name);

  res = NewInstance(ResourceT);
  res->name = StrDup(name);
  res->ptr = ptr;
  res->dynamic = dynamic;
  return res;
}

static ListT *ResList;

void StartResourceManager() {
  ResList = NewList();
}

void StopResourceManager() {
  MemUnref(ResList);
}

void ResAddStatic(const StrT name, PtrT ptr) {
  ListPushFront(ResList, NewResource(name, ptr, FALSE));
}

void ResAdd(const StrT name, PtrT ptr) {
  ListPushFront(ResList, NewResource(name, ptr, TRUE));
}

static CmpT FindByName(const ResourceT *res, const StrT name) {
  return strcmp(res->name, name);
}

PtrT ResGet(const StrT name) {
  ResourceT *res = ListSearch(ResList, (CompareFuncT)FindByName, (PtrT)name);

  if (!res)
    PANIC("Resource '%s' not found.", name);
  
  return res->ptr;
}
