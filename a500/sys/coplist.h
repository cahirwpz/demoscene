#ifndef __COPLIST_H__
#define __COPLIST_H__

#include "gfx.h"
#include "hardware.h"

typedef union {
  struct {
    WORD vpos;
    WORD hpos;
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
 
static inline void CopMakePlayfield(CopListT *list, CopInsT **bplptr, BitmapT *bitmap) {
  UWORD i, modulo;
  BOOL hires = bitmap->width > 320;

  CopMove16(list, bplcon0, BPLCON0_BPU(bitmap->depth) | BPLCON0_COLOR |
            (hires ? BPLCON0_HIRES : 0));
  CopMove16(list, bplcon1, 0);
  CopMove16(list, bplcon2, BPLCON2_PF2P2 | BPLCON2_PF1P2);
  CopMove16(list, bplcon3, 0);
  
  modulo = bitmap->interleaved ? (bitmap->width / 8 * (bitmap->depth - 1)) : 0;

  CopMove16(list, bpl1mod, modulo);
  CopMove16(list, bpl2mod, modulo);

  for (i = 0; i < bitmap->depth; i++) {
    CopInsT *bplpt = CopMove32(list, bplpt[i], bitmap->planes[i]);

    if (bplptr)
      bplptr[i] = bplpt;
  }

  CopMove16(list, ddfstrt, hires ? 0x3c : 0x38);
  CopMove16(list, ddfstop, 0xd0);
  CopMove16(list, fmode, 0);
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

  CopMove16(list, diwstrt, (ys << 8) | xs);
  CopMove16(list, diwstop, (ye << 8) | xe);
}

static inline CopInsT *CopSetRGB(CopListT *list, UWORD i, UWORD value) {
  return CopMove16(list, color[i], value);
}

#endif
