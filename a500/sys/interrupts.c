#include <exec/nodes.h>
#include <exec/execbase.h>

#include "common.h"
#include "interrupts.h"

void DumpInterrupts() {
  LONG i;

  for (i = 0; i < 16; i++) {
    struct IntVector *intvec = &SysBase->IntVects[i];
    struct Node *node = intvec->iv_Node;

    if ((i == INTB_PORTS) || (i == INTB_COPER) || (i == INTB_VERTB) || 
        (i == INTB_EXTER) || (i == INTB_NMI))
    {
      struct List *list = (struct List *)intvec->iv_Data;

      if (IsListEmpty(list))
        continue;

      node = list->lh_Head;

      Log("INT%ld: [%lx, %lx]\n", i, 
          (LONG)intvec->iv_Code, (LONG)intvec->iv_Data);
      for (;;) {
        Log("> %lx (%ld) '%s'\n", (LONG)node, (LONG)node->ln_Pri, node->ln_Name);
        if (node == list->lh_TailPred)
          break;
        node = node->ln_Succ;
      }
    } else {
      if (!intvec->iv_Code)
        continue;
      Log("INT%ld: [%lx, %lx]\n", i, 
          (LONG)intvec->iv_Code, (LONG)intvec->iv_Data);
      while (node) {
        Log("> %lx (%ld) '%s'\n", (LONG)node, (LONG)node->ln_Pri, node->ln_Name);
        node = node->ln_Succ;
      }
    }
  }
}
