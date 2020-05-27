#include "effect.h"
#include "2d.h"
#include "blitter.h"
#include "copper.h"
#include "fx.h"
#include "memory.h"

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  4

static BitmapT *screen;
static CopInsT *bplptr[DEPTH];
static CopListT *cp;
static short plane, planeC;

#include "data/shapes-pal.c"
#include "data/night.c"

static void Load(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH);
}

static void UnLoad(void) {
  DeleteBitmap(screen);
}

static void Init(void) {
  /* Set up clipping window. */
  ClipWin.minX = fx4i(0);
  ClipWin.maxX = fx4i(319);
  ClipWin.minY = fx4i(0);
  ClipWin.maxY = fx4i(255);

  plane = DEPTH - 1;
  planeC = 0;

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);
  BitmapClear(screen);

  cp = NewCopList(100);
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen, DEPTH);
  CopLoadPal(cp, &shapes_pal, 0);
  CopEnd(cp);

  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DeleteCopList(cp);
}

static Point2D tmpPoint[2][16];

static void DrawPolygon(Point2D *out, short n) {
  short *pos = (short *)out;
  short x1, y1, x2, y2;

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
  u_char *flags = shape->viewPointFlags;
  short n = shape->polygons;

  while (--n >= 0) {
    u_char clipFlags = 0;
    u_char outside = 0xff;

    Point2D *out = tmpPoint[0];
    short *vertex = (short *)*polygons++;
    short n = (*vertex++) + 1;
    short m = n;

    while (--m >= 0) {
      short k = *vertex++;
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

static void Render(void) {
  // int lines = ReadLineCounter();
  short i, a = frameCount * 64;
  Matrix2D t;

  BlitterClear(screen, plane);
  LoadIdentity2D(&t);
  Rotate2D(&t, frameCount * 8);
  Scale2D(&t, fx12f(1.0) + SIN(a) / 2, fx12f(1.0) + COS(a) / 2);
  Translate2D(&t, fx4i(screen->width / 2), fx4i(screen->height / 2));
  Transform2D(&t, shape.viewPoint, shape.origPoint, shape.points);
  PointsInsideBox(shape.viewPoint, shape.viewPointFlags, shape.points);
  BlitterLineSetup(screen, plane, LINE_EOR|LINE_ONEDOT);
  DrawShape(&shape);
  BlitterFill(screen, plane);
  // Log("shape: %d\n", ReadLineCounter() - lines);

  for (i = 0; i < DEPTH; i++) {
    short j = (plane + i) % DEPTH;
    CopInsSet32(bplptr[i], screen->planes[j]);
  }

  TaskWaitVBlank();

  if (planeC & 1)
    plane = (plane + 1) % DEPTH;

  planeC ^= 1;
}

EFFECT(shapes, Load, UnLoad, Init, Kill, Render);
