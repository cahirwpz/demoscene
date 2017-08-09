#ifndef __COLOR_H__
#define __COLOR_H__

#include <exec/types.h>

extern UBYTE colortab[4096];

/* Each argument must be in range 0-15. */
__regargs UWORD ColorTransition(UWORD from, UWORD to, UWORD step);
UWORD ColorTransitionRGB(WORD sr, WORD sg, WORD sb, WORD dr, WORD dg, WORD db, UWORD step);
__regargs UWORD ColorIncreaseContrastRGB(WORD r, WORD g, WORD b, UWORD step);
__regargs UWORD ColorDecreaseContrastRGB(WORD r, WORD g, WORD b, UWORD step);

#endif
