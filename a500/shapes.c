#include "2d.h"
#include "blitter.h"
#include "coplist.h"
#include "interrupts.h"

static BitmapT *screen;
static CopInsT *bplptr[5];
static CopListT *cp;

typedef struct Shape {
  UWORD nPoints;
  UWORD nEdges;

  PointT *points;
  PointT *outPoints;
  EdgeT *edges;
} ShapeT;

static PointT sPoints[] = {
  { -50, -50 },
  { -50,  50 },
  {  50,  50 },
  {  50, -50 }
};

static PointT sOutPoints[4];

static EdgeT sEdges[] = {
  { 0, 1 },
  { 1, 2 },
  { 2, 3 },
  { 3, 0 }
};

static ShapeT shape = { 4, 4, sPoints, sOutPoints, sEdges };

static WORD plane, planeC;

void Load() {
  screen = NewBitmap(320, 256, 5, FALSE);
  cp = NewCopList(100);

  plane = screen->depth - 1;
  planeC = 0;
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void DrawShape(ShapeT *shape) {
  PointT *point = shape->outPoints;
  EdgeT *edge = shape->edges;
  UWORD n = shape->nEdges;

  while (n--) {
    UWORD i1 = edge->p1;
    UWORD i2 = edge->p2;

    WaitBlitter();
    BlitterLine(screen, plane, LINE_EOR, ONEDOT,
                point[i1].x, point[i1].y, point[i2].x, point[i2].y);

    edge++;
  }
}

static ULONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB)
    frameCount++;

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

static BOOL Loop() {
  UWORD i, a = (frameCount * 8) & 0x1ff;
  View2D t;

  BlitterClear(screen, plane);

  Identity2D(&t);
  Rotate2D(&t, frameCount);
  Scale2D(&t, 256 + sincos[a].sin / 2, 256 + sincos[a].cos / 2);
  Translate2D(&t, screen->width / 2, screen->height / 2);
  Transform2D(&t, shape.outPoints, shape.points, shape.nPoints);
  DrawShape(&shape);

  WaitBlitter();
  BlitterFill(screen, plane);
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

  return !LeftMouseButton();
}

void Main() {
  APTR OldIntLevel3;

  OldIntLevel3 = InterruptVector->IntLevel3;
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
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

  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  while (Loop());

  custom->intena = INTF_LEVEL3;
  InterruptVector->IntLevel3 = OldIntLevel3;
}
