#ifndef __COLOR_H__
#define __COLOR_H__

#include "types.h"

extern u_char colortab[4096];

/* Each argument must be in range 0-15. */
__regargs u_short ColorTransition(u_short from, u_short to, u_short step);
u_short ColorTransitionRGB(short sr, short sg, short sb, short dr, short dg, short db, u_short step);
__regargs u_short ColorIncreaseContrastRGB(short r, short g, short b, u_short step);
__regargs u_short ColorDecreaseContrastRGB(short r, short g, short b, u_short step);

#endif
