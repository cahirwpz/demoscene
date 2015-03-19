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
  CopMove16(list, bplcon2, BPLCON2_PF2P2 | BPLCON2_PF1P2);
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
    w /= 2;
  if (mode & MODE_LACE)
    h /= 2;

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
    ddfstrt = ((xs - 9) / 2) & ~3;
    ddfstop = ddfstrt + (w / 4 - 8);
  } else {
    ddfstrt = ((xs - 17) / 2) & ~7;
    ddfstop = ddfstrt + (w / 2 - 8);
  }

  CopMove16(list, ddfstrt, ddfstrt);
  CopMove16(list, ddfstop, ddfstop);
  CopMove16(list, fmode, 0);
}
 
__regargs void CopSetupBitplanes(CopListT *list, CopInsT **bplptr,
                                 BitmapT *bitmap, UWORD depth) 
{
  WORD i, modulo = 0;
  
  if (bitmap->flags & BM_INTERLEAVED)
    modulo = bitmap->bytesPerRow * (depth - 1);

  CopMove16(list, bpl1mod, modulo);
  CopMove16(list, bpl2mod, modulo);

  for (i = 0; i < depth; i++) {
    CopInsT *bplpt = CopMove32(list, bplpt[i], bitmap->planes[i]);

    if (bplptr)
      bplptr[i] = bplpt;
  }
}

__regargs void CopSetupBitplanesArea(CopListT *list, CopInsT **bpltptr, 
                                     BitmapT *bitmap, UWORD xs, UWORD ys, 
                                     UWORD width) 
{
  WORD i;
  LONG start = ys * bitmap->bytesPerRow + (xs >> 3);
  WORD modulo = bitmap->bytesPerRow - (width >> 3);

  CopMove16(list, bplcon0, BPLCON0_BPU(bitmap->depth) | BPLCON0_COLOR);
  CopMove16(list, bplcon1, 0);
  CopMove16(list, bplcon2, BPLCON2_PF2P2 | BPLCON2_PF1P2);
  CopMove16(list, bplcon3, 0);
  
  CopMove16(list, bpl1mod, modulo);
  CopMove16(list, bpl2mod, modulo);

  for (i = 0; i < bitmap->depth; i++)
    CopMove32(list, bplpt[i], bitmap->planes[i] + start);
}
