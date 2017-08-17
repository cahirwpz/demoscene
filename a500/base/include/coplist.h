#ifndef __COPLIST_H__
#define __COPLIST_H__

#include "gfx.h"
#include "hardware.h"

#define MODE_LORES  0
#define MODE_HIRES  BPLCON0_HIRES
#define MODE_DUALPF BPLCON0_DBLPF
#define MODE_LACE   BPLCON0_LACE
#define MODE_HAM    BPLCON0_HOMOD

/* Copper instructions assumptions for PAL systems:
 *
 * Copper resolution is 1 color clock = 4 pixels in low resolution.
 *
 * > Vertical Position range is 0..311
 *   Keep it mind that 'vp' counter overflows at 255 !
 *
 * > Horizontal Position range is in 0..266
 *   In fact WAIT & SKIP uses 'hp' without the least significant bit !
 *
 * MOVE & SKIP take 2 color clocks
 * WAIT takes 3 color clocks (last to wake up)
 */

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
  CopInsT entry[0]; 
} CopListT;

__regargs CopListT *NewCopList(UWORD length);
__regargs void DeleteCopList(CopListT *list);
__regargs void CopListActivate(CopListT *list);
__regargs void CopInit(CopListT *list);

/* Low-level functions */
__regargs CopInsT *CopMoveWord(CopListT *list, UWORD reg, UWORD data);
__regargs CopInsT *CopMoveLong(CopListT *list, UWORD reg, APTR data);

#define CSREG(reg) (UWORD)offsetof(struct Custom, reg)
#define CopMove16(cp, reg, data) CopMoveWord(cp, CSREG(reg), data)
#define CopMove32(cp, reg, data) CopMoveLong(cp, CSREG(reg), data)

__regargs CopInsT *CopWait(CopListT *list, UWORD vp, UWORD hp);
__regargs CopInsT *CopWaitMask(CopListT *list, UWORD vp, UWORD hp, 
                               UWORD vpmask asm("d2"), UWORD hpmask asm("d3"));

/* The most significant bit of vertical position cannot be masked out (overlaps
 * with blitter-finished-disable bit), so we have to pass upper bit as well. */
#define CopWaitH(cp, vp, hp) CopWaitMask(cp, vp & 128, hp, 0, 255)
#define CopWaitV(cp, vp) CopWaitMask(cp, vp, 0, 255, 0)

__regargs CopInsT *CopSkip(CopListT *list, UWORD vp, UWORD hp);
__regargs CopInsT *CopSkipMask(CopListT *list, UWORD vp, UWORD hp, 
                               UWORD vpmask asm("d2"), UWORD hpmask asm("d3"));

#define CopSkipH(cp, vp, hp) CopSkipMask(cp, vp & 128, hp, 0, 255)
#define CopSkipV(cp, vp) CopSkipMask(cp, vp, 0, 255, 0)

__regargs void CopEnd(CopListT *list);

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

/* High-level functions */
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
void CopSetupBitplaneArea(CopListT *list, UWORD mode, UWORD depth,
                          BitmapT *bitmap, WORD x, WORD y, Area2D *area);
__regargs void CopUpdateBitplanes(CopInsT **bplptr, BitmapT *bitmap, WORD n);

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
