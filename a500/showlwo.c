#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"
#include "memory.h"

#define WIDTH  320
#define HEIGHT 256
#define DEPTH 1

static Object3D *cube;
static CopListT *cp;
static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[DEPTH];

static void Load() {
  cube = LoadLWO("data/new_2.lwo", SPFlt(80));
  CalculateEdges(cube);
}

static void UnLoad() {
  DeleteObject3D(cube);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);
}

static void Init() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, 1);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 1);

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

static void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

__regargs static void CalculatePerspective(Point3D *p, WORD points) {
  WORD *src = (WORD *)p;
  WORD *dst = (WORD *)p;
  WORD n = points;

  while (--n >= 0) {
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;

    *dst++ = div16(256 * x, z) + WIDTH / 2;
    *dst++ = div16(256 * y, z) + HEIGHT / 2;
    dst++;
  }
}

__regargs static void DrawObject(Object3D *object) {
  Point3D *point = object->cameraPoint;
  UWORD *edge = (UWORD *)object->edge;
  WORD edges = object->edges;

  BlitterLineSetup(screen[active], 0, LINE_OR, LINE_SOLID);

  while (--edges >= 0) {
    Point3D *p0 = (APTR)point + (ULONG)*edge++;
    Point3D *p1 = (APTR)point + (ULONG)*edge++;
    BlitterLineSync(p0->x, p0->y, p1->x, p1->y);
  }
}

static Point3D rotate = { 0, 0, 0 };

static void Render() {
  Matrix3D t;

  rotate.x += 16;
  rotate.y += 16;
  rotate.z += 16;

  {
    LONG lines = ReadLineCounter();
    LoadRotate3D(&t, rotate.x, rotate.y, rotate.z);
    Translate3D(&t, 0, 0, fx4i(-250));
    Transform3D(&t, cube->cameraPoint, cube->point, cube->points);

    CalculatePerspective(cube->cameraPoint, cube->points);
    Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  BitmapClear(screen[active], DEPTH);
  WaitBlitter();

  {
    LONG lines = ReadLineCounter();
    DrawObject(cube);
    Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();
  ITER(i, 0, DEPTH - 1, CopInsSet32(bplptr[i], screen[active]->planes[i]));
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
