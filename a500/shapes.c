#include "startup.h"
#include "2d.h"
#include "bltop.h"
#include "coplist.h"
#include "fx.h"
#include "memory.h"
#include "ilbm.h"

STRPTR __cwdpath = "data";

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  5

static ShapeT *shape;
static PaletteT *palette;
static BitmapT *screen;
static CopInsT *bplptr[5];
static CopListT *cp;
static WORD plane, planeC;

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
  shape = LoadShape("boxes.2d");
  palette = LoadPalette("boxes-pal.ilbm");
}

static void UnLoad() {
  DeleteShape(shape);
  DeleteBitmap(screen);
  DeletePalette(palette);
}

static void Init() {
  /* Set up clipping window. */
  ClipWin.minX = fx4i(0);
  ClipWin.maxX = fx4i(319);
  ClipWin.minY = fx4i(0);
  ClipWin.maxY = fx4i(255);

  plane = DEPTH - 1;
  planeC = 0;

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;
  BitmapClear(screen, DEPTH);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen, DEPTH);
  CopLoadPal(cp, palette, 0);
  CopEnd(cp);

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER;

  DeleteCopList(cp);
}

static Point2D tmpPoint[2][16];

static inline void DrawPolygon(Point2D *out, WORD n) {
  WORD *pos = (WORD *)out;
  WORD x1, y1, x2, y2;

  x1 = *pos++ >> 4;
  y1 = *pos++ >> 4;
  n--;

  while (--n >= 0) {
    x2 = *pos++ >> 4;
    y2 = *pos++ >> 4;
    BlitterLine(x1, y1, x2, y2);
    x1 = x2; y1 = y2;
  }
}

static __regargs void DrawShape(ShapeT *shape) {
  IndexListT **polygons = shape->polygon;
  Point2D *point = shape->viewPoint;
  UBYTE *flags = shape->viewPointFlags;
  WORD n = shape->polygons;

  while (--n >= 0) {
    UBYTE clipFlags = 0;
    UBYTE outside = 0xff;

    Point2D *out = tmpPoint[0];
    WORD *vertex = (WORD *)*polygons++;
    WORD n = (*vertex++) + 1;
    WORD m = n;

    while (--m >= 0) {
      WORD k = *vertex++;
      clipFlags |= flags[k];
      outside &= flags[k];
      *out++ = point[k];
    }

    if (!outside) {
      Point2D *out = tmpPoint[1];
      n = ClipPolygon2D(tmpPoint[0], &out, n, clipFlags);
      DrawPolygon(out, n);
    }
  }
}

static void Render() {
  // LONG lines = ReadLineCounter();
  WORD i, a = frameCount * 64;
  Matrix2D t;

  BlitterClear(screen, plane);
  LoadIdentity2D(&t);
  Rotate2D(&t, frameCount * 8);
  Scale2D(&t, fx12f(1.0) + SIN(a) / 2, fx12f(1.0) + COS(a) / 2);
  Translate2D(&t, fx4i(screen->width / 2), fx4i(screen->height / 2));
  Transform2D(&t, shape->viewPoint, shape->origPoint, shape->points);
  PointsInsideBox(shape->viewPoint, shape->viewPointFlags, shape->points);
  BlitterLineSetup(screen, plane, LINE_EOR, LINE_ONEDOT);
  DrawShape(shape);
  BlitterFill(screen, plane);
  WaitBlitter();
  // Log("shape: %ld\n", ReadLineCounter() - lines);
  WaitVBlank();

  for (i = 0; i < DEPTH; i++) {
    WORD j = (plane + i) % DEPTH;
    CopInsSet32(bplptr[i], screen->planes[j]);
  }

  if (planeC & 1)
    plane = (plane + 1) % DEPTH;

  planeC ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
