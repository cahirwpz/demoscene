#ifndef __COPPER_H__
#define __COPPER_H__

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

/* Last Horizontal Position in line one can reliably wait on. */
#define LASTHP 0xDE

typedef union {
  struct {
    u_char vp;
    u_char hp;
    u_char vpmask;
    u_char hpmask;
  } wait;
  struct {
    short reg;
    short data;
  } move;
} CopInsT;

#define CLF_VPOVF 1 /* Vertical Position counter overflowed */

typedef struct {
  CopInsT *curr;
  u_short length;
  u_short flags;
  CopInsT entry[0]; 
} CopListT;

CopListT *NewCopList(u_short length);
void DeleteCopList(CopListT *list);
void CopInit(CopListT *list);

/* @brief Enable copper and activate copper list.
 * @warning This function busy-waits for vertical blank. */
void CopListActivate(CopListT *list);

/* @brief Set up copper list to start after vertical blank. */
static inline void CopListRun(CopListT *list) {
  custom->cop1lc = (u_int)list->entry;
}

/* Low-level functions */
CopInsT *CopMoveWord(CopListT *list, u_short reg, u_short data);
CopInsT *CopMoveLong(CopListT *list, u_short reg, void *data);

#define CSREG(reg) (u_short)offsetof(struct Custom, reg)
#define CopMove16(cp, reg, data) CopMoveWord(cp, CSREG(reg), data)
#define CopMove32(cp, reg, data) CopMoveLong(cp, CSREG(reg), data)

/* Official way to represent no-op copper instruction. */
#define CopNoOp(cp) CopMoveWord(cp, 0x1FE, 0)

CopInsT *CopWait(CopListT *list, u_short vp, u_short hp);
CopInsT *CopWaitMask(CopListT *list, u_short vp, u_short hp, 
                     u_short vpmask asm("d2"), u_short hpmask asm("d3"));

/* Handles Copper Vertical Position counter overflow, by inserting CopWaitEOL
 * at first WAIT instruction with VP >= 256. */
CopInsT *CopWaitSafe(CopListT *list, u_short vp, u_short hp);

/* The most significant bit of vertical position cannot be masked out (overlaps
 * with blitter-finished-disable bit), so we have to pass upper bit as well. */
#define CopWaitH(cp, vp, hp) CopWaitMask(cp, vp & 128, hp, 0, 255)
#define CopWaitV(cp, vp) CopWaitMask(cp, vp, 0, 255, 0)
#define CopWaitEOL(cp, vp) CopWait(cp, vp, LASTHP)

CopInsT *CopSkip(CopListT *list, u_short vp, u_short hp);
CopInsT *CopSkipMask(CopListT *list, u_short vp, u_short hp, 
                     u_short vpmask asm("d2"), u_short hpmask asm("d3"));

#define CopSkipH(cp, vp, hp) CopSkipMask(cp, vp & 128, hp, 0, 255)
#define CopSkipV(cp, vp) CopSkipMask(cp, vp, 0, 255, 0)

void CopEnd(CopListT *list);

static inline void CopInsSet32(CopInsT *ins, void *data) {
  asm volatile("movew %0,%2\n"
               "swap  %0\n"
               "movew %0,%1\n"
               : "+d" (data)
               : "m" (ins[0].move.data), "m" (ins[1].move.data));
}

static inline void CopInsSet16(CopInsT *ins, u_short data) {
  ins->move.data = data;
}

/* High-level functions */
CopInsT *CopLoadPal(CopListT *list, const PaletteT *palette, u_short start);
CopInsT *CopLoadColor(CopListT *list, u_short start, u_short end, u_short color);

void CopSetupMode(CopListT *list, u_short mode, u_short depth);
void CopSetupDisplayWindow(CopListT *list, u_short mode, 
                           u_short xs, u_short ys, u_short w, u_short h);
void CopSetupBitplaneFetch(CopListT *list, u_short mode,
                           u_short xs, u_short w);
void CopSetupBitplanes(CopListT *list, CopInsT **bplptr,
                       const BitmapT *bitmap, u_short depth);
void CopSetupBitplaneArea(CopListT *list, u_short mode, u_short depth,
                          const BitmapT *bitmap, short x, short y,
                          const Area2D *area);
void CopUpdateBitplanes(CopInsT **bplptr, const BitmapT *bitmap, short n);
void CopSetupDualPlayfield(CopListT *list, CopInsT **bplptr,
                           const BitmapT *pf1, const BitmapT *pf2);

static inline void CopSetupGfxSimple(CopListT *list, u_short mode, u_short depth,
                                     u_short xs, u_short ys, u_short w, u_short h) 
{
  CopSetupMode(list, mode, depth);
  CopSetupDisplayWindow(list, mode, xs, ys, w, h);
  CopSetupBitplaneFetch(list, mode, xs, w);
}

static inline CopInsT *CopSetColor(CopListT *list, short i, u_short value) {
  return CopMove16(list, color[i], value);
}

#endif
