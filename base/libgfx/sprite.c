#include <graphics/gfxbase.h>
#include <proto/graphics.h>
#include <proto/exec.h>

#include "sprite.h"
#include "memory.h"
#include "common.h"

__regargs SpriteT *NewSprite(u_short height, bool attached) {
  SpriteT *sprite = MemAlloc(sizeof(SpriteT), MEMF_PUBLIC|MEMF_CLEAR);

  sprite->height = height;
  sprite->data = MemAlloc((height ? (height + 2) : 1) * 4, MEMF_CHIP|MEMF_CLEAR);

  if (attached)
    sprite->attached = NewSprite(height, false);

  return sprite;
}

__regargs SpriteT *CloneSystemPointer() {
  struct SimpleSprite *sprite = GfxBase->SimpleSprites[0];
  u_short height = sprite->height;
  SpriteT *pointer = NewSprite(height, false);

  memcpy(pointer->data + 2, sprite->posctldata + 2, height * sizeof(int));

  return pointer;
}

__regargs void DeleteSprite(SpriteT *sprite) {
  if (sprite) {
    if (sprite->attached)
      DeleteSprite(sprite->attached);

    MemFree(sprite->data);
    MemFree(sprite);
  }
}

static inline void UpdateSpriteInternal(SpriteT *sprite, u_short hstart, u_short vstart) {
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

__regargs void UpdateSprite(SpriteT *sprite, u_short hstart, u_short vstart) {
  SpriteT *attached = sprite->attached;

  UpdateSpriteInternal(sprite, hstart, vstart);

  if (attached) {
    int *dst = (int *)attached->data;
    int *src = (int *)sprite->data;

    *dst = *src | 0x80;
  }
}

__regargs void CopSetupSprites(CopListT *list, CopInsT **sprptr) {
  u_short *data = NullSprite->data;
  short i;

  for (i = 0; i < 8; i++) {
    CopInsT *ins = CopMove32(list, sprpt[i], data);
    if (sprptr)
      *sprptr++ = ins;
  }
}

__regargs void CopSetupManualSprites(CopListT *list, CopInsT **sprptr) {
  u_short *data = NullSprite->data;
  short i;

  for (i = 0; i < 8; i++) {
    sprptr[i] = CopMove16(list, spr[i].pos, data[0]);
    CopMove16(list, spr[i].ctl, data[1]);
    CopMove32(list, sprpt[i], data + 2);
  }
}

static SpriteT NullSpriteData = { (SpriteT *)NULL, 0, (u_short *)NULL };
SpriteT *NullSprite = &NullSpriteData;

void InitNullSprite(void) {
  Log("[Init] Null sprite.\n");
  NullSprite->data = AllocMem(4, MEMF_CHIP|MEMF_CLEAR);
}

void KillNullSprite(void) {
  Log("[Quit] Null sprite.\n");
  FreeMem(NullSprite->data, 4);
}

ADD2INIT(InitNullSprite, 0);
ADD2EXIT(KillNullSprite, 0);
