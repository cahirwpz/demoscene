#ifndef __COPPER_H__
#define __COPPER_H__

#include <gfx.h>
#include <playfield.h>

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

typedef struct {
  CopInsT *curr;
  u_short length;
  u_char  overflow; /* -1 if Vertical Position counter overflowed */
  CopInsT entry[0]; 
} CopListT;

CopListT *NewCopList(u_short length);
void DeleteCopList(CopListT *list);

static inline void CopInit(CopListT *list) {
  list->curr = list->entry;
  list->overflow = 0;
}

static inline void CopEnd(CopListT *list) {
  CopInsT *ins = list->curr;
  *((u_int *)ins)++ = 0xfffffffe;
  list->curr = ins;
}

/* @brief Enable copper and activate copper list.
 * @warning This function busy-waits for vertical blank. */
void CopListActivate(CopListT *list);

/* @brief Set up copper list to start after vertical blank. */
static inline void CopListRun(CopListT *list) {
  custom->cop1lc = (u_int)list->entry;
}

/* Low-level functions */
#define CSREG(reg) offsetof(struct Custom, reg)
#define CopMove16(cp, reg, data) CopMoveWord((cp), CSREG(reg), (data))
#define CopMove32(cp, reg, data) CopMoveLong((cp), CSREG(reg), (int)(data))

static inline CopInsT *CopMoveWord(CopListT *list, short reg, short data) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  *((u_short *)ins)++ = reg;
  *((u_short *)ins)++ = data;
  list->curr = ins;
  return pos;
}

static inline void CopInsSet16(CopInsT *ins, short data) {
  ins->move.data = data;
}

static inline CopInsT *CopMoveLong(CopListT *list, short reg, int data) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  *((u_short *)ins)++ = reg + 2;
  *((u_short *)ins)++ = data;
  *((u_short *)ins)++ = reg;
  *((u_short *)ins)++ = swap16(data);
  list->curr = ins;
  return pos;
}

static inline void CopInsSet32(CopInsT *ins, void *data) {
  asm volatile("movew %0,%2\n"
               "swap  %0\n"
               "movew %0,%1\n"
               : "+d" (data)
               : "m" (ins[1].move.data), "m" (ins[0].move.data));
}

/* Official way to represent no-op copper instruction. */
#define CopNoOp(cp) CopMoveWord(cp, 0x1FE, 0)

/* Wait for raster beam position to be greater or equal to (vp, hp). */
static inline CopInsT *CopWait(CopListT *list, short vp, short hp) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  *((u_char *)ins)++ = vp;
  *((u_char *)ins)++ = hp | 1;
  *((u_short *)ins)++ = 0xfffe;
  list->curr = ins;
  return pos;
}

/* Handles Copper Vertical Position counter overflow, by inserting CopWaitEOL
 * at first WAIT instruction with VP >= 256. */
static inline CopInsT *CopWaitSafe(CopListT *list, short vp, short hp) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  if (vp > 255 && !list->overflow) {
    list->overflow = -1;
    /* Wait for last waitable position to control when overflow occurs. */
    *((u_int *)ins)++ = 0xffdffffe;
  }
  *((u_char *)ins)++ = vp;
  *((u_char *)ins)++ = hp | 1;
  *((u_short *)ins)++ = 0xfffe;
  list->curr = ins;
  return pos;
}

/* Similar to CopWait, but masks bits in beam position counters. */
static inline CopInsT *CopWaitMask(CopListT *list, short vp, short hp,
                                   short vpmask, short hpmask) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  *((u_char *)ins)++ = vp;
  *((u_char *)ins)++ = hp | 1;
  *((u_char *)ins)++ = 0x80 | vpmask;
  *((u_char *)ins)++ = hpmask & 0xfe;
  list->curr = ins;
  return pos;
}

/* The most significant bit of vertical position cannot be masked out (overlaps
 * with blitter-finished-disable bit), so we have to pass upper bit as well. */
#define CopWaitH(cp, vp, hp) CopWaitMask((cp), (vp) & 128, (hp), 0, 255)
#define CopWaitV(cp, vp) CopWaitMask((cp), (vp), 0, 255, 0)

/* Skip next instruction if the video beam has already reached a specified
 * (vp, hp) position. */
static inline CopInsT *CopSkip(CopListT *list, short vp, short hp) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  *((u_char *)ins)++ = vp;
  *((u_char *)ins)++ = hp | 1;
  *((u_short *)ins)++ = 0xffff;
  list->curr = ins;
  return pos;
}

/* Similar to CopSkip, but masks bits in beam position counters. */
static inline CopInsT *CopSkipMask(CopListT *list, short vp, short hp, 
                                   short vpmask, short hpmask) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  *((u_char *)ins)++ = vp;
  *((u_char *)ins)++ = hp | 1;
  *((u_char *)ins)++ = 0x80 | vpmask;
  *((u_char *)ins)++ = hpmask | 1;
  list->curr = ins;
  return pos;
}

#define CopSkipH(cp, vp, hp) CopSkipMask((cp), (vp) & 128, (hp), 0, 255)
#define CopSkipV(cp, vp) CopSkipMask((cp), (vp), 0, 255, 0)

/* High-level functions */
CopInsT *CopLoadPal(CopListT *list, const PaletteT *palette, short start);
CopInsT *CopLoadColor(CopListT *list, short start, short end, short color);

void CopSetupMode(CopListT *list, u_short mode, u_short depth);
/* Arguments must be always specified in low resolution coordinates. */
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

static inline CopInsT *CopSetColor(CopListT *list, short i, short value) {
  return CopMove16(list, color[i], value);
}

#endif
