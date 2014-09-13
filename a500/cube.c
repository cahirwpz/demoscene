#include "blitter.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

#define WIDTH  320
#define HEIGHT 256

static Object3D *cube;

static CopListT *cp[2];
static BitmapT *screen[2];
static UWORD active = 0;

static void MakeCopperList(CopListT *cp, UWORD num) {
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopShowPlayfield(cp, screen[num]);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xfff);
  CopEnd(cp);
}

void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, 1, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 1, FALSE);
  cube = LoadObject3D("data/cube.3d");

  cp[0] = NewCopList(100);
  MakeCopperList(cp[0], 0);
  cp[1] = NewCopList(100);
  MakeCopperList(cp[1], 1);

  ClipFrustum.near = fx4i(-100);
  ClipFrustum.far = fx4i(-400);
}

void Kill() {
  DeleteObject3D(cube);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static Point3D tmpPoint[2][16];

static void DrawObject(Object3D *object) {
  Point3D *point = object->cameraPoint;
  PolygonT *polygon = object->polygon;
  UBYTE *flags = object->cameraPointFlags;
  UWORD *vertex = object->polygonVertex;
  WORD polygons = object->polygons;
  Point3D *normal = object->polygonNormal;

  for (; polygons--; normal++, polygon++) {
    WORD i, n = polygon->vertices;
    UBYTE clipFlags = 0;
    UBYTE outside = 0xff;
    Point3D *in = tmpPoint[0];
    Point3D *out = tmpPoint[1];

    /* Check polygon visibility. */
    {
      Point3D *p = &point[vertex[polygon->index]];

      if (normal->x * p->x + normal->y * p->y + normal->z * p->z >= 0)
        continue;
    }

    for (i = 0; i < n; i++) {
      UWORD k = vertex[polygon->index + i];

      clipFlags |= flags[k];
      outside &= flags[k];
      in[i] = point[k];
      out[i] = point[k];
    }

    if (outside)
      continue;

    n = ClipPolygon3D(in, &out, n, clipFlags);

    /* Perspective mapping. */
    for (i = 0; i < n; i++) {
      out[i].x = div16(256 * out[i].x, out[i].z) + 160;
      out[i].y = div16(256 * out[i].y, out[i].z) + 128;
    }

    BlitterLineSetup(screen[active], 0, LINE_OR, LINE_SOLID);

    while (--n > 0) {
#if 0
      Log ("(%ld %ld %ld) - (%ld %ld %ld)\n",
           (LONG)out[0].x, (LONG)out[0].y, (LONG)out[0].z,
           (LONG)out[1].x, (LONG)out[1].y, (LONG)out[1].z);
#endif

      BlitterLine(out[0].x, out[0].y, out[1].x, out[1].y);
      WaitBlitter();
      out++;
    }
  }
}

static Point3D rotate = { 0, 0, 0 };

static BOOL Loop() {
  Matrix3D t;

  BlitterClear(screen[active], 0);
  WaitBlitter();

  rotate.x += 4;
  rotate.y += 4;
  rotate.z += 4;

  LoadRotate3D(&t, rotate.x, rotate.y, rotate.z);
  Translate3D(&t, 0, 0, fx4i(-250));
  Transform3D(&t, cube->cameraPoint, cube->point, cube->points);
  PointsInsideFrustum(cube->cameraPoint, cube->cameraPointFlags, cube->points);
  UpdatePolygonNormals(cube);

  DrawObject(cube);

  return !LeftMouseButton();
}

void Init() {
  CopListActivate(cp[active]);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

void Main() {
  while (Loop()) {
    CopListRun(cp[active]);
    WaitVBlank();
    active ^= 1;
  }
}
