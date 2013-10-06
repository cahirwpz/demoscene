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
static SpriteT *sprite[12];

void Load() {
  WORD i;

  cp = NewCopList(100);
  screen = NewBitmap(320, 256, 1, FALSE);
  bitmap = LoadILBM("data/sprites16.ilbm", TRUE);

  for (i = 0; i < 12; i++) {
    sprite[i] = NewSpriteFromBitmap(20, bitmap, 0, 22 * i + 1);
    UpdateSpritePos(sprite[i], X(0), Y(128));
  }

  nullspr = NewSprite(1, FALSE);
  UpdateSpritePos(nullspr, X(0), Y(-10));
}

void Kill() {
  WORD i;

  DeleteSprite(nullspr);

  for (i = 0; i < 12; i++)
    DeleteSprite(sprite[i]);

  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
  DeleteBitmap(screen);
}

static CopInsT *spr0ptr = NULL;
static CopInsT *spr1ptr = NULL;

static UWORD move[] = { 0, 1, 0, 2, 0 };

static void MoveSprite() {
  static UWORD counter = 0;
  static UWORD x = X(0);
  static UWORD y = Y(0);
  static WORD dx = 1;
  static WORD dy = 1;
  UWORD i;

  if (x >= X(304))
    dx = -1;
  if (x <= X(0))
    dx = 1;
  if (y >= Y(236))
    dy = -1;
  if (y <= Y(0))
    dy = 1;

  x += dx;
  y += dy;

  i = move[(counter >> 4) & 3];

  if (dx < 0)
    i += 3;
  else
    i += 9;

  UpdateSpritePos(sprite[i], x, y);

  /* why the line below resolves a bug with shadow sprite ? */
  if (i < 11)
    UpdateSpritePos(sprite[i+1], 0, Y(-1));

  if (spr0ptr) {
    CopInsSet32(spr0ptr, sprite[i]->data);
    CopInsSet32(spr1ptr, sprite[i]->attached->data);
  }

  counter++;
}

void Main() {
  CopInit(cp);

  CopMove16(cp, fmode, 0);
  CopMove16(cp, bplcon0, BPLCON0_BPU(screen->depth) | BPLCON0_COLOR);
  CopMove16(cp, bplcon1, 0);
  CopMove16(cp, bplcon2, 0x24);
  CopMove32(cp, bplpt[0], screen->planes[0]);

  CopMove16(cp, ddfstrt, 0x38);
  CopMove16(cp, ddfstop, 0xd0);

  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopMove16(cp, color[0], 0x346);
  CopLoadPal(cp, bitmap->palette, 16);

  {
    UWORD i;

    spr0ptr = CopMove32(cp, sprpt[0], NULL);
    spr1ptr = CopMove32(cp, sprpt[1], NULL);

    for (i = 2; i < 8; i++)
      CopMove32(cp, sprpt[i], nullspr->data);
  }

  CopEnd(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE | DMAF_MASTER;

  while (!LeftMouseButton()) {
    WaitLine(Y(200));
    MoveSprite();
    WaitVBlank();
  }
}
