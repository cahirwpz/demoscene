#include <string.h>

#include "debug.h"
#include "memory.h"
#include "resource.h"

extern ResourceT ResourceList[];

BOOL ResourcesAlloc() {
  ResourceT *res;

  for (res = ResourceList; res->Name; res++) {
    if (!(res->Ptr = res->AllocFunc()))
      return FALSE;

    LOG("Allocated resource '%s' at $%lx.\n", res->Name, res->Ptr);
  }

  return TRUE;
}

BOOL ResourcesInit() {
  ResourceT *res;

  for (res = ResourceList; res->Name; res++) {
    if (!res->InitFunc)
      continue;

    LOG("Initiating resource '%s'.\n", res->Name);

    if (!res->InitFunc(res->Ptr))
      return FALSE;
  }

  return TRUE;
}

void ResourcesFree() {
  ResourceT *res;

  for (res = ResourceList; res->Name; res++) {
    LOG("Freeing resource '%s' at $%lx.\n", res->Name, res->Ptr);

    if (res->FreeFunc)
      res->FreeFunc(res->Ptr);
    else if (res->AllocFunc)
      DELETE(res->Ptr);
  }
}

APTR GetResource(const char *name) {
  struct Resource *res;

  for (res = ResourceList; res->Name; res++) {
    if (strcmp(res->Name, name) == 0) {
      LOG("Fetched resource '%s' at $%lx.\n", res->Name, res->Ptr);
      return res->Ptr;
    }
  }

  LOG("Resource '%s' not found.\n", name);
  
  return NULL;
}
