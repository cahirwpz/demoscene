#include "blitter.h"
#include "memory.h"
#include "coplist.h"
#include "file.h"
#include "3d.h"
#include "reader.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

#define WIDTH  320
#define HEIGHT 256

static Object3D *cube;
static View3D view3d;

static CopListT *cp;
static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[8];

static BoxT box = { 40, 32, 280 - 1, 224 - 1 };

Object3D *NewObject3D(UWORD nVertex, UWORD nEdge) {
  Object3D *object = AllocMemSafe(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);

  object->nVertex = nVertex;
  object->nEdge = nEdge;

  object->vertex = AllocMemSafe(sizeof(VertexT) * nVertex, MEMF_PUBLIC);
  object->edge = AllocMemSafe(sizeof(EdgeT) * nEdge, MEMF_PUBLIC);
  object->point = AllocMemSafe(sizeof(PointT) * nVertex, MEMF_PUBLIC);
  object->pointFlags = AllocMemSafe(sizeof(UBYTE) * nVertex, MEMF_PUBLIC);

  return object;
}

void DeleteObject3D(Object3D *object) {
  FreeMem(object->vertex, sizeof(VertexT) * object->nVertex);
  FreeMem(object->edge, sizeof(EdgeT) * object->nEdge);
  FreeMem(object->point, sizeof(PointT) * object->nVertex);
  FreeMem(object->pointFlags, sizeof(UBYTE) * object->nVertex);
  FreeMem(object, sizeof(Object3D));
}

Object3D *LoadObject3D(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  char *data = file;
  Object3D *object = NULL;
  WORD i, nVertex, nEdge;

  if (!file)
    return NULL;
  
  if (ReadNumber(&data, &nVertex) && ReadNumber(&data, &nEdge)) {
    object = NewObject3D(nVertex, nEdge);

    for (i = 0; i < object->nVertex; i++) {
      if (!ReadNumber(&data, &object->vertex[i].x) ||
          !ReadNumber(&data, &object->vertex[i].y) ||
          !ReadNumber(&data, &object->vertex[i].z))
        goto error;
    }

    for (i = 0; i < object->nEdge; i++) {
      if (!ReadNumber(&data, &object->edge[i].p1) ||
          !ReadNumber(&data, &object->edge[i].p2))
        goto error;
    }

    FreeAutoMem(file);
    return object;
  }

error:
  DeleteObject3D(object);
  FreeAutoMem(file);
  return NULL;
}

void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, 1, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 1, FALSE);
  cube = LoadObject3D("data/cube.3d");
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[0]);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);
}

void Kill() {
  DeleteObject3D(cube);
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
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
        BlitterLine(screen[active], 0, LINE_OR, 0, line.x1, line.y1, line.x2, line.y2);
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
    WaitBlitter();
    BlitterClear(screen[active], 0);

    view3d.rotateX++;
    view3d.rotateY++;
    view3d.rotateZ++;

    CalculateView3D(&view3d);
    TransformVertices(&view3d, cube);
    PointsInsideBox(cube->point, cube->pointFlags, cube->nVertex, &box);

    DrawObject(cube);

    WaitBlitter();
    BlitterLine(screen[active], 0, LINE_OR, 0, box.minX, box.minY, box.maxX, box.minY);
    WaitBlitter();
    BlitterLine(screen[active], 0, LINE_OR, 0, box.maxX, box.minY, box.maxX, box.maxY);
    WaitBlitter();
    BlitterLine(screen[active], 0, LINE_OR, 0, box.maxX, box.maxY, box.minX, box.maxY);
    WaitBlitter();
    BlitterLine(screen[active], 0, LINE_OR, 0, box.minX, box.maxY, box.minX, box.minY);

    WaitBlitter();
    WaitVBlank();
    CopInsSet32(bplptr[0], screen[active]->planes[0]);

    active ^= 1;
  }
}
