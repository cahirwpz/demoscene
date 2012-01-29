#include <clib/debug_protos.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <string.h>

#include "common.h"
#include "resource.h"

extern struct Resource ResourceList[];

BOOL ResourcesAlloc()
{
  struct Resource *res;

  for (res = ResourceList; res->Name; res++) {
    if (!(res->Ptr = res->AllocFunc()))
      return FALSE;

    KPrintF("Allocated resource '%s' at $%lx.\n", res->Name, res->Ptr);
  }

  return TRUE;
}

BOOL ResourcesInit() {
  struct Resource *res;

  for (res = ResourceList; res->Name; res++) {
    if (!res->InitFunc)
      continue;

    KPrintF("Initiating resource '%s'.\n", res->Name);

    if (!res->InitFunc(res->Ptr))
      return FALSE;
  }

  return TRUE;
}

void ResourcesFree() {
  struct Resource *res;

  for (res = ResourceList; res->Name; res++) {
    KPrintF("Freeing resource '%s' at $%lx.\n", res->Name, res->Ptr);

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
      KPrintF("Fetched resource '%s' at $%lx.\n", res->Name, res->Ptr);
      return res->Ptr;
    }
  }

  KPrintF("Resource '%s' not found.\n", name);
  
  return NULL;
}
