#include "2d.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "interrupts.h"
#include "file.h"
#include "reader.h"

typedef struct Shape {
  UWORD nPoint;
  UWORD nEdge;

  PointT *point;
  PointT *outPoint;
  EdgeT *edge;
} ShapeT;

ShapeT *NewShape(UWORD nPoint, UWORD nEdge) {
  ShapeT *shape = AllocMemSafe(sizeof(ShapeT), MEMF_PUBLIC|MEMF_CLEAR);

  shape->nPoint = nPoint;
  shape->nEdge = nEdge;

  shape->point = AllocMemSafe(sizeof(PointT) * nPoint, MEMF_PUBLIC);
  shape->outPoint = AllocMemSafe(sizeof(PointT) * nPoint, MEMF_PUBLIC);
  shape->edge = AllocMemSafe(sizeof(EdgeT) * nEdge, MEMF_PUBLIC);

  return shape;
}

void DeleteShape(ShapeT *shape) {
  FreeMem(shape->edge, sizeof(EdgeT) * shape->nEdge);
  FreeMem(shape->outPoint, sizeof(PointT) * shape->nPoint);
  FreeMem(shape->point, sizeof(PointT) * shape->nPoint);
  FreeMem(shape, sizeof(ShapeT));
}

ShapeT *LoadShape(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  char *data = file;
  ShapeT *shape = NULL;
  WORD i, nPoint, nEdge;

  if (!file)
    return NULL;
  
  if (ReadNumber(&data, &nPoint) && ReadNumber(&data, &nEdge)) {
    shape = NewShape(nPoint, nEdge);

    for (i = 0; i < shape->nPoint; i++) {
      if (!ReadNumber(&data, &shape->point[i].x) ||
          !ReadNumber(&data, &shape->point[i].y))
        goto error;
    }

    for (i = 0; i < shape->nEdge; i++) {
      if (!ReadNumber(&data, &shape->edge[i].p1) ||
          !ReadNumber(&data, &shape->edge[i].p2))
        goto error;
    }

    FreeAutoMem(file);
    return shape;
  }

error:
  DeleteShape(shape);
  FreeAutoMem(file);
  return NULL;
}

static ShapeT *shape;
static BitmapT *screen;
static CopInsT *bplptr[5];
static CopListT *cp;
static WORD plane, planeC;

void Load() {
  screen = NewBitmap(320, 256, 5, FALSE);
  shape = LoadShape("data/box.2d");
  cp = NewCopList(100);

  plane = screen->depth - 1;
  planeC = 0;
}

void Kill() {
  DeleteShape(shape);
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void DrawShape(ShapeT *shape) {
  PointT *point = shape->outPoint;
  EdgeT *edge = shape->edge;
  UWORD n = shape->nEdge;

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
  Transform2D(&t, shape->outPoint, shape->point, shape->nPoint);
  DrawShape(shape);

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
