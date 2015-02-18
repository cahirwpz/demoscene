#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH  1

static Object3D *cube;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen[2];
static UWORD active = 0;

static void Load() {
  cube = LoadLWO("data/codi.lwo", SPFlt(256));
}

static void UnLoad() {
  DeleteObject3D(cube);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakeDispWin(cp, X(32), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopSetRGB(cp,  0, 0x000);
  CopSetRGB(cp,  1, 0xFFF);
  CopEnd(cp);
}

static void Init() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

static void Kill() {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
}

static inline void DrawLine(APTR start, WORD x0, WORD y0, WORD x1, WORD y1) {
  if (y0 > y1) {
    swapr(x0, x1);
    swapr(y0, y1);
  }

  {
    APTR data = start + (((y0 << 5) + (x0 >> 3)) & ~1);
    WORD dmax = x1 - x0;
    WORD dmin = y1 - y0;
    WORD derr;
    UWORD bltcon1 = LINE_SOLID;

    if (dmax < 0)
      dmax = -dmax;

    if (dmax >= dmin) {
      if (x0 >= x1)
        bltcon1 |= (AUL | SUD);
      else
        bltcon1 |= SUD;
    } else {
      if (x0 >= x1)
        bltcon1 |= SUL;
      swapr(dmax, dmin);
    }

    derr = 2 * dmin - dmax;
    if (derr < 0)
      bltcon1 |= SIGNFLAG;
    bltcon1 |= rorw(x0 & 15, 4);

    {
      UWORD bltcon0 = rorw(x0 & 15, 4) | LINE_OR;
      UWORD bltamod = derr - dmax;
      UWORD bltbmod = 2 * dmin;
      UWORD bltsize = (dmax << 6) + 66;
      APTR bltapt = (APTR)(LONG)derr;

      WaitBlitter();

      custom->bltcon0 = bltcon0;
      custom->bltcon1 = bltcon1;
      custom->bltamod = bltamod;
      custom->bltbmod = bltbmod;
      custom->bltapt = bltapt;
      custom->bltcpt = data;
      custom->bltdpt = data;
      custom->bltsize = bltsize;
    }
  }
}

static void UpdatePolygonNormals2(Object3D *object) {
  Point3D *point = object->cameraPoint;
  UBYTE *flags = object->polygonFlags;
  IndexListT **polygons = object->polygon;
  IndexListT *polygon;

  while ((polygon = *polygons++)) {
    UWORD *v = polygon->indices;

    Point3D *p1 = &point[*v++];
    Point3D *p2 = &point[*v++];
    Point3D *p3 = &point[*v++];

    WORD ax = p1->x - p2->x;
    WORD ay = p1->y - p2->y;
    WORD az = p1->z - p2->z;
    WORD bx = p2->x - p3->x;
    WORD by = p2->y - p3->y;
    WORD bz = p2->z - p3->z;

    WORD x = normfx(ay * bz - by * az);
    WORD y = normfx(az * bx - bz * ax);
    WORD z = normfx(ax * by - bx * ay);

    *flags++ = (x * p1->x + y * p1->y + z * p1->z < 0);
  }
}

static void PerspectiveProjection(Point2D *out, Point3D *in, WORD n) {
  WORD *src = (WORD *)in;
  WORD *dst = (WORD *)out;

  while (--n >= 0) {
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;

    *dst++ = div16(256 * x, z) + WIDTH / 2;
    *dst++ = div16(256 * y, z) + HEIGHT / 2;
  }
}

static void DrawObject(Object3D *object) {
  Point2D *point = object->screenPoint;
  IndexListT **polygons = object->polygon;
  IndexListT *polygon;
  UBYTE *flags = object->polygonFlags;
  APTR start = screen[active]->planes[0];

  while ((polygon = *polygons++)) {
    if (*flags++) {
      WORD *index = polygon->indices;
      Point2D *pf = &point[*index++];
      Point2D *p0 = pf;
      Point2D *p1 = &point[*index];
      WORD n = polygon->count;

      while (--n >= 0) {
        DrawLine(start, p0->x, p0->y, p1->x, p1->y);
        p0 = p1; p1 = &point[*index++];
      }

      DrawLine(start, p0->x, p0->y, pf->x, pf->y);
    }
  }
}

static void Render() {
  LONG a = ReadFrameCounter() * 8;
  Matrix3D t;

  BlitterClearSync(screen[active], 0);

  {
    // LONG lines = ReadLineCounter();
    LoadRotate3D(&t, a, a, a);
    Translate3D(&t, 0, 0, fx4i(-250));
    Transform3D(&t, cube->cameraPoint, cube->point, cube->points);
    UpdatePolygonNormals2(cube);
    PerspectiveProjection(cube->screenPoint, cube->cameraPoint, cube->points);
    // Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = WIDTH / 8;
  custom->bltdmod = WIDTH / 8;

  {
    // LONG lines = ReadLineCounter();
    DrawObject(cube);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();
  CopInsSet32(bplptr[0], screen[active]->planes[0]);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
