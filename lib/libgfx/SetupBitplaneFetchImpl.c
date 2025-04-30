#ifdef __COPPER_H__
#define STORE(reg, val) CopMove16(list, reg, val)

void CopSetupBitplaneFetch(CopListT *list, u_short mode, hpos xstart, u_short w)
#else
#define STORE(reg, val) custom->reg = val

void SetupBitplaneFetch(u_short mode, hpos xstart, u_short w)
#endif
{
  u_char ddfstrt, ddfstop;
  short xs = xstart.hpos;

  /* DDFSTRT and DDFSTOP have resolution of 4 clocks.
   *
   * Only bits 7..2 of DDFSTRT and DDFSTOP are meaningful on OCS!
   *
   * Values to determine Display Data Fetch Start and Stop must be divisible by
   * 16 pixels, because hardware fetches bitplanes in 16-bit word units.
   * Bitplane fetcher uses 4 clocks for HiRes (1 clock = 4 pixels) and 8 clocks
   * for LoRes (1 clock = 2 pixels) to fetch enough data to display it.
   *
   * HS = Horizontal Start, W = Width (divisible by 16)
   *
   * For LoRes: DDFSTART = HS / 2 - 8.5, DDFSTOP = DDFSTRT + W / 2 - 8
   * For HiRes: DDFSTART = HS / 2 - 4.5, DDFSTOP = DDFSTRT + W / 4 - 8 */
 
  if (mode & MODE_HIRES) {
    xs -= 9;
    w >>= 2;
    ddfstrt = (xs >> 1) & ~3; /* 4 clock resolution */
  } else {
    xs -= 17;
    w >>= 1;
    ddfstrt = (xs >> 1) & ~7; /* 8 clock resolution */
  }

  ddfstop = ddfstrt + w - 8;

  /* Found in UAE source code - DDFSTRT & DDFSTOP matching for:
   * - ECS: does not require DMA or DIW enabled,
   * - OCS: requires DMA and DIW enabled. */

  STORE(ddfstrt, ddfstrt);
  STORE(ddfstop, ddfstop);
  STORE(bplcon1, BPLCON1_PF2H(xs) | BPLCON1_PF1H(xs));
  STORE(fmode, fmode(0, FMODE_BPAGEM | FMODE_BLP32));
}
