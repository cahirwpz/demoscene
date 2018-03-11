#include <exec/nodes.h>
#include <exec/execbase.h>

#include "common.h"
#include "interrupts.h"

static const char *IntName[] = {
  [INTB_EXTER] = "EXTER",
  [INTB_DSKSYNC] = "DSKSYNC",
  [INTB_RBF] = "RBF",
  [INTB_AUD3] = "AUD3",
  [INTB_AUD2] = "AUD2",
  [INTB_AUD1] = "AUD1",
  [INTB_AUD0] = "AUD0",
  [INTB_BLIT] = "BLIT",
  [INTB_VERTB] = "VERTB",
  [INTB_COPER] = "COPER",
  [INTB_PORTS] = "PORTS",
  [INTB_SOFTINT] = "SOFTINT",
  [INTB_DSKBLK] = "DSKBLK",
  [INTB_TBE] = "TBE",
};

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

      Log("%s: [C:%lx, D:%lx]\n", IntName[i], 
          (LONG)intvec->iv_Code, (LONG)intvec->iv_Data);
      for (;;) {
        struct Interrupt *intr = (struct Interrupt *)node;
        Log(" - %lx [C:%lx, D:%lx] (%ld) '%s'\n",
            (LONG)node, (LONG)intr->is_Code, (LONG)intr->is_Data,
            (LONG)node->ln_Pri, node->ln_Name);
        if (node == list->lh_TailPred)
          break;
        node = node->ln_Succ;
      }
    } else {
      if (!intvec->iv_Code)
        continue;
      Log("%s: [C:%lx, D:%lx]\n", IntName[i], 
          (LONG)intvec->iv_Code, (LONG)intvec->iv_Data);
      while (node) {
        Log(" - %lx (%ld) '%s'\n",
            (LONG)node, (LONG)node->ln_Pri, node->ln_Name);
        node = node->ln_Succ;
      }
    }
  }
}
