#include <exec/memory.h>
#include <inline/exec.h>

#include "startup.h"
#include "coplist.h"
#include "hardware.h"

__regargs CopListT *NewCopList(UWORD length) {
  CopListT *copList = AllocMem(sizeof(CopListT) + length * sizeof(ULONG),
                               MEMF_CHIP|MEMF_CLEAR);
  copList->length = length - 1;
  copList->index = 0;
  copList->flags = 0;

  return copList;
}

__regargs void DeleteCopList(CopListT *copList) {
  FreeMem(copList, sizeof(CopListT) + (copList->length + 1) * sizeof(ULONG));
}

__regargs void CopListActivate(CopListT *copList) {
  WaitVBlank();
  /* Write copper list address. */
  custom->cop1lc = (ULONG)copList->entry;
  /* Activate it immediately */
  custom->copjmp1 = 0;
  /* Enable copper DMA */
  custom->dmacon = DMAF_MASTER | DMAF_COPPER | DMAF_SETCLR;
}

__regargs void CopInit(CopListT *copList) {
  copList->index = 0;
  copList->flags = 0;
}

__regargs void CopWait(CopListT *copList, UWORD vp, UWORD hp) {
  if (vp <= 255) {
    if (copList->index < copList->length) {
      UWORD *word = (UWORD *)&copList->entry[copList->index++];

      word[0] = (vp << 8) | (hp & 0xff) | 1;
      word[1] = 0xfffe;
    }
  } else {
    if (copList->index - 1 < copList->length) {
      if (!copList->flags) {
        copList->entry[copList->index++] = 0xffdffffe;
        copList->flags |= 1;
      }

      {
        UWORD *word = (UWORD *)&copList->entry[copList->index++];

        word[0] = ((vp - 255) << 8) | (hp & 0xff) | 1;
        word[1] = 0xfffe;
      }
    }
  }
}

__regargs void CopMove16(CopListT *copList, UWORD reg, UWORD data) {
  if (copList->index < copList->length) {
    UWORD *word = (UWORD *)&copList->entry[copList->index++];
    
    word[0] = reg & 0x01fe;
    word[1] = data;
  }
}

__regargs void CopMove32(CopListT *copList, UWORD reg, ULONG data) {
  if (copList->index - 1 < copList->length) {
    UWORD *word = (UWORD *)&copList->entry[copList->index];

    word[0] = reg & 0x01fe;
    word[1] = data >> 16;
    word[2] = (reg + 2) & 0x01fe;
    word[3] = data;

    copList->index += 2;
  }
}

__regargs void CopEnd(CopListT *copList) {
  if (copList->index <= copList->length)
    copList->entry[copList->index++] = 0xfffffffe;
}
