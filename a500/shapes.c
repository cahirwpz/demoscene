#include "2d.h"
#include "blitter.h"
#include "coplist.h"
#include "fx.h"
#include "interrupts.h"
#include "memory.h"

static ShapeT *shape;
static BitmapT *screen;
static CopInsT *bplptr[5];
static CopListT *cp;
static WORD plane, planeC;

void Load() {
  screen = NewBitmap(320, 256, 5, FALSE);
  shape = LoadShape("data/boxes.2d");
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen);
  CopMakeDispWin(cp, 0x81, 0x2c, screen->width, screen->height);
  {
    UWORD i, j = 2;

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
  }
  CopEnd(cp);

  plane = screen->depth - 1;
  planeC = 0;

  /* Set up clipping window. */
  ClipWin.minX = fx4i(0);
  ClipWin.maxX = fx4i(319);
  ClipWin.minY = fx4i(0);
  ClipWin.maxY = fx4i(255);
}

void Kill() {
  DeleteShape(shape);
  DeleteCopList(cp);
  DeleteBitmap(screen);
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

static ULONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB)
    frameCount++;

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

void Init() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

void Main() {
  while (!LeftMouseButton()) {
    UWORD i, a = frameCount * 64;
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
      WORD j = plane + i;

      if (j >= screen->depth)
        j -= screen->depth;

      CopInsSet32(bplptr[i], screen->planes[j]);
    }

    if (planeC & 1) {
      plane++;
      if (plane >= screen->depth)
        plane -= screen->depth;
    }
    planeC ^= 1;
  }
}
