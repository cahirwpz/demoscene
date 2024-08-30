#ifndef __COLOR_H__
#define __COLOR_H__

#include "types.h"

extern u_char colortab[4096];

/* Each argument must be in range 0-15. */
u_short ColorTransition(u_short from, u_short to, u_short step);

u_short HsvToRgb(short h, short s, short v);

void FadeBlack(const u_short *colors, short count, u_int start, short step);

#endif
