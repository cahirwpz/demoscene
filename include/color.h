#ifndef __COLOR_H__
#define __COLOR_H__

#include "types.h"

extern u_char colortab[4096];

/* Each argument must be in range 0-15. */
u_short ColorTransition(u_short from, u_short to, u_short step);

#endif
