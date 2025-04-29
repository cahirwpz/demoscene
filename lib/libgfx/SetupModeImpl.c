#ifdef __COPPER_H__
#define STORE(reg, val) CopMove16(list, reg, val)

void CopSetupMode(CopListT *list, u_short mode, u_short depth)
#else
#define STORE(reg, val) custom->reg = val

void SetupMode(u_short mode, u_short depth)
#endif
{
  STORE(bplcon0, BPLCON0_BPU(depth) | BPLCON0_COLOR | mode);
  /* bplcon1 is set up by SetupBitplaneFetch */
  /* default video priority: SP07 > PF1 > PF2 */
  STORE(bplcon2, BPLCON2_PF1P_BOTTOM | BPLCON2_PF2P_BOTTOM);
  /* for compatibility with AGA */
  STORE(bplcon3, BPLCON3_PF2OF(3) | BPLCON3_SPRES(0));
}
