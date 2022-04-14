#include <playfield.h>

void SetupBitplaneFetch(u_short mode, u_short xs, u_short w) {
  u_char ddfstrt, ddfstop;

  if (mode & MODE_HIRES) {
    xs -= 9;
    w >>= 2;
    ddfstrt = (xs >> 1) & ~3;
  } else {
    xs -= 17;
    w >>= 1;
    ddfstrt = (xs >> 1) & ~7;
  }

  ddfstop = ddfstrt + w - 8;

  custom->ddfstrt = ddfstrt;
  custom->ddfstop = ddfstop;
  custom->bplcon1 = ((xs & 15) << 4) | (xs & 15);
  custom->fmode = 0;
}
