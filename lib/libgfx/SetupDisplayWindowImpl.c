#ifdef __COPPER_H__
#define STORE(reg, val) CopMove16(list, reg, val)

void CopSetupDisplayWindow(CopListT *list, u_short mode, 
                           u_short xs, u_short ys, u_short w, u_short h)
#else
#define STORE(reg, val) custom->reg = val

void SetupDisplayWindow(u_short mode, u_short xs, u_short ys,
                        u_short w, u_short h)
#endif
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

  STORE(diwstrt, (ys << 8) | xs);
  STORE(diwstop, (ye << 8) | xe);
}
