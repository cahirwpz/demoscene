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

#endif
