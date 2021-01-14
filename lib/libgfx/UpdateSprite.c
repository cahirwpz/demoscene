#include <sprite.h>

static inline void UpdateSpriteInternal(const SpriteT *sprite,
                                        u_short hstart, u_short vstart)
{
  u_short vstop = vstart + sprite->height + 1;
  u_char lowctl = hstart & 1;

  /*
   * SPRxPOS:
   *  Bits 15-8 contain the low 8 bits of VSTART
   *  Bits 7-0 contain the high 8 bits of HSTART
   *
   * SPRxCTL:
   *  Bits 15-8       The low eight bits of VSTOP
   *  Bit 7           (Used in attachment)
   *  Bits 6-3        Unused (make zero)
   *  Bit 2           The VSTART high bit
   *  Bit 1           The VSTOP high bit
   *  Bit 0           The HSTART low bit
   */

  if (vstart & 0x100)
    lowctl |= 4;
  if (vstop & 0x100)
    lowctl |= 2;

  {
    u_char *spr = (u_char *)sprite->data;

    *spr++ = vstart;
    *spr++ = hstart >> 1;
    *spr++ = vstop;
    *spr++ = lowctl;
  }
}

void UpdateSprite(const SpriteT *sprite, u_short hstart, u_short vstart) {
  UpdateSpriteInternal(sprite, hstart, vstart);

  if (sprite->attached) {
    int *dst = (int *)sprite[1].data;
    int *src = (int *)sprite[0].data;

    *dst = *src | 0x80;
  }
}
