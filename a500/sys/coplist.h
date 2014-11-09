#ifndef __COPLIST_H__
#define __COPLIST_H__

#include "gfx.h"
#include "hardware.h"

typedef union {
  struct {
    UBYTE vp;
    UBYTE hp;
    UBYTE vpmask;
    UBYTE hpmask;
  } wait;
  struct {
    WORD reg;
    WORD data;
  } move;
} CopInsT;

typedef struct {
  CopInsT *curr;

  UWORD length;
  UWORD flags;

  CopInsT entry[0]; 
} CopListT;

__regargs CopListT *NewCopList(UWORD length);
__regargs void DeleteCopList(CopListT *copList);
__regargs void CopListActivate(CopListT *copList);
__regargs void CopInit(CopListT *copList);
__regargs CopInsT *CopWait(CopListT *copList, UWORD vp, UWORD hp);
__regargs CopInsT *CopWaitMask(CopListT *list,
                               UWORD vp, UWORD hp, UWORD vpmask, UWORD hpmask);
__regargs CopInsT *CopLoadPal(CopListT *list, PaletteT *palette, UWORD start);
__regargs CopInsT *CopLoadColor(CopListT *list, UWORD start, UWORD end, UWORD color);

static inline CopInsT *CopMoveWord(CopListT *list, UWORD reg, UWORD data) {
  CopInsT *ptr = list->curr;
  UWORD *ins = (UWORD *)ptr;

  *ins++ = reg & 0x01fe;
  *ins++ = data;

  list->curr = (CopInsT *)ins;
  return ptr;
}

static inline CopInsT *CopMoveLong(CopListT *list, UWORD reg, APTR data) {
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

static inline void CopEnd(CopListT *list) {
  ULONG *ins = (ULONG *)list->curr;

  *ins++ = 0xfffffffe;

  list->curr = (CopInsT *)ins;

  // Log("Copper list entries: %ld.\n", (LONG)(list->curr - list->entry));
}

#define CSREG(reg) (UWORD)offsetof(struct Custom, reg)
#define CopMove16(cp, reg, data) CopMoveWord(cp, CSREG(reg), data)
#define CopMove32(cp, reg, data) CopMoveLong(cp, CSREG(reg), data)

static inline void CopInsSet32(CopInsT *ins, APTR data) {
  asm volatile("movew %0,%2\n"
               "swap  %0\n"
               "movew %0,%1\n"
               : "+d" (data)
               : "m" (ins[0].move.data), "m" (ins[1].move.data));
}

static inline void CopInsSet16(CopInsT *ins, UWORD data) {
  ins->move.data = data;
}

static inline void CopInsSetRGB24(CopInsT *ins, UBYTE r, UBYTE g, UBYTE b) {
  ins->move.data = ((r & 0xf0) << 4) | (g & 0xf0) | ((b & 0xf0) >> 4);
}
 
static inline void CopMakePlayfield(CopListT *list, CopInsT **bplptr, BitmapT *bitmap, UWORD depth) {
  UWORD i, modulo;

  CopMove16(list, bplcon0, BPLCON0_BPU(depth) | BPLCON0_COLOR |
            (bitmap->width > 512 ? BPLCON0_HIRES : 0));
  CopMove16(list, bplcon1, 0);
  CopMove16(list, bplcon2, BPLCON2_PF2P2 | BPLCON2_PF1P2);
  CopMove16(list, bplcon3, 0);
  
  modulo = (bitmap->flags & BM_INTERLEAVED) ? (bitmap->bytesPerRow * (depth - 1)) : 0;

  CopMove16(list, bpl1mod, modulo);
  CopMove16(list, bpl2mod, modulo);

  for (i = 0; i < depth; i++) {
    CopInsT *bplpt = CopMove32(list, bplpt[i], bitmap->planes[i]);

    if (bplptr)
      bplptr[i] = bplpt;
  }
}

static inline void CopShowPlayfield(CopListT *list, BitmapT *bitmap) {
  UWORD i;

  CopMove16(list, bplcon0, BPLCON0_BPU(bitmap->depth) | BPLCON0_COLOR);
  CopMove16(list, bplcon1, 0);
  CopMove16(list, bplcon2, BPLCON2_PF2P2 | BPLCON2_PF1P2);
  CopMove16(list, bplcon3, 0);
  
  CopMove16(list, bpl1mod, 0);
  CopMove16(list, bpl2mod, 0);

  for (i = 0; i < bitmap->depth; i++)
    CopMove32(list, bplpt[i], bitmap->planes[i]);
}

static inline void CopShowPlayfieldArea(CopListT *list, BitmapT *bitmap, UWORD xs, UWORD ys, UWORD width) {
  UWORD i;
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

/* Arguments must be always specified in low resolution coordinates. */
static inline void
CopMakeDispWin(CopListT *list, UBYTE xs, UBYTE ys, UWORD w, UWORD h) {
  /* vstart  $00 ..  $ff */
  /* hstart  $00 ..  $ff */
  /* vstop   $80 .. $17f */
  /* hstop  $100 .. $1ff */
  UBYTE xe = xs + w;
  UBYTE ye = ys + h;
  UWORD ddfstrt = ((xs - 17) / 2) & ~7;
  UWORD ddfstop = ddfstrt + (w / 2 - 8);

  CopMove16(list, diwstrt, (ys << 8) | xs);
  CopMove16(list, diwstop, (ye << 8) | xe);

  CopMove16(list, ddfstrt, ddfstrt);
  CopMove16(list, ddfstop, ddfstop);
  CopMove16(list, fmode, 0);
}

static inline void
CopMakeDispWinHiRes(CopListT *list, UBYTE xs, UBYTE ys, UWORD w, UWORD h) {
  /* vstart  $00 ..  $ff */
  /* hstart  $00 ..  $ff */
  /* vstop   $80 .. $17f */
  /* hstop  $100 .. $1ff */
  UBYTE xe = xs + w / 2;
  UBYTE ye = ys + h;
  UWORD ddfstrt = ((xs - 9) / 2) & ~3;
  UWORD ddfstop = ddfstrt + (w / 4 - 8);

  CopMove16(list, diwstrt, (ys << 8) | xs);
  CopMove16(list, diwstop, (ye << 8) | xe);

  CopMove16(list, ddfstrt, ddfstrt);
  CopMove16(list, ddfstop, ddfstop);
  CopMove16(list, fmode, 0);
}

static inline CopInsT *CopSetRGB(CopListT *list, UWORD i, UWORD value) {
  return CopMove16(list, color[i], value);
}

static inline void CopListRun(CopListT *list) {
  custom->cop1lc = (ULONG)list->entry;
}

#endif
