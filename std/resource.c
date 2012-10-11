#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/hashmap.h"
#include "std/resource.h"

static HashMapT *Resources = NULL;

void StartResourceManager() {
  if (!Resources)
    Resources = NewHashMap(50);
}

void StopResourceManager() {
  if (Resources) {
    MemUnref(Resources);
    Resources = NULL;
  }
}

void ResAddStatic(const StrT name, PtrT ptr) {
  ASSERT(ptr, "Missing content for resource '%s'.", name);

  HashMapAddLink(Resources, name, ptr);
}

void ResAdd(const StrT name, PtrT ptr) {
  ASSERT(ptr, "Missing content for resource '%s'.", name);

  HashMapAdd(Resources, name, ptr);
}

PtrT ResGet(const StrT name) {
  PtrT ptr = HashMapFind(Resources, name);

  if (!ptr)
    PANIC("Resource '%s' not found.", name);
  
  return ptr;
}
