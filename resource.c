#include <clib/alib_protos.h>
#include <clib/debug_protos.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <string.h>

#include "common.h"
#include "resource.h"

extern struct ResourceDesc ResourceDescList[];
static struct MinList ResourceList;

BOOL ResourcesAlloc()
{
  NewList((struct List *)&ResourceList);

  struct ResourceDesc *resDesc;

  for (resDesc = ResourceDescList; resDesc->Name; resDesc++) {
    APTR res = resDesc->AllocFunc();

    if (!res)
      return FALSE;

    struct ResourceNode *resNode = NEW_SZ(struct ResourceNode);

    resNode->Desc = resDesc;
    resNode->Ptr = res;

    KPrintF("Allocated resource '%s' at $%lx.\n", resDesc->Name, res);

    AddTail((struct List *)&ResourceList, (struct Node *)resNode);
  }

  return TRUE;
}

BOOL ResourcesInit() {
  struct MinNode *node;

  for (node = ResourceList.mlh_Head; node->mln_Succ; node = node->mln_Succ) {
    struct ResourceNode *resNode = (struct ResourceNode *)node;

    if (!resNode->Desc->InitFunc)
      continue;

    KPrintF("Initiating resource '%s'.\n", resNode->Desc->Name);

    if (!resNode->Desc->InitFunc(resNode->Ptr))
      return FALSE;
  }

  return TRUE;
}

void ResourcesFree() {
  while (!IsListEmpty((struct List *)&ResourceList)) {
    struct ResourceNode *resNode = (struct ResourceNode *)
      RemHead((struct List *)&ResourceList);

    KPrintF("Freeing resource '%s' at $%lx.\n", resNode->Desc->Name, resNode->Ptr);

    if (resNode->Desc->FreeFunc)
      resNode->Desc->FreeFunc(resNode->Ptr);
    else
      DELETE(resNode->Ptr);

    DELETE(resNode);
  }
}

APTR GetResource(const char *name) {
  struct MinNode *node;

  for (node = ResourceList.mlh_Head; node->mln_Succ; node = node->mln_Succ) {
    struct ResourceNode *resNode = (struct ResourceNode *)node;

    if (strcmp(resNode->Desc->Name, name) == 0) {
      KPrintF("Fetched resource '%s' at $%lx.\n", resNode->Desc->Name, resNode->Ptr);
      return resNode->Ptr;
    }
  }

  KPrintF("Resource '%s' not found.\n", name);
  
  return NULL;
}
