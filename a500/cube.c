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
  CalculateVertexPolygonMap(cube);
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

static inline void DrawLine(APTR start, Point2D *p0, Point2D *p1) {
  WORD x0, y0, x1, y1;

  if (p0->y > p1->y)
    swapr(p0, p1);

  x0 = p0->x;
  y0 = p0->y;
  x1 = p1->x;
  y1 = p1->y;

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
    WORD *v = polygon->indices;

    WORD *p1 = (WORD *)&point[*v++];
    WORD *p2 = (WORD *)&point[*v++];
    WORD *p3 = (WORD *)&point[*v++];
    WORD *p0 = p1;

    WORD ax, ay, az, bx, by, bz;
    LONG n;

    ax = *p1++; bx = *p2++; ax -= bx; bx -= *p3++;
    ay = *p1++; by = *p2++; ay -= by; by -= *p3++;
    az = *p1++; bz = *p2++; az -= bz; bz -= *p3++;

    n  = normfx(ay * bz - by * az) * (*p0++);
    n += normfx(az * bx - bz * ax) * (*p0++);
    n += normfx(ax * by - bx * ay) * (*p0++);

    *flags++ = n < 0;
  }
}

static __regargs void PerspectiveProjection(Object3D *object) {
  WORD *src = (WORD *)object->cameraPoint;
  WORD *dst = (WORD *)object->screenPoint;
  IndexListT **vertexPolygons = object->vertexPolygon;
  UBYTE *flags = object->polygonFlags;
  WORD n = object->vertices;

  do {
    IndexListT *vp = *vertexPolygons++;
    WORD count = vp->count - 1;
    WORD *index = vp->indices;

    do {
      if (flags[*index++]) {
        WORD x = *src++;
        WORD y = *src++;
        WORD z = *src++;

        *dst++ = div16(x << 8, z) + WIDTH / 2;
        *dst++ = div16(y << 8, z) + HEIGHT / 2;

        goto end;
      }
    } while (--count != -1);

    src += 3;
    dst += 2;
end:
  } while (--n > 0);
}

static __regargs void DrawObject(Object3D *object, APTR start) {
  IndexListT **polygons = object->polygon;
  IndexListT *polygon;
  UBYTE *flags = object->polygonFlags;
  Point2D *point = object->screenPoint;

  while ((polygon = *polygons++)) {
    if (*flags++) {
      WORD *index = polygon->indices;
      WORD n = polygon->count;
      Point2D *p0 = &point[index[n - 1]];
      Point2D *p1;

      while (--n >= 0) {
        p1 = &point[*index++];
        DrawLine(start, p0, p1);
        p0 = p1; 
      }
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
    Transform3D(&t, cube->cameraPoint, cube->vertex, cube->vertices);
    UpdatePolygonNormals2(cube);
    PerspectiveProjection(cube);
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
    DrawObject(cube, screen[active]->planes[0]);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();
  CopInsSet32(bplptr[0], screen[active]->planes[0]);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
