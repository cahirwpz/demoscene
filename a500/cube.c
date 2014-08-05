#include "blitter.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

#define WIDTH  320
#define HEIGHT 256

static Object3D *cube;

static CopListT *cp;
static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[8];

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

  ClipFrustum.near = fx4i(-100);
  ClipFrustum.far = fx4i(-400);
}

void Kill() {
  DeleteObject3D(cube);
  DeleteCopList(cp);
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

    while (--n > 0) {
      Line2D line;

      line.x1 = out->x;
      line.y1 = out->y;
      //Log ("(%ld %ld %ld) -", (LONG)out->x, (LONG)out->y, (LONG)out->z);

      out++;
      line.x2 = out->x;
      line.y2 = out->y;
      //Log ("- (%ld %ld %ld)\n", (LONG)out->x, (LONG)out->y, (LONG)out->z);

      WaitBlitter();
      BlitterLine(screen[active], 0, LINE_OR, LINE_SOLID, &line);
    }
  }
}

static Point3D rotate = { 0, 0, 0 };

static BOOL Loop() {
  Matrix3D t;

  WaitBlitter();
  BlitterClear(screen[active], 0);

  rotate.x += 4;
  rotate.y += 4;
  rotate.z += 4;

  LoadRotate3D(&t, rotate.x, rotate.y, rotate.z);
  Translate3D(&t, 0, 0, fx4i(-250));
  Transform3D(&t, cube->cameraPoint, cube->point, cube->points);
  PointsInsideFrustum(cube->cameraPoint, cube->cameraPointFlags, cube->points);
  UpdatePolygonNormals(cube);

  DrawObject(cube);

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
