#include <exec/memory.h>
#include <proto/exec.h>

#include "sprite.h"

__regargs SpriteT *NewSprite(UWORD height) {
  SpriteT *sprite = AllocMem(sizeof(SpriteT), MEMF_PUBLIC|MEMF_CLEAR);

  sprite->height = height;
  sprite->data = AllocMem((height + 2) * 4, MEMF_CHIP|MEMF_CLEAR);

  return sprite;
}

__regargs SpriteT *NewSpriteFromBitmap(UWORD height, BitmapT *bitmap,
                                       UWORD xstart, UWORD ystart)
{
  SpriteT *sprite = NewSprite(height);
  UWORD *bpl0 = (UWORD *)bitmap->planes[0] + ystart * 2;
  UWORD *bpl1 = (UWORD *)bitmap->planes[1] + ystart * 2;
  UWORD *data = &sprite->data[2];
  WORD yend = ystart + sprite->height;

  for (; ystart < yend; ystart++) {
    *data++ = *bpl0;
    *data++ = *bpl1;

    bpl0 += 2;
    bpl1 += 2;
  }

  return sprite;
}

__regargs void DeleteSprite(SpriteT *sprite) {
  FreeMem(sprite->data, (sprite->height + 2) * 4);
  FreeMem(sprite, sizeof(SpriteT));
}

__regargs void UpdateSprite(SpriteT *sprite) {
  UWORD hstart = sprite->x;
  UWORD vstart = sprite->y;
  UWORD vstop = vstart + sprite->height + 1;

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

  sprite->data[0] = (vstart << 8) | ((hstart >> 1) & 255);
  sprite->data[1] = (vstop << 8) | ((vstart >> 6) & 4) | ((vstop >> 7) & 2) | (hstart & 1);
}
