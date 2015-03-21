#include "coplist.h"
#include "memory.h"

__regargs CopListT *NewCopList(UWORD length) {
  CopListT *list = MemAlloc(sizeof(CopListT) + length * sizeof(CopInsT),
                            MEMF_CHIP|MEMF_CLEAR);

  list->length = length;

  CopInit(list);

  return list;
}

__regargs void DeleteCopList(CopListT *list) {
  MemFree(list, sizeof(CopListT) + list->length * sizeof(CopInsT));
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

    *((UBYTE *)ins)++ = vp;
    *((UBYTE *)ins)++ = hp | 1;
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

    *((UBYTE *)ins)++ = vp;
    *((UBYTE *)ins)++ = hp | 1;
    *((UBYTE *)ins)++ = 0x80 | vpmask;
    *((UBYTE *)ins)++ = hpmask & 0xfe;

    list->curr = (CopInsT *)ins;

    return ptr;
  }
}

__regargs CopInsT *CopLoadPal(CopListT *list, PaletteT *palette, UWORD start) {
  CopInsT *ptr = list->curr;
  UWORD *ins = (UWORD *)ptr;
  UBYTE *c = (UBYTE *)palette->colors;
  WORD n = min(palette->count, (UWORD)(32 - start)) - 1;

  do {
    UBYTE r = *c++ & 0xf0;
    UBYTE g = *c++ & 0xf0;
    UBYTE b = *c++ & 0xf0;

    *ins++ = CSREG(color[start++]);
    *ins++ = (r << 4) | (UBYTE)(g | (b >> 4));
  } while (--n != -1);

  list->curr = (CopInsT *)ins;
  return ptr;
}

__regargs CopInsT *CopLoadColor(CopListT *list, UWORD start, UWORD end, UWORD color) {
  CopInsT *ptr = list->curr;
  UWORD *ins = (UWORD *)ptr;

  while (start <= end) {
    *ins++ = CSREG(color[start++]);
    *ins++ = color;
  }

  list->curr = (CopInsT *)ins;
  return ptr;
}

__regargs CopInsT *CopSetColor(CopListT *list, WORD i, ColorT *color) {
  UBYTE *c = (UBYTE *)color;
  UBYTE r = *c++ & 0xf0;
  UBYTE g = *c++ & 0xf0;
  UBYTE b = *c++ & 0xf0;

  return CopMove16(list, color[i], (r << 4) | (UBYTE)(g | (b >> 4)));
}

__regargs void CopSetupMode(CopListT *list, UWORD mode, UWORD depth) {
  CopMove16(list, bplcon0, BPLCON0_BPU(depth) | BPLCON0_COLOR | mode);
  CopMove16(list, bplcon2, BPLCON2_PF2P2 | BPLCON2_PF1P2 | BPLCON2_PF2PRI);
  CopMove16(list, bplcon3, 0);
}

/* Arguments must be always specified in low resolution coordinates. */
__regargs void CopSetupDisplayWindow(CopListT *list, UWORD mode, 
                                     UWORD xs, UWORD ys, UWORD w, UWORD h)
{
  /* vstart  $00 ..  $ff */
  /* hstart  $00 ..  $ff */
  /* vstop   $80 .. $17f */
  /* hstop  $100 .. $1ff */
  UBYTE xe, ye;

  if (mode & MODE_HIRES)
    w >>= 1;
  if (mode & MODE_LACE)
    h >>= 1;

  xe = xs + w;
  ye = ys + h;

  CopMove16(list, diwstrt, (ys << 8) | xs);
  CopMove16(list, diwstop, (ye << 8) | xe);
}

__regargs void CopSetupBitplaneFetch(CopListT *list, UWORD mode,
                                     UWORD xs, UWORD w)
{
  UWORD ddfstrt, ddfstop;

  if (mode & MODE_HIRES) {
    xs -= 9;
    w >>= 2;
    ddfstrt = (xs >> 1) & ~3;
    ddfstop = ddfstrt + w - 8;
  } else {
    xs -= 17;
    w >>= 1;
    ddfstrt = (xs >> 1) & ~7;
    ddfstop = ddfstrt + w - 8;
  }

  CopMove16(list, ddfstrt, ddfstrt);
  CopMove16(list, ddfstop, ddfstop);
  CopMove16(list, fmode, 0);
}
 
__regargs void CopSetupBitplanes(CopListT *list, CopInsT **bplptr,
                                 BitmapT *bitmap, UWORD depth) 
{
  {
    APTR *planes = bitmap->planes;
    WORD n = depth - 1;
    WORD i = 0;

    do {
      CopInsT *ins = CopMove32(list, bplpt[i++], *planes++);

      if (bplptr)
        *bplptr++ = ins;
    } while (--n != -1);
  }

  {
    WORD modulo = 0;

    if (bitmap->flags & BM_INTERLEAVED)
      modulo = (WORD)bitmap->bytesPerRow * (WORD)(depth - 1);

    CopMove16(list, bpl1mod, modulo);
    CopMove16(list, bpl2mod, modulo);
  }
}

__regargs void CopUpdateBitplanes(CopInsT **bplptr, BitmapT *bitmap, WORD n) {
  APTR *planes = bitmap->planes;

  while (--n >= 0)
    CopInsSet32(*bplptr++, *planes++);
}
