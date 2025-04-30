#include <common.h>

#define FMODE_MASK (FMODE_BPAGEM|FMODE_BLP32|FMODE_BSCAN2)

#define BUS16 0
#define BUS32 1
#define BUS64 2

#define LORES 0
#define HIRES 1
#define SHRES 2

#ifdef __COPPER_H__
#define STORE(reg, val) CopMove16(list, reg, val)

void CopSetupBitplaneFetch(CopListT *list, u_short mode, hpos xstart, u_short w)
#else
#define STORE(reg, val) custom->reg = val

void SetupBitplaneFetch(u_short mode, hpos xstart, u_short w)
#endif
{
  u_short ddfstrt, ddfstop;
  short xs = xstart.hpos << 2; /* convert to shres */
  short fetchstart, fetchsize, prefetch, shift;

  short bus = mode & 3;
  short res = LORES;

  if (bus == 3)
    bus = BUS64;

  if (mode & MODE_SHRES) {
    res = SHRES;
  } else if (mode & MODE_HIRES) {
    res = HIRES;
    w <<= 1; /* convert from hires to shres */
  } else {
    w <<= 2; /* convert from lores to shres */
  }

  /* Method is taken from amifb.c Linux driver */
  fetchstart = 16 << (bus - res + 2);
  fetchsize = 64 << max(bus - res, 0);
  prefetch = 64 >> max(res - bus, 0);
  xs -= 4;

  ddfstrt = (xs & -fetchstart) - prefetch;
  ddfstop = ddfstrt + w - fetchsize;

  shift = (xs & (fetchstart - 1)) >> 2;

  /*
   * DDFSTRT and DDFSTOP have resolution of 4 clocks.
   *
   * Only bits 7..2 of DDFSTRT and DDFSTOP are meaningful on OCS!
   *
   * Values to determine Display Data Fetch Start and Stop must be divisible by
   * 16 pixels, because hardware fetches bitplanes in 16-bit word units.
   * Bitplane fetcher uses 4 clocks for HiRes (1 clock = 4 pixels) and 8 clocks
   * for LoRes (1 clock = 2 pixels) to fetch enough data to display it.
   */

  /* Found in UAE source code - DDFSTRT & DDFSTOP matching for:
   * - ECS: does not require DMA or DIW enabled,
   * - OCS: requires DMA and DIW enabled. */

  STORE(ddfstrt, ddfstrt >> 3);
  STORE(ddfstop, ddfstop >> 3);
  STORE(bplcon1, BPLCON1_PF2H(xs) | BPLCON1_PF1H(xs));
  STORE(fmode, fmode(mode, FMODE_BPAGEM | FMODE_BLP32 | FMODE_BSCAN2));
}
