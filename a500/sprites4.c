#include "hardware.h"
#include "interrupts.h"
#include "coplist.h"
#include "ilbm.h"
#include "print.h"
#include "sprite.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

static BitmapT *screen;
static BitmapT *bitmap;
static CopListT *cp;
static SpriteT *nullspr;
static SpriteT *sprite[3];

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  bitmap = LoadILBM("data/sprites4.ilbm", TRUE);
  cp = NewCopList(100);

  sprite[0] = NewSpriteFromBitmap(19, bitmap, 0, 0);
  sprite[1] = NewSpriteFromBitmap(24, bitmap, 0, 19);
  sprite[2] = NewSpriteFromBitmap(42, bitmap, 0, 43);
  UpdateSpritePos(sprite[0], X(0), Y(113));
  UpdateSpritePos(sprite[1], X(0), Y(110));
  UpdateSpritePos(sprite[2], X(0), Y(101));

  nullspr = NewSprite(1, FALSE);
  UpdateSpritePos(nullspr, X(0), Y(0));
}

void Kill() {
  DeleteSprite(nullspr);
  DeleteSprite(sprite[0]);
  DeleteSprite(sprite[1]);
  DeleteSprite(sprite[2]);
  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
  DeleteBitmap(screen);
}

static CopInsT *sprptr;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    static UWORD counter = 0;
    static UWORD x = X(0);
    static WORD direction = 1;
    WORD i = (WORD)(counter * 2 / 50) % 4;

    if (x >= X(304))
      direction = -1;
    if (x <= X(0))
      direction = 1;
    x += direction;

    if (i > 2)
      i = 4 - i;

    sprite[i]->x = x;
    UpdateSprite(sprite[i]);

    if (sprptr) {
      sprptr[0].move.data = (ULONG)sprite[i]->data >> 16;
      sprptr[1].move.data = (ULONG)sprite[i]->data;
    }

    counter++;
  }

  /*
   * Clear interrupt flags for this level to avoid infinite re-entering
   * interrupt handler.
   */
  custom->intreq = INTF_LEVEL3;
}

void Main() {
  APTR OldIntLevel3;

  /* Our vertical blank interrupt handler. */
  OldIntLevel3 = InterruptVector->IntLevel3;
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_LEVEL3 | INTF_INTEN;

  CopInit(cp);

  CopMove16(cp, bplcon0, BPLCON0_BPU(screen->depth) | BPLCON0_COLOR);
  CopMove16(cp, bplcon1, 0);
  CopMove16(cp, bplcon2, 0x24);
  CopMove32(cp, bplpt[0], screen->planes[0]);

  CopMove16(cp, ddfstrt, 0x38);
  CopMove16(cp, ddfstop, 0xd0);

  {
    UWORD i;

    sprptr = CopMove32(cp, sprpt[0], sprite[0]->data);

    for (i = 1; i < 8; i++)
      CopMove32(cp, sprpt[i], nullspr->data);
  }

  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopLoadPal(cp, bitmap->palette, 16);

  CopEnd(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE | DMAF_MASTER;

  WaitMouse();

  /* Restore original vertical blank handler. */
  custom->intena = INTF_LEVEL3;
  InterruptVector->IntLevel3 = OldIntLevel3;
}
