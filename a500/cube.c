#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  4

static Object3D *cube;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen;
static UWORD active = 0;

static void Load() {
  cube = LoadOBJ("data/cube.3d");
}

static void UnLoad() {
  DeleteObject3D(cube);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr, screen, DEPTH);
  CopSetRGB(cp,  0, 0x000);
  CopSetRGB(cp,  1, 0x111);
  CopSetRGB(cp,  2, 0x222);
  CopSetRGB(cp,  3, 0x333);
  CopSetRGB(cp,  4, 0x444);
  CopSetRGB(cp,  5, 0x555);
  CopSetRGB(cp,  6, 0x666);
  CopSetRGB(cp,  7, 0x777);
  CopSetRGB(cp,  8, 0x888);
  CopSetRGB(cp,  9, 0x999);
  CopSetRGB(cp, 10, 0xAAA);
  CopSetRGB(cp, 11, 0xBBB);
  CopSetRGB(cp, 12, 0xCCC);
  CopSetRGB(cp, 13, 0xDDD);
  CopSetRGB(cp, 14, 0xEEE);
  CopSetRGB(cp, 15, 0xFFF);
  CopEnd(cp);
}

static void Init() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH * 2);

  cp = NewCopList(100);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;

  ClipFrustum.near = fx4i(-100);
  ClipFrustum.far = fx4i(-400);
}

static void Kill() {
  DeleteBitmap(screen);
  DeleteCopList(cp);
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

    BlitterLineSetup(screen, active, LINE_OR, LINE_SOLID);

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

static void Render() {
  LONG a = ReadFrameCounter() * 8;
  Matrix3D t;

  BlitterClearSync(screen, active);

  LoadRotate3D(&t, a, a, a);
  Translate3D(&t, 0, 0, fx4i(-250));
  Transform3D(&t, cube->cameraPoint, cube->point, cube->points);
  PointsInsideFrustum(cube->cameraPoint, cube->cameraPointFlags, cube->points);
  UpdatePolygonNormals(cube);

  DrawObject(cube);

  WaitVBlank();

  {
    WORD n = DEPTH;

    while (--n >= 0)
      CopInsSet32(bplptr[n], screen->planes[(active + n + 1 - DEPTH) & 7]);
  }

  active = (active + 1) & 7;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
