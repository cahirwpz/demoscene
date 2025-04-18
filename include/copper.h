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
  CopInsT lo;
  CopInsT hi;
} CopInsPairT;

typedef struct {
  CopInsT *curr;
  u_short length;
  u_char  overflow; /* -1 if Vertical Position counter overflowed */
  u_char  finished; /* -1 if correctly terminated */
  CopInsT entry[0]; 
} CopListT;

/* @brief Returns a new copper list of `length` entries ready to be used. */
CopListT *NewCopList(int length);

/* @brief Reclaim memory used by the copper list. */
void DeleteCopList(CopListT *list);

/* @brief Reuse existing copper list by resetting to initial state. */
CopListT *CopListReset(CopListT *list);

/* @brief Finish off copper list by inserting special WAIT instruction. */
CopListT *CopListFinish(CopListT *list);

/* @brief Enable copper and activate copper list.
 * @warning This function busy-waits for vertical blank. */
void CopListActivate(CopListT *list);

/* @brief Stop executing current copper list immediately.
 * @note Disables all systems that depend on being refreshed by Copper
 *       on frame-by-frame basis (bitplanes & sprites). */
void CopperStop(void);

/* @brief Set up copper list to start after vertical blank. */
static inline void CopListRun(CopListT *list) {
  custom->cop1lc = (u_int)list->entry;
}

/* @brief Return a pointer to the current copper instruction pointer. */
static inline void *CopInsPtr(CopListT *list) {
  return list->curr;
}

/* Low-level functions */
#define CSREG(reg) offsetof(struct Custom, reg)
#define CopInsMove16(ins, reg, data) \
  ins = _CopInsMove16(ins, CSREG(reg), (data))
#define CopInsMove32(ins, reg, data) \
  ins = _CopInsMove32(ins, CSREG(reg), (int)(data))

static inline CopInsT *_CopInsMove16(CopInsT *ins, short reg, short data) {
  stwi(ins, reg);
  stwi(ins, data);
  return ins;
}

static inline CopInsPairT *_CopInsMove32(CopInsT *ins, short reg, int data) {
  stwi(ins, reg + 2);
  stwi(ins, data);
  stwi(ins, reg);
  stwi(ins, swap16(data));
  return (CopInsPairT *)ins;
}

static inline void CopInsSet16(CopInsT *ins, short data) {
  ins->move.data = data;
}

static inline void CopInsSet32(CopInsPairT *ins, void *data) {
  asm volatile("movew %0,%2\n"
               "swap  %0\n"
               "movew %0,%1\n"
               : "+d" (data)
               : "m" (ins->hi.move.data), "m" (ins->lo.move.data));
}

#define CopMove16(cp, reg, data) _CopMove16((cp), CSREG(reg), (data))
#define CopMove32(cp, reg, data) _CopMove32((cp), CSREG(reg), (int)(data))

static inline CopInsT *_CopMove16(CopListT *list, short reg, short data) {
  CopInsT *pos = list->curr;
  list->curr = _CopInsMove16(list->curr, reg, data);
  return pos;
}

static inline CopInsPairT *_CopMove32(CopListT *list, short reg, int data) {
  CopInsPairT *pos = (CopInsPairT *)list->curr;
  list->curr = (CopInsT *)_CopInsMove32(list->curr, reg, data);
  return pos;
}

/* Official way to represent no-op copper instruction. */
#define CopNoOp(cp) _CopMove16(cp, 0x1FE, 0)

/* Wait for raster beam position to be greater or equal to (vp, hp). */
#define CopInsWait(ins, vp, hp) \
  ins = _CopInsWait(ins, vp, hp)

static inline CopInsT *_CopInsWait(CopInsT *ins, short vp, short hp) {
  stbi(ins, vp);
  stbi(ins, hp | 1);
  stwi(ins, 0xfffe);
  return ins;
}

static inline CopInsT *CopWait(CopListT *list, short vp, short hp) {
  CopInsT *pos = list->curr;
  CopInsWait(list->curr, vp, hp);
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
    stli(ins, 0xffdffffe);
  }
  stbi(ins, vp);
  stbi(ins, hp | 1);
  stwi(ins, 0xfffe);
  list->curr = ins;
  return pos;
}

/* Similar to CopWait, but masks bits in beam position counters. */
static inline CopInsT *CopWaitMask(CopListT *list, short vp, short hp,
                                   short vpmask, short hpmask) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  stbi(ins, vp);
  stbi(ins, hp | 1);
  stbi(ins, 0x80 | vpmask);
  stbi(ins, hpmask & 0xfe);
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
  stbi(ins, vp);
  stbi(ins, hp | 1);
  stwi(ins, 0xffff);
  list->curr = ins;
  return pos;
}

/* Similar to CopSkip, but masks bits in beam position counters. */
static inline CopInsT *CopSkipMask(CopListT *list, short vp, short hp, 
                                   short vpmask, short hpmask) {
  CopInsT *pos = list->curr;
  CopInsT *ins = list->curr;
  stbi(ins, vp);
  stbi(ins, hp | 1);
  stbi(ins, 0x80 | vpmask);
  stbi(ins, hpmask | 1);
  list->curr = ins;
  return pos;
}

#define CopSkipH(cp, vp, hp) CopSkipMask((cp), (vp) & 128, (hp), 0, 255)
#define CopSkipV(cp, vp) CopSkipMask((cp), (vp), 0, 255, 0)

/* High-level functions */
CopInsT *CopLoadColor(CopListT *list, short start, short end, short color);

/* Load `ncols` colors from array `col`. A range of color register will be set
 * starting from `start` ending at `start + ncols - 1`. */
CopInsT *CopLoadColorArray(CopListT *list, const u_short *colors, short count,
                           int start);

#define CopLoadColors(list, colors, start) \
  CopLoadColorArray((list), (colors), nitems(colors), (start))

void CopSetupMode(CopListT *list, u_short mode, u_short depth);
/* Arguments must be always specified in low resolution coordinates. */
void CopSetupDisplayWindow(CopListT *list, u_short mode, 
                           u_short xs, u_short ys, u_short w, u_short h);
void CopSetupBitplaneFetch(CopListT *list, u_short mode,
                           u_short xs, u_short w);
CopInsPairT *CopSetupBitplanes(CopListT *list, const BitmapT *bitmap,
                               u_short depth);
void CopSetupBitplaneArea(CopListT *list, u_short mode, u_short depth,
                          const BitmapT *bitmap, short x, short y,
                          const Area2D *area);
void CopUpdateBitplanes(CopInsPairT *bplptr, const BitmapT *bitmap, short n);

static inline CopInsT *CopSetColor(CopListT *list, short i, short value) {
  return CopMove16(list, color[i], value);
}

#endif
