#include <color.h>

/* Each argument must be in range 0-15. */
u_short ColorTransition(u_short from, u_short to, u_short step) {
  short r = (from & 0xf00) | ((to >> 4) & 0x0f0) | step;
  short g = ((from << 4) & 0xf00) | (to & 0x0f0) | step;
  short b = ((from << 8) & 0xf00) | ((to << 4) & 0x0f0) | step;
  
  return (colortab[r] << 4) | colortab[g] | (colortab[b] >> 4);
}
