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
  LONG unused = list->length - (list->curr - list->entry);
  if (unused >= 100)
    Log("Unused copper list entries: %ld.\n", unused);
  MemFree(list);
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
  list->curr = list->entry;
  list->flags = 0;
}

__regargs CopInsT *CopMoveWord(CopListT *list, UWORD reg, UWORD data) {
  CopInsT *ptr = list->curr;
  UWORD *ins = (UWORD *)ptr;

  *ins++ = reg & 0x01fe;
  *ins++ = data;

  list->curr = (CopInsT *)ins;
  return ptr;
}

__regargs CopInsT *CopMoveLong(CopListT *list, UWORD reg, APTR data) {
  CopInsT *ptr = list->curr;
  UWORD *ins = (UWORD *)ptr;

  reg &= 0x01fe;

  *ins++ = reg;
  *ins++ = (ULONG)data >> 16;
  *ins++ = reg + 2;
  *ins++ = (ULONG)data;

  list->curr = (CopInsT *)ins;
  return ptr;
}

__regargs void CopEnd(CopListT *list) {
  ULONG *ins = (ULONG *)list->curr;

  *ins++ = 0xfffffffe;

  list->curr = (CopInsT *)ins;
}

__regargs CopInsT *CopWait(CopListT *list, UWORD vp, UWORD hp) {
  CopInsT *ptr = list->curr;
  APTR ins = ptr;

  *((UBYTE *)ins)++ = vp;
  *((UBYTE *)ins)++ = hp | 1;
  *((UWORD *)ins)++ = 0xfffe;

  list->curr = (CopInsT *)ins;
  return ptr;
}

__regargs CopInsT *CopWaitSafe(CopListT *list, UWORD vp, UWORD hp) {
  if (!(list->flags & CLF_VPOVF) && (vp >= 256)) {
    /* Wait for last waitable position to control when overflow occurs. */
    CopWaitEOL(list, 255);
    list->flags |= CLF_VPOVF;
  }

  return CopWait(list, vp, hp);
}

__regargs CopInsT *CopWaitMask(CopListT *list, UWORD vp, UWORD hp,
                               UWORD vpmask asm("d2"), UWORD hpmask asm("d3"))
{
  CopInsT *ptr = list->curr;
  UBYTE *ins = (UBYTE *)ptr;

  *ins++ = vp;
  *ins++ = hp | 1;
  *ins++ = 0x80 | vpmask;
  *ins++ = hpmask & 0xfe;

  list->curr = (CopInsT *)ins;
  return ptr;
}

__regargs CopInsT *CopSkip(CopListT *list, UWORD vp, UWORD hp) {
  CopInsT *ptr = list->curr;
  APTR ins = ptr;

  *((UBYTE *)ins)++ = vp;
  *((UBYTE *)ins)++ = hp | 1;
  *((UWORD *)ins)++ = 0xffff;

  list->curr = (CopInsT *)ins;
  return ptr;
}

__regargs CopInsT *CopSkipMask(CopListT *list, UWORD vp, UWORD hp,
                               UWORD vpmask asm("d2"), UWORD hpmask asm("d3"))
{
  CopInsT *ptr = (CopInsT *)list->curr;
  UBYTE *ins = (UBYTE *)ptr;

  *ins++ = vp;
  *ins++ = hp | 1;
  *ins++ = 0x80 | vpmask;
  *ins++ = hpmask | 1;

  list->curr = (CopInsT *)ins;
  return ptr;
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
  UBYTE ddfstrt, ddfstop;

  /* DDFSTRT and DDFSTOP have resolution of 4 clocks.
   *
   * Only bits 7..2 of DDFSTRT and DDFSTOP are meaningful on OCS!
   *
   * Values to determine Display Data Fetch Start and Stop must be divisible by
   * 16 pixels, because hardware fetches bitplanes in 16-bit word units.
   * Bitplane fetcher uses 4 clocks for HiRes (1 clock = 4 pixels) and 8 clocks
   * for LoRes (1 clock = 2 pixels) to fetch enough data to display it.
   *
   * HS = Horizontal Start, W = Width (divisible by 16)
   *
   * For LoRes: DDFSTART = HS / 2 - 8.5, DDFSTOP = DDFSTRT + W / 2 - 8
   * For HiRes: DDFSTART = HS / 2 - 4.5, DDFSTOP = DDFSTRT + W / 4 - 8 */
 
  if (mode & MODE_HIRES) {
    xs -= 9;
    w >>= 2;
    ddfstrt = (xs >> 1) & ~3; /* 4 clock resolution */
  } else {
    xs -= 17;
    w >>= 1;
    ddfstrt = (xs >> 1) & ~7; /* 8 clock resolution */
  }

  ddfstop = ddfstrt + w - 8;

  /* Found in UAE source code - DDFSTRT & DDFSTOP matching for:
   * - ECS: does not require DMA or DIW enabled, 
   * - OCS: requires DMA and DIW enabled. */

  CopMove16(list, ddfstrt, ddfstrt);
  CopMove16(list, ddfstop, ddfstop);
  CopMove16(list, bplcon1, ((xs & 15) << 4) | (xs & 15));
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

void CopSetupBitplaneArea(CopListT *list, UWORD mode, UWORD depth,
                          BitmapT *bitmap, WORD x, WORD y, Area2D *area)
{
  APTR *planes = bitmap->planes;
  LONG start;
  WORD modulo;
  WORD w;
  WORD i;

  if (area) {
    w = (area->w + 15) & ~15;
    /* Seems that bitplane fetcher has to be active for at least two words! */
    if (w < 32)
      w = 32;
    start = bitmap->bytesPerRow * area->y + ((area->x >> 3) & ~1);
    modulo = bitmap->bytesPerRow - ((w >> 3) & ~1);
    x -= (area->x & 15);
  } else {
    w = (bitmap->width + 15) & ~15;
    start = 0;
    modulo = 0;
  }

  for (i = 0; i < depth; i++)
    CopMove32(list, bplpt[i], *planes++ + start);

  CopMove16(list, bpl1mod, modulo);
  CopMove16(list, bpl2mod, modulo);

  CopSetupBitplaneFetch(list, mode, x, w);
}

__regargs void CopUpdateBitplanes(CopInsT **bplptr, BitmapT *bitmap, WORD n) {
  APTR *planes = bitmap->planes;

  while (--n >= 0)
    CopInsSet32(*bplptr++, *planes++);
}
