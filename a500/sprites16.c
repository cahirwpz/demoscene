#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "ilbm.h"
#include "sprite.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 1

static BitmapT *screen;
static BitmapT *bitmap;
static CopListT *cp;
static SpriteT *nullspr;
static SpriteT *sprite[12];
static CopInsT *sprptr[8];

static void Load() {
  WORD i;

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  bitmap = LoadILBM("data/sprites16.ilbm");
  nullspr = NewSprite(0, FALSE);
  cp = NewCopList(100);

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetRGB(cp, 0, 0x346);
  CopLoadPal(cp, bitmap->palette, 16);
  CopMakeSprites(cp, sprptr, nullspr);
  CopEnd(cp);

  for (i = 0; i < 12; i++) {
    sprite[i] = NewSpriteFromBitmap(20, bitmap, 0, 22 * i + 1);
    UpdateSpritePos(sprite[i], X(0), Y(128));
  }
}

static void UnLoad() {
  WORD i;

  DeleteSprite(nullspr);

  for (i = 0; i < 12; i++)
    DeleteSprite(sprite[i]);

  DeleteCopList(cp);
  DeletePalette(bitmap->palette);
  DeleteBitmap(bitmap);
  DeleteBitmap(screen);
}

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

  if (sprptr[0] && sprptr[1]) {
    CopInsSet32(sprptr[0], sprite[i]->data);
    CopInsSet32(sprptr[1], sprite[i]->attached->data);
  }

  counter++;
}

static void Init() {
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE;
}

static void Render() {
  WaitLine(Y(200));
  MoveSprite();
  WaitVBlank();
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
