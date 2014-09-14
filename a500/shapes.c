#include "startup.h"
#include "2d.h"
#include "blitter.h"
#include "coplist.h"
#include "fx.h"
#include "memory.h"

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  5

static ShapeT *shape;
static BitmapT *screen;
static CopInsT *bplptr[5];
static CopListT *cp;
static WORD plane, planeC;

static void Load() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  shape = LoadShape("data/boxes.2d");
  cp = NewCopList(100);
}

static void UnLoad() {
  DeleteShape(shape);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void MakeCopperList(CopListT *cp) {
  UWORD i, j = 2;

  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopMakePlayfield(cp, bplptr, screen);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0x111);
  for (i = 0; i < 2; i++)
    CopSetRGB(cp, j++, 0x222);
  for (i = 0; i < 4; i++)
    CopSetRGB(cp, j++, 0x444);
  for (i = 0; i < 8; i++)
    CopSetRGB(cp, j++, 0x888);
  for (i = 0; i < 16; i++)
    CopSetRGB(cp, j++, 0xfff);
  CopEnd(cp);
}

static void Init() {
  plane = screen->depth - 1;
  planeC = 0;

  /* Set up clipping window. */
  ClipWin.minX = fx4i(0);
  ClipWin.maxX = fx4i(319);
  ClipWin.minY = fx4i(0);
  ClipWin.maxY = fx4i(255);

  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

static Point2D tmpPoint[2][16];

static __regargs void DrawShape(ShapeT *shape) {
  Point2D *point = shape->viewPoint;
  PolygonT *polygon = shape->polygon;
  UBYTE *flags = shape->viewPointFlags;
  UWORD *vertex = shape->polygonVertex;
  UWORD polygons = shape->polygons;

  while (polygons--) {
    WORD i, n = polygon->vertices;
    UBYTE clipFlags = 0;
    UBYTE outside = 0xff;
    Point2D *in = tmpPoint[0];
    Point2D *out = tmpPoint[1];

    for (i = 0; i < n; i++) {
      UWORD k = vertex[polygon->index + i];

      clipFlags |= flags[k];
      outside &= flags[k];
      in[i] = point[k];
    }

    if (!outside) {
      n = ClipPolygon2D(in, &out, n, clipFlags);

      for (i = 0; i < n; i++) {
        out[i].x /= 16;
        out[i].y /= 16;
      }

      BlitterLineSetup(screen, plane, LINE_EOR, LINE_ONEDOT);

      while (--n > 0) {
        Line2D *line = (Line2D *)out++;

        BlitterLine(line->x1, line->y1, line->x2, line->y2);
        WaitBlitter();
      }
    }

    polygon++;
  }
}

static void Render() {
  WORD i, a = frameCount * 64;
  Matrix2D t;

  BlitterClear(screen, plane);
  WaitBlitter();

  LoadIdentity2D(&t);
  Rotate2D(&t, frameCount * 8);
  Scale2D(&t, fx12f(1.0) + SIN(a) / 2, fx12f(1.0) + COS(a) / 2);
  Translate2D(&t, fx4i(screen->width / 2), fx4i(screen->height / 2));
  Transform2D(&t, shape->viewPoint, shape->origPoint, shape->points);
  PointsInsideBox(shape->viewPoint, shape->viewPointFlags, shape->points);

  {
    // LONG lines = ReadLineCounter();
    DrawShape(shape);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  BlitterFill(screen, plane);
  WaitBlitter();

  WaitVBlank();

  for (i = 0; i < screen->depth; i++) {
    WORD j = (plane + i) % DEPTH;
    CopInsSet32(bplptr[i], screen->planes[j]);
  }

  if (planeC & 1)
    plane = (plane + 1) % DEPTH;

  planeC ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, NULL, Render };
