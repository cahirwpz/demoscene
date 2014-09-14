#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "ilbm.h"
#include "sprite.h"

static BitmapT *screen;
static BitmapT *bitmap;
static CopListT *cp;
static SpriteT *nullspr;
static SpriteT *sprite[3];
static CopInsT *sprptr[8];

static void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  bitmap = LoadILBM("data/sprites4.ilbm", TRUE);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopLoadPal(cp, bitmap->palette, 16);
  CopMakeSprites(cp, sprptr, nullspr);
  CopEnd(cp);

  sprite[0] = NewSpriteFromBitmap(19, bitmap, 0, 0);
  sprite[1] = NewSpriteFromBitmap(24, bitmap, 0, 19);
  sprite[2] = NewSpriteFromBitmap(42, bitmap, 0, 43);
  UpdateSpritePos(sprite[0], X(0), Y(113));
  UpdateSpritePos(sprite[1], X(0), Y(110));
  UpdateSpritePos(sprite[2], X(0), Y(101));

  nullspr = NewSprite(0, FALSE);
}

static void UnLoad() {
  DeleteSprite(nullspr);
  DeleteSprite(sprite[0]);
  DeleteSprite(sprite[1]);
  DeleteSprite(sprite[2]);
  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
  DeleteBitmap(screen);
}

static void MoveSprite() {
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

  if (sprptr[0])
    CopInsSet32(sprptr[0], sprite[i]->data);

  counter++;
}

static void Init() {
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE;
}

static void Render() {
  WaitLine(Y(256));
  MoveSprite();
  WaitVBlank();
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
