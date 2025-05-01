#include <playfield.h>

void SetupPlayfield(u_short mode, u_short depth,
                    hpos xs, vpos ys, u_short w, u_short h)
{
  SetupMode(mode, depth);
  SetupBitplaneFetch(mode, xs, w);
  SetupDisplayWindow(mode, xs, ys, w, h);
}
