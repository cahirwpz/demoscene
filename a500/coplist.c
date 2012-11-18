#include <exec/memory.h>
#include <proto/exec.h>

#include "coplist.h"
#include "hardware.h"

__regargs CopListT *NewCopList(UWORD length) {
  CopListT *list = AllocMem(sizeof(CopListT) + 2 * length * sizeof(UWORD),
                               MEMF_CHIP|MEMF_CLEAR);

  list->length = length;

  CopInit(list);

  return list;
}

__regargs void DeleteCopList(CopListT *list) {
  FreeMem(list, sizeof(CopListT) + 2 * list->length * sizeof(UWORD));
}

__regargs void CopListActivate(CopListT *list) {
  WaitVBlank();
  /* Write copper list address. */
  custom->cop1lc = (ULONG)list->entry;
  /* Activate it immediately */
  custom->copjmp1 = 0;
  /* Enable copper DMA */
  custom->dmacon = DMAF_MASTER | DMAF_COPPER | DMAF_SETCLR;
}

__regargs void CopInit(CopListT *list) {
  list->flags = 0;
  list->last = &list->entry[2 * (list->length - 1)];
  list->curr = list->entry;
}

__regargs void CopWait(CopListT *list, UWORD vp, UWORD hp) {
  UWORD *insn = list->curr;

  if (insn < list->last) {
    if (vp < 256) {
      *insn++ = (vp << 8) | (hp & 0xff) | 1;
      *insn++ = 0xfffe;
    } else {
      if (!list->flags) {
        *((ULONG *)insn)++ = 0xffdffffe;
        list->flags |= 1;
      }

      if (insn < list->last) {
        *insn++ = ((vp - 255) << 8) | (hp & 0xff) | 1;
        *insn++ = 0xfffe;
      }
    }

    list->curr = insn;
  }
}

__regargs void CopMove16(CopListT *list, UWORD reg, UWORD data) {
  UWORD *insn = list->curr;

  if (insn < list->last) {
    *insn++ = reg & 0x01fe;
    *insn++ = data;

    list->curr = insn;
  }
}

__regargs void CopMove32(CopListT *list, UWORD reg, ULONG data) {
  UWORD *insn = list->curr;

  if (insn - 2 < list->last) {
    reg &= 0x01fe;

    *insn++ = reg;
    *insn++ = data >> 16;
    *insn++ = reg + 2;
    *insn++ = data;

    list->curr = insn;
  }
}

__regargs void CopEnd(CopListT *list) {
  if (list->curr <= list->last)
    *((ULONG *)list->curr)++ = 0xfffffffe;
}
