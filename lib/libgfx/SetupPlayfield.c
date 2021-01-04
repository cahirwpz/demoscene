#include <playfield.h>

void SetupPlayfield(u_short mode, u_short depth,
                    u_short xs, u_short ys, u_short w, u_short h)
{
  custom->bplcon0 = BPLCON0_BPU(depth) | BPLCON0_COLOR | mode;
  custom->bplcon2 = BPLCON2_PF2P2 | BPLCON2_PF1P2 | BPLCON2_PF2PRI;
  custom->bplcon3 = 0;

  if (mode & MODE_HIRES)
    w >>= 1;
  if (mode & MODE_LACE)
    h >>= 1;

  {
    u_char xe = xs + w;
    u_char ye = ys + h;
    custom->diwstrt = (ys << 8) | xs;
    custom->diwstop = (ye << 8) | xe;
  }

  {
    u_short ddfstrt;

    w >>= 1;

    if (mode & MODE_HIRES) {
      xs -= 9;
      ddfstrt = (xs >> 1) & ~3; /* 4 clock resolution */
    } else {
      xs -= 17;
      ddfstrt = (xs >> 1) & ~7; /* 8 clock resolution */
    }

    /* Found in UAE source code - DDFSTRT & DDFSTOP matching for:
     * - ECS: does not require DMA or DIW enabled, 
     * - OCS: requires DMA and DIW enabled. */

    custom->ddfstrt = ddfstrt;
    custom->ddfstop = ddfstrt + w - 8;
    custom->bplcon1 = ((xs & 15) << 4) | (xs & 15);
    custom->fmode = 0;
  }
}
