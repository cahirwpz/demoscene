#ifndef __COLOR_H__
#define __COLOR_H__

#include <exec/types.h>

extern UBYTE colortab[4096];

/* Each argument must be in range 0-15. */
static inline UWORD ColorTransition(UWORD from, UWORD to, UWORD step) {
  WORD r = (from & 0xf00) | ((to & 0xf00) >> 4) | step;
  WORD g = ((from & 0x0f0) << 4) | (to & 0x0f0) | step;
  WORD b = ((from & 0x00f) << 8) | ((to & 0x00f) << 4) | step;
  
  return (colortab[r] << 4) | colortab[g] | (colortab[b] >> 4);
}

static inline UWORD ColorTransitionRGB(WORD sr, WORD sg, WORD sb, WORD dr, WORD dg, WORD db, UWORD step) {
  WORD r = ((sr & 0xf0) << 4) | (dr & 0xf0) | step;
  WORD g = ((sg & 0xf0) << 4) | (dg & 0xf0) | step;
  WORD b = ((sb & 0xf0) << 4) | (db & 0xf0) | step;
  
  return (colortab[r] << 4) | colortab[g] | (colortab[b] >> 4);
}

static inline UWORD ColorIncreaseContrastRGB(WORD r, WORD g, WORD b, UWORD step) {
  r &= 0xf0;
  r += colortab[(WORD)(r | step)];
  if (r > 0xf0) r = 0xf0;

  g &= 0xf0;
  g += colortab[(WORD)(g | step)];
  if (g > 0xf0) g = 0xf0;

  b &= 0xf0;
  b += colortab[(WORD)(b | step)];
  if (b > 0xf0) b = 0xf0;

  return (r << 4) | g | (b >> 4);
}

static inline UWORD ColorDecreaseContrastRGB(WORD r, WORD g, WORD b, UWORD step) {
  r &= 0xf0;
  r -= colortab[(WORD)(r | step)];
  if (r < 0) r = 0;

  g &= 0xf0;
  g -= colortab[(WORD)(g | step)];
  if (g < 0) g = 0;

  b &= 0xf0;
  b -= colortab[(WORD)(b | step)];
  if (b < 0) b = 0;

  return (r << 4) | g | (b >> 4);
}

#endif
