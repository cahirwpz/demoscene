#include <proto/exec.h>
#include <exec/execbase.h>
#include <exec/memory.h>

#include "debug.h"
#include "memory.h"

#define THRESHOLD 4096

void BootMemory(void) {
  struct List *list = &SysBase->MemList;
  struct Node *node;

  for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ) {
    struct MemHeader *mh = (struct MemHeader *)node;

    if (mh->mh_Attributes & MEMF_PUBLIC) {
      struct MemChunk *mc;
     
      for (mc = mh->mh_First; mc; mc = mc->mc_Next) {
        if (mc->mc_Bytes >= THRESHOLD)
          AddMemory((void *)mc + sizeof(struct MemChunk), mc->mc_Bytes,
                    mh->mh_Attributes);
      }
    }
  }
}
