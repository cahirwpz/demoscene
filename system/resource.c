#include <string.h>

#include "debug.h"
#include "common.h"
#include "resource.h"

extern struct Resource ResourceList[];

BOOL ResourcesAlloc()
{
  struct Resource *res;

  for (res = ResourceList; res->Name; res++) {
    if (!(res->Ptr = res->AllocFunc()))
      return FALSE;

    LOG("Allocated resource '%s' at $%lx.\n", res->Name, res->Ptr);
  }

  return TRUE;
}

BOOL ResourcesInit() {
  struct Resource *res;

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
  struct Resource *res;

  for (res = ResourceList; res->Name; res++) {
    LOG("Freeing resource '%s' at $%lx.\n", res->Name, res->Ptr);

    if (res->FreeFunc)
      res->FreeFunc(res->Ptr);
    else
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
