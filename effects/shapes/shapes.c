#include <effect.h>
#include <2d.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <system/memory.h>

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  4

static BitmapT *screen;
static CopInsPairT *bplptr;
static CopListT *cp;
static short plane, planeC;

#include "data/shapes-pal.c"
#include "data/night.c"

static void Init(void) {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  /* Set up clipping window. */
  ClipWin.minX = fx4i(0);
  ClipWin.maxX = fx4i(319);
  ClipWin.minY = fx4i(0);
  ClipWin.maxY = fx4i(255);

  plane = DEPTH - 1;
  planeC = 0;

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);
  BitmapClear(screen);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  LoadColors(shapes_colors, 0);

  cp = NewCopList(100);
  bplptr = CopSetupBitplanes(cp, screen, DEPTH);
  CopListFinish(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DeleteCopList(cp);
  DeleteBitmap(screen);
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

static void DrawShape(ShapeT *shape) {
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

PROFILE(Shapes);

static void Render(void) {
  short i;

  ProfilerStart(Shapes);
  {
    short a = frameCount * 64;
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
  }
  ProfilerStop(Shapes);

  for (i = 0; i < DEPTH; i++) {
    short j = (plane + i) % DEPTH;
    CopInsSet32(&bplptr[i], screen->planes[j]);
  }

  TaskWaitVBlank();

  if (planeC & 1)
    plane = (plane + 1) % DEPTH;

  planeC ^= 1;
}

EFFECT(Shapes, NULL, NULL, Init, Kill, Render, NULL);
