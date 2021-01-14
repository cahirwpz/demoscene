#include <copper.h>

/* Arguments must be always specified in low resolution coordinates. */
void CopSetupDisplayWindow(CopListT *list, u_short mode, 
                           u_short xs, u_short ys, u_short w, u_short h)
{
  /* vstart  $00 ..  $ff */
  /* hstart  $00 ..  $ff */
  /* vstop   $80 .. $17f */
  /* hstop  $100 .. $1ff */
  u_char xe, ye;

  if (mode & MODE_HIRES)
    w >>= 1;
  if (mode & MODE_LACE)
    h >>= 1;

  xe = xs + w;
  ye = ys + h;

  CopMove16(list, diwstrt, (ys << 8) | xs);
  CopMove16(list, diwstop, (ye << 8) | xe);
}
