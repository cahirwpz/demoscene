#include <playfield.h>

/* Arguments must be always specified in low resolution coordinates. */
void SetupDisplayWindow(u_short mode, u_short xs, u_short ys,
                        u_short w, u_short h)
{
  u_char xe, ye;

  if (mode & MODE_HIRES)
    w >>= 1;
  if (mode & MODE_LACE)
    h >>= 1;

  xe = xs + w;
  ye = ys + h;

  custom->diwstrt = (ys << 8) | xs;
  custom->diwstop = (ye << 8) | xe;
}
