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
  ins[0].move.data = (ULONG)data >> 16;
  ins[1].move.data = (ULONG)data;
}

static inline void CopInsSet16(CopInsT *ins, UWORD data) {
  ins->move.data = data;
}

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

#endif
