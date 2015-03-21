#ifndef __COPLIST_H__
#define __COPLIST_H__

#include "gfx.h"
#include "hardware.h"

#define MODE_LORES  0
#define MODE_HIRES  BPLCON0_HIRES
#define MODE_DUALPF BPLCON0_DBLPF
#define MODE_LACE   BPLCON0_LACE
#define MODE_HAM    BPLCON0_HOMOD

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
__regargs CopInsT *CopSetColor(CopListT *list, WORD i, ColorT *color);

__regargs void CopSetupMode(CopListT *list, UWORD mode, UWORD depth);
__regargs void CopSetupDisplayWindow(CopListT *list, UWORD mode, 
                                     UWORD xs, UWORD ys, UWORD w, UWORD h);
__regargs void CopSetupBitplaneFetch(CopListT *list, UWORD mode,
                                     UWORD xs, UWORD w);
__regargs void CopSetupBitplanes(CopListT *list, CopInsT **bplptr,
                                 BitmapT *bitmap, UWORD depth);
__regargs void CopUpdateBitplanes(CopInsT **bplptr, BitmapT *bitmap, WORD n);

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

static inline void CopSetupGfxSimple(CopListT *list, UWORD mode, UWORD depth,
                                     UWORD xs, UWORD ys, UWORD w, UWORD h) 
{
  CopSetupMode(list, mode, depth);
  CopSetupDisplayWindow(list, mode, xs, ys, w, h);
  CopSetupBitplaneFetch(list, mode, xs, w);
}

static inline CopInsT *CopSetRGB(CopListT *list, WORD i, UWORD value) {
  return CopMove16(list, color[i], value);
}

static inline void CopListRun(CopListT *list) {
  custom->cop1lc = (ULONG)list->entry;
}

#endif
