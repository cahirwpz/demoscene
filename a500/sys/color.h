#ifndef __COLOR_H__
#define __COLOR_H__

#include <exec/types.h>

extern UBYTE colortab[4096];

/* Each argument must be in range 0-15. */
static inline UWORD ColorTransition(UWORD from, UWORD to, UWORD step) {
  WORD r = (from & 0xf00) | ((to & 0xf00) >> 4) | step;
  WORD g = ((from & 0x0f0) << 4) | (to & 0x0f0) | step;
  WORD b = ((from & 0x00f) << 8) | ((to & 0x00f) << 4) | step;
  
  return (colortab[r] << 8) | (colortab[g] << 4) | colortab[b];
}

static inline UWORD ColorTransitionRGB(UBYTE sr, UBYTE sg, UBYTE sb, UBYTE dr, UBYTE dg, UBYTE db, UWORD step) {
  WORD r = ((sr & 0xf0) << 4) | (dr & 0xf0) | step;
  WORD g = ((sg & 0xf0) << 4) | (dg & 0xf0) | step;
  WORD b = ((sb & 0xf0) << 4) | (db & 0xf0) | step;
  
  return (colortab[r] << 8) | (colortab[g] << 4) | colortab[b];
}

static inline UWORD ColorIntensifyRGB(UBYTE dr, UBYTE dg, UBYTE db, UWORD step) {
  WORD r = colortab[(dr & 0xf0) | step];
  WORD g = colortab[(dg & 0xf0) | step];
  WORD b = colortab[(db & 0xf0) | step];

  r += (dr >> 4);
  g += (dg >> 4);
  b += (db >> 4);

  if (r < 0) r = 0;
  if (r > 15) r = 15;
  if (g < 0) g = 0;
  if (g > 15) g = 15;
  if (b < 0) b = 0;
  if (b > 15) b = 15;
  
  return (r << 8) | (g << 4) | b;
}

#endif
