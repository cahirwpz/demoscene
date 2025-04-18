#include <sprite.h>

void SpriteUpdatePos(SpriteT *spr, short hstart, short vstart) {
  u_char *raw = (u_char *)spr->sprdat;

  /*
   * SPRxPOS:
   *  Bits 15-8 contain the low 8 bits of VSTART
   *  Bits 7-0 contain the high 8 bits of HSTART
   */

  *raw++ = vstart;
  *raw++ = (u_short)hstart >> 1;
 
  /*
   * SPRxCTL:
   *  Bits 15-8       The low eight bits of VSTOP
   *  Bit 7           (Used in attachment)
   *  Bits 6-3        Unused (make zero)
   *  Bit 2           The VSTART high bit
   *  Bit 1           The VSTOP high bit
   *  Bit 0           The HSTART low bit
   */

  {
    u_short vstop = vstart + spr->height;
    u_char lowctl = hstart & 1;
    
    *raw++ = vstop;

    if (spr->attached)
      lowctl += 0x80;
    if (vstart & 0x100)
      lowctl += 4;
    if (vstop & 0x100)
      lowctl += 2;
    *raw++ = lowctl;
  }
}
