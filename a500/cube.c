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

static CopListT *cp;
static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[8];

static Box2D box = { 40, 32, 280 - 1, 224 - 1 };

Object3D *NewObject3D(UWORD points, UWORD edges) {
  Object3D *object = AllocMemSafe(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);

  object->points = points;
  object->edges = edges;

  object->point = AllocMemSafe(sizeof(Point3D) * points, MEMF_PUBLIC);
  object->edge = AllocMemSafe(sizeof(EdgeT) * edges, MEMF_PUBLIC);
  object->cameraPoint = AllocMemSafe(sizeof(Point3D) * points, MEMF_PUBLIC);
  object->frustumPointFlags = AllocMemSafe(points, MEMF_PUBLIC);

  return object;
}

void DeleteObject3D(Object3D *object) {
  FreeMem(object->point, sizeof(Point3D) * object->points);
  FreeMem(object->edge, sizeof(EdgeT) * object->edges);
  FreeMem(object->cameraPoint, sizeof(Point3D) * object->points);
  FreeMem(object->frustumPointFlags, object->points);
  FreeMem(object, sizeof(Object3D));
}

Object3D *LoadObject3D(char *filename) {
  char *file = ReadFile(filename, MEMF_PUBLIC);
  char *data = file;
  Object3D *object = NULL;
  WORD i, points, edges;

  if (!file)
    return NULL;
  
  if (ReadNumber(&data, &points) && ReadNumber(&data, &edges)) {
    object = NewObject3D(points, edges);

    for (i = 0; i < object->points; i++) {
      if (!ReadNumber(&data, &object->point[i].x) ||
          !ReadNumber(&data, &object->point[i].y) ||
          !ReadNumber(&data, &object->point[i].z))
        goto error;
    }

    for (i = 0; i < object->edges; i++) {
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
  WORD n = object->edges;
  Point3D *point = object->cameraPoint;
  UBYTE *frustumFlags = object->frustumPointFlags;
  EdgeT *edge = object->edge;

  while (n--) {
    UWORD i1 = edge->p1;
    UWORD i2 = edge->p2;
    BOOL outside = frustumFlags[i1] & frustumFlags[i2];
    BOOL clip = frustumFlags[i1] | frustumFlags[i2];

    if (!outside) {
      Line2D line = { point[i1].x, point[i1].y, point[i2].x, point[i2].y };

      if (!outside && clip)
        outside = !ClipLine2D(&line, &box);

      if (!outside) {
        WaitBlitter();
        BlitterLine(screen[active], 0, LINE_OR, LINE_SOLID, &line);
      }
    }

    edge++;
  }
}

static Point3D rotate = { 0, 0, 0 };

static BOOL Loop() {
  Matrix3D t;

  WaitBlitter();
  BlitterClear(screen[active], 0);

  LoadRotate3D(&t, rotate.x++, rotate.y++, rotate.z++);
  Translate3D(&t, 160, 128, 300);
  Transform3D(&t, cube->cameraPoint, cube->point, cube->points);
  PointsInsideFrustum(cube->cameraPoint, cube->frustumPointFlags, cube->points, 10, 1000);

  DrawObject(cube);

#if 0
  WaitBlitter();
  BlitterLine(screen[active], 0, LINE_OR, LINE_SOLID, box.minX, box.minY, box.maxX, box.minY);
  WaitBlitter();
  BlitterLine(screen[active], 0, LINE_OR, LINE_SOLID, box.maxX, box.minY, box.maxX, box.maxY);
  WaitBlitter();
  BlitterLine(screen[active], 0, LINE_OR, LINE_SOLID, box.maxX, box.maxY, box.minX, box.maxY);
  WaitBlitter();
  BlitterLine(screen[active], 0, LINE_OR, LINE_SOLID, box.minX, box.maxY, box.minX, box.minY);
#endif

  WaitBlitter();
  WaitVBlank();
  CopInsSet32(bplptr[0], screen[active]->planes[0]);

  active ^= 1;

  return !LeftMouseButton();
}

void Main() {
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  while (Loop());
}
