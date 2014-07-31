#include "blitter.h"
#include "coplist.h"
#include "3d.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

static Object3D object;
static View3D view3d;

static BitmapT *screen;
static CopListT *cp;

static BoxT box = { 40, 32, 280 - 1, 224 - 1 };

void Load() {
  screen = NewBitmap(320, 256, 1, FALSE);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, NULL, screen);
  CopMakeDispWin(cp, X(0), Y(0), screen->width, screen->height);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);

  {
    static VertexT vertex[8] = {
      { -100, -100, -100 },
      {  100, -100, -100 },
      {  100,  100, -100 },
      { -100,  100, -100 },
      { -100, -100,  100 },
      {  100, -100,  100 },
      {  100,  100,  100 },
      { -100,  100,  100 }
    };

    static EdgeT edge[12] = {
      { 0, 1 },
      { 1, 2 },
      { 2, 3 },
      { 3, 0 },
      { 4, 5 },
      { 5, 6 },
      { 6, 7 },
      { 7, 4 },
      { 0, 4 },
      { 1, 5 },
      { 2, 6 },
      { 3, 7 }
    };

    static PointT point[8];
    static UBYTE pointFlags[8];

    object.nVertex = 8;
    object.nEdge = 12;
    object.vertex = vertex;
    object.edge = edge;
    object.point = point;
    object.pointFlags = pointFlags;
  }
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

static void DrawObject(Object3D *object) {
  WORD n = object->nEdge;
  PointT *point = object->point;
  UBYTE *pointFlags = object->pointFlags;
  EdgeT *edge = object->edge;

  while (n--) {
    UWORD i1 = edge->p1;
    UWORD i2 = edge->p2;
    BOOL outside = pointFlags[i1] & pointFlags[i2];
    BOOL clip = pointFlags[i1] | pointFlags[i2];

    if (!outside) {
      LineT line = { point[i1].x, point[i1].y, point[i2].x, point[i2].y };

      if (!outside && clip)
        outside = !ClipLine(&line, &box);

      if (!outside) {
        WaitBlitter();
        BlitterLine(screen, 0, LINE_OR, 0, line.x1, line.y1, line.x2, line.y2);
      }
    }

    edge++;
  }
}

void Main() {
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  view3d.viewerX = 160;
  view3d.viewerY = 128;
  view3d.viewerZ = 300;

  while (!LeftMouseButton()) {
    view3d.rotateX++;
    view3d.rotateY++;
    view3d.rotateZ++;

    CalculateView3D(&view3d);
    TransformVertices(&view3d, &object);
    PointsInsideBox(object.point, object.pointFlags, object.nVertex, &box);

    WaitLine(Y(180));

    WaitBlitter();
    BlitterClear(screen, 0);

    DrawObject(&object);

    WaitBlitter();
    BlitterLine(screen, 0, LINE_OR, 0, box.minX, box.minY, box.maxX, box.minY);
    WaitBlitter();
    BlitterLine(screen, 0, LINE_OR, 0, box.maxX, box.minY, box.maxX, box.maxY);
    WaitBlitter();
    BlitterLine(screen, 0, LINE_OR, 0, box.maxX, box.maxY, box.minX, box.maxY);
    WaitBlitter();
    BlitterLine(screen, 0, LINE_OR, 0, box.minX, box.maxY, box.minX, box.minY);
  }
}
