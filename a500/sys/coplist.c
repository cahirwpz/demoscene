#include <exec/memory.h>
#include <proto/exec.h>

#include "coplist.h"

__regargs CopListT *NewCopList(UWORD length) {
  CopListT *list = AllocMem(sizeof(CopListT) + length * sizeof(CopInsT),
                            MEMF_CHIP|MEMF_CLEAR);

  list->length = length;

  CopInit(list);

  return list;
}

__regargs void DeleteCopList(CopListT *list) {
  FreeMem(list, sizeof(CopListT) + list->length * sizeof(CopInsT));
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
  list->curr = list->entry;
}

__regargs CopInsT *CopWait(CopListT *list, UWORD vp, UWORD hp) {
  UWORD *ins = (UWORD *)list->curr;

  if ((vp >= 256) && (!list->flags)) {
    *((ULONG *)ins)++ = 0xffdffffe;
    list->flags |= 1;
  }

  {
    CopInsT *ptr = (CopInsT *)ins;

    *ins++ = (vp << 8) | (hp & 0xfe) | 1;
    *ins++ = 0xfffe;

    list->curr = (CopInsT *)ins;

    return ptr;
  }
}

__regargs CopInsT *CopWaitMask(CopListT *list,
                               UWORD vp, UWORD hp, UWORD vpmask, UWORD hpmask)
{
  UWORD *ins = (UWORD *)list->curr;

  if ((vp >= 256) && (!list->flags)) {
    *((ULONG *)ins)++ = 0xffdffffe;
    list->flags |= 1;
  }

  {
    CopInsT *ptr = (CopInsT *)ins;

    *ins++ = (vp << 8) | (hp & 0xfe) | 1;
    *ins++ = 0x8000 | ((vpmask << 8) & 0x7f00) | (hpmask & 0xfe);

    list->curr = (CopInsT *)ins;

    return ptr;
  }
}

__regargs CopInsT *CopLoadPal(CopListT *list, PaletteT *palette, UWORD start) {
  CopInsT *ptr = list->curr;
  UWORD *ins = (UWORD *)ptr;
  ColorT *c = palette->colors;
  UWORD i;
  UWORD n = min(palette->count, 32 - start);

  for (i = 0; i < n; i++, c++) {
    *ins++ = CSREG(color[i + start]);
    *ins++ = ((c->r & 0xf0) << 4) | (c->g & 0xf0) | ((c->b & 0xf0) >> 4);
  }

  list->curr = (CopInsT *)ins;
  return ptr;
}
